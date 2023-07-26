#pragma once

#include <array>
#include <numeric>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "devices.h"

namespace PicoDriver {

    template<size_t Index, typename Head, typename ... Rest>
    struct TypeAt {
        using type = typename TypeAt<Index - 1, Rest ...>::type;
    };

    template<typename Head, typename ... Rest>
    struct TypeAt<0, Head, Rest ...> {
        using type = Head;
    };

    template<size_t Index, typename ... TypeList>
    using TypeAt_t = typename TypeAt<Index, TypeList ...>::type;

    // TODO: check max size < 255 (size of uint, or change behaviour)
    template<typename ... Devices>
    struct Memory {

        static constexpr std::array<uint16_t, sizeof...(Devices)> Sizes{ sizeof(MemoryRepresentation<Devices>) ... };

        std::array<uint8_t, std::accumulate(Sizes.cbegin(), Sizes.cend(), size_t{0})> byteRepresentation;

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
            using CurrentType = MemoryRepresentation<TypeAt_t<Index, Devices ...>>;
            return reinterpret_cast<std::add_pointer_t<CurrentType>>(byteRepresentation.data() + offset(Index));
        }


        template<size_t Index>
        constexpr auto getEntry() const {
            using CurrentType = MemoryRepresentation<TypeAt_t<Index, Devices ...>>;
            return reinterpret_cast<std::add_pointer_t<CurrentType>>(byteRepresentation.data() + offset(Index));
        }

    } __attribute__((packed));

}