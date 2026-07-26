// Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/platform/polyglot_migration_telemetry.h"

#include <string_view>

namespace copperfin::platform {

namespace {

bool bridge_succeeded(PolyglotBridgeOutcome outcome) noexcept {
    return outcome == PolyglotBridgeOutcome::success;
}

bool bridge_used_fallback(PolyglotBridgeOutcome outcome) noexcept {
    return outcome == PolyglotBridgeOutcome::fallback_native ||
        outcome == PolyglotBridgeOutcome::fallback_artifact;
}

const char* bridge_outcome_name(PolyglotBridgeOutcome outcome) noexcept {
    switch (outcome) {
    case PolyglotBridgeOutcome::success:
        return "success";
    case PolyglotBridgeOutcome::cancelled:
        return "cancelled";
    case PolyglotBridgeOutcome::failed:
        return "failed";
    case PolyglotBridgeOutcome::fallback_native:
        return "fallback-native";
    case PolyglotBridgeOutcome::fallback_artifact:
        return "fallback-artifact";
    }
    return "failed";
}

}  // namespace

void record_polyglot_route_event(
    PolyglotMigrationTelemetryStream& stream,
    std::string_view capability_id,
    const PolyglotRouteDecision& decision) {
    stream.events.push_back(
        PolyglotMigrationEvent{
            "polyglot.route.selected",
            std::string(capability_id),
            decision.reason_code,
            polyglot_route_state_name(decision.state),
            0U,
            0U,
            decision.selection != PolyglotRouteSelection::native});
}

void record_polyglot_bridge_event(
    PolyglotMigrationTelemetryStream& stream,
    std::string_view capability_id,
    std::uint64_t latency_ms,
    const PolyglotBridgeInvocationDecision& decision) {
    stream.events.push_back(
        PolyglotMigrationEvent{
            "polyglot.latency.outcome",
            std::string(capability_id),
            decision.error_code,
            bridge_outcome_name(decision.outcome),
            latency_ms,
            0U,
            bridge_succeeded(decision.outcome)});
    if (bridge_used_fallback(decision.outcome)) {
        stream.events.push_back(
            PolyglotMigrationEvent{
                "polyglot.fallback.applied",
                std::string(capability_id),
                decision.error_code,
                bridge_outcome_name(decision.outcome),
                latency_ms,
                0U,
                true});
    }
}

void record_polyglot_parity_events(
    PolyglotMigrationTelemetryStream& stream,
    const PolyglotParityComparisonRequest& request,
    const PolyglotParityComparisonResult& result) {
    for (const PolyglotParityTelemetryEvent& event : result.telemetry) {
        stream.events.push_back(
            PolyglotMigrationEvent{
                event.event_name,
                event.capability_id.empty() ? request.capability_id : event.capability_id,
                event.reason_code,
                {},
                0U,
                event.mismatch_count,
                event.parity_match});
    }
}

}  // namespace copperfin::platform
