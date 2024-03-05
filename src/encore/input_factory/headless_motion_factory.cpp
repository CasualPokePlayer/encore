// Copyright 2024 Encore Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "headless_motion_factory.h"

namespace Headless {

class HeadlessMotion;

class HeadlessMotion final : public Input::MotionDevice {
public:
    explicit HeadlessMotion(GetMotionCallback callback_) : callback(callback_) {}

    std::tuple<Common::Vec3<float>, Common::Vec3<float>> GetStatus() const override {
        Common::Vec3<float> accel, gyro;
        callback(&accel.x, &accel.y, &accel.z, &gyro.x, &gyro.y, &gyro.z);
        return std::make_tuple(accel, gyro);
    }

private:
    GetMotionCallback callback;
};

} // namespace Headless

using namespace Headless;

HeadlessMotionFactory::HeadlessMotionFactory(InputCallbackInterface& input_interface)
    : callback(input_interface.GetMotion) {}

std::unique_ptr<Input::MotionDevice> HeadlessMotionFactory::Create(
    const Common::ParamPackage& params) {
    return std::make_unique<HeadlessMotion>(callback);
}
