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

#include "runtime_access.h"

using namespace PicoDriver;

// TODO: add concepts for all the different device types
static auto constexpr I2CDevice0 = i2c0;

using LEDPWM = PWM<Pin<25>, Hz<100u>>;
using DeviceStructure = DeviceList<
                                LEDPWM, ADC<Pin<16>>
                            >;
using i2cDevice = I2CSlave<I2CDevice0, SDA<Pin<15>>, SCL<Pin<14>>, Address<32>, Baudrate<100000>, 
                            DeviceStructure
                        >;
int main() {
    stdio_init_all();

    i2cDevice::install();
    i2cDevice::run();

    std::array<uint8_t, 255> deviceMemory;
    auto access = RuntimeAccess::RuntimeAccess<DeviceList<FixedPWMType>>::createRuntimeAccessFromInfo(deviceMemory);
    auto otheraccess = RuntimeAccess::RuntimeAccess<DeviceList<FixedPWMType>>::createRuntimeAccessFromInfo(deviceMemory);

    swap(access, otheraccess);

    if (access) {
        // begin, end iterator access
        for (auto &currentDevice : (*access)) {
            if (auto pwm = std::get_if<MemoryRepresentation<FixedPWMType> *>(&currentDevice); pwm) {
                (*pwm)->pwmValue = 0;
            } 
        }
    }

    return EXIT_SUCCESS;
}