// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_runtime_pipeline_support.h"

#include "../src/runtime/runtime_pipeline_test_hooks.h"

#include <atomic>
#include <chrono>
#include <stdexcept>
#include <thread>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace cf_test_runtime_pipeline {
namespace {

constexpr auto kProbeTimeout = std::chrono::seconds(10);
std::atomic_uint fixture_namespace_sequence{0U};

#if defined(_WIN32)
std::filesystem::path normalize_windows_fixture_root_spelling(
    const std::filesystem::path& candidate) {
    std::wstring buffer(256U, L'\0');
    for (;;) {
        const DWORD length = ::GetLongPathNameW(
            candidate.c_str(),
            buffer.data(),
            static_cast<DWORD>(buffer.size()));
        if (length == 0U) {
            return candidate;
        }
        if (length < buffer.size()) {
            buffer.resize(length);
            return std::filesystem::path(buffer);
        }
        buffer.assign(static_cast<std::size_t>(length) + 1U, L'\0');
    }
}
#endif

std::filesystem::path create_fixture_namespace_root() {
    const std::filesystem::path base = std::filesystem::temp_directory_path();
#if defined(_WIN32)
    const unsigned long process_id = static_cast<unsigned long>(::_getpid());
#else
    const unsigned long process_id = static_cast<unsigned long>(::getpid());
#endif
    for (;;) {
        const unsigned int sequence = fixture_namespace_sequence.fetch_add(1U);
        const std::filesystem::path candidate = base /
            ("cfp-" + std::to_string(process_id) + "-" + std::to_string(sequence));
        std::error_code error;
        if (std::filesystem::create_directory(candidate, error)) {
#if defined(_WIN32)
            // Windows runners may expose TEMP through an 8.3 alias such as
            // RUNNER~1. Keep exact provenance assertions independent of that
            // host spelling while retaining the actual directory-entry name.
            return normalize_windows_fixture_root_spelling(candidate);
#else
            return candidate;
#endif
        }
        if (error && error != std::errc::file_exists) {
            throw std::runtime_error("Unable to create runtime-pipeline fixture namespace.");
        }
    }
}

bool wait_for_path(const std::filesystem::path& path) {
    const auto deadline = std::chrono::steady_clock::now() + kProbeTimeout;
    std::error_code error;
    while (std::chrono::steady_clock::now() < deadline) {
        if (std::filesystem::exists(path, error) && !error) {
            return true;
        }
        error.clear();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return std::filesystem::exists(path, error) && !error;
}

std::string first_result_line(const std::filesystem::path& path) {
    const std::string result = read_text(path);
    const std::size_t newline = result.find('\n');
    return newline == std::string::npos ? result : result.substr(0U, newline);
}

#if defined(_WIN32)
std::wstring quote_process_argument(const std::filesystem::path& value) {
    return L"\"" + value.native() + L"\"";
}

struct ProbeProcess {
    HANDLE handle = nullptr;
};

ProbeProcess start_probe_process(
    const std::filesystem::path& executable_path,
    const std::string& probe_id,
    const std::filesystem::path& ready_path,
    const std::filesystem::path& go_path,
    const std::filesystem::path& result_path) {
    std::wstring command_line = quote_process_argument(executable_path) +
        L" --fixture-isolation-probe \"" +
        std::wstring(probe_id.begin(), probe_id.end()) + L"\" " +
        quote_process_argument(ready_path) + L" " +
        quote_process_argument(go_path) + L" " +
        quote_process_argument(result_path);
    STARTUPINFOW startup_info{};
    startup_info.cb = sizeof(startup_info);
    PROCESS_INFORMATION process_info{};
    if (!::CreateProcessW(
            executable_path.c_str(),
            command_line.data(),
            nullptr,
            nullptr,
            FALSE,
            0U,
            nullptr,
            nullptr,
            &startup_info,
            &process_info)) {
        return {};
    }
    (void)::CloseHandle(process_info.hThread);
    return {.handle = process_info.hProcess};
}

int wait_for_probe_process(ProbeProcess& process) {
    if (process.handle == nullptr) {
        return -1;
    }
    const DWORD wait_result = ::WaitForSingleObject(process.handle, 15000U);
    DWORD exit_code = 1U;
    if (wait_result != WAIT_OBJECT_0 ||
        ::GetExitCodeProcess(process.handle, &exit_code) == 0) {
        exit_code = 1U;
    }
    (void)::CloseHandle(process.handle);
    process.handle = nullptr;
    return static_cast<int>(exit_code);
}
#else
struct ProbeProcess {
    pid_t process_id = -1;
};

ProbeProcess start_probe_process(
    const std::filesystem::path& executable_path,
    const std::string& probe_id,
    const std::filesystem::path& ready_path,
    const std::filesystem::path& go_path,
    const std::filesystem::path& result_path) {
    const pid_t process_id = ::fork();
    if (process_id == 0) {
        ::execl(
            executable_path.c_str(),
            executable_path.c_str(),
            "--fixture-isolation-probe",
            probe_id.c_str(),
            ready_path.c_str(),
            go_path.c_str(),
            result_path.c_str(),
            nullptr);
        _exit(127);
    }
    return {.process_id = process_id};
}

int wait_for_probe_process(ProbeProcess& process) {
    if (process.process_id <= 0) {
        return -1;
    }
    int status = 1;
    if (::waitpid(process.process_id, &status, 0) != process.process_id ||
        !WIFEXITED(status)) {
        return -1;
    }
    return WEXITSTATUS(status);
}
#endif

}  // namespace

ScopedRuntimePipelineFixtureNamespace::ScopedRuntimePipelineFixtureNamespace()
    : root_(create_fixture_namespace_root()),
      tmpdir_("TMPDIR", copperfin::test_support::path_to_utf8_string(root_)),
      temp_("TEMP", copperfin::test_support::path_to_utf8_string(root_)),
      tmp_("TMP", copperfin::test_support::path_to_utf8_string(root_)) {
}

ScopedRuntimePipelineFixtureNamespace::~ScopedRuntimePipelineFixtureNamespace() {
    std::error_code ignored;
    std::filesystem::remove_all(root_, ignored);
}

const std::filesystem::path& ScopedRuntimePipelineFixtureNamespace::root() const {
    return root_;
}

int run_runtime_pipeline_fixture_isolation_probe(
    const std::string& probe_id,
    const std::filesystem::path& ready_path,
    const std::filesystem::path& go_path,
    const std::filesystem::path& result_path) {
    if (probe_id == "materialization-lock") {
        const std::string configuration = read_text(ready_path.string() + ".config");
        std::istringstream lines(configuration);
        std::string project_text;
        std::string output_text;
        std::string runtime_host_text;
        if (!std::getline(lines, project_text) ||
            !std::getline(lines, output_text) ||
            !std::getline(lines, runtime_host_text)) {
            return 1;
        }

        const std::filesystem::path project_dir =
            copperfin::test_support::path_from_utf8_string(project_text);
        const std::filesystem::path output_dir =
            copperfin::test_support::path_from_utf8_string(output_text);
        const std::filesystem::path runtime_host =
            copperfin::test_support::path_from_utf8_string(runtime_host_text);
        write_text(ready_path, "ready\n");
        if (!wait_for_path(go_path)) {
            return 1;
        }

        copperfin::studio::StudioDocumentModel document;
        document.path = (project_dir / "concurrent-child.pjx").string();
        copperfin::studio::StudioProjectWorkspace workspace;
        workspace.available = true;
        workspace.project_title = "ConcurrentMaterialization";
        workspace.home_directory = project_dir.string();
        workspace.build_plan.available = true;
        workspace.build_plan.can_build = true;
        workspace.build_plan.project_title = workspace.project_title;
        workspace.build_plan.output_path = (output_dir / "ConcurrentMaterialization.app").string();
        workspace.build_plan.output_kind = "app";
        workspace.build_plan.startup_item = "startup.prg";
        workspace.build_plan.startup_record_index = 1U;
        workspace.entries = {
            {.record_index = 1U, .name = "startup.prg", .relative_path = "startup.prg", .type_title = "Program"}
        };
        const auto plan = copperfin::runtime::create_runtime_package_plan(
            document,
            workspace,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile(),
            output_dir.string(),
            copperfin::runtime::BuildConfiguration::debug,
            false,
            false);
        const auto result = copperfin::runtime::materialize_runtime_package(
            plan,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile(),
            runtime_host.string());
        write_text(result_path, result.ok ? "ok\n" : "busy\n");
        return 0;
    }

    const ScopedRuntimePipelineFixtureNamespace fixture_namespace;
    const std::filesystem::path probe_root =
        std::filesystem::temp_directory_path() / "fixture-isolation-probe";
    const std::filesystem::path marker_path = probe_root / (probe_id + ".marker");
    std::error_code error;
    std::filesystem::remove_all(probe_root, error);
    error.clear();
    std::filesystem::create_directories(probe_root, error);
    if (error) {
        return 1;
    }
    write_text(marker_path, probe_id);
    write_text(ready_path, "ready\n");
    if (!wait_for_path(go_path)) {
        return 1;
    }

    const bool marker_survived = std::filesystem::exists(marker_path, error) && !error;
    write_text(
        result_path,
        fixture_namespace.root().string() + "\n" +
            (marker_survived ? "marker-survived\n" : "marker-missing\n"));
    std::filesystem::remove_all(probe_root, error);
    return marker_survived ? 0 : 1;
}

void test_runtime_pipeline_fixtures_are_process_isolated(
    const std::filesystem::path& executable_path) {
    namespace fs = std::filesystem;
    const fs::path coordination_root = fs::temp_directory_path() / "fixture-isolation-coordination";
    std::error_code error;
    fs::remove_all(coordination_root, error);
    error.clear();
    fs::create_directories(coordination_root, error);
    expect(!error, "#4067: fixture-isolation coordination root should be created");
    if (error) {
        return;
    }

    const fs::path first_ready = coordination_root / "first.ready";
    const fs::path second_ready = coordination_root / "second.ready";
    const fs::path go_path = coordination_root / "go";
    const fs::path first_result = coordination_root / "first.result";
    const fs::path second_result = coordination_root / "second.result";
    ProbeProcess first = start_probe_process(
        executable_path, "first", first_ready, go_path, first_result);
    ProbeProcess second = start_probe_process(
        executable_path, "second", second_ready, go_path, second_result);
    const bool both_ready = wait_for_path(first_ready) && wait_for_path(second_ready);
    expect(both_ready,
           "#4067: independent runtime-pipeline probe processes should reach their synchronization barrier");
    write_text(go_path, "go\n");

    const int first_exit = wait_for_probe_process(first);
    const int second_exit = wait_for_probe_process(second);
    expect(first_exit == 0 && second_exit == 0,
           "#4067: concurrent runtime-pipeline probe processes should retain their own fixtures");
    const std::string first_root = first_result_line(first_result);
    const std::string second_root = first_result_line(second_result);
    expect(!first_root.empty() && !second_root.empty() && first_root != second_root,
           "#4067: concurrent runtime-pipeline test processes should receive distinct temporary namespaces");
    expect(
        !first_root.empty() && !second_root.empty() &&
            std::filesystem::path(first_root).filename().string().starts_with("cfp-") &&
            std::filesystem::path(second_root).filename().string().starts_with("cfp-") &&
            std::filesystem::path(first_root).filename().string().size() < 32U &&
            std::filesystem::path(second_root).filename().string().size() < 32U,
        "#4067: process fixture namespaces should retain short, readable Windows-safe leaf names");
    expect(!first_root.empty() && !fs::exists(first_root) &&
               !second_root.empty() && !fs::exists(second_root),
           "#4067: each probe should clean up only its own temporary namespace");

    fs::remove_all(coordination_root, error);
}

int run_materialization_lock_probe_process(
    const std::filesystem::path& executable_path,
    const std::filesystem::path& config_path,
    const std::filesystem::path& ready_path,
    const std::filesystem::path& go_path,
    const std::filesystem::path& result_path) {
    write_text(ready_path.string() + ".config", read_text(config_path));
    ProbeProcess process = start_probe_process(
        executable_path,
        "materialization-lock",
        ready_path,
        go_path,
        result_path);
    if (!wait_for_path(ready_path)) {
        return -1;
    }
    write_text(go_path, "go\n");
    return wait_for_probe_process(process);
}

}  // namespace cf_test_runtime_pipeline
