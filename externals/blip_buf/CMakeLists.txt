add_library(blip_buf STATIC
    blip_buf.c
    blip_buf.h
)

create_target_directory_groups(blip_buf)

target_include_directories(blip_buf INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
