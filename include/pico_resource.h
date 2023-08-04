#pragma once

#include <cstdint>

#if !defined(MINIMAL) || MINIMAL == 0

#include "hardware/pio.h"

namespace PicoDriver {

    // TODO: Type for PIO program
    enum struct PIODevice : uint8_t {
        Zero = 0, One = 1
    };

    enum struct PIOStateMachine : uint8_t {
        Zero = 0, One = 1, Two = 2, Three = 3
    };

    template<PIODevice D, PIOStateMachine SM>
    struct PIOResource {
        static inline pio_hw_t *Device = (D == PIODevice::Zero) ? (pio0) : (pio1);
        static inline auto StateMachine = static_cast<uint>(SM);
    };

    // TODO: Do the rest
    enum struct DMAChannel : uint8_t {
        Zero = 0, One = 1, Two = 2, Three = 3, Four = 4, Five = 5, Six = 6
    };

    template<DMAChannel C>
    struct DMAResource {
        static inline constexpr auto channel = static_cast<uint>(C);
    };

    template<typename ... Resources>
    struct PicoResource : public Resources ... {

    };

}
#endif