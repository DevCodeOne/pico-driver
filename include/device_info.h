#pragma once

#include <type_traits>
#include <cstdint>

#include "devices.h"
#include "device_mapping.h"

namespace PicoDriver {

    template<typename DeviceTypeList>
    struct DeviceInfo;

    struct DeviceInfoType {
        static inline constexpr uint8_t Id = 0x0;
    };

    template<typename ... Devices>
    struct MapToType<DeviceInfo<DeviceList<Devices ...>>> {
        using Type = MemoryRepresentation<DeviceInfo<DeviceList<Devices ...>>>;
        using TagType = DeviceInfoType;
    };

    template<typename T>
    constexpr bool IsUniqueSet(const T &iterateable) {
        return std::accumulate(iterateable.cbegin(), iterateable.cend(), size_t{0}, [&iterateable](const auto &sum, const auto &currentElement) {
            return sum + std::count(iterateable.cbegin(), iterateable.cend(), currentElement);
        }) == iterateable.size();
    }

    template<typename ... Devices>
    struct DeviceInfo<DeviceList<Devices ...>> {
        bool install(volatile MemoryRepresentation<DeviceInfo> *memory) { 
            std::copy(std::begin(MemoryRepresentation<DeviceInfo>::deviceIds), std::end(MemoryRepresentation<DeviceInfo>::deviceIds), 
                std::begin(memory->data));
            return true; 
        };

        bool doWork(volatile MemoryRepresentation<DeviceInfo> *memory) { return true; }

        void swap(const DeviceInfo &other) { }
    };

    template<typename DL>
    struct MemoryRepresentation<DeviceInfo<DL>> {
        private:
        template<typename Devices>
        struct ExtractDevices;

        template<typename ... Devices>
        // TODO: fix this, Ids can be the same, the types have to be different
        // requires (IsUniqueSet(std::array<uint8_t, sizeof...(Devices) + 1>{ IdValue<DeviceInfo<DeviceList<Devices ...>>>, IdValue<Devices> ... }))
        struct ExtractDevices<DeviceList<Devices ...>> {
            using ByteRepresentation = std::array<uint8_t, sizeof...(Devices) + 1>;

            static inline constexpr uint8_t NumDevices = static_cast<uint8_t>(sizeof...(Devices));
            static inline constexpr ByteRepresentation deviceIds{ NumDevices , /*IdValue<DeviceInfo<DeviceList<Devices ...>>>, */IdValue<Devices> ... };
        };
        using ExtractDevicesType = ExtractDevices<DL>;
        using ByteRepresentation = ExtractDevicesType::ByteRepresentation;

        public:

        ~MemoryRepresentation() = delete;

        uint8_t data[ExtractDevicesType::NumDevices + 1];
        static inline constexpr ByteRepresentation deviceIds = ExtractDevicesType::deviceIds;
    } __attribute__((packed));


};