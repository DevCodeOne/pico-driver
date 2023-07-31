#pragma once

#include <cstddef>
#include <type_traits>

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
    //TODO: add dummy type which can always be used for the parameters, 
    // so one doesn't have to specify the exact type for the memory representation, when the parameter doesn't matter

    // Ids start at 1, since 0 is reserved for device-info
    template<>
    struct GenId<FixedPWMType> {
        static inline constexpr uint8_t value = 0x1;
    };

    template<>
    struct GenId<ADCType> {
        static inline constexpr uint8_t value = 0x2;
    };

}