// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#pragma once

#include "copperfin/runtime/prg_engine.h"

#include <charconv>
#include <cmath>
#include <optional>
#include <string_view>

namespace copperfin::runtime {

enum class ConstantNumericArithmeticOperation {
    addition,
    subtraction,
    multiplication,
    power,
};

struct ConstantNumericArithmeticMetadata {
    PrgNumericMetadata intrinsic;
    PrgNumericMetadata materialized;

    bool operator==(const ConstantNumericArithmeticMetadata&) const = default;
};

namespace detail {

inline std::optional<unsigned> constant_arithmetic_decimals_setting(
    std::string_view text) noexcept {
    if (text.empty() || (text.size() > 1U && text.front() == '0')) return std::nullopt;
    unsigned value = 0U;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() || value > 18U)
        return std::nullopt;
    return value;
}

inline bool bounded_constant_arithmetic_operand(const PrgValue& value) noexcept {
    if (value.kind != PrgValueKind::number || value.is_null || !value.numeric_metadata ||
        !std::isfinite(value.number_value) || std::abs(value.number_value) >= 10000000000.0)
        return false;
    const auto& metadata = *value.numeric_metadata;
    if (metadata.display_width < 1U || metadata.display_width > 31U) return false;
    if (metadata.storage == PrgNumericStorage::integer)
        return metadata.display_width <= 12U && metadata.decimal_places == 0U &&
            std::trunc(value.number_value) == value.number_value &&
            std::abs(value.number_value) <= 2147483647.0;
    if (metadata.storage != PrgNumericStorage::real || metadata.decimal_places > 18U)
        return false;
    const auto suffix = metadata.decimal_places + (metadata.decimal_places > 0U ? 1U : 0U);
    return metadata.display_width >= suffix && metadata.display_width - suffix <= 15U;
}

} // namespace detail

// RQ-CF-PRG-NUMERIC-CONSTANT-ARITHMETIC-METADATA-001: bounded recovery for
// the native-evidenced constant power/multiply/additive precision chain. This
// is metadata-only: it never changes payloads, rounds, allocates, or recognizes
// source spelling. Unknown origins and descriptor shapes remain unknown.
inline std::optional<ConstantNumericArithmeticMetadata>
constant_numeric_arithmetic_metadata(
    ConstantNumericArithmeticOperation operation,
    const PrgValue& left,
    const PrgValue& right,
    double result,
    std::string_view decimals_text,
    bool constant_literal_node) noexcept {
    if (!constant_literal_node || !std::isfinite(result) ||
        std::abs(result) >= 10000000000.0 ||
        !detail::bounded_constant_arithmetic_operand(left) ||
        !detail::bounded_constant_arithmetic_operand(right))
        return std::nullopt;
    const auto decimals = detail::constant_arithmetic_decimals_setting(decimals_text);
    if (!decimals) return std::nullopt;
    const auto& l = *left.numeric_metadata;
    const auto& r = *right.numeric_metadata;
    const bool li = l.storage == PrgNumericStorage::integer;
    const bool ri = r.storage == PrgNumericStorage::integer;
    unsigned scale = 0U;
    unsigned width = 0U;
    switch (operation) {
    case ConstantNumericArithmeticOperation::power:
        if (!li || !ri || right.number_value >= 0.0) return std::nullopt;
        scale = *decimals;
        width = scale + 15U;
        break;
    case ConstantNumericArithmeticOperation::multiplication: {
        const auto* computed = !li && l.display_width == l.decimal_places + 15U ? &l
            : !ri && r.display_width == r.decimal_places + 15U ? &r : nullptr;
        if (!computed || !(li || ri)) return std::nullopt;
        scale = computed->decimal_places;
        width = scale + 15U;
        break;
    }
    case ConstantNumericArithmeticOperation::addition:
    case ConstantNumericArithmeticOperation::subtraction: {
        const auto* real = !li && ri ? &l : li && !ri ? &r : nullptr;
        if (!real) return std::nullopt;
        scale = real->decimal_places;
        if (real->display_width == scale + 15U) {
            width = scale + 15U;
        } else {
            const auto whole = real->display_width - scale - (scale > 0U ? 1U : 0U);
            const auto integer_width = li ? l.display_width : r.display_width;
            width = (whole > integer_width ? whole : integer_width) + scale +
                (scale > 0U ? 1U : 0U) + 1U;
        }
        break;
    }
    }
    if (width < 1U || width > 31U || scale > 18U || scale + 11U > 31U)
        return std::nullopt;
    return ConstantNumericArithmeticMetadata{
        {width, scale, PrgNumericStorage::real},
        {scale + 11U, scale, PrgNumericStorage::real}};
}

} // namespace copperfin::runtime
