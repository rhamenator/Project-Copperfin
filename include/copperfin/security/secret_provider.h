// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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
