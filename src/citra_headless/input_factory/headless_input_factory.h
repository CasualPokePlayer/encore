// Copyright 2023 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "common/common_types.h"

namespace Headless {

using GetButtonCallback = bool (*)(u32);
using GetAxisCallback = void (*)(u32, float*, float*);
using GetTouchCallback = bool (*)(float*, float*);
using GetMotionCallback = void (*)(float*, float*, float*, float*, float*, float*);

struct InputCallbackInterface {
    GetButtonCallback GetButton;
    GetAxisCallback GetAxis;
    GetTouchCallback GetTouch;
    GetMotionCallback GetMotion;
};

} // namespace Headless
