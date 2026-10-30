// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#pragma once

#include "copperfin/runtime/prg_engine.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>

namespace copperfin::runtime {

namespace detail {

struct NumericDisplayWideUnsigned {
    std::uint64_t high;
    std::uint64_t low;
};

inline NumericDisplayWideUnsigned numeric_display_multiply_wide(
    const std::uint64_t left, const std::uint64_t right) noexcept {
    const std::uint64_t left_low = static_cast<std::uint32_t>(left);
    const std::uint64_t left_high = left >> 32U;
    const std::uint64_t right_low = static_cast<std::uint32_t>(right);
    const std::uint64_t right_high = right >> 32U;
    const std::uint64_t product0 = left_low * right_low;
    const std::uint64_t product1 = left_low * right_high;
    const std::uint64_t product2 = left_high * right_low;
    const std::uint64_t product3 = left_high * right_high;
    std::uint64_t low = product0;
    const std::uint64_t add1 = product1 << 32U;
    low += add1;
    const std::uint64_t carry1 = low < add1 ? 1U : 0U;
    const std::uint64_t add2 = product2 << 32U;
    const std::uint64_t before2 = low;
    low += add2;
    const std::uint64_t carry2 = low < before2 ? 1U : 0U;
    return {product3 + (product1 >> 32U) + (product2 >> 32U) + carry1 + carry2, low};
}

inline std::optional<std::uint64_t> numeric_display_round_power_of_two(
    NumericDisplayWideUnsigned value, const int binary_shift) noexcept {
    if (binary_shift >= 0) {
        if (binary_shift >= 64 || value.high != 0U ||
            (binary_shift != 0 && value.low > (std::numeric_limits<std::uint64_t>::max() >> binary_shift)))
            return std::nullopt;
        return value.low << binary_shift;
    }
    const unsigned right = static_cast<unsigned>(-binary_shift);
    if (right >= 128U) return 0U;
    std::uint64_t quotient = 0U;
    bool at_least_half = false;
    if (right >= 64U) {
        const unsigned high_shift = right - 64U;
        quotient = value.high >> high_shift;
        at_least_half = high_shift == 0U
            ? ((value.low >> 63U) & 1U) != 0U
            : ((value.high >> (high_shift - 1U)) & 1U) != 0U;
    } else if (right != 0U) {
        quotient = (value.high << (64U - right)) | (value.low >> right);
        at_least_half = ((value.low >> (right - 1U)) & 1U) != 0U;
    } else {
        quotient = value.low;
    }
    if (at_least_half && quotient == std::numeric_limits<std::uint64_t>::max())
        return std::nullopt;
    return quotient + (at_least_half ? 1U : 0U);
}

struct NumericDisplayRoundedDecimal {
    std::string text;
    bool zero;
};

inline std::optional<NumericDisplayRoundedDecimal> numeric_display_exact_fixed_half_away(
    const double value, const unsigned decimals) {
    if (decimals > 15U) return std::nullopt;
    const auto bits = std::bit_cast<std::uint64_t>(value);
    const auto exponent_bits = static_cast<unsigned>((bits >> 52U) & 0x7FFU);
    const auto fraction = bits & ((std::uint64_t{1} << 52U) - 1U);
    if (exponent_bits == 0x7FFU) return std::nullopt;
    const std::uint64_t significand = exponent_bits == 0U
        ? fraction : ((std::uint64_t{1} << 52U) | fraction);
    const int exponent = exponent_bits == 0U ? -1074 : static_cast<int>(exponent_bits) - 1023 - 52;
    std::uint64_t power5 = 1U;
    for (unsigned index = 0U; index < decimals; ++index) power5 *= 5U;
    const auto rounded = numeric_display_round_power_of_two(
        numeric_display_multiply_wide(significand, power5), exponent + static_cast<int>(decimals));
    if (!rounded) return std::nullopt;
    std::string digits = std::to_string(*rounded);
    const bool zero = *rounded == 0U;
    if (decimals == 0U) return NumericDisplayRoundedDecimal{std::move(digits), zero};
    if (digits.size() <= decimals)
        digits.insert(0U, static_cast<std::size_t>(decimals + 1U - digits.size()), '0');
    digits.insert(digits.size() - decimals, 1U, '.');
    return NumericDisplayRoundedDecimal{std::move(digits), zero};
}

} // namespace detail

// RQ-CF-PRG-NUMERIC-VALUE-DISPLAY-001: pure, bounded support for the frozen
// value-associated Numeric frontier. This remains unused until its direct gate
// passes and a separately reviewed consumer boundary supplies provenance.
inline std::optional<std::string> value_associated_numeric_display(
    const double value, PrgNumericMetadata metadata,
    const unsigned setting, const bool fixed) {
    if (!std::isfinite(value) || std::abs(value) >= 10000000000.0 || setting > 18U ||
        metadata.display_width < 1U || metadata.display_width > 31U)
        return std::nullopt;
    if (metadata.storage == PrgNumericStorage::integer) {
        if (metadata.display_width > 12U || std::trunc(value) != value ||
            std::abs(value) > 2147483647.0)
            return std::nullopt;
        metadata.decimal_places = 0U;
    } else {
        if (metadata.storage != PrgNumericStorage::real || metadata.decimal_places > 18U)
            return std::nullopt;
        const auto suffix = metadata.decimal_places + (metadata.decimal_places > 0U ? 1U : 0U);
        if (metadata.display_width <= suffix || metadata.display_width - suffix > 12U)
            return std::nullopt;
    }

    if (value == 0.0) {
        std::string zero = "0";
        if (fixed && setting > 0U) {
            zero.push_back('.');
            zero.append(setting, '0');
        }
        return zero;
    }

    const unsigned selected_decimals = fixed ? setting : metadata.decimal_places;
    const unsigned converted_decimals = std::min(selected_decimals, 15U);
    auto rounded_result = detail::numeric_display_exact_fixed_half_away(value, converted_decimals);
    if (!rounded_result) return std::nullopt;
    auto rounded = std::move(*rounded_result);

    if (!rounded.zero) {
        if (!fixed && std::trunc(value) == value) {
            const auto point = rounded.text.find('.');
            if (point != std::string::npos) {
                while (!rounded.text.empty() && rounded.text.back() == '0') rounded.text.pop_back();
                if (!rounded.text.empty() && rounded.text.back() == '.') rounded.text.pop_back();
            }
        }
        if (selected_decimals > converted_decimals &&
            (fixed || rounded.text.find('.') != std::string::npos)) {
            if (rounded.text.find('.') == std::string::npos) rounded.text.push_back('.');
            rounded.text.append(selected_decimals - converted_decimals, '0');
        }
        if (std::signbit(value)) rounded.text.insert(rounded.text.begin(), '-');
        return rounded.text;
    }

    if (fixed && selected_decimals > converted_decimals) {
        if (rounded.text.find('.') == std::string::npos) rounded.text.push_back('.');
        rounded.text.append(selected_decimals - converted_decimals, '0');
    }
    const bool preserve_sign = std::signbit(value) &&
        (!fixed || setting >= metadata.decimal_places);
    if (!fixed) rounded.text = "0";
    const bool omit_leading_zero = preserve_sign &&
        metadata.display_width < metadata.decimal_places + 3U;
    if (omit_leading_zero && !rounded.text.empty() && rounded.text.front() == '0')
        rounded.text.erase(rounded.text.begin());
    if (preserve_sign) rounded.text.insert(rounded.text.begin(), '-');
    return rounded.text;
}

} // namespace copperfin::runtime
