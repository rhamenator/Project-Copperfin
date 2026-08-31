// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/bounded_wide_string.h"

#include <iostream>
#include <memory>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

// #5402: get_company_name() (external_process_policy.cpp) and
// read_file_version_metadata() (file_version.cpp) both used to construct a
// std::wstring from a Win32 VerQueryValueW result via the
// null-terminator-scanning std::wstring(const wchar_t*) constructor,
// ignoring the API's own declared length. A version resource whose string
// data has no null terminator within that declared length made this an
// out-of-bounds heap read on attacker-influenced input (an executable's
// version resource is not necessarily already trusted at the point these
// functions run). bounded_wide_string() is the fix both call sites now
// share; this test allocates the buffer at the exact declared length with
// no terminator, so a regression back to the unbounded constructor is
// exactly the kind of heap-buffer-overflow ASan is built to catch.
void test_bounded_wide_string_stops_at_declared_length_without_terminator() {
    constexpr std::size_t declared_length = 8U;
    auto buffer = std::make_unique<wchar_t[]>(declared_length);
    for (std::size_t index = 0U; index < declared_length; ++index) {
        buffer[index] = L'A' + static_cast<wchar_t>(index);
    }

    const std::wstring result =
        copperfin::platform::bounded_wide_string(buffer.get(), declared_length);

    expect(result.size() == declared_length,
           "#5402: a non-terminated buffer should be read exactly to its declared length");
    expect(result == L"ABCDEFGH",
           "#5402: a non-terminated buffer's content should be preserved verbatim");
}

void test_bounded_wide_string_honors_an_embedded_terminator() {
    constexpr std::size_t declared_length = 8U;
    auto buffer = std::make_unique<wchar_t[]>(declared_length);
    buffer[0] = L'H';
    buffer[1] = L'i';
    buffer[2] = L'\0';
    for (std::size_t index = 3U; index < declared_length; ++index) {
        buffer[index] = L'Z';
    }

    const std::wstring result =
        copperfin::platform::bounded_wide_string(buffer.get(), declared_length);

    expect(result == L"Hi",
           "#5402: an embedded terminator well within the declared length should still stop the string there");
}

void test_bounded_wide_string_handles_a_zero_declared_length() {
    const wchar_t probe = L'X';
    const std::wstring result =
        copperfin::platform::bounded_wide_string(&probe, 0U);
    expect(result.empty(),
           "#5402: a zero declared length should produce an empty string without reading the buffer");
}

}  // namespace

int main() {
    test_bounded_wide_string_stops_at_declared_length_without_terminator();
    test_bounded_wide_string_honors_an_embedded_terminator();
    test_bounded_wide_string_handles_a_zero_declared_length();

    if (failures != 0) {
        std::cerr << failures << " assertion(s) failed.\n";
        return 1;
    }
    std::cout << "All bounded wide string tests passed.\n";
    return 0;
}
