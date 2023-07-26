#pragma once

#include <cstdint>
#include <type_traits>

namespace PicoDriver {
    namespace Devices {
        enum struct DeviceId : uint8_t { PWM, ADC, DRV8825, HX711 };

        struct PWM final : public std::integral_constant<DeviceId, DeviceId::PWM> {};
        struct ADC final : public std::integral_constant<DeviceId, DeviceId::PWM> {};
        struct DRV8825 final : public std::integral_constant<DeviceId, DeviceId::PWM> {};
        struct HX711 final : public std::integral_constant<DeviceId, DeviceId::PWM> {};
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