// Copyright 2023 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "savestate_mt.h"

namespace Headless {

class SaveBuf : public std::streambuf {
public:
    std::atomic_size_t buffer_pos{0};

    explicit SaveBuf(std::shared_ptr<u8[]> buffer_) : buffer(std::move(buffer_)) {}

protected:
    std::streamsize xsputn(const char_type* s, std::streamsize count) override {
        const auto pos = buffer_pos.load(std::memory_order_relaxed);
        std::memcpy(&buffer[pos], s, count);
        buffer_pos.store(pos + count, std::memory_order_relaxed);
        return count;
    }

private:
    std::shared_ptr<u8[]> buffer;
};

class LoadBuf : public std::streambuf {
public:
    std::atomic_size_t buffer_filled{0};
    std::atomic_bool buffer_full{false};

    explicit LoadBuf(std::shared_ptr<u8[]> buffer_) : buffer(std::move(buffer_)) {}

protected:
    std::streamsize xsgetn(char_type* s, std::streamsize count) override {
        while (!buffer_full.load(std::memory_order_relaxed)) [[unlikely]] {
            const auto filled = buffer_filled.load(std::memory_order_relaxed);
            const auto buffer_avail = filled - buffer_pos;
            if (buffer_avail >= static_cast<std::size_t>(count)) {
                std::memcpy(s, &buffer[buffer_pos], count);
                buffer_pos += count;
                return count;
            }
        }

        const std::streamsize buffer_avail =
            buffer_filled.load(std::memory_order_relaxed) - buffer_pos;
        count = std::min(buffer_avail, count);
        std::memcpy(s, &buffer[buffer_pos], count);
        buffer_pos += count;
        return count;
    }

private:
    std::shared_ptr<u8[]> buffer;
    std::size_t buffer_pos{0};
};

} // namespace Headless

using namespace Headless;

constexpr auto ONE_MiB = 0x100000u;
constexpr auto FOUR_MiB = ONE_MiB * 4;
// (uncompressed) states should never be more than 300 MiB
constexpr auto MAX_UNCOMPRESSED_STATE_SIZE = ONE_MiB * 300;
// (compressed) states are usually a bit less than 32MiB, but never can be sure...
constexpr auto STARTING_COMPRESSED_STATE_SIZE = ONE_MiB * 32;

Savestate_MT::Savestate_MT(Core::System& system_) : system(system_) {
    state_buffer = std::make_shared_for_overwrite<u8[]>(MAX_UNCOMPRESSED_STATE_SIZE);
    cur_state.reserve(STARTING_COMPRESSED_STATE_SIZE);
    cstream = ZSTD_createCStream();
    dstream = ZSTD_createDStream();
}

Savestate_MT::~Savestate_MT() {
    ZSTD_freeCStream(cstream);
    ZSTD_freeDStream(dstream);
}

std::size_t Savestate_MT::StartSaveState() {
    SaveBuf save_buf(state_buffer);
    std::atomic_bool state_done{false};

    std::thread compression_thread([&]() {
        ZSTD_initCStream(cstream, ZSTD_fast);
        ZSTD_CCtx_setParameter(cstream, ZSTD_c_nbWorkers, std::thread::hardware_concurrency() / 2);

        ZSTD_inBuffer in_buf = {
            .src = state_buffer.get(),
            .size = 0,
            .pos = 0,
        };

        cur_state.resize(STARTING_COMPRESSED_STATE_SIZE);
        ZSTD_outBuffer out_buf = {
            .dst = cur_state.data(),
            .size = STARTING_COMPRESSED_STATE_SIZE,
            .pos = 0,
        };

        while (!state_done.load(std::memory_order_relaxed)) {
            const auto pos = save_buf.buffer_pos.load(std::memory_order_relaxed);
            if (in_buf.pos < pos) {
                in_buf.size = pos;
                ZSTD_compressStream2(cstream, &out_buf, &in_buf, ZSTD_e_continue);
                if (out_buf.pos == out_buf.size) {
                    out_buf.size += FOUR_MiB;
                    cur_state.resize(out_buf.size);
                    out_buf.dst = cur_state.data();
                }
            }
        }

        in_buf.size = save_buf.buffer_pos.load(std::memory_order_relaxed);
        while (ZSTD_compressStream2(cstream, &out_buf, &in_buf, ZSTD_e_end) != 0) {
            out_buf.size += FOUR_MiB;
            cur_state.resize(out_buf.size);
            out_buf.dst = cur_state.data();
        }

        cur_state.resize(out_buf.pos);
    });

    oarchive oa{save_buf, boost::archive::archive_flags::no_header |
                              boost::archive::archive_flags::no_codecvt};
    oa& system;

    state_done.store(true, std::memory_order_relaxed);
    compression_thread.join();
    return cur_state.size();
}

void Savestate_MT::FinishSaveState(void* dest_buffer) {
    std::memcpy(dest_buffer, cur_state.data(), cur_state.size());
}

void Savestate_MT::LoadState(void* src_buffer, std::size_t buffer_len) {
    LoadBuf load_buf(state_buffer);

    std::thread decompression_thread([&]() {
        ZSTD_initDStream(dstream);
        ZSTD_outBuffer out_buf = {
            .dst = state_buffer.get(),
            .size = MAX_UNCOMPRESSED_STATE_SIZE,
            .pos = 0,
        };

        ZSTD_inBuffer in_buf = {
            .src = src_buffer,
            .size = std::min(static_cast<std::size_t>(ONE_MiB), buffer_len),
            .pos = 0,
        };

        ZSTD_decompressStream(dstream, &out_buf, &in_buf);
        load_buf.buffer_filled.store(out_buf.pos, std::memory_order_relaxed);
        in_buf.size = std::min(in_buf.size + FOUR_MiB, buffer_len);

        while (in_buf.size != buffer_len) {
            ZSTD_decompressStream(dstream, &out_buf, &in_buf);
            load_buf.buffer_filled.store(out_buf.pos, std::memory_order_relaxed);
            in_buf.size = std::min(in_buf.size + FOUR_MiB, buffer_len);
        }

        while (in_buf.pos != buffer_len) {
            ZSTD_decompressStream(dstream, &out_buf, &in_buf);
            load_buf.buffer_filled.store(out_buf.pos, std::memory_order_relaxed);
        }

        load_buf.buffer_filled.store(out_buf.pos, std::memory_order_relaxed);
        load_buf.buffer_full.store(true, std::memory_order_relaxed);
    });

    iarchive ia{load_buf, boost::archive::archive_flags::no_header |
                              boost::archive::archive_flags::no_codecvt};
    ia& system;

    decompression_thread.join(); // should be a no-op in practice
}
