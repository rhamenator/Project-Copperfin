// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/safe_regex.h"

#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

void test_supported_subset() {
    using copperfin::platform::SafeRegexError;
    using copperfin::platform::search_safe_regex;
    using copperfin::platform::validate_safe_regex;

    expect(validate_safe_regex(R"(^item-[A-Z]\d+$)") == SafeRegexError::none,
           "supported anchors, classes, and shorthand should validate");
    const auto anchored = search_safe_regex("item-Z42", R"(^item-[A-Z]\d+$)");
    expect(anchored.ok() && anchored.matched && anchored.byte_offset == 0U &&
               anchored.byte_length == 8U,
           "anchored supported pattern should match the complete input");

    const auto greedy = search_safe_regex("abbb ab", "ab*");
    expect(greedy.ok() && greedy.matched && greedy.byte_offset == 0U &&
               greedy.byte_length == 4U,
           "search should select the leftmost-longest match");
    const auto later = search_safe_regex("abbb ab", "ab*", 5U);
    expect(later.ok() && later.matched && later.byte_offset == 5U &&
               later.byte_length == 2U,
           "search should honor the zero-based starting byte offset");

    const auto optional = search_safe_regex("color colour", "colou?r", 0U);
    expect(optional.ok() && optional.matched && optional.byte_length == 5U,
           "zero-or-one should match the earliest supported spelling");
    const auto positive = search_safe_regex("x=12345", R"(\d+)");
    expect(positive.ok() && positive.matched && positive.byte_offset == 2U &&
               positive.byte_length == 5U,
           "one-or-more shorthand should return exact match bounds");

    const auto insensitive = search_safe_regex("READY", "ready", 0U, true);
    expect(insensitive.ok() && insensitive.matched && insensitive.byte_length == 5U,
           "ASCII case-insensitive literals should be invariant");
    const auto negated = search_safe_regex("a", "[^A]", 0U, true);
    expect(negated.ok() && !negated.matched,
           "ASCII case folding should preserve negated-class meaning");
}

void test_fail_closed_contract() {
    using copperfin::platform::SafeRegexError;
    using copperfin::platform::SafeRegexLimits;
    using copperfin::platform::search_safe_regex;
    using copperfin::platform::validate_safe_regex;

    for (const std::string pattern : {"a|b", "(ab)", "a{2}", "[z-a]", "a**", "[\\D]"}) {
        expect(validate_safe_regex(pattern) == SafeRegexError::invalid_pattern,
               "unsupported or malformed syntax should fail closed: " + pattern);
    }
    expect(validate_safe_regex(std::string(257U, 'a')) == SafeRegexError::pattern_too_large,
           "pattern byte ceiling should be enforced");
    const auto oversized = search_safe_regex(std::string(65537U, 'a'), "a");
    expect(oversized.error == SafeRegexError::input_too_large && !oversized.matched,
           "input byte ceiling should be enforced");
    const auto invalid_start = search_safe_regex("abc", "a", 4U);
    expect(invalid_start.error == SafeRegexError::invalid_start,
           "starting byte offset beyond the terminal boundary should fail closed");

    SafeRegexLimits raised;
    raised.max_input_bytes = 65537U;
    expect(validate_safe_regex("a", raised) == SafeRegexError::invalid_limits,
           "callers should not raise hard ceilings");

    const std::string bounded_input(65536U, 'a');
    const auto adversarial = search_safe_regex(
        bounded_input,
        "a*a*a*a*a*a*a*a*b");
    expect(adversarial.ok() && !adversarial.matched,
           "overlapping repetition should complete without a backtracking engine");
}

}  // namespace

int main() {
    test_supported_subset();
    test_fail_closed_contract();
    if (failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
