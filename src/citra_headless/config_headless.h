// Copyright 2023 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <string>

#include "common/settings.h"
#include "core/core.h"

namespace Headless {

struct ConfigCallbackInterface {
    bool (*GetBoolean)(const char* label);
    u64 (*GetInteger)(const char* label);
    double (*GetFloat)(const char* label);
    void (*GetString)(const char* label, char* buffer, u32 buffer_size);
};

class Config_Headless {
public:
    Config_Headless(Core::System& system, ConfigCallbackInterface& callbacks);
    ~Config_Headless();

    void Reload();
    u32 GetWindowScaleFactor() const;

private:
    template <typename Type, bool ranged>
    void ReadSetting(Settings::Setting<Type, ranged>& setting);

    void LoadConstantSettings();
    void LoadSyncSettings();
    void LoadNonSyncSettings();

    Core::System& system;
    ConfigCallbackInterface callbacks;
    u32 window_scale_factor;
};

} // namespace Headless
