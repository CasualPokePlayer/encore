// Copyright 2023 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>

#include "core/frontend/input.h"
#include "headless_input_factory.h"

namespace Headless {

class HeadlessMotionFactory final : public Input::Factory<Input::MotionDevice> {
public:
    explicit HeadlessMotionFactory(InputCallbackInterface& input_interface);

    std::unique_ptr<Input::MotionDevice> Create(const Common::ParamPackage& params) override;

private:
    GetMotionCallback callback;
};

} // namespace Headless
