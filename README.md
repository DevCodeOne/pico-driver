# What it is

Generate custom i2c device which can be controlled by another controller.
Memorylayout for the devices is generated at compile time.

# TODO

- Add concepts for all the different types
- Id system needs to take configuration into consideration
- Implement devices
- Add I2CSettings type
- Add possible input adapter, to support e.g. can bus in the future
- Add i2c master, to that it can be used as a hub
- Maybe safe last state into flash