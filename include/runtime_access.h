#pragma once

#include <algorithm>
#include <ranges>
#include <sys/wait.h>
#include <variant>
#include <array>
#include <cstddef>

#include "device_mapping.h"
#include "device_memory.h"
#include "device_info.h"
#include "devices.h"

// This class is not allowed to use chip specific class or headers, it should be consumed by other devices, 
// which want to communicate with this i2c_slave

namespace PicoDriver {

namespace RuntimeAccess {

    static inline constexpr size_t MaxDevices = 12;

    template<typename DeviceTagList>
    class RuntimeAccess;

    // TODO: create runtime device info class, which can read the num of devices from the memory over i2c
    template<typename ... DeviceTags>
    class RuntimeAccess<DeviceList<DeviceTags ...>> {
        public:
            // using DeviceInfoMemoryRepresentationType = MappedType<DeviceInfo<DeviceList<DeviceTags ...>>>;
            static inline constexpr std::array<size_t, sizeof...(DeviceTags) + 1> MemorySizes{sizeof(MemoryRepresentation<DeviceTags>) .../*, sizeof(DeviceInfoMemoryRepresentationType)*/};
            // TODO: ignore device info size and use next smaller size, since there's only one device info, but the next smaller device could be used <MaxDevices> times
            static inline constexpr size_t MaxPossibleMemoryLayoutSize = MaxDevices * *std::max_element(MemorySizes.cbegin(), MemorySizes.cend());
            // TODO: pointer have to point to the correct location in memory
            using DeviceMemoryType = std::variant<std::monostate, std::add_pointer_t<MemoryRepresentation<DeviceTags>> .../*, std::add_pointer_t<DeviceInfoMemoryRepresentationType>*/>;
            static std::optional<RuntimeAccess> createRuntimeAccessFromInfo(std::ranges::range auto &deviceInfo) {
                return std::optional<RuntimeAccess>(RuntimeAccess(deviceInfo));
            }

            auto &operator[](size_t index)  { return devices[index]; }
            const auto &operator[](size_t index) const { return devices[index]; }

            // iterate the memoryrepresentations of the devices
            auto begin() { return devices.begin(); }
            auto end() { return devices.end(); }

            auto cbegin() const { return devices.cbegin(); }
            auto cend() const { return devices.end(); }

            // Use this function to send the updated values to the pico
            auto rawData() const { return deviceMemory.data(); }
        private:

            template<size_t I, typename ... D>
            struct GenerateMemoryRepresentation{
                static DeviceMemoryType generate(size_t index, uint8_t *base, ptrdiff_t &offset) {
                    using CurrentMemoryType = MemoryRepresentation<std::tuple_element_t<I, std::tuple<D ...>>>;
                    if (index == I) {
                        offset += MemorySizes[I];
                        return reinterpret_cast<CurrentMemoryType *>(base + offset);
                    }

                    if constexpr (I > 0) {
                        return GenerateMemoryRepresentation<I - 1, D ...>::generate(index, base, offset);
                    } else {
                        return std::monostate{};
                    }
                }
            };

            // Range should be of type uint8_t
            RuntimeAccess(std::ranges::range auto &deviceInfo) {
                ptrdiff_t currentOffset = 0;
                size_t currentDeviceIndex = 0;

                for (auto &currentDeviceId : deviceInfo) {
                    devices[currentDeviceIndex++] = GenerateMemoryRepresentation<sizeof...(DeviceTags) - 1, DeviceTags ...>
                        ::generate(currentDeviceId, deviceMemory.data(), currentOffset);
                }
            }

            std::array<DeviceMemoryType, MaxDevices> devices;
            std::array<uint8_t, MaxPossibleMemoryLayoutSize> deviceMemory;
    };
}
}