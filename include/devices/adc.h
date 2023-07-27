#pragma once

#include "device_memory.h"

namespace PicoDriver {
    namespace DeviceTags {
        struct ADC final : public std::integral_constant<DeviceId, DeviceId::PWM> {};
    }

    template<>
    struct MemoryRepresentation<DeviceTags::ADC> {
        ~MemoryRepresentation() = delete;

        uint16_t adcValue;
    } __attribute__((packed));

}