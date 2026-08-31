// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/physical_path_containment.h"
#include "copperfin/security/sha256.h"
#include "copperfin/security/workspace_agent_process_parser.h"
#include "copperfin/security/workspace_agent_process_parser_test_hooks.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

using copperfin::security::WorkspaceAgentProcessArgumentParserContract;
using copperfin::security::WorkspaceAgentProcessParserDependencyContract;
using copperfin::security::WorkspaceAgentProcessParserBoundary;
using copperfin::security::WorkspaceAgentProcessParserConfiguration;
using copperfin::security::WorkspaceAgentWindowsProcessParserBinding;
using copperfin::security::workspace_agent_maximum_windows_process_parser_image_bytes;
namespace fs = std::filesystem;

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
        root = fs::canonical(fs::temp_directory_path()) /
            ("copperfin-agent-parser-" + std::to_string(
                std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(root / "bin");
        write(root / "bin" / "trusted-tool", "trusted-v1");
        write(root / "bin" / "other-tool", "other-v1");
    }

    ~TempTree() {
        std::error_code ignored;
        fs::remove_all(root, ignored);
    }

    static void write(const fs::path& path, const std::string& bytes) {
        std::ofstream output(path, std::ios::binary | std::ios::trunc);
        output << bytes;
    }

    WorkspaceAgentProcessParserConfiguration configuration() const {
        const auto trusted =
            copperfin::security::inspect_physical_path_containment(
                root / "bin" / "trusted-tool", root / "bin");
        return {
            .windows_bindings = {{
                .trusted_absolute_executable = root / "bin" / "trusted-tool",
                .expected_identity = trusted.identity,
                .expected_sha256 = digest_for(trusted),
                .dependency_contract = WorkspaceAgentProcessParserDependencyContract::
                    self_contained_launch_image_v1,
                .contract = WorkspaceAgentProcessArgumentParserContract::
                    windows_c_runtime_argv_v1}}};
    }

    static std::string digest_for(
        const copperfin::security::PhysicalPathContainmentResult& containment) {
        const auto snapshot =
            copperfin::security::read_physically_contained_file_snapshot(
                containment, containment.canonical_path.parent_path());
        if (!snapshot.ok) {
            return {};
        }
        const auto digest =
            copperfin::security::sha256_hex_for_text(snapshot.bytes);
        return digest.ok ? digest.hex_digest : std::string{};
    }

    fs::path root;
};

void expect_content_free_denial(
    const copperfin::security::WorkspaceAgentProcessParserAuthorization& result,
    std::string_view diagnostic,
    const std::string& message) {
    expect(!result.allowed && result.contract ==
               WorkspaceAgentProcessArgumentParserContract::none &&
               result.diagnostic_code == diagnostic,
           message);
}

void test_configuration_and_exact_identity_authority() {
    TempTree tree;
    auto configuration = tree.configuration();
    const auto boundary = WorkspaceAgentProcessParserBoundary::create(configuration);
    expect(boundary.has_value(),
           "RQ-CF-AGENT-018: one direct singly-linked trusted executable must configure Windows parser authority");
    if (!boundary.has_value()) {
        return;
    }

    const auto trusted = copperfin::security::inspect_physical_path_containment(
        tree.root / "bin" / "trusted-tool", tree.root / "bin");
    const auto allowed = boundary->authorize_windows(
        trusted.canonical_path, trusted.identity);
    expect(allowed.allowed && allowed.contract ==
               WorkspaceAgentProcessArgumentParserContract::
                   windows_c_runtime_argv_v1 &&
               allowed.diagnostic_code ==
                   "workspace_agent.process_argument_parser_allowed",
           "RQ-CF-AGENT-018: exact configured path and complete identity must authorize only the known Windows parser contract");

    const auto other = copperfin::security::inspect_physical_path_containment(
        tree.root / "bin" / "other-tool", tree.root / "bin");
    expect_content_free_denial(
        boundary->authorize_windows(other.canonical_path, other.identity),
        "workspace_agent.process_argument_parser_not_trusted",
        "RQ-CF-AGENT-018: an unconfigured executable must fail without path or identity reflection");
    expect_content_free_denial(
        boundary->authorize_windows(trusted.canonical_path, other.identity),
        "workspace_agent.process_argument_parser_not_trusted",
        "RQ-CF-AGENT-018: a mismatched supplied identity must not borrow parser authority");

    std::error_code time_error;
    const auto original_write_time =
        fs::last_write_time(trusted.canonical_path, time_error);
    if (!time_error) {
        TempTree::write(tree.root / "bin" / "trusted-tool", "hostile-v1");
        fs::last_write_time(
            trusted.canonical_path, original_write_time, time_error);
        const auto metadata_preserved =
            copperfin::security::inspect_physical_path_containment(
                trusted.canonical_path, trusted.canonical_path.parent_path());
        if (!time_error && metadata_preserved.allowed &&
            metadata_preserved.identity == trusted.identity) {
            expect_content_free_denial(
                boundary->authorize_windows(
                    metadata_preserved.canonical_path,
                    metadata_preserved.identity),
                "workspace_agent.process_argument_parser_contents_changed",
                "RQ-CF-AGENT-018: same-size overwritten bytes with restored filesystem metadata must revoke parser authority");
        }
    }

    TempTree::write(tree.root / "bin" / "trusted-tool", "trusted-v2-longer");
    expect_content_free_denial(
        boundary->authorize_windows(trusted.canonical_path, trusted.identity),
        "workspace_agent.process_argument_parser_identity_changed",
        "RQ-CF-AGENT-018: changed executable identity must revoke captured parser authority");
}

void test_invalid_configuration_fails_closed() {
    TempTree tree;
    auto invalid_schema = tree.configuration();
    invalid_schema.schema_version = 2U;
    expect(!WorkspaceAgentProcessParserBoundary::create(invalid_schema).has_value(),
           "RQ-CF-AGENT-018: unknown trusted configuration schemas must fail closed");
    expect(!WorkspaceAgentProcessParserBoundary::create({}).has_value(),
           "RQ-CF-AGENT-018: empty Windows parser authority must not manufacture trust");

    auto relative = tree.configuration();
    relative.windows_bindings.front().trusted_absolute_executable =
        "bin/trusted-tool";
    expect(!WorkspaceAgentProcessParserBoundary::create(relative).has_value(),
           "RQ-CF-AGENT-018: relative executable authority must fail closed");

    auto directory = tree.configuration();
    directory.windows_bindings.front().trusted_absolute_executable =
        tree.root / "bin";
    expect(!WorkspaceAgentProcessParserBoundary::create(directory).has_value(),
           "RQ-CF-AGENT-018: directories must not receive argument-parser authority");

    auto absent_identity = tree.configuration();
    absent_identity.windows_bindings.front().expected_identity = {};
    expect(!WorkspaceAgentProcessParserBoundary::create(absent_identity).has_value(),
           "RQ-CF-AGENT-018: trusted configuration must supply the expected executable identity");

    auto wrong_expected_identity = tree.configuration();
    wrong_expected_identity.windows_bindings.front().expected_identity =
        copperfin::security::inspect_physical_path_containment(
            tree.root / "bin" / "other-tool", tree.root / "bin").identity;
    expect(!WorkspaceAgentProcessParserBoundary::create(
                wrong_expected_identity).has_value(),
           "RQ-CF-AGENT-018: a pre-positioned file must not manufacture the trusted host's expected identity");

    auto absent_digest = tree.configuration();
    absent_digest.windows_bindings.front().expected_sha256.clear();
    expect(!WorkspaceAgentProcessParserBoundary::create(absent_digest).has_value(),
           "RQ-CF-AGENT-018: trusted configuration must supply a canonical expected executable digest");

    auto malformed_digest = tree.configuration();
    malformed_digest.windows_bindings.front().expected_sha256 =
        std::string(64U, 'A');
    expect(!WorkspaceAgentProcessParserBoundary::create(malformed_digest).has_value(),
           "RQ-CF-AGENT-018: non-lowercase or malformed expected digests must fail closed");

    auto wrong_digest = tree.configuration();
    wrong_digest.windows_bindings.front().expected_sha256 =
        std::string(64U, '0');
    expect(!WorkspaceAgentProcessParserBoundary::create(wrong_digest).has_value(),
           "RQ-CF-AGENT-018: physical metadata must not authorize executable bytes that differ from trusted content evidence");

    std::error_code symlink_error;
    fs::create_symlink(
        tree.root / "bin" / "trusted-tool",
        tree.root / "bin" / "trusted-symlink",
        symlink_error);
    if (!symlink_error) {
        auto indirect = tree.configuration();
        indirect.windows_bindings.front().trusted_absolute_executable =
            tree.root / "bin" / "trusted-symlink";
        expect(!WorkspaceAgentProcessParserBoundary::create(indirect).has_value(),
               "RQ-CF-AGENT-018: indirect executable paths must not receive parser authority");
    }

    auto unknown_contract = tree.configuration();
    unknown_contract.windows_bindings.front().contract =
        static_cast<WorkspaceAgentProcessArgumentParserContract>(99U);
    expect(!WorkspaceAgentProcessParserBoundary::create(unknown_contract).has_value(),
           "RQ-CF-AGENT-018: unknown parser contracts must fail closed");

    auto missing_dependency_contract = tree.configuration();
    missing_dependency_contract.windows_bindings.front().dependency_contract =
        WorkspaceAgentProcessParserDependencyContract::none;
    expect(!WorkspaceAgentProcessParserBoundary::create(
                missing_dependency_contract).has_value(),
           "RQ-CF-AGENT-018: parser authority must require exact-digest trusted product evidence of a self-contained launch image");

    auto unknown_dependency_contract = tree.configuration();
    unknown_dependency_contract.windows_bindings.front().dependency_contract =
        static_cast<WorkspaceAgentProcessParserDependencyContract>(99U);
    expect(!WorkspaceAgentProcessParserBoundary::create(
                unknown_dependency_contract).has_value(),
           "RQ-CF-AGENT-018: unknown parser dependency contracts must fail closed");

    auto duplicate = tree.configuration();
    // Keep the fill value independent of vector storage: push_back may relocate
    // windows_bindings before it copies its argument.
    const auto duplicate_binding = duplicate.windows_bindings.front();
    duplicate.windows_bindings.push_back(duplicate_binding);
    expect(!WorkspaceAgentProcessParserBoundary::create(duplicate).has_value(),
           "RQ-CF-AGENT-018: duplicate canonical parser bindings must fail closed");

    auto excessive = tree.configuration();
    // assign may discard and reallocate the vector's current storage before it
    // reads the fill value, so an element reference is not a valid argument.
    const auto excessive_binding = excessive.windows_bindings.front();
    excessive.windows_bindings.assign(
        copperfin::security::workspace_agent_maximum_windows_process_parser_bindings + 1U,
        excessive_binding);
    expect(!WorkspaceAgentProcessParserBoundary::create(excessive).has_value(),
           "RQ-CF-AGENT-018: parser-binding count overflow must fail before capture");

    const auto oversized_path = tree.root / "bin" / "oversized-tool";
    TempTree::write(oversized_path, "");
    std::error_code resize_error;
    fs::resize_file(
        oversized_path,
        workspace_agent_maximum_windows_process_parser_image_bytes + 1U,
        resize_error);
    if (!resize_error) {
        const auto oversized =
            copperfin::security::inspect_physical_path_containment(
                oversized_path, tree.root / "bin");
        auto oversized_configuration = tree.configuration();
        oversized_configuration.windows_bindings.front()
            .trusted_absolute_executable = oversized_path;
        oversized_configuration.windows_bindings.front().expected_identity =
            oversized.identity;
        expect(!WorkspaceAgentProcessParserBoundary::create(
                    oversized_configuration).has_value(),
               "RQ-CF-AGENT-018: oversized parser images must fail before snapshot allocation");
    }

    std::error_code link_error;
    fs::create_hard_link(
        tree.root / "bin" / "trusted-tool",
        tree.root / "bin" / "trusted-alias",
        link_error);
    if (!link_error) {
        expect(!WorkspaceAgentProcessParserBoundary::create(
                    tree.configuration()).has_value(),
               "RQ-CF-AGENT-018: multiply-linked executable identities must not receive parser authority");
    }
}

// #5421 round-2 review: capture_binding() and authorize_windows() each
// checked link_count against the pre-read, walk-time identity, but not
// against the fresh post-read identity read_physically_contained_file_snapshot_from_handle()
// returns -- since that function's own freshness check deliberately
// excludes link_count (issue #5420), a hard link added between the
// pre-read check and the read completing went undetected. Fixed by
// checking it via a single shared helper (read_trusted_executable_snapshot())
// that both functions call. This test proves it: the trusted executable
// has link_count 1 when capture_binding() performs its pre-read check, and
// a test-only hook creates a hard link to it immediately before the read
// itself -- a window no other test in this file reaches, since the
// pre-existing hard-link test above links before the boundary is ever
// created, exercising only the pre-read check.
fs::path g_process_parser_hard_link_hook_target;
fs::path g_process_parser_hard_link_hook_alias;

void process_parser_create_hard_link_test_hook() {
    std::error_code ignored;
    fs::create_hard_link(
        g_process_parser_hard_link_hook_target,
        g_process_parser_hard_link_hook_alias,
        ignored);
}

void test_capture_binding_rejects_hard_link_added_during_read() {
    TempTree tree;
    const auto configuration = tree.configuration();

    g_process_parser_hard_link_hook_target = tree.root / "bin" / "trusted-tool";
    g_process_parser_hard_link_hook_alias =
        tree.root / "bin" / "trusted-tool-mid-read-alias";
    copperfin::security::set_workspace_agent_process_parser_pre_read_test_hook_for_testing(
        &process_parser_create_hard_link_test_hook);

    const auto boundary = WorkspaceAgentProcessParserBoundary::create(configuration);

    // Defensive: clear the hook even if it never fired (e.g. an earlier
    // check in capture_binding() failed closed before reaching the read),
    // so a later test is never affected by a leftover hook.
    copperfin::security::set_workspace_agent_process_parser_pre_read_test_hook_for_testing(
        nullptr);

    std::error_code link_check_error;
    const bool link_created = fs::exists(
        g_process_parser_hard_link_hook_alias, link_check_error);
    expect(link_created,
           "the test hook should have created the mid-read hard link "
           "(this test's environment must support hard links to be "
           "meaningful)");
    if (link_created) {
        expect(!boundary.has_value(),
               "RQ-CF-AGENT-018: a hard link added between the pre-read "
               "check and the read must still deny parser authority -- "
               "issue #5421 round-2 regression test");
    }

    std::error_code cleanup_error;
    fs::remove(g_process_parser_hard_link_hook_alias, cleanup_error);
}

}  // namespace

int main() {
    test_configuration_and_exact_identity_authority();
    test_invalid_configuration_fails_closed();
    test_capture_binding_rejects_hard_link_added_during_read();
    if (failures == 0) {
        std::cout << "workspace-agent process parser tests passed\n";
    }
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
