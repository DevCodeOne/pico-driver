#pragma once

#include <type_traits>
#include <cstdint>
#include <concepts>

#include "device_memory.h"
#include "devices.h"
#include "device_helper_types.h"
#include "device_mapping.h"

namespace PicoDriver {
    template<typename Pin>
    class Output;

    struct OutputType {
        static inline constexpr uint8_t Id = 0x7;
        static inline constexpr char Name[] = "OUT";
    };

    template<typename Pin>
    struct MapToType<Output<Pin>> {
        using Type = MemoryRepresentation<OutputType>;
        using TagType = OutputType;
    };

    template<>
    struct MemoryRepresentation<OutputType> {
        ~MemoryRepresentation() = delete;

        uint8_t value;
    } __attribute__((packed));

}

// Device-specific code and includes
#if !defined(MINIMAL) || MINIMAL == 0

#include "hardware/gpio.h"

namespace PicoDriver {
    template<typename Pin>
    class Output {
        public:

        bool install(volatile MemoryRepresentation<OutputType> *memory) { 
            gpio_init(Pin::value);
            gpio_set_dir(Pin::value, GPIO_OUT);

            return true; 
        }

        bool doWork(volatile MemoryRepresentation<OutputType> *memory) { 
            gpio_put(Pin::value, memory->value);
            return true; 
        }

    };

}

#endif