// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/platform/invariant_numeric.h"

#include <charconv>
#include <cmath>
#include <limits>
#include <locale>
#include <sstream>
#include <string>

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
        if (begin == end || *begin == '+' || *begin == '-') {
            return std::nullopt;
        }
    } else if (*begin == '-') {
        if (begin + 1 == end || begin[1] == '+' || begin[1] == '-') {
            return std::nullopt;
        }
    }

    if (allow_nonfinite) {
        const bool negative = value.front() == '-';
        const char* token_begin =
            (value.front() == '+' || negative) ? value.data() + 1 : value.data();
        std::string token{token_begin, end};
        for (char& character : token) {
            if (character >= 'A' && character <= 'Z') {
                character = static_cast<char>(character - 'A' + 'a');
            }
        }
        if (token == "nan") {
            return std::numeric_limits<double>::quiet_NaN();
        }
        if (token == "inf" || token == "infinity") {
            const double infinity = std::numeric_limits<double>::infinity();
            return negative ? -infinity : infinity;
        }
    }

    double parsed = 0.0;
#if defined(__APPLE__) && defined(_LIBCPP_VERSION)
    // Apple libc++ may not provide floating-point std::from_chars. Preserve
    // its locale-independent, no-leading-whitespace, full-consumption
    // contract with a classic-locale stream on that platform/STL pair.
    std::istringstream parser{std::string{begin, end}};
    parser.imbue(std::locale::classic());
    parser >> std::noskipws >> parsed;
    if (parser.fail() || !parser.eof()) {
        return std::nullopt;
    }
#else
    const auto result = std::from_chars(begin, end, parsed, std::chars_format::general);
    if (result.ec != std::errc{} || result.ptr != end) {
        return std::nullopt;
    }
#endif
    if (!allow_nonfinite && !std::isfinite(parsed)) {
        return std::nullopt;
    }
    return parsed;
}

}  // namespace copperfin::platform
