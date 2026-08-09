// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstddef>
#include <string_view>

namespace copperfin::platform {

enum class SafeRegexError {
    none,
    invalid_limits,
    input_too_large,
    pattern_too_large,
    state_limit_exceeded,
    invalid_pattern,
    invalid_start
};

struct SafeRegexLimits final {
    std::size_t max_input_bytes = 64U * 1024U;
    std::size_t max_pattern_bytes = 256U;
    std::size_t max_state_count = 512U;
};

struct SafeRegexMatch final {
    SafeRegexError error = SafeRegexError::none;
    bool matched = false;
    std::size_t byte_offset = 0U;
    std::size_t byte_length = 0U;

    [[nodiscard]] bool ok() const noexcept {
        return error == SafeRegexError::none;
    }
};

[[nodiscard]] SafeRegexError validate_safe_regex(
    std::string_view pattern,
    const SafeRegexLimits& limits = {});

[[nodiscard]] SafeRegexMatch search_safe_regex(
    std::string_view input,
    std::string_view pattern,
    std::size_t start_byte_offset = 0U,
    bool ignore_ascii_case = false,
    const SafeRegexLimits& limits = {});

}  // namespace copperfin::platform
