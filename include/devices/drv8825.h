#pragma once

#include "device_memory.h"
#include "device_mapping.h"
#include "devices.h"
#include "pico_resource.h"

namespace PicoDriver {
    struct NoDirectionPin {};
    struct NoEnablePin {};

    using DRV8825SettingsType = uint8_t;

    namespace DRV8825Settings {
        static inline constexpr uint8_t DisableAfterSteps = 0b1;
    }

    template<typename DeviceResources, typename StepPin, typename DirPin, typename EnablePin, typename Freq>
    requires (Freq::value > 0)
    class DRV8825;

    template<typename DirPin, typename EnablePin>
    struct StepperTagName {
        static inline constexpr char Name[] = "DEStepper";
    };

    template<typename EnablePin>
    struct StepperTagName<NoDirectionPin, EnablePin> {
        static inline constexpr char Name[] = "EStepper";
    };

    template<typename DirPin>
    struct StepperTagName<DirPin, NoEnablePin> {
        static inline constexpr char Name[] = "DStepper";
    };

    template<>
    struct StepperTagName<NoDirectionPin, NoEnablePin> {
        static inline constexpr char Name[] = "Stepper";
    };

    template<typename DirPin, typename EnablePin>
    struct StepperMotorTag : public StepperTagName<DirPin, EnablePin> {
        static inline constexpr uint8_t Id = 0x3 
                                        + (std::is_same_v<DirPin, NoDirectionPin> ? 1 : 0)  // 4 only direction pin
                                        + (std::is_same_v<EnablePin, NoEnablePin> ? 2 : 0); // 5 only enable pin, 6 with both
    };

    template<typename DeviceResource, typename StepPin, typename DirPin, typename EnablePin, typename Freq>
    struct MapToType<DRV8825<DeviceResource, StepPin, DirPin, EnablePin, Freq>> {
        using TagType = StepperMotorTag<DirPin, EnablePin>;
        using Type = MemoryRepresentation<TagType>;

        // TODO: use this type to allocate resources at compile time; so pins can't be used multiple times, except when using as a bus (e.g. i2c)
        using UsedPins = std::tuple<StepPin, DirPin, EnablePin>;
    };

    enum struct StepperDirection : uint8_t { Left, Right };

    template<typename Pin>
    struct EnableValue {
        bool enable;
    };

    template<>
    struct EnableValue<NoEnablePin> {};

    template<typename Pin>
    struct DirectionValue {
        StepperDirection direction;
    };

    template<>
    struct DirectionValue<NoDirectionPin> {};

    template<typename DirPin, typename EnablePin>
    struct MemoryRepresentation<StepperMotorTag<DirPin, EnablePin>> final : public DirectionValue<DirPin>, public EnableValue<EnablePin> {
        ~MemoryRepresentation() = delete;

        uint16_t steps;
        DRV8825SettingsType settings;
    } __attribute__((packed));

}

// Device-specific code and includes
#if !defined(MINIMAL) || MINIMAL == 0

#include <optional>
#include <mutex>

#include "generated/drv8825.pio.h"

#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"

// TODO: add PIO device resource or program
namespace PicoDriver {


    struct DRV8825Program {
        static uint offset;
        static std::once_flag initProgram;
    };

    template<typename DeviceResources, typename StepPin, typename DirPin, typename EnablePin, typename Freq>
    requires (Freq::value > 0)
    class DRV8825 {
        public:

        using Tag = StepperMotorTag<DirPin, EnablePin>;
        // TODO: configure these differently
        static inline constexpr auto TargetFrequency = 4000u;
        static inline constexpr auto PIFrequency = 125'000'000ull;

        bool install(volatile MemoryRepresentation<Tag> *memory) { 
            std::call_once(DRV8825Program::initProgram, []() {
                DRV8825Program::offset = pio_add_program(DeviceResources::Device, &drv8825_program);
            });
            // TODO: Set clkdiv to multiple of Freq
            drv8825_program_init(DeviceResources::Device, DeviceResources::StateMachine, 
                DRV8825Program::offset, StepPin::value, static_cast<uint16_t>(PIFrequency / TargetFrequency));
            initDirection(memory);
            initEnable(memory);
            return true; 
        }

        bool doWork(volatile MemoryRepresentation<Tag> *memory) { 
            stepsToDo += memory->steps;
            memory->steps = 0;
            setEnable(memory);
            setDirection(memory);

            // TODO: this should be an interrupt, or maybe not
            if (stepsToDo == 0 || dma_channel_is_busy(DeviceResources::channel)) {
                return true;
            } 
            auto [stepsTaken, wordsToTransfer] = calculateMasks(stepsToDo);
            stepsToDo -= stepsTaken;

            dma_channel_config c = dma_channel_get_default_config(DeviceResources::channel);
            channel_config_set_read_increment(&c, false);
            channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
            channel_config_set_dreq(&c, pio_get_dreq(DeviceResources::Device, DeviceResources::StateMachine, true));
            dma_channel_configure(DeviceResources::channel, &c,
                &(DeviceResources::Device)->txf[DeviceResources::StateMachine],
                masks.data(),
                wordsToTransfer,
                true);

            // for (int i = 0; i < wordsToTransfer; ++i) {
            //     for (int j = 0; j < 31; ++j) {
            //         if ((masks[i] >> j) & 0b1) {
            //             printf("-");
            //         } else {
            //             printf("_");
            //         }
            //     }
            //     printf("\n");
            // }
            return true; 
        }

        private:

        // Fill the bitmasks with the step patterns and return how many steps were done
        // TODO: Seperate into own function e.g. function generator
        std::pair<uint16_t, uint8_t> calculateMasks(uint16_t maxSteps) {
            // Right now static frequency, but can be changed later
            static constexpr auto frequency = Freq::value;
            // TODO: make configureable and add as value later on
            static constexpr auto frequencyPIO = 400;
            static constexpr auto oneEveryXCycle = frequencyPIO / frequency;
            unsigned int cycleCount = 0;
            unsigned int stepsCount = 0;
            uint8_t arrayIndex = 0;
            bool bitToSet = 0;

            // When oneEveryXCycle is the same, we could do bit += oneEveryXCycle
            constexpr size_t maxSize = MaxBitmasks * sizeof(typename decltype(masks)::value_type) * 8;
            std::fill(std::begin(masks), std::end(masks), 0);
            for (unsigned int bit = 0; stepsCount < maxSteps && bit < maxSize; ++bit) {
                // Numbers from 0..31, remainder
                unsigned int bitIndex = bit & 0b11111;
                // Divide by 32
                arrayIndex = bit >> 5;
                masks[arrayIndex] |= bitToSet << bitIndex;

                if (cycleCount == oneEveryXCycle) {
                    cycleCount = 0;
                    bitToSet = (!bitToSet) & 0b1;
                    stepsCount = stepsCount + (1 - bitToSet);
                } else {
                    ++cycleCount;
                }
            }
            return std::make_pair(stepsCount, arrayIndex + 1);
        }

        template<typename T, typename = decltype((void) T::direction)>
        bool initDirection(volatile T *memory) {
            gpio_init(DirPin::value);
            gpio_set_dir(DirPin::value, GPIO_OUT);
            return true;
        }

        bool initDirection(...) { return true; }


        template<typename T, typename = decltype((void) T::direction)>
        bool setDirection(volatile T *memory) {
            // TODO: Abort dma_channel transmition and start again ?
            // For now wait for end of the transmission
            if (!stepsTransmitted()) {
                return true;
            }

            gpio_put(DirPin::value, memory->direction);

            // TODO: set direction
            return true;
        }

        bool setDirection(...) { return true; }

        template<typename T, typename = decltype((void) T::enable)>
        bool initEnable(volatile T *memory) {
            gpio_init(EnablePin::value);
            gpio_set_dir(EnablePin::value, GPIO_OUT);
            // Disable driver
            gpio_put(EnablePin::value, true);
            return true;
        }

        bool initEnable(...) { return true; }

        template<typename T, typename = decltype((void) T::enable)>
        bool setEnable(volatile T *memory) {
            bool newValue = false;
            if (!stepsTransmitted()) {
                newValue = true;
            } else if (memory->settings & DRV8825Settings::DisableAfterSteps) {
                newValue = false;
            }

            // Enable if true, low level is enabled
            gpio_put(EnablePin::value, !newValue);

            // TODO: Set Pin to value
            return true;
        }

        bool setEnable(...) { return true; }

        bool stepsTransmitted() { return stepsToDo == 0 && !dma_channel_is_busy(DeviceResources::channel); }

        static inline constexpr uint8_t MaxBitmasks = 32;
        uint16_t stepsToDo = 0;
        std::array<uint32_t, MaxBitmasks> masks{};
    };

}
#endif