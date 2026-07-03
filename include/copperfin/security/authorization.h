// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include "copperfin/security/security_model.h"

#include <string>

namespace copperfin::security {

[[nodiscard]] bool role_has_permission(
    const NativeSecurityProfile& profile,
    const std::string& role_id,
    const std::string& permission_id);

}  // namespace copperfin::security
