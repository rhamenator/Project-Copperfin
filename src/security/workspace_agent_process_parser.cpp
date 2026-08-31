// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/workspace_agent_process_parser.h"

#include "copperfin/security/sha256.h"
#include "copperfin/security/workspace_agent_process_parser_test_hooks.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>

namespace copperfin::security {

namespace {

#if defined(COPPERFIN_ENABLE_WORKSPACE_AGENT_PROCESS_PARSER_TEST_HOOKS)
// See workspace_agent_process_parser_test_hooks.h. Relaxed ordering is
// sufficient: this exists only for single-threaded test setup/teardown
// around a call to capture_binding()/authorize_windows(), never for
// production synchronization.
std::atomic<void (*)()> pre_read_test_hook{nullptr};
#endif

bool path_has_embedded_nul(const std::filesystem::path& path) {
    const auto& native = path.native();
    return native.find(typename std::filesystem::path::value_type{}) !=
        std::filesystem::path::string_type::npos;
}

bool supported_contract(
    WorkspaceAgentProcessArgumentParserContract contract) noexcept {
    return contract ==
        WorkspaceAgentProcessArgumentParserContract::windows_c_runtime_argv_v1;
}

bool supported_dependency_contract(
    const WorkspaceAgentProcessParserDependencyContract contract) noexcept {
    return contract == WorkspaceAgentProcessParserDependencyContract::
        self_contained_launch_image_v1;
}

bool valid_sha256(const std::string_view digest) noexcept {
    return digest.size() == 64U &&
        std::all_of(digest.begin(), digest.end(), [](const char character) {
            return (character >= '0' && character <= '9') ||
                (character >= 'a' && character <= 'f');
        });
}

bool constant_time_equal(
    const std::string_view left,
    const std::string_view right) noexcept {
    if (left.size() != right.size()) {
        return false;
    }
    std::uint8_t difference = 0U;
    for (std::size_t index = 0U; index < left.size(); ++index) {
        difference |= static_cast<std::uint8_t>(left[index]) ^
            static_cast<std::uint8_t>(right[index]);
    }
    return difference == 0U;
}

struct CapturedExecutable {
    PhysicalPathContainmentResult containment;
    std::string sha256;
};

// Reads a trusted-executable snapshot from an already-verified handle and
// re-checks link_count against the fresh post-read identity before
// returning it, since read_physically_contained_file_snapshot_from_handle()'s
// own freshness check (content_equal()) deliberately excludes link_count
// (issue #5420) -- a hard link added to the executable between the
// pre-read containment check and this read completing would otherwise go
// undetected. Both capture_binding() and authorize_windows() call this
// single helper rather than each re-implementing the check, so there is
// exactly one place that can omit it -- an earlier version of this file's
// migration to the handle-based read did omit it independently in both
// functions, caught only by adversarial review (see CHANGELOG.md).
PhysicalFileSnapshotResult read_trusted_executable_snapshot(
    const PhysicalPathContainmentHandle& handle,
    const std::uint64_t maximum_bytes) {
#if defined(COPPERFIN_ENABLE_WORKSPACE_AGENT_PROCESS_PARSER_TEST_HOOKS)
    if (const auto hook =
            pre_read_test_hook.load(std::memory_order_relaxed);
        hook != nullptr) {
        pre_read_test_hook.store(nullptr, std::memory_order_relaxed);
        hook();
    }
#endif
    auto snapshot =
        read_physically_contained_file_snapshot_from_handle(handle, maximum_bytes);
    if (snapshot.ok && snapshot.containment.identity.link_count != 1U) {
        return PhysicalFileSnapshotResult{
            .ok = false,
            .bytes = {},
            .containment = snapshot.containment,
            .failure = PhysicalPathContainmentFailure::identity_changed};
    }
    return snapshot;
}

std::optional<CapturedExecutable> capture_binding(
    const WorkspaceAgentWindowsProcessParserBinding& binding) {
    const auto& path = binding.trusted_absolute_executable;
    if (!supported_contract(binding.contract) ||
        !supported_dependency_contract(binding.dependency_contract) ||
        !valid_sha256(binding.expected_sha256) || path.empty() ||
        path_has_embedded_nul(path) || !path.is_absolute() ||
        path.parent_path().empty()) {
        return std::nullopt;
    }
    // Uses the atomic check-and-open primitive (issue #5409/#5420) so the
    // read below is bound to the exact object just verified, never
    // reopened by path string. The is_regular_file() pre-check the
    // string-based version needed is no longer necessary: the handle-based
    // read already rejects a non-regular-file target internally (via the
    // same handle, not a further path-string resolution), and this
    // function's only observable outcome either way is std::nullopt.
    auto handle = inspect_and_open_physically_contained_path(path, path.parent_path());
    const auto& captured = handle.result();
    if (!captured.allowed || captured.identity.link_count != 1U ||
        captured.identity != binding.expected_identity) {
        return std::nullopt;
    }
    const auto snapshot = read_trusted_executable_snapshot(
        handle,
        workspace_agent_maximum_windows_process_parser_image_bytes);
    if (!snapshot.ok) {
        return std::nullopt;
    }
    const auto digest = sha256_hex_for_text(snapshot.bytes);
    if (!digest.ok ||
        !constant_time_equal(digest.hex_digest, binding.expected_sha256)) {
        return std::nullopt;
    }
    return CapturedExecutable{
        .containment = snapshot.containment,
        .sha256 = digest.hex_digest};
}

WorkspaceAgentProcessParserAuthorization denied(std::string diagnostic_code) {
    WorkspaceAgentProcessParserAuthorization result;
    result.diagnostic_code = std::move(diagnostic_code);
    return result;
}

}  // namespace

#if defined(COPPERFIN_ENABLE_WORKSPACE_AGENT_PROCESS_PARSER_TEST_HOOKS)
void set_workspace_agent_process_parser_pre_read_test_hook_for_testing(
    void (*hook)()) {
    pre_read_test_hook.store(hook, std::memory_order_relaxed);
}
#endif

WorkspaceAgentProcessParserBoundary::WorkspaceAgentProcessParserBoundary(
    std::vector<CapturedBinding> bindings)
    : bindings_(std::move(bindings)) {}

std::optional<WorkspaceAgentProcessParserBoundary>
WorkspaceAgentProcessParserBoundary::create(
    const WorkspaceAgentProcessParserConfiguration& configuration) {
    if (configuration.schema_version != 1U ||
        configuration.windows_bindings.empty() ||
        configuration.windows_bindings.size() >
            workspace_agent_maximum_windows_process_parser_bindings) {
        return std::nullopt;
    }

    std::vector<CapturedBinding> captured_bindings;
    captured_bindings.reserve(configuration.windows_bindings.size());
    for (const auto& binding : configuration.windows_bindings) {
        const auto captured = capture_binding(binding);
        if (!captured.has_value()) {
            return std::nullopt;
        }
        const bool duplicate = std::any_of(
            captured_bindings.begin(),
            captured_bindings.end(),
            [&](const CapturedBinding& prior) {
                return prior.canonical_executable ==
                        captured->containment.canonical_path ||
                    (prior.identity.storage_id ==
                         captured->containment.identity.storage_id &&
                     prior.identity.file_id ==
                         captured->containment.identity.file_id);
            });
        if (duplicate) {
            return std::nullopt;
        }
        captured_bindings.push_back({
            .canonical_executable = captured->containment.canonical_path,
            .identity = captured->containment.identity,
            .sha256 = captured->sha256,
            .contract = binding.contract});
    }
    return WorkspaceAgentProcessParserBoundary(std::move(captured_bindings));
}

WorkspaceAgentProcessParserAuthorization
WorkspaceAgentProcessParserBoundary::authorize_windows(
    const std::filesystem::path& canonical_executable,
    const PhysicalPathIdentity& executable_identity) const {
    const auto binding = std::find_if(
        bindings_.begin(), bindings_.end(), [&](const CapturedBinding& candidate) {
            return candidate.canonical_executable == canonical_executable;
        });
    if (binding == bindings_.end() ||
        binding->identity != executable_identity) {
        return denied("workspace_agent.process_argument_parser_not_trusted");
    }

    // Atomic check-and-open primitive (issue #5409/#5420): the read below
    // is bound to the exact object this walk verified, never reopened by
    // path string.
    auto handle = inspect_and_open_physically_contained_path(
        binding->canonical_executable,
        binding->canonical_executable.parent_path());
    const auto& current = handle.result();
    if (!current.allowed || current.canonical_path != binding->canonical_executable ||
        current.identity != binding->identity) {
        return denied("workspace_agent.process_argument_parser_identity_changed");
    }

    const auto snapshot = read_trusted_executable_snapshot(
        handle,
        workspace_agent_maximum_windows_process_parser_image_bytes);
    if (!snapshot.ok) {
        return denied("workspace_agent.process_argument_parser_identity_changed");
    }
    const auto digest = sha256_hex_for_text(snapshot.bytes);
    if (!digest.ok || !constant_time_equal(digest.hex_digest, binding->sha256)) {
        return denied("workspace_agent.process_argument_parser_contents_changed");
    }

    return {
        .allowed = true,
        .contract = binding->contract,
        .diagnostic_code = "workspace_agent.process_argument_parser_allowed"};
}

}  // namespace copperfin::security
