// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include <charconv>
#include <cmath>
#include <limits>
#include <optional>
#include <string_view>

namespace copperfin::vfp {

inline bool is_report_settings_root_record(int objtype) {
    return objtype == 1;
}

inline int truncate_report_layout_geometry(double value, int fallback = 0) {
    if (!std::isfinite(value) ||
        value < static_cast<double>(std::numeric_limits<int>::min()) ||
        value > static_cast<double>(std::numeric_limits<int>::max())) {
        return fallback;
    }
    return static_cast<int>(value);
}

inline std::optional<int> parse_truncated_fixed_decimal_int(std::string_view value) {
    if (value.empty()) {
        return std::nullopt;
    }

    const std::size_t decimal = value.find('.');
    if (decimal != std::string_view::npos &&
        value.find('.', decimal + 1U) != std::string_view::npos) {
        return std::nullopt;
    }
    const std::string_view integer = value.substr(0U, decimal);
    if (integer.empty()) {
        return std::nullopt;
    }
    if (decimal != std::string_view::npos) {
        for (const char character : value.substr(decimal + 1U)) {
            if (character < '0' || character > '9') {
                return std::nullopt;
            }
        }
    }

    int parsed = 0;
    const auto result = std::from_chars(integer.data(), integer.data() + integer.size(), parsed);
    if (result.ec != std::errc{} || result.ptr != integer.data() + integer.size()) {
        return std::nullopt;
    }
    return parsed;
}

} // namespace copperfin::vfp
