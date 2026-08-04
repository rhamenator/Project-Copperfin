// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace copperfin::platform {

enum class PolyglotRouteState {
    off,
    shadow,
    canary,
    on,
    retire_legacy
};

enum class PolyglotRouteConfigError {
    none,
    invalid_document,
    invalid_version,
    missing_routes,
    capability_id_required,
    invalid_capability_id,
    invalid_state,
    invalid_canary_percentage,
    duplicate_capability_id
};

enum class PolyglotRouteSelection {
    native,
    shadow,
    candidate
};

struct PolyglotRouteConfig {
    std::string capability_id;
    std::string state;
    std::uint8_t canary_percentage = 0U;
};

struct PolyglotRouteEntry {
    std::string capability_id;
    PolyglotRouteState state = PolyglotRouteState::off;
    std::uint8_t canary_percentage = 0U;
};

struct PolyglotRouteRegistry {
    std::vector<PolyglotRouteEntry> entries;
};

struct PolyglotRouteRegistryResult {
    PolyglotRouteRegistry registry;
    PolyglotRouteConfigError error = PolyglotRouteConfigError::none;
    std::string error_code;

    [[nodiscard]] bool ok() const noexcept {
        return error == PolyglotRouteConfigError::none;
    }
};

struct PolyglotRouteDecision {
    PolyglotRouteState state = PolyglotRouteState::off;
    PolyglotRouteSelection selection = PolyglotRouteSelection::native;
    bool invoke_native = true;
    bool invoke_candidate = false;
    bool candidate_primary = false;
    bool native_fallback_allowed = true;
    std::string reason_code;
};

[[nodiscard]] const char* polyglot_route_state_name(PolyglotRouteState state) noexcept;
[[nodiscard]] bool parse_polyglot_route_state(
    std::string_view value,
    PolyglotRouteState& state) noexcept;
[[nodiscard]] PolyglotRouteRegistryResult load_polyglot_route_registry(
    const std::vector<PolyglotRouteConfig>& configs);
[[nodiscard]] PolyglotRouteRegistryResult load_polyglot_route_registry_json(
    std::string_view document);
[[nodiscard]] PolyglotRouteDecision evaluate_polyglot_route(
    const PolyglotRouteRegistry& registry,
    std::string_view capability_id,
    std::uint8_t selection_sample = 0U);

}  // namespace copperfin::platform
