// Copyright 2023 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "emu_window_headless.h"

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
