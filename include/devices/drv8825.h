#pragma once

#include "device_memory.h"
#include "device_mapping.h"
#include "devices.h"

namespace PicoDriver {
    struct NoDirectionPin {};

    template<typename StepPin, typename DirPin, typename Freq>
    requires (Freq::value > 0)
    class DRV8825;

    struct StepperMotorWithDir {
        static inline constexpr uint8_t Id = 0x3;
    };

    struct StepperMotorWithoutDir {
        static inline constexpr uint8_t Id = 0x4;
    };

    template<typename StepPin, typename DirPin, typename Freq>
    struct MapToType<DRV8825<StepPin, DirPin, Freq>> {
        using TagType = StepperMotorWithDir;
        using Type = MemoryRepresentation<TagType>;
    };

    template<typename StepPin, typename Freq>
    struct MapToType<DRV8825<StepPin, NoDirectionPin, Freq>> {
        using TagType = StepperMotorWithoutDir;
        using Type = MemoryRepresentation<TagType>;
    };

    enum struct StepperDirection : uint8_t { Left, Right };

    template<>
    struct MemoryRepresentation<StepperMotorWithDir> {
        ~MemoryRepresentation() = delete;

        uint16_t steps;
        StepperDirection direction;
    } __attribute__((packed));

    template<>
    struct MemoryRepresentation<StepperMotorWithoutDir> {
        ~MemoryRepresentation() = delete;

        uint16_t steps;
    } __attribute__((packed));

}

// Device-specific code and includes
#if !defined(MINIMAL) || MINIMAL == 0

namespace PicoDriver {
    template<typename StepPin, typename DirPin, typename Freq>
    requires (Freq::value > 0)
    class DRV8825 {
        public:

        bool install(volatile MemoryRepresentation<StepperMotorWithDir> *memory) { return true; }
        bool doWork(volatile MemoryRepresentation<StepperMotorWithDir> *memory) { return true; }
    };

    template<typename StepPin, typename Freq>
    requires (Freq::value > 0)
    class DRV8825<StepPin, NoDirectionPin, Freq> {
        public:

        bool install(volatile MemoryRepresentation<StepperMotorWithoutDir> *memory) { return true; }
        bool doWork(volatile MemoryRepresentation<StepperMotorWithoutDir> *memory) { return true; }
    };
}
#endif