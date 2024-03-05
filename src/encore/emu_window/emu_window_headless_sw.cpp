// Copyright 2024 Encore Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "common/settings.h"
#include "emu_window_headless_sw.h"
#include "video_core/gpu.h"
#include "video_core/renderer_base.h"
#include "video_core/renderer_software/renderer_software.h"

using namespace Headless;

EmuWindow_Headless_SW::EmuWindow_Headless_SW(Core::System& system) : EmuWindow_Headless(system) {
    framebuffer = std::make_unique<u32[]>(layout.width * layout.height);
    ReloadConfig(1);
}

EmuWindow_Headless_SW::~EmuWindow_Headless_SW() = default;

void EmuWindow_Headless_SW::Present() {
    // TODO: Recheck this when the software renderer isn't just broken
    const auto& renderer = static_cast<SwRenderer::RendererSoftware&>(system.GPU().Renderer());
    const auto draw_screen = [&](VideoCore::ScreenId screen_id) {
        const auto& info = renderer.Screen(screen_id);
        const auto src = info.pixels.data();
        const auto src_width = static_cast<u32>(info.width);
        const auto src_height = static_cast<u32>(info.height);
        const auto dst_rect =
            screen_id == VideoCore::ScreenId::TopLeft ? layout.top_screen : layout.bottom_screen;
        // ASSERT(src_height == dst_rect.GetWidth() && src_width == dst_rect.GetHeight());
        const auto dst_width = layout.width;
        // const auto dst_height = layout.height;
        auto dst = &framebuffer[dst_rect.left + dst_rect.top * dst_width];
        for (u32 i = 0; i < src_height; i++) {
            for (u32 j = 0; j < src_width; j++) {
                dst[j * dst_width + i] = src[i * src_width + j];
            }
        }
    };

    // draw_screen(VideoCore::ScreenId::TopLeft);
    draw_screen(VideoCore::ScreenId::Bottom);
}

std::pair<u32, u32> EmuWindow_Headless_SW::GetVideoDimensions() const {
    return std::make_pair(layout.width, layout.height);
}

void EmuWindow_Headless_SW::ReadFrameBuffer(u32* dest_buffer) const {
    std::memcpy(dest_buffer, framebuffer.get(), layout.width * layout.height * sizeof(u32));
}

void EmuWindow_Headless_SW::ReloadConfig(u32 window_scale_factor) {
    std::fill_n(framebuffer.get(), layout.width * layout.height,
                static_cast<u8>(Settings::values.bg_red.GetValue() * 255) << 24 |
                    static_cast<u8>(Settings::values.bg_green.GetValue() * 255) << 16 |
                    static_cast<u8>(Settings::values.bg_blue.GetValue() * 255) << 8);
}
