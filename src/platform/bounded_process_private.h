// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/platform/bounded_process.h"
#include "copperfin/platform/private_executable_image.h"

#include <filesystem>
#include <string>

namespace copperfin::platform {

struct PrivateWindowsBoundedProcessRequest {
    std::u16string command_line;
    std::u16string environment_block;
    std::filesystem::path working_directory;
    BoundedProcessRequest transport;
    void (*launch_committed)(void*) noexcept = nullptr;
    void* launch_committed_context = nullptr;
};

enum class CurrentProcessElevation {
    not_elevated,
    elevated,
    unavailable,
    unsupported
};

// Trusted implementation seam. The security controller supplies one opaque
// exact image and its internally retained plan. Non-Windows hosts fail closed.
[[nodiscard]] BoundedProcessResult run_bounded_windows_private_executable(
    const PrivateExecutableImage& image,
    const PrivateWindowsBoundedProcessRequest& request) noexcept;

[[nodiscard]] CurrentProcessElevation current_process_elevation() noexcept;

}  // namespace copperfin::platform
