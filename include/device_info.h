#pragma once

#include <type_traits>

#include "devices.h"
#include "gen_id.h"

namespace PicoDriver {

    template<typename DeviceTypeList>
    struct DeviceInfo;

    template<typename DeviceTypeList>
    struct GenId<DeviceInfo<DeviceTypeList>> {
        static inline constexpr auto value = 0x0;
    };

    template<typename ... Devices>
    struct DeviceInfo<DeviceList<Devices ...>> {

        bool install(volatile MemoryRepresentation<DeviceInfo> *memory) { 
            std::copy(std::end(MemoryRepresentation<DeviceInfo>::deviceIds), std::begin(MemoryRepresentation<DeviceInfo>::deviceIds), 
                std::begin(memory->data));
            return true; 
        };

        bool doWork(volatile MemoryRepresentation<DeviceInfo> *memory) { return true; }
    };

    template<typename T>
    constexpr bool IsUniqueSet(const T &iterateable) {
        return std::accumulate(iterateable.cbegin(), iterateable.cend(), size_t{0}, [&iterateable](const auto &sum, const auto &currentElement) {
            return sum + std::count(iterateable.cbegin(), iterateable.cend(), currentElement);
        }) == iterateable.size();
    }

    template<typename ... Devices>
    requires(IsUniqueSet(std::array<char, sizeof...(Devices) + 1>{ GenId<DeviceInfo<DeviceList<Devices ...>>>::value, GenId<Devices>::value ... }))
    struct MemoryRepresentation<DeviceInfo<DeviceList<Devices ...>>> {
        using ByteRepresentation = std::array<uint8_t, sizeof...(Devices) + 1>;

        ~MemoryRepresentation() = delete;

        char data[sizeof...(Devices) + 1];
        static inline constexpr ByteRepresentation deviceIds{ GenId<DeviceInfo<DeviceList<Devices ...>>>::value, GenId<Devices>::value ... };
    } __attribute__((packed));


};