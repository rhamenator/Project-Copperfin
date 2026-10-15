// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#pragma once

#include "copperfin/runtime/prg_engine.h"

#include <limits>
#include <optional>

namespace copperfin::runtime {

struct NumericUnaryMetadataResult {
    std::optional<PrgNumericMetadata> metadata;
    bool overflow = false;
};

// RQ-CF-PRG-NUMERIC-UNARY-ATTRIBUTES-001: attributes only, not payload
// negation, constant-expression context, assignment normalization or display.
// Native evidence/limits: fixtures/vfp9-numeric-unary-metadata/attributes-requirements.md.
inline NumericUnaryMetadataResult negated_numeric_metadata(
    const std::optional<PrgNumericMetadata>& source) noexcept {
    if (!source) {
        return {};
    }
    if (source->display_width == std::numeric_limits<std::uint32_t>::max()) {
        return {std::nullopt, true};
    }
    return {PrgNumericMetadata{source->display_width + 1U, source->decimal_places}, false};
}

} // namespace copperfin::runtime
