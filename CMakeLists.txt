if(IDF_VER)
    if(IDF_TARGET STREQUAL "esp8266")
        # ESP8266 RTOS SDK
        idf_component_register(
            INCLUDE_DIRS
                src
            SRC_DIRS
                src
                src/platform
                src/drivers
        )
    else()
        # ESP-IDF
        idf_component_register(
            INCLUDE_DIRS
                src
            SRC_DIRS
                src
                src/platform
                src/drivers
            PRIV_REQUIRES
                driver
                esp_timer
                soc
        )
    endif()
    return()
endif()

if(PICO_SDK_VERSION_STRING)
    # Pico SDK
    add_library(OneWireNg STATIC
        src/OneWireNg.cpp
        src/OneWireNg_BitBang.cpp
        src/platform/Platform_Delay.cpp
        src/platform/Platform_TimeCritical.cpp
        src/drivers/DSTherm.cpp
    )
    target_include_directories(OneWireNg PUBLIC src)
    target_link_libraries(OneWireNg PUBLIC
        pico_stdlib
        hardware_pio
    )
    return()
endif()
