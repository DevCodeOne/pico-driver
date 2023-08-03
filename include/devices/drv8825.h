#pragma once

#include "device_memory.h"
#include "device_mapping.h"
#include "devices.h"

namespace PicoDriver {
    struct NoDirectionPin {};
    struct NoEnablePin {};

    template<typename StepPin, typename DirPin, typename EnablePin, typename Freq>
    requires (Freq::value > 0)
    class DRV8825;

    template<typename DirPin, typename EnablePin>
    struct StepperMotorTag {
        static inline constexpr uint8_t Id = 0x3 + (std::is_same_v<DirPin, NoDirectionPin> ? 1 : 0) + (std::is_same_v<EnablePin, NoEnablePin> ? 2 : 0);
    };

    template<typename StepPin, typename DirPin, typename EnablePin, typename Freq>
    struct MapToType<DRV8825<StepPin, DirPin, EnablePin, Freq>> {
        using TagType = StepperMotorTag<DirPin, EnablePin>;
        using Type = MemoryRepresentation<TagType>;

        // TODO: use this type to allocate resources at compile time; so pins can't be used multiple times, except when using as a bus (e.g. i2c)
        using UsedPins = std::tuple<StepPin, DirPin, EnablePin>;
    };

    enum struct StepperDirection : uint8_t { Left, Right };

    template<typename Pin>
    struct EnableValue {
        bool enable;
    };

    template<>
    struct EnableValue<NoEnablePin> {};

    template<typename Pin>
    struct DirectionValue {
        StepperDirection direction;
    };

    template<>
    struct DirectionValue<NoDirectionPin> {};

    template<typename DirPin, typename EnablePin>
    struct MemoryRepresentation<StepperMotorTag<DirPin, EnablePin>> final : public DirectionValue<DirPin>, public EnableValue<EnablePin> {
        ~MemoryRepresentation() = delete;

        uint16_t steps;
    } __attribute__((packed));

}

// Device-specific code and includes
#if !defined(MINIMAL) || MINIMAL == 0

#include "generated/drv8825.pio.h"

namespace PicoDriver {
    template<typename StepPin, typename DirPin, typename EnablePin, typename Freq>
    requires (Freq::value > 0)
    class DRV8825 {
        public:

        using Tag = StepperMotorTag<DirPin, EnablePin>;

        bool install(volatile MemoryRepresentation<Tag> *memory) { return true; }
        bool doWork(volatile MemoryRepresentation<Tag> *memory) { 
            setEnable(memory);
            setDirection(memory);
            return true; 
        }

        private:

        template<typename T, typename = decltype((void) T::direction)>
        bool setDirection(volatile T *memory) {
            memory->direction = StepperDirection::Left;
            return true;
        }
        bool setDirection(...) { return true; }


        template<typename T, typename = decltype((void) T::enable)>
        bool setEnable(volatile T *memory) {
            memory->enable = 0;
            return true;
        }
        bool setEnable(...) { return true; }
    };

}
#endif