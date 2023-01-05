cmake_minimum_required(VERSION 3.13)

set(FAMILY rp2040)
set(PICO_PLATFORM rp2040)
set(BOARD pico_sdk)
set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
#include(pico_sdk_import.cmake)

project(main C CXX ASM )

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
add_executable(main)

pico_generate_pio_header(main ${CMAKE_CURRENT_LIST_DIR}/shift.pio)

add_subdirectory(gpib)

target_sources(main PUBLIC main.c usb_descriptors.c usbtmc_app.c)

# Make sure TinyUSB can find tusb_config.h
target_include_directories(main PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(main PUBLIC pico_stdlib tinyusb_device tinyusb_board
    hardware_adc hardware_dma hardware_gpio hardware_pio pico_unique_id)

# enable usb output, disable uart output
# pico_enable_stdio_usb(main 1)
# pico_enable_stdio_uart(main 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(main)
