// Copyright 2024 Encore Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "encore_context.h"

#ifdef _WIN32
#define ENCORE_EXPORT extern "C" __declspec(dllexport)
#else
#define ENCORE_EXPORT extern "C" __attribute__((visibility("default")))
#endif

using namespace Headless;

ENCORE_EXPORT EncoreContext* Encore_CreateContext(ConfigCallbackInterface* config_interface,
                                                  GLCallbackInterface* gl_interface,
                                                  InputCallbackInterface* input_interface) {
    return new EncoreContext(*config_interface, *gl_interface, *input_interface);
}

ENCORE_EXPORT void Encore_DestroyContext(EncoreContext* context) {
    delete context;
}

ENCORE_EXPORT bool Encore_InstallCIA(EncoreContext* context, const char* cia_path,
                                     char* string_buffer, u32 string_size) {
    const auto& result = context->InstallCIA(cia_path);
    const auto& msg = std::get<std::string>(result);
    auto len = std::min(msg.length(), static_cast<std::size_t>(string_size - 1));
    std::memcpy(string_buffer, msg.c_str(), len);
    string_buffer[len] = '\0';
    return std::get<bool>(result);
}

ENCORE_EXPORT bool Encore_LoadROM(EncoreContext* context, const char* rom_path,
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

ENCORE_EXPORT bool Encore_RunFrame(EncoreContext* context) {
    return context->RunFrame();
}

ENCORE_EXPORT void Encore_Reset(EncoreContext* context) {
    context->Reset();
}

ENCORE_EXPORT void Encore_GetVideoBufferDimensions(EncoreContext* context, u32* w, u32* h) {
    const auto& buffer_dimensions = context->GetVideoBufferDimensions();
    *w = std::get<0>(buffer_dimensions);
    *h = std::get<1>(buffer_dimensions);
}

ENCORE_EXPORT u32 Encore_GetGLTexture(EncoreContext* context) {
    return context->GetGLTexture();
}

ENCORE_EXPORT void Encore_ReadFrameBuffer(EncoreContext* context, u32* dest_buffer) {
    context->ReadFrameBuffer(dest_buffer);
}

ENCORE_EXPORT void Encore_GetAudio(EncoreContext* context, const s16** buffer, u32* frames) {
    const auto& audio = context->GetAudio();
    *buffer = audio.data();
    *frames = static_cast<u32>(audio.size());
}

ENCORE_EXPORT void Encore_ReloadConfig(EncoreContext* context) {
    context->ReloadConfig();
}

ENCORE_EXPORT u32 Encore_StartSaveState(EncoreContext* context) {
    return static_cast<u32>(context->StartSaveState());
}

ENCORE_EXPORT void Encore_FinishSaveState(EncoreContext* context, void* dest_buffer) {
    context->FinishSaveState(dest_buffer);
}

ENCORE_EXPORT void Encore_LoadState(EncoreContext* context, void* src_buffer, u32 buffer_len) {
    context->LoadState(src_buffer, buffer_len);
}

ENCORE_EXPORT void Encore_GetMemoryRegion(EncoreContext* context, u32 region, const u8** ptr,
                                          u32* size) {
    const auto& memory_region = context->GetMemoryRegion(static_cast<Memory::Region>(region));
    *ptr = std::get<const u8*>(memory_region);
    *size = static_cast<u32>(std::get<std::size_t>(memory_region));
}

ENCORE_EXPORT const u8* Encore_GetPagePointer(EncoreContext* context, u32 addr) {
    return context->GetPagePointer(addr);
}

ENCORE_EXPORT void Encore_GetTouchScreenLayout(EncoreContext* context, u32* x, u32* y, u32* width,
                                               u32* height, bool* rotated, bool* enabled) {
    const auto& touch_screen_layout = context->GetTouchScreenLayout();
    const auto& touch_screen_rect = std::get<Common::Rectangle<u32>>(touch_screen_layout);
    *x = touch_screen_rect.left;
    *y = touch_screen_rect.top;
    *width = touch_screen_rect.GetWidth();
    *height = touch_screen_rect.GetHeight();
    *rotated = std::get<1>(touch_screen_layout);
    *enabled = std::get<2>(touch_screen_layout);
}
