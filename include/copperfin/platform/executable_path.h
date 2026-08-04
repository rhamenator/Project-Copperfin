// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace copperfin::platform {

#if !defined(_WIN32)
[[nodiscard]] std::optional<std::string> default_posix_search_path();
#endif

[[nodiscard]] std::filesystem::path resolve_executable_invocation_path(
    const std::filesystem::path& invocation_path);

[[nodiscard]] std::filesystem::path resolve_running_executable_path(
    const std::filesystem::path& fallback_invocation_path = {});

}  // namespace copperfin::platform
