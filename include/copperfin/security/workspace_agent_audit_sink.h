// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/security/workspace_agent_session.h"

#include <cstddef>
#include <filesystem>

namespace copperfin::security {

// Governing requirements: RQ-CF-AGENT-001, RQ-CF-AGENT-005,
// RQ-CF-AGENT-006, RQ-CF-AGENT-021, and RQ-CF-AGENT-028.

inline constexpr std::size_t workspace_agent_audit_min_log_bytes = 512U;
inline constexpr std::size_t workspace_agent_audit_default_log_bytes = 4U * 1024U * 1024U;
inline constexpr std::size_t workspace_agent_audit_max_log_bytes = 64U * 1024U * 1024U;

class WorkspaceAgentSessionAuditFileSink final {
public:
    WorkspaceAgentSessionAuditFileSink(
        const std::filesystem::path& storage_root,
        const std::filesystem::path& relative_log_path,
        std::size_t max_log_bytes = workspace_agent_audit_default_log_bytes);

    WorkspaceAgentSessionAuditFileSink(const WorkspaceAgentSessionAuditFileSink&) = delete;
    WorkspaceAgentSessionAuditFileSink& operator=(const WorkspaceAgentSessionAuditFileSink&) = delete;
    WorkspaceAgentSessionAuditFileSink(WorkspaceAgentSessionAuditFileSink&&) = delete;
    WorkspaceAgentSessionAuditFileSink& operator=(WorkspaceAgentSessionAuditFileSink&&) = delete;

    [[nodiscard]] bool ready() const noexcept;
    [[nodiscard]] std::filesystem::path log_path() const;

    // The returned callback refers to this instance. The sink object must
    // outlive every controller call that uses the callback.
    [[nodiscard]] WorkspaceAgentSessionAuditSink session_sink() noexcept;

private:
    static WorkspaceAgentSessionAuditCommitResult commit_thunk(
        const WorkspaceAgentSessionAuditEvent& event,
        void* context);
    [[nodiscard]] WorkspaceAgentSessionAuditCommitResult commit(
        const WorkspaceAgentSessionAuditEvent& event) const;

    bool ready_ = false;
    std::filesystem::path storage_root_;
    std::filesystem::path log_path_;
    std::size_t max_log_bytes_ = 0U;
};

}  // namespace copperfin::security
