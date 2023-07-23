// Copyright 2023 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>

#include "core/frontend/input.h"
#include "headless_input_factory.h"

namespace Headless {

class HeadlessAxisFactory final : public Input::Factory<Input::AnalogDevice> {
public:
    explicit HeadlessAxisFactory(InputCallbackInterface& input_interface);

    std::unique_ptr<Input::AnalogDevice> Create(const Common::ParamPackage& params) override;

private:
    GetAxisCallback callback;
};

} // namespace Headless
