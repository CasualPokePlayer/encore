// Copyright 2024 Encore Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "headless_button_factory.h"

namespace Headless {

class HeadlessButton;

class HeadlessButton final : public Input::ButtonDevice {
public:
    explicit HeadlessButton(GetButtonCallback callback_, u32 button_)
        : callback(callback_), button(button_) {}

    bool GetStatus() const override {
        return callback(button);
    }

private:
    GetButtonCallback callback;
    u32 button;
};

} // namespace Headless

using namespace Headless;

HeadlessButtonFactory::HeadlessButtonFactory(InputCallbackInterface& input_interface)
    : callback(input_interface.GetButton) {}

std::unique_ptr<Input::ButtonDevice> HeadlessButtonFactory::Create(
    const Common::ParamPackage& params) {
    const u32 button = params.Get("button", 0);
    return std::make_unique<HeadlessButton>(callback, button);
}
