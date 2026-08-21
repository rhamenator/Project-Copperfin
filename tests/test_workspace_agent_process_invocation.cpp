// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/workspace_agent_session.h"
#include "copperfin/security/workspace_agent_tool_registry.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using copperfin::security::WorkspaceAgentAccessMode;
using copperfin::security::WorkspaceAgentActivationRequest;
using copperfin::security::WorkspaceAgentProcessEnvironmentPolicy;
using copperfin::security::WorkspaceAgentProcessInvocationPreflightRequest;
using copperfin::security::WorkspaceAgentSessionAuditCommitResult;
using copperfin::security::WorkspaceAgentSessionAuditEvent;
using copperfin::security::WorkspaceAgentSessionAuditSink;
using copperfin::security::WorkspaceAgentSessionController;

template <typename T>
concept HasArguments = requires(T value) {
    value.arguments;
};

template <typename T>
concept HasEnvironment = requires(T value) {
    value.environment;
};

template <typename T>
concept HasCommand = requires(T value) {
    value.command;
};

template <typename T>
concept HasSearchPath = requires(T value) {
    value.search_path;
};

static_assert(
    HasArguments<WorkspaceAgentProcessInvocationPreflightRequest> &&
        !HasEnvironment<WorkspaceAgentProcessInvocationPreflightRequest> &&
        !HasCommand<WorkspaceAgentProcessInvocationPreflightRequest> &&
        !HasSearchPath<WorkspaceAgentProcessInvocationPreflightRequest>,
    "RQ-CF-AGENT-011: invocation preflight accepts only direct arguments, never a command, environment, or PATH search");

int failures = 0;
std::filesystem::path running_test_executable;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

class TempTree {
public:
    TempTree() {
        const auto suffix = std::chrono::steady_clock::now()
                                .time_since_epoch()
                                .count();
        root = std::filesystem::temp_directory_path() /
            ("copperfin-agent-process-invocation-" + std::to_string(suffix));
        workspace = root / "workspace";
        outside = root / "outside";
        std::filesystem::create_directories(workspace / "bin");
        std::filesystem::create_directories(workspace / "working");
        std::filesystem::create_directories(outside / "bin");
        std::filesystem::create_directories(outside / "working");
        write_executable(workspace / "bin" / "workspace-tool");
        write_executable(outside / "bin" / "local-tool");
    }

    ~TempTree() {
        std::error_code ignored;
        std::filesystem::remove_all(root, ignored);
    }

    static void write_executable(const std::filesystem::path& path) {
#if defined(_WIN32)
        // Process admission intentionally requires a launch-compatible PE on
        // Windows.  Copy the test executable so this non-executing preflight
        // fixture exercises that real boundary instead of bypassing it.
        std::error_code copy_error;
        std::filesystem::copy_file(
            running_test_executable,
            path,
            std::filesystem::copy_options::overwrite_existing,
            copy_error);
        if (copy_error) {
            throw std::runtime_error("Windows PE fixture copy failed");
        }
#else
        std::ofstream stream(path, std::ios::binary | std::ios::trunc);
        stream << "must not execute\n";
        stream.close();
        std::error_code permission_error;
        std::filesystem::permissions(
            path,
            std::filesystem::perms::owner_exec |
                std::filesystem::perms::group_exec |
                std::filesystem::perms::others_exec,
            std::filesystem::perm_options::add,
            permission_error);
#endif
    }

    std::filesystem::path root;
    std::filesystem::path workspace;
    std::filesystem::path outside;
};

WorkspaceAgentSessionAuditCommitResult commit_audit(
    const WorkspaceAgentSessionAuditEvent&,
    void*) {
    return {.ok = true, .receipt = "process-invocation-test-receipt"};
}

WorkspaceAgentSessionAuditSink audit_sink() {
    return {.commit = commit_audit};
}

WorkspaceAgentActivationRequest activation_request(WorkspaceAgentAccessMode mode) {
    WorkspaceAgentActivationRequest request{
        .requested_mode = mode,
        .feature_enabled = true,
        .permission_granted = true,
        .trusted_product_ui = true,
        .audit_sink_available = true,
        .warning_presented = false,
        .warning_id = {},
        .user_confirmed = false};
    if (mode == WorkspaceAgentAccessMode::unrestricted_local) {
        request.warning_presented = true;
        request.warning_id =
            copperfin::security::workspace_agent_unrestricted_warning_id;
        request.user_confirmed = true;
    }
    return request;
}

WorkspaceAgentProcessInvocationPreflightRequest invocation_request(
    std::uint64_t generation,
    std::string tool_id,
    std::filesystem::path executable,
    std::filesystem::path working_directory,
    std::vector<std::string> arguments) {
    return {
        .session_generation = generation,
        .tool_id = std::move(tool_id),
        .executable_path = std::move(executable),
        .working_directory = std::move(working_directory),
        .arguments = std::move(arguments)};
}

void expect_content_free_denial(
    const copperfin::security::WorkspaceAgentProcessInvocationPreflightResult& result,
    const std::string& diagnostic,
    const std::string& message) {
    expect(!result.allowed && result.diagnostic_code == diagnostic &&
               result.tool_id.empty() &&
               result.canonical_executable_path.empty() &&
               result.canonical_working_directory.empty() &&
               result.arguments.empty(),
           message);
}

void test_bounded_direct_arguments_and_isolated_environment() {
    TempTree tree;
    WorkspaceAgentSessionController controller(tree.workspace);
    const auto start = controller.start(
        activation_request(WorkspaceAgentAccessMode::workspace_sandbox),
        audit_sink());
    expect(start.activated,
           "RQ-CF-AGENT-011: sandbox fixture must activate");

    const std::vector<std::string> direct_arguments{
        "", "literal && not-shell", "caf\xc3\xa9", "*.prg",
        "\xf0\x9f\x98\x80"};
    const auto allowed = controller.preflight_process_invocation_request(
        invocation_request(
            start.session.generation,
            std::string(
                copperfin::security::workspace_agent_tool_workspace_run_process),
            "bin/workspace-tool",
            "working",
            direct_arguments));
    expect(allowed.allowed &&
               allowed.diagnostic_code ==
                   "workspace_agent.process_invocation_request_allowed" &&
               allowed.arguments == direct_arguments &&
               !allowed.canonical_executable_path.empty() &&
               !allowed.canonical_working_directory.empty() &&
               allowed.environment_policy ==
                   WorkspaceAgentProcessEnvironmentPolicy::isolated_session_v1 &&
               !copperfin::security::
                   workspace_agent_process_environment_inherits_parent(
                       allowed.environment_policy),
           "RQ-CF-AGENT-011: direct arguments must remain distinct and require the isolated non-inheriting environment profile");

    const auto empty_vector = controller.preflight_process_invocation_request(
        invocation_request(
            start.session.generation,
            std::string(
                copperfin::security::workspace_agent_tool_workspace_run_process),
            "bin/workspace-tool",
            "working",
            {}));
    expect(empty_vector.allowed && empty_vector.arguments.empty(),
           "RQ-CF-AGENT-011: a process invocation may have no additional arguments");

    std::vector<std::string> exact_count(
        copperfin::security::workspace_agent_process_max_argument_count,
        "x");
    const auto exact_count_result =
        controller.preflight_process_invocation_request(invocation_request(
            start.session.generation,
            std::string(
                copperfin::security::workspace_agent_tool_workspace_run_process),
            "bin/workspace-tool",
            "working",
            exact_count));
    expect(exact_count_result.allowed &&
               exact_count_result.arguments.size() == exact_count.size(),
           "RQ-CF-AGENT-011: the exact argument-count limit must remain admissible");

    const std::vector<std::string> exact_byte_limits{
        std::string(
            copperfin::security::workspace_agent_process_max_argument_bytes,
            'a'),
        std::string(
            copperfin::security::workspace_agent_process_max_argument_bytes,
            'b')};
    const auto exact_byte_result =
        controller.preflight_process_invocation_request(invocation_request(
            start.session.generation,
            std::string(
                copperfin::security::workspace_agent_tool_workspace_run_process),
            "bin/workspace-tool",
            "working",
            exact_byte_limits));
    expect(exact_byte_result.allowed &&
               exact_byte_result.arguments == exact_byte_limits,
           "RQ-CF-AGENT-011: exact per-element and aggregate byte limits must remain admissible");

    std::vector<std::string> too_many(
        copperfin::security::workspace_agent_process_max_argument_count + 1U,
        "x");
    expect_content_free_denial(
        controller.preflight_process_invocation_request(invocation_request(
            start.session.generation,
            std::string(
                copperfin::security::workspace_agent_tool_workspace_run_process),
            "bin/workspace-tool",
            "working",
            std::move(too_many))),
        "workspace_agent.process_argument_count_exceeded",
        "RQ-CF-AGENT-011: excessive argument count must fail without reflecting targets or content");

    expect_content_free_denial(
        controller.preflight_process_invocation_request(invocation_request(
            start.session.generation,
            std::string(
                copperfin::security::workspace_agent_tool_workspace_run_process),
            "bin/workspace-tool",
            "working",
            {std::string(
                copperfin::security::workspace_agent_process_max_argument_bytes +
                    1U,
                'x')})),
        "workspace_agent.process_argument_size_exceeded",
        "RQ-CF-AGENT-011: oversized argument elements must fail closed");

    expect_content_free_denial(
        controller.preflight_process_invocation_request(invocation_request(
            start.session.generation,
            std::string(
                copperfin::security::workspace_agent_tool_workspace_run_process),
            "bin/workspace-tool",
            "working",
            {std::string(3000U, 'a'), std::string(3000U, 'b'),
             std::string(3000U, 'c')})),
        "workspace_agent.process_argument_total_size_exceeded",
        "RQ-CF-AGENT-011: excessive aggregate argument bytes must fail closed");

    std::string embedded_nul("before\0after", 12U);
    expect_content_free_denial(
        controller.preflight_process_invocation_request(invocation_request(
            start.session.generation,
            std::string(
                copperfin::security::workspace_agent_tool_workspace_run_process),
            "bin/workspace-tool",
            "working",
            {embedded_nul})),
        "workspace_agent.process_argument_embedded_nul",
        "RQ-CF-AGENT-011: embedded NUL arguments must fail closed");

    const std::vector<std::string> invalid_utf8_cases{
        std::string{"\xc0\xaf", 2U},
        std::string{"\x80", 1U},
        std::string{"\xe2\x82", 2U},
        std::string{"\xe2\x28\xa1", 3U},
        std::string{"\xed\xa0\x80", 3U},
        std::string{"\xf4\x90\x80\x80", 4U}};
    for (std::size_t index = 0U; index < invalid_utf8_cases.size(); ++index) {
        expect_content_free_denial(
            controller.preflight_process_invocation_request(invocation_request(
                start.session.generation,
                std::string(
                    copperfin::security::workspace_agent_tool_workspace_run_process),
                "bin/workspace-tool",
                "working",
                {invalid_utf8_cases[index]})),
            "workspace_agent.process_argument_invalid_utf8",
            "RQ-CF-AGENT-011: malformed UTF-8 class " +
                std::to_string(index) + " must fail closed");
    }
}

void test_session_tool_and_local_target_binding() {
    TempTree tree;
    WorkspaceAgentSessionController inactive(tree.workspace);
    expect_content_free_denial(
        inactive.preflight_process_invocation_request(invocation_request(
            1U,
            std::string(
                copperfin::security::workspace_agent_tool_workspace_run_process),
            "bin/workspace-tool",
            "working",
            {"secret-like-content"})),
        "workspace_agent.session_not_active",
        "RQ-CF-AGENT-011: inactive denial must precede target and argument reflection");

    WorkspaceAgentSessionController sandbox(tree.workspace);
    const auto sandbox_start = sandbox.start(
        activation_request(WorkspaceAgentAccessMode::workspace_sandbox),
        audit_sink());
    expect_content_free_denial(
        sandbox.preflight_process_invocation_request(invocation_request(
            sandbox_start.session.generation,
            std::string(copperfin::security::workspace_agent_tool_local_run_process),
            tree.outside / "bin" / "local-tool",
            tree.outside / "working",
            {"outside"})),
        "workspace_agent.tool_capability_denied",
        "RQ-CF-AGENT-011: sandbox denial must precede outside-target and argument reflection");
    expect_content_free_denial(
        sandbox.preflight_process_invocation_request(invocation_request(
            sandbox_start.session.generation + 1U,
            std::string(
                copperfin::security::workspace_agent_tool_workspace_run_process),
            "bin/workspace-tool",
            "working",
            {"stale"})),
        "workspace_agent.tool_stale_session",
        "RQ-CF-AGENT-011: stale sessions must fail before target and argument reflection");

    WorkspaceAgentSessionController unrestricted(tree.workspace);
    const auto unrestricted_start = unrestricted.start(
        activation_request(WorkspaceAgentAccessMode::unrestricted_local),
        audit_sink());
    const auto local = unrestricted.preflight_process_invocation_request(
        invocation_request(
            unrestricted_start.session.generation,
            std::string(copperfin::security::workspace_agent_tool_local_run_process),
            tree.outside / "bin" / "local-tool",
            tree.outside / "working",
            {"--direct", "value with spaces"}));
    expect(unrestricted_start.activated && local.allowed &&
               local.arguments ==
                   std::vector<std::string>({"--direct", "value with spaces"}) &&
               local.canonical_executable_path == std::filesystem::canonical(
                   tree.outside / "bin" / "local-tool"),
           "RQ-CF-AGENT-011: warned unrestricted sessions may bind bounded direct arguments to an explicit local process target");
}

}  // namespace

int main(int argc, char** argv) {
    if (argc > 0 && argv[0] != nullptr) {
        std::error_code canonical_error;
        running_test_executable = std::filesystem::canonical(
            std::filesystem::path(argv[0]), canonical_error);
    }
#if defined(_WIN32)
    if (running_test_executable.empty()) {
        std::cerr << "FAIL: Windows PE fixtures require the running test executable\n";
        return EXIT_FAILURE;
    }
#endif
    test_bounded_direct_arguments_and_isolated_environment();
    test_session_tool_and_local_target_binding();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
