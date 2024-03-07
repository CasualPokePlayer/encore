// Copyright 2024 Encore Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <codecvt>
#include <locale>

#include "common/file_util.h"
#include "common/settings.h"
#include "core/hle/service/cfg/cfg.h"
#include "core/hle/service/ptm/ptm.h"
#include "core/hle/service/service.h"

#include "config_headless.h"

using namespace Headless;

Config_Headless::Config_Headless(Core::System& system_, ConfigCallbackInterface& callbacks_)
    : system(system_), callbacks(callbacks_) {
    ASSERT(!system.IsPoweredOn());
    LoadConstantSettings();
    LoadSyncSettings();
    Reload();
}

Config_Headless::~Config_Headless() = default;

void Config_Headless::Reload() {
    LoadNonSyncSettings();
    system.ApplySettings();
}

template <>
void Config_Headless::ReadSetting(Settings::Setting<std::string>& setting) {
    char buffer[4096]{};
    callbacks.GetString(setting.GetLabel().c_str(), buffer, sizeof(buffer));
    setting = buffer;
}

template <>
void Config_Headless::ReadSetting(Settings::Setting<bool>& setting) {
    setting = callbacks.GetBoolean(setting.GetLabel().c_str());
}

template <typename Type, bool ranged>
void Config_Headless::ReadSetting(Settings::Setting<Type, ranged>& setting) {
    if constexpr (std::is_floating_point_v<Type>) {
        setting = static_cast<Type>(callbacks.GetFloat(setting.GetLabel().c_str()));
    } else {
        setting = static_cast<Type>(callbacks.GetInteger(setting.GetLabel().c_str()));
    }
}

// we don't want these changing regardless of the frontend
void Config_Headless::LoadConstantSettings() {
    // Controls
    Settings::values.current_input_profile.name = "Headless";

    for (int i = 0; i < Settings::NativeButton::NumButtons; ++i) {
        Settings::values.current_input_profile.buttons[i] =
            fmt::format("engine:headless,button:{}", i);
    }

    for (int i = 0; i < Settings::NativeAnalog::NumAnalogs; ++i) {
        Settings::values.current_input_profile.analogs[i] =
            fmt::format("engine:headless,axis:{}", i);
    }

    Settings::values.current_input_profile.motion_device = "engine:headless";
    Settings::values.current_input_profile.touch_device = "engine:headless";

    Settings::values.current_input_profile.use_touch_from_button = false;
    Settings::values.current_input_profile.touch_from_button_map_index = 0;

    Settings::values.current_input_profile_index = 0;
    Settings::values.input_profiles.clear();
    Settings::values.input_profiles.push_back(Settings::values.current_input_profile);
    Settings::values.touch_from_button_maps.clear();

    // Renderer
    Settings::values.physical_device =
        0; // doesn't mean anything outside of Vulkan (not yet supported)
    Settings::values.spirv_shader_gen = true; // ditto
    Settings::values.async_presentation =
        false; // only allow presenting on the main thread (doesn't make sense otherwise)
    Settings::values.use_gles = false;             // only standard OpenGL supported for now
    Settings::values.use_disk_shader_cache = true; // no need to expose this to the user
    Settings::values.frame_limit = 0;              // unthrottled (frontend handles this)
    Settings::values.use_vsync_new = false;        // frontend handles this

    // not sure what these are, probably don't want to expose them
    Settings::values.pp_shader_name = "none (builtin)";
    Settings::values.anaglyph_shader_name = "dubois (builtin)";

    // Utility
    // not going to support these for now
    Settings::values.dump_textures = false;
    Settings::values.custom_textures = false;
    Settings::values.preload_textures = false;
    Settings::values.async_custom_loading = false;

    // Audio
    Settings::values.audio_emulation =
        Settings::AudioEmulation::HLE;                // only HLE audio supports savestates
    Settings::values.enable_audio_stretching = false; // handled frontend side
    Settings::values.volume = 1;
    Settings::values.output_type =
        AudioCore::SinkType::Null; // we use a different interface for this
    Settings::values.output_device = "None";
    // not sure about this
    // Settings::values.input_type = AudioCore::InputType::Static;
    // Settings::values.input_device = "Static Noise";
    Settings::values.input_type = AudioCore::InputType::Null;
    Settings::values.input_device = "None";

    // Data Storage
    Settings::values.use_custom_storage = false; // we'll control this with the user directory

    // System
    Settings::values.init_time_offset = 0; // offset to real time?

    // Camera
    for (int i = 0; i < Service::CAM::NumCameras; ++i) {
        // TODO image?
        Settings::values.camera_name[i] = "blank";
        Settings::values.camera_config[i] = "";
        Settings::values.camera_flip[i] = 0;
    }

    // Debugging
    Settings::values.record_frame_times = false;
    Settings::values.renderer_debug = false;
    Settings::values.use_gdbstub = false;
    Settings::values.gdbstub_port = 0;

    // TODO make this changeable
    for (const auto& service_module : Service::service_module_map) {
        Settings::values.lle_modules.emplace(service_module.name, false);
    }

    // Video Dumping
    Settings::values.output_format = "";
    Settings::values.format_options = "";

    Settings::values.video_encoder = "";
    Settings::values.video_encoder_options = "";
    Settings::values.video_bitrate = 0;

    Settings::values.audio_encoder = "";
    Settings::values.audio_encoder_options = "";
    Settings::values.audio_bitrate = 0;

    // Miscellaneous
    Settings::values.log_filter = "";
}

void Config_Headless::LoadSyncSettings() {
    // Core
    ReadSetting(Settings::values.use_cpu_jit);
    ReadSetting(Settings::values.cpu_clock_percentage);

    // Renderer
    ReadSetting(Settings::values.graphics_api);
    ReadSetting(Settings::values.async_shader_compilation);
    ReadSetting(Settings::values.use_hw_shader);
    ReadSetting(Settings::values.shaders_accurate_mul);
    ReadSetting(Settings::values.use_shader_jit);

    // Audio
    ReadSetting(Settings::values.volume);

    // Data Storage
    ReadSetting(Settings::values.use_virtual_sd);

    char user_directory_path_buffer[4096]{};
    callbacks.GetString("user_directory", user_directory_path_buffer,
                        sizeof(user_directory_path_buffer));
    FileUtil::ResetUserPath();
    FileUtil::SetUserPath(user_directory_path_buffer);

    // System
    ReadSetting(Settings::values.is_new_3ds);
    ReadSetting(Settings::values.lle_applets);
    ReadSetting(Settings::values.region_value);
    ReadSetting(Settings::values.init_clock);
    ReadSetting(Settings::values.init_time);
    ReadSetting(Settings::values.init_ticks_type);
    ReadSetting(Settings::values.init_ticks_override);
    ReadSetting(Settings::values.plugin_loader_enabled);
    ReadSetting(Settings::values.allow_plugin_loader);

    // CFG
    const auto cfg = Service::CFG::GetModule(system);

    char username_buffer[11]{};
    callbacks.GetString("username", username_buffer, sizeof(username_buffer));
    cfg->SetUsername(std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(
        username_buffer));
    cfg->SetBirthday(static_cast<u8>(callbacks.GetInteger("birthmonth")),
                     static_cast<u8>(callbacks.GetInteger("birthday")));
    cfg->SetSystemLanguage(
        static_cast<Service::CFG::SystemLanguage>(callbacks.GetInteger("language")));
    cfg->SetSoundOutputMode(
        static_cast<Service::CFG::SoundOutputMode>(callbacks.GetInteger("sound_mode")));
    cfg->UpdateConfigNANDSavegame();

    // PTM
    Service::PTM::Module::SetPlayCoins(static_cast<u16>(callbacks.GetInteger("playcoins")));
}

void Config_Headless::LoadNonSyncSettings() {
    // Renderer
    ReadSetting(Settings::values.resolution_factor);
    ReadSetting(Settings::values.texture_filter);
    ReadSetting(Settings::values.texture_sampling);

    ReadSetting(Settings::values.mono_render_option);
    ReadSetting(Settings::values.render_3d);
    ReadSetting(Settings::values.factor_3d);
    ReadSetting(Settings::values.filter_mode);

    ReadSetting(Settings::values.bg_red);
    ReadSetting(Settings::values.bg_green);
    ReadSetting(Settings::values.bg_blue);

    // Layout
    ReadSetting(Settings::values.layout_option);
    ReadSetting(Settings::values.swap_screen);
    ReadSetting(Settings::values.upright_screen);
    ReadSetting(Settings::values.large_screen_proportion);
    ReadSetting(Settings::values.custom_layout);
    ReadSetting(Settings::values.custom_top_left);
    ReadSetting(Settings::values.custom_top_top);
    ReadSetting(Settings::values.custom_top_right);
    ReadSetting(Settings::values.custom_top_bottom);
    ReadSetting(Settings::values.custom_bottom_left);
    ReadSetting(Settings::values.custom_bottom_top);
    ReadSetting(Settings::values.custom_bottom_right);
    ReadSetting(Settings::values.custom_bottom_bottom);
    ReadSetting(Settings::values.custom_second_layer_opacity);
}
