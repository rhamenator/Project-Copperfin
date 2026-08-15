// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/workspace_agent_session.h"
#include "copperfin/security/workspace_agent_target_containment.h"
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
using copperfin::security::WorkspaceAgentFileTargetBoundary;
using copperfin::security::WorkspaceAgentFileTargetPreflightRequest;
using copperfin::security::WorkspaceAgentSessionAuditCommitResult;
using copperfin::security::WorkspaceAgentSessionAuditEvent;
using copperfin::security::WorkspaceAgentSessionAuditSink;
using copperfin::security::WorkspaceAgentSessionController;

int failures = 0;

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
            ("copperfin-agent-target-" + std::to_string(suffix));
        workspace = root / "workspace";
        outside = root / "outside";
        std::filesystem::create_directories(workspace / "nested");
        std::filesystem::create_directories(outside);
        write(workspace / "inside.prg", "? 'inside'\n");
        write(workspace / "nested" / "child.prg", "? 'child'\n");
        write(outside / "outside.prg", "? 'outside'\n");
    }

    ~TempTree() {
        std::error_code ignored;
        std::filesystem::remove_all(root, ignored);
        std::filesystem::remove_all(root.string() + "-moved", ignored);
    }

    static void write(const std::filesystem::path& path, const std::string& text) {
        std::ofstream stream(path, std::ios::binary | std::ios::trunc);
        stream << text;
    }

    std::filesystem::path root;
    std::filesystem::path workspace;
    std::filesystem::path outside;
};

WorkspaceAgentSessionAuditCommitResult commit_audit(
    const WorkspaceAgentSessionAuditEvent&,
    void*) {
    return {.ok = true, .receipt = "target-containment-test-receipt"};
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

WorkspaceAgentFileTargetPreflightRequest target_request(
    std::uint64_t generation,
    std::string tool_id,
    std::filesystem::path target) {
    return {
        .session_generation = generation,
        .tool_id = std::move(tool_id),
        .target_path = std::move(target)};
}

void test_boundary_rejects_aliases_and_indirection() {
    TempTree tree;
    expect(!WorkspaceAgentFileTargetBoundary::create("relative/workspace").has_value(),
           "RQ-CF-AGENT-009: trusted workspace configuration must require an absolute root");
    expect(!WorkspaceAgentFileTargetBoundary::create(tree.workspace / "inside.prg").has_value(),
           "RQ-CF-AGENT-009: a regular file cannot become the workspace root");

    auto boundary = WorkspaceAgentFileTargetBoundary::create(tree.workspace);
    expect(boundary.has_value(),
           "RQ-CF-AGENT-009: a direct absolute workspace directory must configure the boundary");
    if (!boundary.has_value()) {
        return;
    }

    const auto inside = boundary->inspect_workspace_file("nested/child.prg");
    expect(inside.allowed && inside.canonical_path ==
               std::filesystem::canonical(tree.workspace / "nested" / "child.prg") &&
               inside.identity.link_count == 1U,
           "RQ-CF-AGENT-009: a direct existing regular workspace file must resolve to exact identity");
    for (const std::filesystem::path& invalid : {
             std::filesystem::path{},
             std::filesystem::path{"."},
             std::filesystem::path{"../outside/outside.prg"},
             std::filesystem::path{"nested/../inside.prg"},
             tree.workspace / "inside.prg"}) {
        const auto denied = boundary->inspect_workspace_file(invalid);
        expect(!denied.allowed && denied.canonical_path.empty(),
               "RQ-CF-AGENT-009: non-strict workspace target spellings must fail without reflection");
    }
    const std::filesystem::path embedded_nul{
        std::string{"inside.prg\0suffix", 17U}};
    expect(!boundary->inspect_workspace_file(embedded_nul).allowed,
           "RQ-CF-AGENT-009: embedded-NUL workspace targets must fail before filesystem lookup");
    expect(!boundary->inspect_workspace_file("missing.prg").allowed,
           "RQ-CF-AGENT-009: a missing target must not claim existing-file containment");
    expect(!boundary->inspect_workspace_file("nested").allowed,
           "RQ-CF-AGENT-009: a directory must not claim regular-file containment");

    std::error_code link_error;
    std::filesystem::create_symlink(
        tree.outside / "outside.prg", tree.workspace / "redirect.prg", link_error);
    if (!link_error) {
        const auto redirected = boundary->inspect_workspace_file("redirect.prg");
        expect(!redirected.allowed &&
                   redirected.diagnostic_code ==
                       "workspace_agent.target_indirect_component",
               "RQ-CF-AGENT-009: a symlink target must fail closed");
    }

    link_error.clear();
    std::filesystem::create_hard_link(
        tree.workspace / "inside.prg", tree.workspace / "alias.prg", link_error);
    if (!link_error) {
        const auto multiply_linked = boundary->inspect_workspace_file("inside.prg");
        expect(!multiply_linked.allowed &&
                   multiply_linked.diagnostic_code ==
                       "workspace_agent.target_multiply_linked",
               "RQ-CF-AGENT-009: multiply linked workspace files must fail closed");
    }

    const auto local = boundary->inspect_local_file(tree.outside / "outside.prg");
    expect(local.allowed && !local.canonical_path.empty(),
           "RQ-CF-AGENT-009: unrestricted-local inspection may identify a direct absolute file outside the workspace");
    expect(!boundary->inspect_local_file("outside/outside.prg").allowed,
           "RQ-CF-AGENT-009: unrestricted-local targets must still use unambiguous absolute paths");
}

void test_session_bound_file_target_preflight() {
    TempTree tree;
    WorkspaceAgentSessionController inactive(tree.workspace);
    const auto inactive_result = inactive.preflight_file_target_request(
        target_request(
            1U,
            std::string(copperfin::security::workspace_agent_tool_workspace_inspect),
            "inside.prg"));
    expect(!inactive_result.allowed &&
               inactive_result.diagnostic_code ==
                   "workspace_agent.session_not_active" &&
               inactive_result.canonical_path.empty(),
           "RQ-CF-AGENT-009: inactive denial must precede target lookup and reveal no path");

    WorkspaceAgentSessionController unconfigured;
    const auto unconfigured_start = unconfigured.start(
        request_for(WorkspaceAgentAccessMode::workspace_sandbox), audit_sink());
    const auto unconfigured_target = unconfigured.preflight_file_target_request(
        target_request(
            unconfigured_start.session.generation,
            std::string(copperfin::security::workspace_agent_tool_workspace_inspect),
            "inside.prg"));
    expect(!unconfigured_target.allowed &&
               unconfigured_target.diagnostic_code ==
                   "workspace_agent.target_workspace_root_not_configured",
           "RQ-CF-AGENT-009: a valid session without a product-owned workspace root must fail closed");

    WorkspaceAgentSessionController sandbox(tree.workspace);
    const auto sandbox_start = sandbox.start(
        request_for(WorkspaceAgentAccessMode::workspace_sandbox), audit_sink());
    expect(sandbox_start.activated,
           "RQ-CF-AGENT-009: sandbox fixture session must activate");
    auto invalid_schema_request = target_request(
        sandbox_start.session.generation,
        std::string(copperfin::security::workspace_agent_tool_workspace_inspect),
        "inside.prg");
    invalid_schema_request.schema_version = 2U;
    const auto invalid_schema = sandbox.preflight_file_target_request(
        invalid_schema_request);
    expect(!invalid_schema.allowed &&
               invalid_schema.diagnostic_code ==
                   "workspace_agent.tool_invalid_schema" &&
               invalid_schema.canonical_path.empty(),
           "RQ-CF-AGENT-009: unknown request schemas must fail before target lookup");
    const auto unknown_tool = sandbox.preflight_file_target_request(target_request(
        sandbox_start.session.generation,
        "provider.inspect.v1",
        "inside.prg"));
    expect(!unknown_tool.allowed &&
               unknown_tool.diagnostic_code ==
                   "workspace_agent.tool_not_registered" &&
               unknown_tool.canonical_path.empty(),
           "RQ-CF-AGENT-009: provider-defined tools must fail before target lookup");
    const auto inspect = sandbox.preflight_file_target_request(target_request(
        sandbox_start.session.generation,
        std::string(copperfin::security::workspace_agent_tool_workspace_inspect),
        "inside.prg"));
    expect(inspect.allowed && !inspect.canonical_path.empty() &&
               inspect.diagnostic_code == "workspace_agent.target_request_allowed",
           "RQ-CF-AGENT-009: an admitted workspace file tool must bind the exact session and physical target");
    const auto edit = sandbox.preflight_file_target_request(target_request(
        sandbox_start.session.generation,
        std::string(copperfin::security::workspace_agent_tool_workspace_apply_edit),
        "nested/child.prg"));
    expect(edit.allowed,
           "RQ-CF-AGENT-009: the registered existing-file edit class must share workspace containment");
    const auto outside = sandbox.preflight_file_target_request(target_request(
        sandbox_start.session.generation,
        std::string(copperfin::security::workspace_agent_tool_local_inspect),
        tree.outside / "outside.prg"));
    expect(!outside.allowed &&
               outside.diagnostic_code ==
                   "workspace_agent.tool_capability_denied" &&
               outside.canonical_path.empty(),
           "RQ-CF-AGENT-009: sandbox sessions must deny local targets before filesystem reflection");
    const auto process = sandbox.preflight_file_target_request(target_request(
        sandbox_start.session.generation,
        std::string(copperfin::security::workspace_agent_tool_workspace_run_process),
        "inside.prg"));
    expect(!process.allowed &&
               process.diagnostic_code == "workspace_agent.target_not_file_tool",
           "RQ-CF-AGENT-009: process tools must not pass through the file-target boundary");
    const auto stale = sandbox.preflight_file_target_request(target_request(
        sandbox_start.session.generation + 1U,
        std::string(copperfin::security::workspace_agent_tool_workspace_inspect),
        "inside.prg"));
    expect(!stale.allowed &&
               stale.diagnostic_code == "workspace_agent.tool_stale_session",
           "RQ-CF-AGENT-009: stale generations must fail before target lookup");
    const auto stopped = sandbox.stop(audit_sink());
    expect(stopped.revoked,
           "RQ-CF-AGENT-009: sandbox fixture session must stop");
    expect(!sandbox.preflight_file_target_request(target_request(
                sandbox_start.session.generation,
                std::string(copperfin::security::workspace_agent_tool_workspace_inspect),
                "inside.prg"))
                .allowed,
           "RQ-CF-AGENT-009: stopped sessions must not retain target authority");

    WorkspaceAgentSessionController unrestricted(tree.workspace);
    const auto unrestricted_start = unrestricted.start(
        request_for(WorkspaceAgentAccessMode::unrestricted_local), audit_sink());
    const auto local = unrestricted.preflight_file_target_request(target_request(
        unrestricted_start.session.generation,
        std::string(copperfin::security::workspace_agent_tool_local_inspect),
        tree.outside / "outside.prg"));
    expect(unrestricted_start.activated && local.allowed &&
               local.canonical_path ==
                   std::filesystem::canonical(tree.outside / "outside.prg"),
           "RQ-CF-AGENT-009: warned unrestricted sessions may identify a direct absolute local file");
    const auto local_edit = unrestricted.preflight_file_target_request(target_request(
        unrestricted_start.session.generation,
        std::string(copperfin::security::workspace_agent_tool_local_apply_edit),
        tree.outside / "outside.prg"));
    expect(local_edit.allowed,
           "RQ-CF-AGENT-009: the unrestricted existing-file edit class must share local identity checks");
}

void test_workspace_root_replacement_fails_closed() {
    TempTree tree;
    auto boundary = WorkspaceAgentFileTargetBoundary::create(tree.workspace);
    expect(boundary.has_value(),
           "RQ-CF-AGENT-009: replacement fixture must configure the original root");
    if (!boundary.has_value()) {
        return;
    }
    const std::filesystem::path moved = tree.root.string() + "-moved";
    std::filesystem::rename(tree.workspace, moved);
    std::filesystem::create_directories(tree.workspace);
    TempTree::write(tree.workspace / "inside.prg", "replacement\n");
    const auto replaced = boundary->inspect_workspace_file("inside.prg");
    expect(!replaced.allowed &&
               replaced.diagnostic_code ==
                   "workspace_agent.target_root_identity_changed",
           "RQ-CF-AGENT-009: replacement of the configured workspace identity must fail closed");
}

}  // namespace

int main() {
    test_boundary_rejects_aliases_and_indirection();
    test_session_bound_file_target_preflight();
    test_workspace_root_replacement_fails_closed();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
