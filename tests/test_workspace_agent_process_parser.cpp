// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/physical_path_containment.h"
#include "copperfin/security/workspace_agent_process_parser.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

using copperfin::security::WorkspaceAgentProcessArgumentParserContract;
using copperfin::security::WorkspaceAgentProcessParserBoundary;
using copperfin::security::WorkspaceAgentProcessParserConfiguration;
using copperfin::security::WorkspaceAgentWindowsProcessParserBinding;
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
        return {
            .windows_bindings = {{
                .trusted_absolute_executable = root / "bin" / "trusted-tool",
                .contract = WorkspaceAgentProcessArgumentParserContract::
                    windows_c_runtime_argv_v1}}};
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

    auto duplicate = tree.configuration();
    duplicate.windows_bindings.push_back(duplicate.windows_bindings.front());
    expect(!WorkspaceAgentProcessParserBoundary::create(duplicate).has_value(),
           "RQ-CF-AGENT-018: duplicate canonical parser bindings must fail closed");

    auto excessive = tree.configuration();
    excessive.windows_bindings.assign(
        copperfin::security::workspace_agent_maximum_windows_process_parser_bindings + 1U,
        excessive.windows_bindings.front());
    expect(!WorkspaceAgentProcessParserBoundary::create(excessive).has_value(),
           "RQ-CF-AGENT-018: parser-binding count overflow must fail before capture");

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

}  // namespace

int main() {
    test_configuration_and_exact_identity_authority();
    test_invalid_configuration_fails_closed();
    if (failures == 0) {
        std::cout << "workspace-agent process parser tests passed\n";
    }
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
