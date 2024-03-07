// Copyright 2024 Encore Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "common/common_types.h"
#include "core/core.h"
#include "core/frontend/emu_window.h"

namespace Headless {

class EmuWindow_Headless : public Frontend::EmuWindow {
public:
    explicit EmuWindow_Headless(Core::System& system);
    virtual ~EmuWindow_Headless();

    void RunFrame();

    virtual std::pair<u32, u32> GetVideoVirtualDimensions() const;
    virtual std::pair<u32, u32> GetVideoBufferDimensions() const = 0;
    virtual void ReadFrameBuffer(u32* dest_buffer) const = 0;
    virtual void ReloadConfig() = 0;

    void PollEvents() override;

protected:
    Core::System& system;

    virtual void Present() = 0;

private:
    bool frame_has_passed;
};

} // namespace Headless
