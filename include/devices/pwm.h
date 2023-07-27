#pragma once

#include <type_traits>
#include <cstdint>
#include <concepts>

#if __has_include("hardware/pwm.h") || ONLYGENERATE_HEADER == 0
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#else
#ifndef ONLYGENERATE_HEDER
#define ONLYGENERATE_HEADER
#endif
#endif

#include "device_memory.h"
#include "devices.h"

namespace PicoDriver {

    template<auto Value>
    requires (std::is_unsigned_v<decltype(Value)>)
    using Hz = std::integral_constant<decltype(Value), Value>;

    // TODO: add Freq type which is configurable at runtime with different MemoryRepresentation
    template<typename Pin, typename Freq>
    requires (Freq::value > 0)
    class PWM {
        public:
        #if ONLYGENERATE_HEADER == 0
        bool install() { 
            gpio_set_function(Pin::value, GPIO_FUNC_PWM);

            uint sliceNum = pwm_gpio_to_slice_num(Pin::value);
            pwm_set_enabled(sliceNum, true);
            return true; 
        }

        bool doWork(volatile MemoryRepresentation<PWM> *memory) { 
            pwm_set_gpio_level(Pin::value, memory->pwmValue);
            return true; 
        }
        #else
        bool install() { return true; }
        bool doWork(volatile MemoryRepresentation<PWM> *memory) { return true; }
        #endif

    };

    template<typename Pin, typename Freq>
    struct MemoryRepresentation<PWM<Pin, Freq>> {
        ~MemoryRepresentation() = delete;

        uint16_t pwmValue;
    } __attribute__((packed));




}