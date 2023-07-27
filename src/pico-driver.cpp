#include <cstdlib>
#include <array>
#include <type_traits>

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
using i2cDevice = I2CSlave<I2CDevice0, SDA<Pin<15>>, SCL<Pin<14>>, Address<32>, Baudrate<100000>, 
                            DeviceList<
                                PWM<Pin<13>, Hz<100u>>,
                                ADC<Pin<16>>
                            >
                        >;

int main() {

    i2cDevice::install();
    i2cDevice::run();

    return EXIT_SUCCESS;
}