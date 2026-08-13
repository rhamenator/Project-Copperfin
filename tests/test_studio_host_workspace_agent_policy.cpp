// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_environment_support.h"
#include "test_process_capture_support.h"

#include <filesystem>
#include <iostream>
#include <string>

namespace {
int failures = 0;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void expect_contains(const std::string& value, const std::string& expected, const char* message) {
    expect(value.find(expected) != std::string::npos, message);
}

std::size_t count_occurrences(const std::string& value, const std::string& expected) {
    std::size_t count = 0U;
    std::size_t offset = 0U;
    while ((offset = value.find(expected, offset)) != std::string::npos) {
        ++count;
        offset += expected.size();
    }
    return count;
}
}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_workspace_agent_policy <copperfin_studio_host>\n";
        return 2;
    }
    const auto work = std::filesystem::temp_directory_path();
    auto result = copperfin::test_support::normalize_captured_process_line_endings(
        copperfin::test_support::run_process_capture(
            argv[1], {"--workspace-agent-policy", "--json"}, work));
    expect(result.started && result.exit_code == 0, "JSON policy descriptor should succeed");
    expect_contains(result.stdout_text, "\"schemaVersion\": 1", "descriptor version must be explicit");
    expect_contains(result.stdout_text, "\"defaultMode\": \"advisory\"", "advisory must be the default");
    expect_contains(result.stdout_text, "\"featureEnabledByDefault\": false", "feature must default off");
    expect_contains(result.stdout_text, "\"providerAuthenticationGrantsLocalAuthority\": false", "provider authentication must not grant local authority");
    expect_contains(result.stdout_text, "\"permissionId\": \"ai.workspace_agent\"", "native permission must be named");
    expect_contains(result.stdout_text, "\"trustedProductUiRequired\": true", "trusted UI must remain mandatory");
    expect_contains(result.stdout_text, "\"auditSinkRequired\": true", "audit availability must remain mandatory");
    expect_contains(result.stdout_text, "\"id\": \"workspace-agent.unrestricted-local.v1\"", "warning version must remain exact");
    expect_contains(result.stdout_text, "\"name\": \"advisory\"", "advisory mode must be advertised");
    expect_contains(result.stdout_text, "\"name\": \"workspace_sandbox\"", "sandbox mode must be advertised");
    expect_contains(result.stdout_text, "\"name\": \"unrestricted_local\"", "unrestricted mode must be advertised");
    expect_contains(result.stdout_text, "\"accessOutsideWorkspace\": true", "unrestricted outside-workspace capability must be explicit");
    expect_contains(result.stdout_text, "\"useNetwork\": true", "unrestricted network capability must be explicit");
    expect_contains(result.stdout_text, "\"elevatePrivileges\": false", "privilege elevation must remain denied");
    expect(count_occurrences(result.stdout_text, "\"accessOutsideWorkspace\": true") == 1U,
        "only unrestricted mode may advertise outside-workspace access");
    expect(count_occurrences(result.stdout_text, "\"useNetwork\": true") == 1U,
        "only unrestricted mode may advertise network access");
    expect(count_occurrences(result.stdout_text, "\"readWorkspaceFiles\": true") == 2U,
        "only mutable modes may advertise workspace reads");
    expect(count_occurrences(result.stdout_text, "\"writeWorkspaceFiles\": true") == 2U,
        "only mutable modes may advertise workspace writes");
    expect(count_occurrences(result.stdout_text, "\"runLocalProcesses\": true") == 2U,
        "only mutable modes may advertise local process execution");
    expect(count_occurrences(result.stdout_text, "\"elevatePrivileges\": false") == 3U,
        "every advertised mode must deny privilege elevation");

    result = copperfin::test_support::normalize_captured_process_line_endings(
        copperfin::test_support::run_process_capture(
            argv[1], {"--workspace-agent-policy", "--json", "--list-subsystems"}, work));
    expect(result.started && result.exit_code == 2, "mixed policy and operational switches must fail");
    expect(result.stdout_text.empty(), "invalid policy arguments must not emit a partial descriptor");
    expect_contains(result.stderr_text, "accepts only", "invalid policy arguments should be diagnosed");

    result = copperfin::test_support::normalize_captured_process_line_endings(
        copperfin::test_support::run_process_capture(
            argv[1], {"--workspace-agent-policy", "--activate-unrestricted"}, work));
    expect(result.started && result.exit_code == 2, "generic CLI activation attempts must fail");

    result = copperfin::test_support::normalize_captured_process_line_endings(
        copperfin::test_support::run_process_capture(argv[1], {}, work));
    expect(result.started && result.exit_code == 2, "ordinary no-argument usage should remain unchanged");
    expect_contains(result.stdout_text,
        "copperfin_studio_host --workspace-agent-policy [--json]",
        "usage should advertise the read-only policy descriptor");

    {
        const copperfin::test_support::ScopedEnvironmentValue locale(
            "COPPERFIN_LOCALE", "qps-ploc");
        result = copperfin::test_support::normalize_captured_process_line_endings(
            copperfin::test_support::run_process_capture(
                argv[1], {"--workspace-agent-policy", "--json"}, work));
        expect(result.started && result.exit_code == 0,
            "pseudo-localized JSON policy descriptor should succeed");
        expect_contains(result.stdout_text, "[!! Enable unrestricted local agent access? !!]",
            "warning title should use the selected product locale");
        expect_contains(result.stdout_text, "\"id\": \"workspace-agent.unrestricted-local.v1\"",
            "localization must preserve the invariant warning id");
        expect_contains(result.stdout_text, "\"name\": \"unrestricted_local\"",
            "localization must preserve invariant mode names");
    }
    return failures == 0 ? 0 : 1;
}
