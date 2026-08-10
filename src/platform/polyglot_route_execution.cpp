// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/polyglot_route_execution.h"

#include <exception>
#include <string>
#include <utility>

namespace copperfin::platform {

namespace {

bool canonical_capability_id(const std::string& capability_id) {
    return load_polyglot_route_registry({
        PolyglotRouteConfig{capability_id, "off", 0U}}).ok();
}

bool valid_route_state(const PolyglotRouteState state) noexcept {
    switch (state) {
    case PolyglotRouteState::off:
    case PolyglotRouteState::shadow:
    case PolyglotRouteState::canary:
    case PolyglotRouteState::on:
    case PolyglotRouteState::retire_legacy:
        return true;
    }
    return false;
}

bool valid_registry(const PolyglotRouteRegistry& registry) {
    std::vector<PolyglotRouteConfig> configs;
    configs.reserve(registry.entries.size());
    for (const PolyglotRouteEntry& entry : registry.entries) {
        if (!valid_route_state(entry.state)) {
            return false;
        }
        configs.push_back(PolyglotRouteConfig{
            entry.capability_id,
            polyglot_route_state_name(entry.state),
            entry.canary_percentage});
    }
    return load_polyglot_route_registry(configs).ok();
}

void append_telemetry(
    PolyglotMigrationTelemetryStream& destination,
    const PolyglotMigrationTelemetryStream& source) {
    destination.events.insert(
        destination.events.end(), source.events.begin(), source.events.end());
}

void record_execution_event(
    PolyglotRouteExecutionResult& result,
    const std::string& capability_id) {
    result.telemetry.events.push_back(
        PolyglotMigrationEvent{
            "polyglot.execution.completed",
            capability_id,
            result.error_code,
            polyglot_route_result_authority_name(result.authority),
            0U,
            0U,
            result.ok()});
}

void record_fallback_execution(
    PolyglotRouteExecutionResult& result,
    const std::string& capability_id) {
    result.telemetry.events.push_back(
        PolyglotMigrationEvent{
            "polyglot.fallback.executed",
            capability_id,
            result.native.success
                ? "polyglot.execution.native_fallback_success"
                : "polyglot.execution.native_fallback_failed",
            "native",
            0U,
            0U,
            result.native.success});
}

void invoke_native_once(
    const PolyglotRouteExecutionRequest& request,
    PolyglotRouteExecutionResult& result) {
    ++result.native_invocation_count;
    try {
        result.native = request.invoke_native();
        if (!result.native.success && result.native.error_code.empty()) {
            result.native.error_code = "polyglot.execution.native_failed";
        }
    } catch (const std::exception&) {
        result.native = {
            false, "polyglot.execution.native_exception", {}};
    } catch (...) {
        result.native = {
            false, "polyglot.execution.native_exception", {}};
    }
}

void invoke_candidate_once(
    const PolyglotRouteExecutionRequest& request,
    PolyglotRouteExecutionResult& result) {
    ++result.candidate_invocation_count;
    result.candidate = invoke_polyglot_artifact(
        *request.artifact_admission, request.candidate_request);
    append_telemetry(result.telemetry, result.candidate.telemetry);
}

void finish_native(PolyglotRouteExecutionResult& result) {
    result.authority = PolyglotRouteResultAuthority::native;
    if (result.native.success) {
        result.status = PolyglotRouteExecutionStatus::success;
        result.error_code = "polyglot.execution.native_success";
    } else {
        result.status = PolyglotRouteExecutionStatus::native_failed;
        result.error_code = result.native.error_code;
    }
}

void finish_candidate(PolyglotRouteExecutionResult& result) {
    result.authority = PolyglotRouteResultAuthority::candidate;
    if (result.candidate.ok()) {
        result.status = PolyglotRouteExecutionStatus::success;
        result.error_code = "polyglot.execution.candidate_success";
    } else if (result.candidate.status == PolyglotArtifactInvocationStatus::cancelled) {
        result.status = PolyglotRouteExecutionStatus::cancelled;
        result.error_code = result.candidate.error_code;
    } else {
        result.status = PolyglotRouteExecutionStatus::candidate_failed;
        result.error_code = result.candidate.decision.use_artifact_fallback
            ? "polyglot.execution.artifact_fallback_unsupported"
            : result.candidate.error_code;
    }
}

bool request_is_valid(
    const PolyglotRouteExecutionRequest& request,
    const PolyglotRouteDecision& route,
    std::string& error_code) {
    if (route.invoke_candidate && request.artifact_admission == nullptr) {
        error_code = "polyglot.execution.artifact_admission_required";
        return false;
    }
    const bool fallback_may_need_native = route.candidate_primary &&
        route.native_fallback_allowed &&
        request.candidate_request.policy.fallback ==
            PolyglotFallbackPolicy::fallback_native;
    if ((route.invoke_native || fallback_may_need_native) && !request.invoke_native) {
        error_code = "polyglot.execution.native_invoker_required";
        return false;
    }
    if (route.selection == PolyglotRouteSelection::shadow &&
        !request.normalize_shadow_parity) {
        error_code = "polyglot.execution.shadow_normalizer_required";
        return false;
    }
    return true;
}

void compare_shadow(
    const PolyglotRouteExecutionRequest& request,
    PolyglotRouteExecutionResult& result) {
    try {
        PolyglotShadowParityValues values = request.normalize_shadow_parity(
            result.native, result.candidate);
        PolyglotParityComparisonRequest comparison;
        comparison.capability_id = request.capability_id;
        comparison.native_success = result.native.success;
        comparison.candidate_success = result.candidate.ok();
        comparison.native_error_code = result.native.error_code;
        comparison.candidate_error_code = result.candidate.error_code;
        comparison.fields = std::move(values.fields);
        comparison.native_order = std::move(values.native_order);
        comparison.candidate_order = std::move(values.candidate_order);
        result.parity = compare_polyglot_outputs(
            request.parity_policy, comparison);
        result.parity_evaluated = true;
        record_polyglot_parity_events(result.telemetry, comparison, result.parity);
    } catch (const std::exception&) {
        result.status = PolyglotRouteExecutionStatus::parity_failed;
        result.error_code = "polyglot.execution.shadow_normalizer_exception";
    } catch (...) {
        result.status = PolyglotRouteExecutionStatus::parity_failed;
        result.error_code = "polyglot.execution.shadow_normalizer_exception";
    }
}

}  // namespace

PolyglotRouteExecutionResult execute_polyglot_route(
    const PolyglotRouteExecutionRequest& request) {
    PolyglotRouteExecutionResult result;
    if (request.registry == nullptr) {
        result.error_code = "polyglot.execution.registry_required";
        return result;
    }

    if (!canonical_capability_id(request.capability_id)) {
        result.error_code = "polyglot.execution.invalid_capability_id";
        return result;
    }
    if (request.selection_sample > 99U) {
        result.error_code = "polyglot.execution.invalid_selection_sample";
        return result;
    }
    if (!valid_registry(*request.registry)) {
        result.error_code = "polyglot.execution.invalid_registry";
        return result;
    }

    result.route = evaluate_polyglot_route(
        *request.registry, request.capability_id, request.selection_sample);
    if (!request_is_valid(request, result.route, result.error_code)) {
        return result;
    }
    record_polyglot_route_event(
        result.telemetry, request.capability_id, result.route);

    if (result.route.selection == PolyglotRouteSelection::native) {
        invoke_native_once(request, result);
        finish_native(result);
        record_execution_event(result, request.capability_id);
        return result;
    }

    if (result.route.selection == PolyglotRouteSelection::shadow) {
        invoke_native_once(request, result);
        invoke_candidate_once(request, result);
        finish_native(result);
        compare_shadow(request, result);
        record_execution_event(result, request.capability_id);
        return result;
    }

    invoke_candidate_once(request, result);
    if (!result.candidate.ok() &&
        result.candidate.decision.use_native_fallback &&
        result.route.native_fallback_allowed) {
        invoke_native_once(request, result);
        result.native_fallback_executed = true;
        finish_native(result);
        record_fallback_execution(result, request.capability_id);
    } else {
        finish_candidate(result);
    }
    record_execution_event(result, request.capability_id);
    return result;
}

const char* polyglot_route_execution_status_name(
    const PolyglotRouteExecutionStatus status) noexcept {
    switch (status) {
    case PolyglotRouteExecutionStatus::success:
        return "success";
    case PolyglotRouteExecutionStatus::invalid_request:
        return "invalid-request";
    case PolyglotRouteExecutionStatus::native_failed:
        return "native-failed";
    case PolyglotRouteExecutionStatus::candidate_failed:
        return "candidate-failed";
    case PolyglotRouteExecutionStatus::cancelled:
        return "cancelled";
    case PolyglotRouteExecutionStatus::parity_failed:
        return "parity-failed";
    }
    return "invalid-request";
}

const char* polyglot_route_result_authority_name(
    const PolyglotRouteResultAuthority authority) noexcept {
    switch (authority) {
    case PolyglotRouteResultAuthority::none:
        return "none";
    case PolyglotRouteResultAuthority::native:
        return "native";
    case PolyglotRouteResultAuthority::candidate:
        return "candidate";
    }
    return "none";
}

}  // namespace copperfin::platform
