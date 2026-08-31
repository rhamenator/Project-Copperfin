// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/workspace_agent_audit_sink.h"

#include "copperfin/platform/path.h"
#include "copperfin/security/audit_stream.h"

#include <algorithm>
#include <array>
#include <limits>
#include <string_view>
#include <system_error>

namespace copperfin::security {

namespace {

constexpr std::string_view workspace_agent_session_audit_event_name =
    "workspace_agent.session.v1";
constexpr std::string_view workspace_agent_process_audit_event_name =
    "workspace_agent.process.v2";
constexpr std::string_view workspace_agent_file_read_audit_event_name =
    "workspace_agent.file_read.v3";

bool path_has_embedded_nul(const std::filesystem::path& path) {
    for (const auto& component : path) {
        if (component.native().find(std::filesystem::path::value_type{}) !=
            std::filesystem::path::string_type::npos) {
            return true;
        }
    }
    return false;
}

#if defined(_WIN32)
// Win32's CreateFileW/GetFullPathNameW silently strip a trailing '.' or ' '
// from each path component before resolving it, so "log." and "log " name
// the same object as "log". A component-boundary check alone cannot reject
// this: it must inspect each component's own last character.
bool path_has_windows_alias_prone_component(const std::filesystem::path& path) {
    for (const auto& component : path) {
        const auto& native = component.native();
        if (!native.empty() &&
            (native.back() == L'.' || native.back() == L' ')) {
            return true;
        }
    }
    return false;
}

bool path_component_is_reserved_windows_device_name(
    const std::filesystem::path::string_type& component) {
    // Legacy MS-DOS device syntax ("NUL:", "COM1:") is still honored by
    // Win32 path resolution for backward compatibility, so a reserved name
    // can be terminated by ':' as well as by '.' -- stopping at '.' alone is
    // a well-known bypass for naive reserved-name checks.
    const auto stop = component.find_first_of(L".:");
    const std::filesystem::path::string_type stem =
        (stop == std::filesystem::path::string_type::npos)
            ? component
            : component.substr(0U, stop);
    if (stem.empty() || stem.size() > 4U) {
        return false;
    }
    std::filesystem::path::string_type upper;
    upper.reserve(stem.size());
    for (const wchar_t ch : stem) {
        upper.push_back(
            (ch >= L'a' && ch <= L'z') ? static_cast<wchar_t>(ch - (L'a' - L'A')) : ch);
    }
    static constexpr std::wstring_view reserved_names[] = {
        L"CON", L"PRN", L"AUX", L"NUL",
        L"COM1", L"COM2", L"COM3", L"COM4", L"COM5", L"COM6", L"COM7", L"COM8", L"COM9",
        L"LPT1", L"LPT2", L"LPT3", L"LPT4", L"LPT5", L"LPT6", L"LPT7", L"LPT8", L"LPT9"
    };
    for (const std::wstring_view name : reserved_names) {
        if (upper == name) {
            return true;
        }
    }
    return false;
}

// Windows reserves these names as device objects at every path component,
// not just the final one, and regardless of any extension: CreateFileW on
// "NUL.txt" opens the NUL device rather than creating a file with that
// name. Matching is case-insensitive against the portion of the component
// before its first '.', mirroring how Windows itself resolves the name.
bool path_has_reserved_windows_device_name_component(
    const std::filesystem::path& path) {
    for (const auto& component : path) {
        if (path_component_is_reserved_windows_device_name(component.native())) {
            return true;
        }
    }
    return false;
}
#else
bool path_has_windows_alias_prone_component(const std::filesystem::path&) {
    return false;
}

bool path_has_reserved_windows_device_name_component(const std::filesystem::path&) {
    return false;
}
#endif

bool relative_log_path_is_safe(const std::filesystem::path& path) {
    if (path.empty() || path.is_absolute() || path.has_root_name() ||
        path.has_root_directory() || path.filename().empty() ||
        path_has_embedded_nul(path) ||
        path_has_windows_alias_prone_component(path) ||
        path_has_reserved_windows_device_name_component(path)) {
        return false;
    }
    for (const auto& component : path) {
        if (component == "." || component == "..") {
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
        event.diagnostic_code ==
            "workspace_agent.session_layout_cleanup_capacity_reached" ||
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

bool process_launch_failure_diagnostic_is_valid(std::string_view diagnostic) {
    static constexpr std::array<std::string_view, 18U> diagnostics{{
        "polyglot.process.exit_query_failed",
        "polyglot.process.executable_path_unsupported",
        "polyglot.process.input_write_failed",
        "polyglot.process.input_writer_create_failed",
        "polyglot.process.image_binding_failed",
        "polyglot.process.job_assign_failed",
        "polyglot.process.job_configure_failed",
        "polyglot.process.job_create_failed",
        "polyglot.process.launch_failed",
        "polyglot.process.output_pipe_failed",
        "polyglot.process.output_read_failed",
        "polyglot.process.output_reader_create_failed",
        "polyglot.process.resume_failed",
        "polyglot.process.transport_pipe_failed",
        "polyglot.process.tree_termination_failed",
        "polyglot.process.wait_failed",
        "polyglot.process.working_directory_path_unsupported",
        "workspace_agent.process_execution_failed"}};
    return std::find(diagnostics.begin(), diagnostics.end(), diagnostic) !=
        diagnostics.end();
}

bool process_denial_diagnostic_is_valid(std::string_view diagnostic) {
    static constexpr std::array<std::string_view, 6U> diagnostics{{
        "workspace_agent.process_execution_elevated_host_denied",
        "workspace_agent.process_execution_elevation_unavailable",
        "workspace_agent.process_execution_invalid_controls",
        "workspace_agent.process_execution_platform_unavailable",
        "workspace_agent.process_execution_working_directory_unavailable",
        "workspace_agent.process_execution_requires_unrestricted_local"}};
    return std::find(diagnostics.begin(), diagnostics.end(), diagnostic) !=
        diagnostics.end();
}

bool process_audit_event_is_valid(const WorkspaceAgentSessionAuditEvent& event) {
    const bool instance_id_valid = event.process_instance_id.size() == 32U &&
        std::all_of(
            event.process_instance_id.begin(), event.process_instance_id.end(),
            [](const unsigned char value) {
                return (value >= '0' && value <= '9') ||
                    (value >= 'a' && value <= 'f');
            }) &&
        std::any_of(
            event.process_instance_id.begin(), event.process_instance_id.end(),
            [](const char value) { return value != '0'; });
    if (event.schema_version != 2U || !instance_id_valid ||
        !event.operation_instance_id.empty() ||
        event.operation_id == 0U ||
        event.requested_mode != event.effective_mode ||
        (event.effective_mode != WorkspaceAgentAccessMode::workspace_sandbox &&
         event.effective_mode != WorkspaceAgentAccessMode::unrestricted_local)) {
        return false;
    }
    if (event.kind == WorkspaceAgentSessionEventKind::process_launch_intent) {
        return event.outcome == "pending" &&
            event.diagnostic_code == "workspace_agent.process_launch_intent";
    }
    if (event.outcome == "denied") {
        return process_denial_diagnostic_is_valid(event.diagnostic_code);
    }
    if (event.outcome == "exited") {
        return event.diagnostic_code == "polyglot.process.exited";
    }
    if (event.outcome == "cancelled") {
        return event.diagnostic_code == "polyglot.process.cancelled";
    }
    if (event.outcome == "timed-out") {
        return event.diagnostic_code == "polyglot.process.timeout";
    }
    if (event.outcome == "output-limit-exceeded") {
        return event.diagnostic_code == "polyglot.process.stderr_limit_exceeded" ||
            event.diagnostic_code == "polyglot.process.stdout_limit_exceeded";
    }
    if (event.outcome == "invalid-request") {
        return event.diagnostic_code == "polyglot.process.invalid_request";
    }
    return event.outcome == "launch-failed" &&
        process_launch_failure_diagnostic_is_valid(event.diagnostic_code);
}

bool operation_instance_id_is_valid(const std::string_view value) {
    return value.size() == 32U &&
        std::all_of(value.begin(), value.end(), [](const unsigned char character) {
            return (character >= '0' && character <= '9') ||
                (character >= 'a' && character <= 'f');
        }) &&
        std::any_of(value.begin(), value.end(), [](const char character) {
            return character != '0';
        });
}

bool workspace_file_read_audit_event_is_valid(
    const WorkspaceAgentSessionAuditEvent& event) {
    if (event.schema_version != 3U || !event.process_instance_id.empty() ||
        !operation_instance_id_is_valid(event.operation_instance_id) ||
        event.operation_id == 0U ||
        event.requested_mode != event.effective_mode ||
        (event.effective_mode != WorkspaceAgentAccessMode::workspace_sandbox &&
         event.effective_mode != WorkspaceAgentAccessMode::unrestricted_local)) {
        return false;
    }
    if (event.kind == WorkspaceAgentSessionEventKind::workspace_file_read_intent) {
        return event.outcome == "pending" &&
            event.diagnostic_code == "workspace_agent.file_read_intent";
    }
    if (event.kind != WorkspaceAgentSessionEventKind::workspace_file_read_outcome) {
        return false;
    }
    if (event.outcome == "captured") {
        return event.diagnostic_code == "workspace_agent.file_read_captured";
    }
    return event.outcome == "failed" &&
        (event.diagnostic_code == "workspace_agent.file_read_failed" ||
         event.diagnostic_code == "workspace_agent.file_read_identity_changed" ||
         event.diagnostic_code == "workspace_agent.file_read_size_limit_exceeded" ||
         event.diagnostic_code == "workspace_agent.file_read_target_unavailable" ||
         event.diagnostic_code == "workspace_agent.target_root_identity_changed");
}

bool session_audit_event_is_valid(const WorkspaceAgentSessionAuditEvent& event) {
    if (event.session_generation == 0U ||
        event.session_generation == std::numeric_limits<std::uint64_t>::max()) {
        return false;
    }
    if (event.kind == WorkspaceAgentSessionEventKind::process_launch_intent ||
        event.kind == WorkspaceAgentSessionEventKind::process_launch_outcome) {
        return process_audit_event_is_valid(event);
    }
    if (event.kind == WorkspaceAgentSessionEventKind::workspace_file_read_intent ||
        event.kind == WorkspaceAgentSessionEventKind::workspace_file_read_outcome) {
        return workspace_file_read_audit_event_is_valid(event);
    }
    if (event.schema_version != 1U || !event.process_instance_id.empty() ||
        !event.operation_instance_id.empty() ||
        event.operation_id != 0U) {
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
        case WorkspaceAgentSessionEventKind::layout_cleanup_intent:
            return event.outcome == "pending" &&
                event.requested_mode == WorkspaceAgentAccessMode::advisory &&
                event.effective_mode == WorkspaceAgentAccessMode::advisory &&
                event.diagnostic_code ==
                    "workspace_agent.session_layout_cleanup_intent";
        case WorkspaceAgentSessionEventKind::layout_cleanup_outcome:
            if (event.requested_mode != WorkspaceAgentAccessMode::advisory ||
                event.effective_mode != WorkspaceAgentAccessMode::advisory) {
                return false;
            }
            if (event.outcome == "cleaned") {
                return event.diagnostic_code ==
                    "workspace_agent.environment_session_layout_cleaned";
            }
            return event.outcome == "retained" &&
                (event.diagnostic_code ==
                     "workspace_agent.environment_session_layout_cleanup_invalid_receipt" ||
                 event.diagnostic_code ==
                     "workspace_agent.environment_storage_root_identity_changed" ||
                 event.diagnostic_code ==
                     "workspace_agent.environment_session_layout_cleanup_identity_changed" ||
                 event.diagnostic_code ==
                     "workspace_agent.environment_session_layout_cleanup_not_empty" ||
                 event.diagnostic_code ==
                     "workspace_agent.environment_session_layout_cleanup_failed" ||
                 event.diagnostic_code ==
                     "workspace_agent.session_environment_boundary_unavailable");
        case WorkspaceAgentSessionEventKind::process_launch_intent:
        case WorkspaceAgentSessionEventKind::process_launch_outcome:
        case WorkspaceAgentSessionEventKind::workspace_file_read_intent:
        case WorkspaceAgentSessionEventKind::workspace_file_read_outcome:
            return false;
    }
    return false;
}

}  // namespace

WorkspaceAgentSessionAuditFileSink::WorkspaceAgentSessionAuditFileSink(
    const std::filesystem::path& storage_root,
    const std::filesystem::path& relative_log_path,
    const std::size_t max_log_bytes) {
    if (path_has_embedded_nul(storage_root) ||
        !relative_log_path_is_safe(relative_log_path) ||
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
        std::string(event.schema_version == 2U
                    ? workspace_agent_process_audit_event_name
                    : event.schema_version == 3U
                        ? workspace_agent_file_read_audit_event_name
                        : workspace_agent_session_audit_event_name),
            serialize_workspace_agent_session_audit_event(event),
            max_log_bytes_);
    if (!appended.ok || appended.entry_hash.empty()) {
        return {};
    }
    return {.ok = true, .receipt = appended.entry_hash};
}

}  // namespace copperfin::security
