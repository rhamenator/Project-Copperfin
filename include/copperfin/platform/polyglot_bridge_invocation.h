// Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include <cstdint>
#include <string>

namespace copperfin::platform {

enum class PolyglotBridgeFailure {
    none,
    timeout,
    cancellation,
    candidate_error,
    protocol_error,
    unavailable
};

enum class PolyglotCancellationPolicy {
    propagate,
    ignore
};

enum class PolyglotFallbackPolicy {
    fail_fast,
    fallback_native,
    fallback_artifact
};

enum class PolyglotBridgePolicyError {
    none,
    timeout_required,
    latency_budget_required,
    latency_budget_exceeds_timeout,
    cancellation_policy_invalid,
    fallback_policy_invalid,
    max_attempts_required
};

enum class PolyglotBridgeOutcome {
    success,
    cancelled,
    failed,
    fallback_native,
    fallback_artifact
};

struct PolyglotBridgeInvocationPolicy {
    std::uint32_t timeout_ms = 5000U;
    std::uint32_t latency_budget_ms = 4500U;
    PolyglotCancellationPolicy cancellation = PolyglotCancellationPolicy::propagate;
    PolyglotFallbackPolicy fallback = PolyglotFallbackPolicy::fail_fast;
    std::uint32_t max_attempts = 1U;
};

struct PolyglotBridgePolicyValidation {
    PolyglotBridgePolicyError error = PolyglotBridgePolicyError::none;
    std::string error_code;

    [[nodiscard]] bool ok() const noexcept {
        return error == PolyglotBridgePolicyError::none;
    }
};

struct PolyglotBridgeInvocationRequest {
    std::uint32_t elapsed_ms = 0U;
    bool cancellation_requested = false;
    PolyglotBridgeFailure failure = PolyglotBridgeFailure::none;
};

struct PolyglotBridgeInvocationDecision {
    PolyglotBridgeOutcome outcome = PolyglotBridgeOutcome::failed;
    PolyglotBridgeFailure failure = PolyglotBridgeFailure::none;
    bool cancellation_propagated = false;
    bool use_native_fallback = false;
    bool use_artifact_fallback = false;
    std::uint32_t max_attempts = 0U;
    std::string error_code;
};

[[nodiscard]] PolyglotBridgePolicyValidation validate_polyglot_bridge_policy(
    const PolyglotBridgeInvocationPolicy& policy);
[[nodiscard]] PolyglotBridgeInvocationDecision evaluate_polyglot_bridge_invocation(
    const PolyglotBridgeInvocationPolicy& policy,
    const PolyglotBridgeInvocationRequest& request);
[[nodiscard]] const char* polyglot_bridge_failure_name(PolyglotBridgeFailure failure) noexcept;
[[nodiscard]] const char* polyglot_fallback_policy_name(PolyglotFallbackPolicy policy) noexcept;
[[nodiscard]] const char* polyglot_cancellation_policy_name(
    PolyglotCancellationPolicy policy) noexcept;

}  // namespace copperfin::platform
