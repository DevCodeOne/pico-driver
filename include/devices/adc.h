#pragma once

#include "device_memory.h"

namespace PicoDriver {
    template<typename Pin>
    class ADC;

    template<typename Pin>
    struct MemoryRepresentation<ADC<Pin>> {
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

        bool install(volatile MemoryRepresentation<ADC> *memory) { return true; }
        bool doWork(volatile MemoryRepresentation<ADC> *memory) { return true; }
    };

}
#endif