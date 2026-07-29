// Copyright 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include <charconv>
#include <optional>
#include <string_view>

namespace copperfin::platform {

// Parse machine/VFP numeric text without consulting the host display locale.
// Nonfinite values are rejected unless the consuming binary-field contract
// explicitly opts in to preserving them.
[[nodiscard]] std::optional<double> try_parse_invariant_double(
    std::string_view value,
    bool allow_nonfinite = false);

template <typename Integer>
[[nodiscard]] std::optional<Integer> try_parse_invariant_integer(std::string_view value) {
    if (value.empty()) {
        return std::nullopt;
    }
    if (value.front() == '+') {
        value.remove_prefix(1U);
        if (value.empty()) {
            return std::nullopt;
        }
    }

    Integer parsed{};
    const char* begin = value.data();
    const char* end = begin + value.size();
    const auto result = std::from_chars(begin, end, parsed, 10);
    if (result.ec != std::errc{} || result.ptr != end) {
        return std::nullopt;
    }
    return parsed;
}

}  // namespace copperfin::platform
