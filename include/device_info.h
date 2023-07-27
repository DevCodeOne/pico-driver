#pragma once

#include "device_memory.h"
#include "devices.h"
#include <type_traits>

namespace PicoDriver {

    namespace DeviceTags {
        struct DeviceInfo final : public std::integral_constant<uint8_t, 255> {};
    };

    template<>
    struct MemoryRepresentation<DeviceTags::DeviceInfo> {
        ~MemoryRepresentation() = delete;

        uint16_t value;
    } __attribute__((packed));


    template<typename ... DeviceTypes>
    struct DeviceInfo {
        using ByteRepresentation = std::array<uint8_t, sizeof...(DeviceTypes)>;
        using Tag = DeviceTags::DeviceInfo;

        std::array<DeviceTags::DeviceId, sizeof...(DeviceTypes)> value{ DeviceTypes::Tag::value ...};
    } __attribute__((packed));

    template<typename ... Devices>
    struct MemoryRepresentation<DeviceInfo<Devices ...>> {
        using ByteRepresentation = typename DeviceInfo<Devices ...>::ByteRepresentation;
    };
};