// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "copperfin/platform/polyglot_bridge_invocation.h"
#include "copperfin/platform/polyglot_parity_comparator.h"
#include "copperfin/platform/polyglot_route_registry.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace copperfin::platform {

struct PolyglotMigrationEvent {
    std::string category;
    std::string capability_id;
    std::string reason_code;
    std::string detail;
    std::uint64_t latency_ms = 0U;
    std::uint32_t mismatch_count = 0U;
    bool successful = false;
};

struct PolyglotMigrationTelemetryStream {
    std::vector<PolyglotMigrationEvent> events;
};

void record_polyglot_route_event(
    PolyglotMigrationTelemetryStream& stream,
    std::string_view capability_id,
    const PolyglotRouteDecision& decision);
void record_polyglot_bridge_event(
    PolyglotMigrationTelemetryStream& stream,
    std::string_view capability_id,
    std::uint64_t latency_ms,
    const PolyglotBridgeInvocationDecision& decision);
void record_polyglot_parity_events(
    PolyglotMigrationTelemetryStream& stream,
    const PolyglotParityComparisonRequest& request,
    const PolyglotParityComparisonResult& result);

}  // namespace copperfin::platform
