// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/polyglot_artifact_adapter.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace copperfin::platform {

namespace {

std::uint32_t bounded_elapsed(const std::uint64_t elapsed_ms) noexcept {
    return static_cast<std::uint32_t>(std::min<std::uint64_t>(
        elapsed_ms,
        std::numeric_limits<std::uint32_t>::max()));
}

void decide_and_record(
    PolyglotArtifactInvocationResult& result,
    const std::string& capability_id,
    const PolyglotBridgeInvocationPolicy& policy,
    const PolyglotBridgeFailure failure,
    const bool cancellation_requested = false) {
    result.decision = evaluate_polyglot_bridge_invocation(
        policy,
        PolyglotBridgeInvocationRequest{
            .elapsed_ms = bounded_elapsed(result.process.elapsed_ms),
            .cancellation_requested = cancellation_requested,
            .failure = failure});
    record_polyglot_bridge_event(
        result.telemetry,
        capability_id,
        result.process.elapsed_ms,
        result.decision);
}

PolyglotArtifactInvocationResult reject_before_launch(
    const PolyglotArtifactInvocationStatus status,
    std::string error_code,
    const std::string& capability_id,
    const PolyglotBridgeInvocationPolicy& policy,
    const PolyglotBridgeFailure failure) {
    PolyglotArtifactInvocationResult result;
    result.status = status;
    result.error_code = std::move(error_code);
    decide_and_record(result, capability_id, policy, failure);
    return result;
}

PolyglotArtifactInvocationResult reject_without_telemetry(
    const PolyglotArtifactInvocationStatus status,
    std::string error_code) {
    PolyglotArtifactInvocationResult result;
    result.status = status;
    result.error_code = std::move(error_code);
    return result;
}

}  // namespace

PolyglotArtifactInvocationResult invoke_polyglot_artifact(
    PolyglotArtifactAdmissionResult& admission,
    const PolyglotArtifactInvocationRequest& request) {
    std::vector<PolyglotSupportingArtifactAdmissionResult> no_supporting_artifacts;
    const std::vector<PolyglotSupportingArtifactArgumentBinding>
        no_supporting_bindings;
    return invoke_polyglot_artifact(
        admission, no_supporting_artifacts, no_supporting_bindings, request);
}

PolyglotArtifactInvocationResult invoke_polyglot_artifact(
    PolyglotArtifactAdmissionResult& admission,
    std::vector<PolyglotSupportingArtifactAdmissionResult>&
        supporting_artifact_admissions,
    const std::vector<PolyglotSupportingArtifactArgumentBinding>&
        supporting_artifact_arguments,
    const PolyglotArtifactInvocationRequest& request) {
    const std::string capability_id = request.invocation.capability_id;
    if (!admission.ok()) {
        return reject_without_telemetry(
            PolyglotArtifactInvocationStatus::artifact_rejected,
            admission.error_code().empty()
                ? std::string("polyglot.adapter.artifact_not_admitted")
                : admission.error_code());
    }
    if (capability_id != admission.capability_id()) {
        return reject_without_telemetry(
            PolyglotArtifactInvocationStatus::artifact_rejected,
            "polyglot.adapter.capability_id_mismatch");
    }
    if (supporting_artifact_arguments.size() !=
        supporting_artifact_admissions.size()) {
        return reject_without_telemetry(
            PolyglotArtifactInvocationStatus::artifact_rejected,
            "polyglot.adapter.supporting_artifact_binding_required");
    }
    std::vector<bool> bound_arguments(request.artifact_arguments.size(), false);
    std::vector<bool> bound_admissions(
        supporting_artifact_admissions.size(), false);
    for (const auto& binding : supporting_artifact_arguments) {
        if (binding.argument_index >= request.artifact_arguments.size() ||
            binding.admission_index >= supporting_artifact_admissions.size() ||
            bound_arguments[binding.argument_index] ||
            bound_admissions[binding.admission_index]) {
            return reject_without_telemetry(
                PolyglotArtifactInvocationStatus::artifact_rejected,
                "polyglot.adapter.invalid_supporting_artifact_binding");
        }
        auto& supporting =
            supporting_artifact_admissions[binding.admission_index];
        if (!supporting.ok()) {
            return reject_without_telemetry(
                PolyglotArtifactInvocationStatus::artifact_rejected,
                supporting.error_code().empty()
                    ? std::string(
                          "polyglot.adapter.supporting_artifact_rejected")
                    : supporting.error_code());
        }
        if (supporting.capability_id() != capability_id) {
            return reject_without_telemetry(
                PolyglotArtifactInvocationStatus::artifact_rejected,
                "polyglot.adapter.supporting_artifact_capability_mismatch");
        }
        if (request.artifact_arguments[binding.argument_index] !=
            supporting.resolved_path()) {
            return reject_without_telemetry(
                PolyglotArtifactInvocationStatus::artifact_rejected,
                "polyglot.adapter.supporting_artifact_argument_mismatch");
        }
        bound_arguments[binding.argument_index] = true;
        bound_admissions[binding.admission_index] = true;
    }

    const auto serialized = serialize_polyglot_invocation_request(request.invocation);
    if (!serialized.ok()) {
        return reject_without_telemetry(
            PolyglotArtifactInvocationStatus::invalid_request,
            serialized.error_code);
    }

    const auto policy_validation = validate_polyglot_bridge_policy(request.policy);
    if (!policy_validation.ok()) {
        return reject_before_launch(
            PolyglotArtifactInvocationStatus::invalid_request,
            policy_validation.error_code,
            capability_id,
            request.policy,
            PolyglotBridgeFailure::protocol_error);
    }
    if (request.policy.max_attempts != 1U) {
        return reject_before_launch(
            PolyglotArtifactInvocationStatus::invalid_request,
            "polyglot.adapter.multiple_attempts_unsupported",
            capability_id,
            request.policy,
            PolyglotBridgeFailure::protocol_error);
    }

    PolyglotArtifactInvocationResult result;
    result.request_document = serialized.document;
    BoundedProcessRequest process_request{
        .executable_path = admission.authorization().resolved_path,
        .arguments = request.artifact_arguments,
        .working_directory = request.working_directory,
        .environment = request.environment,
        .standard_input = result.request_document,
        .timeout_ms = request.policy.timeout_ms,
        .poll_interval_ms = request.poll_interval_ms,
        .stdin_limit_bytes = request.stdin_limit_bytes,
        .stdout_limit_bytes = request.stdout_limit_bytes,
        .stderr_limit_bytes = request.stderr_limit_bytes,
        .cancellation_requested = request.cancellation_requested};

    for (const auto& binding : supporting_artifact_arguments) {
        auto& supporting =
            supporting_artifact_admissions[binding.admission_index];
        if (!revalidate_polyglot_supporting_artifact_admission(supporting)) {
            result.status = PolyglotArtifactInvocationStatus::artifact_rejected;
            result.error_code = supporting.error_code();
            decide_and_record(
                result,
                capability_id,
                request.policy,
                PolyglotBridgeFailure::unavailable);
            return result;
        }
        if (process_request.arguments[binding.argument_index] !=
            supporting.resolved_path()) {
            result.status = PolyglotArtifactInvocationStatus::artifact_rejected;
            result.error_code =
                "polyglot.adapter.supporting_artifact_path_changed";
            decide_and_record(
                result,
                capability_id,
                request.policy,
                PolyglotBridgeFailure::unavailable);
            return result;
        }
    }

    // Supporting files are checked first and the executable last. No
    // allocation, policy decision, callback, or other user-controlled work
    // occurs after successful executable revalidation and before the owned
    // launch call.
    if (!revalidate_polyglot_artifact_admission(admission)) {
        result.status = PolyglotArtifactInvocationStatus::artifact_rejected;
        result.error_code = admission.error_code();
        decide_and_record(
            result,
            capability_id,
            request.policy,
            PolyglotBridgeFailure::unavailable);
        return result;
    }
    result.artifact_revalidated = true;
    if (process_request.executable_path != admission.authorization().resolved_path) {
        result.status = PolyglotArtifactInvocationStatus::artifact_rejected;
        result.error_code = "polyglot.adapter.artifact_path_changed";
        decide_and_record(
            result,
            capability_id,
            request.policy,
            PolyglotBridgeFailure::unavailable);
        return result;
    }
    result.process = run_bounded_process(process_request);

    switch (result.process.status) {
    case BoundedProcessStatus::cancelled:
        result.status = PolyglotArtifactInvocationStatus::cancelled;
        result.error_code = result.process.error_code;
        decide_and_record(
            result,
            capability_id,
            request.policy,
            PolyglotBridgeFailure::cancellation,
            true);
        return result;
    case BoundedProcessStatus::timed_out:
        result.status = PolyglotArtifactInvocationStatus::process_failed;
        result.error_code = result.process.error_code;
        decide_and_record(
            result,
            capability_id,
            request.policy,
            PolyglotBridgeFailure::timeout);
        return result;
    case BoundedProcessStatus::invalid_request:
        result.status = PolyglotArtifactInvocationStatus::invalid_request;
        result.error_code = result.process.error_code;
        decide_and_record(
            result,
            capability_id,
            request.policy,
            PolyglotBridgeFailure::protocol_error);
        return result;
    case BoundedProcessStatus::output_limit_exceeded:
        result.status = PolyglotArtifactInvocationStatus::process_failed;
        result.error_code = result.process.error_code;
        decide_and_record(
            result,
            capability_id,
            request.policy,
            PolyglotBridgeFailure::protocol_error);
        return result;
    case BoundedProcessStatus::launch_failed:
        result.status = PolyglotArtifactInvocationStatus::process_failed;
        result.error_code = result.process.error_code;
        decide_and_record(
            result,
            capability_id,
            request.policy,
            PolyglotBridgeFailure::unavailable);
        return result;
    case BoundedProcessStatus::exited:
        break;
    }

    if (result.process.exit_code != 0) {
        result.status = PolyglotArtifactInvocationStatus::process_failed;
        result.error_code = "polyglot.adapter.nonzero_exit";
        decide_and_record(
            result,
            capability_id,
            request.policy,
            PolyglotBridgeFailure::candidate_error);
        return result;
    }

    result.response = parse_polyglot_interop_envelope(
        result.process.standard_output,
        PolyglotInteropEnvelopeExpectation{
            .capability_id = capability_id,
            .correlation_id = request.invocation.correlation_id,
            .protocol_version = request.invocation.protocol_version,
            .max_document_bytes = request.stdout_limit_bytes,
            .max_nesting_depth = request.invocation.max_nesting_depth});
    if (!result.response.ok()) {
        result.status = PolyglotArtifactInvocationStatus::response_rejected;
        result.error_code = result.response.error_code;
        decide_and_record(
            result,
            capability_id,
            request.policy,
            PolyglotBridgeFailure::protocol_error);
        return result;
    }
    if (result.response.envelope.kind == PolyglotInteropEnvelopeKind::error) {
        result.status = PolyglotArtifactInvocationStatus::candidate_error;
        result.error_code = "polyglot.adapter.candidate_error";
        decide_and_record(
            result,
            capability_id,
            request.policy,
            PolyglotBridgeFailure::candidate_error);
        return result;
    }

    decide_and_record(
        result,
        capability_id,
        request.policy,
        PolyglotBridgeFailure::none);
    if (result.decision.outcome != PolyglotBridgeOutcome::success) {
        result.status = PolyglotArtifactInvocationStatus::latency_budget_exceeded;
        result.error_code = result.decision.error_code;
        return result;
    }
    result.status = PolyglotArtifactInvocationStatus::success;
    result.error_code = "polyglot.adapter.success";
    return result;
}

const char* polyglot_artifact_invocation_status_name(
    const PolyglotArtifactInvocationStatus status) noexcept {
    switch (status) {
    case PolyglotArtifactInvocationStatus::success:
        return "success";
    case PolyglotArtifactInvocationStatus::invalid_request:
        return "invalid-request";
    case PolyglotArtifactInvocationStatus::artifact_rejected:
        return "artifact-rejected";
    case PolyglotArtifactInvocationStatus::cancelled:
        return "cancelled";
    case PolyglotArtifactInvocationStatus::process_failed:
        return "process-failed";
    case PolyglotArtifactInvocationStatus::response_rejected:
        return "response-rejected";
    case PolyglotArtifactInvocationStatus::candidate_error:
        return "candidate-error";
    case PolyglotArtifactInvocationStatus::latency_budget_exceeded:
        return "latency-budget-exceeded";
    }
    return "invalid-request";
}

}  // namespace copperfin::platform
