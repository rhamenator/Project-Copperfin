// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/polyglot_route_impact.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

using copperfin::platform::PolyglotRouteImpactMeasurement;
using copperfin::platform::PolyglotRouteImpactPolicy;
using copperfin::platform::PolyglotRouteImpactRequest;
using copperfin::platform::PolyglotRouteImplementation;
using copperfin::platform::PolyglotRouteSecurityProfile;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

PolyglotRouteImpactPolicy policy() {
    return {
        .minimum_sample_count = 100U,
        .maximum_p95_latency_us = 1000U,
        .minimum_throughput_per_second = 1000U,
        .maximum_peak_memory_kib = 100'000U,
        .maximum_p95_startup_ms = 1000U,
        .maximum_security_profile =
            PolyglotRouteSecurityProfile::admitted_process,
        .weights = {25U, 25U, 25U, 25U}};
}

PolyglotRouteImpactMeasurement measurement(
    PolyglotRouteImplementation implementation,
    PolyglotRouteSecurityProfile security_profile,
    std::uint64_t latency,
    std::uint64_t throughput,
    std::uint64_t memory,
    std::uint64_t startup) {
    return {
        .implementation = implementation,
        .security_profile = security_profile,
        .runtime_available = true,
        .security_approved = true,
        .contract_compatible = true,
        .sample_count = 1000U,
        .failure_count = 0U,
        .parity_mismatch_count = 0U,
        .p95_latency_us = latency,
        .throughput_per_second = throughput,
        .peak_memory_kib = memory,
        .p95_startup_ms = startup};
}

std::vector<PolyglotRouteImpactMeasurement> representative_measurements() {
    return {
        measurement(
            PolyglotRouteImplementation::direct_cpp,
            PolyglotRouteSecurityProfile::trusted_native,
            900U, 1100U, 50'000U, 10U),
        measurement(
            PolyglotRouteImplementation::cpp_dotnet_wrapper,
            PolyglotRouteSecurityProfile::admitted_process,
            300U, 2000U, 60'000U, 200U),
        measurement(
            PolyglotRouteImplementation::csharp_service,
            PolyglotRouteSecurityProfile::admitted_process,
            400U, 1800U, 70'000U, 300U)};
}

PolyglotRouteImpactRequest request() {
    return {
        .capability_id = "samples.dotnet.add-v1",
        .policy = policy(),
        .measurements = representative_measurements()};
}

void test_measured_route_and_fallback_order() {
    const auto result =
        copperfin::platform::evaluate_polyglot_route_impact(request());
    expect(result.ok(), "representative evidence should be valid");
    expect(result.recommendation_ready,
           "representative evidence should produce a recommendation");
    expect(result.preferred ==
               PolyglotRouteImplementation::cpp_dotnet_wrapper,
           "measured wrapper impact should win instead of a fixed route preference");
    expect(result.fallback_chain == std::vector<PolyglotRouteImplementation>{
               PolyglotRouteImplementation::csharp_service,
               PolyglotRouteImplementation::direct_cpp},
           "eligible fallbacks should be ordered by measured impact");
    expect(result.assessments.size() == 3U &&
               result.assessments[0].implementation ==
                   PolyglotRouteImplementation::direct_cpp &&
               result.assessments[1].implementation ==
                   PolyglotRouteImplementation::cpp_dotnet_wrapper &&
               result.assessments[2].implementation ==
                   PolyglotRouteImplementation::csharp_service,
           "assessment output should have canonical route order");
    expect(result.assessments[0].impact_score == 579'750U &&
               result.assessments[1].impact_score == 400'000U &&
               result.assessments[2].impact_score == 488'875U,
           "integer normalization and weighting should remain exact");
}

void test_input_order_does_not_change_decision() {
    auto reversed = request();
    std::reverse(reversed.measurements.begin(), reversed.measurements.end());
    const auto result =
        copperfin::platform::evaluate_polyglot_route_impact(reversed);
    expect(result.ok() && result.recommendation_ready &&
               result.preferred ==
                   PolyglotRouteImplementation::cpp_dotnet_wrapper,
           "input order should not change the preferred route");
    expect(result.fallback_chain == std::vector<PolyglotRouteImplementation>{
               PolyglotRouteImplementation::csharp_service,
               PolyglotRouteImplementation::direct_cpp},
           "input order should not change the fallback chain");
}

void test_hard_gates_precede_impact_score() {
    auto gated = request();
    gated.measurements[1].security_approved = false;
    gated.measurements[2].parity_mismatch_count = 1U;
    const auto result =
        copperfin::platform::evaluate_polyglot_route_impact(gated);
    expect(result.ok() && result.recommendation_ready &&
               result.preferred == PolyglotRouteImplementation::direct_cpp,
           "only a fully eligible route may be recommended");
    expect(result.fallback_chain.empty(),
           "ineligible routes should not appear in the fallback chain");
    expect(result.assessments[1].reason_code ==
               "polyglot.impact.security_not_approved" &&
               result.assessments[2].reason_code ==
               "polyglot.impact.parity_mismatch",
           "hard-gate reasons should remain machine-readable");
}

void test_fail_closed_without_sufficient_evidence() {
    auto insufficient = request();
    for (auto& item : insufficient.measurements) {
        item.sample_count = 10U;
    }
    const auto result =
        copperfin::platform::evaluate_polyglot_route_impact(insufficient);
    expect(!result.ok() && !result.recommendation_ready,
           "insufficient evidence should not produce a recommendation");
    expect(result.error_code == "polyglot.impact.no_eligible_route",
           "no eligible route should have a stable error code");
    expect(result.preferred == PolyglotRouteImplementation::direct_cpp &&
               result.fallback_chain.empty(),
           "failure should retain native/no-promotion defaults");
}

void test_security_profile_limit() {
    auto limited = request();
    limited.measurements[2].security_profile =
        PolyglotRouteSecurityProfile::remote_service;
    const auto result =
        copperfin::platform::evaluate_polyglot_route_impact(limited);
    expect(result.ok() && result.recommendation_ready,
           "a disallowed service should not invalidate other evidence");
    expect(result.assessments[2].reason_code ==
               "polyglot.impact.security_profile_exceeds_policy",
           "security exposure should be a hard eligibility gate");
}

void test_each_impact_gate_has_a_stable_reason() {
    struct GateCase {
        const char* expected;
        void (*mutate)(PolyglotRouteImpactMeasurement&);
    };
    const std::vector<GateCase> cases{
        {"polyglot.impact.runtime_unavailable", [](auto& value) {
             value.runtime_available = false;
         }},
        {"polyglot.impact.contract_incompatible", [](auto& value) {
             value.contract_compatible = false;
         }},
        {"polyglot.impact.insufficient_samples", [](auto& value) {
             value.sample_count = 99U;
         }},
        {"polyglot.impact.execution_failures", [](auto& value) {
             value.failure_count = 1U;
         }},
        {"polyglot.impact.parity_mismatch", [](auto& value) {
             value.parity_mismatch_count = 1U;
         }},
        {"polyglot.impact.latency_budget_exceeded", [](auto& value) {
             value.p95_latency_us = 1001U;
         }},
        {"polyglot.impact.throughput_budget_missed", [](auto& value) {
             value.throughput_per_second = 999U;
         }},
        {"polyglot.impact.memory_budget_exceeded", [](auto& value) {
             value.peak_memory_kib = 100'001U;
         }},
        {"polyglot.impact.startup_budget_exceeded", [](auto& value) {
             value.p95_startup_ms = 1001U;
         }}};

    for (const GateCase& gate : cases) {
        auto gated = request();
        gate.mutate(gated.measurements[1]);
        const auto result =
            copperfin::platform::evaluate_polyglot_route_impact(gated);
        expect(result.ok() && result.recommendation_ready,
               std::string("other eligible routes should survive ") + gate.expected);
        expect(result.assessments[1].reason_code == gate.expected,
               std::string("unexpected reason for ") + gate.expected);
        expect(std::find(result.fallback_chain.begin(),
                         result.fallback_chain.end(),
                         PolyglotRouteImplementation::cpp_dotnet_wrapper) ==
                   result.fallback_chain.end() &&
                   result.preferred !=
                       PolyglotRouteImplementation::cpp_dotnet_wrapper,
               std::string("gated route should not be recommended for ") +
                   gate.expected);
    }
}

void test_deterministic_tie_break() {
    auto tied = request();
    const auto common = measurement(
        PolyglotRouteImplementation::direct_cpp,
        PolyglotRouteSecurityProfile::trusted_native,
        500U, 2000U, 50'000U, 500U);
    tied.measurements = {common, common, common};
    tied.measurements[0].implementation =
        PolyglotRouteImplementation::csharp_service;
    tied.measurements[0].security_profile =
        PolyglotRouteSecurityProfile::admitted_process;
    tied.measurements[1].implementation =
        PolyglotRouteImplementation::cpp_dotnet_wrapper;
    tied.measurements[1].security_profile =
        PolyglotRouteSecurityProfile::admitted_process;
    tied.measurements[2].implementation =
        PolyglotRouteImplementation::direct_cpp;
    const auto result =
        copperfin::platform::evaluate_polyglot_route_impact(tied);
    expect(result.ok() && result.preferred ==
               PolyglotRouteImplementation::direct_cpp,
           "exact ties should prefer the least exposed implementation class");
    expect(result.fallback_chain == std::vector<PolyglotRouteImplementation>{
               PolyglotRouteImplementation::cpp_dotnet_wrapper,
               PolyglotRouteImplementation::csharp_service},
           "exact tie fallback order should be stable");
}

void test_invalid_contract_inputs() {
    auto invalid_capability = request();
    invalid_capability.capability_id = "Not Canonical";
    expect(copperfin::platform::evaluate_polyglot_route_impact(
               invalid_capability).error_code ==
               "polyglot.impact.invalid_capability_id",
           "capability identity should reuse the route registry contract");

    auto invalid_policy = request();
    invalid_policy.policy.weights.startup = 24U;
    expect(copperfin::platform::evaluate_polyglot_route_impact(
               invalid_policy).error_code == "polyglot.impact.invalid_policy",
           "weights that do not total 100 should be rejected");

    auto duplicate = request();
    duplicate.measurements[2].implementation =
        PolyglotRouteImplementation::direct_cpp;
    expect(copperfin::platform::evaluate_polyglot_route_impact(
               duplicate).error_code == "polyglot.impact.duplicate_route",
           "duplicate implementation evidence should be rejected");

    auto missing = request();
    missing.measurements.pop_back();
    expect(copperfin::platform::evaluate_polyglot_route_impact(
               missing).error_code == "polyglot.impact.wrong_route_count",
           "all three route classes should be represented");

    auto impossible_counts = request();
    impossible_counts.measurements[0].failure_count = 1001U;
    expect(copperfin::platform::evaluate_polyglot_route_impact(
               impossible_counts).error_code ==
               "polyglot.impact.invalid_measurement",
           "failure counts above the sample count should be rejected");

    auto excessive_metric = request();
    excessive_metric.measurements[0].p95_latency_us =
        1'000'000'000'001ULL;
    expect(copperfin::platform::evaluate_polyglot_route_impact(
               excessive_metric).error_code ==
               "polyglot.impact.invalid_measurement",
           "metrics above the fixed arithmetic ceiling should be rejected");
}

}  // namespace

int main() {
    test_measured_route_and_fallback_order();
    test_input_order_does_not_change_decision();
    test_hard_gates_precede_impact_score();
    test_fail_closed_without_sufficient_evidence();
    test_security_profile_limit();
    test_each_impact_gate_has_a_stable_reason();
    test_deterministic_tie_break();
    test_invalid_contract_inputs();
    std::cout << "polyglot route impact tests passed\n";
    return 0;
}
