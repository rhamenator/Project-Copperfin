// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "copperfin/security/security_model.h"

#include <string>

namespace copperfin::security {

[[nodiscard]] bool role_has_permission(
    const NativeSecurityProfile& profile,
    const std::string& role_id,
    const std::string& permission_id);

}  // namespace copperfin::security
