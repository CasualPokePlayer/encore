set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/CMakeModules)

add_library(citra-headless
    SHARED
    audio_resampler.cpp
    audio_resampler.h
    cinterface.cpp
    citra_context.cpp
    citra_context.h
    config_headless.cpp
    config_headless.h
    blip_buf/blip_buf.cpp
    blip_buf/blip_buf.h
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

create_target_directory_groups(citra-headless)

target_link_libraries(citra-headless PRIVATE citra_common citra_core input_common network)
target_link_libraries(citra-headless PRIVATE Boost::serialization glad libzstd_static)
target_link_libraries(citra-headless PRIVATE ${PLATFORM_LIBRARIES} SDL2::SDL2 Threads::Threads)
set_target_properties(citra-headless PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ENABLE_LTO})

if (CITRA_USE_PRECOMPILED_HEADERS)
    target_precompile_headers(citra-headless PRIVATE precompiled_headers.h)
endif()
