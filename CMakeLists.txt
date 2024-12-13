function(target_optimize_size target)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
            CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${target} PRIVATE -Os)
    endif()
endfunction()

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

    target_optimize_size(${COMPONENT_LIB})
elseif(PICO_SDK_VERSION_STRING)
    # Pico SDK
    file(GLOB target_src
        src/*.cpp
        src/platform/*.cpp
        src/drivers/*.cpp
    )
    add_library(OneWireNg STATIC ${target_src})

    target_include_directories(OneWireNg PUBLIC src)
    target_link_libraries(OneWireNg PUBLIC
        pico_stdlib
        hardware_pio
    )

    target_optimize_size(OneWireNg)
endif()
