// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::platform {

enum class PolyglotRouteImplementation : std::uint8_t {
    direct_cpp,
    cpp_dotnet_wrapper,
    csharp_service
};

enum class PolyglotRouteSecurityProfile : std::uint8_t {
    trusted_native,
    admitted_process,
    remote_service
};

enum class PolyglotRouteImpactError : std::uint8_t {
    none,
    invalid_capability_id,
    invalid_policy,
    wrong_route_count,
    duplicate_route,
    invalid_measurement,
    no_eligible_route
};

struct PolyglotRouteImpactWeights {
    std::uint16_t latency = 25U;
    std::uint16_t throughput = 25U;
    std::uint16_t memory = 25U;
    std::uint16_t startup = 25U;
};

struct PolyglotRouteImpactPolicy {
    std::uint32_t minimum_sample_count = 30U;
    std::uint64_t maximum_p95_latency_us = 0U;
    std::uint64_t minimum_throughput_per_second = 0U;
    std::uint64_t maximum_peak_memory_kib = 0U;
    std::uint64_t maximum_p95_startup_ms = 0U;
    PolyglotRouteSecurityProfile maximum_security_profile =
        PolyglotRouteSecurityProfile::admitted_process;
    PolyglotRouteImpactWeights weights;
};

struct PolyglotRouteImpactMeasurement {
    PolyglotRouteImplementation implementation =
        PolyglotRouteImplementation::direct_cpp;
    PolyglotRouteSecurityProfile security_profile =
        PolyglotRouteSecurityProfile::trusted_native;
    bool runtime_available = false;
    bool security_approved = false;
    bool contract_compatible = false;
    std::uint32_t sample_count = 0U;
    std::uint32_t failure_count = 0U;
    std::uint32_t parity_mismatch_count = 0U;
    std::uint64_t p95_latency_us = 0U;
    std::uint64_t throughput_per_second = 0U;
    std::uint64_t peak_memory_kib = 0U;
    std::uint64_t p95_startup_ms = 0U;
};

struct PolyglotRouteImpactAssessment {
    PolyglotRouteImplementation implementation =
        PolyglotRouteImplementation::direct_cpp;
    bool eligible = false;
    std::uint64_t impact_score = 0U;
    std::string reason_code;
};

struct PolyglotRouteImpactRequest {
    std::string capability_id;
    PolyglotRouteImpactPolicy policy;
    std::vector<PolyglotRouteImpactMeasurement> measurements;
};

struct PolyglotRouteImpactResult {
    PolyglotRouteImpactError error = PolyglotRouteImpactError::none;
    std::string error_code;
    bool recommendation_ready = false;
    PolyglotRouteImplementation preferred =
        PolyglotRouteImplementation::direct_cpp;
    std::vector<PolyglotRouteImplementation> fallback_chain;
    std::vector<PolyglotRouteImpactAssessment> assessments;

    [[nodiscard]] bool ok() const noexcept {
        return error == PolyglotRouteImpactError::none;
    }
};

[[nodiscard]] const char* polyglot_route_implementation_name(
    PolyglotRouteImplementation implementation) noexcept;
[[nodiscard]] const char* polyglot_route_security_profile_name(
    PolyglotRouteSecurityProfile profile) noexcept;

// Evaluates already-captured benchmark evidence. This function is advisory:
// it does not mutate a route registry, invoke a route, or promote traffic.
[[nodiscard]] PolyglotRouteImpactResult evaluate_polyglot_route_impact(
    const PolyglotRouteImpactRequest& request);

}  // namespace copperfin::platform
