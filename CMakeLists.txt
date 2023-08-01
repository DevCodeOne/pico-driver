cmake_minimum_required(VERSION 3.12)
project(pico-driver C CXX ASM)

cmake_policy(SET CMP0077 NEW)
option(BUILD_PICO_DEVICE_DRIVER "Set to false if consuming this" ON)

if (BUILD_PICO_DEVICE_DRIVER)
include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)
pico_sdk_init()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

endif()

add_subdirectory(src)