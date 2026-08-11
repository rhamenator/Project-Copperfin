// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/polyglot_benchmark.h"

#include <algorithm>
#include <array>
#include <limits>
#include <utility>

namespace copperfin::platform {

namespace {

constexpr std::uint32_t kMaximumWarmupIterations = 100U;
constexpr std::uint32_t kMaximumMeasuredIterations = 10'000U;
constexpr std::size_t kMaximumWorkloads = 100U;
constexpr std::uint64_t kMaximumMetricValue = 1'000'000'000'000ULL;

std::size_t route_index(const PolyglotRouteImplementation implementation) noexcept {
    switch (implementation) {
    case PolyglotRouteImplementation::direct_cpp: return 0U;
    case PolyglotRouteImplementation::cpp_dotnet_wrapper: return 1U;
    case PolyglotRouteImplementation::csharp_service: return 2U;
    }
    return 3U;
}

PolyglotBenchmarkResult reject(
    const PolyglotBenchmarkError error,
    std::string error_code) {
    PolyglotBenchmarkResult result;
    result.error = error;
    result.error_code = std::move(error_code);
    return result;
}

bool valid_request(const PolyglotBenchmarkRequest& request) {
    if (request.capability_id.empty() || request.warmup_iterations >
            kMaximumWarmupIterations || request.measured_iterations == 0U ||
        request.measured_iterations > kMaximumMeasuredIterations ||
        request.workloads.empty() || request.workloads.size() > kMaximumWorkloads ||
        request.routes.size() != 3U || !request.invoke) {
        return false;
    }
    const std::uint64_t sample_count =
        static_cast<std::uint64_t>(request.measured_iterations) *
        static_cast<std::uint64_t>(request.workloads.size());
    if (sample_count == 0U ||
        sample_count > std::numeric_limits<std::uint32_t>::max()) {
        return false;
    }
    for (const auto& workload : request.workloads) {
        if (workload.workload_id.empty() || workload.arguments_json.empty() ||
            workload.expected_payload_json.empty()) {
            return false;
        }
    }
    std::array<bool, 3U> seen{};
    for (const auto& route : request.routes) {
        const std::size_t index = route_index(route.implementation);
        if (index >= seen.size() || seen[index]) {
            return false;
        }
        seen[index] = true;
    }
    return true;
}

bool valid_observation(const PolyglotBenchmarkObservation& observation) noexcept {
    return observation.latency_us > 0U &&
        observation.latency_us <= kMaximumMetricValue &&
        observation.peak_memory_kib <= kMaximumMetricValue &&
        observation.startup_ms <= kMaximumMetricValue;
}

std::uint64_t percentile95(std::vector<std::uint64_t> values) {
    std::sort(values.begin(), values.end());
    const std::size_t rank =
        (values.size() * 95U + 99U) / 100U;
    return values[rank - 1U];
}

}  // namespace

PolyglotBenchmarkResult run_polyglot_benchmark(
    const PolyglotBenchmarkRequest& request) {
    if (!valid_request(request)) {
        return reject(
            PolyglotBenchmarkError::invalid_request,
            "polyglot.benchmark.invalid_request");
    }

    std::vector<PolyglotRouteImpactMeasurement> preflight_measurements;
    preflight_measurements.reserve(request.routes.size());
    for (const auto& route : request.routes) {
        preflight_measurements.push_back({
            .implementation = route.implementation,
            .security_profile = route.security_profile,
            .runtime_available = false,
            .security_approved = route.security_approved,
            .contract_compatible = route.contract_compatible});
    }
    const auto preflight = evaluate_polyglot_route_impact({
        request.capability_id, request.policy, preflight_measurements});
    if (preflight.error != PolyglotRouteImpactError::no_eligible_route) {
        return reject(
            PolyglotBenchmarkError::invalid_request,
            "polyglot.benchmark.invalid_request");
    }

    PolyglotBenchmarkResult result;
    result.error_code = "polyglot.benchmark.complete";
    const std::uint32_t sample_count = static_cast<std::uint32_t>(
        request.measured_iterations * request.workloads.size());
    for (const auto& route : request.routes) {
        PolyglotRouteImpactMeasurement measurement{
            .implementation = route.implementation,
            .security_profile = route.security_profile,
            .runtime_available = route.runtime_available,
            .security_approved = route.security_approved,
            .contract_compatible = route.contract_compatible};
        if (!route.runtime_available) {
            result.measurements.push_back(measurement);
            continue;
        }

        try {
            for (std::uint32_t iteration = 0U;
                 iteration < request.warmup_iterations; ++iteration) {
                for (const auto& workload : request.workloads) {
                    const auto observation = request.invoke({
                        route.implementation, &workload, iteration, true});
                    if (!valid_observation(observation)) {
                        return reject(
                            PolyglotBenchmarkError::invalid_observation,
                            "polyglot.benchmark.invalid_observation");
                    }
                    if (!observation.invocation_succeeded ||
                        !observation.parity_matched) {
                        return reject(
                            PolyglotBenchmarkError::invocation_failed,
                            "polyglot.benchmark.warmup_failed");
                    }
                }
            }

            std::vector<std::uint64_t> latencies;
            std::vector<std::uint64_t> startups;
            latencies.reserve(sample_count);
            startups.reserve(sample_count);
            std::uint64_t total_latency = 0U;
            for (std::uint32_t iteration = 0U;
                 iteration < request.measured_iterations; ++iteration) {
                for (const auto& workload : request.workloads) {
                    const auto observation = request.invoke({
                        route.implementation, &workload, iteration, false});
                    if (!valid_observation(observation) ||
                        total_latency >
                            std::numeric_limits<std::uint64_t>::max() -
                                observation.latency_us) {
                        return reject(
                            PolyglotBenchmarkError::invalid_observation,
                            "polyglot.benchmark.invalid_observation");
                    }
                    latencies.push_back(observation.latency_us);
                    startups.push_back(observation.startup_ms);
                    total_latency += observation.latency_us;
                    measurement.peak_memory_kib = std::max(
                        measurement.peak_memory_kib,
                        observation.peak_memory_kib);
                    if (!observation.invocation_succeeded) {
                        ++measurement.failure_count;
                    }
                    if (!observation.parity_matched) {
                        ++measurement.parity_mismatch_count;
                    }
                }
            }
            measurement.sample_count = sample_count;
            measurement.p95_latency_us = percentile95(std::move(latencies));
            measurement.p95_startup_ms = percentile95(std::move(startups));
            measurement.throughput_per_second = std::max<std::uint64_t>(
                1U,
                (static_cast<std::uint64_t>(sample_count) * 1'000'000U) /
                    total_latency);
        } catch (...) {
            return reject(
                PolyglotBenchmarkError::invocation_failed,
                "polyglot.benchmark.invocation_failed");
        }
        result.measurements.push_back(measurement);
    }

    result.impact = evaluate_polyglot_route_impact({
        request.capability_id, request.policy, result.measurements});
    return result;
}

}  // namespace copperfin::platform
