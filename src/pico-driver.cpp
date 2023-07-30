#include <cstdlib>
#include <array>
#include <type_traits>
#include <variant>

#include "device_memory.h"
#include "i2c_slave.h"
#include "devices.h"
#include "devices/pwm.h"
#include "devices/adc.h"
#include "devices/hx711.h"
#include "devices/drv8825.h"

#include "build_config.h"
#include "runtime_access.h"

using namespace PicoDriver;

// TODO: add concepts for all the different device types
static auto constexpr I2CDevice0 = i2c0;

using LEDPWM = PWM<Pin<25>, Hz<100u>>;
using DeviceStructure = DeviceList<
                                LEDPWM,
                                ADC<Pin<16>>
                            >;
using i2cDevice = I2CSlave<I2CDevice0, SDA<Pin<15>>, SCL<Pin<14>>, Address<32>, Baudrate<100000>, 
                            DeviceStructure
                        >;
int main() {
    stdio_init_all();

    i2cDevice::install();
    i2cDevice::run();

    std::array<char, 128> deviceInfoBytes;
    auto access = RuntimeAccess::RuntimeAccess<DeviceStructure>::createRuntimeAccessFromInfo(deviceInfoBytes);

    if (access) {
        // access[0] access operator
        (*access)[0];

        // begin, end iterator access
        for (auto &currentDevice : (*access)) {
            if (auto pwm = std::get_if<MemoryRepresentation<LEDPWM> *>(&currentDevice); pwm) {
                (*pwm)->pwmValue = 0;
            }
        }

        // returns runtimedevice which is a variant of devices
        // runtimedevice has access to memoryrepresentation
    }

    return EXIT_SUCCESS;
}