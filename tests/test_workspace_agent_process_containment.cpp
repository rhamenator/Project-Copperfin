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
#include <type_traits>

namespace {

using copperfin::security::WorkspaceAgentAccessMode;
using copperfin::security::WorkspaceAgentActivationRequest;
using copperfin::security::WorkspaceAgentProcessTargetBoundary;
using copperfin::security::WorkspaceAgentProcessTargetInspection;
using copperfin::security::WorkspaceAgentProcessTargetPins;
using copperfin::security::WorkspaceAgentProcessTargetPreflightRequest;
using copperfin::security::WorkspaceAgentSessionAuditCommitResult;
using copperfin::security::WorkspaceAgentSessionAuditEvent;
using copperfin::security::WorkspaceAgentSessionAuditSink;
using copperfin::security::WorkspaceAgentSessionController;

int failures = 0;
std::filesystem::path running_test_executable;

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

template <typename T>
concept HasNativeHandle = requires(T value) {
    value.native_handle;
};

template <typename T>
concept HasPinnedPath = requires(T value) {
    value.canonical_path;
};

static_assert(
    !HasCommandText<WorkspaceAgentProcessTargetPreflightRequest> &&
        !HasArguments<WorkspaceAgentProcessTargetPreflightRequest> &&
        !HasEnvironment<WorkspaceAgentProcessTargetPreflightRequest> &&
        !HasCallerSelectedRoot<WorkspaceAgentProcessTargetPreflightRequest> &&
        !HasSearchPath<WorkspaceAgentProcessTargetPreflightRequest>,
    "RQ-CF-AGENT-010: target preflight must not expose commands, arguments, environment, root, or search-path authority");

static_assert(
    !std::is_copy_constructible_v<WorkspaceAgentProcessTargetPins> &&
        !std::is_copy_assignable_v<WorkspaceAgentProcessTargetPins> &&
        std::is_nothrow_move_constructible_v<WorkspaceAgentProcessTargetPins> &&
        std::is_nothrow_move_assignable_v<WorkspaceAgentProcessTargetPins> &&
        !HasNativeHandle<WorkspaceAgentProcessTargetPins> &&
        !HasPinnedPath<WorkspaceAgentProcessTargetPins>,
    "RQ-CF-AGENT-023: target pins must be move-only and expose no native handle or path");

static_assert(
    !std::is_copy_constructible_v<WorkspaceAgentProcessTargetBoundary> &&
        !std::is_copy_assignable_v<WorkspaceAgentProcessTargetBoundary> &&
        std::is_nothrow_move_constructible_v<
            WorkspaceAgentProcessTargetBoundary> &&
        std::is_nothrow_move_assignable_v<WorkspaceAgentProcessTargetBoundary>,
    "RQ-CF-AGENT-023: moving a target boundary must transfer rather than duplicate its logical authority");

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
#if defined(_WIN32)
        std::error_code copy_error;
        std::filesystem::copy_file(
            running_test_executable,
            path,
            std::filesystem::copy_options::overwrite_existing,
            copy_error);
        if (copy_error) {
            write_plain(path);
        }
#else
        write_plain(path);
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
    expect(!boundary->inspect_local_process(
                R"(//server/share/tool.exe)",
                R"(C:\Windows)")
                .allowed,
           "RQ-CF-AGENT-010: forward-slash UNC executables must fail before lookup");
    expect(!boundary->inspect_local_process(
                R"(C:\Windows\System32\cmd.exe)",
                R"(\\server/share\working)")
                .allowed,
           "RQ-CF-AGENT-010: mixed-separator UNC working directories must fail before lookup");
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
#else
    const auto non_image = boundary->inspect_workspace_process(
        "not-executable", ".");
    expect(!non_image.allowed &&
               non_image.diagnostic_code ==
                   "workspace_agent.process_executable_image_invalid",
           "RQ-CF-AGENT-017: Windows shell/text content must not become a direct process image");
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

void test_boundary_retains_exact_process_target_objects() {
    TempTree tree;
    auto boundary = WorkspaceAgentProcessTargetBoundary::create(tree.workspace);
    auto other_boundary = WorkspaceAgentProcessTargetBoundary::create(tree.workspace);
    expect(boundary.has_value() && other_boundary.has_value(),
           "RQ-CF-AGENT-023: pin fixtures must configure independent trusted boundaries");
    if (!boundary.has_value() || !other_boundary.has_value()) {
        return;
    }

    WorkspaceAgentProcessTargetInspection forged;
    forged.allowed = true;
    forged.canonical_executable_path = tree.workspace / "bin/workspace-tool";
    forged.canonical_working_directory = tree.workspace;
    const auto forged_pin = boundary->pin_process_targets(forged);
    expect(!forged_pin.pinned && !forged_pin.pins.has_value() &&
               forged_pin.diagnostic_code ==
                   "workspace_agent.process_target_pin_authority_unavailable",
           "RQ-CF-AGENT-023: caller-constructed inspection fields must not authorize pins");

    auto cross_boundary_inspection = boundary->inspect_workspace_process(
        "bin/workspace-tool", ".");
    const auto cross_boundary = other_boundary->pin_process_targets(
        cross_boundary_inspection);
    expect(!cross_boundary.pinned &&
               cross_boundary.diagnostic_code ==
                   "workspace_agent.process_target_pin_authority_unavailable",
           "RQ-CF-AGENT-023: an inspection from another logical boundary must fail closed");
    auto correct_boundary = boundary->pin_process_targets(
        cross_boundary_inspection);
    expect(correct_boundary.pinned && correct_boundary.pins.has_value() &&
               correct_boundary.pins->valid(),
           "RQ-CF-AGENT-023: a cross-boundary denial must not consume the issuing boundary's one-attempt authority");
    const auto replay = boundary->pin_process_targets(cross_boundary_inspection);
    expect(!replay.pinned &&
               replay.diagnostic_code ==
                   "workspace_agent.process_target_pin_authority_unavailable",
           "RQ-CF-AGENT-023: an inspection authority must not authorize a second pin bundle");
    correct_boundary.pins.reset();

    auto move_source = WorkspaceAgentProcessTargetBoundary::create(tree.workspace);
    expect(move_source.has_value(),
           "RQ-CF-AGENT-023: move-authority fixture must configure a boundary");
    if (move_source.has_value()) {
        auto move_inspection = move_source->inspect_workspace_process(
            "bin/workspace-tool", ".");
        WorkspaceAgentProcessTargetBoundary move_destination(
            std::move(*move_source));
        const auto moved_from = move_source->pin_process_targets(move_inspection);
        auto moved_to = move_destination.pin_process_targets(move_inspection);
        expect(!moved_from.pinned && moved_to.pinned &&
                   moved_to.pins.has_value() && moved_to.pins->valid(),
               "RQ-CF-AGENT-023: moving a boundary must transfer its exact logical authority and invalidate the source");
    }

    auto edited = boundary->inspect_workspace_process(
        "bin/workspace-tool", ".");
    auto edited_copy = edited;
    edited.canonical_working_directory = tree.workspace / "nested";
    const auto edited_denial = boundary->pin_process_targets(edited);
    const auto edited_replay = boundary->pin_process_targets(edited_copy);
    expect(!edited_denial.pinned && !edited_replay.pinned,
           "RQ-CF-AGENT-023: editing public inspection fields must fail and consume the one-attempt authority");

    auto stale = boundary->inspect_workspace_process(
        "bin/workspace-tool", "nested");
    const auto original_executable = tree.workspace / "bin/workspace-tool";
    const auto moved_executable = tree.workspace / "bin/workspace-tool-old";
    std::filesystem::rename(original_executable, moved_executable);
    TempTree::write_executable(original_executable);
    const auto stale_pin = boundary->pin_process_targets(stale);
    expect(!stale_pin.pinned && !stale_pin.pins.has_value() &&
               stale_pin.diagnostic_code ==
                   "workspace_agent.process_target_pin_identity_changed",
           "RQ-CF-AGENT-023: path replacement after inspection must not pin a different executable");
    std::filesystem::remove(original_executable);
    std::filesystem::rename(moved_executable, original_executable);

    auto retained = boundary->inspect_workspace_process(
        "bin/workspace-tool", "nested");
    auto retained_pin = boundary->pin_process_targets(retained);
    expect(retained_pin.pinned && retained_pin.pins.has_value() &&
               retained_pin.pins->valid(),
           "RQ-CF-AGENT-023: exact root, executable, and working directory objects must be retained opaquely");
    std::error_code rename_error;
    std::filesystem::rename(
        original_executable, moved_executable, rename_error);
#if defined(_WIN32)
    expect(rename_error && retained_pin.pins->valid(),
           "RQ-CF-AGENT-023: Windows pins must deny executable replacement while retained");
    retained_pin.pins.reset();
    rename_error.clear();
    std::filesystem::rename(
        original_executable, moved_executable, rename_error);
    expect(!rename_error,
           "RQ-CF-AGENT-023: releasing Windows pins must release replacement exclusion");
#else
    expect(!rename_error && retained_pin.pins->valid(),
           "RQ-CF-AGENT-023: POSIX pins must retain the exact opened executable object across name replacement");
#endif
}

void test_session_bound_process_target_pin_preflight() {
    TempTree tree;
    WorkspaceAgentSessionController inactive(tree.workspace);
    const auto inactive_pin = inactive.pin_process_target_request(
        process_request(
            1U,
            std::string(
                copperfin::security::workspace_agent_tool_workspace_run_process),
            "bin/workspace-tool",
            "."));
    expect(!inactive_pin.pinned &&
               inactive_pin.diagnostic_code ==
                   "workspace_agent.session_not_active",
           "RQ-CF-AGENT-023: inactive sessions must fail before target pinning");

    WorkspaceAgentSessionController controller(tree.workspace);
    const auto started = controller.start(
        request_for(WorkspaceAgentAccessMode::workspace_sandbox), audit_sink());
    expect(started.activated,
           "RQ-CF-AGENT-023: target-pin fixture session must activate");
    const auto file_tool = controller.pin_process_target_request(
        process_request(
            started.session.generation,
            std::string(copperfin::security::workspace_agent_tool_workspace_inspect),
            "bin/workspace-tool",
            "."));
    expect(!file_tool.pinned &&
               file_tool.diagnostic_code ==
                   "workspace_agent.target_not_process_tool",
           "RQ-CF-AGENT-023: a file tool must not enter target pinning");
    const auto stale = controller.pin_process_target_request(
        process_request(
            started.session.generation + 1U,
            std::string(
                copperfin::security::workspace_agent_tool_workspace_run_process),
            "bin/workspace-tool",
            "."));
    expect(!stale.pinned &&
               stale.diagnostic_code == "workspace_agent.tool_stale_session",
           "RQ-CF-AGENT-023: stale generations must fail before target pinning");

    auto pinned = controller.pin_process_target_request(
        process_request(
            started.session.generation,
            std::string(
                copperfin::security::workspace_agent_tool_workspace_run_process),
            "bin/workspace-tool",
            "nested"));
    expect(pinned.pinned && pinned.pins.has_value() &&
               pinned.pins->valid() &&
               pinned.session_generation == started.session.generation &&
               pinned.diagnostic_code ==
                   "workspace_agent.process_target_pin_request_allowed",
           "RQ-CF-AGENT-023: exact active process authority must admit opaque target pins");
    const auto still_denied =
        controller.revalidate_serialized_process_invocation_for_launch({}, {});
    expect(!still_denied.allowed &&
               still_denied.diagnostic_code ==
                   "workspace_agent.process_launch_revalidation_pinning_unavailable",
           "RQ-CF-AGENT-023: target pins alone must not weaken the launch gate");
    const auto stopped = controller.stop(audit_sink());
    expect(stopped.revoked && pinned.pins->valid(),
           "RQ-CF-AGENT-023: pins are resource retention only and must not substitute for the separate revocation lease");
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
    test_boundary_requires_explicit_direct_process_targets();
    test_boundary_rejects_process_target_indirection_and_aliasing();
    test_session_bound_process_target_preflight();
    test_workspace_root_replacement_fails_closed();
    test_boundary_retains_exact_process_target_objects();
    test_session_bound_process_target_pin_preflight();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
