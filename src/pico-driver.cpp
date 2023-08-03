#include <cstdlib>
#include <array>
#include <type_traits>
#include <variant>
#include <cstdio>

#include "device_memory.h"
#include "i2c_slave.h"
#include "devices.h"
#include "devices/pwm.h"
#include "devices/adc.h"
#include "devices/hx711.h"
#include "devices/drv8825.h"

#include "runtime_access.h"

using namespace PicoDriver;

// TODO: add concepts for all the different device types
static auto constexpr I2CDevice0 = i2c0;

using LEDPWM = PWM<Pin<25>, Hz<100u>>;
using DosingPump = DRV8825<Pin<26>, NoDirectionPin, Hz<200u>>;
using Servo = DRV8825<Pin<28>, Pin<27>, Hz<200u>>;
using DeviceStructure = DeviceList<
                                LEDPWM, ADC<Pin<16>>, DosingPump, Servo
                            >;
using i2cDevice = I2CSlave<I2CDevice0, SDA<Pin<4>>, SCL<Pin<5>>, Address<0x17>, Baudrate<400'000>, 
                            DeviceStructure
                        >;
int main() {
    // stdio_init_all();

    i2cDevice::install();
    // puts("Installed i2c device ...");
    while (1) {
        i2cDevice::run();
        // i2cDevice::printMemoryMap<16>();
        // printf("Got %u events \n", i2cDevice::numEvents());
        // sleep_ms(250);
    }

    /*std::array<uint8_t, 255> deviceMemory;
    using RuntimeAccessType = RuntimeAccess::RuntimeAccess<DeviceList<FixedPWMType>>;
    auto access = RuntimeAccessType::createRuntimeAccessFromInfo(deviceMemory);

    if (access) {
        // Set device
        for (auto &currentDevice : (*access)) {
            if (auto pwm = std::get_if<MemoryRepresentation<FixedPWMType> *>(&currentDevice); pwm) {
                (*pwm)->pwmValue = 0;
                // Send new data over i2c
                const auto memorySlice = access->toRawMemorySlice(*pwm);
            }
        }
        // Update to device memory
    }*/

    return EXIT_SUCCESS;
}