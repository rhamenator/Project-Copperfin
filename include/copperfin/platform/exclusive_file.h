// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <filesystem>
#include <string_view>

namespace copperfin::platform {

// Creates one new durable file without replacing an existing filesystem entry.
// The caller owns parent-directory creation and cleanup policy.
bool write_new_durable_file(
    const std::filesystem::path& path,
    std::string_view bytes);

}  // namespace copperfin::platform
