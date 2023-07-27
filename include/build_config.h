#pragma once

#include <tuple>

#include "devices/pwm.h"
#include "devices/adc.h"
#include "devices/drv8825.h"
#include "devices/hx711.h"

#include "type_utils.h"

namespace PicoDriver {

    template<typename ... Devices>
    requires (TypeUtils::IsTypeSet<Devices ...>)
    struct DeviceIdMapping {

    };

}