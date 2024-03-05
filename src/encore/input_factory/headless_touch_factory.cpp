// Copyright 2024 Encore Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "headless_touch_factory.h"

namespace Headless {

class HeadlessTouch;

class HeadlessTouch final : public Input::TouchDevice {
public:
    explicit HeadlessTouch(GetTouchCallback callback_) : callback(callback_) {}

    std::tuple<float, float, bool> GetStatus() const override {
        float x, y;
        const auto touching = callback(&x, &y);
        return std::make_tuple(x, y, touching);
    }

private:
    GetTouchCallback callback;
};

} // namespace Headless

using namespace Headless;

HeadlessTouchFactory::HeadlessTouchFactory(InputCallbackInterface& input_interface)
    : callback(input_interface.GetTouch) {}

std::unique_ptr<Input::TouchDevice> HeadlessTouchFactory::Create(
    const Common::ParamPackage& params) {
    return std::make_unique<HeadlessTouch>(callback);
}
