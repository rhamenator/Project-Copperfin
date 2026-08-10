// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/platform/polyglot_artifact_adapter.h"
#include "copperfin/platform/polyglot_parity_comparator.h"
#include "copperfin/platform/polyglot_route_registry.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace copperfin::platform {

enum class PolyglotRouteExecutionStatus : std::uint8_t {
    success,
    invalid_request,
    native_failed,
    candidate_failed,
    cancelled,
    parity_failed
};

enum class PolyglotRouteResultAuthority : std::uint8_t {
    none,
    native,
    candidate
};

struct PolyglotNativeInvocationResult {
    bool success = false;
    std::string error_code;
    // Opaque caller-owned result bytes. The coordinator does not parse or
    // reinterpret native runtime state.
    std::string payload;
};

struct PolyglotShadowParityValues {
    std::vector<PolyglotParityField> fields;
    std::vector<std::string> native_order;
    std::vector<std::string> candidate_order;
};

using PolyglotNativeInvoker = std::function<PolyglotNativeInvocationResult()>;
using PolyglotShadowParityNormalizer = std::function<PolyglotShadowParityValues(
    const PolyglotNativeInvocationResult&,
    const PolyglotArtifactInvocationResult&)>;

struct PolyglotRouteExecutionRequest {
    const PolyglotRouteRegistry* registry = nullptr;
    std::string capability_id;
    std::uint8_t selection_sample = 0U;
    PolyglotArtifactAdmissionResult* artifact_admission = nullptr;
    PolyglotArtifactInvocationRequest candidate_request;
    PolyglotNativeInvoker invoke_native;
    PolyglotShadowParityNormalizer normalize_shadow_parity;
    PolyglotParityPolicy parity_policy;
};

struct PolyglotRouteExecutionResult {
    PolyglotRouteExecutionStatus status =
        PolyglotRouteExecutionStatus::invalid_request;
    PolyglotRouteResultAuthority authority = PolyglotRouteResultAuthority::none;
    std::string error_code = "polyglot.execution.invalid_request";
    PolyglotRouteDecision route;
    PolyglotNativeInvocationResult native;
    PolyglotArtifactInvocationResult candidate;
    PolyglotParityComparisonResult parity;
    bool parity_evaluated = false;
    bool native_fallback_executed = false;
    std::uint32_t native_invocation_count = 0U;
    std::uint32_t candidate_invocation_count = 0U;
    PolyglotMigrationTelemetryStream telemetry;

    [[nodiscard]] bool ok() const noexcept {
        return status == PolyglotRouteExecutionStatus::success;
    }
};

// Applies one already-validated registry decision. Native work is invoked
// synchronously through the caller-owned callback on this calling thread.
// Candidate work can run only through invoke_polyglot_artifact. No retry or
// second-artifact fallback is performed.
[[nodiscard]] PolyglotRouteExecutionResult execute_polyglot_route(
    const PolyglotRouteExecutionRequest& request);

[[nodiscard]] const char* polyglot_route_execution_status_name(
    PolyglotRouteExecutionStatus status) noexcept;
[[nodiscard]] const char* polyglot_route_result_authority_name(
    PolyglotRouteResultAuthority authority) noexcept;

}  // namespace copperfin::platform
