#pragma once

#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <numeric>
#include <optional>
#include <type_traits>
#include <tuple>

#include <hardware/i2c.h>
#include <pico/i2c_slave.h>
#include <pico/stdlib.h>

#include "device_memory.h"
#include "device_info.h"
#include "devices.h"

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

    template<typename T>
    constexpr bool IsUniqueSet(const T &iterateable) {
        return std::accumulate(iterateable.cbegin(), iterateable.cend(), size_t{0}, [&iterateable](const auto &sum, const auto &currentElement) {
            return sum + std::count(iterateable.cbegin(), iterateable.cend(), currentElement);
        }) == iterateable.size();
    }

    // TODO: Add DeviceInfo in front
    template<auto *I2CDevice, typename SDAPin, typename SCLPin, typename I2CAddress, typename Baudrate, typename DeviceListTypeWithoutInfo>
    class I2CSlave {
        private:
            using DeviceListType = typename DeviceListTypeWithoutInfo::AppendDevice<DeviceInfo<DeviceListTypeWithoutInfo>>;

            template<typename D>
            struct ValuesType;

            template<typename ... Devices>
            requires (TypeUtils::IsTypeSet<Devices ...>)
            struct ValuesType<DeviceList<Devices ...>> {

                static inline std::optional<uint8_t> memAddress;
                static inline Memory<DeviceInfo<DeviceListTypeWithoutInfo>, Devices ...> data;
                static inline std::tuple<Devices ...> runtimeDevices;

                using LoopDevices = TypeUtils::ConstexprFor<0, sizeof...(Devices), Devices ...>;
            };

            using Values = ValuesType<DeviceListType>;

            static inline auto &memAddress  = Values::memAddress;
            static inline auto &data  = Values::data;
            static inline auto &runtimeDevices  = Values::runtimeDevices;

            using LoopDevices = typename Values::LoopDevices;

        public:

            static bool install() { 
                installRuntimeDevices();

                gpio_init(SDAPin::value);
                gpio_set_function(SDAPin::value, GPIO_FUNC_I2C);
                gpio_pull_up(SDAPin::value);


                gpio_init(SCLPin::value);
                gpio_set_function(SCLPin::value, GPIO_FUNC_I2C);
                gpio_pull_up(SCLPin::value);

                i2c_init(I2CDevice, Baudrate::value);

                i2c_slave_init(I2CDevice, I2CAddress::value, &I2CSlave::handler);

                installRuntimeDevices();
                return true; 
            }

            [[noreturn]] static void run() { 
                while(1) {
                    LoopDevices::call([](auto index, auto &instance) {
                        constexpr auto Index = decltype(index)::value + 1;
                        return instance.doWork(data.template getEntry<Index>());
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
                LoopDevices::call([](uint16_t index, auto &instance) {
                    return instance.install();
                }, runtimeDevices);
            }

        };

}