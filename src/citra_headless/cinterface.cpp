// Copyright 2023 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "citra_context.h"

#ifdef _WIN32
#define CITRA_EXPORT extern "C" __declspec(dllexport)
#else
#define CITRA_EXPORT extern "C" __attribute__((visibility("default")))
#endif

using namespace Headless;

CITRA_EXPORT CitraContext* Citra_CreateContext(ConfigCallbackInterface* config_interface,
                                               GLCallbackInterface* gl_interface,
                                               InputCallbackInterface* input_interface) {
    return new CitraContext(*config_interface, *gl_interface, *input_interface);
}

CITRA_EXPORT void Citra_DestroyContext(CitraContext* context) {
    delete context;
}

CITRA_EXPORT bool Citra_InstallCIA(CitraContext* context, const char* cia_path, bool force,
                                   char* string_buffer, u32 string_size) {
    const auto& result = context->InstallCIA(cia_path, force);
    const auto& msg = std::get<std::string>(result);
    auto len = std::min(msg.length(), static_cast<std::size_t>(string_size - 1));
    std::memcpy(string_buffer, msg.c_str(), len);
    string_buffer[len] = '\0';
    return std::get<bool>(result);
}

CITRA_EXPORT bool Citra_LoadROM(CitraContext* context, const char* rom_path,
                                char* error_message_buffer, u32 error_message_buffer_size) {
    const auto& error_message = context->LoadROM(rom_path);
    if (error_message) {
        auto len = std::min(error_message->length(),
                            static_cast<std::size_t>(error_message_buffer_size - 1));
        std::memcpy(error_message_buffer, error_message->c_str(), len);
        error_message_buffer[len] = '\0';
        return false;
    }

    return true;
}

CITRA_EXPORT void Citra_RunFrame(CitraContext* context) {
    context->RunFrame();
}

CITRA_EXPORT void Citra_Reset(CitraContext* context) {
    context->Reset();
}

CITRA_EXPORT void Citra_GetVideoDimensions(CitraContext* context, u32* x, u32* y) {
    const auto& video_dimensions = context->GetVideoDimensions();
    *x = std::get<0>(video_dimensions);
    *y = std::get<1>(video_dimensions);
}

CITRA_EXPORT u32 Citra_GetGLTexture(CitraContext* context) {
    return context->GetGLTexture();
}

CITRA_EXPORT void Citra_ReadFrameBuffer(CitraContext* context, u32* dest_buffer) {
    context->ReadFrameBuffer(dest_buffer);
}

CITRA_EXPORT void Citra_GetAudio(CitraContext* context, const s16** buffer, u32* frames) {
    const auto& audio = context->GetAudio();
    *buffer = audio.data();
    *frames = static_cast<u32>(audio.size());
}

CITRA_EXPORT void Citra_ReloadConfig(CitraContext* context) {
    context->ReloadConfig();
}

CITRA_EXPORT u32 Citra_StartSaveState(CitraContext* context) {
    return static_cast<u32>(context->StartSaveState());
}

CITRA_EXPORT void Citra_FinishSaveState(CitraContext* context, void* dest_buffer) {
    context->FinishSaveState(dest_buffer);
}

CITRA_EXPORT void Citra_LoadState(CitraContext* context, void* src_buffer, u32 buffer_len) {
    context->LoadState(src_buffer, buffer_len);
}
