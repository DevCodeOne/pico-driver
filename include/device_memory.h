#pragma once

#include <array>
#include <numeric>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "type_utils.h"

namespace PicoDriver {

    template<typename Device>
    struct MemoryRepresentation;

    template<typename ... Devices>
    requires (sizeof...(Devices) < 255)
    struct Memory {

        static constexpr std::array<uint16_t, sizeof...(Devices)> Sizes{ sizeof(MemoryRepresentation<typename Devices::Tag>) ... };

        volatile uint8_t byteRepresentation[std::accumulate(Sizes.cbegin(), Sizes.cend(), size_t{0})];

        static uint16_t constexpr offset(uint16_t index) {
            return std::accumulate(Sizes.cbegin(), Sizes.cbegin() + index, static_cast<uint16_t>(0));
        }

        void writeRaw(uint8_t offset, uint8_t value) {
            byteRepresentation[offset] = value;
        }

        uint8_t readRaw(uint8_t offset) {
            return byteRepresentation[offset];
        }

        template<size_t Index>
        constexpr auto getEntry() {
            using CurrentType = MemoryRepresentation<typename TypeUtils::TypeAt_t<Index, Devices ...>::Tag>;
            return reinterpret_cast<std::add_pointer_t<std::add_volatile_t<CurrentType>>>(byteRepresentation + offset(Index));
        }


        template<size_t Index>
        constexpr auto getEntry() const {
            using CurrentType = MemoryRepresentation<typename TypeUtils::TypeAt_t<Index, Devices ...>::Tag>;
            return reinterpret_cast<std::add_pointer_t<std::add_volatile_t<CurrentType>>>(byteRepresentation + offset(Index));
        }

    } __attribute__((packed));

}