// Copyright 2024 Encore Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "audio_core/audio_types.h"
#include "audio_core/dsp_interface.h"
#include "audio_resampler.h"

using namespace Headless;

constexpr auto OUT_SAMPLE_RATE = 44100;

AudioResampler::AudioResampler(Core::System& system_) : system(system_) {
    blip_l = blip_new(1024 * 2);
    blip_set_rates(blip_l, AudioCore::native_sample_rate, OUT_SAMPLE_RATE);
    blip_r = blip_new(1024 * 2);
    blip_set_rates(blip_r, AudioCore::native_sample_rate, OUT_SAMPLE_RATE);
}

AudioResampler::~AudioResampler() {
    blip_delete(blip_l);
    blip_delete(blip_r);
}

void AudioResampler::Flush() {
    auto& fifo = system.DSP().GetFifo();
    in_buffer.resize(fifo.Size() * 2);
    fifo.Pop(in_buffer.data());
    const auto num_samples = in_buffer.size() / 2;
    for (unsigned i = 0; i < num_samples; i++) {
        s16 sample_l = in_buffer[i * 2 + 0];
        if (sample_l != latch_l) {
            blip_add_delta(blip_l, i, latch_l - sample_l);
            latch_l = sample_l;
        }

        s16 sample_r = in_buffer[i * 2 + 1];
        if (sample_r != latch_r) {
            blip_add_delta(blip_r, i, latch_r - sample_r);
            latch_r = sample_r;
        }
    }

    blip_end_frame(blip_l, static_cast<unsigned>(num_samples));
    blip_end_frame(blip_r, static_cast<unsigned>(num_samples));

    const auto num_out_samples = blip_samples_avail(blip_l);
    ASSERT(num_out_samples == blip_samples_avail(blip_r));
    out_buffer.resize(num_out_samples * 2);
    blip_read_samples(blip_l, out_buffer.data() + 0, num_out_samples, true);
    blip_read_samples(blip_r, out_buffer.data() + 1, num_out_samples, true);
}

std::span<const s16> AudioResampler::GetAudio() {
    return out_buffer;
}
