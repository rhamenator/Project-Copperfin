// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/process_arguments.h"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

using copperfin::platform::ProcessArgumentTarget;
using copperfin::platform::serialize_process_arguments;

int failures = 0;

void expect(const bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

void test_posix_exact_elements_and_caps() {
    const std::string non_utf8("\xff\xfe", 2U);
    const auto result = serialize_process_arguments(
        "/opt/Copperfin tool",
        {"", "alpha beta", non_utf8},
        ProcessArgumentTarget::posix_v1,
        39U);
    expect(
        result.ok &&
            result.diagnostic_code == "platform.process_arguments.serialized" &&
            result.windows_command_line.empty() &&
            result.posix_arguments ==
                std::vector<std::string>{
                    "/opt/Copperfin tool", "", "alpha beta", non_utf8},
        "RQ-CF-AGENT-015: POSIX serialization must preserve argv[0], empty elements, spaces, and exact non-NUL bytes");

    const auto exact = serialize_process_arguments(
        "tool", {}, ProcessArgumentTarget::posix_v1, 5U);
    const auto short_cap = serialize_process_arguments(
        "tool", {}, ProcessArgumentTarget::posix_v1, 4U);
    expect(
        exact.ok && !short_cap.ok &&
            short_cap.diagnostic_code ==
                "platform.process_arguments.size_limit_exceeded" &&
            short_cap.posix_arguments.empty(),
        "RQ-CF-AGENT-015: POSIX cap must include each element terminator and fail content-free");
}

void test_windows_quoting_utf8_and_caps() {
    const auto result = serialize_process_arguments(
        R"(C:\Program Files\Copperfin\tool.exe)",
        {"", "plain", R"(one"two)", R"(trail\\)", "\xe9\x9b\xaa"},
        ProcessArgumentTarget::windows_command_line_v1,
        32767U);
    const std::u16string expected =
        uR"("C:\Program Files\Copperfin\tool.exe" "" "plain" "one\"two" "trail\\\\" "雪")";
    expect(
        result.ok && result.posix_arguments.empty() &&
            result.windows_command_line == expected,
        "RQ-CF-AGENT-015: Windows serialization must quote every element and apply the CRT quote/backslash convention");

    const auto exact = serialize_process_arguments(
        "tool", {}, ProcessArgumentTarget::windows_command_line_v1, 7U);
    const auto short_cap = serialize_process_arguments(
        "tool", {}, ProcessArgumentTarget::windows_command_line_v1, 6U);
    expect(
        exact.ok && exact.windows_command_line == uR"("tool")" &&
            !short_cap.ok &&
            short_cap.diagnostic_code ==
                "platform.process_arguments.size_limit_exceeded" &&
            short_cap.windows_command_line.empty(),
        "RQ-CF-AGENT-015: Windows cap must include the CreateProcessW terminating NUL and fail content-free");

    const std::string invalid_utf8("\xc0\xaf", 2U);
    const auto invalid = serialize_process_arguments(
        "tool",
        {invalid_utf8},
        ProcessArgumentTarget::windows_command_line_v1,
        32767U);
    expect(
        !invalid.ok &&
            invalid.diagnostic_code ==
                "platform.process_arguments.invalid_utf8" &&
            invalid.windows_command_line.empty(),
        "RQ-CF-AGENT-015: invalid Windows UTF-8 must fail without reflecting partially serialized content");
}

void test_fail_closed_inputs() {
    const auto empty = serialize_process_arguments(
        "", {}, ProcessArgumentTarget::posix_v1, 10U);
    const auto nul = serialize_process_arguments(
        "tool",
        {std::string("a\0b", 3U)},
        ProcessArgumentTarget::posix_v1,
        10U);
    const auto invalid_target = serialize_process_arguments(
        "tool",
        {},
        static_cast<ProcessArgumentTarget>(99U),
        std::numeric_limits<std::size_t>::max());
    const auto zero_cap = serialize_process_arguments(
        "tool", {}, ProcessArgumentTarget::posix_v1, 0U);
    expect(
        !empty.ok &&
            empty.diagnostic_code ==
                "platform.process_arguments.empty_executable" &&
            !nul.ok &&
            nul.diagnostic_code ==
                "platform.process_arguments.embedded_nul" &&
            !invalid_target.ok &&
            invalid_target.diagnostic_code ==
                "platform.process_arguments.invalid_policy" &&
            !zero_cap.ok &&
            zero_cap.diagnostic_code ==
                "platform.process_arguments.invalid_policy",
        "RQ-CF-AGENT-015: empty executable, embedded NUL, unknown target, and zero cap must fail closed");
}

}  // namespace

int main() {
    test_posix_exact_elements_and_caps();
    test_windows_quoting_utf8_and_caps();
    test_fail_closed_inputs();
    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
