// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/path.h"

#include <filesystem>
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

void test_utf8_round_trip() {
    const std::string utf8 = "caf\xC3\xA9-\xD0\x96-\xE7\x8C\xAB";
    const auto path = copperfin::platform::path_from_utf8_string(utf8);
    expect(!path.empty(), "#35: valid UTF-8 should produce a native path");
    expect(copperfin::platform::path_to_utf8_string(path) == utf8,
           "#35: native path conversion should preserve UTF-8 bytes");
}

void test_empty_path_contract() {
    expect(copperfin::platform::path_from_utf8_string("").empty(),
           "#35: an empty UTF-8 value should produce an empty path");
    expect(copperfin::platform::path_to_utf8_string({}).empty(),
           "#35: an empty path should produce an empty UTF-8 value");
}

void test_invalid_utf8_contract() {
    const std::string invalid_utf8 = "\xC3\x28";
    const auto path = copperfin::platform::path_from_utf8_string(invalid_utf8);
#if defined(_WIN32)
    expect(path.empty(), "#35: Windows path conversion should reject invalid UTF-8");
#else
    expect(copperfin::platform::path_to_utf8_string(path) == invalid_utf8,
           "#35: POSIX path conversion should preserve native path bytes");
#endif
}

void test_platform_component_comparison() {
    const std::filesystem::path exact("Copperfin");
    expect(copperfin::platform::path_component_equal_for_platform(exact, exact),
           "#35: identical path components should compare equal");

    const std::filesystem::path different_case("copperfin");
#if defined(_WIN32)
    expect(copperfin::platform::path_component_equal_for_platform(exact, different_case),
           "#35: Windows path components should compare without case");
#else
    expect(!copperfin::platform::path_component_equal_for_platform(exact, different_case),
           "#35: POSIX path components should preserve case sensitivity");
#endif
}

}  // namespace

int main() {
    test_utf8_round_trip();
    test_empty_path_contract();
    test_invalid_utf8_contract();
    test_platform_component_comparison();

    if (failures == 0) {
        std::cout << "Platform path tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
