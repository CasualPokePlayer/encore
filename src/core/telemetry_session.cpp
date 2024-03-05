// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <cryptopp/osrng.h>

#include "common/assert.h"
#include "common/common_types.h"
#include "common/file_util.h"
#include "common/logging/log.h"
#include "common/scm_rev.h"
#include "common/settings.h"
#include "core/core.h"
#include "core/loader/loader.h"
#include "core/telemetry_session.h"

namespace Core {

namespace Telemetry = Common::Telemetry;

u64 GetTelemetryId() {
    return 0;
}

u64 RegenerateTelemetryId() {
    return 0;
}

bool VerifyLogin(const std::string& username, const std::string& token) {
    return false;
}

TelemetrySession::TelemetrySession() = default;

TelemetrySession::~TelemetrySession() {}

void TelemetrySession::AddInitialInfo(Loader::AppLoader& app_loader) {}

bool TelemetrySession::SubmitTestcase() {
    return false;
}

} // namespace Core
