cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
#include(pico_sdk_import.cmake)

project(my_project C CXX ASM )

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
add_executable(main)

pico_generate_pio_header(main ${CMAKE_CURRENT_LIST_DIR}/shift.pio)

target_sources(main PRIVATE main.c)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(main pico_stdlib hardware_adc hardware_dma hardware_gpio hardware_pio pico_stdio_usb)

# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(main)
