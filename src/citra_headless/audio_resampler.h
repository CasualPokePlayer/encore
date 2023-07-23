// Copyright 2023 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "blip_buf/blip_buf.h"

#include "core/core.h"

namespace Headless {

class AudioResampler {
public:
    explicit AudioResampler(Core::System& system);
    ~AudioResampler();

    void Flush();
    std::span<const s16> GetAudio();

private:
    Core::System& system;
    std::vector<s16> in_buffer, out_buffer;
    blip_t* blip_l;
    blip_t* blip_r;
    s16 latch_l{0}, latch_r{0};
};

} // namespace Headless
