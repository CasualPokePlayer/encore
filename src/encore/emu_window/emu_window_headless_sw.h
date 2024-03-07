// Copyright 2024 Encore Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "emu_window_headless.h"

namespace Headless {

class EmuWindow_Headless_SW final : public EmuWindow_Headless {
public:
    explicit EmuWindow_Headless_SW(Core::System& system);
    ~EmuWindow_Headless_SW();

    std::pair<u32, u32> GetVideoBufferDimensions() const override;
    void ReadFrameBuffer(u32* dest_buffer) const override;
    void ReloadConfig() override;

protected:
    void Present() override;

private:
    Layout::FramebufferLayout const layout{Layout::DefaultFrameLayout(
        Core::kScreenTopWidth, Core::kScreenTopHeight + Core::kScreenBottomHeight, false, false)};
    std::unique_ptr<u32[]> framebuffer;
};

} // namespace Headless
