// Copyright 2024 Encore Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "audio_resampler.h"
#include "config_headless.h"
#include "emu_window/emu_window_headless.h"
#include "emu_window/emu_window_headless_gl.h"
#include "input_factory/headless_input_factory.h"
#include "savestate_mt.h"

namespace Headless {

class EncoreContext {
public:
    EncoreContext(ConfigCallbackInterface& config_interface, GLCallbackInterface& gl_interface,
                  InputCallbackInterface& input_interface);
    ~EncoreContext();

    std::pair<bool, std::string> InstallCIA(const std::string& cia_path);
    std::optional<std::string> LoadROM(const std::string& rom_path);
    void RunFrame();
    void Reset();
    std::pair<u32, u32> GetVideoVirtualDimensions() const;
    std::pair<u32, u32> GetVideoBufferDimensions() const;
    u32 GetGLTexture() const;
    void ReadFrameBuffer(u32* dest_buffer);
    std::span<const s16> GetAudio() const;
    void ReloadConfig() const;
    std::size_t StartSaveState();
    void FinishSaveState(void* dest_buffer);
    void LoadState(void* src_buffer, std::size_t buffer_len) const;
    std::pair<const u8*, std::size_t> GetMemoryRegion(Memory::Region region) const;
    std::tuple<Common::Rectangle<u32>, bool, bool> GetTouchScreenLayout() const;

private:
    Core::System& system;
    std::unique_ptr<EmuWindow_Headless> window;
    std::unique_ptr<Config_Headless> config;
    std::unique_ptr<Savestate_MT> savestate_mt;
    std::unique_ptr<AudioResampler> audio_resampler;
};

} // namespace Headless
