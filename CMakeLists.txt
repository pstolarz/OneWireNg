if(IDF_VER)
    # ESP-IDF build
    idf_component_register(
        INCLUDE_DIRS
            src
        SRC_DIRS
            src
            src/platform
            src/drivers
    )
    return()
endif()

if(PICO_SDK_VERSION_STRING)
    # Pico SDK build
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
