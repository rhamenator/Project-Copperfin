// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/path.h"
#include "copperfin/security/workspace_agent_environment.h"
#include "copperfin/security/workspace_agent_session.h"
#include "copperfin/security/workspace_agent_tool_registry.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace {

using copperfin::security::WorkspaceAgentAccessMode;
using copperfin::security::WorkspaceAgentActivationRequest;
using copperfin::security::WorkspaceAgentEnvironmentEntry;
using copperfin::security::WorkspaceAgentIsolatedEnvironmentBoundary;
using copperfin::security::WorkspaceAgentIsolatedEnvironmentConfiguration;
using copperfin::security::WorkspaceAgentProcessEnvironmentPreflightResult;
using copperfin::security::WorkspaceAgentProcessEnvironmentPolicy;
using copperfin::security::WorkspaceAgentProcessInvocationPreflightRequest;
using copperfin::security::WorkspaceAgentSessionAuditCommitResult;
using copperfin::security::WorkspaceAgentSessionAuditEvent;
using copperfin::security::WorkspaceAgentSessionAuditSink;
using copperfin::security::WorkspaceAgentSessionController;

template <typename T>
concept HasEnvironmentInput = requires(T value) {
    value.environment;
};

template <typename T>
concept HasEnvironmentEntriesInput = requires(T value) {
    value.environment_entries;
};

static_assert(
    !HasEnvironmentInput<WorkspaceAgentProcessInvocationPreflightRequest> &&
        !HasEnvironmentEntriesInput<WorkspaceAgentProcessInvocationPreflightRequest>,
    "RQ-CF-AGENT-012: tool requests must not supply environment names or values");

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
            ("copperfin-agent-environment-" + std::to_string(suffix));
        workspace = root / "workspace";
        session_storage = root / "sessions";
        approved_one = root / "approved-one";
        approved_two = root / "approved-two";
        windows_system_root = root / "windows-root";
        outside = root / "outside";
        std::filesystem::create_directories(workspace / "bin");
        std::filesystem::create_directories(workspace / "working");
        std::filesystem::create_directories(session_storage);
        std::filesystem::create_directories(approved_one);
        std::filesystem::create_directories(approved_two);
        std::filesystem::create_directories(windows_system_root);
        std::filesystem::create_directories(outside);
        create_session_layout(1U);
        write_executable(workspace / "bin" / "workspace-tool");
    }

    ~TempTree() {
        std::error_code ignored;
        std::filesystem::remove_all(root, ignored);
    }

    void create_session_layout(std::uint64_t generation) const {
        const auto session =
            session_storage / ("session-" + std::to_string(generation));
        for (const std::string_view leaf :
             {"home", "temp", "config", "cache", "data"}) {
            std::filesystem::create_directories(session / leaf);
        }
    }

    static void write_executable(const std::filesystem::path& path) {
        std::ofstream stream(path, std::ios::binary | std::ios::trunc);
        stream << "must not execute\n";
        stream.close();
        std::error_code error;
        std::filesystem::permissions(
            path,
            std::filesystem::perms::owner_exec |
                std::filesystem::perms::group_exec |
                std::filesystem::perms::others_exec,
            std::filesystem::perm_options::add,
            error);
    }

    WorkspaceAgentIsolatedEnvironmentConfiguration configuration() const {
        WorkspaceAgentIsolatedEnvironmentConfiguration result{
            .trusted_session_storage_root = session_storage,
            .trusted_executable_directories = {approved_one, approved_two},
            .trusted_windows_system_root = {}};
#if defined(_WIN32)
        result.trusted_windows_system_root = windows_system_root;
#endif
        return result;
    }

    std::filesystem::path root;
    std::filesystem::path workspace;
    std::filesystem::path session_storage;
    std::filesystem::path approved_one;
    std::filesystem::path approved_two;
    std::filesystem::path windows_system_root;
    std::filesystem::path outside;
};

WorkspaceAgentSessionAuditCommitResult commit_audit(
    const WorkspaceAgentSessionAuditEvent&,
    void*) {
    return {.ok = true, .receipt = "isolated-environment-test-receipt"};
}

WorkspaceAgentSessionAuditSink audit_sink() {
    return {.commit = commit_audit};
}

WorkspaceAgentActivationRequest activation_request() {
    return {
        .requested_mode = WorkspaceAgentAccessMode::workspace_sandbox,
        .feature_enabled = true,
        .permission_granted = true,
        .trusted_product_ui = true,
        .audit_sink_available = true,
        .warning_presented = false,
        .warning_id = {},
        .user_confirmed = false};
}

WorkspaceAgentProcessInvocationPreflightRequest invocation_request(
    std::uint64_t generation) {
    return {
        .session_generation = generation,
        .tool_id = std::string(
            copperfin::security::workspace_agent_tool_workspace_run_process),
        .executable_path = "bin/workspace-tool",
        .working_directory = "working",
        .arguments = {"literal && not-shell", "value with spaces"}};
}

const std::string* find_entry(
    const std::vector<WorkspaceAgentEnvironmentEntry>& entries,
    std::string_view name) {
    const auto found = std::find_if(
        entries.begin(), entries.end(),
        [name](const WorkspaceAgentEnvironmentEntry& entry) {
            return entry.name == name;
        });
    return found == entries.end() ? nullptr : &found->value;
}

void expect_content_free_denial(
    const WorkspaceAgentProcessEnvironmentPreflightResult& result,
    std::string_view diagnostic,
    const std::string& message) {
    expect(!result.allowed && result.diagnostic_code == diagnostic &&
               result.session_generation == 0U && result.tool_id.empty() &&
               result.canonical_executable_path.empty() &&
               result.canonical_working_directory.empty() &&
               result.arguments.empty() && result.environment_entries.empty(),
           message);
}

void test_fixed_non_inheriting_environment() {
    TempTree tree;
    WorkspaceAgentSessionController controller(tree.workspace, tree.configuration());
    const auto start = controller.start(activation_request(), audit_sink());
    expect(start.activated && start.session.generation == 1U,
           "RQ-CF-AGENT-012: fixture session must activate at generation one");

    const auto result = controller.preflight_process_environment_request(
        invocation_request(start.session.generation));
    expect(result.allowed &&
               result.diagnostic_code ==
                   "workspace_agent.process_environment_request_allowed" &&
               result.environment_policy ==
                   WorkspaceAgentProcessEnvironmentPolicy::isolated_session_v1 &&
               !copperfin::security::
                   workspace_agent_process_environment_inherits_parent(
                       result.environment_policy) &&
               result.environment_platform ==
                   copperfin::security::
                       workspace_agent_process_environment_host_platform(),
           "RQ-CF-AGENT-012: exact invocation must produce only the host isolated-session profile");
    expect(std::is_sorted(
               result.environment_entries.begin(),
               result.environment_entries.end(),
               [](const auto& left, const auto& right) {
#if defined(_WIN32)
                   std::string lhs = left.name;
                   std::string rhs = right.name;
                   std::transform(lhs.begin(), lhs.end(), lhs.begin(), [](unsigned char ch) {
                       return static_cast<char>(std::tolower(ch));
                   });
                   std::transform(rhs.begin(), rhs.end(), rhs.begin(), [](unsigned char ch) {
                       return static_cast<char>(std::tolower(ch));
                   });
                   return lhs < rhs;
#else
                   return left.name < right.name;
#endif
               }),
           "RQ-CF-AGENT-012: entries must have deterministic platform ordering");

    const auto session = std::filesystem::canonical(
        tree.session_storage / "session-1");
    const std::string expected_home =
        copperfin::platform::path_to_utf8_string(session / "home");
    const std::string expected_temp =
        copperfin::platform::path_to_utf8_string(session / "temp");
    const std::string* home = find_entry(result.environment_entries, "HOME");
    expect(home != nullptr && *home == expected_home,
           "RQ-CF-AGENT-012: HOME must be generation-specific and session-owned");
#if defined(_WIN32)
    expect(result.environment_entries.size() == 10U &&
               find_entry(result.environment_entries, "TEMP") != nullptr &&
               *find_entry(result.environment_entries, "TEMP") == expected_temp &&
               find_entry(result.environment_entries, "USERPROFILE") != nullptr &&
               find_entry(result.environment_entries, "SystemRoot") != nullptr,
           "RQ-CF-AGENT-012: Windows profile, temporary, and system roots must be explicit");
#else
    expect(result.environment_entries.size() == 9U &&
               find_entry(result.environment_entries, "TMPDIR") != nullptr &&
               *find_entry(result.environment_entries, "TMPDIR") == expected_temp &&
               find_entry(result.environment_entries, "XDG_CONFIG_HOME") != nullptr &&
               find_entry(result.environment_entries, "XDG_CACHE_HOME") != nullptr &&
               find_entry(result.environment_entries, "XDG_DATA_HOME") != nullptr &&
               find_entry(result.environment_entries, "LANG") != nullptr &&
               *find_entry(result.environment_entries, "LANG") == "C" &&
               find_entry(result.environment_entries, "LC_ALL") != nullptr &&
               *find_entry(result.environment_entries, "LC_ALL") == "C",
           "RQ-CF-AGENT-012: POSIX profile, temporary, XDG, and locale roots must be explicit");
#endif
    expect(find_entry(result.environment_entries, "AWS_SECRET_ACCESS_KEY") == nullptr &&
               find_entry(result.environment_entries, "GITHUB_TOKEN") == nullptr &&
               find_entry(result.environment_entries, "SSH_AUTH_SOCK") == nullptr &&
               find_entry(result.environment_entries, "COPPERFIN_LICENSE_PATH") == nullptr,
           "RQ-CF-AGENT-012: ambient credential and product-secret keys must remain absent");

    const std::string expected_path =
        copperfin::platform::path_to_utf8_string(
            std::filesystem::canonical(tree.approved_one)) +
#if defined(_WIN32)
        ";" +
#else
        ":" +
#endif
        copperfin::platform::path_to_utf8_string(
            std::filesystem::canonical(tree.approved_two));
    const std::string* path = find_entry(result.environment_entries, "PATH");
    expect(path != nullptr && *path == expected_path,
           "RQ-CF-AGENT-012: PATH must contain only ordered product-approved directories");
}

void test_configuration_and_layout_fail_closed() {
    TempTree tree;
    const auto valid_boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(tree.configuration());
    expect(valid_boundary.has_value(),
           "RQ-CF-AGENT-012: valid trusted configuration must create a boundary");
    if (valid_boundary.has_value()) {
        const auto zero_generation = valid_boundary->construct(
            0U, WorkspaceAgentProcessEnvironmentPolicy::isolated_session_v1);
        expect(!zero_generation.allowed && zero_generation.entries.empty() &&
                   zero_generation.diagnostic_code ==
                       "workspace_agent.environment_invalid_session_generation",
               "RQ-CF-AGENT-012: zero generation must fail without environment content");
        const auto invalid_policy = valid_boundary->construct(
            1U, static_cast<WorkspaceAgentProcessEnvironmentPolicy>(99U));
        expect(!invalid_policy.allowed && invalid_policy.entries.empty() &&
                   invalid_policy.diagnostic_code ==
                       "workspace_agent.environment_invalid_policy",
               "RQ-CF-AGENT-012: unknown environment policies must fail without content");
    }

    auto invalid_schema = tree.configuration();
    invalid_schema.schema_version = 2U;
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(invalid_schema).has_value(),
           "RQ-CF-AGENT-012: unknown trusted-configuration schemas must fail");

    auto no_path = tree.configuration();
    no_path.trusted_executable_directories.clear();
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(no_path).has_value(),
           "RQ-CF-AGENT-012: an empty approved executable-directory set must fail");

    auto duplicate_path = tree.configuration();
    duplicate_path.trusted_executable_directories.push_back(tree.approved_one);
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(duplicate_path).has_value(),
           "RQ-CF-AGENT-012: duplicate approved directories must fail");

    auto too_many_paths = tree.configuration();
    too_many_paths.trusted_executable_directories.assign(
        copperfin::security::workspace_agent_environment_max_path_directories + 1U,
        tree.approved_one);
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(
                too_many_paths).has_value(),
           "RQ-CF-AGENT-012: excessive approved-directory count must fail before copying");

    const auto delimited = tree.root /
#if defined(_WIN32)
        "bad;path";
#else
        "bad:path";
#endif
    std::filesystem::create_directory(delimited);
    auto ambiguous_path = tree.configuration();
    ambiguous_path.trusted_executable_directories = {delimited};
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(
                ambiguous_path).has_value(),
           "RQ-CF-AGENT-012: platform PATH-delimiter ambiguity must fail at trusted configuration");

#if defined(_WIN32)
    auto missing_system_root = tree.configuration();
    missing_system_root.trusted_windows_system_root.clear();
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(
                missing_system_root).has_value(),
           "RQ-CF-AGENT-012: Windows construction requires an explicit trusted system root");
#else
    auto unexpected_system_root = tree.configuration();
    unexpected_system_root.trusted_windows_system_root = tree.windows_system_root;
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(
                unexpected_system_root).has_value(),
           "RQ-CF-AGENT-012: POSIX construction rejects unused Windows configuration");
#endif

    WorkspaceAgentSessionController no_boundary(tree.workspace);
    const auto no_boundary_start = no_boundary.start(activation_request(), audit_sink());
    expect_content_free_denial(
        no_boundary.preflight_process_environment_request(
            invocation_request(no_boundary_start.session.generation)),
        "workspace_agent.process_environment_boundary_unavailable",
        "RQ-CF-AGENT-012: a controller without trusted environment configuration must fail without reflection");

    WorkspaceAgentSessionController missing_layout(
        tree.workspace, tree.configuration());
    const auto missing_layout_start = missing_layout.start(
        activation_request(), audit_sink());
    std::filesystem::remove_all(tree.session_storage / "session-1" / "cache");
    expect_content_free_denial(
        missing_layout.preflight_process_environment_request(
            invocation_request(missing_layout_start.session.generation)),
        "workspace_agent.environment_session_layout_unavailable",
        "RQ-CF-AGENT-012: incomplete session-owned layout must fail without reflecting paths");
}

void test_physical_identity_and_session_binding() {
    TempTree tree;
    WorkspaceAgentSessionController controller(tree.workspace, tree.configuration());
    const auto start = controller.start(activation_request(), audit_sink());

    const auto approved_replacement = tree.root / "approved-replacement";
    std::filesystem::create_directory(approved_replacement);
    std::filesystem::remove(tree.approved_two);
    std::filesystem::rename(approved_replacement, tree.approved_two);
    expect_content_free_denial(
        controller.preflight_process_environment_request(
            invocation_request(start.session.generation)),
        "workspace_agent.environment_path_identity_changed",
        "RQ-CF-AGENT-012: replacement of an approved executable directory must fail closed");

    TempTree stale_tree;
    WorkspaceAgentSessionController stale(
        stale_tree.workspace, stale_tree.configuration());
    const auto stale_start = stale.start(activation_request(), audit_sink());
    expect_content_free_denial(
        stale.preflight_process_environment_request(
            invocation_request(stale_start.session.generation + 1U)),
        "workspace_agent.tool_stale_session",
        "RQ-CF-AGENT-012: stale session generations must fail before environment construction");

    TempTree storage_tree;
    WorkspaceAgentSessionController replaced_storage(
        storage_tree.workspace, storage_tree.configuration());
    const auto storage_start = replaced_storage.start(
        activation_request(), audit_sink());
    const auto storage_replacement = storage_tree.root / "sessions-replacement";
    std::filesystem::create_directory(storage_replacement);
    std::filesystem::remove_all(storage_tree.session_storage);
    std::filesystem::rename(storage_replacement, storage_tree.session_storage);
    expect_content_free_denial(
        replaced_storage.preflight_process_environment_request(
            invocation_request(storage_start.session.generation)),
        "workspace_agent.environment_storage_root_identity_changed",
        "RQ-CF-AGENT-012: replacement of the trusted session-storage root must fail closed");

#if defined(_WIN32)
    TempTree system_tree;
    WorkspaceAgentSessionController replaced_system(
        system_tree.workspace, system_tree.configuration());
    const auto system_start = replaced_system.start(
        activation_request(), audit_sink());
    const auto system_replacement = system_tree.root / "windows-root-replacement";
    std::filesystem::create_directory(system_replacement);
    std::filesystem::remove(system_tree.windows_system_root);
    std::filesystem::rename(system_replacement, system_tree.windows_system_root);
    expect_content_free_denial(
        replaced_system.preflight_process_environment_request(
            invocation_request(system_start.session.generation)),
        "workspace_agent.environment_system_root_identity_changed",
        "RQ-CF-AGENT-012: replacement of the trusted Windows system root must fail closed");
#endif

#if !defined(_WIN32)
    TempTree indirect_tree;
    WorkspaceAgentSessionController indirect(
        indirect_tree.workspace, indirect_tree.configuration());
    const auto indirect_start = indirect.start(activation_request(), audit_sink());
    const auto config = indirect_tree.session_storage / "session-1" / "config";
    std::filesystem::remove(config);
    std::filesystem::create_directory_symlink(indirect_tree.outside, config);
    expect_content_free_denial(
        indirect.preflight_process_environment_request(
            invocation_request(indirect_start.session.generation)),
        "workspace_agent.environment_session_layout_unavailable",
        "RQ-CF-AGENT-012: indirect session layout components must fail closed");
#endif
}

}  // namespace

int main() {
    test_fixed_non_inheriting_environment();
    test_configuration_and_layout_fail_closed();
    test_physical_identity_and_session_binding();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
