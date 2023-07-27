#pragma once

#include "device_memory.h"
#include "devices.h"
#include <type_traits>

namespace PicoDriver {

    template<typename DeviceTypeList>
    struct DeviceInfo;

    template<typename ... Devices>
    struct DeviceInfo<DeviceList<Devices ...>> {
        bool install() { return true; };
        bool doWork(volatile MemoryRepresentation<DeviceInfo> *memory) { return true; }
    };

    template<typename ... Devices>
    struct MemoryRepresentation<DeviceInfo<DeviceList<Devices ...>>> {
        using ByteRepresentation = std::array<uint8_t, sizeof...(Devices)>;

        ~MemoryRepresentation() = delete;

        ByteRepresentation data;
    } __attribute__((packed));


};