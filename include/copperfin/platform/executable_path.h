// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace copperfin::platform {

[[nodiscard]] std::optional<std::string> default_executable_search_path();

[[nodiscard]] std::filesystem::path resolve_executable_invocation_path(
    const std::filesystem::path& invocation_path);

[[nodiscard]] std::filesystem::path resolve_running_executable_path(
    const std::filesystem::path& fallback_invocation_path = {});

}  // namespace copperfin::platform
