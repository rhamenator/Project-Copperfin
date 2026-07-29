// Copyright 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/platform/invariant_numeric.h"

#include <charconv>
#include <cmath>

namespace copperfin::platform {

std::optional<double> try_parse_invariant_double(std::string_view value, const bool allow_nonfinite) {
    if (value.empty()) {
        return std::nullopt;
    }

    // VFP source and persisted fields use a period as the decimal separator.
    // from_chars is locale-independent, unlike strtod/std::stod.
    const char* begin = value.data();
    const char* end = begin + value.size();
    if (*begin == '+') {
        ++begin;
    }
    if (begin == end) {
        return std::nullopt;
    }

    double parsed = 0.0;
    const auto result = std::from_chars(begin, end, parsed, std::chars_format::general);
    if (result.ec != std::errc{} || result.ptr != end || (!allow_nonfinite && !std::isfinite(parsed))) {
        return std::nullopt;
    }
    return parsed;
}

}  // namespace copperfin::platform
