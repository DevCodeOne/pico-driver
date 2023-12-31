#pragma once

#include <array>
#include <numeric>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "type_utils.h"
#include "device_mapping.h"

namespace PicoDriver {

    template<typename Device>
    struct MemoryRepresentation;

    namespace Detail {
        template<typename MemoryType>
        struct ExtractMemoryTag;

        template<typename TagType>
        struct ExtractMemoryTag<MemoryRepresentation<TagType>> {
            using Type = TagType;
        };
    }

    template<typename MemoryType>
    using ExtractMemoryTag = Detail::ExtractMemoryTag<MemoryType>::Type;

    template<typename ArrType>
    constexpr size_t accumulateElements(const ArrType &arr) {
        return std::accumulate(arr.cbegin(), arr.cend(), size_t{0});
    }

    template<typename ... Devices>
    concept ByteAddresseable = (accumulateElements(std::array<uint8_t, sizeof...(Devices)> { sizeof(MappedType<Devices>) ... }) < 255);

    // TODO: Maybe add possibility to using multiple addresses or using 16-bit addresses
    template<typename ... Devices>
    requires (ByteAddresseable<Devices ...>)
    struct Memory {

        static constexpr std::array<uint16_t, sizeof...(Devices)> Sizes{ sizeof(MappedType<Devices>) ... };

        volatile uint8_t byteRepresentation[accumulateElements(Sizes)];

        static uint8_t constexpr numDevices() {
            return Sizes.size();
        }

        static uint8_t constexpr offset(uint8_t index) {
            return std::accumulate(Sizes.cbegin(), Sizes.cbegin() + index, static_cast<uint16_t>(0));
        }

        uint8_t memorySize() const { return sizeof(byteRepresentation); }

        void writeRaw(uint8_t offset, uint8_t value) {
            if (offset < sizeof(byteRepresentation)) {
                byteRepresentation[offset] = value;
            }
        }

        uint8_t readRaw(uint8_t offset) {
            if (offset > sizeof(byteRepresentation)) {
                return 0x00;
            }

            return byteRepresentation[offset];
        }

        template<size_t Index>
        constexpr auto getEntry() {
            using CurrentType = typename std::tuple_element_t<Index, std::tuple<MappedType<Devices> ...>>;
            return reinterpret_cast<std::add_pointer_t<std::add_volatile_t<CurrentType>>>(byteRepresentation + offset(Index));
        }


        template<size_t Index>
        constexpr auto getEntry() const {
            using CurrentType = typename std::tuple_element_t<Index, std::tuple<MappedType<Devices> ...>>;
            return reinterpret_cast<std::add_pointer_t<std::add_volatile_t<CurrentType>>>(byteRepresentation + offset(Index));
        }

    } __attribute__((packed));

}