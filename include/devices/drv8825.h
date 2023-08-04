#pragma once

#include "device_memory.h"
#include "device_mapping.h"
#include "devices.h"
#include "pico_resource.h"

namespace PicoDriver {
    struct NoDirectionPin {};
    struct NoEnablePin {};

    template<typename DeviceResources, typename StepPin, typename DirPin, typename EnablePin, typename Freq>
    requires (Freq::value > 0)
    class DRV8825;

    template<typename DirPin, typename EnablePin>
    struct StepperMotorTag {
        static inline constexpr uint8_t Id = 0x3 + (std::is_same_v<DirPin, NoDirectionPin> ? 1 : 0) + (std::is_same_v<EnablePin, NoEnablePin> ? 2 : 0);
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
    } __attribute__((packed));

}

// Device-specific code and includes
#if !defined(MINIMAL) || MINIMAL == 0

#include <charconv>

#include "generated/drv8825.pio.h"

#include "hardware/dma.h"

// TODO: add PIO device resource or program
namespace PicoDriver {
    template<typename DeviceResources, typename StepPin, typename DirPin, typename EnablePin, typename Freq>
    requires (Freq::value > 0)
    class DRV8825 {
        public:

        using Tag = StepperMotorTag<DirPin, EnablePin>;

        bool install(volatile MemoryRepresentation<Tag> *memory) { 
            const auto offset = pio_add_program(DeviceResources::Device, &drv8825_program);
            drv8825_program_init(DeviceResources::Device, DeviceResources::StateMachine, 
                offset, PICO_DEFAULT_LED_PIN, static_cast<uint16_t>(125'000'000ull / 2000));
                        return true; 
        }

        bool doWork(volatile MemoryRepresentation<Tag> *memory) { 
            // TODO: Test, remove later
            memory->steps = 2;
            stepsToDo += memory->steps;
            memory->steps = 0;

            // TODO: this should be an interrupt, or maybe not
            if (stepsToDo > 0 && !dma_channel_is_busy(DeviceResources::channel)) {
                // TODO: If Direction was changed abort dma and do update
                setEnable(memory);
                setDirection(memory);
            } else {
                // There's still data to be send, go on
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
            static constexpr auto frequencyPIO = 200;
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
        bool setDirection(volatile T *memory) {
            memory->direction = StepperDirection::Left;
            return true;
        }
        bool setDirection(...) { return true; }


        template<typename T, typename = decltype((void) T::enable)>
        bool setEnable(volatile T *memory) {
            memory->enable = 0;
            return true;
        }
        bool setEnable(...) { return true; }

        static inline constexpr uint8_t MaxBitmasks = 32;
        uint16_t stepsToDo = 0;
        std::array<uint32_t, MaxBitmasks> masks{};
    };

}
#endif