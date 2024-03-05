set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/CMakeModules)

add_library(encore
    SHARED
    audio_resampler.cpp
    audio_resampler.h
    cinterface.cpp
    config_headless.cpp
    config_headless.h
    encore_context.cpp
    encore_context.h
    emu_window/emu_window_headless.cpp
    emu_window/emu_window_headless.h
    emu_window/emu_window_headless_gl.cpp
    emu_window/emu_window_headless_gl.h
    emu_window/emu_window_headless_sw.cpp
    emu_window/emu_window_headless_sw.h
    input_factory/headless_axis_factory.cpp
    input_factory/headless_axis_factory.h
    input_factory/headless_button_factory.cpp
    input_factory/headless_button_factory.h
    input_factory/headless_input_factory.h
    input_factory/headless_motion_factory.cpp
    input_factory/headless_motion_factory.h
    input_factory/headless_touch_factory.cpp
    input_factory/headless_touch_factory.h
    precompiled_headers.h
    savestate_mt.cpp
    savestate_mt.h
)

create_target_directory_groups(encore)

target_link_libraries(encore PRIVATE encore_common encore_core input_common)
target_link_libraries(encore PRIVATE blip_buf glad zstd)
target_link_libraries(encore PRIVATE ${PLATFORM_LIBRARIES} Threads::Threads)
set_target_properties(encore PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ENABLE_LTO})

if (ENCORE_USE_PRECOMPILED_HEADERS)
    target_precompile_headers(encore PRIVATE precompiled_headers.h)
endif()
