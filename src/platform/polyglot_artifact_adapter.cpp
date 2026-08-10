// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/polyglot_artifact_adapter.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>

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

}  // namespace

PolyglotArtifactInvocationResult invoke_polyglot_artifact(
    PolyglotArtifactAdmissionResult& admission,
    const PolyglotArtifactInvocationRequest& request) {
    const std::string capability_id = request.invocation.capability_id;
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
    if (!admission.ok()) {
        return reject_before_launch(
            PolyglotArtifactInvocationStatus::artifact_rejected,
            admission.error_code().empty()
                ? std::string("polyglot.adapter.artifact_not_admitted")
                : admission.error_code(),
            capability_id,
            request.policy,
            PolyglotBridgeFailure::unavailable);
    }
    if (capability_id != admission.capability_id()) {
        return reject_before_launch(
            PolyglotArtifactInvocationStatus::artifact_rejected,
            "polyglot.adapter.capability_id_mismatch",
            capability_id,
            request.policy,
            PolyglotBridgeFailure::protocol_error);
    }

    const auto serialized = serialize_polyglot_invocation_request(request.invocation);
    if (!serialized.ok()) {
        return reject_before_launch(
            PolyglotArtifactInvocationStatus::invalid_request,
            serialized.error_code,
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

    // No allocation, policy decision, callback, or other user-controlled work
    // occurs between successful revalidation and the owned launch call.
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
