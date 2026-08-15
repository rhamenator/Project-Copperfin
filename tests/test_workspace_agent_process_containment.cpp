// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/workspace_agent_process_containment.h"
#include "copperfin/security/workspace_agent_session.h"
#include "copperfin/security/workspace_agent_tool_registry.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

using copperfin::security::WorkspaceAgentAccessMode;
using copperfin::security::WorkspaceAgentActivationRequest;
using copperfin::security::WorkspaceAgentProcessTargetBoundary;
using copperfin::security::WorkspaceAgentProcessTargetPreflightRequest;
using copperfin::security::WorkspaceAgentSessionAuditCommitResult;
using copperfin::security::WorkspaceAgentSessionAuditEvent;
using copperfin::security::WorkspaceAgentSessionAuditSink;
using copperfin::security::WorkspaceAgentSessionController;

int failures = 0;

template <typename T>
concept HasCommandText = requires(T value) {
    value.command;
};

template <typename T>
concept HasArguments = requires(T value) {
    value.arguments;
};

template <typename T>
concept HasEnvironment = requires(T value) {
    value.environment;
};

template <typename T>
concept HasCallerSelectedRoot = requires(T value) {
    value.workspace_root;
};

template <typename T>
concept HasSearchPath = requires(T value) {
    value.search_path;
};

static_assert(
    !HasCommandText<WorkspaceAgentProcessTargetPreflightRequest> &&
        !HasArguments<WorkspaceAgentProcessTargetPreflightRequest> &&
        !HasEnvironment<WorkspaceAgentProcessTargetPreflightRequest> &&
        !HasCallerSelectedRoot<WorkspaceAgentProcessTargetPreflightRequest> &&
        !HasSearchPath<WorkspaceAgentProcessTargetPreflightRequest>,
    "RQ-CF-AGENT-010: target preflight must not expose commands, arguments, environment, root, or search-path authority");

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
            ("copperfin-agent-process-target-" + std::to_string(suffix));
        workspace = root / "workspace";
        outside = root / "outside";
        std::filesystem::create_directories(workspace / "bin");
        std::filesystem::create_directories(workspace / "nested");
        std::filesystem::create_directories(outside / "bin");
        std::filesystem::create_directories(outside / "working");
        write_executable(workspace / "bin" / "workspace-tool");
        write_executable(outside / "bin" / "local-tool");
        write_plain(workspace / "not-executable");
    }

    ~TempTree() {
        std::error_code ignored;
        std::filesystem::remove_all(root, ignored);
        std::filesystem::remove_all(root.string() + "-moved", ignored);
    }

    static void write_plain(const std::filesystem::path& path) {
        std::ofstream stream(path, std::ios::binary | std::ios::trunc);
        stream << "test\n";
    }

    static void write_executable(const std::filesystem::path& path) {
        write_plain(path);
        std::error_code permission_error;
        std::filesystem::permissions(
            path,
            std::filesystem::perms::owner_exec |
                std::filesystem::perms::group_exec |
                std::filesystem::perms::others_exec,
            std::filesystem::perm_options::add,
            permission_error);
    }

    std::filesystem::path root;
    std::filesystem::path workspace;
    std::filesystem::path outside;
};

WorkspaceAgentSessionAuditCommitResult commit_audit(
    const WorkspaceAgentSessionAuditEvent&,
    void*) {
    return {.ok = true, .receipt = "process-containment-test-receipt"};
}

WorkspaceAgentSessionAuditSink audit_sink() {
    return {.commit = commit_audit};
}

WorkspaceAgentActivationRequest request_for(WorkspaceAgentAccessMode mode) {
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

WorkspaceAgentProcessTargetPreflightRequest process_request(
    std::uint64_t generation,
    std::string tool_id,
    std::filesystem::path executable,
    std::filesystem::path working_directory) {
    return {
        .session_generation = generation,
        .tool_id = std::move(tool_id),
        .executable_path = std::move(executable),
        .working_directory = std::move(working_directory)};
}

void test_boundary_requires_explicit_direct_process_targets() {
    TempTree tree;
    expect(!WorkspaceAgentProcessTargetBoundary::create("relative/workspace").has_value(),
           "RQ-CF-AGENT-010: process containment must require an absolute trusted root");
    expect(!WorkspaceAgentProcessTargetBoundary::create(
                tree.workspace / "bin" / "workspace-tool")
                .has_value(),
           "RQ-CF-AGENT-010: a file cannot configure the process workspace boundary");

    auto boundary = WorkspaceAgentProcessTargetBoundary::create(tree.workspace);
    expect(boundary.has_value(),
           "RQ-CF-AGENT-010: a direct absolute workspace root must configure process containment");
    if (!boundary.has_value()) {
        return;
    }

    const auto root_working_directory = boundary->inspect_workspace_process(
        "bin/workspace-tool", ".");
    expect(root_working_directory.allowed &&
               root_working_directory.canonical_executable_path ==
                   std::filesystem::canonical(
                       tree.workspace / "bin" / "workspace-tool") &&
               root_working_directory.canonical_working_directory ==
                   std::filesystem::canonical(tree.workspace),
           "RQ-CF-AGENT-010: explicit workspace executable and root working directory must bind physical identities");
    const auto nested_working_directory = boundary->inspect_workspace_process(
        "bin/workspace-tool", "nested");
    expect(nested_working_directory.allowed &&
               nested_working_directory.canonical_working_directory ==
                   std::filesystem::canonical(tree.workspace / "nested"),
           "RQ-CF-AGENT-010: a direct contained workspace working directory must be admitted");

    for (const std::filesystem::path& invalid_executable : {
             std::filesystem::path{},
             std::filesystem::path{"workspace-tool"} / "..",
             std::filesystem::path{"../outside/bin/local-tool"},
             tree.workspace / "bin" / "workspace-tool"}) {
        const auto denied = boundary->inspect_workspace_process(
            invalid_executable, ".");
        expect(!denied.allowed && denied.canonical_executable_path.empty() &&
                   denied.canonical_working_directory.empty(),
               "RQ-CF-AGENT-010: ambiguous workspace executable spellings must fail without reflection");
    }
    for (const std::filesystem::path& invalid_working_directory : {
             std::filesystem::path{},
             std::filesystem::path{".."},
             std::filesystem::path{"nested/.."},
             tree.workspace}) {
        const auto denied = boundary->inspect_workspace_process(
            "bin/workspace-tool", invalid_working_directory);
        expect(!denied.allowed && denied.canonical_executable_path.empty() &&
                   denied.canonical_working_directory.empty(),
               "RQ-CF-AGENT-010: ambiguous workspace working directories must fail without reflection");
    }
    const std::filesystem::path executable_nul{
        std::string{"bin/tool\0suffix", 15U}};
    const std::filesystem::path working_directory_nul{
        std::string{"nested\0suffix", 13U}};
    expect(!boundary->inspect_workspace_process(executable_nul, ".").allowed,
           "RQ-CF-AGENT-010: executable NULs must fail before lookup");
    expect(!boundary->inspect_workspace_process(
                "bin/workspace-tool", working_directory_nul)
                .allowed,
           "RQ-CF-AGENT-010: working-directory NULs must fail before lookup");
#if defined(_WIN32)
    expect(!boundary->inspect_workspace_process(
                "bin/workspace-tool:stream", ".")
                .allowed,
           "RQ-CF-AGENT-010: workspace executable alternate streams must fail before lookup");
    expect(!boundary->inspect_workspace_process(
                "bin/workspace-tool", "nested:stream")
                .allowed,
           "RQ-CF-AGENT-010: workspace working-directory alternate streams must fail before lookup");
    expect(!boundary->inspect_local_process(
                std::filesystem::path(
                    (tree.outside / "bin" / "local-tool").wstring() +
                    L":stream"),
                tree.outside / "working")
                .allowed,
           "RQ-CF-AGENT-010: local executable alternate streams must fail before lookup");
    expect(!boundary->inspect_local_process(
                tree.outside / "bin" / "local-tool",
                std::filesystem::path(
                    (tree.outside / "working").wstring() + L":stream"))
                .allowed,
           "RQ-CF-AGENT-010: local working-directory alternate streams must fail before lookup");
    expect(!boundary->inspect_local_process(
                R"(\\?\C:\Windows\System32\cmd.exe)",
                R"(\\?\C:\Windows)")
                .allowed,
           "RQ-CF-AGENT-010: Windows device paths must fail before lookup");
    expect(!boundary->inspect_local_process(
                R"(\\server\share\tool.exe)",
                R"(C:\Windows)")
                .allowed,
           "RQ-CF-AGENT-010: UNC executables must fail before lookup");
    expect(!boundary->inspect_local_process(
                R"(C:\Windows\System32\cmd.exe)",
                R"(\\server\share\working)")
                .allowed,
           "RQ-CF-AGENT-010: UNC working directories must fail before lookup");
#endif
    expect(!boundary->inspect_workspace_process("missing", ".").allowed,
           "RQ-CF-AGENT-010: a missing executable must fail closed");
    expect(!boundary->inspect_workspace_process("workspace-tool", ".").allowed,
           "RQ-CF-AGENT-010: a bare name must resolve only as an explicit workspace-relative path, never through PATH");
    expect(!boundary->inspect_workspace_process("nested", ".").allowed,
           "RQ-CF-AGENT-010: a directory cannot become an executable target");
    expect(!boundary->inspect_workspace_process(
                "bin/workspace-tool", "not-executable")
                .allowed,
           "RQ-CF-AGENT-010: a regular file cannot become a working directory");
#if !defined(_WIN32)
    expect(!boundary->inspect_workspace_process("not-executable", ".").allowed,
           "RQ-CF-AGENT-010: POSIX process targets must carry execute permission");
#endif

    const auto local = boundary->inspect_local_process(
        tree.outside / "bin" / "local-tool",
        tree.outside / "working");
    expect(local.allowed &&
               local.canonical_executable_path ==
                   std::filesystem::canonical(
                       tree.outside / "bin" / "local-tool") &&
               local.canonical_working_directory ==
                   std::filesystem::canonical(tree.outside / "working"),
           "RQ-CF-AGENT-010: unrestricted-local inspection may bind explicit direct absolute process targets");
    expect(!boundary->inspect_local_process(
                "outside/bin/local-tool", tree.outside / "working")
                .allowed,
           "RQ-CF-AGENT-010: local executable targets must be absolute");
    expect(!boundary->inspect_local_process(
                tree.outside / "bin" / "local-tool", "outside/working")
                .allowed,
           "RQ-CF-AGENT-010: local working-directory targets must be absolute");
}

void test_boundary_rejects_process_target_indirection_and_aliasing() {
    TempTree tree;
    auto boundary = WorkspaceAgentProcessTargetBoundary::create(tree.workspace);
    expect(boundary.has_value(),
           "RQ-CF-AGENT-010: indirection fixture must configure process containment");
    if (!boundary.has_value()) {
        return;
    }

    std::error_code link_error;
    std::filesystem::create_symlink(
        tree.outside / "bin" / "local-tool",
        tree.workspace / "bin" / "redirect-tool",
        link_error);
    if (!link_error) {
        const auto redirected = boundary->inspect_workspace_process(
            "bin/redirect-tool", ".");
        expect(!redirected.allowed &&
                   redirected.diagnostic_code ==
                       "workspace_agent.process_executable_indirect_component",
               "RQ-CF-AGENT-010: an indirect workspace executable must fail closed");
    }

    link_error.clear();
    std::filesystem::create_directory_symlink(
        tree.outside / "working",
        tree.workspace / "redirect-working",
        link_error);
    if (!link_error) {
        const auto redirected = boundary->inspect_workspace_process(
            "bin/workspace-tool", "redirect-working");
        expect(!redirected.allowed &&
                   redirected.diagnostic_code ==
                       "workspace_agent.process_working_directory_indirect_component",
               "RQ-CF-AGENT-010: an indirect workspace working directory must fail closed");

        const auto local_redirected = boundary->inspect_local_process(
            tree.outside / "bin" / "local-tool",
            tree.workspace / "redirect-working");
        expect(!local_redirected.allowed &&
                   local_redirected.diagnostic_code ==
                       "workspace_agent.process_working_directory_indirect_component",
               "RQ-CF-AGENT-010: unrestricted mode must still reject an indirect working-directory leaf");
    }

    link_error.clear();
    std::filesystem::create_hard_link(
        tree.workspace / "bin" / "workspace-tool",
        tree.workspace / "bin" / "workspace-tool-alias",
        link_error);
    if (!link_error) {
        const auto multiply_linked = boundary->inspect_workspace_process(
            "bin/workspace-tool", ".");
        expect(!multiply_linked.allowed &&
                   multiply_linked.diagnostic_code ==
                       "workspace_agent.process_executable_multiply_linked",
               "RQ-CF-AGENT-010: multiply linked executable targets must fail closed");
    }
}

void test_session_bound_process_target_preflight() {
    TempTree tree;
    WorkspaceAgentSessionController inactive(tree.workspace);
    const auto inactive_result = inactive.preflight_process_target_request(
        process_request(
            1U,
            std::string(
                copperfin::security::workspace_agent_tool_workspace_run_process),
            "bin/workspace-tool",
            "."));
    expect(!inactive_result.allowed &&
               inactive_result.diagnostic_code ==
                   "workspace_agent.session_not_active" &&
               inactive_result.canonical_executable_path.empty() &&
               inactive_result.canonical_working_directory.empty(),
           "RQ-CF-AGENT-010: inactive denial must precede process target reflection");

    WorkspaceAgentSessionController unconfigured;
    const auto unconfigured_start = unconfigured.start(
        request_for(WorkspaceAgentAccessMode::workspace_sandbox), audit_sink());
    const auto unconfigured_target =
        unconfigured.preflight_process_target_request(process_request(
            unconfigured_start.session.generation,
            std::string(
                copperfin::security::workspace_agent_tool_workspace_run_process),
            "bin/workspace-tool",
            "."));
    expect(!unconfigured_target.allowed &&
               unconfigured_target.diagnostic_code ==
                   "workspace_agent.process_workspace_root_not_configured",
           "RQ-CF-AGENT-010: process preflight without a trusted product root must fail closed");

    WorkspaceAgentSessionController sandbox(tree.workspace);
    const auto sandbox_start = sandbox.start(
        request_for(WorkspaceAgentAccessMode::workspace_sandbox), audit_sink());
    expect(sandbox_start.activated,
           "RQ-CF-AGENT-010: sandbox fixture session must activate");
    auto invalid_schema_request = process_request(
        sandbox_start.session.generation,
        std::string(
            copperfin::security::workspace_agent_tool_workspace_run_process),
        "bin/workspace-tool",
        ".");
    invalid_schema_request.schema_version = 2U;
    const auto invalid_schema = sandbox.preflight_process_target_request(
        invalid_schema_request);
    expect(!invalid_schema.allowed &&
               invalid_schema.diagnostic_code ==
                   "workspace_agent.tool_invalid_schema",
           "RQ-CF-AGENT-010: unknown schemas must fail before process target lookup");
    const auto unknown_tool = sandbox.preflight_process_target_request(
        process_request(
            sandbox_start.session.generation,
            "provider.run.v1",
            "bin/workspace-tool",
            "."));
    expect(!unknown_tool.allowed &&
               unknown_tool.diagnostic_code ==
                   "workspace_agent.tool_not_registered",
           "RQ-CF-AGENT-010: provider-defined tools must fail before process target lookup");
    const auto allowed = sandbox.preflight_process_target_request(process_request(
        sandbox_start.session.generation,
        std::string(
            copperfin::security::workspace_agent_tool_workspace_run_process),
        "bin/workspace-tool",
        "nested"));
    expect(allowed.allowed &&
               allowed.diagnostic_code ==
                   "workspace_agent.process_target_request_allowed" &&
               !allowed.canonical_executable_path.empty() &&
               !allowed.canonical_working_directory.empty(),
           "RQ-CF-AGENT-010: sandbox process preflight must bind the exact session and both physical targets");
    const auto file_tool = sandbox.preflight_process_target_request(process_request(
        sandbox_start.session.generation,
        std::string(copperfin::security::workspace_agent_tool_workspace_inspect),
        "bin/workspace-tool",
        "."));
    expect(!file_tool.allowed &&
               file_tool.diagnostic_code ==
                   "workspace_agent.target_not_process_tool",
           "RQ-CF-AGENT-010: file tools must not enter the process boundary");
    const auto outside = sandbox.preflight_process_target_request(process_request(
        sandbox_start.session.generation,
        std::string(copperfin::security::workspace_agent_tool_local_run_process),
        tree.outside / "bin" / "local-tool",
        tree.outside / "working"));
    expect(!outside.allowed &&
               outside.diagnostic_code ==
                   "workspace_agent.tool_capability_denied" &&
               outside.canonical_executable_path.empty() &&
               outside.canonical_working_directory.empty(),
           "RQ-CF-AGENT-010: sandbox denial must precede local target reflection");
    const auto stale = sandbox.preflight_process_target_request(process_request(
        sandbox_start.session.generation + 1U,
        std::string(
            copperfin::security::workspace_agent_tool_workspace_run_process),
        "bin/workspace-tool",
        "."));
    expect(!stale.allowed &&
               stale.diagnostic_code == "workspace_agent.tool_stale_session",
           "RQ-CF-AGENT-010: stale generations must fail before target inspection");
    const auto stopped = sandbox.stop(audit_sink());
    expect(stopped.revoked,
           "RQ-CF-AGENT-010: sandbox fixture session must stop");
    expect(!sandbox.preflight_process_target_request(process_request(
                sandbox_start.session.generation,
                std::string(
                    copperfin::security::workspace_agent_tool_workspace_run_process),
                "bin/workspace-tool",
                "."))
                .allowed,
           "RQ-CF-AGENT-010: stopped sessions must retain no process preflight authority");

    WorkspaceAgentSessionController unrestricted(tree.workspace);
    const auto unrestricted_start = unrestricted.start(
        request_for(WorkspaceAgentAccessMode::unrestricted_local), audit_sink());
    const auto local = unrestricted.preflight_process_target_request(
        process_request(
            unrestricted_start.session.generation,
            std::string(
                copperfin::security::workspace_agent_tool_local_run_process),
            tree.outside / "bin" / "local-tool",
            tree.outside / "working"));
    expect(unrestricted_start.activated && local.allowed &&
               local.canonical_executable_path ==
                   std::filesystem::canonical(
                       tree.outside / "bin" / "local-tool"),
           "RQ-CF-AGENT-010: warned unrestricted sessions may identify explicit absolute local process targets");
}

void test_workspace_root_replacement_fails_closed() {
    TempTree tree;
    auto boundary = WorkspaceAgentProcessTargetBoundary::create(tree.workspace);
    expect(boundary.has_value(),
           "RQ-CF-AGENT-010: replacement fixture must configure the original root");
    if (!boundary.has_value()) {
        return;
    }
    const std::filesystem::path moved = tree.root.string() + "-moved";
    std::filesystem::rename(tree.workspace, moved);
    std::error_code symlink_error;
    std::filesystem::create_directory_symlink(
        moved, tree.workspace, symlink_error);
    if (!symlink_error) {
        const auto redirected = boundary->inspect_workspace_process(
            "bin/workspace-tool", ".");
        expect(!redirected.allowed &&
                   redirected.diagnostic_code ==
                       "workspace_agent.process_workspace_root_identity_changed",
               "RQ-CF-AGENT-010: symlink-back replacement of the configured root must fail closed");
        std::filesystem::remove(tree.workspace);
    }
    std::filesystem::create_directories(tree.workspace / "bin");
    TempTree::write_executable(tree.workspace / "bin" / "workspace-tool");
    const auto replaced = boundary->inspect_workspace_process(
        "bin/workspace-tool", ".");
    expect(!replaced.allowed &&
               replaced.diagnostic_code ==
                   "workspace_agent.process_workspace_root_identity_changed",
           "RQ-CF-AGENT-010: replacement of the configured workspace identity must fail closed");
}

}  // namespace

int main() {
    test_boundary_requires_explicit_direct_process_targets();
    test_boundary_rejects_process_target_indirection_and_aliasing();
    test_session_bound_process_target_preflight();
    test_workspace_root_replacement_fails_closed();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
