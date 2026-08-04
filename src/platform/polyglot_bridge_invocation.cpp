// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/polyglot_bridge_invocation.h"

namespace copperfin::platform {

namespace {

PolyglotBridgePolicyValidation invalid_policy(
    PolyglotBridgePolicyError error,
    const char* error_code) {
    PolyglotBridgePolicyValidation result;
    result.error = error;
    result.error_code = error_code;
    return result;
}

bool valid_cancellation_policy(PolyglotCancellationPolicy policy) noexcept {
    return policy == PolyglotCancellationPolicy::propagate ||
        policy == PolyglotCancellationPolicy::ignore;
}

bool valid_fallback_policy(PolyglotFallbackPolicy policy) noexcept {
    return policy == PolyglotFallbackPolicy::fail_fast ||
        policy == PolyglotFallbackPolicy::fallback_native ||
        policy == PolyglotFallbackPolicy::fallback_artifact;
}

const char* failure_error_code(PolyglotBridgeFailure failure) noexcept {
    switch (failure) {
    case PolyglotBridgeFailure::none:
        return "polyglot.bridge.success";
    case PolyglotBridgeFailure::timeout:
        return "polyglot.bridge.timeout";
    case PolyglotBridgeFailure::cancellation:
        return "polyglot.bridge.cancelled";
    case PolyglotBridgeFailure::candidate_error:
        return "polyglot.bridge.candidate_error";
    case PolyglotBridgeFailure::protocol_error:
        return "polyglot.bridge.protocol_error";
    case PolyglotBridgeFailure::unavailable:
        return "polyglot.bridge.unavailable";
    }
    return "polyglot.bridge.invalid_failure";
}

void apply_fallback(
    const PolyglotBridgeInvocationPolicy& policy,
    PolyglotBridgeInvocationDecision& decision) {
    switch (policy.fallback) {
    case PolyglotFallbackPolicy::fail_fast:
        decision.outcome = PolyglotBridgeOutcome::failed;
        break;
    case PolyglotFallbackPolicy::fallback_native:
        decision.outcome = PolyglotBridgeOutcome::fallback_native;
        decision.use_native_fallback = true;
        break;
    case PolyglotFallbackPolicy::fallback_artifact:
        decision.outcome = PolyglotBridgeOutcome::fallback_artifact;
        decision.use_artifact_fallback = true;
        break;
    }
}

}  // namespace

PolyglotBridgePolicyValidation validate_polyglot_bridge_policy(
    const PolyglotBridgeInvocationPolicy& policy) {
    if (policy.timeout_ms == 0U) {
        return invalid_policy(
            PolyglotBridgePolicyError::timeout_required,
            "polyglot.bridge.policy.timeout_required");
    }
    if (policy.latency_budget_ms == 0U) {
        return invalid_policy(
            PolyglotBridgePolicyError::latency_budget_required,
            "polyglot.bridge.policy.latency_budget_required");
    }
    if (policy.latency_budget_ms > policy.timeout_ms) {
        return invalid_policy(
            PolyglotBridgePolicyError::latency_budget_exceeds_timeout,
            "polyglot.bridge.policy.latency_budget_exceeds_timeout");
    }
    if (!valid_cancellation_policy(policy.cancellation)) {
        return invalid_policy(
            PolyglotBridgePolicyError::cancellation_policy_invalid,
            "polyglot.bridge.policy.cancellation_invalid");
    }
    if (!valid_fallback_policy(policy.fallback)) {
        return invalid_policy(
            PolyglotBridgePolicyError::fallback_policy_invalid,
            "polyglot.bridge.policy.fallback_invalid");
    }
    if (policy.max_attempts == 0U) {
        return invalid_policy(
            PolyglotBridgePolicyError::max_attempts_required,
            "polyglot.bridge.policy.max_attempts_required");
    }
    return {};
}

PolyglotBridgeInvocationDecision evaluate_polyglot_bridge_invocation(
    const PolyglotBridgeInvocationPolicy& policy,
    const PolyglotBridgeInvocationRequest& request) {
    PolyglotBridgeInvocationDecision decision;
    decision.max_attempts = policy.max_attempts;
    const auto policy_validation = validate_polyglot_bridge_policy(policy);
    if (!policy_validation.ok()) {
        decision.failure = PolyglotBridgeFailure::protocol_error;
        decision.error_code = policy_validation.error_code;
        return decision;
    }

    if (request.cancellation_requested) {
        decision.failure = PolyglotBridgeFailure::cancellation;
        decision.error_code = failure_error_code(decision.failure);
        if (policy.cancellation == PolyglotCancellationPolicy::propagate) {
            decision.outcome = PolyglotBridgeOutcome::cancelled;
            decision.cancellation_propagated = true;
            return decision;
        }
        apply_fallback(policy, decision);
        return decision;
    }

    if (request.elapsed_ms >= policy.timeout_ms) {
        decision.failure = PolyglotBridgeFailure::timeout;
    } else if (request.elapsed_ms >= policy.latency_budget_ms) {
        decision.failure = PolyglotBridgeFailure::timeout;
        decision.error_code = "polyglot.bridge.latency_budget_exceeded";
    } else {
        decision.failure = request.failure;
    }

    if (decision.failure == PolyglotBridgeFailure::none) {
        decision.outcome = PolyglotBridgeOutcome::success;
        decision.error_code = failure_error_code(decision.failure);
        return decision;
    }
    if (decision.error_code.empty()) {
        decision.error_code = failure_error_code(decision.failure);
    }
    apply_fallback(policy, decision);
    return decision;
}

const char* polyglot_bridge_failure_name(PolyglotBridgeFailure failure) noexcept {
    switch (failure) {
    case PolyglotBridgeFailure::none:
        return "none";
    case PolyglotBridgeFailure::timeout:
        return "timeout";
    case PolyglotBridgeFailure::cancellation:
        return "cancellation";
    case PolyglotBridgeFailure::candidate_error:
        return "candidate-error";
    case PolyglotBridgeFailure::protocol_error:
        return "protocol-error";
    case PolyglotBridgeFailure::unavailable:
        return "unavailable";
    }
    return "protocol-error";
}

const char* polyglot_fallback_policy_name(PolyglotFallbackPolicy policy) noexcept {
    switch (policy) {
    case PolyglotFallbackPolicy::fail_fast:
        return "fail-fast";
    case PolyglotFallbackPolicy::fallback_native:
        return "fallback-native";
    case PolyglotFallbackPolicy::fallback_artifact:
        return "fallback-artifact";
    }
    return "fail-fast";
}

const char* polyglot_cancellation_policy_name(PolyglotCancellationPolicy policy) noexcept {
    switch (policy) {
    case PolyglotCancellationPolicy::propagate:
        return "propagate";
    case PolyglotCancellationPolicy::ignore:
        return "ignore";
    }
    return "propagate";
}

}  // namespace copperfin::platform
