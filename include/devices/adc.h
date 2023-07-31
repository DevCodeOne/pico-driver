#pragma once

#include "device_memory.h"

namespace PicoDriver {
    template<typename Pin>
    class ADC;

    struct ADCType {};

    template<typename Pin>
    struct MapToType<ADC<Pin>> {
        using Type = MemoryRepresentation<ADCType>;
        using TagType = ADCType;
    };

    template<>
    struct MemoryRepresentation<ADCType> {
        ~MemoryRepresentation() = delete;

        uint16_t adcValue;
    } __attribute__((packed));

}

// Device-specific code and includes
#if !defined(MINIMAL) || MINIMAL == 0

namespace PicoDriver {
    template<typename Pin>
    class ADC {
        public:

        bool install(volatile MemoryRepresentation<ADCType> *memory) { return true; }
        bool doWork(volatile MemoryRepresentation<ADCType> *memory) { return true; }
    };

}
#endif