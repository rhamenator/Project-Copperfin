// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/workspace_agent_session.h"

#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <string_view>
#include <utility>

namespace copperfin::security {

namespace {

struct AuditOutcome {
    bool committed = false;
    std::string receipt;
};

AuditOutcome commit_audit_event(
    const WorkspaceAgentSessionAuditEvent& event,
    const WorkspaceAgentSessionAuditSink& sink) {
    WorkspaceAgentSessionAuditCommitResult result;
    if (sink.commit != nullptr) {
        try {
            result = sink.commit(event, sink.context);
        } catch (...) {
            result = {};
        }
    }
    if (!result.ok || result.receipt.empty()) {
        return {};
    }
    return {.committed = true, .receipt = std::move(result.receipt)};
}

std::string event_kind_name(WorkspaceAgentSessionEventKind kind) {
    switch (kind) {
        case WorkspaceAgentSessionEventKind::start:
            return "start";
        case WorkspaceAgentSessionEventKind::stop:
            return "stop";
    }
    return "invalid";
}

std::string json_escape(std::string_view value) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    for (const unsigned char character : value) {
        switch (character) {
            case '\"':
                stream << "\\\"";
                break;
            case '\\':
                stream << "\\\\";
                break;
            case '\b':
                stream << "\\b";
                break;
            case '\f':
                stream << "\\f";
                break;
            case '\n':
                stream << "\\n";
                break;
            case '\r':
                stream << "\\r";
                break;
            case '\t':
                stream << "\\t";
                break;
            default:
                if (character < 0x20U || character == 0x7fU) {
                    stream << "\\u"
                           << std::hex << std::setw(4) << std::setfill('0')
                           << static_cast<unsigned int>(character)
                           << std::dec << std::setw(0);
                } else {
                    stream << static_cast<char>(character);
                }
                break;
        }
    }
    return stream.str();
}

WorkspaceAgentActivationDecision controller_denial(std::string diagnostic_code) {
    WorkspaceAgentActivationDecision decision;
    decision.diagnostic_code = std::move(diagnostic_code);
    decision.audit_required = true;
    return decision;
}

bool satisfies_tool_requirements(
    const WorkspaceAgentCapabilities& capabilities,
    const WorkspaceAgentToolRequirements& requirements) noexcept {
    return (!requirements.read_workspace_files || capabilities.read_workspace_files) &&
        (!requirements.write_workspace_files || capabilities.write_workspace_files) &&
        (!requirements.run_local_processes || capabilities.run_local_processes) &&
        (!requirements.access_outside_workspace || capabilities.access_outside_workspace) &&
        (!requirements.use_network || capabilities.use_network) &&
        (!requirements.elevate_privileges || capabilities.elevate_privileges);
}

}  // namespace

WorkspaceAgentSessionController::WorkspaceAgentSessionController(
    const std::filesystem::path& trusted_absolute_workspace_root)
    : file_target_boundary_(WorkspaceAgentFileTargetBoundary::create(
          trusted_absolute_workspace_root)),
      process_target_boundary_(WorkspaceAgentProcessTargetBoundary::create(
          trusted_absolute_workspace_root)) {}

WorkspaceAgentSessionStartResult WorkspaceAgentSessionController::start(
    const WorkspaceAgentActivationRequest& request,
    const WorkspaceAgentSessionAuditSink& audit_sink) {
    std::uint64_t candidate_generation = 0U;
    bool session_already_active = false;
    {
        std::lock_guard lock(mutex_);
        if (transition_ != Transition::idle) {
            WorkspaceAgentSessionStartResult result;
            result.diagnostic_code = "workspace_agent.session_transition_in_progress";
            result.session = active_session_;
            return result;
        }
        if (next_generation_ == std::numeric_limits<std::uint64_t>::max()) {
            WorkspaceAgentSessionStartResult result;
            result.diagnostic_code = "workspace_agent.session_generation_exhausted";
            result.session = active_session_;
            return result;
        }
        transition_ = Transition::starting;
        candidate_generation = next_generation_++;
        session_already_active = active_session_.active;
    }

    try {
        WorkspaceAgentActivationDecision decision;
        if (session_already_active) {
            decision = controller_denial("workspace_agent.session_already_active");
        } else {
            try {
                decision = evaluate_workspace_agent_activation(request);
            } catch (...) {
                decision = controller_denial("workspace_agent.policy_evaluation_failed");
            }
        }
        const WorkspaceAgentSessionAuditEvent event{
            .kind = WorkspaceAgentSessionEventKind::start,
            .session_generation = candidate_generation,
            .requested_mode = request.requested_mode,
            .effective_mode = decision.effective_mode,
            .outcome = decision.allowed ? "allowed" : "denied",
            .diagnostic_code = decision.diagnostic_code};
        const AuditOutcome audit = commit_audit_event(event, audit_sink);

        WorkspaceAgentSessionStartResult result;
        result.audit_committed = audit.committed;
        result.audit_receipt = audit.receipt;
        result.policy_decision = decision;
        result.diagnostic_code = audit.committed
            ? decision.diagnostic_code
            : "workspace_agent.session_audit_commit_failed";

        {
            std::lock_guard lock(mutex_);
            if (audit.committed && decision.allowed && !active_session_.active) {
                active_session_ = {
                    .active = true,
                    .generation = candidate_generation,
                    .effective_mode = decision.effective_mode,
                    .capabilities = decision.capabilities,
                    .activation_audit_receipt = audit.receipt};
                result.activated = true;
            }
            transition_ = Transition::idle;
            result.session = active_session_;
        }
        return result;
    } catch (...) {
        std::lock_guard lock(mutex_);
        transition_ = Transition::idle;
        throw;
    }
}

WorkspaceAgentSessionStopResult WorkspaceAgentSessionController::stop(
    const WorkspaceAgentSessionAuditSink& audit_sink) {
    WorkspaceAgentSessionSnapshot revoked_session;
    {
        std::lock_guard lock(mutex_);
        if (transition_ != Transition::idle) {
            WorkspaceAgentSessionStopResult result;
            result.diagnostic_code = "workspace_agent.session_transition_in_progress";
            result.session = active_session_;
            return result;
        }
        if (!active_session_.active) {
            WorkspaceAgentSessionStopResult result;
            result.diagnostic_code = "workspace_agent.session_not_active";
            result.session = active_session_;
            return result;
        }
        transition_ = Transition::stopping;
        revoked_session = active_session_;
        active_session_ = {};
    }

    const WorkspaceAgentSessionAuditEvent event{
        .kind = WorkspaceAgentSessionEventKind::stop,
        .session_generation = revoked_session.generation,
        .requested_mode = revoked_session.effective_mode,
        .effective_mode = WorkspaceAgentAccessMode::advisory,
        .outcome = "revoked",
        .diagnostic_code = "workspace_agent.session_stopped"};
    const AuditOutcome audit = commit_audit_event(event, audit_sink);

    WorkspaceAgentSessionStopResult result;
    result.revoked = true;
    result.audit_committed = audit.committed;
    result.audit_receipt = audit.receipt;
    result.diagnostic_code = audit.committed
        ? event.diagnostic_code
        : "workspace_agent.session_stop_audit_commit_failed";
    {
        std::lock_guard lock(mutex_);
        transition_ = Transition::idle;
        result.session = active_session_;
    }
    return result;
}

WorkspaceAgentSessionSnapshot WorkspaceAgentSessionController::snapshot() const {
    std::lock_guard lock(mutex_);
    return active_session_;
}

WorkspaceAgentToolPreflightResult WorkspaceAgentSessionController::preflight_tool_request(
    const WorkspaceAgentToolPreflightRequest& request) const {
    WorkspaceAgentToolPreflightResult result;
    if (request.schema_version != 1U) {
        result.diagnostic_code = "workspace_agent.tool_invalid_schema";
        return result;
    }
    const WorkspaceAgentToolDefinition* definition =
        find_workspace_agent_product_tool(request.tool_id);
    if (definition == nullptr) {
        result.diagnostic_code = "workspace_agent.tool_not_registered";
        return result;
    }

    std::lock_guard lock(mutex_);
    if (transition_ != Transition::idle) {
        result.diagnostic_code = "workspace_agent.session_transition_in_progress";
        return result;
    }
    if (!active_session_.active) {
        result.diagnostic_code = "workspace_agent.session_not_active";
        return result;
    }
    if (request.session_generation == 0U ||
        request.session_generation != active_session_.generation) {
        result.diagnostic_code = "workspace_agent.tool_stale_session";
        return result;
    }
    result.session_generation = active_session_.generation;
    result.effective_mode = active_session_.effective_mode;
    result.tool_id = std::string(definition->id);
    if (!satisfies_tool_requirements(
            active_session_.capabilities,
            definition->requirements)) {
        result.diagnostic_code = "workspace_agent.tool_capability_denied";
        return result;
    }
    result.allowed = true;
    result.diagnostic_code = "workspace_agent.tool_request_allowed";
    return result;
}

WorkspaceAgentFileTargetPreflightResult
WorkspaceAgentSessionController::preflight_file_target_request(
    const WorkspaceAgentFileTargetPreflightRequest& request) const {
    WorkspaceAgentFileTargetPreflightResult result;
    const WorkspaceAgentToolPreflightRequest tool_request{
        .schema_version = request.schema_version,
        .session_generation = request.session_generation,
        .tool_id = request.tool_id};
    const auto preliminary = preflight_tool_request(tool_request);
    if (!preliminary.allowed) {
        result.diagnostic_code = preliminary.diagnostic_code;
        return result;
    }

    result.session_generation = preliminary.session_generation;
    result.effective_mode = preliminary.effective_mode;
    result.tool_id = preliminary.tool_id;
    const WorkspaceAgentToolDefinition* definition =
        find_workspace_agent_product_tool(preliminary.tool_id);
    if (definition == nullptr) {
        result.diagnostic_code = "workspace_agent.tool_not_registered";
        return result;
    }

    WorkspaceAgentFileTargetInspection inspection;
    switch (definition->target_kind) {
        case WorkspaceAgentToolTargetKind::workspace_file:
            if (!file_target_boundary_.has_value()) {
                result.diagnostic_code =
                    "workspace_agent.target_workspace_root_not_configured";
                return result;
            }
            inspection = file_target_boundary_->inspect_workspace_file(
                request.target_path);
            break;
        case WorkspaceAgentToolTargetKind::local_file:
            if (!file_target_boundary_.has_value()) {
                // Construction of the product-owned boundary is also the
                // trusted configuration gate for local target inspection.
                result.diagnostic_code =
                    "workspace_agent.target_workspace_root_not_configured";
                return result;
            }
            inspection = file_target_boundary_->inspect_local_file(
                request.target_path);
            break;
        case WorkspaceAgentToolTargetKind::workspace_process:
        case WorkspaceAgentToolTargetKind::local_process:
        case WorkspaceAgentToolTargetKind::network_endpoint:
            result.diagnostic_code = "workspace_agent.target_not_file_tool";
            return result;
    }
    if (!inspection.allowed) {
        result.diagnostic_code = inspection.diagnostic_code;
        return result;
    }

    // Recheck after filesystem inspection so a stop or transition observed
    // during resolution cannot yield an allowed result. A future executor must
    // still repeat this beside the side effect because stop can follow return.
    const auto final_preflight = preflight_tool_request(tool_request);
    if (!final_preflight.allowed ||
        final_preflight.session_generation != preliminary.session_generation ||
        final_preflight.tool_id != preliminary.tool_id) {
        result = {};
        result.diagnostic_code = final_preflight.allowed
            ? "workspace_agent.tool_stale_session"
            : final_preflight.diagnostic_code;
        return result;
    }

    result.allowed = true;
    result.canonical_path = std::move(inspection.canonical_path);
    result.identity = inspection.identity;
    result.diagnostic_code = "workspace_agent.target_request_allowed";
    return result;
}

WorkspaceAgentProcessTargetPreflightResult
WorkspaceAgentSessionController::preflight_process_target_request(
    const WorkspaceAgentProcessTargetPreflightRequest& request) const {
    WorkspaceAgentProcessTargetPreflightResult result;
    const WorkspaceAgentToolPreflightRequest tool_request{
        .schema_version = request.schema_version,
        .session_generation = request.session_generation,
        .tool_id = request.tool_id};
    const auto preliminary = preflight_tool_request(tool_request);
    if (!preliminary.allowed) {
        result.diagnostic_code = preliminary.diagnostic_code;
        return result;
    }

    result.session_generation = preliminary.session_generation;
    result.effective_mode = preliminary.effective_mode;
    result.tool_id = preliminary.tool_id;
    const WorkspaceAgentToolDefinition* definition =
        find_workspace_agent_product_tool(preliminary.tool_id);
    if (definition == nullptr) {
        result.diagnostic_code = "workspace_agent.tool_not_registered";
        return result;
    }

    WorkspaceAgentProcessTargetInspection inspection;
    switch (definition->target_kind) {
        case WorkspaceAgentToolTargetKind::workspace_process:
            if (!process_target_boundary_.has_value()) {
                result.diagnostic_code =
                    "workspace_agent.process_workspace_root_not_configured";
                return result;
            }
            inspection = process_target_boundary_->inspect_workspace_process(
                request.executable_path, request.working_directory);
            break;
        case WorkspaceAgentToolTargetKind::local_process:
            if (!process_target_boundary_.has_value()) {
                result.diagnostic_code =
                    "workspace_agent.process_workspace_root_not_configured";
                return result;
            }
            inspection = process_target_boundary_->inspect_local_process(
                request.executable_path, request.working_directory);
            break;
        case WorkspaceAgentToolTargetKind::workspace_file:
        case WorkspaceAgentToolTargetKind::local_file:
        case WorkspaceAgentToolTargetKind::network_endpoint:
            result.diagnostic_code = "workspace_agent.target_not_process_tool";
            return result;
    }
    if (!inspection.allowed) {
        result.diagnostic_code = inspection.diagnostic_code;
        return result;
    }

    // Recheck after both filesystem inspections. This closes only the
    // point-in-time preflight interval; the executor must repeat beside launch.
    const auto final_preflight = preflight_tool_request(tool_request);
    if (!final_preflight.allowed ||
        final_preflight.session_generation != preliminary.session_generation ||
        final_preflight.tool_id != preliminary.tool_id) {
        result = {};
        result.diagnostic_code = final_preflight.allowed
            ? "workspace_agent.tool_stale_session"
            : final_preflight.diagnostic_code;
        return result;
    }

    result.allowed = true;
    result.canonical_executable_path =
        std::move(inspection.canonical_executable_path);
    result.executable_identity = inspection.executable_identity;
    result.canonical_working_directory =
        std::move(inspection.canonical_working_directory);
    result.working_directory_identity = inspection.working_directory_identity;
    result.diagnostic_code = "workspace_agent.process_target_request_allowed";
    return result;
}

std::string serialize_workspace_agent_session_audit_event(
    const WorkspaceAgentSessionAuditEvent& event) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << "{\"schema_version\":" << event.schema_version
           << ",\"event\":\"" << event_kind_name(event.kind)
           << "\",\"session_generation\":" << event.session_generation
           << ",\"requested_mode\":\""
           << json_escape(workspace_agent_access_mode_name(event.requested_mode))
           << "\",\"effective_mode\":\""
           << json_escape(workspace_agent_access_mode_name(event.effective_mode))
           << "\",\"outcome\":\"" << json_escape(event.outcome)
           << "\",\"diagnostic_code\":\"" << json_escape(event.diagnostic_code)
           << "\"}";
    return stream.str();
}

}  // namespace copperfin::security
