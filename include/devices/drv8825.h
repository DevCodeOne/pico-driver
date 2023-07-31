#pragma once

#include "device_memory.h"
#include "devices.h"
#include <sys/wait.h>


namespace PicoDriver {
    template<typename StepPin, typename DirPin, typename Freq>
    requires (Freq::value > 0)
    class DRV8825;

    template<typename StepPin, typename DirPin, typename Freq>
    struct MemoryRepresentation<DRV8825<StepPin, DirPin, Freq>> {
        ~MemoryRepresentation() = delete;

        uint16_t pwmValue;
    } __attribute__((packed));
}

// Device-specific code and includes
#if !defined(MINIMAL) || MINIMAL == 0

namespace PicoDriver {
    template<typename StepPin, typename DirPin, typename Freq>
    requires (Freq::value > 0)
    class DRV8825 {
        public:

        bool install(volatile MemoryRepresentation<DRV8825> *memory) { return true; }
        bool doWork(volatile MemoryRepresentation<DRV8825> *memory) { return true; }
    };
}
#endif