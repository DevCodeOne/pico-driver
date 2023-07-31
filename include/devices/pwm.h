#pragma once

#include <type_traits>
#include <cstdint>
#include <concepts>

#include "device_memory.h"
#include "devices.h"
#include "device_helper_types.h"

namespace PicoDriver {
    template<typename Pin, typename Freq>
    requires (Freq::value > 0)
    class PWM;

    template<typename Pin, typename Freq>
    struct MemoryRepresentation<PWM<Pin, Freq>> {
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
    template<typename Pin, typename Freq>
    requires (Freq::value > 0)
    class PWM {
        public:
        

        bool install(volatile MemoryRepresentation<PWM> *memory) { 
            gpio_set_function(Pin::value, GPIO_FUNC_PWM);

            uint sliceNum = pwm_gpio_to_slice_num(Pin::value);
            pwm_set_enabled(sliceNum, true);
            return true; 
        }

        bool doWork(volatile MemoryRepresentation<PWM> *memory) { 
            pwm_set_gpio_level(Pin::value, memory->pwmValue);
            return true; 
        }

    };

}

#endif