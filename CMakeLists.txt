if(IDF_VER)
    # ESP-IDF
    if(IDF_VERSION_MAJOR GREATER_EQUAL 5)
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
    else()
        idf_component_register(
            INCLUDE_DIRS
                src
            SRC_DIRS
                src
                src/platform
                src/drivers
        )
    endif()
    set(TARGET ${COMPONENT_LIB})
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
    set(TARGET OneWireNg)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
        CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${TARGET} PRIVATE -Os)
endif()
