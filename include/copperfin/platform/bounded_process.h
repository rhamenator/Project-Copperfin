// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace copperfin::platform {

enum class BoundedProcessStatus {
    exited,
    cancelled,
    timed_out,
    output_limit_exceeded,
    invalid_request,
    launch_failed
};

struct BoundedProcessEnvironmentVariable {
    std::string name;
    std::string value;
};

struct BoundedProcessRequest {
    std::string executable_path;
    std::vector<std::string> arguments;
    std::string working_directory;
    // Complete child environment. The default is intentionally empty; ambient
    // agent/build secrets are never inherited implicitly.
    std::vector<BoundedProcessEnvironmentVariable> environment;
    // Exact request bytes delivered to child stdin. Empty input closes stdin
    // immediately. The limit must be positive and cannot exceed the fixed
    // implementation hard ceiling.
    std::string standard_input{};
    std::uint32_t timeout_ms = 5000U;
    std::uint32_t poll_interval_ms = 10U;
    std::uint32_t stdin_limit_bytes = 1024U * 1024U;
    // Output is captured as exact bytes. Limits must be positive and cannot
    // exceed the same implementation hard ceiling.
    std::uint32_t stdout_limit_bytes = 1024U * 1024U;
    std::uint32_t stderr_limit_bytes = 1024U * 1024U;
    // Must return promptly. true, or an exception, fails closed as cancellation.
    std::function<bool()> cancellation_requested;
};

struct BoundedProcessResult {
    BoundedProcessStatus status = BoundedProcessStatus::invalid_request;
    bool started = false;
    bool process_tree_closed = false;
    int exit_code = -1;
    int native_error = 0;
    std::uint64_t elapsed_ms = 0U;
    // Windows Job Objects can report a reliable owned-job peak. Generic POSIX
    // wait4() rusage is deliberately not exposed here because fork() can seed
    // the child's high-water mark with the launcher's pre-exec resident set.
    bool peak_memory_available = false;
    std::uint64_t peak_memory_kib = 0U;
    std::string error_code = "polyglot.process.invalid_request";
    std::string standard_output;
    std::string standard_error;

    [[nodiscard]] bool completed() const noexcept {
        return status == BoundedProcessStatus::exited;
    }
};

// Starts an absolute executable directly without a shell, polls cancellation
// while it runs, delivers exact stdin and captures stdout/stderr under fixed
// byte ceilings, and owns one process group/job. Paths and arguments containing
// NUL are rejected. Closing the invocation always removes descendants so an
// artifact cannot leave helpers behind after returning.
[[nodiscard]] BoundedProcessResult run_bounded_process(
    const BoundedProcessRequest& request);

[[nodiscard]] const char* bounded_process_status_name(
    BoundedProcessStatus status) noexcept;

}  // namespace copperfin::platform
