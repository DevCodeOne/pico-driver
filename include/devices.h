#pragma once

#include <cstdint>
#include <type_traits>

#include "device_memory.h"

namespace PicoDriver {
    namespace DeviceTags {
        // TODO: fix this
        enum struct DeviceId : uint8_t { PWM, ADC, DRV8825, HX711 };

    };

}