add_executable(pico_driver
        pico-driver.cpp
        )

# pull in common dependencies
target_link_libraries(pico_driver pico_stdlib)

# create map/bin/hex file etc.
pico_add_extra_outputs(pico_driver)