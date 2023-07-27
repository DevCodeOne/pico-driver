#pragma once

#include <type_traits>
#include <cstdint>

#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "device_memory.h"
#include "devices.h"

namespace PicoDriver {

    namespace DeviceTags {
        struct PWM final : public std::integral_constant<DeviceId, DeviceId::PWM> {};
    }

    template<>
    struct MemoryRepresentation<DeviceTags::PWM> {
        ~MemoryRepresentation() = delete;

        uint16_t pwmValue;
    } __attribute__((packed));


    template<typename Pin, typename Freq>
    class PWM {
        public:
        using Tag = DeviceTags::PWM;

        bool install() { 
            gpio_set_function(Pin::value, GPIO_FUNC_PWM);

            uint sliceNum = pwm_gpio_to_slice_num(Pin::value);
            pwm_set_enabled(sliceNum, true);
            return true; 
        }

        bool doWork(volatile MemoryRepresentation<Tag> *memory) { 
            pwm_set_gpio_level(Pin::value, memory->pwmValue);
            return true; 
        }
    };

}