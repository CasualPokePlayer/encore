// Copyright 2023 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "headless_axis_factory.h"

namespace Headless {

class HeadlessAxis;

class HeadlessAxis final : public Input::AnalogDevice {
public:
    explicit HeadlessAxis(GetAxisCallback callback_, u32 axis_)
        : callback(callback_), axis(axis_) {}

    std::tuple<float, float> GetStatus() const override {
        float x, y;
        callback(axis, &x, &y);
        return std::make_tuple(x, y);
    }

private:
    GetAxisCallback callback;
    u32 axis;
};

} // namespace Headless

using namespace Headless;

HeadlessAxisFactory::HeadlessAxisFactory(InputCallbackInterface& input_interface)
    : callback(input_interface.GetAxis) {}

std::unique_ptr<Input::AnalogDevice> HeadlessAxisFactory::Create(
    const Common::ParamPackage& params) {
    const u32 axis = params.Get("axis", 0);
    return std::make_unique<HeadlessAxis>(callback, axis);
}
