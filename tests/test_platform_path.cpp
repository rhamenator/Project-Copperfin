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

void test_case_insensitive_normalized_path_comparison() {
    const std::filesystem::path normalized("workspace/Copperfin.DBF");
    const std::filesystem::path equivalent("workspace/ignored/../copperfin.dbf");
    expect(copperfin::platform::path_equal_case_insensitive(normalized, equivalent),
           "#35: VFP path identity should normalize and compare without case on every host");

    const std::filesystem::path different("workspace/Copperfin.FPT");
    expect(!copperfin::platform::path_equal_case_insensitive(normalized, different),
           "#35: VFP path identity should reject a different normalized path");
}

// #5405: path_has_embedded_nul, path_has_dot_component,
// path_has_windows_alias_prone_component, and
// path_has_reserved_windows_device_name_component were duplicated verbatim
// across four workspace-agent security files; this is now their single
// shared definition. Direct coverage here is in addition to (not instead
// of) each of those four files' own existing regression coverage, which
// already exercises these predicates through their real strict-spelling
// call sites and continues to pass unchanged.
void test_embedded_nul_detection() {
    expect(!copperfin::platform::path_has_embedded_nul("ordinary/path.prg"),
           "#5405: an ordinary path should not report an embedded NUL");
    const std::filesystem::path::string_type embedded{
        std::filesystem::path::value_type{'a'},
        std::filesystem::path::value_type{},
        std::filesystem::path::value_type{'b'}};
    expect(copperfin::platform::path_has_embedded_nul(std::filesystem::path(embedded)),
           "#5405: a component containing a NUL byte must be detected");
}

void test_dot_component_detection() {
    expect(!copperfin::platform::path_has_dot_component("nested/child.prg"),
           "#5405: an ordinary relative path should have no dot component");
    expect(copperfin::platform::path_has_dot_component("nested/./child.prg"),
           "#5405: a literal '.' component must be detected");
    expect(copperfin::platform::path_has_dot_component("../outside/outside.prg"),
           "#5405: a literal '..' component must be detected");
}

void test_windows_alias_prone_component_detection() {
#if defined(_WIN32)
    expect(copperfin::platform::path_has_windows_alias_prone_component(
               std::filesystem::path(L"tool.")),
           "#5405: a trailing-dot component must be detected on Windows");
    expect(copperfin::platform::path_has_windows_alias_prone_component(
               std::filesystem::path(L"tool ")),
           "#5405: a trailing-space component must be detected on Windows");
    expect(!copperfin::platform::path_has_windows_alias_prone_component(
               std::filesystem::path(L"tool")),
           "#5405: an unambiguous component must not be flagged on Windows");
#else
    expect(!copperfin::platform::path_has_windows_alias_prone_component("tool."),
           "#5405: this check is always false on non-Windows platforms");
#endif
}

void test_reserved_windows_device_name_detection() {
#if defined(_WIN32)
    for (const wchar_t* device_name : {L"NUL", L"con", L"COM1", L"lpt1.txt"}) {
        expect(copperfin::platform::path_has_reserved_windows_device_name_component(
                   std::filesystem::path(device_name)),
               "#5405: a reserved device name component must be detected");
    }
    // Legacy MS-DOS device syntax ("NUL:") is still honored by Win32 path
    // resolution, so a colon-suffixed form must also be detected -- this is
    // the exact bypass an adversarial review found in the first, per-file
    // duplicated version of this check (issue #5404's follow-up fix).
    for (const wchar_t* device_name : {L"NUL:", L"NUL:hidden.txt", L"com1:stream"}) {
        expect(copperfin::platform::path_has_reserved_windows_device_name_component(
                   std::filesystem::path(device_name)),
               "#5405: a colon-suffixed reserved device name must be detected");
    }
    expect(!copperfin::platform::path_has_reserved_windows_device_name_component(
               std::filesystem::path(L"normal.txt")),
           "#5405: an ordinary filename must not be flagged as a reserved device name");
#else
    expect(!copperfin::platform::path_has_reserved_windows_device_name_component("NUL"),
           "#5405: this check is always false on non-Windows platforms");
#endif
}

}  // namespace

int main() {
    test_utf8_round_trip();
    test_empty_path_contract();
    test_invalid_utf8_contract();
    test_platform_component_comparison();
    test_case_insensitive_normalized_path_comparison();
    test_embedded_nul_detection();
    test_dot_component_detection();
    test_windows_alias_prone_component_detection();
    test_reserved_windows_device_name_detection();

    if (failures == 0) {
        std::cout << "Platform path tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
