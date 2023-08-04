#pragma once

namespace PicoDriver {
    template<size_t Number> 
    using Baudrate = std::integral_constant<uint8_t, Number>;

    template<uint16_t Number> 
    using Address = std::integral_constant<uint8_t, Number>;

    template<uint8_t Number> 
    using Pin = std::integral_constant<uint8_t, Number>;

    template<auto Value>
    requires (std::is_unsigned_v<decltype(Value)>)
    using Hz = std::integral_constant<decltype(Value), Value>;

    template<typename Pin>
    struct SDA { 
        static constexpr auto value = Pin::value;
    };

    template<typename Pin>
    struct SCL { 
        static constexpr auto value = Pin::value;
    };

}