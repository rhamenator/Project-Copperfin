// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <string>

namespace copperfin::security {

struct SecretResolveResult {
    bool ok = false;
    std::string value;
    std::string error;
};

[[nodiscard]] SecretResolveResult resolve_secret_reference(const std::string& reference);

}  // namespace copperfin::security
