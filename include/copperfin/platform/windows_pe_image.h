// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <filesystem>

namespace copperfin::platform {

enum class WindowsPeImageStatus {
    unreadable,
    invalid,
    executable,
    dynamic_library
};

enum class WindowsPeMachine {
    unknown,
    x86,
    x64,
    arm64
};

enum class WindowsPeSubsystem {
    unsupported,
    windows_gui,
    windows_console
};

struct WindowsPeImageInspection {
    WindowsPeImageStatus status = WindowsPeImageStatus::unreadable;
    WindowsPeMachine machine = WindowsPeMachine::unknown;
    WindowsPeSubsystem subsystem = WindowsPeSubsystem::unsupported;
    bool pe32_plus = false;
    bool managed = false;
    bool clr_directory_slot_present = false;
};

// Parses the loader-relevant PE headers without executing or mapping the image.
// The result is portable so synthetic format regressions can run on every host.
[[nodiscard]] WindowsPeImageInspection inspect_windows_pe_image(
    const std::filesystem::path& path) noexcept;

// Returns the native Windows host architecture. Non-Windows and unsupported
// Windows architectures return unknown and therefore fail compatibility.
[[nodiscard]] WindowsPeMachine native_windows_pe_host_machine() noexcept;

// This deliberately models only directly supported combinations: x86 on x86,
// x86 or x64 on x64, and arm64 on arm64. Emulation not represented here fails
// closed until it has direct platform evidence.
[[nodiscard]] bool windows_pe_image_is_launch_compatible(
    const WindowsPeImageInspection& image,
    WindowsPeMachine host_machine) noexcept;

}  // namespace copperfin::platform
