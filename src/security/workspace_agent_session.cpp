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

}  // namespace

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
