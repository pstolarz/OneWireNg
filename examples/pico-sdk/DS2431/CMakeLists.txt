cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(DS2431)
pico_sdk_init()

add_executable(DS2431 DS2431.cpp)

add_subdirectory(OneWireNg)
target_link_libraries(DS2431 PRIVATE OneWireNg)
target_compile_definitions(DS2431 PRIVATE USE_OD_MODE)
target_compile_definitions(DS2431 PRIVATE CONFIG_OVERDRIVE_ENABLED)
target_compile_definitions(DS2431 PRIVATE CONFIG_CRC16_ENABLED)
target_compile_definitions(DS2431 PRIVATE PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS=-1)

pico_enable_stdio_usb(DS2431 1)
pico_enable_stdio_uart(DS2431 0)

pico_add_extra_outputs(DS2431)
