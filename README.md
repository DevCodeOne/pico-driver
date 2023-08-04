# What it is

Generate custom i2c device which can be controlled by another controller.
The memory layout for the devices is generated at compile time.
This system can be used to program microcontrollers, which are used on pcbs and use hardwired devices.
Instead of buying gpio expanders, or pwm drivers one can use inexpensive microcontrollers.

# TODO

- Add memoryrepresentation sizes, so that unsupported types can at least be skipped
- Add multiple definitions, which can be used and are differentiable via their address
- Add concepts for all the different types
- Add possibility to later on support bigger memory sizes than 255
- Id system needs to take configuration into consideration
- Implement more devices
- Add I2CSettings type
- Add possible input adapter, to support e.g. can bus in the future
- Add i2c master, to that it can be used as a hub
- Maybe safe last state into flash

## TODO sometime later, don't know how to do that nicely
- Add some kind of mechanism to generate device via build-system