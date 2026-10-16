// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#pragma once
#include "copperfin/runtime/prg_engine.h"
#include <cmath>
#include <optional>

namespace copperfin::runtime {

// RQ-CF-PRG-NUMERIC-REAL-ASSIGNMENT-001: bounded real Numeric scalar '='.
// Native observations/contract: fixtures/vfp9-numeric-real-assignment/.
// Rounded magnitude 1e10 controls do NOT authorize a wider magnitude rule.
inline std::optional<PrgNumericMetadata> assigned_real_numeric_metadata(
    std::optional<PrgNumericMetadata> metadata, double payload) noexcept {
    if (!metadata || metadata->storage != PrgNumericStorage::real ||
        metadata->decimal_places < 1U || metadata->decimal_places > 18U ||
        !std::isfinite(payload) || std::abs(payload) >= 10000000000.0) {
        return metadata;
    }
    // Scale checked before arithmetic: width is at most 30; old lexical width
    // is intentionally discarded. Negative zero has no extra sign width.
    return PrgNumericMetadata{11U + metadata->decimal_places + (payload < 0.0 ? 1U : 0U),
                              metadata->decimal_places, PrgNumericStorage::real};
}

} // namespace copperfin::runtime
