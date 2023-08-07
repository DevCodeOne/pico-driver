#include <cstdlib>
#include <array>
#include <type_traits>
#include <variant>
#include <cstdio>

#include "hardware/clocks.h"

#include "device_memory.h"
#include "i2c_slave.h"
#include "devices.h"
#include "devices/pwm.h"
#include "devices/adc.h"
#include "devices/hx711.h"
#include "devices/drv8825.h"
#include "devices/output.h"
#include "pico_resource.h"

#include "runtime_access.h"

using namespace PicoDriver;

// TODO: add concepts for all the different device types
static auto constexpr I2CDevice0 = i2c1;

// TODO: do resource check
// TODO: add pins to resources somehow (values can't be named the same), or use a tuple in the future
using DosingPump = DRV8825<PicoResource<PIOResource<PIODevice::Zero, PIOStateMachine::Zero>, DMAResource<DMAChannel::Two>>,
                            // Pin<20>, NoDirectionPin, Pin<21>, Hz<200u>>;
                            Pin<20>, NoDirectionPin, Pin<21>, Hz<100u>>;
using LEDPWM = PWM<Pin<25>, Hz<100u>>;
using GenOutput = Output<Pin<12>>;
using DeviceStructure = DeviceList<
                                DosingPump,
                                ADC<Pin<26>>,
                                InternalTemperatureADC,
                                GenOutput,
                                LEDPWM
                            >;
using i2cDevice = I2CSlave<I2CDevice0, SDA<Pin<2>>, SCL<Pin<3>>, Address<0x17>, Baudrate<400'000>, 
                            DeviceStructure
                        >;
int main() {
    stdio_init_all();


    i2cDevice::install();
    // TODO: integrate this into adc code
    adc_set_temp_sensor_enabled(true);
    // puts("Installed i2c device ...");
    while (1) {
        i2cDevice::run();
        // i2cDevice::printMemoryMap<16>();
        // printf("Got %u events \n", i2cDevice::numEvents());
        sleep_ms(250);
    }

    /*std::array<uint8_t, 255> deviceMemory;
    using RuntimeAccessType = RuntimeAccess::RuntimeAccess<DeviceList<FixedPWMType>>;
    auto access = RuntimeAccessType::createRuntimeAccessFromInfo(deviceMemory);

    if (access) {
        const char *tagName = "DStepper";
        // Set device
        for (auto &currentDevice : (*access)) {
            if (std::string_view("FPWM") == currentDevice.tagName()) {
                if (auto pwm = std::get_if<MemoryRepresentation<FixedPWMType> *>(&currentDevice); pwm) {
                    (*pwm)->pwmValue = 0;
                    // Send new data over i2c
                    const auto memorySlice = access->toRawMemorySlice(*pwm);
                }
            }
        }
        // Update to device memory
    }*/

    return EXIT_SUCCESS;
}