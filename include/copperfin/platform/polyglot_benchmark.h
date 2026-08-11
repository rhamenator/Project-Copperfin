// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/platform/polyglot_route_impact.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace copperfin::platform {

enum class PolyglotBenchmarkError : std::uint8_t {
    none,
    invalid_request,
    invalid_observation,
    invocation_failed
};

struct PolyglotBenchmarkWorkload {
    std::string workload_id;
    std::string arguments_json;
    std::string expected_payload_json;
};

struct PolyglotBenchmarkRoute {
    PolyglotRouteImplementation implementation =
        PolyglotRouteImplementation::direct_cpp;
    PolyglotRouteSecurityProfile security_profile =
        PolyglotRouteSecurityProfile::trusted_native;
    bool runtime_available = false;
    bool security_approved = false;
    bool contract_compatible = false;
};

struct PolyglotBenchmarkObservation {
    bool invocation_succeeded = false;
    bool parity_matched = false;
    std::uint64_t latency_us = 0U;
    std::uint64_t peak_memory_kib = 0U;
    std::uint64_t startup_ms = 0U;
};

struct PolyglotBenchmarkInvocation {
    PolyglotRouteImplementation implementation =
        PolyglotRouteImplementation::direct_cpp;
    const PolyglotBenchmarkWorkload* workload = nullptr;
    std::uint32_t iteration = 0U;
    bool warmup = false;
};

using PolyglotBenchmarkInvoke =
    std::function<PolyglotBenchmarkObservation(const PolyglotBenchmarkInvocation&)>;

struct PolyglotBenchmarkRequest {
    std::string capability_id;
    std::uint32_t warmup_iterations = 3U;
    std::uint32_t measured_iterations = 10U;
    PolyglotRouteImpactPolicy policy;
    std::vector<PolyglotBenchmarkWorkload> workloads;
    std::vector<PolyglotBenchmarkRoute> routes;
    PolyglotBenchmarkInvoke invoke;
};

struct PolyglotBenchmarkResult {
    PolyglotBenchmarkError error = PolyglotBenchmarkError::none;
    std::string error_code;
    std::vector<PolyglotRouteImpactMeasurement> measurements;
    PolyglotRouteImpactResult impact;

    [[nodiscard]] bool ok() const noexcept {
        return error == PolyglotBenchmarkError::none && impact.ok();
    }
};

// Runs a bounded, caller-supplied representative workload and aggregates its
// observations for the advisory route-impact evaluator. This function owns no
// registry, artifact, network, or promotion authority.
[[nodiscard]] PolyglotBenchmarkResult run_polyglot_benchmark(
    const PolyglotBenchmarkRequest& request);

}  // namespace copperfin::platform
