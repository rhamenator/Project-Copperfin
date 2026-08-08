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
    std::uint32_t timeout_ms = 5000U;
    std::uint32_t poll_interval_ms = 10U;
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
    std::string error_code = "polyglot.process.invalid_request";

    [[nodiscard]] bool completed() const noexcept {
        return status == BoundedProcessStatus::exited;
    }
};

// Starts an absolute executable directly without a shell, polls cancellation
// while it runs, and owns one process group/job. Paths and arguments containing
// NUL are rejected. Closing the invocation always removes descendants so an
// artifact cannot leave helpers behind after returning.
[[nodiscard]] BoundedProcessResult run_bounded_process(
    const BoundedProcessRequest& request);

[[nodiscard]] const char* bounded_process_status_name(
    BoundedProcessStatus status) noexcept;

}  // namespace copperfin::platform
