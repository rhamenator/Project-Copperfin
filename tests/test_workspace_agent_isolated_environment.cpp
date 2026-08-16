// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/environment.h"
#include "copperfin/platform/path.h"
#include "copperfin/platform/private_directory.h"
#include "copperfin/security/sha256.h"
#include "copperfin/security/workspace_agent_environment.h"
#include "copperfin/security/workspace_agent_process_parser.h"
#include "copperfin/security/workspace_agent_session.h"
#include "copperfin/security/workspace_agent_tool_registry.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <string_view>
#include <stdexcept>
#include <type_traits>
#include <thread>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace {

using copperfin::security::WorkspaceAgentAccessMode;
using copperfin::security::WorkspaceAgentActivationRequest;
using copperfin::security::WorkspaceAgentEnvironmentEntry;
using copperfin::security::WorkspaceAgentIsolatedEnvironmentBoundary;
using copperfin::security::WorkspaceAgentIsolatedEnvironmentConfiguration;
using copperfin::security::WorkspaceAgentMaterializedProcessImage;
using copperfin::security::WorkspaceAgentMaterializedProcessLaunch;
using copperfin::security::WorkspaceAgentMaterializedProcessLaunchResult;
using copperfin::security::WorkspaceAgentProcessEnvironmentPlatform;
using copperfin::security::WorkspaceAgentProcessEnvironmentPreflightResult;
using copperfin::security::WorkspaceAgentProcessEnvironmentPolicy;
using copperfin::security::WorkspaceAgentProcessArgumentParserContract;
using copperfin::security::WorkspaceAgentProcessParserDependencyContract;
using copperfin::security::WorkspaceAgentProcessParserConfiguration;
using copperfin::security::WorkspaceAgentProcessInvocationPreflightRequest;
using copperfin::security::WorkspaceAgentPreparedProcessLaunch;
using copperfin::security::WorkspaceAgentPreparedProcessLaunchResult;
using copperfin::security::WorkspaceAgentProcessExecutionControls;
using copperfin::security::WorkspaceAgentSerializedProcessEnvironmentPreflightResult;
using copperfin::security::WorkspaceAgentSerializedProcessInvocationPreflightResult;
using copperfin::security::WorkspaceAgentSessionAuditCommitResult;
using copperfin::security::WorkspaceAgentSessionAuditEvent;
using copperfin::security::WorkspaceAgentSessionAuditSink;
using copperfin::security::WorkspaceAgentSessionController;
using copperfin::security::workspace_agent_environment_max_total_bytes;
using copperfin::security::workspace_agent_serialized_environment_maximum_units;

bool valid_process_instance_id(const std::string_view value) {
    return value.size() == 32U &&
        std::all_of(value.begin(), value.end(), [](const unsigned char byte) {
            return (byte >= '0' && byte <= '9') ||
                (byte >= 'a' && byte <= 'f');
        }) && value != std::string_view("00000000000000000000000000000000");
}

template <typename T>
concept HasEnvironmentInput = requires(T value) {
    value.environment;
};

template <typename T>
concept HasEnvironmentEntriesInput = requires(T value) {
    value.environment_entries;
};

template <typename T>
concept HasPublicSessionDirectoryIdentity = requires(T value) {
    value.session_directory_identity;
};

template <typename T>
concept HasPublicChildDirectoryIdentities = requires(T value) {
    value.child_directory_identities;
};

template <typename T>
concept HasPublicPlan = requires(T value) {
    value.plan;
};

template <typename T>
concept HasPublicArguments = requires(T value) {
    value.arguments;
};

template <typename T>
concept HasPublicEnvironment = requires(T value) {
    value.environment;
};

template <typename T>
concept HasPublicExecutableBytes = requires(T value) {
    value.executable_snapshot;
};

template <typename T>
concept HasPublicNativeHandle = requires(T value) {
    value.native_handle;
};

template <typename T>
concept HasPublicPath = requires(T value) {
    value.canonical_executable_path;
};

template <typename T>
concept HasPublicDigest = requires(T value) {
    value.executable_sha256;
};

template <typename T>
concept HasPublicExecute = requires(T value) {
    value.execute();
};

template <typename T>
concept HasPublicLaunch = requires(T value) {
    value.launch();
};

static_assert(
    !HasEnvironmentInput<WorkspaceAgentProcessInvocationPreflightRequest> &&
        !HasEnvironmentEntriesInput<WorkspaceAgentProcessInvocationPreflightRequest>,
    "RQ-CF-AGENT-012: tool requests must not supply environment names or values");
static_assert(
    !HasPublicSessionDirectoryIdentity<
        copperfin::security::WorkspaceAgentSessionLayoutPreparationResult> &&
        !HasPublicChildDirectoryIdentities<
            copperfin::security::WorkspaceAgentSessionLayoutPreparationResult>,
    "RQ-CF-AGENT-020: cleanup-authorizing identities must remain opaque");
static_assert(
    !std::is_copy_constructible_v<WorkspaceAgentIsolatedEnvironmentBoundary> &&
        !std::is_copy_assignable_v<WorkspaceAgentIsolatedEnvironmentBoundary> &&
        std::is_nothrow_move_constructible_v<
            WorkspaceAgentIsolatedEnvironmentBoundary> &&
        std::is_nothrow_move_assignable_v<
            WorkspaceAgentIsolatedEnvironmentBoundary>,
    "RQ-CF-AGENT-020: boundary authority must not be duplicated by copying");
static_assert(
    !std::is_copy_constructible_v<WorkspaceAgentPreparedProcessLaunch> &&
        !std::is_copy_assignable_v<WorkspaceAgentPreparedProcessLaunch> &&
        std::is_nothrow_move_constructible_v<
            WorkspaceAgentPreparedProcessLaunch> &&
        std::is_nothrow_move_assignable_v<
            WorkspaceAgentPreparedProcessLaunch> &&
        !HasPublicPlan<WorkspaceAgentPreparedProcessLaunch> &&
        !HasPublicArguments<WorkspaceAgentPreparedProcessLaunch> &&
        !HasPublicEnvironment<WorkspaceAgentPreparedProcessLaunch> &&
        !HasPublicExecutableBytes<WorkspaceAgentPreparedProcessLaunch> &&
        !HasPublicNativeHandle<WorkspaceAgentPreparedProcessLaunch> &&
        !HasPublicPath<WorkspaceAgentPreparedProcessLaunch> &&
        !HasPublicDigest<WorkspaceAgentPreparedProcessLaunch> &&
        !HasPublicExecute<WorkspaceAgentPreparedProcessLaunch> &&
        !HasPublicLaunch<WorkspaceAgentPreparedProcessLaunch>,
    "RQ-CF-AGENT-025: prepared launch candidates must remain opaque and move-only");
static_assert(
    std::is_nothrow_default_constructible_v<
        WorkspaceAgentPreparedProcessLaunchResult> &&
        std::is_nothrow_move_constructible_v<
            WorkspaceAgentPreparedProcessLaunchResult>,
    "RQ-CF-AGENT-025: the allocation-failure denial must require no diagnostic allocation");
static_assert(
    !std::is_copy_constructible_v<WorkspaceAgentMaterializedProcessImage> &&
        !std::is_copy_assignable_v<WorkspaceAgentMaterializedProcessImage> &&
        std::is_nothrow_move_constructible_v<
            WorkspaceAgentMaterializedProcessImage> &&
        !HasPublicExecutableBytes<WorkspaceAgentMaterializedProcessImage> &&
        !HasPublicNativeHandle<WorkspaceAgentMaterializedProcessImage> &&
        !HasPublicPath<WorkspaceAgentMaterializedProcessImage> &&
        !HasPublicDigest<WorkspaceAgentMaterializedProcessImage> &&
        !HasPublicExecute<WorkspaceAgentMaterializedProcessImage> &&
        !HasPublicLaunch<WorkspaceAgentMaterializedProcessImage>,
    "RQ-CF-AGENT-026: native materialized images must remain opaque and move-only");
static_assert(
    !std::is_copy_constructible_v<WorkspaceAgentMaterializedProcessLaunch> &&
        !std::is_copy_assignable_v<WorkspaceAgentMaterializedProcessLaunch> &&
        std::is_nothrow_move_constructible_v<
            WorkspaceAgentMaterializedProcessLaunch> &&
        std::is_nothrow_move_assignable_v<
            WorkspaceAgentMaterializedProcessLaunch> &&
        !HasPublicPlan<WorkspaceAgentMaterializedProcessLaunch> &&
        !HasPublicArguments<WorkspaceAgentMaterializedProcessLaunch> &&
        !HasPublicEnvironment<WorkspaceAgentMaterializedProcessLaunch> &&
        !HasPublicExecutableBytes<WorkspaceAgentMaterializedProcessLaunch> &&
        !HasPublicNativeHandle<WorkspaceAgentMaterializedProcessLaunch> &&
        !HasPublicPath<WorkspaceAgentMaterializedProcessLaunch> &&
        !HasPublicDigest<WorkspaceAgentMaterializedProcessLaunch> &&
        !HasPublicExecute<WorkspaceAgentMaterializedProcessLaunch> &&
        !HasPublicLaunch<WorkspaceAgentMaterializedProcessLaunch>,
    "RQ-CF-AGENT-026: materialized launches must remain opaque and move-only");
static_assert(
    std::is_nothrow_default_constructible_v<
        WorkspaceAgentMaterializedProcessLaunchResult> &&
        std::is_nothrow_move_constructible_v<
            WorkspaceAgentMaterializedProcessLaunchResult>,
    "RQ-CF-AGENT-026: materialization allocation failure must return without allocating");

int failures = 0;
std::filesystem::path running_test_executable;

#if defined(_WIN32)
enum class TestProcessElevation {
    not_elevated,
    elevated,
    unavailable
};

TestProcessElevation test_process_elevation() noexcept {
    HANDLE token = nullptr;
    if (::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &token) == FALSE) {
        return TestProcessElevation::unavailable;
    }
    TOKEN_ELEVATION elevation{};
    DWORD returned = 0U;
    const BOOL queried = ::GetTokenInformation(
        token, TokenElevation, &elevation, sizeof(elevation), &returned);
    (void)::CloseHandle(token);
    if (queried == FALSE || returned < sizeof(elevation)) {
        return TestProcessElevation::unavailable;
    }
    return elevation.TokenIsElevated == 0U
        ? TestProcessElevation::not_elevated
        : TestProcessElevation::elevated;
}

int run_test_driver_with_lua_token() {
    HANDLE process_token = nullptr;
    HANDLE lua_token = nullptr;
    if (::OpenProcessToken(
            ::GetCurrentProcess(),
            TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_QUERY,
            &process_token) == FALSE ||
        ::CreateRestrictedToken(
            process_token, LUA_TOKEN | DISABLE_MAX_PRIVILEGE, 0U, nullptr, 0U,
            nullptr, 0U, nullptr, &lua_token) == FALSE) {
        const DWORD error = ::GetLastError();
        if (process_token != nullptr) {
            (void)::CloseHandle(process_token);
        }
        std::cerr << "FAIL: unable to create non-elevated Windows test token: "
                  << error << '\n';
        return EXIT_FAILURE;
    }
    (void)::CloseHandle(process_token);

    std::wstring command_line = L"\"" + running_test_executable.native() +
        L"\" --workspace-agent-non-elevated-test-driver-v1";
    command_line.push_back(L'\0');
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = ::GetStdHandle(STD_INPUT_HANDLE);
    startup.hStdOutput = ::GetStdHandle(STD_OUTPUT_HANDLE);
    startup.hStdError = ::GetStdHandle(STD_ERROR_HANDLE);
    PROCESS_INFORMATION process{};
    const BOOL created = ::CreateProcessAsUserW(
        lua_token, running_test_executable.c_str(), command_line.data(), nullptr,
        nullptr, TRUE, 0U, nullptr, nullptr, &startup, &process);
    const DWORD create_error = created == FALSE ? ::GetLastError() : ERROR_SUCCESS;
    (void)::CloseHandle(lua_token);
    if (created == FALSE) {
        std::cerr << "FAIL: unable to launch non-elevated Windows test driver: "
                  << create_error << '\n';
        return EXIT_FAILURE;
    }
    (void)::CloseHandle(process.hThread);
    const DWORD wait = ::WaitForSingleObject(process.hProcess, 60'000U);
    DWORD exit_code = EXIT_FAILURE;
    if (wait != WAIT_OBJECT_0 ||
        ::GetExitCodeProcess(process.hProcess, &exit_code) == FALSE) {
        const DWORD wait_error = wait == WAIT_FAILED
            ? ::GetLastError()
            : ERROR_TIMEOUT;
        (void)::TerminateProcess(process.hProcess, EXIT_FAILURE);
        (void)::WaitForSingleObject(process.hProcess, 5000U);
        std::cerr << "FAIL: non-elevated Windows test driver did not complete: "
                  << wait_error << '\n';
        exit_code = EXIT_FAILURE;
    }
    (void)::CloseHandle(process.hProcess);
    return static_cast<int>(exit_code);
}
#endif

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

#if defined(_WIN32)
bool output_working_directory_matches(
    const std::string& output,
    const std::filesystem::path& expected) {
    constexpr std::string_view prefix = "cwd=";
    const std::size_t beginning = output.find(prefix);
    if (beginning == std::string::npos) {
        return false;
    }
    const std::size_t value_beginning = beginning + prefix.size();
    const std::size_t ending = output.find('\n', value_beginning);
    if (ending == std::string::npos || ending == value_beginning) {
        return false;
    }
    std::error_code equivalent_error;
    const bool equivalent = std::filesystem::equivalent(
        copperfin::platform::path_from_utf8_string(
            output.substr(value_beginning, ending - value_beginning)),
        expected, equivalent_error);
    return equivalent && !equivalent_error;
}
#endif

class TempTree {
public:
    explicit TempTree(const bool create_initial_layout = false) {
        const auto suffix = std::chrono::steady_clock::now()
                                .time_since_epoch()
                                .count();
        root = std::filesystem::canonical(
                   std::filesystem::temp_directory_path()) /
            ("copperfin-agent-environment-" + std::to_string(suffix));
        workspace = root / "workspace";
        session_storage = root / "sessions";
        approved_one = root / "approved-one";
        approved_two = root / "approved-two";
        windows_system_root = root / "windows-root";
        outside = root / "outside";
        std::filesystem::create_directory(root);
#if !defined(_WIN32)
        std::filesystem::permissions(
            root,
            std::filesystem::perms::owner_all,
            std::filesystem::perm_options::replace);
#endif
        std::filesystem::create_directories(workspace / "bin");
        std::filesystem::create_directories(workspace / "working");
        std::filesystem::create_directories(approved_one);
        std::filesystem::create_directories(approved_two);
        std::filesystem::create_directories(windows_system_root);
        std::filesystem::create_directories(outside);
        require_private_directory(session_storage);
        if (create_initial_layout) {
            create_session_layout(1U);
        }
        write_executable(workspace / "bin" / "workspace-tool");
        write_executable(workspace / "bin" / "other-tool");
    }

    ~TempTree() {
        std::error_code ignored;
        std::filesystem::remove_all(root, ignored);
    }

    void create_session_layout(std::uint64_t generation) const {
        const auto session =
            session_storage / ("session-" + std::to_string(generation));
        require_private_directory(session);
        for (const std::string_view leaf :
             {"home", "temp", "config", "cache", "data"}) {
            require_private_directory(session / leaf);
        }
    }

    static void require_private_directory(const std::filesystem::path& path) {
        const auto created =
            copperfin::platform::create_private_directory(path);
        if (!created.ok) {
            throw std::runtime_error("private test directory creation failed");
        }
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
            throw std::runtime_error("Windows PE fixture copy failed");
        }
#else
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
#endif
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

    WorkspaceAgentIsolatedEnvironmentConfiguration execution_configuration() const {
        auto result = configuration();
#if defined(_WIN32)
        std::array<wchar_t, MAX_PATH + 1U> system_root{};
        const UINT length = ::GetWindowsDirectoryW(
            system_root.data(), static_cast<UINT>(system_root.size()));
        if (length == 0U || length >= system_root.size()) {
            throw std::runtime_error("Windows execution root discovery failed");
        }
        result.trusted_windows_system_root =
            std::filesystem::path(std::wstring_view(system_root.data(), length));
#endif
        return result;
    }

    WorkspaceAgentProcessParserConfiguration parser_configuration() const {
        const auto trusted =
            copperfin::security::inspect_physical_path_containment(
                workspace / "bin" / "workspace-tool", workspace / "bin");
        const auto snapshot =
            copperfin::security::read_physically_contained_file_snapshot(
                trusted, workspace / "bin");
        const auto digest = snapshot.ok
            ? copperfin::security::sha256_hex_for_text(snapshot.bytes)
            : copperfin::security::Sha256Result{};
        return {
            .windows_bindings = {{
                .trusted_absolute_executable =
                    workspace / "bin" / "workspace-tool",
                .expected_identity = trusted.identity,
                .expected_sha256 = digest.ok ? digest.hex_digest : std::string{},
                .dependency_contract = WorkspaceAgentProcessParserDependencyContract::
                    self_contained_launch_image_v1,
                .contract = WorkspaceAgentProcessArgumentParserContract::
                    windows_c_runtime_argv_v1}}};
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

struct AuditCapture {
    std::vector<WorkspaceAgentSessionAuditEvent> events;
    std::size_t fail_on_event = 0U;
};

WorkspaceAgentSessionAuditCommitResult capture_audit(
    const WorkspaceAgentSessionAuditEvent& event,
    void* context) {
    auto* capture = static_cast<AuditCapture*>(context);
    if (capture == nullptr) {
        return {};
    }
    capture->events.push_back(event);
    if (capture->fail_on_event != 0U &&
        capture->events.size() == capture->fail_on_event) {
        return {};
    }
    return {.ok = true, .receipt = "isolated-environment-captured-receipt"};
}

WorkspaceAgentSessionAuditSink audit_sink(AuditCapture& capture) {
    return {.commit = capture_audit, .context = &capture};
}

#if !defined(_WIN32)
struct ForkingIntentAudit {
    pid_t original_process = 0;
    pid_t child_process = -1;
};

WorkspaceAgentSessionAuditCommitResult capture_audit_with_intent_fork(
    const WorkspaceAgentSessionAuditEvent& event,
    void* context) {
    auto* capture = static_cast<ForkingIntentAudit*>(context);
    if (capture == nullptr) {
        return {};
    }
    if (::getpid() == capture->original_process &&
        capture->child_process < 0 &&
        event.kind == copperfin::security::WorkspaceAgentSessionEventKind::
            process_launch_intent) {
        capture->child_process = ::fork();
        if (capture->child_process < 0) {
            return {};
        }
    }
    return {.ok = true, .receipt = "isolated-environment-fork-receipt"};
}
#endif

struct ReentrantStopAudit {
    WorkspaceAgentSessionController* controller = nullptr;
    bool stop_attempted = false;
    copperfin::security::WorkspaceAgentSessionStopResult stop_result;
    std::vector<WorkspaceAgentSessionAuditEvent> events;
};

WorkspaceAgentSessionAuditCommitResult capture_audit_with_reentrant_stop(
    const WorkspaceAgentSessionAuditEvent& event,
    void* context) {
    auto* capture = static_cast<ReentrantStopAudit*>(context);
    if (capture == nullptr || capture->controller == nullptr) {
        return {};
    }
    capture->events.push_back(event);
    if (!capture->stop_attempted &&
        event.kind == copperfin::security::WorkspaceAgentSessionEventKind::
            process_launch_intent) {
        capture->stop_attempted = true;
        capture->stop_result = capture->controller->stop(audit_sink());
    }
    return {.ok = true, .receipt = "isolated-environment-reentrant-receipt"};
}

struct SlowIntentAudit {
    std::mutex mutex;
    std::condition_variable changed;
    bool intent_entered = false;
    bool release_intent = false;
    std::vector<WorkspaceAgentSessionAuditEvent> events;
};

WorkspaceAgentSessionAuditCommitResult capture_slow_intent_audit(
    const WorkspaceAgentSessionAuditEvent& event,
    void* context) {
    auto* capture = static_cast<SlowIntentAudit*>(context);
    if (capture == nullptr) {
        return {};
    }
    std::unique_lock lock(capture->mutex);
    capture->events.push_back(event);
    if (event.kind == copperfin::security::WorkspaceAgentSessionEventKind::
            process_launch_intent) {
        capture->intent_entered = true;
        capture->changed.notify_all();
        capture->changed.wait(lock, [&capture] {
            return capture->release_intent;
        });
    }
    return {.ok = true, .receipt = "isolated-environment-slow-receipt"};
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

WorkspaceAgentActivationRequest unrestricted_activation_request() {
    return {
        .requested_mode = WorkspaceAgentAccessMode::unrestricted_local,
        .feature_enabled = true,
        .permission_granted = true,
        .trusted_product_ui = true,
        .audit_sink_available = true,
        .warning_presented = true,
        .warning_id =
            copperfin::security::workspace_agent_unrestricted_warning_id,
        .user_confirmed = true};
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

WorkspaceAgentProcessInvocationPreflightRequest execution_invocation_request(
    std::uint64_t generation) {
    auto request = invocation_request(generation);
    request.arguments = {"--workspace-agent-child-v1", "literal-payload"};
    return request;
}

#if defined(_WIN32)
WorkspaceAgentProcessInvocationPreflightRequest waiting_execution_invocation_request(
    std::uint64_t generation) {
    auto request = invocation_request(generation);
    request.arguments = {"--workspace-agent-child-wait-v1"};
    return request;
}
#endif

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

void expect_serialization_content_free_denial(
    const WorkspaceAgentSerializedProcessEnvironmentPreflightResult& result,
    std::string_view diagnostic,
    const std::string& message) {
    expect(!result.allowed && result.diagnostic_code == diagnostic &&
               !result.environment_plan.allowed &&
               result.environment_plan.session_generation == 0U &&
               result.environment_plan.tool_id.empty() &&
               result.environment_plan.environment_entries.empty() &&
               result.posix_environment.empty() &&
               result.windows_environment_block.empty(),
           message);
}

void expect_invocation_serialization_content_free_denial(
    const WorkspaceAgentSerializedProcessInvocationPreflightResult& result,
    std::string_view diagnostic,
    const std::string& message) {
    expect(!result.allowed && result.diagnostic_code == diagnostic &&
               !result.serialized_environment.allowed &&
               result.posix_arguments.empty() &&
               result.windows_command_line.empty() &&
               result.argument_parser_contract ==
                   WorkspaceAgentProcessArgumentParserContract::none,
           message);
}

void test_fixed_non_inheriting_environment() {
    TempTree tree;
    WorkspaceAgentSessionController controller(
        tree.workspace, tree.configuration(), tree.parser_configuration());
    const auto start = controller.start(activation_request(), audit_sink());
    expect(start.activated && start.session.generation == 1U,
           "RQ-CF-AGENT-012: fixture session must activate at generation one");

    const auto result = controller.preflight_process_environment_request(
        invocation_request(start.session.generation));
    expect(
        workspace_agent_serialized_environment_maximum_units(
            WorkspaceAgentProcessEnvironmentPlatform::windows_v1, 10U) ==
                workspace_agent_environment_max_total_bytes + 11U &&
            workspace_agent_serialized_environment_maximum_units(
                WorkspaceAgentProcessEnvironmentPlatform::posix_v1, 9U) ==
                workspace_agent_environment_max_total_bytes + 9U &&
            workspace_agent_serialized_environment_maximum_units(
                static_cast<WorkspaceAgentProcessEnvironmentPlatform>(99U),
                1U) == 0U,
        "RQ-CF-AGENT-013: the caller cap must include every entry terminator and the final Windows block terminator");
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

    const auto serialized =
        controller.preflight_serialized_process_environment_request(
            invocation_request(start.session.generation));
    expect(serialized.allowed &&
               serialized.diagnostic_code ==
                   "workspace_agent.process_environment_serialization_request_allowed" &&
               serialized.environment_plan.allowed &&
               serialized.environment_plan.session_generation ==
                   result.session_generation &&
               serialized.environment_plan.canonical_executable_path ==
                   result.canonical_executable_path &&
               serialized.environment_plan.executable_identity ==
                   result.executable_identity &&
               serialized.environment_plan.canonical_working_directory ==
                   result.canonical_working_directory &&
               serialized.environment_plan.working_directory_identity ==
                   result.working_directory_identity &&
               serialized.environment_plan.arguments == result.arguments &&
               serialized.environment_plan.environment_entries ==
                   result.environment_entries,
           "RQ-CF-AGENT-013: serialization must remain bound to the exact admitted invocation and environment");
#if defined(_WIN32)
    expect(serialized.posix_environment.empty() &&
               serialized.windows_environment_block.size() >= 2U &&
               serialized.windows_environment_block[
                   serialized.windows_environment_block.size() - 1U] == u'\0' &&
               serialized.windows_environment_block[
                   serialized.windows_environment_block.size() - 2U] == u'\0',
           "RQ-CF-AGENT-013: the controller must emit only a double-NUL Windows UTF-16 block");
#else
    bool exact_posix = serialized.windows_environment_block.empty() &&
        serialized.posix_environment.size() == result.environment_entries.size();
    for (std::size_t index = 0U;
         exact_posix && index < result.environment_entries.size(); ++index) {
        exact_posix = serialized.posix_environment[index] ==
            result.environment_entries[index].name + "=" +
                result.environment_entries[index].value;
    }
    expect(exact_posix,
           "RQ-CF-AGENT-013: the controller must emit only exact POSIX name=value entries");
#endif

    const auto serialized_invocation =
        controller.preflight_serialized_process_invocation_request(
            invocation_request(start.session.generation));
    expect(serialized_invocation.allowed &&
               serialized_invocation.diagnostic_code ==
                   "workspace_agent.process_argument_serialization_request_allowed" &&
               serialized_invocation.serialized_environment.allowed &&
               serialized_invocation.serialized_environment.environment_plan
                       .canonical_executable_path ==
                   result.canonical_executable_path &&
               serialized_invocation.serialized_environment.environment_plan
                       .arguments ==
                   result.arguments,
           "RQ-CF-AGENT-015: platform arguments must remain bound to the exact bracketed invocation and fixed environment");
#if defined(_WIN32)
    expect(serialized_invocation.posix_arguments.empty() &&
               !serialized_invocation.windows_command_line.empty() &&
               serialized_invocation.argument_parser_contract ==
                   WorkspaceAgentProcessArgumentParserContract::
                       windows_c_runtime_argv_v1,
           "RQ-CF-AGENT-018: Windows preflight must bind the command line to exact trusted C-runtime parser authority");
#else
    std::vector<std::string> expected_arguments{
        copperfin::platform::path_to_utf8_string(
            result.canonical_executable_path)};
    expected_arguments.insert(
        expected_arguments.end(), result.arguments.begin(), result.arguments.end());
    expect(serialized_invocation.windows_command_line.empty() &&
               serialized_invocation.posix_arguments == expected_arguments &&
               serialized_invocation.argument_parser_contract ==
                   WorkspaceAgentProcessArgumentParserContract::posix_argv_v1,
           "RQ-CF-AGENT-018: POSIX preflight must retain native argv authority without a Windows parser binding");
#endif

    const auto launch_revalidation =
        controller.revalidate_serialized_process_invocation_for_launch(
            invocation_request(start.session.generation), serialized_invocation);
    expect(!launch_revalidation.allowed &&
               launch_revalidation.diagnostic_code ==
                   "workspace_agent.process_launch_revalidation_pinning_unavailable",
           "RQ-CF-AGENT-019: even a valid point-in-time plan must fail closed until launch pins and revocation binding exist");

    const auto invalid =
        controller.revalidate_serialized_process_invocation_for_launch(
            invocation_request(start.session.generation), {});
    expect(!invalid.allowed &&
               invalid.diagnostic_code ==
                   "workspace_agent.process_launch_revalidation_pinning_unavailable",
           "RQ-CF-AGENT-019: a denied input plan must receive the same content-free unavailable result");

    auto altered_plan = serialized_invocation;
#if defined(_WIN32)
    altered_plan.windows_command_line.push_back(u' ');
#else
    altered_plan.posix_arguments.push_back("injected");
#endif
    const auto altered =
        controller.revalidate_serialized_process_invocation_for_launch(
            invocation_request(start.session.generation), altered_plan);
    expect(!altered.allowed &&
               altered.diagnostic_code ==
                   "workspace_agent.process_launch_revalidation_pinning_unavailable",
           "RQ-CF-AGENT-019: caller-held plan content must not affect the invariant denial contract");
}

void test_prepared_launch_candidate_binds_plan_pins_and_revocation() {
    WorkspaceAgentPreparedProcessLaunch empty;
    expect(!empty.valid() && empty.session_generation() == 0U,
           "RQ-CF-AGENT-025: an empty prepared launch candidate must carry no authority");

    TempTree tree;
    WorkspaceAgentSessionController controller(
        tree.workspace, tree.configuration(), tree.parser_configuration());
    const auto inactive = controller.prepare_process_launch_candidate(
        invocation_request(1U));
    expect(!inactive.prepared && !inactive.candidate.has_value() &&
               inactive.diagnostic_code == "workspace_agent.session_not_active",
           "RQ-CF-AGENT-025: an inactive session must not prepare a launch candidate");

    const auto started = controller.start(activation_request(), audit_sink());
    expect(started.activated,
           "RQ-CF-AGENT-025: prepared-candidate fixture must activate");
    const auto stale = controller.prepare_process_launch_candidate(
        invocation_request(started.session.generation + 1U));
    expect(!stale.prepared && !stale.candidate.has_value() &&
               stale.diagnostic_code == "workspace_agent.tool_stale_session",
           "RQ-CF-AGENT-025: stale generation input must fail before candidate preparation");

    auto prepared = controller.prepare_process_launch_candidate(
        invocation_request(started.session.generation));
    expect(prepared.prepared && prepared.candidate.has_value() &&
               prepared.candidate->valid() &&
               prepared.candidate->session_generation() ==
                   started.session.generation &&
               prepared.diagnostic_code ==
                   "workspace_agent.process_launch_candidate_prepared",
           "RQ-CF-AGENT-025: one opaque candidate must bind the exact plan, authenticated pins, and generation lease");
    if (!prepared.candidate.has_value()) {
        std::cerr << "RQ-CF-AGENT-025 prepare diagnostic="
                  << prepared.diagnostic_code << '\n';
        return;
    }

    auto held = std::move(*prepared.candidate);
    expect(held.valid() && !prepared.candidate->valid() &&
               prepared.candidate->session_generation() == 0U,
           "RQ-CF-AGENT-025: moving a candidate must transfer rather than duplicate its authority");
    prepared.candidate.reset();
    const auto still_denied =
        controller.revalidate_serialized_process_invocation_for_launch({}, {});
    expect(!still_denied.allowed &&
               still_denied.diagnostic_code ==
                   "workspace_agent.process_launch_revalidation_pinning_unavailable",
           "RQ-CF-AGENT-025: a prepared candidate must not weaken the invariant launch gate");

    std::atomic<bool> stop_finished = false;
    copperfin::security::WorkspaceAgentSessionStopResult stop_result;
    std::thread stop_thread([&controller, &stop_finished, &stop_result] {
        stop_result = controller.stop(audit_sink());
        stop_finished.store(true);
    });
    bool stopping_observed = false;
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (std::chrono::steady_clock::now() < deadline) {
        const auto during_stop = controller.preflight_tool_request({
            .session_generation = started.session.generation,
            .tool_id = std::string(
                copperfin::security::workspace_agent_tool_workspace_run_process)});
        if (during_stop.diagnostic_code ==
            "workspace_agent.session_transition_in_progress") {
            stopping_observed = true;
            break;
        }
        std::this_thread::yield();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    expect(stopping_observed && !stop_finished.load(),
           "RQ-CF-AGENT-025: stop must not revoke while a prepared candidate owns its exact-generation lease");
    held = WorkspaceAgentPreparedProcessLaunch{};
    stop_thread.join();
    expect(stop_finished.load() && stop_result.revoked &&
               !controller.snapshot().active,
           "RQ-CF-AGENT-025: discarding the candidate must release pins before allowing revocation to complete");
}

void test_prepared_candidate_materializes_only_retained_snapshot() {
    WorkspaceAgentMaterializedProcessLaunch empty;
    expect(!empty.valid() && empty.session_generation() == 0U,
           "RQ-CF-AGENT-026: an empty materialized launch must carry no authority");

    TempTree tree;
    WorkspaceAgentSessionController controller(
        tree.workspace, tree.configuration(), tree.parser_configuration());
    const auto started = controller.start(activation_request(), audit_sink());
    expect(started.activated,
           "RQ-CF-AGENT-026: materialization fixture must activate exact generation authority");

    TempTree other_tree;
    WorkspaceAgentSessionController other_controller(
        other_tree.workspace, other_tree.configuration(),
        other_tree.parser_configuration());
    const auto other_started =
        other_controller.start(activation_request(), audit_sink());
    auto cross_controller_candidate =
        controller.prepare_process_launch_candidate(
            invocation_request(started.session.generation));
    expect(cross_controller_candidate.candidate.has_value(),
           "RQ-CF-AGENT-026: cross-controller fixture must prepare source authority");
    if (!cross_controller_candidate.candidate.has_value()) {
        return;
    }
    auto cross_controller = other_controller.materialize_process_launch_candidate(
        std::move(*cross_controller_candidate.candidate));
    expect(other_started.activated && !cross_controller.materialized &&
               !cross_controller.launch.has_value() &&
               cross_controller.diagnostic_code ==
                   "workspace_agent.process_image_candidate_unavailable" &&
               std::filesystem::is_empty(
                   other_tree.session_storage / "session-1" / "temp"),
           "RQ-CF-AGENT-026: a prepared candidate must not transfer materialization authority between controllers");

    auto prepared = controller.prepare_process_launch_candidate(
        invocation_request(started.session.generation));
    expect(prepared.prepared && prepared.candidate.has_value() &&
               prepared.candidate->valid(),
           "RQ-CF-AGENT-026: materialization requires one valid opaque prepared candidate");
    if (!prepared.candidate.has_value()) {
        return;
    }

#if !defined(_WIN32)
    std::ofstream(tree.workspace / "bin" / "workspace-tool",
                  std::ios::binary | std::ios::trunc)
        << "changed after snapshot\n";
#endif
    auto materialized = controller.materialize_process_launch_candidate(
        std::move(*prepared.candidate));
    expect(materialized.materialized && materialized.launch.has_value() &&
               materialized.launch->valid() &&
               materialized.launch->session_generation() ==
                   started.session.generation &&
               materialized.diagnostic_code ==
                   "workspace_agent.process_launch_materialized" &&
               !prepared.candidate->valid(),
           "RQ-CF-AGENT-026: one attempt must consume the candidate and bind the exact retained snapshot to a native image");
    const auto temporary =
        tree.session_storage / "session-1" / "temp";
#if defined(_WIN32)
    expect(std::filesystem::exists(
               temporary / "copperfin-agent-image-1.exe"),
           "RQ-CF-AGENT-026: Windows must retain one handle-protected image while authority is live");
#else
    expect(std::filesystem::is_empty(temporary),
           "RQ-CF-AGENT-026: POSIX must unlink the image before exposing materialized authority");
#endif

    const auto still_denied =
        controller.revalidate_serialized_process_invocation_for_launch({}, {});
    expect(!still_denied.allowed &&
               still_denied.diagnostic_code ==
                   "workspace_agent.process_launch_revalidation_pinning_unavailable",
           "RQ-CF-AGENT-026: materialization must not silently weaken the invariant execution denial");

    std::atomic<bool> stop_finished = false;
    copperfin::security::WorkspaceAgentSessionStopResult stop_result;
    std::thread stop_thread([&controller, &stop_finished, &stop_result] {
        stop_result = controller.stop(audit_sink());
        stop_finished.store(true);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    expect(!stop_finished.load(),
           "RQ-CF-AGENT-026: stop must wait while materialized launch authority retains the generation lease");
    materialized.launch.reset();
    stop_thread.join();
    expect(stop_finished.load() && stop_result.revoked &&
               !std::filesystem::exists(
                   temporary / "copperfin-agent-image-1.exe"),
           "RQ-CF-AGENT-026: image cleanup must precede lease release and completed revocation");

    const auto cleaned = controller.cleanup_pending_session_layout(audit_sink());
    expect(cleaned.cleaned &&
               !std::filesystem::exists(
                   tree.session_storage / "session-1"),
           "RQ-CF-AGENT-026: controlled image lifetime must preserve identity-bound empty-layout cleanup");
    expect(other_controller.stop(audit_sink()).revoked,
           "RQ-CF-AGENT-026: cross-controller denial must leave the unrelated controller normally revocable");
}

void test_materialized_execution_is_windows_unrestricted_and_audited() {
    std::uint64_t first_controller_operation_id = 0U;
    std::string first_controller_process_instance_id;
    {
        TempTree tree;
        WorkspaceAgentSessionController controller(
            tree.workspace, tree.configuration(), tree.parser_configuration());
        const auto started = controller.start(activation_request(), audit_sink());
        auto prepared = controller.prepare_process_launch_candidate(
            execution_invocation_request(started.session.generation));
        if (!prepared.candidate.has_value()) {
            expect(false,
                   "RQ-CF-AGENT-028: sandbox-denial fixture must prepare one opaque candidate");
            return;
        }
        auto materialized = controller.materialize_process_launch_candidate(
            std::move(*prepared.candidate));
        if (!materialized.launch.has_value()) {
            expect(false,
                   "RQ-CF-AGENT-028: sandbox-denial fixture must materialize one exact image");
            return;
        }
        AuditCapture audit;
        const auto denied = controller.execute_materialized_process_launch(
            std::move(*materialized.launch), {}, audit_sink(audit));
        expect(!denied.attempted && denied.intent_audit_committed &&
                   denied.outcome_audit_committed && denied.operation_id != 0U &&
                   valid_process_instance_id(denied.process_instance_id) &&
                   denied.diagnostic_code ==
                       "workspace_agent.process_execution_requires_unrestricted_local" &&
                   audit.events.size() == 2U &&
                   audit.events[0].schema_version == 2U &&
                   audit.events[0].kind ==
                       copperfin::security::WorkspaceAgentSessionEventKind::
                           process_launch_intent &&
                   audit.events[1].kind ==
                       copperfin::security::WorkspaceAgentSessionEventKind::
                           process_launch_outcome &&
                   audit.events[0].operation_id == audit.events[1].operation_id &&
                   audit.events[0].process_instance_id ==
                       denied.process_instance_id &&
                   audit.events[1].process_instance_id ==
                       denied.process_instance_id &&
                   audit.events[1].outcome == "denied" &&
                   std::filesystem::is_empty(
                       tree.session_storage / "session-1" / "temp"),
               "RQ-CF-AGENT-028: workspace-sandbox mode must consume and audit the attempt without starting an unsandboxed process");
        first_controller_operation_id = denied.operation_id;
        first_controller_process_instance_id = denied.process_instance_id;
        expect(controller.stop(audit_sink()).revoked &&
                   controller.cleanup_pending_session_layout(audit_sink()).cleaned,
               "RQ-CF-AGENT-028: sandbox denial must leave normal revocation and empty-layout cleanup available");
    }

    TempTree tree;
    WorkspaceAgentSessionController controller(
        tree.workspace, tree.execution_configuration(),
        tree.parser_configuration());
    const auto started = controller.start(
        unrestricted_activation_request(), audit_sink());
    expect(started.activated &&
               started.session.effective_mode ==
                   WorkspaceAgentAccessMode::unrestricted_local,
           "RQ-CF-AGENT-028: execution fixture requires exact warned unrestricted-local authority");

    auto unaudited_prepared = controller.prepare_process_launch_candidate(
        execution_invocation_request(started.session.generation));
    if (!unaudited_prepared.candidate.has_value()) {
        expect(false,
               "RQ-CF-AGENT-028: intent-audit fixture must prepare one candidate");
        return;
    }
    auto unaudited_materialized = controller.materialize_process_launch_candidate(
        std::move(*unaudited_prepared.candidate));
    if (!unaudited_materialized.launch.has_value()) {
        expect(false,
               "RQ-CF-AGENT-028: intent-audit fixture must materialize one image");
        return;
    }
    AuditCapture failing_intent;
    failing_intent.fail_on_event = 1U;
    const auto unaudited = controller.execute_materialized_process_launch(
        std::move(*unaudited_materialized.launch), {}, audit_sink(failing_intent));
    expect(!unaudited.attempted && !unaudited.intent_audit_committed &&
               !unaudited.outcome_audit_committed &&
               unaudited.diagnostic_code ==
                   "workspace_agent.process_execution_intent_audit_failed" &&
               failing_intent.events.size() == 1U &&
               std::filesystem::is_empty(
                   tree.session_storage / "session-1" / "temp"),
           "RQ-CF-AGENT-028: failed durable intent must start no process and must discard the private image and lease");

    auto prepared = controller.prepare_process_launch_candidate(
        execution_invocation_request(started.session.generation));
    if (!prepared.candidate.has_value()) {
        expect(false,
               "RQ-CF-AGENT-028: execution fixture must prepare one exact candidate");
        return;
    }
    auto materialized = controller.materialize_process_launch_candidate(
        std::move(*prepared.candidate));
    if (!materialized.launch.has_value()) {
        expect(false,
               "RQ-CF-AGENT-028: execution fixture must materialize one exact image");
        return;
    }
    AuditCapture audit;
    const auto executed = controller.execute_materialized_process_launch(
        std::move(*materialized.launch), WorkspaceAgentProcessExecutionControls{},
        audit_sink(audit));
#if defined(_WIN32)
    const std::string expected_argv0 = copperfin::platform::path_to_utf8_string(
        std::filesystem::canonical(tree.workspace / "bin" / "workspace-tool"));
    const bool execution_contract_holds =
        executed.attempted && executed.intent_audit_committed &&
               executed.outcome_audit_committed && executed.operation_id != 0U &&
               executed.operation_id != first_controller_operation_id &&
               valid_process_instance_id(executed.process_instance_id) &&
               executed.process_instance_id !=
                   first_controller_process_instance_id &&
               executed.process.started && executed.process.completed() &&
               executed.process.process_tree_closed &&
               executed.process.exit_code == 23 &&
               executed.process.standard_output.find(
                   "workspace-agent-child-v1\n") != std::string::npos &&
               executed.process.standard_output.find(
                   "argv0=" + expected_argv0 + "\n") != std::string::npos &&
               executed.process.standard_output.find(
                   "payload=literal-payload\n") != std::string::npos &&
               output_working_directory_matches(
                   executed.process.standard_output,
                   std::filesystem::canonical(tree.workspace / "working")) &&
               executed.process.standard_output.find("ambient=<unset>\n") !=
                   std::string::npos &&
               executed.diagnostic_code == "polyglot.process.exited" &&
               audit.events.size() == 2U &&
               audit.events[0].operation_id == audit.events[1].operation_id &&
               audit.events[0].process_instance_id ==
                   executed.process_instance_id &&
               audit.events[1].process_instance_id ==
                   executed.process_instance_id &&
               audit.events[1].outcome == "exited" &&
               std::filesystem::is_empty(
                   tree.session_storage / "session-1" / "temp");
    if (!execution_contract_holds) {
        std::cerr << "RQ-CF-AGENT-028 execution diagnostics: status="
                  << copperfin::platform::bounded_process_status_name(
                         executed.process.status)
                  << " error=" << executed.process.error_code
                  << " native_error=" << executed.process.native_error
                  << " started=" << executed.process.started
                  << " tree_closed=" << executed.process.process_tree_closed
                  << " exit=" << executed.process.exit_code
                  << " diagnostic=" << executed.diagnostic_code
                  << " stdout=" << executed.process.standard_output
                  << " stderr=" << executed.process.standard_error << '\n';
    }
    expect(execution_contract_holds,
           "RQ-CF-AGENT-028: warned non-elevated Windows execution must consume the exact image and fixed argv/environment/cwd under bounded process-tree ownership and paired audit");
#else
    expect(!executed.attempted && executed.intent_audit_committed &&
               executed.outcome_audit_committed && executed.operation_id != 0U &&
               executed.operation_id != first_controller_operation_id &&
               valid_process_instance_id(executed.process_instance_id) &&
               executed.process_instance_id !=
                   first_controller_process_instance_id &&
               !executed.process.started &&
               executed.diagnostic_code ==
                   "workspace_agent.process_execution_platform_unavailable" &&
               audit.events.size() == 2U &&
               audit.events[1].outcome == "denied" &&
               std::filesystem::is_empty(
                   tree.session_storage / "session-1" / "temp"),
           "RQ-CF-AGENT-028: non-Windows hosts must consume and audit the exact attempt without exposing or executing the retained image");
#endif
    expect(controller.stop(audit_sink()).revoked &&
               controller.cleanup_pending_session_layout(audit_sink()).cleaned,
           "RQ-CF-AGENT-028: completed or platform-denied execution must preserve revocation and cleanup lifecycle");
}

void test_committed_execution_releases_revocation_lease_before_child_exit() {
#if defined(_WIN32)
    TempTree tree;
    WorkspaceAgentSessionController controller(
        tree.workspace, tree.execution_configuration(),
        tree.parser_configuration());
    const auto started = controller.start(
        unrestricted_activation_request(), audit_sink());
    auto prepared = controller.prepare_process_launch_candidate(
        waiting_execution_invocation_request(started.session.generation));
    if (!prepared.candidate.has_value()) {
        expect(false,
               "RQ-CF-AGENT-028: revocation-lifecycle fixture must prepare one candidate");
        return;
    }
    auto materialized = controller.materialize_process_launch_candidate(
        std::move(*prepared.candidate));
    if (!materialized.launch.has_value()) {
        expect(false,
               "RQ-CF-AGENT-028: revocation-lifecycle fixture must materialize one image");
        return;
    }

    std::atomic_bool process_poll_observed{false};
    std::atomic_bool execution_finished{false};
    WorkspaceAgentProcessExecutionControls controls;
    controls.timeout_ms = 5000U;
    controls.cancellation_requested = [&process_poll_observed] {
        process_poll_observed.store(true, std::memory_order_release);
        return false;
    };
    copperfin::security::WorkspaceAgentProcessExecutionResult execution_result;
    std::thread execution_thread(
        [&controller, &controls, &execution_result, &execution_finished,
         launch = std::move(*materialized.launch)]() mutable {
            execution_result = controller.execute_materialized_process_launch(
                std::move(launch), controls, audit_sink());
            execution_finished.store(true, std::memory_order_release);
        });

    const auto observation_deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (!process_poll_observed.load(std::memory_order_acquire) &&
           std::chrono::steady_clock::now() < observation_deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    const bool running_was_observed =
        process_poll_observed.load(std::memory_order_acquire);
    const auto stopped = controller.stop(audit_sink());
    const bool finished_when_stop_returned =
        execution_finished.load(std::memory_order_acquire);
    execution_thread.join();

    const bool revocation_contract_holds =
        running_was_observed && stopped.revoked &&
               !finished_when_stop_returned && execution_result.attempted &&
               execution_result.process.completed() &&
               execution_result.process.exit_code == 29 &&
               execution_result.outcome_audit_committed;
    if (!revocation_contract_holds) {
        std::cerr << "RQ-CF-AGENT-028 revocation diagnostics: observed="
                  << running_was_observed << " stopped=" << stopped.revoked
                  << " finished_at_stop=" << finished_when_stop_returned
                  << " status="
                  << copperfin::platform::bounded_process_status_name(
                         execution_result.process.status)
                  << " error=" << execution_result.process.error_code
                  << " native_error=" << execution_result.process.native_error
                  << " started=" << execution_result.process.started
                  << " tree_closed="
                  << execution_result.process.process_tree_closed
                  << " exit=" << execution_result.process.exit_code
                  << " diagnostic=" << execution_result.diagnostic_code << '\n';
    }
    expect(revocation_contract_holds,
           "RQ-CF-AGENT-028: job assignment must release the exact-generation revocation lease before the bounded child exits");
    expect(controller.cleanup_pending_session_layout(audit_sink()).cleaned,
           "RQ-CF-AGENT-028: image destruction after a revoked running invocation must permit normal layout cleanup");
#endif
}

void test_process_intent_audit_reentrant_stop_is_denied() {
    TempTree tree;
    WorkspaceAgentSessionController controller(
        tree.workspace, tree.execution_configuration(),
        tree.parser_configuration());
    const auto started = controller.start(
        unrestricted_activation_request(), audit_sink());
    auto prepared = controller.prepare_process_launch_candidate(
        execution_invocation_request(started.session.generation));
    if (!prepared.candidate.has_value()) {
        expect(false,
               "RQ-CF-AGENT-028: reentrant-audit fixture must prepare one candidate");
        return;
    }
    auto materialized = controller.materialize_process_launch_candidate(
        std::move(*prepared.candidate));
    if (!materialized.launch.has_value()) {
        expect(false,
               "RQ-CF-AGENT-028: reentrant-audit fixture must materialize one image");
        return;
    }

    ReentrantStopAudit audit{};
    audit.controller = &controller;
    const auto executed = controller.execute_materialized_process_launch(
        std::move(*materialized.launch), WorkspaceAgentProcessExecutionControls{},
        {.commit = capture_audit_with_reentrant_stop, .context = &audit});
    expect(audit.stop_attempted && !audit.stop_result.revoked &&
               audit.stop_result.diagnostic_code ==
                   "workspace_agent.session_reentrant_audit_transition_denied" &&
               audit.stop_result.session.active && audit.events.size() == 2U &&
               executed.intent_audit_committed &&
               executed.outcome_audit_committed,
           "RQ-CF-AGENT-028: an intent-audit callback must not wait on its own retained launch lease through reentrant stop");
    expect(controller.stop(audit_sink()).revoked &&
               controller.cleanup_pending_session_layout(audit_sink()).cleaned,
           "RQ-CF-AGENT-028: denied reentrant stop must preserve a later explicit revocation and cleanup");
}

void test_process_intent_audit_forked_continuation_is_denied() {
#if !defined(_WIN32)
    TempTree tree;
    WorkspaceAgentSessionController controller(
        tree.workspace, tree.execution_configuration(),
        tree.parser_configuration());
    const auto started = controller.start(
        unrestricted_activation_request(), audit_sink());
    auto prepared = controller.prepare_process_launch_candidate(
        execution_invocation_request(started.session.generation));
    if (!prepared.candidate.has_value()) {
        expect(false,
               "RQ-CF-AGENT-028: forked-audit fixture must prepare one candidate");
        return;
    }
    auto materialized = controller.materialize_process_launch_candidate(
        std::move(*prepared.candidate));
    if (!materialized.launch.has_value()) {
        expect(false,
               "RQ-CF-AGENT-028: forked-audit fixture must materialize one image");
        return;
    }

    ForkingIntentAudit audit{.original_process = ::getpid()};
    const auto executed = controller.execute_materialized_process_launch(
        std::move(*materialized.launch), WorkspaceAgentProcessExecutionControls{},
        {.commit = capture_audit_with_intent_fork, .context = &audit});
    if (::getpid() != audit.original_process) {
        const bool child_denied = executed.intent_audit_committed &&
            !executed.attempted && !executed.outcome_audit_committed &&
            executed.diagnostic_code ==
                "workspace_agent.process_execution_process_changed_after_intent_audit";
        ::_exit(child_denied ? EXIT_SUCCESS : EXIT_FAILURE);
    }

    int child_status = 0;
    const bool child_reaped = audit.child_process > 0 &&
        ::waitpid(audit.child_process, &child_status, 0) == audit.child_process;
    expect(child_reaped && WIFEXITED(child_status) &&
               WEXITSTATUS(child_status) == EXIT_SUCCESS &&
               executed.intent_audit_committed &&
               executed.outcome_audit_committed && !executed.attempted,
           "RQ-CF-AGENT-028: a callback-side fork must not let the child continuation execute or duplicate the outcome correlation pair");
    expect(controller.stop(audit_sink()).revoked &&
               controller.cleanup_pending_session_layout(audit_sink()).cleaned,
           "RQ-CF-AGENT-028: callback-side fork denial must preserve parent revocation and cleanup");
#endif
}

void test_process_cancellation_reentrant_stop_is_denied() {
#if defined(_WIN32)
    TempTree tree;
    WorkspaceAgentSessionController controller(
        tree.workspace, tree.configuration(), tree.parser_configuration());
    const auto started = controller.start(
        unrestricted_activation_request(), audit_sink());
    auto prepared = controller.prepare_process_launch_candidate(
        execution_invocation_request(started.session.generation));
    if (!prepared.candidate.has_value()) {
        expect(false,
               "RQ-CF-AGENT-028: reentrant-cancellation fixture must prepare one candidate");
        return;
    }
    auto materialized = controller.materialize_process_launch_candidate(
        std::move(*prepared.candidate));
    if (!materialized.launch.has_value()) {
        expect(false,
               "RQ-CF-AGENT-028: reentrant-cancellation fixture must materialize one image");
        return;
    }

    bool stop_attempted = false;
    copperfin::security::WorkspaceAgentSessionStopResult stop_result;
    WorkspaceAgentProcessExecutionControls controls;
    controls.cancellation_requested = [&] {
        stop_attempted = true;
        stop_result = controller.stop(audit_sink());
        return true;
    };
    const auto executed = controller.execute_materialized_process_launch(
        std::move(*materialized.launch), controls, audit_sink());
    expect(stop_attempted && !stop_result.revoked &&
               stop_result.diagnostic_code ==
                   "workspace_agent.session_reentrant_cancellation_transition_denied" &&
               stop_result.session.active && executed.attempted &&
               executed.process.status ==
                   copperfin::platform::BoundedProcessStatus::cancelled &&
               executed.intent_audit_committed &&
               executed.outcome_audit_committed,
           "RQ-CF-AGENT-028: a cancellation callback must not wait on its own retained launch lease through reentrant stop");
    expect(controller.stop(audit_sink()).revoked &&
               controller.cleanup_pending_session_layout(audit_sink()).cleaned,
           "RQ-CF-AGENT-028: denied cancellation reentry must preserve later revocation and cleanup");
#endif
}

void test_process_intent_audit_allows_concurrent_stop_to_revoke() {
    TempTree tree;
    WorkspaceAgentSessionController controller(
        tree.workspace, tree.execution_configuration(),
        tree.parser_configuration());
    const auto started = controller.start(
        unrestricted_activation_request(), audit_sink());
    auto prepared = controller.prepare_process_launch_candidate(
        execution_invocation_request(started.session.generation));
    if (!prepared.candidate.has_value()) {
        expect(false,
               "RQ-CF-AGENT-028: concurrent-stop fixture must prepare one candidate");
        return;
    }
    auto materialized = controller.materialize_process_launch_candidate(
        std::move(*prepared.candidate));
    if (!materialized.launch.has_value()) {
        expect(false,
               "RQ-CF-AGENT-028: concurrent-stop fixture must materialize one image");
        return;
    }

    SlowIntentAudit audit;
    copperfin::security::WorkspaceAgentProcessExecutionResult execution_result;
    std::thread execution_thread(
        [&controller, &audit, &execution_result,
         launch = std::move(*materialized.launch)]() mutable {
            execution_result = controller.execute_materialized_process_launch(
                std::move(launch), WorkspaceAgentProcessExecutionControls{},
                {.commit = capture_slow_intent_audit, .context = &audit});
        });
    {
        std::unique_lock lock(audit.mutex);
        audit.changed.wait(lock, [&audit] { return audit.intent_entered; });
    }

    std::atomic_bool stop_finished{false};
    copperfin::security::WorkspaceAgentSessionStopResult stop_result;
    std::thread stop_thread([&controller, &stop_finished, &stop_result] {
        stop_result = controller.stop(audit_sink());
        stop_finished.store(true, std::memory_order_release);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    const bool stopped_during_callback =
        stop_finished.load(std::memory_order_acquire);
    {
        std::lock_guard lock(audit.mutex);
        audit.release_intent = true;
    }
    audit.changed.notify_all();
    execution_thread.join();
    stop_thread.join();

    expect(!stopped_during_callback && stop_result.revoked &&
               stop_result.diagnostic_code == "workspace_agent.session_stopped" &&
               execution_result.intent_audit_committed &&
               execution_result.outcome_audit_committed,
           "RQ-CF-AGENT-028: an unrelated thread's stop must wait through a slow intent audit and then revoke normally");
    expect(controller.cleanup_pending_session_layout(audit_sink()).cleaned,
           "RQ-CF-AGENT-028: concurrent stop after intent audit must preserve normal cleanup");
}

void test_windows_serialization_requires_exact_parser_authority() {
#if defined(_WIN32)
    TempTree missing_authority;
    WorkspaceAgentSessionController missing_controller(
        missing_authority.workspace, missing_authority.configuration());
    const auto missing_start = missing_controller.start(
        activation_request(), audit_sink());
    expect_invocation_serialization_content_free_denial(
        missing_controller.preflight_serialized_process_invocation_request(
            invocation_request(missing_start.session.generation)),
        "workspace_agent.process_argument_parser_authority_unavailable",
        "RQ-CF-AGENT-018: Windows serialization without trusted-host parser configuration must fail without reflection");

    TempTree wrong_identity;
    auto parser_configuration = wrong_identity.parser_configuration();
    parser_configuration.windows_bindings.front().trusted_absolute_executable =
        wrong_identity.workspace / "bin" / "other-tool";
    parser_configuration.windows_bindings.front().expected_identity =
        copperfin::security::inspect_physical_path_containment(
            wrong_identity.workspace / "bin" / "other-tool",
            wrong_identity.workspace / "bin").identity;
    const auto wrong_snapshot =
        copperfin::security::read_physically_contained_file_snapshot(
            copperfin::security::inspect_physical_path_containment(
                wrong_identity.workspace / "bin" / "other-tool",
                wrong_identity.workspace / "bin"),
            wrong_identity.workspace / "bin");
    const auto wrong_digest = wrong_snapshot.ok
        ? copperfin::security::sha256_hex_for_text(wrong_snapshot.bytes)
        : copperfin::security::Sha256Result{};
    parser_configuration.windows_bindings.front().expected_sha256 =
        wrong_digest.ok ? wrong_digest.hex_digest : std::string{};
    WorkspaceAgentSessionController wrong_controller(
        wrong_identity.workspace,
        wrong_identity.configuration(),
        parser_configuration);
    const auto wrong_start = wrong_controller.start(
        activation_request(), audit_sink());
    expect_invocation_serialization_content_free_denial(
        wrong_controller.preflight_serialized_process_invocation_request(
            invocation_request(wrong_start.session.generation)),
        "workspace_agent.process_argument_parser_not_trusted",
        "RQ-CF-AGENT-018: Windows serialization must not transfer parser authority between executable identities");
#endif
}

void test_secure_generation_layout_preparation() {
    TempTree tree(false);
    const auto boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(tree.configuration());
    expect(boundary.has_value(),
           "RQ-CF-AGENT-014: a private trusted storage root must create the preparation boundary");
    if (!boundary.has_value()) {
        return;
    }

    const auto zero = boundary->prepare_session_layout(0U);
    expect(!zero.prepared && zero.session_generation == 0U &&
               zero.diagnostic_code ==
                   "workspace_agent.environment_invalid_session_generation",
           "RQ-CF-AGENT-014: generation zero must fail without layout authority");

    const auto prepared = boundary->prepare_session_layout(1U);
    expect(prepared.prepared && prepared.session_generation == 1U &&
               prepared.diagnostic_code ==
                   "workspace_agent.environment_session_layout_prepared",
           "RQ-CF-AGENT-014: a new generation must receive one complete private layout");
    const auto session = tree.session_storage / "session-1";
    bool complete_private_layout =
        copperfin::platform::verify_private_directory(session).ok;
    for (const std::string_view leaf :
         {"home", "temp", "config", "cache", "data"}) {
        complete_private_layout = complete_private_layout &&
            copperfin::platform::verify_private_directory(session / leaf).ok;
    }
    expect(complete_private_layout,
           "RQ-CF-AGENT-014: every prepared generation directory must satisfy the platform privacy contract");

    const auto construction = boundary->construct(
        1U, WorkspaceAgentProcessEnvironmentPolicy::isolated_session_v1);
    expect(construction.allowed,
           "RQ-CF-AGENT-014: the isolated environment must consume the verified prepared layout");

    const auto repeated = boundary->prepare_session_layout(1U);
    expect(!repeated.prepared && repeated.session_generation == 0U &&
               repeated.diagnostic_code ==
                   "workspace_agent.environment_session_layout_exists",
           "RQ-CF-AGENT-014: preparation must never adopt or overwrite an existing generation");

    TempTree partial_tree(false);
    const auto partial_boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(
            partial_tree.configuration());
    expect(partial_boundary.has_value(),
           "RQ-CF-AGENT-014: the partial-layout fixture must retain a valid private root");
    if (!partial_boundary.has_value()) {
        return;
    }
    TempTree::require_private_directory(
        partial_tree.session_storage / "session-1");
    TempTree::require_private_directory(
        partial_tree.session_storage / "session-1" / "home");
    const auto partial = partial_boundary->prepare_session_layout(1U);
    expect(!partial.prepared && partial.session_generation == 0U &&
               partial.diagnostic_code ==
                   "workspace_agent.environment_session_layout_exists" &&
               std::filesystem::exists(
                   partial_tree.session_storage / "session-1" / "home"),
           "RQ-CF-AGENT-014: a partial preexisting layout must fail without repair or deletion");

    TempTree replaced_tree(false);
    const auto replaced_boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(
            replaced_tree.configuration());
    expect(replaced_boundary.has_value(),
           "RQ-CF-AGENT-014: the replacement fixture must capture a private root");
    if (replaced_boundary.has_value()) {
        const auto replacement = replaced_tree.root / "sessions-replacement";
        TempTree::require_private_directory(replacement);
        std::filesystem::remove(replaced_tree.session_storage);
        std::filesystem::rename(replacement, replaced_tree.session_storage);
        const auto replaced = replaced_boundary->prepare_session_layout(1U);
        expect(!replaced.prepared && replaced.session_generation == 0U &&
                   replaced.diagnostic_code ==
                       "workspace_agent.environment_storage_root_identity_changed" &&
                   !std::filesystem::exists(
                       replaced_tree.session_storage / "session-1"),
               "RQ-CF-AGENT-014: root replacement must fail before creating a generation layout");
    }

    TempTree path_tree(false);
    const auto path_boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(
            path_tree.configuration());
    expect(path_boundary.has_value(),
           "RQ-CF-AGENT-014: the executable-path replacement fixture must create its boundary");
    if (path_boundary.has_value()) {
        const auto replacement = path_tree.root / "approved-replacement";
        std::filesystem::create_directory(replacement);
        std::filesystem::remove(path_tree.approved_two);
        std::filesystem::rename(replacement, path_tree.approved_two);
        const auto replaced = path_boundary->prepare_session_layout(1U);
        expect(!replaced.prepared && replaced.session_generation == 0U &&
                   replaced.diagnostic_code ==
                       "workspace_agent.environment_path_identity_changed" &&
                   !std::filesystem::exists(
                       path_tree.session_storage / "session-1"),
               "RQ-CF-AGENT-014: executable-directory replacement must fail before creating a generation layout");
    }

#if defined(_WIN32)
    TempTree system_tree(false);
    const auto system_boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(
            system_tree.configuration());
    expect(system_boundary.has_value(),
           "RQ-CF-AGENT-014: the system-root replacement fixture must create its boundary");
    if (system_boundary.has_value()) {
        const auto replacement =
            system_tree.root / "windows-root-replacement";
        std::filesystem::create_directory(replacement);
        std::filesystem::remove(system_tree.windows_system_root);
        std::filesystem::rename(replacement, system_tree.windows_system_root);
        const auto replaced = system_boundary->prepare_session_layout(1U);
        expect(!replaced.prepared && replaced.session_generation == 0U &&
                   replaced.diagnostic_code ==
                       "workspace_agent.environment_system_root_identity_changed" &&
                   !std::filesystem::exists(
                       system_tree.session_storage / "session-1"),
               "RQ-CF-AGENT-014: Windows system-root replacement must fail before creating a generation layout");
    }
#endif
}

void test_identity_bound_empty_layout_cleanup() {
    TempTree tree(false);
    const auto boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(tree.configuration());
    expect(boundary.has_value(),
           "RQ-CF-AGENT-020: cleanup fixture must create a trusted environment boundary");
    if (!boundary.has_value()) {
        return;
    }
    const auto prepared = boundary->prepare_session_layout(1U);
    expect(prepared.prepared && prepared.session_generation == 1U,
           "RQ-CF-AGENT-020: successful preparation must return an opaque cleanup receipt");
    const auto other_boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(tree.configuration());
    expect(other_boundary.has_value(),
           "RQ-CF-AGENT-020: cross-boundary fixture must capture the same trusted root");
    if (other_boundary.has_value()) {
        const auto denied =
            other_boundary->cleanup_empty_session_layout(prepared);
        expect(!denied.cleaned && denied.diagnostic_code ==
                   "workspace_agent.environment_session_layout_cleanup_invalid_receipt" &&
                   std::filesystem::exists(tree.session_storage / "session-1"),
               "RQ-CF-AGENT-020: an opaque receipt must remain bound to the boundary that prepared it");
    }
    const auto cleaned = boundary->cleanup_empty_session_layout(prepared);
    expect(cleaned.cleaned && cleaned.session_generation == 1U &&
               cleaned.diagnostic_code ==
                   "workspace_agent.environment_session_layout_cleaned" &&
               !std::filesystem::exists(tree.session_storage / "session-1"),
           "RQ-CF-AGENT-020: an exact empty prepared layout must be removed completely");

    const auto invalid = boundary->cleanup_empty_session_layout({});
    expect(!invalid.cleaned && invalid.diagnostic_code ==
               "workspace_agent.environment_session_layout_cleanup_invalid_receipt",
           "RQ-CF-AGENT-020: generation numbers without a complete preparation receipt must not authorize cleanup");

    TempTree forged_tree(true);
    const auto forged_boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(
            forged_tree.configuration());
    expect(forged_boundary.has_value(),
           "RQ-CF-AGENT-020: forged-receipt fixture must create its boundary");
    if (forged_boundary.has_value()) {
        copperfin::security::WorkspaceAgentSessionLayoutPreparationResult forged;
        forged.prepared = true;
        forged.session_generation = 1U;
        const auto denied =
            forged_boundary->cleanup_empty_session_layout(forged);
        expect(!denied.cleaned && denied.diagnostic_code ==
                   "workspace_agent.environment_session_layout_cleanup_invalid_receipt" &&
                   std::filesystem::exists(
                       forged_tree.session_storage / "session-1" / "data"),
               "RQ-CF-AGENT-020: public status fields must not forge cleanup authority for a pre-existing layout");
    }

    TempTree occupied_tree(false);
    const auto occupied_boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(
            occupied_tree.configuration());
    expect(occupied_boundary.has_value(),
           "RQ-CF-AGENT-020: occupied cleanup fixture must create its boundary");
    if (occupied_boundary.has_value()) {
        const auto occupied = occupied_boundary->prepare_session_layout(1U);
        std::ofstream(occupied_tree.session_storage / "session-1" / "data" /
                      "retained.txt") << "retain\n";
        const auto denied =
            occupied_boundary->cleanup_empty_session_layout(occupied);
        expect(!denied.cleaned &&
                   (denied.diagnostic_code ==
                        "workspace_agent.environment_session_layout_cleanup_identity_changed" ||
                    denied.diagnostic_code ==
                        "workspace_agent.environment_session_layout_cleanup_not_empty") &&
                   std::filesystem::exists(
                       occupied_tree.session_storage / "session-1" / "data" /
                       "retained.txt"),
               "RQ-CF-AGENT-020: cleanup must never recurse into or remove session content");
    }

    TempTree replaced_tree(false);
    const auto replaced_boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(
            replaced_tree.configuration());
    expect(replaced_boundary.has_value(),
           "RQ-CF-AGENT-020: replaced-child fixture must create its boundary");
    if (replaced_boundary.has_value()) {
        const auto prepared_replaced =
            replaced_boundary->prepare_session_layout(1U);
        const auto data_path =
            replaced_tree.session_storage / "session-1" / "data";
        std::filesystem::remove(data_path);
        TempTree::require_private_directory(data_path);
        const auto denied = replaced_boundary->cleanup_empty_session_layout(
            prepared_replaced);
        expect(!denied.cleaned && denied.diagnostic_code ==
                   "workspace_agent.environment_session_layout_cleanup_identity_changed" &&
                   std::filesystem::exists(data_path),
               "RQ-CF-AGENT-020: a replaced child identity must be preserved and deny cleanup");
    }
}

#if defined(__linux__)
void test_unrepresentable_layout_denied_before_creation() {
    TempTree tree(false);
    std::filesystem::remove(tree.session_storage);

    constexpr std::size_t target_parent_bytes = 4055U;
    std::filesystem::path long_parent = tree.root;
    while (long_parent.native().size() < target_parent_bytes) {
        const std::size_t remaining =
            target_parent_bytes - long_parent.native().size() - 1U;
        const std::size_t component_bytes =
            std::max<std::size_t>(1U, std::min<std::size_t>(200U, remaining));
        long_parent /= std::string(component_bytes, 'a');
        std::filesystem::create_directory(long_parent);
    }
    std::filesystem::permissions(
        long_parent,
        std::filesystem::perms::owner_all,
        std::filesystem::perm_options::replace);
    tree.session_storage = long_parent / "sessions";
    TempTree::require_private_directory(tree.session_storage);

    const auto boundary =
        WorkspaceAgentIsolatedEnvironmentBoundary::create(tree.configuration());
    expect(boundary.has_value(),
           "RQ-CF-AGENT-014: the long-path fixture must admit its private storage root");
    if (!boundary.has_value()) {
        return;
    }
    const auto result = boundary->prepare_session_layout(1U);
    expect(!result.prepared && result.session_generation == 0U &&
               result.diagnostic_code ==
                   "workspace_agent.environment_session_layout_unrepresentable" &&
               !std::filesystem::exists(tree.session_storage / "session-1"),
           "RQ-CF-AGENT-014: an oversized derived environment entry must fail before layout creation");
}
#endif

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
    expect_serialization_content_free_denial(
        no_boundary.preflight_serialized_process_environment_request(
            invocation_request(no_boundary_start.session.generation)),
        "workspace_agent.process_environment_boundary_unavailable",
        "RQ-CF-AGENT-013: serialization denial must not reflect an invocation or environment");
    expect_invocation_serialization_content_free_denial(
        no_boundary.preflight_serialized_process_invocation_request(
            invocation_request(no_boundary_start.session.generation)),
        "workspace_agent.process_environment_boundary_unavailable",
        "RQ-CF-AGENT-015: argument serialization denial must not reflect an invocation, environment, path, or argument");

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

    TempTree insecure_root_tree(false);
    std::filesystem::remove(insecure_root_tree.session_storage);
    std::filesystem::create_directory(insecure_root_tree.session_storage);
#if !defined(_WIN32)
    std::filesystem::permissions(
        insecure_root_tree.session_storage,
        std::filesystem::perms::owner_all |
            std::filesystem::perms::group_read |
            std::filesystem::perms::group_exec,
        std::filesystem::perm_options::replace);
#endif
    expect(!WorkspaceAgentIsolatedEnvironmentBoundary::create(
                insecure_root_tree.configuration()).has_value(),
           "RQ-CF-AGENT-014: an inherited or broadened trusted storage root must be rejected");

#if !defined(_WIN32)
    TempTree broadened_after_capture;
    WorkspaceAgentSessionController broadened_controller(
        broadened_after_capture.workspace,
        broadened_after_capture.configuration());
    const auto broadened_start = broadened_controller.start(
        activation_request(), audit_sink());
    std::filesystem::permissions(
        broadened_after_capture.session_storage,
        std::filesystem::perms::owner_all |
            std::filesystem::perms::group_read |
            std::filesystem::perms::group_exec,
        std::filesystem::perm_options::replace);
    expect_content_free_denial(
        broadened_controller.preflight_process_environment_request(
            invocation_request(broadened_start.session.generation)),
        "workspace_agent.environment_storage_root_identity_changed",
        "RQ-CF-AGENT-014: storage-root access broadening after capture must fail construction");
#endif
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

void test_later_session_layout_is_not_root_replacement() {
    TempTree tree;
    WorkspaceAgentSessionController controller(tree.workspace, tree.configuration());
    const auto first = controller.start(activation_request(), audit_sink());
    expect(first.activated && first.session.generation == 1U,
           "RQ-CF-AGENT-012: first fixture generation must activate");
    expect(controller.stop(audit_sink()).revoked,
           "RQ-CF-AGENT-012: first fixture generation must stop");

    const auto second = controller.start(activation_request(), audit_sink());
    expect(second.activated && second.session.generation == 2U,
           "RQ-CF-AGENT-012: later fixture generation must activate");
    const auto result = controller.preflight_process_environment_request(
        invocation_request(second.session.generation));
    expect(result.allowed &&
               result.diagnostic_code ==
                   "workspace_agent.process_environment_request_allowed" &&
               find_entry(result.environment_entries, "HOME") != nullptr,
           "RQ-CF-AGENT-016: session start must prepare a later generation without treating it as storage-root replacement");
}

void test_session_start_prepares_layout_before_authority() {
    TempTree tree;
    WorkspaceAgentSessionController controller(tree.workspace, tree.configuration());
    const auto started = controller.start(activation_request(), audit_sink());
    bool complete_private_layout = copperfin::platform::verify_private_directory(
        tree.session_storage / "session-1").ok;
    for (const std::string_view leaf :
         {"home", "temp", "config", "cache", "data"}) {
        complete_private_layout = complete_private_layout &&
            copperfin::platform::verify_private_directory(
                tree.session_storage / "session-1" / leaf).ok;
    }
    expect(started.activated && started.audit_committed &&
               started.session.generation == 1U &&
               complete_private_layout,
           "RQ-CF-AGENT-016: configured process-capable start must prepare its exact private generation before authority");

    TempTree preexisting;
    preexisting.create_session_layout(1U);
    WorkspaceAgentSessionController refuses_adoption(
        preexisting.workspace, preexisting.configuration());
    AuditCapture refused_audit;
    const auto refused = refuses_adoption.start(
        activation_request(), audit_sink(refused_audit));
    expect(!refused.activated && refused.audit_committed &&
               refused.diagnostic_code ==
                   "workspace_agent.environment_session_layout_exists" &&
               !refused.policy_decision.allowed &&
               !refuses_adoption.snapshot().active &&
               refused_audit.events.size() == 1U &&
               refused_audit.events.front().outcome == "denied" &&
               refused_audit.events.front().diagnostic_code ==
                   refused.diagnostic_code,
           "RQ-CF-AGENT-016: session start must audit denial and grant no authority when its generation already exists");

    TempTree invalid;
    auto invalid_configuration = invalid.configuration();
    invalid_configuration.schema_version = 2U;
    WorkspaceAgentSessionController invalid_boundary(
        invalid.workspace, invalid_configuration);
    const auto invalid_start = invalid_boundary.start(
        activation_request(), audit_sink());
    expect(!invalid_start.activated && invalid_start.audit_committed &&
               invalid_start.diagnostic_code ==
                   "workspace_agent.session_environment_boundary_unavailable" &&
               !std::filesystem::exists(invalid.session_storage / "session-1"),
           "RQ-CF-AGENT-016: supplied but invalid trusted environment configuration must fail start without creating authority or layout");

    TempTree policy_denied;
    WorkspaceAgentSessionController denied_controller(
        policy_denied.workspace, policy_denied.configuration());
    auto disabled = activation_request();
    disabled.feature_enabled = false;
    const auto denied = denied_controller.start(disabled, audit_sink());
    expect(!denied.activated && denied.audit_committed &&
               !std::filesystem::exists(
                   policy_denied.session_storage / "session-1"),
           "RQ-CF-AGENT-016: policy denial must occur without preparing an unused generation layout");

    TempTree advisory;
    WorkspaceAgentSessionController advisory_controller(
        advisory.workspace, advisory.configuration());
    auto advisory_request = activation_request();
    advisory_request.requested_mode = WorkspaceAgentAccessMode::advisory;
    const auto advisory_start = advisory_controller.start(
        advisory_request, audit_sink());
    expect(advisory_start.activated &&
               !advisory_start.session.capabilities.run_local_processes &&
               !std::filesystem::exists(
                   advisory.session_storage / "session-1"),
           "RQ-CF-AGENT-016: a non-process-capable session must not prepare an unused generation layout");
    expect(advisory_controller.stop(audit_sink()).revoked,
           "RQ-CF-AGENT-016: advisory fixture must revoke cleanly");
    const auto process_after_advisory = advisory_controller.start(
        activation_request(), audit_sink());
    expect(process_after_advisory.activated &&
               process_after_advisory.session.generation == 2U &&
               std::filesystem::exists(
                   advisory.session_storage / "session-2") &&
               !std::filesystem::exists(
                   advisory.session_storage / "session-1"),
           "RQ-CF-AGENT-016: a later process-capable session must prepare only its own fresh generation");

    TempTree audit_failed;
    WorkspaceAgentSessionController audit_failed_controller(
        audit_failed.workspace, audit_failed.configuration());
    const auto uncommitted = audit_failed_controller.start(
        activation_request(), {});
    expect(!uncommitted.activated && !uncommitted.audit_committed &&
               uncommitted.diagnostic_code ==
                   "workspace_agent.session_audit_commit_failed" &&
               std::filesystem::exists(
                   audit_failed.session_storage / "session-1") &&
               !audit_failed_controller.snapshot().active,
           "RQ-CF-AGENT-016: audit failure must withhold authority and leave the prepared generation untouched for later audit-backed cleanup");
    const auto recovered = audit_failed_controller.start(
        activation_request(), audit_sink());
    expect(recovered.activated && recovered.session.generation == 2U &&
               std::filesystem::exists(
                   audit_failed.session_storage / "session-2"),
           "RQ-CF-AGENT-016: a later start must use a fresh generation rather than adopt an orphaned prepared layout");
}

void test_controller_retains_and_audits_explicit_layout_cleanup() {
    TempTree tree;
    WorkspaceAgentSessionController controller(tree.workspace, tree.configuration());
    AuditCapture audit;
    const auto started = controller.start(activation_request(), audit_sink(audit));
    expect(started.activated &&
               std::filesystem::exists(tree.session_storage / "session-1"),
           "RQ-CF-AGENT-021: configured start must retain cleanup authority for its prepared generation");

    const auto active_denial =
        controller.cleanup_pending_session_layout(audit_sink(audit));
    expect(!active_denial.attempted && !active_denial.cleaned &&
               active_denial.diagnostic_code ==
                   "workspace_agent.session_layout_cleanup_active_session" &&
               audit.events.size() == 1U,
           "RQ-CF-AGENT-021: cleanup must not begin while any session authority remains active");

    const auto stopped = controller.stop(audit_sink(audit));
    const auto cleaned =
        controller.cleanup_pending_session_layout(audit_sink(audit));
    expect(stopped.revoked && cleaned.attempted && cleaned.cleaned &&
               cleaned.intent_audit_committed &&
               cleaned.outcome_audit_committed &&
               cleaned.session_generation == started.session.generation &&
               cleaned.diagnostic_code ==
                   "workspace_agent.environment_session_layout_cleaned" &&
               !std::filesystem::exists(tree.session_storage / "session-1") &&
               audit.events.size() == 4U &&
               audit.events[2].kind ==
                   copperfin::security::WorkspaceAgentSessionEventKind::
                       layout_cleanup_intent &&
               audit.events[2].outcome == "pending" &&
               audit.events[3].kind ==
                   copperfin::security::WorkspaceAgentSessionEventKind::
                       layout_cleanup_outcome &&
               audit.events[3].outcome == "cleaned",
           "RQ-CF-AGENT-021: revoked empty layout cleanup must be bracketed by intent and outcome audit records");
    const auto duplicate =
        controller.cleanup_pending_session_layout(audit_sink(audit));
    expect(!duplicate.attempted &&
               duplicate.diagnostic_code ==
                   "workspace_agent.session_layout_cleanup_not_pending" &&
               audit.events.size() == 4U,
           "RQ-CF-AGENT-021: a consumed cleanup receipt must not be reusable");

    TempTree unaudited_tree;
    WorkspaceAgentSessionController unaudited(
        unaudited_tree.workspace, unaudited_tree.configuration());
    const auto uncommitted = unaudited.start(activation_request(), {});
    expect(!uncommitted.activated &&
               std::filesystem::exists(
                   unaudited_tree.session_storage / "session-1"),
           "RQ-CF-AGENT-021: failed start audit fixture must leave its prepared layout pending");
    const auto missing_intent = unaudited.cleanup_pending_session_layout({});
    expect(!missing_intent.attempted && !missing_intent.cleaned &&
               !missing_intent.intent_audit_committed &&
               missing_intent.diagnostic_code ==
                   "workspace_agent.session_layout_cleanup_intent_audit_failed" &&
               std::filesystem::exists(
                   unaudited_tree.session_storage / "session-1"),
           "RQ-CF-AGENT-021: cleanup must not mutate without a durable intent receipt");
    AuditCapture recovery_audit;
    const auto recovered = unaudited.cleanup_pending_session_layout(
        audit_sink(recovery_audit));
    expect(recovered.cleaned && recovery_audit.events.size() == 2U &&
               !std::filesystem::exists(
                   unaudited_tree.session_storage / "session-1"),
           "RQ-CF-AGENT-021: intent-audit failure must retain the opaque receipt for a later explicit retry");

    TempTree outcome_failure_tree;
    WorkspaceAgentSessionController outcome_failure(
        outcome_failure_tree.workspace, outcome_failure_tree.configuration());
    const auto outcome_start = outcome_failure.start(
        activation_request(), audit_sink());
    expect(outcome_start.activated && outcome_failure.stop(audit_sink()).revoked,
           "RQ-CF-AGENT-021: outcome-audit failure fixture must first revoke authority");
    AuditCapture failing_outcome;
    failing_outcome.fail_on_event = 2U;
    const auto cleaned_without_outcome =
        outcome_failure.cleanup_pending_session_layout(
            audit_sink(failing_outcome));
    const auto consumed_after_outcome_failure =
        outcome_failure.cleanup_pending_session_layout(audit_sink());
    expect(cleaned_without_outcome.attempted &&
               cleaned_without_outcome.cleaned &&
               cleaned_without_outcome.intent_audit_committed &&
               !cleaned_without_outcome.outcome_audit_committed &&
               cleaned_without_outcome.diagnostic_code ==
                   "workspace_agent.session_layout_cleanup_outcome_audit_failed" &&
               failing_outcome.events.size() == 2U &&
               !std::filesystem::exists(
                   outcome_failure_tree.session_storage / "session-1") &&
               !consumed_after_outcome_failure.attempted &&
               consumed_after_outcome_failure.diagnostic_code ==
                   "workspace_agent.session_layout_cleanup_not_pending",
           "RQ-CF-AGENT-021: successful mutation must consume its receipt even when outcome audit persistence fails");

    TempTree occupied_tree;
    WorkspaceAgentSessionController occupied(
        occupied_tree.workspace, occupied_tree.configuration());
    AuditCapture occupied_audit;
    const auto occupied_start = occupied.start(
        activation_request(), audit_sink(occupied_audit));
    std::ofstream(occupied_tree.session_storage / "session-1" / "data" /
                  "retained.txt") << "retain\n";
    expect(occupied.stop(audit_sink(occupied_audit)).revoked,
           "RQ-CF-AGENT-021: occupied fixture must revoke before cleanup");
    const auto retained = occupied.cleanup_pending_session_layout(
        audit_sink(occupied_audit));
    expect(occupied_start.activated && retained.attempted && !retained.cleaned &&
               retained.intent_audit_committed &&
               retained.outcome_audit_committed &&
               retained.diagnostic_code !=
                   "workspace_agent.environment_session_layout_cleaned" &&
               std::filesystem::exists(
                   occupied_tree.session_storage / "session-1" / "data" /
                   "retained.txt") &&
               occupied_audit.events.back().outcome == "retained",
           "RQ-CF-AGENT-021: occupied content must be preserved and produce an audited retained outcome");

    TempTree bounded_tree;
    WorkspaceAgentSessionController bounded(
        bounded_tree.workspace, bounded_tree.configuration());
    for (std::size_t index = 0U;
         index <
             copperfin::security::
                 workspace_agent_session_max_pending_layout_cleanups;
         ++index) {
        const auto bounded_start = bounded.start(activation_request(), audit_sink());
        expect(bounded_start.activated && bounded.stop(audit_sink()).revoked,
               "RQ-CF-AGENT-021: every generation below the pending-receipt cap must retain its distinct receipt");
    }
    AuditCapture capacity_audit;
    const auto capacity_denied = bounded.start(
        activation_request(), audit_sink(capacity_audit));
    expect(!capacity_denied.activated && capacity_denied.audit_committed &&
               capacity_denied.diagnostic_code ==
                   "workspace_agent.session_layout_cleanup_capacity_reached" &&
               capacity_audit.events.size() == 1U &&
               capacity_audit.events.front().outcome == "denied" &&
               !std::filesystem::exists(
                   bounded_tree.session_storage / "session-65"),
           "RQ-CF-AGENT-021: the fixed pending-receipt cap must deny before creating another layout");
}

}  // namespace

int main(int argc, char** argv) {
    if (argc == 2 && argv[1] != nullptr &&
        std::string_view(argv[1]) == "--workspace-agent-child-wait-v1") {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return 29;
    }
    if (argc == 3 && argv[1] != nullptr && argv[2] != nullptr &&
        std::string_view(argv[1]) == "--workspace-agent-child-v1") {
        const auto ambient =
            copperfin::platform::read_environment_variable("GITHUB_TOKEN");
        std::cout << "workspace-agent-child-v1\n"
                  << "argv0=" << (argv[0] == nullptr ? "" : argv[0]) << '\n'
                  << "payload=" << argv[2] << '\n'
                  << "cwd=" << copperfin::platform::path_to_utf8_string(
                         std::filesystem::current_path()) << '\n'
                  << "ambient=" << (ambient.has_value() ? *ambient : "<unset>")
                  << '\n';
        return 23;
    }
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
    const bool non_elevated_driver =
        argc == 2 && argv[1] != nullptr &&
        std::string_view(argv[1]) ==
            "--workspace-agent-non-elevated-test-driver-v1";
    const auto elevation = test_process_elevation();
    if (!non_elevated_driver &&
        elevation == TestProcessElevation::elevated) {
        return run_test_driver_with_lua_token();
    }
    if (non_elevated_driver &&
        elevation != TestProcessElevation::not_elevated) {
        std::cerr << "FAIL: restricted Windows test driver remained elevated\n";
        return EXIT_FAILURE;
    }
#endif
    test_fixed_non_inheriting_environment();
    test_prepared_launch_candidate_binds_plan_pins_and_revocation();
    test_prepared_candidate_materializes_only_retained_snapshot();
    test_materialized_execution_is_windows_unrestricted_and_audited();
    test_committed_execution_releases_revocation_lease_before_child_exit();
    test_process_intent_audit_reentrant_stop_is_denied();
    test_process_intent_audit_forked_continuation_is_denied();
    test_process_cancellation_reentrant_stop_is_denied();
    test_process_intent_audit_allows_concurrent_stop_to_revoke();
    test_windows_serialization_requires_exact_parser_authority();
    test_secure_generation_layout_preparation();
    test_identity_bound_empty_layout_cleanup();
#if defined(__linux__)
    test_unrepresentable_layout_denied_before_creation();
#endif
    test_configuration_and_layout_fail_closed();
    test_physical_identity_and_session_binding();
    test_later_session_layout_is_not_root_replacement();
    test_session_start_prepares_layout_before_authority();
    test_controller_retains_and_audits_explicit_layout_cleanup();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
