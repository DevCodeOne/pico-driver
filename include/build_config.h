#pragma once

#include <cstddef>

#ifndef GENERATE_HEADER
    #define GENERATE_HEADER 1
#endif

#include "gen_id.h"
#include "devices/pwm.h"
#include "devices/adc.h"
#include "devices/drv8825.h"
#include "devices/hx711.h"

#include "type_utils.h"

namespace PicoDriver {
    //TODO: add all devices
    // Ids start at 1, since 0 is reserved for device-info
    template<typename Pin, typename Freq>
    struct GenId<PWM<Pin, Freq>> {
        static inline constexpr uint8_t value = 0x1;
    };

    template<typename Pin>
    struct GenId<ADC<Pin>> {
        static inline constexpr uint8_t value = 0x2;
    };

    template<typename StepPin, typename DirPin, typename Freq>
    struct GenId<DRV8825<StepPin, DirPin, Freq>> {
        static inline constexpr uint8_t value = 0x3;
    };
}