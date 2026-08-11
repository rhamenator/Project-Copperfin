// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/platform/bounded_process.h"
#include "copperfin/platform/polyglot_artifact_admission.h"
#include "copperfin/platform/polyglot_bridge_invocation.h"
#include "copperfin/platform/polyglot_interop_envelope.h"
#include "copperfin/platform/polyglot_migration_telemetry.h"
#include "copperfin/platform/polyglot_supporting_artifact_admission.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace copperfin::platform {

struct PolyglotSupportingArtifactArgumentBinding {
    std::size_t argument_index = 0U;
    std::size_t admission_index = 0U;
};

enum class PolyglotArtifactInvocationStatus : std::uint8_t {
    success,
    invalid_request,
    artifact_rejected,
    cancelled,
    process_failed,
    response_rejected,
    candidate_error,
    latency_budget_exceeded
};

struct PolyglotArtifactInvocationRequest {
    PolyglotInteropInvocationRequest invocation;
    PolyglotBridgeInvocationPolicy policy;
    std::vector<std::string> artifact_arguments;
    std::string working_directory;
    // Complete child environment. Ambient host/agent variables are never
    // inherited by this adapter.
    std::vector<BoundedProcessEnvironmentVariable> environment;
    std::uint32_t poll_interval_ms = 10U;
    std::uint32_t stdin_limit_bytes = 1024U * 1024U;
    std::uint32_t stdout_limit_bytes = 1024U * 1024U;
    std::uint32_t stderr_limit_bytes = 1024U * 1024U;
    std::function<bool()> cancellation_requested;
};

struct PolyglotArtifactInvocationResult {
    PolyglotArtifactInvocationStatus status =
        PolyglotArtifactInvocationStatus::invalid_request;
    bool artifact_revalidated = false;
    std::string error_code = "polyglot.adapter.invalid_request";
    std::string request_document;
    BoundedProcessResult process;
    PolyglotInteropEnvelopeResult response;
    PolyglotBridgeInvocationDecision decision;
    PolyglotMigrationTelemetryStream telemetry;

    [[nodiscard]] bool ok() const noexcept {
        return status == PolyglotArtifactInvocationStatus::success &&
            decision.outcome == PolyglotBridgeOutcome::success && response.ok();
    }
};

// Connects one previously admitted executable plus any exact-position-bound
// supporting files to the v1 stdin/stdout JSON protocol. The tokens are
// revalidated immediately beside the owned launch call. This adapter executes
// exactly one candidate attempt; it reports the configured fallback decision
// but never invokes a fallback, native path, route registry, or mutable
// PRG/runtime callback itself.
[[nodiscard]] PolyglotArtifactInvocationResult invoke_polyglot_artifact(
    PolyglotArtifactAdmissionResult& admission,
    const PolyglotArtifactInvocationRequest& request);

[[nodiscard]] PolyglotArtifactInvocationResult invoke_polyglot_artifact(
    PolyglotArtifactAdmissionResult& admission,
    std::vector<PolyglotSupportingArtifactAdmissionResult>&
        supporting_artifact_admissions,
    const std::vector<PolyglotSupportingArtifactArgumentBinding>&
        supporting_artifact_arguments,
    const PolyglotArtifactInvocationRequest& request);

[[nodiscard]] const char* polyglot_artifact_invocation_status_name(
    PolyglotArtifactInvocationStatus status) noexcept;

}  // namespace copperfin::platform
