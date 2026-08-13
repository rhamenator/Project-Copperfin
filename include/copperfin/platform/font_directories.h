// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <filesystem>
#include <vector>

namespace copperfin::platform {

// Returns the ordered native directories searched for installed font files.
// Callers own enumeration, font-name interpretation, filtering, and fallback.
std::vector<std::filesystem::path> font_search_directories();

}  // namespace copperfin::platform
