// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#pragma once
#include "copperfin/runtime/prg_engine.h"
#include <cmath>
#include <optional>

namespace copperfin::runtime {

// RQ-CF-PRG-NUMERIC-INTEGER-ASSIGNMENT-001: ONLY known native Integer
// scalar-memory '=' assignment. No inference from unknown/real descriptors.
// Native source and pre-code contract: fixtures/vfp9-numeric-assignment-context/.
inline std::optional<PrgNumericMetadata> assigned_integer_numeric_metadata(
    std::optional<PrgNumericMetadata> metadata, double payload) noexcept {
    if (!metadata || metadata->storage != PrgNumericStorage::integer ||
        !std::isfinite(payload) || std::trunc(payload) != payload ||
        payload < -2147483647.0 || payload > 2147483647.0) {
        return metadata;
    }
    // Recovered native assigned widths, independent of lexical/source width.
    // Integer's unused native decimal slot is not read or propagated.
    return PrgNumericMetadata{payload <= -1000000000.0 ? 11U : 10U, 0U,
                              PrgNumericStorage::integer};
}

} // namespace copperfin::runtime
