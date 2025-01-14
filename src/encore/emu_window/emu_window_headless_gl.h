// Copyright 2024 Encore Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "../config_headless.h"
#include "emu_window_headless.h"
#include "video_core/renderer_opengl/gl_resource_manager.h"

namespace Headless {

struct GLCallbackInterface {
    void* (*RequestGLContext)();
    void (*ReleaseGLContext)(void* gl_context);
    void (*ActivateGLContext)(void* gl_context);
    void* (*GetGLProcAddress)(const char* proc);
};

class EmuWindow_Headless_GL final : public EmuWindow_Headless {
public:
    explicit EmuWindow_Headless_GL(Core::System& system, GLCallbackInterface& gl_interface);
    ~EmuWindow_Headless_GL();

    u32 GetGLTexture() const;

    std::pair<u32, u32> GetVideoBufferDimensions() const override;
    void ReadFrameBuffer(u32* dest_buffer) const override;
    void ReloadConfig() override;

    std::unique_ptr<Frontend::GraphicsContext> CreateSharedContext() const override;
    void MakeCurrent() override;

protected:
    void Present() override;

private:
    GLCallbackInterface const gl_interface;
    std::unique_ptr<Frontend::GraphicsContext> context;

    u32 width, height;
    OpenGL::OGLTexture final_texture;
    OpenGL::OGLFramebuffer final_texture_fbo;
    OpenGL::OGLBuffer final_texture_pbo;

    void ResetGLTexture();
};

} // namespace Headless
