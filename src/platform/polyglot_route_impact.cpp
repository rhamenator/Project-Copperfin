// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/polyglot_route_impact.h"

#include "copperfin/platform/polyglot_route_registry.h"

#include <algorithm>
#include <array>
#include <limits>
#include <utility>

namespace copperfin::platform {

namespace {

constexpr std::uint64_t kMaximumMetricValue = 1'000'000'000'000ULL;
constexpr std::uint64_t kBasisPoints = 10'000U;

std::size_t implementation_index(
    const PolyglotRouteImplementation implementation) noexcept {
    switch (implementation) {
    case PolyglotRouteImplementation::direct_cpp:
        return 0U;
    case PolyglotRouteImplementation::cpp_dotnet_wrapper:
        return 1U;
    case PolyglotRouteImplementation::csharp_service:
        return 2U;
    }
    return 3U;
}

bool valid_implementation(
    const PolyglotRouteImplementation implementation) noexcept {
    return implementation_index(implementation) < 3U;
}

bool valid_security_profile(
    const PolyglotRouteSecurityProfile profile) noexcept {
    switch (profile) {
    case PolyglotRouteSecurityProfile::trusted_native:
    case PolyglotRouteSecurityProfile::admitted_process:
    case PolyglotRouteSecurityProfile::remote_service:
        return true;
    }
    return false;
}

PolyglotRouteImpactResult invalid_result(
    const PolyglotRouteImpactError error,
    const char* error_code) {
    PolyglotRouteImpactResult result;
    result.error = error;
    result.error_code = error_code;
    return result;
}

bool valid_policy(const PolyglotRouteImpactPolicy& policy) noexcept {
    const std::uint64_t total_weight =
        static_cast<std::uint64_t>(policy.weights.latency) +
        static_cast<std::uint64_t>(policy.weights.throughput) +
        static_cast<std::uint64_t>(policy.weights.memory) +
        static_cast<std::uint64_t>(policy.weights.startup);
    return policy.minimum_sample_count > 0U &&
        policy.maximum_p95_latency_us > 0U &&
        policy.maximum_p95_latency_us <= kMaximumMetricValue &&
        policy.minimum_throughput_per_second > 0U &&
        policy.minimum_throughput_per_second <= kMaximumMetricValue &&
        policy.maximum_peak_memory_kib > 0U &&
        policy.maximum_peak_memory_kib <= kMaximumMetricValue &&
        policy.maximum_p95_startup_ms > 0U &&
        policy.maximum_p95_startup_ms <= kMaximumMetricValue &&
        valid_security_profile(policy.maximum_security_profile) &&
        total_weight == 100U;
}

bool valid_measurement(
    const PolyglotRouteImpactMeasurement& measurement) noexcept {
    if (!valid_implementation(measurement.implementation) ||
        !valid_security_profile(measurement.security_profile) ||
        measurement.failure_count > measurement.sample_count ||
        measurement.parity_mismatch_count > measurement.sample_count ||
        measurement.p95_latency_us > kMaximumMetricValue ||
        measurement.throughput_per_second > kMaximumMetricValue ||
        measurement.peak_memory_kib > kMaximumMetricValue ||
        measurement.p95_startup_ms > kMaximumMetricValue) {
        return false;
    }
    if (!measurement.runtime_available) {
        return true;
    }
    return measurement.sample_count > 0U &&
        measurement.throughput_per_second > 0U;
}

std::uint64_t ratio_score(
    const std::uint64_t numerator,
    const std::uint64_t denominator) noexcept {
    return (numerator * kBasisPoints) / denominator;
}

std::uint64_t impact_score(
    const PolyglotRouteImpactPolicy& policy,
    const PolyglotRouteImpactMeasurement& measurement) noexcept {
    const std::uint64_t latency = ratio_score(
        measurement.p95_latency_us, policy.maximum_p95_latency_us);
    const std::uint64_t throughput = ratio_score(
        policy.minimum_throughput_per_second,
        measurement.throughput_per_second);
    const std::uint64_t memory = ratio_score(
        measurement.peak_memory_kib, policy.maximum_peak_memory_kib);
    const std::uint64_t startup = ratio_score(
        measurement.p95_startup_ms, policy.maximum_p95_startup_ms);
    return latency * policy.weights.latency +
        throughput * policy.weights.throughput +
        memory * policy.weights.memory +
        startup * policy.weights.startup;
}

PolyglotRouteImpactAssessment assess(
    const PolyglotRouteImpactPolicy& policy,
    const PolyglotRouteImpactMeasurement& measurement) {
    PolyglotRouteImpactAssessment result;
    result.implementation = measurement.implementation;
    if (!measurement.runtime_available) {
        result.reason_code = "polyglot.impact.runtime_unavailable";
    } else if (!measurement.security_approved) {
        result.reason_code = "polyglot.impact.security_not_approved";
    } else if (static_cast<std::uint8_t>(measurement.security_profile) >
               static_cast<std::uint8_t>(policy.maximum_security_profile)) {
        result.reason_code = "polyglot.impact.security_profile_exceeds_policy";
    } else if (!measurement.contract_compatible) {
        result.reason_code = "polyglot.impact.contract_incompatible";
    } else if (measurement.sample_count < policy.minimum_sample_count) {
        result.reason_code = "polyglot.impact.insufficient_samples";
    } else if (measurement.failure_count != 0U) {
        result.reason_code = "polyglot.impact.execution_failures";
    } else if (measurement.parity_mismatch_count != 0U) {
        result.reason_code = "polyglot.impact.parity_mismatch";
    } else if (measurement.p95_latency_us > policy.maximum_p95_latency_us) {
        result.reason_code = "polyglot.impact.latency_budget_exceeded";
    } else if (measurement.throughput_per_second <
               policy.minimum_throughput_per_second) {
        result.reason_code = "polyglot.impact.throughput_budget_missed";
    } else if (measurement.peak_memory_kib > policy.maximum_peak_memory_kib) {
        result.reason_code = "polyglot.impact.memory_budget_exceeded";
    } else if (measurement.p95_startup_ms > policy.maximum_p95_startup_ms) {
        result.reason_code = "polyglot.impact.startup_budget_exceeded";
    } else {
        result.eligible = true;
        result.impact_score = impact_score(policy, measurement);
        result.reason_code = "polyglot.impact.eligible";
    }
    return result;
}

}  // namespace

const char* polyglot_route_implementation_name(
    const PolyglotRouteImplementation implementation) noexcept {
    switch (implementation) {
    case PolyglotRouteImplementation::direct_cpp:
        return "direct-cpp";
    case PolyglotRouteImplementation::cpp_dotnet_wrapper:
        return "cpp-dotnet-wrapper";
    case PolyglotRouteImplementation::csharp_service:
        return "csharp-service";
    }
    return "unknown";
}

const char* polyglot_route_security_profile_name(
    const PolyglotRouteSecurityProfile profile) noexcept {
    switch (profile) {
    case PolyglotRouteSecurityProfile::trusted_native:
        return "trusted-native";
    case PolyglotRouteSecurityProfile::admitted_process:
        return "admitted-process";
    case PolyglotRouteSecurityProfile::remote_service:
        return "remote-service";
    }
    return "unknown";
}

PolyglotRouteImpactResult evaluate_polyglot_route_impact(
    const PolyglotRouteImpactRequest& request) {
    if (!load_polyglot_route_registry({
            PolyglotRouteConfig{request.capability_id, "off", 0U}}).ok()) {
        return invalid_result(
            PolyglotRouteImpactError::invalid_capability_id,
            "polyglot.impact.invalid_capability_id");
    }
    if (!valid_policy(request.policy)) {
        return invalid_result(
            PolyglotRouteImpactError::invalid_policy,
            "polyglot.impact.invalid_policy");
    }
    if (request.measurements.size() != 3U) {
        return invalid_result(
            PolyglotRouteImpactError::wrong_route_count,
            "polyglot.impact.wrong_route_count");
    }

    std::array<const PolyglotRouteImpactMeasurement*, 3U> ordered{};
    for (const PolyglotRouteImpactMeasurement& measurement : request.measurements) {
        if (!valid_measurement(measurement)) {
            return invalid_result(
                PolyglotRouteImpactError::invalid_measurement,
                "polyglot.impact.invalid_measurement");
        }
        const std::size_t index = implementation_index(measurement.implementation);
        if (ordered[index] != nullptr) {
            return invalid_result(
                PolyglotRouteImpactError::duplicate_route,
                "polyglot.impact.duplicate_route");
        }
        ordered[index] = &measurement;
    }
    if (std::any_of(ordered.begin(), ordered.end(), [](const auto* value) {
            return value == nullptr;
        })) {
        return invalid_result(
            PolyglotRouteImpactError::duplicate_route,
            "polyglot.impact.duplicate_route");
    }

    PolyglotRouteImpactResult result;
    result.error_code = "polyglot.impact.recommendation_ready";
    for (const PolyglotRouteImpactMeasurement* measurement : ordered) {
        result.assessments.push_back(assess(request.policy, *measurement));
    }

    std::vector<const PolyglotRouteImpactAssessment*> eligible;
    for (const PolyglotRouteImpactAssessment& assessment : result.assessments) {
        if (assessment.eligible) {
            eligible.push_back(&assessment);
        }
    }
    if (eligible.empty()) {
        result.error = PolyglotRouteImpactError::no_eligible_route;
        result.error_code = "polyglot.impact.no_eligible_route";
        return result;
    }
    std::stable_sort(
        eligible.begin(), eligible.end(), [](const auto* left, const auto* right) {
            if (left->impact_score != right->impact_score) {
                return left->impact_score < right->impact_score;
            }
            return implementation_index(left->implementation) <
                implementation_index(right->implementation);
        });
    result.recommendation_ready = true;
    result.preferred = eligible.front()->implementation;
    for (std::size_t index = 1U; index < eligible.size(); ++index) {
        result.fallback_chain.push_back(eligible[index]->implementation);
    }
    return result;
}

}  // namespace copperfin::platform
