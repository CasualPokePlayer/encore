// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include "audio_core/null_sink.h"
#include "audio_core/sink_details.h"
#include "common/logging/log.h"

namespace AudioCore {
namespace {
// sink_details is ordered in terms of desirability, with the best choice at the top.
constexpr std::array sink_details = {
    SinkDetails{SinkType::Null, "None",
                [](std::string_view device_id) -> std::unique_ptr<Sink> {
                    return std::make_unique<NullSink>(device_id);
                },
                [] { return std::vector<std::string>{"None"}; }},
};
} // Anonymous namespace

std::vector<SinkDetails> ListSinks() {
    return {sink_details.begin(), sink_details.end()};
}

const SinkDetails& GetSinkDetails(SinkType sink_type) {
    auto iter = std::find_if(
        sink_details.begin(), sink_details.end(),
        [sink_type](const auto& sink_detail) { return sink_detail.type == sink_type; });

    if (sink_type == SinkType::Auto || iter == sink_details.end()) {
        if (sink_type != SinkType::Auto) {
            LOG_ERROR(Audio, "AudioCore::GetSinkDetails given invalid sink_type {}", sink_type);
        }
        // Auto-select.
        // sink_details is ordered in terms of desirability, with the best choice at the front.
        iter = sink_details.begin();
    }

    return *iter;
}

} // namespace AudioCore
