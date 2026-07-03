// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include <string>

namespace copperfin::security {

struct ProcessHardeningStatus {
    bool applied = false;
    std::string message;
};

[[nodiscard]] ProcessHardeningStatus apply_default_process_hardening();

}  // namespace copperfin::security
