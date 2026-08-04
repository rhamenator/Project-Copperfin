// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/polyglot_migration_telemetry.h"

#include <algorithm>
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

void test_route_and_bridge_events() {
    const auto route_registry = copperfin::platform::load_polyglot_route_registry({
        {"reports.invoice.render", "canary", 100U}});
    const auto route = copperfin::platform::evaluate_polyglot_route(
        route_registry.registry, "reports.invoice.render", 0U);
    copperfin::platform::PolyglotMigrationTelemetryStream stream;
    copperfin::platform::record_polyglot_route_event(stream, "reports.invoice.render", route);
    expect(stream.events.size() == 1U && stream.events[0].category == "polyglot.route.selected" &&
               stream.events[0].capability_id == "reports.invoice.render" &&
               stream.events[0].detail == "canary",
           "route decision should emit invariant route-selected fields");

    auto policy = copperfin::platform::PolyglotBridgeInvocationPolicy{};
    policy.fallback = copperfin::platform::PolyglotFallbackPolicy::fallback_native;
    const auto bridge = copperfin::platform::evaluate_polyglot_bridge_invocation(
        policy, {.elapsed_ms = policy.timeout_ms});
    copperfin::platform::record_polyglot_bridge_event(
        stream, "reports.invoice.render", policy.timeout_ms, bridge);
    expect(stream.events.size() == 3U && stream.events[1].category == "polyglot.latency.outcome" &&
               stream.events[1].reason_code == "polyglot.bridge.timeout" &&
               stream.events[2].category == "polyglot.fallback.applied" &&
               stream.events[2].successful,
           "bridge timeout should emit latency and fallback events");
}

void test_parity_events() {
    copperfin::platform::PolyglotParityComparisonRequest request;
    request.capability_id = "forms.customer.open";
    request.fields = {{"caption", "string", "string", "native", "candidate"}};
    const auto result = copperfin::platform::compare_polyglot_outputs({}, request);
    copperfin::platform::PolyglotMigrationTelemetryStream stream;
    copperfin::platform::record_polyglot_parity_events(stream, request, result);
    expect(stream.events.size() == 2U && stream.events[0].category == "polyglot.parity.checked" &&
               stream.events[1].category == "polyglot.parity.mismatch" &&
               stream.events[1].capability_id == request.capability_id &&
               stream.events[1].reason_code == "polyglot.parity.value_mismatch" &&
               stream.events[1].mismatch_count == 1U,
           "parity comparison should emit checked and mismatch events");
    expect(std::all_of(stream.events.begin(), stream.events.end(), [](const auto& event) {
               return event.category.find("polyglot.") == 0U;
           }),
           "migration event categories should use the invariant polyglot namespace");
}

}  // namespace

int main() {
    test_route_and_bridge_events();
    test_parity_events();
    if (failures != 0) {
        std::cerr << failures << " polyglot migration telemetry test(s) failed\n";
        return 1;
    }
    std::cout << "All polyglot migration telemetry tests passed\n";
    return 0;
}
