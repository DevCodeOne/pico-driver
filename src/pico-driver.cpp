#include <cstdlib>

#include "device_memory.h"
#include "i2c_slave.h"
#include "devices.h"

using namespace PicoDriver;

// TODO: automatically generate some device info in the first part of the device memory
// TODO: generate address with devices + some offset
using MemoryType = Memory<Devices::PWM, Devices::ADC, Devices::DRV8825>;
using i2cDevice = I2CSlave<SDA<Pin<15>>, SCL<Pin<14>>, Address<32>, Baudrate<100000>, MemoryType>;

int main() {

    i2cDevice::install();
    i2cDevice::run();

    return EXIT_SUCCESS;
}