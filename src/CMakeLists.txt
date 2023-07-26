add_executable(pico_driver
        pico-driver.cpp
        )

# pull in common dependencies
target_link_libraries(pico_driver pico_stdlib pico_i2c_slave hardware_i2c)
target_include_directories(pico_driver PUBLIC ../include)

# create map/bin/hex file etc.
pico_add_extra_outputs(pico_driver)