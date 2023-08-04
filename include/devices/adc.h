#pragma once

#include "device_memory.h"

namespace PicoDriver {
    struct NoInputPin {};

    enum struct ADCChannel {
        Zero = 0, One = 1, Two = 2, Three = 3, Four = 4
    };

    // TODO: add check, when pin is used, channel isn't 4
    template<typename Pin>
    struct IsPinAcceptable {
        static inline constexpr bool value = Pin::value >= 26 || Pin::value <= 29;
    };

    template<>
    struct IsPinAcceptable<NoInputPin> : std::true_type { };

    template<typename Pin>
    struct AutoSelectChannel : std::integral_constant<ADCChannel, static_cast<ADCChannel>(Pin::value - 26)> { };

    template<>
    struct AutoSelectChannel<NoInputPin> { };

    template<ADCChannel channel>
    struct Channel : std::integral_constant<ADCChannel, channel> {};

    template<typename Pin, typename Channel = AutoSelectChannel<Pin>>
    requires (IsPinAcceptable<Pin>::value)
    class ADC;

    struct ADCType {
        static inline constexpr uint8_t Id = 0x2;
    };

    template<typename Pin, typename Channel>
    struct MapToType<ADC<Pin, Channel>> {
        using Type = MemoryRepresentation<ADCType>;
        using TagType = ADCType;
    };

    template<>
    struct MemoryRepresentation<ADCType> {
        ~MemoryRepresentation() = delete;

        uint16_t adcValue;

        static inline constexpr auto VREF = 3.3f;
    } __attribute__((packed));

}

// Device-specific code and includes
#if !defined(MINIMAL) || MINIMAL == 0

#include <mutex>

#include "hardware/adc.h"
#include <cstdio>

namespace PicoDriver {

    struct ADCResource {
        static std::once_flag initADC;
    };

    // TODO: maybe add special type for temperature channel
    template<typename Pin, typename Channel>
    requires (IsPinAcceptable<Pin>::value)
    class ADC {
        public:

        bool install(volatile MemoryRepresentation<ADCType> *memory) {
            std::call_once(ADCResource::initADC, []() {
                adc_init();
            });

            initGPIO(memory);
            adc_select_input(static_cast<uint>(Channel::value));

            return true;
        }

        bool doWork(volatile MemoryRepresentation<ADCType> *memory) {
            // constexpr float conversionFactor = 3.3f / (1 << 12);
            uint16_t result = adc_read();
            memory->adcValue = result;
            // float adc = result * conversionFactor;
            // printf("Raw value: 0x%03x, voltage: %f, temp : %f V\n", result, adc,
            //     27.0f - (adc - 0.706f) / 0.001721f);
            return true;
        }

        private:
        template<typename T, typename = decltype((void) T::value)>
        bool initGPIO(volatile T *memory) {
            adc_gpio_init(T::value);
            // TODO: is this correct ?

            return true;
        }

        bool initGPIO(...) { return true; }
    };

}
#endif