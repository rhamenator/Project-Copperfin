// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/platform/polyglot_bridge_invocation.h"

#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

using copperfin::platform::PolyglotBridgeFailure;
using copperfin::platform::PolyglotBridgeOutcome;
using copperfin::platform::PolyglotCancellationPolicy;
using copperfin::platform::PolyglotFallbackPolicy;

void test_policy_validation() {
    copperfin::platform::PolyglotBridgeInvocationPolicy policy;
    expect(copperfin::platform::validate_polyglot_bridge_policy(policy).ok(),
           "default bridge policy should be valid");
    policy.timeout_ms = 0U;
    expect(copperfin::platform::validate_polyglot_bridge_policy(policy).error_code ==
               "polyglot.bridge.policy.timeout_required",
           "zero timeout should have a stable validation error");
    policy = {};
    policy.latency_budget_ms = 6000U;
    expect(copperfin::platform::validate_polyglot_bridge_policy(policy).error_code ==
               "polyglot.bridge.policy.latency_budget_exceeds_timeout",
           "latency budget above timeout should be rejected");
    policy = {};
    policy.max_attempts = 0U;
    expect(copperfin::platform::validate_polyglot_bridge_policy(policy).error_code ==
               "polyglot.bridge.policy.max_attempts_required",
           "zero max attempts should be rejected");
}

void test_success_and_failure_mapping() {
    const copperfin::platform::PolyglotBridgeInvocationPolicy policy{};
    const auto success = copperfin::platform::evaluate_polyglot_bridge_invocation(
        policy, {});
    expect(success.outcome == PolyglotBridgeOutcome::success &&
               success.failure == PolyglotBridgeFailure::none &&
               success.error_code == "polyglot.bridge.success",
           "successful invocation should map to stable success state");

    const auto timeout = copperfin::platform::evaluate_polyglot_bridge_invocation(
        policy, {.elapsed_ms = 5000U});
    expect(timeout.outcome == PolyglotBridgeOutcome::failed &&
               timeout.failure == PolyglotBridgeFailure::timeout &&
               timeout.error_code == "polyglot.bridge.timeout",
           "timeout should fail fast under the default policy");

    const auto latency = copperfin::platform::evaluate_polyglot_bridge_invocation(
        policy, {.elapsed_ms = 4500U});
    expect(latency.error_code == "polyglot.bridge.latency_budget_exceeded",
           "latency budget exhaustion should have a distinct error code");

    const PolyglotBridgeFailure failures_to_map[] = {
        PolyglotBridgeFailure::candidate_error,
        PolyglotBridgeFailure::protocol_error,
        PolyglotBridgeFailure::unavailable};
    const char* expected_codes[] = {
        "polyglot.bridge.candidate_error",
        "polyglot.bridge.protocol_error",
        "polyglot.bridge.unavailable"};
    for (std::size_t index = 0U; index < 3U; ++index) {
        const auto decision = copperfin::platform::evaluate_polyglot_bridge_invocation(
            policy, {.failure = failures_to_map[index]});
        expect(decision.outcome == PolyglotBridgeOutcome::failed &&
                   decision.error_code == expected_codes[index],
               "candidate failure class should map deterministically");
    }
}

void test_fallback_and_cancellation_matrix() {
    auto policy = copperfin::platform::PolyglotBridgeInvocationPolicy{
        .timeout_ms = 100U,
        .latency_budget_ms = 90U,
        .cancellation = PolyglotCancellationPolicy::propagate,
        .fallback = PolyglotFallbackPolicy::fallback_native,
        .max_attempts = 2U};
    const auto timeout = copperfin::platform::evaluate_polyglot_bridge_invocation(
        policy, {.elapsed_ms = 100U});
    expect(timeout.outcome == PolyglotBridgeOutcome::fallback_native &&
               timeout.use_native_fallback && !timeout.use_artifact_fallback &&
               timeout.max_attempts == 2U,
           "fallback-native should select native after timeout");

    policy.fallback = PolyglotFallbackPolicy::fallback_artifact;
    const auto unavailable = copperfin::platform::evaluate_polyglot_bridge_invocation(
        policy, {.failure = PolyglotBridgeFailure::unavailable});
    expect(unavailable.outcome == PolyglotBridgeOutcome::fallback_artifact &&
               unavailable.use_artifact_fallback,
           "fallback-artifact should select the artifact path after failure");

    const auto propagated = copperfin::platform::evaluate_polyglot_bridge_invocation(
        policy, {.cancellation_requested = true});
    expect(propagated.outcome == PolyglotBridgeOutcome::cancelled &&
               propagated.cancellation_propagated && !propagated.use_artifact_fallback,
           "propagated cancellation should not invoke a fallback");

    policy.cancellation = PolyglotCancellationPolicy::ignore;
    const auto ignored = copperfin::platform::evaluate_polyglot_bridge_invocation(
        policy, {.cancellation_requested = true});
    expect(ignored.outcome == PolyglotBridgeOutcome::fallback_artifact &&
               !ignored.cancellation_propagated && ignored.use_artifact_fallback,
           "ignored cancellation should use the configured fallback");
}

}  // namespace

int main() {
    test_policy_validation();
    test_success_and_failure_mapping();
    test_fallback_and_cancellation_matrix();
    if (failures != 0) {
        std::cerr << failures << " polyglot bridge invocation test(s) failed\n";
        return 1;
    }
    std::cout << "All polyglot bridge invocation tests passed\n";
    return 0;
}
