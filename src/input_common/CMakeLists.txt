add_library(input_common STATIC
    analog_from_button.cpp
    analog_from_button.h
    keyboard.cpp
    keyboard.h
    main.cpp
    main.h
    motion_emu.cpp
    motion_emu.h
    precompiled_headers.h
    touch_from_button.cpp
    touch_from_button.h
)

create_target_directory_groups(input_common)
target_link_libraries(input_common PUBLIC encore_core PRIVATE encore_common ${Boost_LIBRARIES})

if (ENCORE_USE_PRECOMPILED_HEADERS)
    target_precompile_headers(input_common PRIVATE precompiled_headers.h)
endif()
