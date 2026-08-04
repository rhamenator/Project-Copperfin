// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/localization/localization.h"

#include <string>
#include <string_view>

namespace copperfin::security {

std::string security_text(
    std::string_view key,
    const localization::PlaceholderMap& placeholders = {});

}  // namespace copperfin::security
