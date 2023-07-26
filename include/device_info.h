#pragma once

#include "device_memory.h"
#include "devices.h"

namespace PicoDriver {

    template<typename ... DeviceTypes>
    struct DeviceInfo {
        using ByteRepresentation = std::array<uint8_t, sizeof...(DeviceTypes)>;

        std::array<Devices::DeviceId, sizeof...(DeviceTypes)> value{ DeviceTypes::value ...};
    } __attribute__((packed));

    template<typename ... Devices>
    struct MemoryRepresentation<DeviceInfo<Devices ...>> {
        using ByteRepresentation = typename DeviceInfo<Devices ...>::ByteRepresentation;
    };
};