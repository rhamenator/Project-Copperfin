// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <string>

#include "copperfin/licensing/license_status.h"
#include "license_payload_value.h"

namespace copperfin::licensing {

// Classifies an already signature-verified license payload. Split out from
// the file-loading/verification orchestration in license_status.cpp so unit
// tests can exercise every date/version boundary deterministically, without
// needing to fake the system clock or a real signed file for each case.
[[nodiscard]] LicenseStatus classify_verified_payload(
    const PayloadFields& fields,
    int current_major_version,
    const std::string& current_date_iso8601);

}  // namespace copperfin::licensing
