#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <algorithm>
#include <numeric>
#include <optional>
#include <type_traits>
#include <tuple>

#include <hardware/i2c.h>
#include <pico/i2c_slave.h>
#include <pico/stdlib.h>

#include "device_helper_types.h"
#include "device_info.h"
#include "device_memory.h"
#include "devices.h"

namespace PicoDriver {

    // TODO: check max memory size of 255 when using 1-byte addresses
    // TODO: check if combination of type and index is unique -> two types cannot share the same id
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
                static inline Memory<Devices ...> data;
                static inline std::tuple<Devices ...> runtimeDevices;

                using LoopDevices = TypeUtils::ConstexprFor<sizeof...(Devices) - 1, sizeof...(Devices), Devices ...>;
            };

            using Values = ValuesType<DeviceListType>;

            static inline auto &memAddress  = Values::memAddress;
            static inline auto &data  = Values::data;
            static inline auto &runtimeDevices  = Values::runtimeDevices;

            using LoopDevices = typename Values::LoopDevices;

        public:

            // TODO: do this without using stdio stuff directly
            template<size_t NumBytesPerRow>
            static void printMemoryMap() {
                for (unsigned int i = 0; i < Values::data.numDevices(); ++i) {
                    printf("offset[%u] = %x \n", i, decltype(Values::data)::offset(i));
                }
                for (unsigned int i = decltype(Values::data)::offset(0); i < Values::data.memorySize(); ++i) {
                    if (i != NumBytesPerRow) {
                        printf(" %x", (unsigned int) Values::data.readRaw(i));
                    } else {
                        i = 0;
                        puts("\n");
                        printf("%x ", (unsigned int) Values::data.readRaw(i));
                    }
                }
            }

            static bool install() { 
                // Init all device memory, so the device is ready, by the time i2c is started
                LoopDevices::call([](auto index, auto &instance) {
                        constexpr auto Index = decltype(index)::value;
                        return instance.install(data.template getEntry<Index>());
                   }, runtimeDevices);


                gpio_init(SDAPin::value);
                gpio_set_function(SDAPin::value, GPIO_FUNC_I2C);
                // gpio_pull_up(SDAPin::value);


                gpio_init(SCLPin::value);
                gpio_set_function(SCLPin::value, GPIO_FUNC_I2C);
                // gpio_pull_up(SCLPin::value);

                i2c_init(I2CDevice, Baudrate::value);

                i2c_slave_init(I2CDevice, I2CAddress::value, &I2CSlave::handler);
                return true; 
            }

            static void run() { 
                LoopDevices::call([](auto index, auto &instance) {
                    constexpr auto Index = decltype(index)::value;
                    return instance.doWork(data.template getEntry<Index>());
                }, runtimeDevices);
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
                    if (!memAddress) {
                        memAddress = 0;
                    }
                    i2c_write_byte_raw(i2c, data.readRaw(*memAddress));
                    ++*memAddress;
                    break;
                    case I2C_SLAVE_FINISH:
                    memAddress = std::nullopt;
                    break;
                }
            }

        };



}