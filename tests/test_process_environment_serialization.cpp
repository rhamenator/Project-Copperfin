// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/process_environment.h"

#include <iostream>
#include <string>
#include <vector>

namespace {

using copperfin::platform::ProcessEnvironmentEntry;
using copperfin::platform::ProcessEnvironmentTarget;
using copperfin::platform::serialize_process_environment;

int failures = 0;

void expect(const bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

void expect_denied(
    const std::vector<ProcessEnvironmentEntry>& entries,
    const ProcessEnvironmentTarget target,
    const std::size_t maximum,
    const std::string& diagnostic,
    const std::string& message) {
    const auto result = serialize_process_environment(entries, target, maximum);
    expect(!result.ok && result.diagnostic_code == diagnostic &&
               result.posix_entries.empty() && result.windows_block.empty(),
           message);
}

void test_posix_exact_bytes_and_limits() {
    const std::vector<ProcessEnvironmentEntry> entries{
        {.name = "B", .value = std::string(1U, static_cast<char>(0xff))},
        {.name = "A", .value = ""}};
    const auto result = serialize_process_environment(
        entries, ProcessEnvironmentTarget::posix_v1, 7U);
    expect(result.ok && result.posix_entries.size() == 2U &&
               result.posix_entries[0] == "B=" + entries[0].value &&
               result.posix_entries[1] == "A=" && result.windows_block.empty(),
           "RQ-CF-AGENT-013: POSIX must preserve input order and exact non-NUL bytes");
    expect_denied(
        entries, ProcessEnvironmentTarget::posix_v1, 6U,
        "platform.process_environment.size_limit_exceeded",
        "RQ-CF-AGENT-013: POSIX must count every entry terminator at the exact cap");

    const auto case_distinct = serialize_process_environment(
        {{.name = "PATH", .value = "one"}, {.name = "Path", .value = "two"}},
        ProcessEnvironmentTarget::posix_v1, 64U);
    expect(case_distinct.ok,
           "RQ-CF-AGENT-013: POSIX environment names must remain case-sensitive");
}

void test_windows_utf16_order_and_limits() {
    const std::vector<ProcessEnvironmentEntry> entries{
        {.name = "b", .value = "\xc3\xa9"},
        {.name = "A", .value = "\xf0\x9f\x98\x80"}};
    const std::u16string expected{
        u'A', u'=', static_cast<char16_t>(0xd83d), static_cast<char16_t>(0xde00),
        u'\0', u'b', u'=', static_cast<char16_t>(0x00e9), u'\0', u'\0'};
    const auto result = serialize_process_environment(
        entries, ProcessEnvironmentTarget::windows_utf16_v1, expected.size());
    expect(result.ok && result.windows_block == expected &&
               result.posix_entries.empty(),
           "RQ-CF-AGENT-013: Windows must sort case-insensitively and encode strict UTF-16");
    expect_denied(
        entries, ProcessEnvironmentTarget::windows_utf16_v1, expected.size() - 1U,
        "platform.process_environment.size_limit_exceeded",
        "RQ-CF-AGENT-013: Windows must count the final block terminator");

    const auto compact_non_bmp = serialize_process_environment(
        {{.name = "A", .value = "\xf0\x9f\x98\x80"}},
        ProcessEnvironmentTarget::windows_utf16_v1, 6U);
    expect(compact_non_bmp.ok && compact_non_bmp.windows_block.size() == 6U,
           "RQ-CF-AGENT-013: Windows must cap decoded UTF-16 units rather than UTF-8 input bytes");
    expect_denied(
        {{.name = "A", .value = "\xf0\x9f\x98\x80"}},
        ProcessEnvironmentTarget::windows_utf16_v1, 5U,
        "platform.process_environment.size_limit_exceeded",
        "RQ-CF-AGENT-013: bounded decoding must stop before exceeding the caller cap");

    const auto empty = serialize_process_environment(
        {}, ProcessEnvironmentTarget::windows_utf16_v1, 2U);
    expect(empty.ok && empty.windows_block == std::u16string(2U, u'\0'),
           "RQ-CF-AGENT-013: an empty Windows environment must be double-NUL terminated");

    constexpr std::size_t caller_limit = 40000U;
    const std::string maximum_value(caller_limit - 4U, 'x');
    expect(serialize_process_environment(
               {{.name = "A", .value = maximum_value}},
               ProcessEnvironmentTarget::windows_utf16_v1,
               caller_limit).ok,
           "RQ-CF-AGENT-013: Unicode Windows blocks above the ANSI limit must remain admissible");
    expect_denied(
        {{.name = "A", .value = maximum_value + "x"}},
        ProcessEnvironmentTarget::windows_utf16_v1,
        caller_limit,
        "platform.process_environment.size_limit_exceeded",
        "RQ-CF-AGENT-013: the explicit Windows caller cap must fail closed");
}

void test_rejected_ambiguity_and_encoding() {
    for (const std::string& name : std::vector<std::string>{
             "", "1A", "A-B", "A=B", std::string("A\0B", 3U)}) {
        expect_denied(
            {{.name = name, .value = "x"}}, ProcessEnvironmentTarget::posix_v1,
            64U, "platform.process_environment.invalid_name",
            "RQ-CF-AGENT-013: non-portable or ambiguous names must fail closed");
    }
    expect_denied(
        {{.name = "A", .value = std::string("x\0y", 3U)}},
        ProcessEnvironmentTarget::posix_v1, 64U,
        "platform.process_environment.embedded_nul",
        "RQ-CF-AGENT-013: embedded value NUL must fail closed");
    expect_denied(
        {{.name = "A", .value = "1"}, {.name = "A", .value = "2"}},
        ProcessEnvironmentTarget::posix_v1, 64U,
        "platform.process_environment.duplicate_name",
        "RQ-CF-AGENT-013: exact POSIX duplicates must fail closed");
    expect_denied(
        {{.name = "Path", .value = "1"}, {.name = "PATH", .value = "2"}},
        ProcessEnvironmentTarget::windows_utf16_v1, 64U,
        "platform.process_environment.duplicate_name",
        "RQ-CF-AGENT-013: case-insensitive Windows duplicates must fail closed");

    for (const std::string& invalid : {
             std::string("\xc0\x80", 2U), std::string("\xe2\x82", 2U),
             std::string("\xed\xa0\x80", 3U), std::string("\xf4\x90\x80\x80", 4U)}) {
        expect_denied(
            {{.name = "A", .value = invalid}},
            ProcessEnvironmentTarget::windows_utf16_v1, 64U,
            "platform.process_environment.invalid_utf8",
            "RQ-CF-AGENT-013: malformed or non-scalar Windows UTF-8 must fail closed");
    }
    expect_denied(
        {}, static_cast<ProcessEnvironmentTarget>(99U), 64U,
        "platform.process_environment.invalid_policy",
        "RQ-CF-AGENT-013: an unknown platform serialization contract must fail closed");
    expect_denied(
        {}, ProcessEnvironmentTarget::posix_v1, 0U,
        "platform.process_environment.invalid_policy",
        "RQ-CF-AGENT-013: a zero serialization cap must fail closed");
}

}  // namespace

int main() {
    test_posix_exact_bytes_and_limits();
    test_windows_utf16_order_and_limits();
    test_rejected_ambiguity_and_encoding();
    if (failures != 0) {
        std::cerr << failures << " process-environment serialization checks failed\n";
        return 1;
    }
    std::cout << "process-environment serialization checks passed\n";
    return 0;
}
