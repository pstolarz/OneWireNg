cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(DallasTemperature)
pico_sdk_init()

add_executable(DallasTemperature DallasTemperature.cpp)

add_subdirectory(OneWireNg)
target_link_libraries(DallasTemperature PRIVATE OneWireNg)

pico_enable_stdio_usb(DallasTemperature 1)
pico_enable_stdio_uart(DallasTemperature 0)

pico_add_extra_outputs(DallasTemperature)
