#pragma once

#include <cstdint>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <tuple>

#include <hardware/i2c.h>
#include <pico/i2c_slave.h>
#include <pico/stdlib.h>

#include "device_memory.h"
#include "device_info.h"

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

    template<size_t Index, size_t Len, typename ... Args>
    struct ConstexprFor {
        template<typename Callable>
        static auto call(Callable callable, std::tuple<Args ...> &args) {
            using ReturnType = decltype(ConstexprFor<Index - 1, Len, Args ...>::call(callable, args));
            using IndexType = std::integral_constant<uint16_t, Index>;
            if constexpr (std::is_same_v<ReturnType, void>) {
                return callable(IndexType{}, std::get<Index>(args)) & ConstexprFor<Index - 1, Len, Args ...>::call(callable, args);
            } else {
                return callable(IndexType{}, std::get<Index>(args));
            }
        }
    };

    template<size_t Len, typename ... Args>
    struct ConstexprFor<0, Len, Args ...> {
        template<typename Callable>
        static auto call(Callable callable, std::tuple<Args ...> &args) {
            using IndexType = std::integral_constant<uint16_t, 0>;
            return callable(IndexType{}, std::get<0>(args));
        }
    };

    template<typename SDAPin, typename SCLPin, typename I2CAddress, typename Baudrate, typename ... Devices>
    class I2CSlave {
        public:

            static bool install() { 
                installRuntimeDevices();

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
                while(1) {
                    ConstexprFor<sizeof...(Devices) - 1, sizeof...(Devices), Devices ...>::call([](auto index, auto &instance) {
                        constexpr auto Index = decltype(index)::value + 1;
                        instance.doWork(data.template getEntry<Index>());
                    }, runtimeDevices);
                }
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

            static void installRuntimeDevices() {
                ConstexprFor<sizeof...(Devices) - 1, sizeof...(Devices), Devices ...>::call([](uint16_t index, auto &instance) {
                    instance.install();
                }, runtimeDevices);
            }

            static inline std::optional<uint8_t> memAddress;
            static inline Memory<DeviceInfo<Devices ...>, Devices ...> data;
            static inline std::tuple<Devices ...> runtimeDevices;
    };
}