#pragma once

// TODO: define for all devices
// forward declare types and then define GenId
template<typename T>
struct GenId;

template<typename Pin, typename Freq>
struct GenId<PWM<Pin, Freq>> {
    static inline constexpr uint8_t value = 0x1;
};