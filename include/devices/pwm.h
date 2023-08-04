#pragma once

#include <sys/wait.h>
#include <type_traits>
#include <cstdint>
#include <concepts>

#include "device_memory.h"
#include "devices.h"
#include "device_helper_types.h"
#include "device_mapping.h"

namespace PicoDriver {
    template<typename Pin, typename Freq>
    requires (Freq::value > 0)
    class PWM;

    struct FixedPWMType {
        static inline constexpr uint8_t Id = 0x1;
        static inline constexpr char Name[] = "FPWM";
    };

    template<typename Pin, typename Freq>
    struct MapToType<PWM<Pin, Freq>> {
        using Type = MemoryRepresentation<FixedPWMType>;
        using TagType = FixedPWMType;
    };

    template<>
    struct MemoryRepresentation<FixedPWMType> {
        ~MemoryRepresentation() = delete;

        uint16_t pwmValue;
    } __attribute__((packed));

}

// Device-specific code and includes
#if !defined(MINIMAL) || MINIMAL == 0
#include "pico/stdlib.h"
#include "hardware/pwm.h"

namespace PicoDriver {
    // TODO: add Freq type which is configurable at runtime with different MemoryRepresentation
    // TODO: set Freq for pwm
    template<typename Pin, typename Freq>
    requires (Freq::value > 0)
    class PWM {
        public:

        bool install(volatile MemoryRepresentation<FixedPWMType> *memory) { 
            gpio_set_function(Pin::value, GPIO_FUNC_PWM);

            uint sliceNum = pwm_gpio_to_slice_num(Pin::value);
            pwm_set_enabled(sliceNum, true);
            return true; 
        }

        bool doWork(volatile MemoryRepresentation<FixedPWMType> *memory) { 
            pwm_set_gpio_level(Pin::value, memory->pwmValue);
            return true; 
        }

    };

}

#endif