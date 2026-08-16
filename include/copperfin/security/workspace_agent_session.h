// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/security/workspace_agent_policy.h"
#include "copperfin/security/workspace_agent_environment.h"
#include "copperfin/security/workspace_agent_process_containment.h"
#include "copperfin/security/workspace_agent_process_parser.h"
#include "copperfin/security/workspace_agent_target_containment.h"
#include "copperfin/security/workspace_agent_tool_registry.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace copperfin::security {

inline constexpr std::size_t
    workspace_agent_session_max_pending_layout_cleanups = 64U;

// Governing requirements: RQ-CF-AGENT-001, RQ-CF-AGENT-005, and
// RQ-CF-AGENT-007, RQ-CF-AGENT-009, RQ-CF-AGENT-010, and
// RQ-CF-AGENT-011, RQ-CF-AGENT-012, RQ-CF-AGENT-013,
// RQ-CF-AGENT-014, RQ-CF-AGENT-015, RQ-CF-AGENT-016, and candidate
// RQ-CF-AGENT-018 through RQ-CF-AGENT-023.

enum class WorkspaceAgentSessionEventKind {
    start,
    stop,
    layout_cleanup_intent,
    layout_cleanup_outcome
};

struct WorkspaceAgentSessionAuditEvent {
    std::uint32_t schema_version = 1U;
    WorkspaceAgentSessionEventKind kind = WorkspaceAgentSessionEventKind::start;
    std::uint64_t session_generation = 0U;
    WorkspaceAgentAccessMode requested_mode = WorkspaceAgentAccessMode::advisory;
    WorkspaceAgentAccessMode effective_mode = WorkspaceAgentAccessMode::advisory;
    std::string outcome;
    std::string diagnostic_code;
};

struct WorkspaceAgentSessionAuditCommitResult {
    bool ok = false;
    std::string receipt;
};

using WorkspaceAgentSessionAuditCommitFunction = WorkspaceAgentSessionAuditCommitResult (*)(
    const WorkspaceAgentSessionAuditEvent& event,
    void* context);

struct WorkspaceAgentSessionAuditSink {
    WorkspaceAgentSessionAuditCommitFunction commit = nullptr;
    void* context = nullptr;
};

struct WorkspaceAgentSessionSnapshot {
    bool active = false;
    std::uint64_t generation = 0U;
    WorkspaceAgentAccessMode effective_mode = WorkspaceAgentAccessMode::advisory;
    WorkspaceAgentCapabilities capabilities{};
    std::string activation_audit_receipt;
};

struct WorkspaceAgentSessionStartResult {
    bool activated = false;
    bool audit_committed = false;
    std::string audit_receipt;
    std::string diagnostic_code;
    WorkspaceAgentActivationDecision policy_decision{};
    WorkspaceAgentSessionSnapshot session{};
};

struct WorkspaceAgentSessionStopResult {
    bool revoked = false;
    bool audit_committed = false;
    std::string audit_receipt;
    std::string diagnostic_code;
    WorkspaceAgentSessionSnapshot session{};
};

struct WorkspaceAgentSessionLayoutCleanupAttemptResult {
    bool attempted = false;
    bool cleaned = false;
    bool intent_audit_committed = false;
    bool outcome_audit_committed = false;
    std::uint64_t session_generation = 0U;
    std::string intent_audit_receipt;
    std::string outcome_audit_receipt;
    std::string diagnostic_code;
};

struct WorkspaceAgentToolPreflightRequest {
    std::uint32_t schema_version = 1U;
    std::uint64_t session_generation = 0U;
    // Exact lookup in the immutable product registry supplies the complete
    // capability set. Provider/model input cannot declare capability fields.
    std::string tool_id;
};

struct WorkspaceAgentToolPreflightResult {
    bool allowed = false;
    std::uint64_t session_generation = 0U;
    WorkspaceAgentAccessMode effective_mode = WorkspaceAgentAccessMode::advisory;
    std::string tool_id;
    std::string diagnostic_code;
};

struct WorkspaceAgentFileTargetPreflightRequest {
    std::uint32_t schema_version = 1U;
    std::uint64_t session_generation = 0U;
    std::string tool_id;
    std::filesystem::path target_path;
};

struct WorkspaceAgentFileTargetPreflightResult {
    bool allowed = false;
    std::uint64_t session_generation = 0U;
    WorkspaceAgentAccessMode effective_mode = WorkspaceAgentAccessMode::advisory;
    std::string tool_id;
    std::filesystem::path canonical_path;
    PhysicalPathIdentity identity{};
    std::string diagnostic_code;
};

struct WorkspaceAgentProcessTargetPreflightRequest {
    std::uint32_t schema_version = 1U;
    std::uint64_t session_generation = 0U;
    std::string tool_id;
    std::filesystem::path executable_path;
    std::filesystem::path working_directory;
};

struct WorkspaceAgentProcessTargetPreflightResult {
    bool allowed = false;
    std::uint64_t session_generation = 0U;
    WorkspaceAgentAccessMode effective_mode = WorkspaceAgentAccessMode::advisory;
    std::string tool_id;
    std::filesystem::path canonical_executable_path;
    PhysicalPathIdentity executable_identity{};
    std::filesystem::path canonical_working_directory;
    PhysicalPathIdentity working_directory_identity{};
    std::string diagnostic_code;
};

inline constexpr std::size_t workspace_agent_process_max_argument_count = 64U;
inline constexpr std::size_t workspace_agent_process_max_argument_bytes = 4096U;
inline constexpr std::size_t workspace_agent_process_max_total_argument_bytes =
    8192U;

// Arguments exclude argv[0], which a future executor must derive from the
// revalidated canonical executable. They are direct argument elements, not a
// command line or shell fragment. No caller-selected environment is accepted.
struct WorkspaceAgentProcessInvocationPreflightRequest {
    std::uint32_t schema_version = 1U;
    std::uint64_t session_generation = 0U;
    std::string tool_id;
    std::filesystem::path executable_path;
    std::filesystem::path working_directory;
    std::vector<std::string> arguments;
};

struct WorkspaceAgentProcessInvocationPreflightResult {
    bool allowed = false;
    std::uint64_t session_generation = 0U;
    WorkspaceAgentAccessMode effective_mode = WorkspaceAgentAccessMode::advisory;
    std::string tool_id;
    std::filesystem::path canonical_executable_path;
    PhysicalPathIdentity executable_identity{};
    std::filesystem::path canonical_working_directory;
    PhysicalPathIdentity working_directory_identity{};
    std::vector<std::string> arguments;
    WorkspaceAgentProcessEnvironmentPolicy environment_policy =
        WorkspaceAgentProcessEnvironmentPolicy::isolated_session_v1;
    std::string diagnostic_code;
};

struct WorkspaceAgentProcessEnvironmentPreflightResult {
    bool allowed = false;
    std::uint64_t session_generation = 0U;
    WorkspaceAgentAccessMode effective_mode = WorkspaceAgentAccessMode::advisory;
    std::string tool_id;
    std::filesystem::path canonical_executable_path;
    PhysicalPathIdentity executable_identity{};
    std::filesystem::path canonical_working_directory;
    PhysicalPathIdentity working_directory_identity{};
    std::vector<std::string> arguments;
    WorkspaceAgentProcessEnvironmentPolicy environment_policy =
        WorkspaceAgentProcessEnvironmentPolicy::isolated_session_v1;
    WorkspaceAgentProcessEnvironmentPlatform environment_platform =
        workspace_agent_process_environment_host_platform();
    std::vector<WorkspaceAgentEnvironmentEntry> environment_entries;
    std::string diagnostic_code;
};

struct WorkspaceAgentSerializedProcessEnvironmentPreflightResult {
    bool allowed = false;
    // The complete logical plan is retained so serialization stays bound to
    // the exact point-in-time invocation and fixed environment it consumed.
    WorkspaceAgentProcessEnvironmentPreflightResult environment_plan{};
    // Exactly one native representation is populated on allow.
    std::vector<std::string> posix_environment;
    std::u16string windows_environment_block;
    std::string diagnostic_code;
};

struct WorkspaceAgentSerializedProcessInvocationPreflightResult {
    bool allowed = false;
    // The complete fixed environment and logical invocation remain attached to
    // the platform argument representation they bracketed.
    WorkspaceAgentSerializedProcessEnvironmentPreflightResult
        serialized_environment{};
    // Exactly one argument representation is populated on allow. POSIX
    // includes argv[0]; Windows includes the complete CreateProcessW command
    // line without its implicit terminating NUL.
    std::vector<std::string> posix_arguments;
    std::u16string windows_command_line;
    WorkspaceAgentProcessArgumentParserContract argument_parser_contract =
        WorkspaceAgentProcessArgumentParserContract::none;
    std::string diagnostic_code;
};

struct WorkspaceAgentLaunchRevalidationResult {
    bool allowed = false;
    std::string diagnostic_code;
};

struct WorkspaceAgentSessionRevocationLeaseState;

// A short-lived, non-executing lease that prevents stop from completing while
// a future executor performs one launch decision for the exact generation.
// This is not launch authority and exposes no native process or target handle.
class WorkspaceAgentSessionRevocationLease {
public:
    WorkspaceAgentSessionRevocationLease() = default;
    ~WorkspaceAgentSessionRevocationLease();
    WorkspaceAgentSessionRevocationLease(
        WorkspaceAgentSessionRevocationLease&&) noexcept;
    WorkspaceAgentSessionRevocationLease& operator=(
        WorkspaceAgentSessionRevocationLease&&) noexcept;
    WorkspaceAgentSessionRevocationLease(
        const WorkspaceAgentSessionRevocationLease&) = delete;
    WorkspaceAgentSessionRevocationLease& operator=(
        const WorkspaceAgentSessionRevocationLease&) = delete;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] std::uint64_t session_generation() const noexcept;

private:
    class Impl;
    explicit WorkspaceAgentSessionRevocationLease(std::unique_ptr<Impl> impl);

    std::unique_ptr<Impl> impl_;

    friend class WorkspaceAgentSessionController;
};

struct WorkspaceAgentSessionRevocationLeaseResult {
    bool acquired = false;
    std::optional<WorkspaceAgentSessionRevocationLease> lease;
    std::string diagnostic_code;
};

struct WorkspaceAgentProcessTargetPinPreflightResult {
    bool pinned = false;
    std::uint64_t session_generation = 0U;
    std::optional<WorkspaceAgentProcessTargetPins> pins;
    std::string diagnostic_code;
};

class WorkspaceAgentSessionController {
public:
    WorkspaceAgentSessionController() = default;
    explicit WorkspaceAgentSessionController(
        const std::filesystem::path& trusted_absolute_workspace_root);
    WorkspaceAgentSessionController(
        const std::filesystem::path& trusted_absolute_workspace_root,
        const WorkspaceAgentIsolatedEnvironmentConfiguration&
            trusted_environment_configuration);
    WorkspaceAgentSessionController(
        const std::filesystem::path& trusted_absolute_workspace_root,
        const WorkspaceAgentIsolatedEnvironmentConfiguration&
            trusted_environment_configuration,
        const WorkspaceAgentProcessParserConfiguration&
            trusted_process_parser_configuration);

    [[nodiscard]] WorkspaceAgentSessionStartResult start(
        const WorkspaceAgentActivationRequest& request,
        const WorkspaceAgentSessionAuditSink& audit_sink);
    [[nodiscard]] WorkspaceAgentSessionStopResult stop(
        const WorkspaceAgentSessionAuditSink& audit_sink);
    // Attempts cleanup of the oldest controller-retained preparation receipt.
    // Authority must already be revoked. A durable intent record is required
    // before mutation, and failed cleanup retains the receipt for an explicit
    // retry. This does not run automatically during stop or process teardown.
    [[nodiscard]] WorkspaceAgentSessionLayoutCleanupAttemptResult
    cleanup_pending_session_layout(
        const WorkspaceAgentSessionAuditSink& audit_sink);
    [[nodiscard]] WorkspaceAgentSessionSnapshot snapshot() const;

    // This is a point-in-time, non-executing preflight, not a reusable
    // authority token. A future executor must submit the registered tool id
    // again immediately beside each controlled side effect and must apply
    // target containment and audit the actual tool outcome separately.
    [[nodiscard]] WorkspaceAgentToolPreflightResult preflight_tool_request(
        const WorkspaceAgentToolPreflightRequest& request) const;

    // Existing-file target inspection is still a point-in-time, non-executing
    // preflight. A future executor must repeat session, target, identity, and
    // operation checks while holding an OS-backed handle beside the side
    // effect; this result is never an authority token.
    [[nodiscard]] WorkspaceAgentFileTargetPreflightResult
    preflight_file_target_request(
        const WorkspaceAgentFileTargetPreflightRequest& request) const;

    // Process-target inspection is a point-in-time, non-executing preflight.
    // It performs no PATH search, command parsing, environment construction, or
    // launch. A future executor must repeat the complete check beside launch,
    // pin targets with platform-backed handles where possible, apply the real
    // sandbox, and audit the actual outcome.
    [[nodiscard]] WorkspaceAgentProcessTargetPreflightResult
    preflight_process_target_request(
        const WorkspaceAgentProcessTargetPreflightRequest& request) const;

    // Invocation preflight adds only a bounded direct argument vector and a
    // mandatory non-inheriting environment profile to the process-target
    // result. It does not serialize a platform command line, construct an
    // environment, apply a sandbox, read an executable, or start a process.
    [[nodiscard]] WorkspaceAgentProcessInvocationPreflightResult
    preflight_process_invocation_request(
        const WorkspaceAgentProcessInvocationPreflightRequest& request) const;

    // This adds a concrete fixed-key isolated environment to a repeated
    // invocation preflight. The same request type deliberately exposes no
    // environment field. Construction reads no ambient variables and still
    // performs no command serialization, directory mutation, or launch.
    [[nodiscard]] WorkspaceAgentProcessEnvironmentPreflightResult
    preflight_process_environment_request(
        const WorkspaceAgentProcessInvocationPreflightRequest& request) const;

    // Serializes only the fixed logical environment returned by a bracketed
    // environment preflight. It reads no ambient state, accepts no caller
    // environment, constructs no command line, and starts no process.
    [[nodiscard]] WorkspaceAgentSerializedProcessEnvironmentPreflightResult
    preflight_serialized_process_environment_request(
        const WorkspaceAgentProcessInvocationPreflightRequest& request) const;

    // Serializes the canonical executable as argv[0] and the admitted direct
    // argument elements for the host platform, bracketed by complete repeated
    // environment preflights. Windows additionally requires trusted-host parser
    // authority bound to the exact canonical executable identity. It performs
    // no shell interpretation, sandbox operation, directory mutation, or
    // process launch.
    [[nodiscard]] WorkspaceAgentSerializedProcessInvocationPreflightResult
    preflight_serialized_process_invocation_request(
        const WorkspaceAgentProcessInvocationPreflightRequest& request) const;

    // Explicit fail-closed promotion gate. Point-in-time plans cannot become
    // launch authority until a future executor retains the trusted containment
    // root, executable and working-directory pins, and a revocation lease
    // through the launch decision. Version 1 therefore always denies without
    // returning or reflecting any part of either input.
    [[nodiscard]] WorkspaceAgentLaunchRevalidationResult
    revalidate_serialized_process_invocation_for_launch(
        const WorkspaceAgentProcessInvocationPreflightRequest& request,
        const WorkspaceAgentSerializedProcessInvocationPreflightResult&
            admitted_plan) const;

    // Acquires only the revocation-lifetime prerequisite for a future direct
    // launcher. While held, stop waits before revoking the exact generation.
    // Callers must keep this lease only around the eventual launch syscall.
    // It does not validate or return a plan, pin paths, apply a sandbox, start
    // a process, or make the launch-promotion gate allow.
    [[nodiscard]] WorkspaceAgentSessionRevocationLeaseResult
    acquire_process_launch_revocation_lease(
        std::uint64_t session_generation) const;

    // Repeats exact-session process-target admission, then consumes a private
    // one-attempt inspection authority to retain the original trusted root,
    // executable, and working-directory objects. The opaque move-only result
    // exposes no path or native handle. It neither acquires the separate
    // revocation lease nor weakens the invariant launch-promotion denial.
    [[nodiscard]] WorkspaceAgentProcessTargetPinPreflightResult
    pin_process_target_request(
        const WorkspaceAgentProcessTargetPreflightRequest& request) const;

private:
    enum class Transition {
        idle,
        starting,
        stopping,
        cleaning
    };

    mutable std::mutex mutex_;
    Transition transition_ = Transition::idle;
    std::uint64_t next_generation_ = 1U;
    WorkspaceAgentSessionSnapshot active_session_{};
    std::shared_ptr<WorkspaceAgentSessionRevocationLeaseState>
        active_revocation_lease_state_;
    std::vector<WorkspaceAgentSessionLayoutPreparationResult>
        pending_layout_cleanups_;
    std::optional<WorkspaceAgentFileTargetBoundary> file_target_boundary_;
    std::optional<WorkspaceAgentProcessTargetBoundary> process_target_boundary_;
    std::optional<WorkspaceAgentIsolatedEnvironmentBoundary>
        process_environment_boundary_;
    std::optional<WorkspaceAgentProcessParserBoundary>
        process_parser_boundary_;
    bool process_environment_configuration_supplied_ = false;
};

[[nodiscard]] std::string serialize_workspace_agent_session_audit_event(
    const WorkspaceAgentSessionAuditEvent& event);

}  // namespace copperfin::security
