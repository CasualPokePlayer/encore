// Copyright 2024 Encore Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "emu_window_headless.h"
#include "video_core/gpu.h"
#include "video_core/renderer_base.h"

using namespace Headless;

EmuWindow_Headless::EmuWindow_Headless(Core::System& system_) : EmuWindow(false), system(system_) {
    strict_context_required = false;
    frame_has_passed = false;
}

EmuWindow_Headless::~EmuWindow_Headless() = default;

void EmuWindow_Headless::RunFrame() {
    while (!frame_has_passed) {
        ASSERT(system.RunLoop() == Core::System::ResultStatus::Success);
    }

    Present();
    frame_has_passed = false;
}

void EmuWindow_Headless::PollEvents() {
    // this is called each frame, so we can use this as a signal that a frame has passed
    frame_has_passed = true;
}

void EmuWindow_Headless::ForcePresent() {
    system.GPU().Renderer().SwapBuffers();
    Present();
    // PollEvents will get called in SwapBuffers, so make sure we keep frame_has_passed false
    frame_has_passed = false;
}
