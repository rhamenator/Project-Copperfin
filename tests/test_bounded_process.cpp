// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/bounded_process.h"
#include "copperfin/platform/environment.h"
#include "copperfin/platform/path.h"

#include <atomic>
#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

namespace {

int failures = 0;

unsigned long current_process_id() {
#if defined(_WIN32)
    return static_cast<unsigned long>(::GetCurrentProcessId());
#else
    return static_cast<unsigned long>(::getpid());
#endif
}

void expect(const bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

void write_marker(const std::filesystem::path& path, const std::string& value) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << value;
}

#if defined(_WIN32)

std::wstring quote_argument(const std::wstring& value) {
    std::wstring result(1U, L'"');
    std::size_t backslashes = 0U;
    for (const wchar_t character : value) {
        if (character == L'\\') {
            ++backslashes;
            continue;
        }
        if (character == L'"') {
            result.append(backslashes * 2U + 1U, L'\\');
            result.push_back(L'"');
            backslashes = 0U;
            continue;
        }
        result.append(backslashes, L'\\');
        backslashes = 0U;
        result.push_back(character);
    }
    result.append(backslashes * 2U, L'\\');
    result.push_back(L'"');
    return result;
}

bool spawn_grandchild(
    const std::filesystem::path& executable,
    const std::filesystem::path& marker) {
    std::wstring command = quote_argument(executable.wstring()) + L" --grandchild " +
        quote_argument(marker.wstring());
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    const BOOL started = ::CreateProcessW(
        executable.c_str(), command.data(), nullptr, nullptr, FALSE,
        CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process);
    if (started == FALSE) {
        return false;
    }
    (void)::CloseHandle(process.hThread);
    (void)::CloseHandle(process.hProcess);
    return true;
}

#else

bool spawn_grandchild(
    const std::filesystem::path& executable,
    const std::filesystem::path& marker) {
    const pid_t child = ::fork();
    if (child == 0) {
        const std::string executable_bytes = executable.string();
        const std::string marker_bytes = marker.string();
        const char* arguments[]{
            executable_bytes.c_str(), "--grandchild", marker_bytes.c_str(), nullptr};
        ::execv(executable_bytes.c_str(), const_cast<char* const*>(arguments));
        _exit(127);
    }
    return child > 0;
}

#endif

int run_helper(
    const std::filesystem::path& executable,
    const std::vector<std::string>& arguments) {
    if (arguments.empty()) {
        return 90;
    }
    if (arguments[0] == "--exit" && arguments.size() == 2U) {
        return std::stoi(arguments[1]);
    }
    if (arguments[0] == "--record" && arguments.size() == 3U) {
        const auto explicit_value =
            copperfin::platform::read_environment_variable("COPPERFIN_TEST_VALUE");
        const auto ambient_path = copperfin::platform::read_environment_variable("PATH");
        write_marker(
            copperfin::platform::path_from_utf8_string(arguments[1]),
            copperfin::platform::path_to_utf8_string(std::filesystem::current_path()) +
                "|" + arguments[2] + "|" + explicit_value.value_or("missing") +
                "|" + (ambient_path.has_value() ? "ambient" : "isolated"));
        return 0;
    }
    if (arguments[0] == "--sleep" && arguments.size() == 2U) {
        std::this_thread::sleep_for(std::chrono::milliseconds(std::stoi(arguments[1])));
        return 0;
    }
    if (arguments[0] == "--grandchild" && arguments.size() == 2U) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1200));
        write_marker(copperfin::platform::path_from_utf8_string(arguments[1]), "orphaned");
        return 0;
    }
    if (arguments[0] == "--tree" && arguments.size() == 2U) {
        if (!spawn_grandchild(
                executable,
                copperfin::platform::path_from_utf8_string(arguments[1]))) {
            return 91;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1200));
        return 0;
    }
    if (arguments[0] == "--tree-exit" && arguments.size() == 2U) {
        return spawn_grandchild(
            executable,
            copperfin::platform::path_from_utf8_string(arguments[1])) ? 0 : 91;
    }
    return 92;
}

int run_tests(const std::filesystem::path& executable) {
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() /
        copperfin::platform::path_from_utf8_string(
            std::string{"copperfin-bounded-process-"} + "\xC3\xA9" + "-" +
                std::to_string(current_process_id()));
    std::error_code error;
    fs::remove_all(root, error);
    error.clear();
    fs::create_directories(root, error);
    expect(!error, "#4700: bounded-process fixture root should be created");

    const std::string executable_path =
        copperfin::platform::path_to_utf8_string(fs::absolute(executable));
    const std::string root_path = copperfin::platform::path_to_utf8_string(root);
    const std::string explicit_environment_value =
        std::string{"explic"} + "\xC3\xAD" + "t";

    const auto exited = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {"--exit", "37"},
        .working_directory = root_path,
        .environment = {},
        .timeout_ms = 2000U,
        .poll_interval_ms = 5U,
        .cancellation_requested = {}});
    expect(exited.completed() && exited.started && exited.exit_code == 37,
           "#4700: direct invocation should preserve the candidate exit code");
    expect(exited.process_tree_closed && exited.error_code == "polyglot.process.exited",
           "#4700: normal completion should close the owned process tree");

    const fs::path record_path = root / "record.txt";
    const auto recorded = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {
            "--record", copperfin::platform::path_to_utf8_string(record_path),
            "space value & no shell"},
        .working_directory = root_path,
        .environment = {{
            .name = "COPPERFIN_TEST_VALUE",
            .value = explicit_environment_value}},
        .timeout_ms = 2000U,
        .poll_interval_ms = 5U,
        .cancellation_requested = {}});
    std::ifstream record(record_path, std::ios::binary);
    const std::string record_text{
        std::istreambuf_iterator<char>(record), std::istreambuf_iterator<char>()};
    expect(recorded.completed() && recorded.exit_code == 0,
           "#4700: direct invocation should complete in the requested directory");
    expect(
        record_text == root_path + "|space value & no shell|" +
            explicit_environment_value + "|isolated",
        "#4700: argv, working directory, and explicit environment should survive without shell parsing or ambient PATH inheritance");

    std::atomic_int cancellation_polls{0};
    const auto cancelled = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {"--sleep", "2000"},
        .working_directory = root_path,
        .environment = {},
        .timeout_ms = 3000U,
        .poll_interval_ms = 10U,
        .cancellation_requested = [&cancellation_polls]() {
            return cancellation_polls.fetch_add(1) >= 3;
        }});
    expect(cancelled.status == copperfin::platform::BoundedProcessStatus::cancelled &&
               cancelled.started && cancelled.process_tree_closed,
           "#4700: live cancellation should stop the owned process tree");
    expect(cancelled.elapsed_ms < 1000U,
           "#4700: cancellation should not wait for the candidate sleep to finish");

    std::atomic_int throwing_cancellation_polls{0};
    const auto callback_failed = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {"--sleep", "2000"},
        .working_directory = root_path,
        .environment = {},
        .timeout_ms = 3000U,
        .poll_interval_ms = 10U,
        .cancellation_requested = [&throwing_cancellation_polls]() {
            if (throwing_cancellation_polls.fetch_add(1) >= 2) {
                throw std::runtime_error("synthetic cancellation callback failure");
            }
            return false;
        }});
    expect(
        callback_failed.status ==
                copperfin::platform::BoundedProcessStatus::cancelled &&
            callback_failed.started && callback_failed.process_tree_closed,
        "#4700: a failing cancellation callback should fail closed and stop the owned process tree");

    const fs::path orphan_marker = root / "orphan.txt";
    const auto timed_out = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {"--tree", copperfin::platform::path_to_utf8_string(orphan_marker)},
        .working_directory = root_path,
        .environment = {},
        .timeout_ms = 100U,
        .poll_interval_ms = 5U,
        .cancellation_requested = {}});
    expect(timed_out.status == copperfin::platform::BoundedProcessStatus::timed_out &&
               timed_out.started && timed_out.process_tree_closed,
           "#4700: timeout should stop the candidate and its descendants");
    std::this_thread::sleep_for(std::chrono::milliseconds(1300));
    expect(!fs::exists(orphan_marker),
           "#4700: a timed-out descendant must not survive to write its marker");

    const fs::path completed_orphan_marker = root / "completed-orphan.txt";
    const auto tree_exited = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {
            "--tree-exit",
            copperfin::platform::path_to_utf8_string(completed_orphan_marker)},
        .working_directory = root_path,
        .environment = {},
        .timeout_ms = 1000U,
        .poll_interval_ms = 5U,
        .cancellation_requested = {}});
    expect(tree_exited.completed() && tree_exited.exit_code == 0 &&
               tree_exited.process_tree_closed,
           "#4700: normal root exit should close its remaining descendant tree");
    std::this_thread::sleep_for(std::chrono::milliseconds(1300));
    expect(!fs::exists(completed_orphan_marker),
           "#4700: a descendant must not outlive a normally completed artifact call");

    const auto pre_cancelled = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {"--sleep", "10"},
        .working_directory = root_path,
        .environment = {},
        .timeout_ms = 100U,
        .poll_interval_ms = 5U,
        .cancellation_requested = []() { return true; }});
    expect(pre_cancelled.status == copperfin::platform::BoundedProcessStatus::cancelled &&
               !pre_cancelled.started,
           "#4700: pre-launch cancellation should not start the candidate");

    const auto missing = copperfin::platform::run_bounded_process({
        .executable_path = root_path + "/missing-artifact",
        .arguments = {},
        .working_directory = root_path,
        .environment = {},
        .timeout_ms = 100U,
        .poll_interval_ms = 5U,
        .cancellation_requested = {}});
    expect(missing.status == copperfin::platform::BoundedProcessStatus::launch_failed &&
               !missing.started &&
               missing.error_code == "polyglot.process.executable_unavailable",
           "#4700: a missing artifact should fail before process creation");

    const auto invalid = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {},
        .working_directory = root_path,
        .environment = {},
        .timeout_ms = 0U,
        .poll_interval_ms = 1U,
        .cancellation_requested = {}});
    expect(invalid.status == copperfin::platform::BoundedProcessStatus::invalid_request &&
               !invalid.started,
           "#4700: a zero execution budget should be rejected");

    const auto invalid_environment = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {},
        .working_directory = root_path,
        .environment = {
            {.name = "DUPLICATE", .value = "one"},
            {.name = "DUPLICATE", .value = "two"}},
        .timeout_ms = 100U,
        .poll_interval_ms = 1U,
        .cancellation_requested = {}});
    expect(
        invalid_environment.status ==
            copperfin::platform::BoundedProcessStatus::invalid_request &&
            !invalid_environment.started,
        "#4700: duplicate explicit environment names should be rejected");

    const auto relative_executable = copperfin::platform::run_bounded_process({
        .executable_path = executable.filename().string(),
        .arguments = {},
        .working_directory = root_path,
        .environment = {},
        .timeout_ms = 100U,
        .poll_interval_ms = 1U,
        .cancellation_requested = {}});
    expect(
        relative_executable.status ==
                copperfin::platform::BoundedProcessStatus::invalid_request &&
            !relative_executable.started,
        "#4700: relative executable paths should be rejected before launch");

    const auto relative_working_directory =
        copperfin::platform::run_bounded_process({
            .executable_path = executable_path,
            .arguments = {},
            .working_directory = ".",
            .environment = {},
            .timeout_ms = 100U,
            .poll_interval_ms = 1U,
            .cancellation_requested = {}});
    expect(
        relative_working_directory.status ==
                copperfin::platform::BoundedProcessStatus::invalid_request &&
            !relative_working_directory.started,
        "#4700: relative working directories should be rejected before launch");

    const auto nul_argument = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {std::string{"truncated\0suffix", 16U}},
        .working_directory = root_path,
        .environment = {},
        .timeout_ms = 100U,
        .poll_interval_ms = 1U,
        .cancellation_requested = {}});
    expect(
        nul_argument.status ==
                copperfin::platform::BoundedProcessStatus::invalid_request &&
            !nul_argument.started,
        "#4700: arguments containing NUL should be rejected before launch");

    fs::remove_all(root, error);
    return failures == 0 ? 0 : 1;
}

}  // namespace

#if defined(_WIN32)
int wmain(int argc, wchar_t** argv) {
    try {
        const std::filesystem::path executable = std::filesystem::absolute(argv[0]);
        std::vector<std::string> arguments;
        for (int index = 1; index < argc; ++index) {
            arguments.push_back(copperfin::platform::path_to_utf8_string(argv[index]));
        }
        return arguments.empty() ? run_tests(executable) : run_helper(executable, arguments);
    } catch (const std::exception& error) {
        std::cerr << "FAIL: bounded-process test exception: " << error.what() << '\n';
        return 99;
    }
}
#else
int main(int argc, char** argv) {
    try {
        const std::filesystem::path executable = std::filesystem::absolute(argv[0]);
        const std::vector<std::string> arguments(argv + 1, argv + argc);
        return arguments.empty() ? run_tests(executable) : run_helper(executable, arguments);
    } catch (const std::exception& error) {
        std::cerr << "FAIL: bounded-process test exception: " << error.what() << '\n';
        return 99;
    }
}
#endif
