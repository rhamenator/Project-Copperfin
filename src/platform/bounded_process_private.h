// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/platform/bounded_process.h"
#include "copperfin/platform/private_executable_image.h"

#include <filesystem>
#include <string>
#include <vector>

namespace copperfin::platform {

struct PrivateWindowsBoundedProcessRequest {
    std::u16string command_line;
    std::u16string environment_block;
    std::filesystem::path working_directory;
    BoundedProcessRequest transport;
    void (*launch_committed)(void*) noexcept = nullptr;
    void* launch_committed_context = nullptr;
};

struct PrivatePosixBoundedProcessRequest {
    // Already-serialized argv (including argv[0]) and envp ("NAME=value")
    // entries, exactly as produced by the trusted plan/parser boundary --
    // this seam does not re-derive or re-parse them.
    std::vector<std::string> arguments;
    std::vector<std::string> environment;
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
// exact image and its internally retained plan. Non-Windows callers of this
// specific overload fail closed; see run_bounded_posix_private_executable for
// the Linux/macOS counterpart.
[[nodiscard]] BoundedProcessResult run_bounded_windows_private_executable(
    const PrivateExecutableImage& image,
    const PrivateWindowsBoundedProcessRequest& request) noexcept;

// POSIX counterpart: execs the image's retained descriptor directly
// (fexecve on Linux; execve("/dev/fd/N", ...) on macOS, which lacks
// fexecve) instead of a path, reusing the same fork/pipe/thread/waitpid
// supervision as the plain POSIX run_bounded_process path. Windows callers
// of this overload fail closed.
[[nodiscard]] BoundedProcessResult run_bounded_posix_private_executable(
    const PrivateExecutableImage& image,
    const PrivatePosixBoundedProcessRequest& request) noexcept;

// Bridges run_posix()'s (bounded_process.cpp, anonymous-namespace) forked-
// child exec step to PrivateExecutableImage::posix_exec_in_child() without
// exposing any native path through that class's header surface: `context`
// must be the address of the same PrivateExecutableImage supplied to
// run_bounded_posix_private_executable() for this launch. Matches the
// bool (*)(void*, char* const[], char* const[]) noexcept exec-override
// callback shape run_posix() invokes in the forked child immediately
// before exec, in place of its default execve(request.executable_path).
[[nodiscard]] bool posix_private_exec_override(
    void* context, char* const argv[], char* const environment[]) noexcept;

[[nodiscard]] CurrentProcessElevation current_process_elevation() noexcept;

}  // namespace copperfin::platform
