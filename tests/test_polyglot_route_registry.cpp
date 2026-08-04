// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/polyglot_route_registry.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

using copperfin::platform::PolyglotRouteConfig;
using copperfin::platform::PolyglotRouteConfigError;
using copperfin::platform::PolyglotRouteSelection;
using copperfin::platform::PolyglotRouteState;

void test_state_names_and_parsing() {
    const std::vector<std::pair<std::string, PolyglotRouteState>> states = {
        {"off", PolyglotRouteState::off},
        {"shadow", PolyglotRouteState::shadow},
        {"canary", PolyglotRouteState::canary},
        {"on", PolyglotRouteState::on},
        {"retire-legacy", PolyglotRouteState::retire_legacy}};
    for (const auto& [name, state] : states) {
        PolyglotRouteState parsed = PolyglotRouteState::off;
        expect(
            copperfin::platform::parse_polyglot_route_state(name, parsed) && parsed == state,
            "route state should parse: " + name);
        expect(
            copperfin::platform::polyglot_route_state_name(state) == name,
            "route state name should remain invariant: " + name);
    }
    PolyglotRouteState ignored = PolyglotRouteState::on;
    expect(
        !copperfin::platform::parse_polyglot_route_state("maybe", ignored) &&
            ignored == PolyglotRouteState::off,
        "invalid route state should fail closed to off");
}

void test_registry_validation_and_empty_default() {
    const auto empty = copperfin::platform::load_polyglot_route_registry({});
    expect(empty.ok() && empty.registry.entries.empty(), "absent route configuration should load as empty");
    const auto absent = copperfin::platform::evaluate_polyglot_route(empty.registry, "reports.invoice.render");
    expect(absent.state == PolyglotRouteState::off, "absent capability should default to off");
    expect(absent.selection == PolyglotRouteSelection::native, "absent capability should select native");
    expect(absent.invoke_native && !absent.invoke_candidate, "default-off route should invoke native only");
    expect(absent.reason_code == "polyglot.route.default_off", "default-off reason code should be stable");

    const auto invalid_state = copperfin::platform::load_polyglot_route_registry({
        {"reports.invoice.render", "maybe", 0U}});
    expect(!invalid_state.ok(), "invalid route state should reject configuration");
    expect(
        invalid_state.error == PolyglotRouteConfigError::invalid_state &&
            invalid_state.error_code == "polyglot.route.invalid_state",
        "invalid route state should expose a stable error code");

    const auto invalid_id = copperfin::platform::load_polyglot_route_registry({
        {"Reports/Invoice", "off", 0U}});
    expect(
        invalid_id.error == PolyglotRouteConfigError::invalid_capability_id,
        "capability ids should preserve lower-case machine syntax");

    const auto duplicate = copperfin::platform::load_polyglot_route_registry({
        {"reports.invoice.render", "off", 0U},
        {"reports.invoice.render", "on", 0U}});
    expect(
        duplicate.error == PolyglotRouteConfigError::duplicate_capability_id,
        "duplicate capability ids should reject the registry");

    const auto non_canary_percentage = copperfin::platform::load_polyglot_route_registry({
        {"reports.invoice.render", "on", 1U}});
    expect(
        non_canary_percentage.error == PolyglotRouteConfigError::invalid_canary_percentage,
        "canary percentage should be rejected for non-canary states");

    const auto out_of_range_percentage = copperfin::platform::load_polyglot_route_registry({
        {"reports.invoice.render", "canary", 101U}});
    expect(
        out_of_range_percentage.error == PolyglotRouteConfigError::invalid_canary_percentage &&
            out_of_range_percentage.error_code == "polyglot.route.canary_percentage_out_of_range",
        "canary percentage above 100 should reject the registry explicitly");
}

void test_json_configuration_loading() {
    const auto loaded = copperfin::platform::load_polyglot_route_registry_json(R"json(
        {
          "registry_version": "1.0",
          "routes": [
            {"capability_id": "reports.invoice.render", "state": "canary", "canary_percentage": 25},
            {"capability_id": "reports.invoice.preview", "state": "off"}
          ]
        }
    )json");
    expect(loaded.ok() && loaded.registry.entries.size() == 2U,
           "valid route JSON should load into the registry");
    if (loaded.ok() && loaded.registry.entries.size() == 2U) {
        expect(loaded.registry.entries[0].state == PolyglotRouteState::canary &&
                   loaded.registry.entries[0].canary_percentage == 25U,
               "JSON loader should preserve canary configuration");
    }

    const auto missing_routes = copperfin::platform::load_polyglot_route_registry_json(
        R"json({"registry_version":"1.0"})json");
    expect(missing_routes.error == PolyglotRouteConfigError::missing_routes,
           "route JSON without routes should report missing_routes");
    const auto malformed = copperfin::platform::load_polyglot_route_registry_json(
        R"json({"registry_version":"1.0","routes":[{"capability_id":"reports.invoice.render","state":"maybe"}]})json");
    expect(malformed.error == PolyglotRouteConfigError::invalid_state,
           "valid JSON with an invalid state should preserve configuration error detail");
    const auto invalid_document = copperfin::platform::load_polyglot_route_registry_json(
        R"json({"registry_version":"1.0","routes":[{"capability_id":"reports.invoice.render","state":"off"}],})json");
    expect(invalid_document.error == PolyglotRouteConfigError::invalid_document,
           "malformed route JSON should report invalid_document");
    const auto invalid_version = copperfin::platform::load_polyglot_route_registry_json(
        R"json({"registry_version":"2.0","routes":[]})json");
    expect(invalid_version.error == PolyglotRouteConfigError::invalid_version,
           "unsupported route registry versions should reject explicitly");

    std::ifstream fixture(COPPERFIN_ROUTE_REGISTRY_EXAMPLE_PATH);
    expect(fixture.is_open(), "canonical route registry example should be readable");
    if (fixture.is_open()) {
        std::ostringstream contents;
        contents << fixture.rdbuf();
        const auto fixture_result = copperfin::platform::load_polyglot_route_registry_json(contents.str());
        expect(fixture_result.ok() && fixture_result.registry.entries.size() == 2U,
               "canonical route registry example should pass the native loader");
    }
}

void test_route_decisions() {
    const auto loaded = copperfin::platform::load_polyglot_route_registry({
        {"route.off", "off", 0U},
        {"route.shadow", "shadow", 0U},
        {"route.canary", "canary", 50U},
        {"route.on", "on", 0U},
        {"route.retire", "retire-legacy", 0U}});
    expect(loaded.ok(), "all supported route states should load");

    const auto off = copperfin::platform::evaluate_polyglot_route(loaded.registry, "route.off");
    expect(off.state == PolyglotRouteState::off && off.selection == PolyglotRouteSelection::native,
           "off should use native only");

    const auto shadow = copperfin::platform::evaluate_polyglot_route(loaded.registry, "route.shadow");
    expect(shadow.state == PolyglotRouteState::shadow && shadow.selection == PolyglotRouteSelection::shadow,
           "shadow should select shadow mode");
    expect(shadow.invoke_native && shadow.invoke_candidate && !shadow.candidate_primary,
           "shadow should invoke both and return native");

    const auto canary_native = copperfin::platform::evaluate_polyglot_route(
        loaded.registry, "route.canary", 50U);
    expect(canary_native.selection == PolyglotRouteSelection::native && canary_native.invoke_native,
           "canary boundary should remain native at the percentage limit");
    const auto canary_candidate = copperfin::platform::evaluate_polyglot_route(
        loaded.registry, "route.canary", 49U);
    expect(canary_candidate.selection == PolyglotRouteSelection::candidate &&
               canary_candidate.candidate_primary && canary_candidate.invoke_candidate &&
               !canary_candidate.invoke_native,
           "canary sample below the percentage should select candidate deterministically");

    const auto on = copperfin::platform::evaluate_polyglot_route(loaded.registry, "route.on");
    expect(on.selection == PolyglotRouteSelection::candidate && on.candidate_primary &&
               on.native_fallback_allowed,
           "on should select candidate with native fallback allowed");

    const auto retired = copperfin::platform::evaluate_polyglot_route(loaded.registry, "route.retire");
    expect(retired.selection == PolyglotRouteSelection::candidate && retired.candidate_primary &&
               !retired.native_fallback_allowed,
           "retire-legacy should select candidate without native fallback");
}

}  // namespace

int main() {
    test_state_names_and_parsing();
    test_registry_validation_and_empty_default();
    test_json_configuration_loading();
    test_route_decisions();
    if (failures != 0) {
        std::cerr << failures << " polyglot route registry test(s) failed\n";
        return 1;
    }
    std::cout << "All polyglot route registry tests passed\n";
    return 0;
}
