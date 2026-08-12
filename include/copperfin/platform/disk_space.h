// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>

namespace copperfin::platform {

[[nodiscard]] std::optional<std::uintmax_t> available_disk_bytes(
    const std::filesystem::path& path);

[[nodiscard]] std::optional<std::uintmax_t> disk_allocation_unit_bytes(
    const std::filesystem::path& path);

}  // namespace copperfin::platform
