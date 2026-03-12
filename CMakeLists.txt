cmake_minimum_required(VERSION 3.13)

# Silence harmless macOS SDK warning
if(APPLE)
    set(CMAKE_OSX_SYSROOT "")
endif()

# Set the SDK path
set(PICO_SDK_PATH /Users/mreff555/src/pi/pico/pico-sdk)

# This must come BEFORE project()
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(test_project C CXX ASM)

pico_sdk_init()

add_executable(test_project
                main.cpp
                lcd.cpp)

target_link_libraries(test_project
    pico_stdlib
    hardware_i2c
    hardware_rtc
)

pico_add_extra_outputs(test_project)
pico_enable_stdio_usb(test_project 1)
pico_enable_stdio_uart(test_project 0)

# === AUTOMATIC FLASHING via `make flash` (macOS only) ===
# if(APPLE)
    add_custom_target(flash
        COMMAND ${CMAKE_COMMAND} -E echo "🔌 Looking for Pico bootloader at /Volumes/RPI-RP2..."
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_BINARY_DIR}/test_project.uf2
                /Volumes/RPI-RP2/
        DEPENDS test_project
        COMMENT "Flashing test_project.uf2 to Raspberry Pi Pico"
        VERBATIM
    )
# endif()