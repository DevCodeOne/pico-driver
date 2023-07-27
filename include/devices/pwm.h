#pragma once

#include <type_traits>
#include <cstdint>
#include <concepts>

#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "device_memory.h"
#include "devices.h"

namespace PicoDriver {

    template<auto Value>
    requires (std::is_unsigned_v<decltype(Value)>)
    using Hz = std::integral_constant<decltype(Value), Value>;

    namespace DeviceTags {
        struct PWM final : public std::integral_constant<DeviceId, DeviceId::PWM> {};
    }

    template<>
    struct MemoryRepresentation<DeviceTags::PWM> {
        ~MemoryRepresentation() = delete;

        uint16_t pwmValue;
    } __attribute__((packed));


    // TODO: add Freq type which is configurable at runtime with different MemoryRepresentation
    template<typename Pin, typename Freq>
    requires (Freq::value > 0)
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