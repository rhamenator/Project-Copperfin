// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include <filesystem>

namespace copperfin::platform {

[[nodiscard]] std::filesystem::path resolve_executable_invocation_path(
    const std::filesystem::path& invocation_path);

[[nodiscard]] std::filesystem::path resolve_running_executable_path(
    const std::filesystem::path& fallback_invocation_path = {});

}  // namespace copperfin::platform
