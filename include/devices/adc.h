#pragma once

#include "device_memory.h"

namespace PicoDriver {

    template<typename Pin>
    class ADC {
        public:
        bool install() { 
            return true; 
        }

        bool doWork(volatile MemoryRepresentation<ADC> *memory) { 
            return true; 
        }
    };

    template<typename Pin>
    struct MemoryRepresentation<ADC<Pin>> {
        ~MemoryRepresentation() = delete;

        uint16_t adcValue;
    } __attribute__((packed));



}