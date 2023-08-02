#pragma once

#include <algorithm>
#include <numeric>
#include <ranges>
#include <sys/wait.h>
#include <variant>
#include <array>
#include <cstddef>

#include "device_mapping.h"
#include "device_memory.h"
#include "device_info.h"
#include "devices.h"

namespace PicoDriver {

namespace RuntimeAccess {

    // TODO: Define via template parameter later
    static inline constexpr size_t MaxDevices = 12;

    struct MemorySliceUpdate {
        const uint8_t address;
        const uint8_t * const data;
        const uint8_t size;
    };

    // This class is not allowed to use chip specific class or headers, it should be consumed by other devices, 
    // which want to communicate with this i2c_slave
    template<typename DeviceTagList>
    class RuntimeAccess;
    
    template<typename DeviceTagList>
    class RuntimeDeviceInfo {
        public:
            static std::optional<RuntimeDeviceInfo> create(const std::ranges::range auto &memoryBytes) {
                uint8_t numDevices = *std::ranges::begin(memoryBytes);
                return RuntimeDeviceInfo{numDevices};
            }

            uint8_t sizeInMemory() const { return mNumDevices + 1; }
            uint8_t numDevices() const { return mNumDevices; }

            void swap(RuntimeDeviceInfo &other) {
                using std::swap;

                swap(mNumDevices, other.mNumDevices);
            }

        private:
            RuntimeDeviceInfo(uint8_t numDevices) noexcept : mNumDevices(numDevices) {}

            uint8_t mNumDevices = 0;
    };

    template<typename TagList>
    void swap(RuntimeDeviceInfo<TagList> &lhs, DeviceInfo<TagList> &rhs) {
        lhs.swap(rhs);
    }

    // TODO: create runtime device info class, which can read the num of devices from the memory over i2c
    template<typename ... DeviceTags>
    class RuntimeAccess<DeviceList<DeviceTags ...>> {
        public:
            using RuntimeDeviceInfoType = RuntimeDeviceInfo<DeviceList<DeviceTags ...>>;

            static inline constexpr std::array<size_t, sizeof...(DeviceTags)> MemorySizes{sizeof(MemoryRepresentation<DeviceTags>) ...};
            // TODO: (MaxDevices + 1) is the max deviceinfo size possible, if there are more devices throw error
            static inline constexpr size_t MaxPossibleMemoryLayoutSize = (MaxDevices + 1) + MaxDevices * *std::max_element(MemorySizes.cbegin(), MemorySizes.cend());
            // TODO: pointer have to point to the correct location in memory
            using DeviceMemoryType = std::variant<std::monostate, std::add_pointer_t<MemoryRepresentation<DeviceTags>> ...>;

            static std::optional<RuntimeAccess> createRuntimeAccessFromInfo(const std::ranges::range auto &deviceMemory) {
                auto deviceInfo = RuntimeDeviceInfoType::create(deviceMemory);

                if (!deviceInfo) {
                    return std::nullopt;
                }

                return RuntimeAccess(*deviceInfo, deviceMemory);
            }

            RuntimeAccess(const RuntimeAccess &other) : deviceMemory(other.deviceMemory), deviceInfo(other.deviceInfo) {
                rebaseMemoryRepresentations();
            }

            RuntimeAccess(RuntimeAccess &&other) : deviceMemory(other.deviceMemory), deviceInfo(other.deviceInfo) {
                rebaseMemoryRepresentations();
            }

            RuntimeAccess &operator=(const RuntimeAccess &other) {
                deviceInfo = other.deviceInfo;
                deviceMemory = other.deviceMemory;

                rebaseMemoryRepresentations();
                return *this;
            }

            RuntimeAccess &operator=(RuntimeAccess &&other) {
                deviceInfo = std::move(other.deviceInfo);
                deviceMemory = other.deviceMemory;

                rebaseMemoryRepresentations();
                return *this;
            }

            auto &operator[](size_t index) { return devices[index]; }
            const auto &operator[](size_t index) const { return devices[index]; }

            // iterate the memoryrepresentations of the devices
            auto begin() { return devices.begin(); }
            auto end() { return devices.end(); }

            auto cbegin() const { return devices.cbegin(); }
            auto cend() const { return devices.end(); }

            void swap(RuntimeAccess &other) {
                using std::swap;

                swap(devices, other.devices);
                swap(deviceMemory, other.deviceMemory);
                swap(deviceInfo, other.deviceInfo);

                rebaseMemoryRepresentations();
            }

            // Access to whole device memory
            auto &rawData() { return deviceMemory; }
            const auto &rawData() const { return deviceMemory; }

            // Access to specific device memory representation
            template<typename DeviceTag>
            auto toRawMemorySlice(MemoryRepresentation<DeviceTag> *ptr) const {
                return MemorySliceUpdate{
                                        .address = static_cast<uint8_t>(reinterpret_cast<uint8_t *>(ptr) - rawData().data()),
                                        .data = reinterpret_cast<uint8_t *>(ptr),
                                        .size = sizeof(MemoryRepresentation<DeviceTag>)
                                        };
            }
        private:

            void rebaseMemoryRepresentations() {
                // Skip first byte plus numDevices to get to the first memory entry
                ptrdiff_t currentOffset = deviceInfo.sizeInMemory();

                for (uint8_t currentDeviceIndex = 0; currentDeviceIndex < deviceInfo.numDevices(); ++currentDeviceIndex) {
                    // + 1 to skip number of devices in structure
                    auto currentDeviceId = deviceMemory[currentDeviceIndex + 1];
                    devices[currentDeviceIndex] = GenerateMemoryRepresentation<sizeof...(DeviceTags) - 1, DeviceTags ...>
                        ::generate(currentDeviceId, deviceMemory.data(), currentOffset);
                }
            }

            template<typename DeviceTag>
            auto *recalculateNewPosition(MemoryRepresentation<DeviceTag> *ptr) {
                return reinterpret_cast<MemoryRepresentation<DeviceTag> *>(static_cast<ptrdiff_t>(ptr) + rawData());
            }

            template<typename DeviceTag>
            auto *recalculateNewPosition(const MemoryRepresentation<DeviceTag> *ptr) const {
                return reinterpret_cast<MemoryRepresentation<DeviceTag> *>(static_cast<ptrdiff_t>(ptr) + rawData());
            }

            template<uint8_t I, typename ... D>
            struct GenerateMemoryRepresentation{
                static DeviceMemoryType generate(uint8_t index, uint8_t *base, ptrdiff_t &offset) {
                    using CurrentMemoryType = MemoryRepresentation<std::tuple_element_t<I, std::tuple<D ...>>>;
                    // DeviceId 0 is reserved for deviceinfo, so id 1 is first (0) tuple element
                    if (index - 1 == I) {
                        auto result = reinterpret_cast<CurrentMemoryType *>(base + offset);
                        offset += MemorySizes[I];
                        return result;
                    }

                    if constexpr (I > 0) {
                        return GenerateMemoryRepresentation<I - 1, D ...>::generate(index, base, offset);
                    } else {
                        return std::monostate{};
                    }
                }
            };

            // Range should be of type uint8_t
            RuntimeAccess(RuntimeDeviceInfoType inst, const std::ranges::range auto &initialMemory) : deviceInfo(inst) {
                // Skip first byte plus numDevices to get to the first memory entry
                std::copy_n(std::cbegin(initialMemory), std::min(initialMemory.size(), deviceMemory.size()), std::begin(deviceMemory));
                rebaseMemoryRepresentations();
            }

            std::array<DeviceMemoryType, MaxDevices> devices;
            std::array<uint8_t, MaxPossibleMemoryLayoutSize> deviceMemory;
            RuntimeDeviceInfoType deviceInfo{};
    };


    template<typename DeviceTagList>
    void swap(RuntimeAccess<DeviceTagList> &lhs, RuntimeAccess<DeviceTagList> &rhs) {
        lhs.swap(rhs);
    }
    }
}