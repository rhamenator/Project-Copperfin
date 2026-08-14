// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/security/workspace_agent_policy.h"

#include <cstdint>
#include <mutex>
#include <string>

namespace copperfin::security {

// Governing requirements: RQ-CF-AGENT-001, RQ-CF-AGENT-005, and
// RQ-CF-AGENT-007.

enum class WorkspaceAgentSessionEventKind {
    start,
    stop
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

// Reuse the policy capability shape so requirements cannot acquire a parallel
// vocabulary that drifts from the admitted session snapshot.
using WorkspaceAgentToolRequirements = WorkspaceAgentCapabilities;

struct WorkspaceAgentToolPreflightRequest {
    std::uint32_t schema_version = 1U;
    std::uint64_t session_generation = 0U;
    // A trusted executor must derive this complete set from its registered
    // tool definition. Provider/model input must not select these fields.
    WorkspaceAgentToolRequirements requirements{};
};

struct WorkspaceAgentToolPreflightResult {
    bool allowed = false;
    std::uint64_t session_generation = 0U;
    WorkspaceAgentAccessMode effective_mode = WorkspaceAgentAccessMode::advisory;
    std::string diagnostic_code;
};

class WorkspaceAgentSessionController {
public:
    [[nodiscard]] WorkspaceAgentSessionStartResult start(
        const WorkspaceAgentActivationRequest& request,
        const WorkspaceAgentSessionAuditSink& audit_sink);
    [[nodiscard]] WorkspaceAgentSessionStopResult stop(
        const WorkspaceAgentSessionAuditSink& audit_sink);
    [[nodiscard]] WorkspaceAgentSessionSnapshot snapshot() const;

    // This is a point-in-time, non-executing preflight, not a reusable
    // authority token. A future executor must submit the complete capability
    // set again immediately beside each controlled side effect and must audit
    // the actual tool outcome separately.
    [[nodiscard]] WorkspaceAgentToolPreflightResult preflight_tool_request(
        const WorkspaceAgentToolPreflightRequest& request) const;

private:
    enum class Transition {
        idle,
        starting,
        stopping
    };

    mutable std::mutex mutex_;
    Transition transition_ = Transition::idle;
    std::uint64_t next_generation_ = 1U;
    WorkspaceAgentSessionSnapshot active_session_{};
};

[[nodiscard]] std::string serialize_workspace_agent_session_audit_event(
    const WorkspaceAgentSessionAuditEvent& event);

}  // namespace copperfin::security
