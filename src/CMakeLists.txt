if (BUILD_PICO_DEVICE_DRIVER)

add_executable(pico_driver
        pico-driver.cpp
)

# pull in common dependencies
target_link_libraries(pico_driver pico_stdlib pico_i2c_slave hardware_i2c hardware_pwm hardware_pio hardware_dma)
target_compile_options(pico_driver PUBLIC -Os)
target_compile_definitions(pico_driver PUBLIC -DGENERATE_HEADER=1)
target_include_directories(pico_driver PUBLIC ../include)
pico_enable_stdio_usb(pico_driver 1)
pico_enable_stdio_uart(pico_driver 0)
pico_generate_pio_header(pico_driver ${PROJECT_SOURCE_DIR}/include/drv8825.pio OUTPUT_DIR ${PROJECT_SOURCE_DIR}/include/generated)

# create map/bin/hex file etc.
pico_add_extra_outputs(pico_driver)
else()
add_library(pico_driver_i2c INTERFACE)
target_include_directories(pico_driver_i2c INTERFACE ../include)
target_compile_definitions(pico_driver_i2c INTERFACE MINIMAL=1)
endif()