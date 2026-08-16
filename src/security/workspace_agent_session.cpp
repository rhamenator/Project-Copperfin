// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/workspace_agent_session.h"

#include "copperfin/platform/path.h"
#include "copperfin/platform/process_arguments.h"
#include "copperfin/platform/process_environment.h"

#include <condition_variable>
#include <iomanip>
#include <limits>
#include <locale>
#include <memory>
#include <sstream>
#include <string_view>
#include <utility>

namespace copperfin::security {

struct WorkspaceAgentSessionRevocationLeaseState {
    explicit WorkspaceAgentSessionRevocationLeaseState(
        const std::uint64_t generation_value)
        : generation(generation_value) {}

    std::mutex mutex;
    std::condition_variable released;
    std::uint64_t generation = 0U;
    bool active = true;
    std::size_t outstanding_leases = 0U;
};

class WorkspaceAgentSessionRevocationLease::Impl {
public:
    explicit Impl(
        std::shared_ptr<WorkspaceAgentSessionRevocationLeaseState> state_value)
        : state(std::move(state_value)) {}

    ~Impl() {
        if (!owns_lease || state == nullptr) {
            return;
        }
        std::lock_guard lock(state->mutex);
        --state->outstanding_leases;
        owns_lease = false;
        if (state->outstanding_leases == 0U) {
            state->released.notify_all();
        }
    }

    [[nodiscard]] bool acquire() {
        std::lock_guard lock(state->mutex);
        if (!state->active) {
            return false;
        }
        ++state->outstanding_leases;
        owns_lease = true;
        return true;
    }

    std::shared_ptr<WorkspaceAgentSessionRevocationLeaseState> state;
    bool owns_lease = false;
};

WorkspaceAgentSessionRevocationLease::WorkspaceAgentSessionRevocationLease(
    std::unique_ptr<Impl> impl)
    : impl_(std::move(impl)) {}

WorkspaceAgentSessionRevocationLease::~WorkspaceAgentSessionRevocationLease() = default;
WorkspaceAgentSessionRevocationLease::WorkspaceAgentSessionRevocationLease(
    WorkspaceAgentSessionRevocationLease&&) noexcept = default;
WorkspaceAgentSessionRevocationLease&
WorkspaceAgentSessionRevocationLease::operator=(
    WorkspaceAgentSessionRevocationLease&&) noexcept = default;

bool WorkspaceAgentSessionRevocationLease::valid() const noexcept {
    return impl_ != nullptr && impl_->owns_lease && impl_->state != nullptr;
}

std::uint64_t
WorkspaceAgentSessionRevocationLease::session_generation() const noexcept {
    return valid() ? impl_->state->generation : 0U;
}

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
        case WorkspaceAgentSessionEventKind::layout_cleanup_intent:
            return "layout_cleanup_intent";
        case WorkspaceAgentSessionEventKind::layout_cleanup_outcome:
            return "layout_cleanup_outcome";
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

bool valid_utf8(std::string_view value) noexcept {
    std::size_t offset = 0U;
    while (offset < value.size()) {
        const auto lead = static_cast<unsigned char>(value[offset]);
        std::size_t continuation_count = 0U;
        std::uint32_t scalar = 0U;
        if (lead <= 0x7fU) {
            ++offset;
            continue;
        }
        if (lead >= 0xc2U && lead <= 0xdfU) {
            continuation_count = 1U;
            scalar = lead & 0x1fU;
        } else if (lead >= 0xe0U && lead <= 0xefU) {
            continuation_count = 2U;
            scalar = lead & 0x0fU;
        } else if (lead >= 0xf0U && lead <= 0xf4U) {
            continuation_count = 3U;
            scalar = lead & 0x07U;
        } else {
            return false;
        }
        if (continuation_count > value.size() - offset - 1U) {
            return false;
        }
        for (std::size_t index = 1U; index <= continuation_count; ++index) {
            const auto continuation =
                static_cast<unsigned char>(value[offset + index]);
            if ((continuation & 0xc0U) != 0x80U) {
                return false;
            }
            scalar = (scalar << 6U) | (continuation & 0x3fU);
        }
        if ((continuation_count == 1U && scalar < 0x80U) ||
            (continuation_count == 2U && scalar < 0x800U) ||
            (continuation_count == 3U && scalar < 0x10000U) ||
            scalar > 0x10ffffU ||
            (scalar >= 0xd800U && scalar <= 0xdfffU)) {
            return false;
        }
        offset += continuation_count + 1U;
    }
    return true;
}

std::string process_arguments_diagnostic(
    const std::vector<std::string>& arguments) {
    if (arguments.size() > workspace_agent_process_max_argument_count) {
        return "workspace_agent.process_argument_count_exceeded";
    }
    std::size_t total_bytes = 0U;
    for (const auto& argument : arguments) {
        if (argument.size() > workspace_agent_process_max_argument_bytes) {
            return "workspace_agent.process_argument_size_exceeded";
        }
        if (argument.find('\0') != std::string::npos) {
            return "workspace_agent.process_argument_embedded_nul";
        }
        if (!valid_utf8(argument)) {
            return "workspace_agent.process_argument_invalid_utf8";
        }
        if (argument.size() >
            workspace_agent_process_max_total_argument_bytes - total_bytes) {
            return "workspace_agent.process_argument_total_size_exceeded";
        }
        total_bytes += argument.size();
    }
    return {};
}

}  // namespace

WorkspaceAgentSessionController::WorkspaceAgentSessionController(
    const std::filesystem::path& trusted_absolute_workspace_root)
    : file_target_boundary_(WorkspaceAgentFileTargetBoundary::create(
          trusted_absolute_workspace_root)),
      process_target_boundary_(WorkspaceAgentProcessTargetBoundary::create(
          trusted_absolute_workspace_root)) {}

WorkspaceAgentSessionController::WorkspaceAgentSessionController(
    const std::filesystem::path& trusted_absolute_workspace_root,
    const WorkspaceAgentIsolatedEnvironmentConfiguration&
        trusted_environment_configuration)
    : file_target_boundary_(WorkspaceAgentFileTargetBoundary::create(
          trusted_absolute_workspace_root)),
      process_target_boundary_(WorkspaceAgentProcessTargetBoundary::create(
          trusted_absolute_workspace_root)),
      process_environment_boundary_(WorkspaceAgentIsolatedEnvironmentBoundary::create(
          trusted_environment_configuration)),
      process_environment_configuration_supplied_(true) {}

WorkspaceAgentSessionController::WorkspaceAgentSessionController(
    const std::filesystem::path& trusted_absolute_workspace_root,
    const WorkspaceAgentIsolatedEnvironmentConfiguration&
        trusted_environment_configuration,
    const WorkspaceAgentProcessParserConfiguration&
        trusted_process_parser_configuration)
    : file_target_boundary_(WorkspaceAgentFileTargetBoundary::create(
          trusted_absolute_workspace_root)),
      process_target_boundary_(WorkspaceAgentProcessTargetBoundary::create(
          trusted_absolute_workspace_root)),
      process_environment_boundary_(WorkspaceAgentIsolatedEnvironmentBoundary::create(
          trusted_environment_configuration)),
      process_parser_boundary_(WorkspaceAgentProcessParserBoundary::create(
          trusted_process_parser_configuration)),
      process_environment_configuration_supplied_(true) {}

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
        std::optional<WorkspaceAgentSessionLayoutPreparationResult> preparation;
        std::shared_ptr<WorkspaceAgentSessionRevocationLeaseState>
            candidate_revocation_state;
        if (decision.allowed && decision.capabilities.run_local_processes &&
            process_environment_configuration_supplied_) {
            if (!process_environment_boundary_.has_value()) {
                decision = controller_denial(
                    "workspace_agent.session_environment_boundary_unavailable");
            } else {
                try {
                    bool cleanup_receipt_capacity_available = false;
                    {
                        std::lock_guard lock(mutex_);
                        cleanup_receipt_capacity_available =
                            pending_layout_cleanups_.size() <
                            workspace_agent_session_max_pending_layout_cleanups;
                        if (cleanup_receipt_capacity_available) {
                            // Allocate receipt storage before creating
                            // filesystem state. The later move cannot orphan a
                            // prepared layout merely because the FIFO grows.
                            pending_layout_cleanups_.reserve(
                                pending_layout_cleanups_.size() + 1U);
                        }
                    }
                    if (!cleanup_receipt_capacity_available) {
                        decision = controller_denial(
                            "workspace_agent.session_layout_cleanup_capacity_reached");
                    } else {
                        preparation =
                            process_environment_boundary_->prepare_session_layout(
                                candidate_generation);
                        if (!preparation->prepared ||
                            preparation->session_generation != candidate_generation) {
                            decision = controller_denial(
                                preparation->diagnostic_code.empty()
                                    ? "workspace_agent.session_environment_preparation_failed"
                                    : preparation->diagnostic_code);
                        }
                    }
                } catch (...) {
                    decision = controller_denial(
                        "workspace_agent.session_environment_preparation_failed");
                }
            }
        }
        if (decision.allowed && decision.capabilities.run_local_processes) {
            try {
                candidate_revocation_state =
                    std::make_shared<WorkspaceAgentSessionRevocationLeaseState>(
                        candidate_generation);
            } catch (...) {
                decision = controller_denial(
                    "workspace_agent.session_revocation_lease_unavailable");
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
            if (preparation.has_value() && preparation->prepared &&
                preparation->session_generation == candidate_generation) {
                pending_layout_cleanups_.push_back(std::move(*preparation));
            }
            if (audit.committed && decision.allowed && !active_session_.active) {
                active_session_ = {
                    .active = true,
                    .generation = candidate_generation,
                    .effective_mode = decision.effective_mode,
                    .capabilities = decision.capabilities,
                    .activation_audit_receipt = audit.receipt};
                active_revocation_lease_state_ =
                    std::move(candidate_revocation_state);
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

WorkspaceAgentSessionLayoutCleanupAttemptResult
WorkspaceAgentSessionController::cleanup_pending_session_layout(
    const WorkspaceAgentSessionAuditSink& audit_sink) {
    const WorkspaceAgentSessionLayoutPreparationResult* preparation = nullptr;
    {
        std::lock_guard lock(mutex_);
        WorkspaceAgentSessionLayoutCleanupAttemptResult result;
        if (transition_ != Transition::idle) {
            result.diagnostic_code =
                "workspace_agent.session_transition_in_progress";
            return result;
        }
        if (active_session_.active) {
            result.diagnostic_code =
                "workspace_agent.session_layout_cleanup_active_session";
            return result;
        }
        if (pending_layout_cleanups_.empty()) {
            result.diagnostic_code =
                "workspace_agent.session_layout_cleanup_not_pending";
            return result;
        }
        transition_ = Transition::cleaning;
        // The cleaning transition prevents every operation that can mutate the
        // receipt FIFO. Borrow the front receipt instead of copying its
        // heap-backed fields after changing state: this assignment cannot
        // throw and therefore cannot strand the controller in `cleaning`.
        preparation = &pending_layout_cleanups_.front();
    }

    try {
        WorkspaceAgentSessionLayoutCleanupAttemptResult result;
        result.session_generation = preparation->session_generation;
        const WorkspaceAgentSessionAuditEvent intent{
            .kind = WorkspaceAgentSessionEventKind::layout_cleanup_intent,
            .session_generation = preparation->session_generation,
            .requested_mode = WorkspaceAgentAccessMode::advisory,
            .effective_mode = WorkspaceAgentAccessMode::advisory,
            .outcome = "pending",
            .diagnostic_code = "workspace_agent.session_layout_cleanup_intent"};
        const AuditOutcome intent_audit = commit_audit_event(intent, audit_sink);
        result.intent_audit_committed = intent_audit.committed;
        result.intent_audit_receipt = intent_audit.receipt;
        if (!intent_audit.committed) {
            result.diagnostic_code =
                "workspace_agent.session_layout_cleanup_intent_audit_failed";
            std::lock_guard lock(mutex_);
            transition_ = Transition::idle;
            return result;
        }

        WorkspaceAgentSessionLayoutCleanupResult cleanup;
        if (!process_environment_boundary_.has_value()) {
            cleanup.diagnostic_code =
                "workspace_agent.session_environment_boundary_unavailable";
        } else {
            try {
                result.attempted = true;
                cleanup =
                    process_environment_boundary_->cleanup_empty_session_layout(
                        *preparation);
            } catch (...) {
                cleanup.diagnostic_code =
                    "workspace_agent.environment_session_layout_cleanup_failed";
            }
        }
        result.cleaned = cleanup.cleaned;

        const WorkspaceAgentSessionAuditEvent outcome{
            .kind = WorkspaceAgentSessionEventKind::layout_cleanup_outcome,
            .session_generation = preparation->session_generation,
            .requested_mode = WorkspaceAgentAccessMode::advisory,
            .effective_mode = WorkspaceAgentAccessMode::advisory,
            .outcome = cleanup.cleaned ? "cleaned" : "retained",
            .diagnostic_code = cleanup.diagnostic_code.empty()
                ? "workspace_agent.environment_session_layout_cleanup_failed"
                : cleanup.diagnostic_code};
        const AuditOutcome outcome_audit = commit_audit_event(outcome, audit_sink);
        result.outcome_audit_committed = outcome_audit.committed;
        result.outcome_audit_receipt = outcome_audit.receipt;
        result.diagnostic_code = outcome_audit.committed
            ? outcome.diagnostic_code
            : "workspace_agent.session_layout_cleanup_outcome_audit_failed";
        {
            std::lock_guard lock(mutex_);
            if (cleanup.cleaned && !pending_layout_cleanups_.empty() &&
                pending_layout_cleanups_.front().session_generation ==
                    preparation->session_generation) {
                pending_layout_cleanups_.erase(pending_layout_cleanups_.begin());
            }
            transition_ = Transition::idle;
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
    std::shared_ptr<WorkspaceAgentSessionRevocationLeaseState> revocation_state;
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
        revocation_state = active_revocation_lease_state_;
    }

    // A lease is held only around a future direct launch decision. Waiting
    // here ensures stop cannot report revocation in the middle of that narrow
    // boundary. The lease release path never takes the controller mutex.
    if (revocation_state != nullptr) {
        std::unique_lock revocation_lock(revocation_state->mutex);
        revocation_state->released.wait(revocation_lock, [&revocation_state] {
            return revocation_state->outstanding_leases == 0U;
        });
        revocation_state->active = false;
    }
    {
        std::lock_guard lock(mutex_);
        active_session_ = {};
        active_revocation_lease_state_.reset();
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

WorkspaceAgentProcessInvocationPreflightResult
WorkspaceAgentSessionController::preflight_process_invocation_request(
    const WorkspaceAgentProcessInvocationPreflightRequest& request) const {
    WorkspaceAgentProcessInvocationPreflightResult result;
    const WorkspaceAgentProcessTargetPreflightRequest target_request{
        .schema_version = request.schema_version,
        .session_generation = request.session_generation,
        .tool_id = request.tool_id,
        .executable_path = request.executable_path,
        .working_directory = request.working_directory};
    const auto target = preflight_process_target_request(target_request);
    if (!target.allowed) {
        result.diagnostic_code = target.diagnostic_code;
        return result;
    }

    result.diagnostic_code = process_arguments_diagnostic(request.arguments);
    if (!result.diagnostic_code.empty()) {
        return result;
    }
    std::vector<std::string> validated_arguments = request.arguments;

    // Recheck after bounded argument validation and copying. This result is
    // still point-in-time only; an executor must repeat every check beside
    // direct launch and independently enforce the platform serialization cap.
    const WorkspaceAgentToolPreflightRequest tool_request{
        .schema_version = request.schema_version,
        .session_generation = request.session_generation,
        .tool_id = request.tool_id};
    const auto final_preflight = preflight_tool_request(tool_request);
    if (!final_preflight.allowed ||
        final_preflight.session_generation != target.session_generation ||
        final_preflight.tool_id != target.tool_id) {
        result = {};
        result.diagnostic_code = final_preflight.allowed
            ? "workspace_agent.tool_stale_session"
            : final_preflight.diagnostic_code;
        return result;
    }

    result.allowed = true;
    result.session_generation = target.session_generation;
    result.effective_mode = target.effective_mode;
    result.tool_id = target.tool_id;
    result.canonical_executable_path = target.canonical_executable_path;
    result.executable_identity = target.executable_identity;
    result.canonical_working_directory = target.canonical_working_directory;
    result.working_directory_identity = target.working_directory_identity;
    result.arguments = std::move(validated_arguments);
    result.diagnostic_code = "workspace_agent.process_invocation_request_allowed";
    return result;
}

WorkspaceAgentProcessTargetPinPreflightResult
WorkspaceAgentSessionController::pin_process_target_request(
    const WorkspaceAgentProcessTargetPreflightRequest& request) const {
    WorkspaceAgentProcessTargetPinPreflightResult result;
    const WorkspaceAgentToolPreflightRequest tool_request{
        .schema_version = request.schema_version,
        .session_generation = request.session_generation,
        .tool_id = request.tool_id};
    const auto preliminary = preflight_tool_request(tool_request);
    if (!preliminary.allowed) {
        result.diagnostic_code = preliminary.diagnostic_code;
        return result;
    }
    if (!process_target_boundary_.has_value()) {
        result.diagnostic_code =
            "workspace_agent.process_workspace_root_not_configured";
        return result;
    }

    const WorkspaceAgentToolDefinition* definition =
        find_workspace_agent_product_tool(preliminary.tool_id);
    if (definition == nullptr) {
        result.diagnostic_code = "workspace_agent.tool_not_registered";
        return result;
    }

    WorkspaceAgentProcessTargetInspection inspection;
    switch (definition->target_kind) {
        case WorkspaceAgentToolTargetKind::workspace_process:
            inspection = process_target_boundary_->inspect_workspace_process(
                request.executable_path, request.working_directory);
            break;
        case WorkspaceAgentToolTargetKind::local_process:
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

    auto pin_result = process_target_boundary_->pin_process_targets(inspection);
    if (!pin_result.pinned || !pin_result.pins.has_value()) {
        result.diagnostic_code = pin_result.diagnostic_code;
        return result;
    }

    // Resource acquisition can race with stop. Recheck exact session/tool
    // admission afterward and release every pin on any mismatch. The returned
    // bundle remains non-executing and is not session or launch authority.
    const auto final_preflight = preflight_tool_request(tool_request);
    if (!final_preflight.allowed ||
        final_preflight.session_generation != preliminary.session_generation ||
        final_preflight.tool_id != preliminary.tool_id) {
        result.diagnostic_code = final_preflight.allowed
            ? "workspace_agent.tool_stale_session"
            : final_preflight.diagnostic_code;
        return result;
    }

    result.pinned = true;
    result.session_generation = final_preflight.session_generation;
    result.pins = std::move(pin_result.pins);
    result.diagnostic_code =
        "workspace_agent.process_target_pin_request_allowed";
    return result;
}

WorkspaceAgentProcessEnvironmentPreflightResult
WorkspaceAgentSessionController::preflight_process_environment_request(
    const WorkspaceAgentProcessInvocationPreflightRequest& request) const {
    WorkspaceAgentProcessEnvironmentPreflightResult result;
    const auto preliminary = preflight_process_invocation_request(request);
    if (!preliminary.allowed) {
        result.diagnostic_code = preliminary.diagnostic_code;
        return result;
    }
    if (!process_environment_boundary_.has_value()) {
        result.diagnostic_code =
            "workspace_agent.process_environment_boundary_unavailable";
        return result;
    }

    const auto environment = process_environment_boundary_->construct(
        preliminary.session_generation, preliminary.environment_policy);
    if (!environment.allowed) {
        result.diagnostic_code = environment.diagnostic_code;
        return result;
    }

    const auto final = preflight_process_invocation_request(request);
    if (!final.allowed ||
        final.session_generation != preliminary.session_generation ||
        final.effective_mode != preliminary.effective_mode ||
        final.tool_id != preliminary.tool_id ||
        final.canonical_executable_path != preliminary.canonical_executable_path ||
        final.executable_identity != preliminary.executable_identity ||
        final.canonical_working_directory !=
            preliminary.canonical_working_directory ||
        final.working_directory_identity !=
            preliminary.working_directory_identity ||
        final.arguments != preliminary.arguments ||
        final.environment_policy != preliminary.environment_policy ||
        environment.session_generation != preliminary.session_generation ||
        environment.policy != preliminary.environment_policy ||
        environment.platform != workspace_agent_process_environment_host_platform()) {
        result.diagnostic_code =
            "workspace_agent.process_environment_stale_invocation";
        return result;
    }

    result.allowed = true;
    result.session_generation = final.session_generation;
    result.effective_mode = final.effective_mode;
    result.tool_id = final.tool_id;
    result.canonical_executable_path = final.canonical_executable_path;
    result.executable_identity = final.executable_identity;
    result.canonical_working_directory = final.canonical_working_directory;
    result.working_directory_identity = final.working_directory_identity;
    result.arguments = final.arguments;
    result.environment_policy = final.environment_policy;
    result.environment_platform = environment.platform;
    result.environment_entries = environment.entries;
    result.diagnostic_code = "workspace_agent.process_environment_request_allowed";
    return result;
}

WorkspaceAgentSerializedProcessEnvironmentPreflightResult
WorkspaceAgentSessionController::preflight_serialized_process_environment_request(
    const WorkspaceAgentProcessInvocationPreflightRequest& request) const {
    WorkspaceAgentSerializedProcessEnvironmentPreflightResult result;
    const auto preliminary = preflight_process_environment_request(request);
    if (!preliminary.allowed) {
        result.diagnostic_code = preliminary.diagnostic_code;
        return result;
    }

    std::vector<copperfin::platform::ProcessEnvironmentEntry> entries;
    entries.reserve(preliminary.environment_entries.size());
    for (const auto& entry : preliminary.environment_entries) {
        entries.push_back({.name = entry.name, .value = entry.value});
    }
    if (preliminary.environment_platform !=
            WorkspaceAgentProcessEnvironmentPlatform::windows_v1 &&
        preliminary.environment_platform !=
            WorkspaceAgentProcessEnvironmentPlatform::posix_v1) {
        result.diagnostic_code =
            "workspace_agent.process_environment_serialization_invalid_platform";
        return result;
    }
    const bool windows = preliminary.environment_platform ==
        WorkspaceAgentProcessEnvironmentPlatform::windows_v1;
    const auto target = windows
        ? copperfin::platform::ProcessEnvironmentTarget::windows_utf16_v1
        : copperfin::platform::ProcessEnvironmentTarget::posix_v1;
    const std::size_t maximum_units =
        workspace_agent_serialized_environment_maximum_units(
            preliminary.environment_platform,
            preliminary.environment_entries.size());
    auto serialized = copperfin::platform::serialize_process_environment(
        entries, target, maximum_units);
    if (!serialized.ok) {
        result.diagnostic_code = serialized.diagnostic_code;
        return result;
    }

    // Serialization can allocate and copy. Repeat the complete logical
    // preflight afterward and reject any changed session, target, argument,
    // environment policy, or fixed entry set.
    const auto final = preflight_process_environment_request(request);
    if (!final.allowed ||
        final.session_generation != preliminary.session_generation ||
        final.effective_mode != preliminary.effective_mode ||
        final.tool_id != preliminary.tool_id ||
        final.canonical_executable_path != preliminary.canonical_executable_path ||
        final.executable_identity != preliminary.executable_identity ||
        final.canonical_working_directory != preliminary.canonical_working_directory ||
        final.working_directory_identity != preliminary.working_directory_identity ||
        final.arguments != preliminary.arguments ||
        final.environment_policy != preliminary.environment_policy ||
        final.environment_platform != preliminary.environment_platform ||
        final.environment_entries != preliminary.environment_entries) {
        result.diagnostic_code =
            "workspace_agent.process_environment_serialization_stale_invocation";
        return result;
    }

    result.allowed = true;
    result.environment_plan = final;
    result.posix_environment = std::move(serialized.posix_entries);
    result.windows_environment_block = std::move(serialized.windows_block);
    result.diagnostic_code =
        "workspace_agent.process_environment_serialization_request_allowed";
    return result;
}

WorkspaceAgentSerializedProcessInvocationPreflightResult
WorkspaceAgentSessionController::preflight_serialized_process_invocation_request(
    const WorkspaceAgentProcessInvocationPreflightRequest& request) const {
    WorkspaceAgentSerializedProcessInvocationPreflightResult result;
    const auto preliminary =
        preflight_serialized_process_environment_request(request);
    if (!preliminary.allowed) {
        result.diagnostic_code = preliminary.diagnostic_code;
        return result;
    }

    const auto platform =
        preliminary.environment_plan.environment_platform;
    if (platform != WorkspaceAgentProcessEnvironmentPlatform::windows_v1 &&
        platform != WorkspaceAgentProcessEnvironmentPlatform::posix_v1) {
        result.diagnostic_code =
            "workspace_agent.process_argument_serialization_invalid_platform";
        return result;
    }
    const bool windows =
        platform == WorkspaceAgentProcessEnvironmentPlatform::windows_v1;
    WorkspaceAgentProcessArgumentParserContract parser_contract =
        WorkspaceAgentProcessArgumentParserContract::posix_argv_v1;
    if (windows) {
        if (!process_parser_boundary_.has_value()) {
            result.diagnostic_code =
                "workspace_agent.process_argument_parser_authority_unavailable";
            return result;
        }
        const auto parser = process_parser_boundary_->authorize_windows(
            preliminary.environment_plan.canonical_executable_path,
            preliminary.environment_plan.executable_identity);
        if (!parser.allowed) {
            result.diagnostic_code = parser.diagnostic_code;
            return result;
        }
        parser_contract = parser.contract;
    }
    const auto target = windows
        ? copperfin::platform::ProcessArgumentTarget::windows_command_line_v1
        : copperfin::platform::ProcessArgumentTarget::posix_v1;
    const std::size_t maximum_units = windows
        ? 32767U
        : std::numeric_limits<std::size_t>::max();
    auto serialized = copperfin::platform::serialize_process_arguments(
        copperfin::platform::path_to_utf8_string(
            preliminary.environment_plan.canonical_executable_path),
        preliminary.environment_plan.arguments,
        target,
        maximum_units);
    if (!serialized.ok) {
        result.diagnostic_code = serialized.diagnostic_code;
        return result;
    }

    // Argument serialization allocates and copies. Repeat the complete
    // environment serialization preflight afterward; neither result is an
    // authority token and a future executor must repeat beside direct launch.
    const auto final = preflight_serialized_process_environment_request(request);
    WorkspaceAgentProcessArgumentParserContract final_parser_contract =
        WorkspaceAgentProcessArgumentParserContract::posix_argv_v1;
    if (windows && final.allowed && final.environment_plan.allowed) {
        const auto final_parser = process_parser_boundary_->authorize_windows(
            final.environment_plan.canonical_executable_path,
            final.environment_plan.executable_identity);
        if (!final_parser.allowed) {
            result.diagnostic_code = final_parser.diagnostic_code;
            return result;
        }
        final_parser_contract = final_parser.contract;
    }
    if (!final.allowed ||
        !final.environment_plan.allowed ||
        !preliminary.environment_plan.allowed ||
        final.diagnostic_code != preliminary.diagnostic_code ||
        final.environment_plan.diagnostic_code !=
            preliminary.environment_plan.diagnostic_code ||
        final.environment_plan.session_generation !=
            preliminary.environment_plan.session_generation ||
        final.environment_plan.effective_mode !=
            preliminary.environment_plan.effective_mode ||
        final.environment_plan.tool_id != preliminary.environment_plan.tool_id ||
        final.environment_plan.canonical_executable_path !=
            preliminary.environment_plan.canonical_executable_path ||
        final.environment_plan.executable_identity !=
            preliminary.environment_plan.executable_identity ||
        final.environment_plan.canonical_working_directory !=
            preliminary.environment_plan.canonical_working_directory ||
        final.environment_plan.working_directory_identity !=
            preliminary.environment_plan.working_directory_identity ||
        final.environment_plan.arguments !=
            preliminary.environment_plan.arguments ||
        final.environment_plan.environment_policy !=
            preliminary.environment_plan.environment_policy ||
        final.environment_plan.environment_platform != platform ||
        final.environment_plan.environment_entries !=
            preliminary.environment_plan.environment_entries ||
        final.posix_environment != preliminary.posix_environment ||
        final.windows_environment_block !=
            preliminary.windows_environment_block ||
        final_parser_contract != parser_contract) {
        result.diagnostic_code =
            "workspace_agent.process_argument_serialization_stale_invocation";
        return result;
    }

    result.allowed = true;
    result.serialized_environment = final;
    result.posix_arguments = std::move(serialized.posix_arguments);
    result.windows_command_line =
        std::move(serialized.windows_command_line);
    result.argument_parser_contract = parser_contract;
    result.diagnostic_code =
        "workspace_agent.process_argument_serialization_request_allowed";
    return result;
}

WorkspaceAgentLaunchRevalidationResult
WorkspaceAgentSessionController::revalidate_serialized_process_invocation_for_launch(
    const WorkspaceAgentProcessInvocationPreflightRequest& request,
    const WorkspaceAgentSerializedProcessInvocationPreflightResult&
        admitted_plan) const {
    static_cast<void>(request);
    static_cast<void>(admitted_plan);
    WorkspaceAgentLaunchRevalidationResult result;
    result.diagnostic_code =
        "workspace_agent.process_launch_revalidation_pinning_unavailable";
    return result;
}

WorkspaceAgentSessionRevocationLeaseResult
WorkspaceAgentSessionController::acquire_process_launch_revocation_lease(
    const std::uint64_t session_generation) const {
    WorkspaceAgentSessionRevocationLeaseResult result;
    std::lock_guard controller_lock(mutex_);
    if (transition_ != Transition::idle) {
        result.diagnostic_code =
            "workspace_agent.session_transition_in_progress";
        return result;
    }
    if (!active_session_.active) {
        result.diagnostic_code = "workspace_agent.session_not_active";
        return result;
    }
    if (session_generation == 0U ||
        session_generation != active_session_.generation) {
        result.diagnostic_code =
            "workspace_agent.process_launch_lease_stale_session";
        return result;
    }
    if (!active_session_.capabilities.run_local_processes) {
        result.diagnostic_code =
            "workspace_agent.process_launch_lease_capability_denied";
        return result;
    }
    const auto state = active_revocation_lease_state_;
    if (state == nullptr || state->generation != session_generation) {
        result.diagnostic_code =
            "workspace_agent.process_launch_lease_unavailable";
        return result;
    }

    auto lease_impl =
        std::make_unique<WorkspaceAgentSessionRevocationLease::Impl>(state);
    if (state->generation != session_generation || !lease_impl->acquire()) {
        result.diagnostic_code =
            "workspace_agent.process_launch_lease_stale_session";
        return result;
    }
    result.lease.emplace(WorkspaceAgentSessionRevocationLease(
        std::move(lease_impl)));
    result.acquired = true;
    result.diagnostic_code =
        "workspace_agent.process_launch_revocation_lease_acquired";
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
