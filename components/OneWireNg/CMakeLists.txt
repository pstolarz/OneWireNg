set(src_dirs src
             src/platform
             src/drivers)

set(inc_dirs src
             src/platform
             src/drivers
             src/utils)

idf_component_register(
    SRC_DIRS ${src_dirs}
    INCLUDE_DIRS ${inc_dirs})
