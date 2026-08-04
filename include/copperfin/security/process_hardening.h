// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <string>

namespace copperfin::security {

struct ProcessHardeningStatus {
    bool applied = false;
    std::string message;
};

[[nodiscard]] ProcessHardeningStatus apply_default_process_hardening();

}  // namespace copperfin::security
