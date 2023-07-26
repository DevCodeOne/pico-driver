#pragma once

#include <cstdint>

namespace PicoDriver {
    namespace Devices {
        struct PWM; 
        struct ADC; 
        struct DRV8825;
        struct HX711;
    };

    template<typename Device>
    struct MemoryRepresentation;

    template<>
    struct MemoryRepresentation<Devices::PWM> {
        ~MemoryRepresentation() = delete;

        uint16_t pwmValue;
    } __attribute__((packed));

    template<>
    struct MemoryRepresentation<Devices::ADC> {
        ~MemoryRepresentation() = delete;

        uint16_t adcValue;
    } __attribute__((packed));

    template<>
    struct MemoryRepresentation<Devices::DRV8825> {
        ~MemoryRepresentation() = delete;

        uint16_t steps;
        bool direction;
    } __attribute__((packed));

    template<>
    struct MemoryRepresentation<Devices::HX711> {
        ~MemoryRepresentation() = delete;

        uint16_t value;
    } __attribute__((packed));

    class Device {

    };
}