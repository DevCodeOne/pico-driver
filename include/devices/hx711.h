#pragma once

#include "devices.h"
#include "device_memory.h"

namespace PicoDriver {
    namespace DeviceTags {
        struct HX711 final : public std::integral_constant<DeviceId, DeviceId::PWM> {};
    }

    template<>
    struct MemoryRepresentation<DeviceTags::HX711> {
        ~MemoryRepresentation() = delete;

        uint16_t value;
    } __attribute__((packed));


}