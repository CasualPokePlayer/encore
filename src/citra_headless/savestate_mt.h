// Copyright 2023 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>

#include <zstd.h>

#include "core/core.h"

namespace Headless {

class Savestate_MT {
public:
    explicit Savestate_MT(Core::System& system);
    ~Savestate_MT();

    std::size_t StartSaveState();
    void FinishSaveState(void* dest_buffer);
    void LoadState(void* src_buffer, std::size_t buffer_len);

private:
    Core::System& system;
    std::shared_ptr<u8[]> state_buffer;
    std::vector<u8> cur_state;
    ZSTD_CStream* cstream;
    ZSTD_DStream* dstream;
};

} // namespace Headless
