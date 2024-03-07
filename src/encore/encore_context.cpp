// Copyright 2024 Encore Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <filesystem>
#include <fmt/format.h>

#include "audio_core/dsp_interface.h"
#include "common/logging/backend.h"
#include "core/core.h"
#include "core/frontend/applets/default_applets.h"
#include "core/frontend/input.h"
#include "core/hle/service/am/am.h"
#include "core/hw/aes/key.h"
#include "core/loader/loader.h"
#include "video_core/gpu.h"
#include "video_core/renderer_base.h"
#include "video_core/renderer_opengl/gl_state.h"

#include "emu_window/emu_window_headless_sw.h"
#include "input_factory/headless_axis_factory.h"
#include "input_factory/headless_button_factory.h"
#include "input_factory/headless_motion_factory.h"
#include "input_factory/headless_touch_factory.h"

#include "encore_context.h"

using namespace Headless;

EncoreContext::EncoreContext(ConfigCallbackInterface& config_interface,
                             GLCallbackInterface& gl_interface,
                             InputCallbackInterface& input_interface)
    : system(Core::System::GetInstance()) {
    config = std::make_unique<Config_Headless>(system, config_interface);
    Frontend::RegisterDefaultApplets(system);
    if (Settings::values.graphics_api.GetValue() == Settings::GraphicsAPI::OpenGL) {
        window = std::make_unique<EmuWindow_Headless_GL>(system, gl_interface);
    } else {
        window = std::make_unique<EmuWindow_Headless_SW>(system);
    }
    savestate_mt = std::make_unique<Savestate_MT>(system);
    audio_resampler = std::make_unique<AudioResampler>(system);
    Input::RegisterFactory<Input::ButtonDevice>(
        "headless", std::make_shared<HeadlessButtonFactory>(input_interface));
    Input::RegisterFactory<Input::AnalogDevice>(
        "headless", std::make_shared<HeadlessAxisFactory>(input_interface));
    Input::RegisterFactory<Input::TouchDevice>(
        "headless", std::make_shared<HeadlessTouchFactory>(input_interface));
    Input::RegisterFactory<Input::MotionDevice>(
        "headless", std::make_shared<HeadlessMotionFactory>(input_interface));
    // we may have set a new aes_keys.txt, force reload it
    HW::AES::InitKeys(true);
}

EncoreContext::~EncoreContext() {
    if (system.IsPoweredOn()) {
        window->MakeCurrent();
        system.Shutdown();
    }

    Input::UnregisterFactory<Input::ButtonDevice>("headless");
    Input::UnregisterFactory<Input::AnalogDevice>("headless");
    Input::UnregisterFactory<Input::TouchDevice>("headless");
    Input::UnregisterFactory<Input::MotionDevice>("headless");
}

std::pair<bool, std::string> EncoreContext::InstallCIA(const std::string& cia_path) {
    FileSys::CIAContainer container;
    const auto container_result = container.Load(cia_path);
    switch (container_result) {
    case Loader::ResultStatus::Error:
        return std::make_pair(false, "Failed to open .cia file!");
    case Loader::ResultStatus::ErrorInvalidFormat:
        return std::make_pair(false, "The .cia file is invalid!");
    case Loader::ResultStatus::Success:
        break; // expected case
    default:
        return std::make_pair(false, "Unknown error occurred while installing .cia file");
    }

    const auto get_installed_path = [&]() {
        const auto title_id = container.GetTitleMetadata().GetTitleID();
        const auto media_type = Service::AM::GetTitleMediaType(title_id);
        return Service::AM::GetTitleContentPath(media_type, title_id);
    };

    const auto is_executable = [](const std::string& installed_path) -> std::optional<std::string> {
        auto loader = Loader::GetLoader(installed_path);
        if (loader) {
            auto executable = false;
            const auto& loader_result = loader->IsExecutable(executable);
            switch (loader_result) {
            case Loader::ResultStatus::Error:
                return "Failed to open .app file!";
            case Loader::ResultStatus::ErrorInvalidFormat:
                return "The .app file is invalid!";
            case Loader::ResultStatus::ErrorEncrypted:
                return "The .app file could not be decrypted!";
            case Loader::ResultStatus::Success:
                break; // expected case
            default:
                return "Unknown error occurred while opening .app file";
            }

            if (executable) {
                return std::nullopt;
            }

            return "The .app file is not executable";
        }

        return "Failed to get loader for .app file";
    };

    // if the CIA is already installed, don't reinstall it
    const auto& maybe_installed_path = get_installed_path();
    if (FileUtil::Exists(maybe_installed_path)) {
        const auto& maybe_executable_result = is_executable(maybe_installed_path);
        if (maybe_executable_result) {
            return std::make_pair(false, *maybe_executable_result);
        }

        return std::make_pair(true, maybe_installed_path);
    }

    const auto install_result = Service::AM::InstallCIA(cia_path);
    switch (install_result) {
    case Service::AM::InstallStatus::ErrorFailedToOpenFile:
        return std::make_pair(false, "Failed to open .cia file!");
    case Service::AM::InstallStatus::ErrorFileNotFound:
        return std::make_pair(false, "The .cia file does not exist!");
    case Service::AM::InstallStatus::ErrorAborted:
        return std::make_pair(false, "The .cia file installation aborted!");
    case Service::AM::InstallStatus::ErrorInvalid:
        return std::make_pair(false, "The .cia file is invalid!");
    case Service::AM::InstallStatus::ErrorEncrypted:
        return std::make_pair(false,
                              "The game that you are trying to load must be decrypted before "
                              "being used with Encore.");
    case Service::AM::InstallStatus::Success:
        break; // Expected case
    default:
        return std::make_pair(false, "Unknown error occurred while installing .cia file");
    }

    const auto& installed_path = get_installed_path();
    const auto& executable_result = is_executable(installed_path);
    if (executable_result) {
        return std::make_pair(false, *executable_result);
    }

    return std::make_pair(true, installed_path);
}

std::optional<std::string> EncoreContext::LoadROM(const std::string& rom_path) {
    window->MakeCurrent();
    const auto load_result = system.Load(*window, rom_path);
    switch (load_result) {
    case Core::System::ResultStatus::ErrorGetLoader:
        return fmt::format("Failed to obtain loader for {} file!",
                           std::filesystem::path(rom_path).extension().string());
    case Core::System::ResultStatus::ErrorLoader:
        return "Failed to load ROM!";
    case Core::System::ResultStatus::ErrorLoader_ErrorEncrypted:
        return "The game that you are trying to load must be decrypted before "
               "being used with Encore.";
    case Core::System::ResultStatus::ErrorLoader_ErrorInvalidFormat:
        return "Error while loading ROM: The ROM format is not supported.";
    case Core::System::ResultStatus::ErrorNotInitialized:
        return "CPUCore not initialized";
    case Core::System::ResultStatus::ErrorSystemMode:
        return "Failed to determine system mode!";
    case Core::System::ResultStatus::Success:
        break; // Expected case
    default:
        return fmt::format("Error while loading ROM: {}", system.GetStatusDetails());
    }

    std::atomic_bool stop_run{};
    system.GPU().Renderer().Rasterizer()->LoadDiskResources(stop_run, [](auto, auto, auto) {});
    return std::nullopt;
}

void EncoreContext::RunFrame() {
    window->MakeCurrent();
    window->RunFrame();
    audio_resampler->Flush();
}

void EncoreContext::Reset() {
    window->MakeCurrent();
    system.Reset();
}

std::pair<u32, u32> EncoreContext::GetVideoVirtualDimensions() const {
    return window->GetVideoVirtualDimensions();
}

std::pair<u32, u32> EncoreContext::GetVideoBufferDimensions() const {
    return window->GetVideoBufferDimensions();
}

u32 EncoreContext::GetGLTexture() const {
    ASSERT(Settings::values.graphics_api.GetValue() == Settings::GraphicsAPI::OpenGL);
    return static_cast<EmuWindow_Headless_GL&>(*window).GetGLTexture();
}

void EncoreContext::ReadFrameBuffer(u32* dest_buffer) {
    window->MakeCurrent();
    return window->ReadFrameBuffer(dest_buffer);
}

std::span<const s16> EncoreContext::GetAudio() const {
    return audio_resampler->GetAudio();
}

void EncoreContext::ReloadConfig() const {
    config->Reload();
    window->ReloadConfig();
}

std::size_t EncoreContext::StartSaveState() {
    window->MakeCurrent();
    return savestate_mt->StartSaveState();
}

void EncoreContext::FinishSaveState(void* dest_buffer) {
    savestate_mt->FinishSaveState(dest_buffer);
}

void EncoreContext::LoadState(void* src_buffer, std::size_t buffer_len) const {
    window->MakeCurrent();
    savestate_mt->LoadState(src_buffer, buffer_len);
}

std::pair<const u8*, std::size_t> EncoreContext::GetMemoryRegion(Memory::Region region) const {
    const auto is_n3ds = Settings::values.is_new_3ds.GetValue();
    switch (region) {
    case Memory::Region::FCRAM:
        return std::make_pair(system.Memory().GetPhysicalPointer(Memory::FCRAM_PADDR),
                              is_n3ds ? Memory::FCRAM_N3DS_SIZE : Memory::FCRAM_SIZE);
    case Memory::Region::VRAM:
        return std::make_pair(system.Memory().GetPhysicalPointer(Memory::VRAM_PADDR),
                              Memory::VRAM_SIZE);
    case Memory::Region::DSP:
        return std::make_pair(system.Memory().GetPhysicalPointer(Memory::DSP_RAM_PADDR),
                              Memory::DSP_RAM_SIZE);
    case Memory::Region::N3DS:
        return std::make_pair(system.Memory().GetPhysicalPointer(Memory::N3DS_EXTRA_RAM_PADDR),
                              is_n3ds ? Memory::N3DS_EXTRA_RAM_SIZE : 0);
    default:
        UNREACHABLE();
    }
}

std::tuple<Common::Rectangle<u32>, bool, bool> EncoreContext::GetTouchScreenLayout() const {
    const auto& layout = window->GetFramebufferLayout();
    // keep in mind is_rotated is true if in "normal" orientation
    // as the 3DS displays in a rotated manner internally, which is rotated again for "normal"
    // orientation
    return std::make_tuple(layout.bottom_screen, !layout.is_rotated, layout.bottom_screen_enabled);
}
