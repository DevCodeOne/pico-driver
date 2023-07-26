#pragma once

#include <cstdint>
#include <cstddef>
#include <optional>

#include <hardware/i2c.h>
#include <pico/i2c_slave.h>
#include <pico/stdlib.h>
#include <type_traits>

namespace PicoDriver {

    template<size_t Number> 
    using Baudrate = std::integral_constant<uint8_t, Number>;

    template<uint16_t Number> 
    using Address = std::integral_constant<uint8_t, Number>;

    template<uint8_t Number> 
    using Pin = std::integral_constant<uint8_t, Number>;

    template<typename Pin>
    struct SDA { 
        static constexpr auto value = Pin::value;
    };

    template<typename Pin>
    struct SCL { 
        static constexpr auto value = Pin::value;
    };

    template<typename SDAPin, typename SCLPin, typename I2CAddress, typename Baudrate, typename MemoryRepresentation>
    class I2CSlave {
        public:

            static bool install() { 
                gpio_init(SDAPin::value);
                gpio_set_function(SDAPin::value, GPIO_FUNC_I2C);
                gpio_pull_up(SDAPin::value);


                gpio_init(SCLPin::value);
                gpio_set_function(SCLPin::value, GPIO_FUNC_I2C);
                gpio_pull_up(SCLPin::value);

                i2c_init(i2c0, Baudrate::value);

                i2c_slave_init(i2c0, I2CAddress::value, &I2CSlave::handler);
                return true; 
            }

            [[noreturn]] static void run() { 
                while(1); 
            }

        private:
            static void handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
                switch (event) {
                    case I2C_SLAVE_RECEIVE:
                    if (!memAddress) {
                        memAddress = i2c_read_byte_raw(i2c);
                    } else {
                        data.writeRaw(*memAddress, i2c_read_byte_raw(i2c));
                        ++*memAddress;
                    }
                    break;
                    case I2C_SLAVE_REQUEST:
                    i2c_write_byte_raw(i2c, data.readRaw(*memAddress));
                    ++*memAddress;
                    break;
                    case I2C_SLAVE_FINISH:
                    memAddress = std::nullopt;
                    break;
                }
            }

            static inline std::optional<uint8_t> memAddress;
            static inline MemoryRepresentation data;
    };
}