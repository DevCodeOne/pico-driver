#pragma once

#include "device_memory.h"

namespace PicoDriver {

    namespace DeviceTags {
        struct ADC final : public std::integral_constant<DeviceId, DeviceId::ADC> {};
    }

    template<>
    struct MemoryRepresentation<DeviceTags::ADC> {
        ~MemoryRepresentation() = delete;

        uint16_t adcValue;
    } __attribute__((packed));

    template<typename Pin>
    class ADC {
        public:
        using Tag = DeviceTags::ADC;

        bool install() { 
            return true; 
        }

        bool doWork(volatile MemoryRepresentation<Tag> *memory) { 
            return true; 
        }
    };

}