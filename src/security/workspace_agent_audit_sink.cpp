// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/workspace_agent_audit_sink.h"

#include "copperfin/platform/path.h"
#include "copperfin/security/audit_stream.h"

#include <limits>
#include <string_view>
#include <system_error>

namespace copperfin::security {

namespace {

constexpr std::string_view workspace_agent_session_audit_event_name =
    "workspace_agent.session.v1";

bool relative_log_path_is_safe(const std::filesystem::path& path) {
    if (path.empty() || path.is_absolute() || path.has_root_name() ||
        path.has_root_directory() || path.filename().empty()) {
        return false;
    }
    for (const auto& component : path) {
        if (component == "." || component == ".." ||
            component.native().find(std::filesystem::path::value_type{}) !=
                std::filesystem::path::string_type::npos) {
            return false;
        }
    }
    return true;
}

bool relative_log_path_has_existing_redirect(
    const std::filesystem::path& canonical_root,
    const std::filesystem::path& relative_log_path) {
    std::filesystem::path candidate = canonical_root;
    for (const auto& component : relative_log_path) {
        candidate /= component;
        std::error_code error;
        const std::filesystem::file_status status =
            std::filesystem::symlink_status(candidate, error);
        if (error) {
            if (error == std::errc::no_such_file_or_directory) {
                return false;
            }
            return true;
        }
        if (std::filesystem::is_symlink(status)) {
            return true;
        }
        if (!std::filesystem::exists(status)) {
            return false;
        }
    }
    return false;
}

bool mode_is_valid(const WorkspaceAgentAccessMode mode) {
    switch (mode) {
        case WorkspaceAgentAccessMode::advisory:
        case WorkspaceAgentAccessMode::workspace_sandbox:
        case WorkspaceAgentAccessMode::unrestricted_local:
            return true;
    }
    return false;
}

bool denied_start_is_valid(const WorkspaceAgentSessionAuditEvent& event) {
    if (event.outcome != "denied" ||
        event.effective_mode != WorkspaceAgentAccessMode::advisory) {
        return false;
    }
    if (event.diagnostic_code == "workspace_agent.warning_required" ||
        event.diagnostic_code == "workspace_agent.warning_version_mismatch" ||
        event.diagnostic_code == "workspace_agent.confirmation_required") {
        return event.requested_mode == WorkspaceAgentAccessMode::unrestricted_local;
    }
    if (event.diagnostic_code == "workspace_agent.invalid_mode") {
        return !mode_is_valid(event.requested_mode);
    }
    return event.diagnostic_code == "workspace_agent.feature_disabled" ||
        event.diagnostic_code == "workspace_agent.permission_denied" ||
        event.diagnostic_code == "workspace_agent.trusted_ui_required" ||
        event.diagnostic_code == "workspace_agent.audit_unavailable" ||
        event.diagnostic_code == "workspace_agent.session_already_active" ||
        event.diagnostic_code == "workspace_agent.policy_evaluation_failed";
}

bool allowed_start_is_valid(const WorkspaceAgentSessionAuditEvent& event) {
    if (event.outcome != "allowed" || event.requested_mode != event.effective_mode) {
        return false;
    }
    switch (event.effective_mode) {
        case WorkspaceAgentAccessMode::advisory:
            return event.diagnostic_code == "workspace_agent.advisory_allowed";
        case WorkspaceAgentAccessMode::workspace_sandbox:
            return event.diagnostic_code == "workspace_agent.sandbox_allowed";
        case WorkspaceAgentAccessMode::unrestricted_local:
            return event.diagnostic_code == "workspace_agent.unrestricted_allowed";
    }
    return false;
}

bool session_audit_event_is_valid(const WorkspaceAgentSessionAuditEvent& event) {
    if (event.schema_version != 1U || event.session_generation == 0U ||
        event.session_generation == std::numeric_limits<std::uint64_t>::max()) {
        return false;
    }
    switch (event.kind) {
        case WorkspaceAgentSessionEventKind::start:
            if (event.outcome == "allowed") {
                return allowed_start_is_valid(event);
            }
            return denied_start_is_valid(event);
        case WorkspaceAgentSessionEventKind::stop:
            return event.outcome == "revoked" &&
                mode_is_valid(event.requested_mode) &&
                event.effective_mode == WorkspaceAgentAccessMode::advisory &&
                event.diagnostic_code == "workspace_agent.session_stopped";
    }
    return false;
}

}  // namespace

WorkspaceAgentSessionAuditFileSink::WorkspaceAgentSessionAuditFileSink(
    const std::filesystem::path& storage_root,
    const std::filesystem::path& relative_log_path,
    const std::size_t max_log_bytes) {
    if (!relative_log_path_is_safe(relative_log_path) ||
        max_log_bytes < workspace_agent_audit_min_log_bytes ||
        max_log_bytes > workspace_agent_audit_max_log_bytes) {
        return;
    }

    std::error_code error;
    const std::filesystem::path canonical_root =
        std::filesystem::canonical(storage_root, error);
    if (error || !std::filesystem::is_directory(canonical_root, error) || error ||
        relative_log_path_has_existing_redirect(canonical_root, relative_log_path)) {
        return;
    }

    try {
        storage_root_ = canonical_root;
        log_path_ = canonical_root / relative_log_path.lexically_normal();
        max_log_bytes_ = max_log_bytes;
        ready_ = true;
    } catch (...) {
        storage_root_.clear();
        log_path_.clear();
        max_log_bytes_ = 0U;
    }
}

bool WorkspaceAgentSessionAuditFileSink::ready() const noexcept {
    return ready_;
}

std::filesystem::path WorkspaceAgentSessionAuditFileSink::log_path() const {
    return log_path_;
}

WorkspaceAgentSessionAuditSink WorkspaceAgentSessionAuditFileSink::session_sink() noexcept {
    if (!ready_) {
        return {};
    }
    return {.commit = commit_thunk, .context = this};
}

WorkspaceAgentSessionAuditCommitResult WorkspaceAgentSessionAuditFileSink::commit_thunk(
    const WorkspaceAgentSessionAuditEvent& event,
    void* context) {
    if (context == nullptr) {
        return {};
    }
    try {
        return static_cast<WorkspaceAgentSessionAuditFileSink*>(context)->commit(event);
    } catch (...) {
        return {};
    }
}

WorkspaceAgentSessionAuditCommitResult WorkspaceAgentSessionAuditFileSink::commit(
    const WorkspaceAgentSessionAuditEvent& event) const {
    if (!ready_ || !session_audit_event_is_valid(event)) {
        return {};
    }
    const AuditAppendResult appended =
        append_bounded_immutable_audit_event_to_contained_file(
            platform::path_to_utf8_string(log_path_),
            platform::path_to_utf8_string(storage_root_),
            std::string(workspace_agent_session_audit_event_name),
            serialize_workspace_agent_session_audit_event(event),
            max_log_bytes_);
    if (!appended.ok || appended.entry_hash.empty()) {
        return {};
    }
    return {.ok = true, .receipt = appended.entry_hash};
}

}  // namespace copperfin::security
