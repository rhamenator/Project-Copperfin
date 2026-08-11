// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/polyglot_benchmark.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using namespace copperfin::platform;

void expect(const bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

PolyglotBenchmarkRequest request() {
    return {
        .capability_id = "samples.dotnet.add-v1",
        .warmup_iterations = 2U,
        .measured_iterations = 10U,
        .policy = {
            .minimum_sample_count = 20U,
            .maximum_p95_latency_us = 10'000U,
            .minimum_throughput_per_second = 100U,
            .maximum_peak_memory_kib = 100'000U,
            .maximum_p95_startup_ms = 1000U,
            .maximum_security_profile =
                PolyglotRouteSecurityProfile::admitted_process,
            .weights = {25U, 25U, 25U, 25U}},
        .workloads = {
            {"positive", R"({"left":20,"right":22})", R"({"sum":42})"},
            {"negative", R"({"left":-20,"right":-22})", R"({"sum":-42})"}},
        .routes = {
            {PolyglotRouteImplementation::direct_cpp,
             PolyglotRouteSecurityProfile::trusted_native, true, true, true},
            {PolyglotRouteImplementation::cpp_dotnet_wrapper,
             PolyglotRouteSecurityProfile::admitted_process, true, true, true},
            {PolyglotRouteImplementation::csharp_service,
             PolyglotRouteSecurityProfile::admitted_process, false, true, true}},
        .invoke = {}};
}

void test_bounded_aggregation_and_unavailable_route() {
    auto input = request();
    std::size_t calls = 0U;
    input.invoke = [&](const PolyglotBenchmarkInvocation& invocation) {
        ++calls;
        const std::uint64_t base =
            invocation.implementation == PolyglotRouteImplementation::direct_cpp
            ? 100U : 200U;
        return PolyglotBenchmarkObservation{
            true, true, base + invocation.iteration,
            base * 10U, base / 10U};
    };
    const auto result = run_polyglot_benchmark(input);
    expect(result.ok() && result.impact.recommendation_ready,
           "valid representative observations should produce an advisory result");
    expect(calls == 48U,
           "only available routes should receive fixed warmup and measured calls");
    expect(result.measurements.size() == 3U &&
               result.measurements[0].sample_count == 20U &&
               result.measurements[0].p95_latency_us == 109U &&
               result.measurements[0].throughput_per_second == 9569U &&
               result.measurements[0].peak_memory_kib == 1000U &&
               result.measurements[0].p95_startup_ms == 10U,
           "aggregation should use nearest-rank p95 and integer throughput");
    expect(result.measurements[2].runtime_available == false &&
               result.measurements[2].sample_count == 0U,
           "an unavailable route should be explicit and must not be fabricated");
    expect(result.impact.assessments[2].reason_code ==
               "polyglot.impact.runtime_unavailable",
           "the evaluator should retain the unavailable-route reason");
}

void test_failures_and_parity_are_hard_gates() {
    auto input = request();
    input.routes[2].runtime_available = true;
    input.invoke = [](const PolyglotBenchmarkInvocation& invocation) {
        return PolyglotBenchmarkObservation{
            invocation.warmup || invocation.implementation !=
                PolyglotRouteImplementation::cpp_dotnet_wrapper,
            invocation.warmup || invocation.implementation !=
                PolyglotRouteImplementation::csharp_service,
            100U, 1000U, 1U};
    };
    const auto result = run_polyglot_benchmark(input);
    expect(result.ok() && result.impact.preferred ==
               PolyglotRouteImplementation::direct_cpp,
           "only a successful parity-matched route should be recommended");
    expect(result.measurements[1].failure_count == 20U &&
               result.measurements[2].parity_mismatch_count == 20U,
           "each measured failed or mismatched observation should be counted");
}

void test_invalid_inputs_fail_before_callback() {
    auto input = request();
    std::size_t calls = 0U;
    input.invoke = [&](const auto&) {
        ++calls;
        return PolyglotBenchmarkObservation{true, true, 1U, 0U, 0U};
    };
    input.routes[2].implementation =
        PolyglotRouteImplementation::direct_cpp;
    expect(run_polyglot_benchmark(input).error_code ==
               "polyglot.benchmark.invalid_request" && calls == 0U,
           "duplicate route classes should fail before invoking work");

    input = request();
    input.policy.weights.startup = 24U;
    input.invoke = [&](const auto&) {
        ++calls;
        return PolyglotBenchmarkObservation{true, true, 1U, 0U, 0U};
    };
    expect(run_polyglot_benchmark(input).error_code ==
               "polyglot.benchmark.invalid_request" && calls == 0U,
           "an invalid impact policy should fail before invoking work");

    input = request();
    input.invoke = [&](const auto&) {
        ++calls;
        return PolyglotBenchmarkObservation{true, true, 0U, 0U, 0U};
    };
    expect(run_polyglot_benchmark(input).error_code ==
               "polyglot.benchmark.invalid_observation",
           "zero-latency or excessive observations should fail closed");

    input = request();
    input.invoke = [](const auto&) -> PolyglotBenchmarkObservation {
        throw std::runtime_error("fixture");
    };
    expect(run_polyglot_benchmark(input).error_code ==
               "polyglot.benchmark.invocation_failed",
           "callback exceptions should not cross the benchmark boundary");

    input = request();
    input.invoke = [](const auto&) {
        return PolyglotBenchmarkObservation{false, false, 1U, 0U, 0U};
    };
    expect(run_polyglot_benchmark(input).error_code ==
               "polyglot.benchmark.warmup_failed",
           "a failed warmup should invalidate the run instead of being hidden");
}

}  // namespace

int main() {
    test_bounded_aggregation_and_unavailable_route();
    test_failures_and_parity_are_hard_gates();
    test_invalid_inputs_fail_before_callback();
    std::cout << "polyglot benchmark tests passed\n";
    return 0;
}
