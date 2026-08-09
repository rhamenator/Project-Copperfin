// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/bounded_process.h"
#include "copperfin/platform/environment.h"
#include "copperfin/platform/path.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
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
#include <fcntl.h>
#include <io.h>
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

void configure_binary_standard_streams() {
#if defined(_WIN32)
    (void)::_setmode(::_fileno(stdin), _O_BINARY);
    (void)::_setmode(::_fileno(stdout), _O_BINARY);
    (void)::_setmode(::_fileno(stderr), _O_BINARY);
#endif
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
    if (arguments[0] == "--emit" && arguments.size() == 1U) {
        configure_binary_standard_streams();
        const std::string stdout_bytes{"out\0\xFF\n", 6U};
        const std::string stderr_bytes{"err\r\n", 5U};
        std::cout.write(stdout_bytes.data(), static_cast<std::streamsize>(stdout_bytes.size()));
        std::cerr.write(stderr_bytes.data(), static_cast<std::streamsize>(stderr_bytes.size()));
        return 0;
    }
    if (arguments[0] == "--echo-input" && arguments.size() == 1U) {
        configure_binary_standard_streams();
        const std::string input{
            std::istreambuf_iterator<char>(std::cin),
            std::istreambuf_iterator<char>()};
        std::cout.write(input.data(), static_cast<std::streamsize>(input.size()));
        return 0;
    }
    if (arguments[0] == "--duplex" && arguments.size() == 2U) {
        configure_binary_standard_streams();
        const std::size_t count = static_cast<std::size_t>(std::stoul(arguments[1]));
        const std::string stdout_block(4096U, 'O');
        const std::string stderr_block(4096U, 'E');
        std::size_t emitted = 0U;
        while (emitted < count) {
            const std::size_t next = std::min(stdout_block.size(), count - emitted);
            std::cout.write(stdout_block.data(), static_cast<std::streamsize>(next));
            std::cout.flush();
            std::cerr.write(stderr_block.data(), static_cast<std::streamsize>(next));
            std::cerr.flush();
            emitted += next;
        }
        const std::string input{
            std::istreambuf_iterator<char>(std::cin),
            std::istreambuf_iterator<char>()};
        std::cout.write(input.data(), static_cast<std::streamsize>(input.size()));
        return 0;
    }
    if (arguments[0] == "--flood" && arguments.size() == 3U) {
        configure_binary_standard_streams();
        std::ostream& output = arguments[1] == "stdout" ? std::cout : std::cerr;
        const std::size_t count = static_cast<std::size_t>(std::stoul(arguments[2]));
        const std::string block(8192U, arguments[1] == "stdout" ? 'O' : 'E');
        std::size_t written = 0U;
        while (written < count) {
            const std::size_t next = std::min(block.size(), count - written);
            output.write(block.data(), static_cast<std::streamsize>(next));
            output.flush();
            written += next;
        }
        return 0;
    }
    if (arguments[0] == "--flood-both" && arguments.size() == 2U) {
        configure_binary_standard_streams();
        const std::size_t count = static_cast<std::size_t>(std::stoul(arguments[1]));
        const std::string stdout_block(4096U, 'O');
        const std::string stderr_block(4096U, 'E');
        std::size_t written = 0U;
        while (written < count) {
            const std::size_t next = std::min(stdout_block.size(), count - written);
            std::cout.write(stdout_block.data(), static_cast<std::streamsize>(next));
            std::cout.flush();
            std::cerr.write(stderr_block.data(), static_cast<std::streamsize>(next));
            std::cerr.flush();
            written += next;
        }
        return 0;
    }
    if (arguments[0] == "--emit-sleep" &&
        (arguments.size() == 2U || arguments.size() == 3U)) {
        configure_binary_standard_streams();
        std::cout << "before-wait" << std::flush;
        std::cerr << "diagnostic" << std::flush;
        if (arguments.size() == 3U) {
            write_marker(
                copperfin::platform::path_from_utf8_string(arguments[2]), "flushed");
        }
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
    expect(exited.standard_output.empty() && exited.standard_error.empty(),
           "#4700: a silent candidate should capture empty output streams");

    const auto emitted = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {"--emit"},
        .working_directory = root_path,
        .environment = {},
        .timeout_ms = 2000U,
        .poll_interval_ms = 5U,
        .stdout_limit_bytes = 1024U,
        .stderr_limit_bytes = 1024U,
        .cancellation_requested = {}});
    expect(emitted.completed() && emitted.exit_code == 0,
           "#4700: exact-byte output helper should complete");
    expect(emitted.standard_output == std::string{"out\0\xFF\n", 6U},
           "#4700: stdout capture should preserve NUL, high-byte, and newline bytes");
    expect(emitted.standard_error == std::string{"err\r\n", 5U},
           "#4700: stderr capture should preserve exact CRLF bytes separately");

    const std::string binary_input{"in\0\xFE\r\n", 6U};
    const auto echoed = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {"--echo-input"},
        .working_directory = root_path,
        .environment = {},
        .standard_input = binary_input,
        .timeout_ms = 2000U,
        .poll_interval_ms = 5U,
        .stdin_limit_bytes = 1024U,
        .stdout_limit_bytes = 1024U,
        .stderr_limit_bytes = 1024U,
        .cancellation_requested = {}});
    expect(echoed.completed() && echoed.exit_code == 0 &&
               echoed.standard_output == binary_input && echoed.standard_error.empty(),
           "#4700: stdin transport should preserve NUL, high-byte, CR, and LF bytes exactly");

    constexpr std::uint32_t duplex_bytes = 256U * 1024U;
    const std::string duplex_input(duplex_bytes, 'I');
    const auto duplex = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {"--duplex", std::to_string(duplex_bytes)},
        .working_directory = root_path,
        .environment = {},
        .standard_input = duplex_input,
        .timeout_ms = 5000U,
        .poll_interval_ms = 5U,
        .stdin_limit_bytes = duplex_bytes,
        .stdout_limit_bytes = duplex_bytes * 2U,
        .stderr_limit_bytes = duplex_bytes,
        .cancellation_requested = {}});
    expect(duplex.completed() && duplex.exit_code == 0,
           "#4700: simultaneous stdin/stdout/stderr saturation should not deadlock");
    expect(duplex.standard_output ==
               std::string(duplex_bytes, 'O') + duplex_input &&
               duplex.standard_error == std::string(duplex_bytes, 'E'),
           "#4700: three-pipe saturation should preserve every admitted byte");

    const std::string rejected_input(1024U * 1024U, 'R');
    const auto input_closed = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {"--exit", "0"},
        .working_directory = root_path,
        .environment = {},
        .standard_input = rejected_input,
        .timeout_ms = 2000U,
        .poll_interval_ms = 5U,
        .stdin_limit_bytes = static_cast<std::uint32_t>(rejected_input.size()),
        .cancellation_requested = {}});
    expect(input_closed.status == copperfin::platform::BoundedProcessStatus::launch_failed &&
               input_closed.started && input_closed.process_tree_closed &&
               input_closed.error_code == "polyglot.process.input_write_failed",
           "#4700: a child that closes stdin before consuming the request should fail closed");

    constexpr std::uint32_t saturation_bytes = 256U * 1024U;
    const auto saturated = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {"--flood-both", std::to_string(saturation_bytes)},
        .working_directory = root_path,
        .environment = {},
        .timeout_ms = 5000U,
        .poll_interval_ms = 5U,
        .stdout_limit_bytes = saturation_bytes,
        .stderr_limit_bytes = saturation_bytes,
        .cancellation_requested = {}});
    expect(saturated.completed() && saturated.exit_code == 0,
           "#4700: concurrent stdout/stderr pipe saturation should not deadlock");
    expect(saturated.standard_output == std::string(saturation_bytes, 'O') &&
               saturated.standard_error == std::string(saturation_bytes, 'E'),
           "#4700: saturation capture should retain both streams exactly to their limits");

    const auto stdout_overflow = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {"--flood", "stdout", "65536"},
        .working_directory = root_path,
        .environment = {},
        .timeout_ms = 5000U,
        .poll_interval_ms = 5U,
        .stdout_limit_bytes = 4096U,
        .stderr_limit_bytes = 4096U,
        .cancellation_requested = {}});
    expect(
        stdout_overflow.status ==
                copperfin::platform::BoundedProcessStatus::output_limit_exceeded &&
            stdout_overflow.error_code == "polyglot.process.stdout_limit_exceeded" &&
            stdout_overflow.standard_output == std::string(4096U, 'O') &&
            stdout_overflow.process_tree_closed,
        "#4700: stdout overflow should retain only the admitted prefix and close the tree");
    expect(
        std::string{copperfin::platform::bounded_process_status_name(
            stdout_overflow.status)} == "output-limit-exceeded",
        "#4700: output-limit status text should remain invariant");

    const auto stderr_overflow = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {"--flood", "stderr", "65536"},
        .working_directory = root_path,
        .environment = {},
        .timeout_ms = 5000U,
        .poll_interval_ms = 5U,
        .stdout_limit_bytes = 4096U,
        .stderr_limit_bytes = 4096U,
        .cancellation_requested = {}});
    expect(
        stderr_overflow.status ==
                copperfin::platform::BoundedProcessStatus::output_limit_exceeded &&
            stderr_overflow.error_code == "polyglot.process.stderr_limit_exceeded" &&
            stderr_overflow.standard_error == std::string(4096U, 'E') &&
            stderr_overflow.process_tree_closed,
        "#4700: stderr overflow should retain only the admitted prefix and close the tree");

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
    const std::string expected_record_suffix =
        "|space value & no shell|" + explicit_environment_value + "|isolated";
    const bool record_suffix_matches =
        record_text.size() >= expected_record_suffix.size() &&
        record_text.compare(
            record_text.size() - expected_record_suffix.size(),
            expected_record_suffix.size(), expected_record_suffix) == 0;
    std::error_code equivalent_error;
    bool working_directory_matches = false;
    if (record_suffix_matches) {
        const std::string recorded_working_directory = record_text.substr(
            0, record_text.size() - expected_record_suffix.size());
        working_directory_matches = fs::equivalent(
            copperfin::platform::path_from_utf8_string(
                recorded_working_directory),
            root, equivalent_error);
    }
    expect(recorded.completed() && recorded.exit_code == 0,
           "#4700: direct invocation should complete in the requested directory");
    expect(
        record_suffix_matches && !equivalent_error && working_directory_matches,
        "#4700: argv, working directory, and explicit environment should survive without shell parsing or ambient PATH inheritance");

    const fs::path output_flushed_marker = root / "output-flushed.txt";
    const auto cancelled = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {
            "--emit-sleep", "2000",
            copperfin::platform::path_to_utf8_string(output_flushed_marker)},
        .working_directory = root_path,
        .environment = {},
        .timeout_ms = 3000U,
        .poll_interval_ms = 10U,
        .cancellation_requested = [&output_flushed_marker]() {
            std::error_code marker_error;
            return fs::exists(output_flushed_marker, marker_error) && !marker_error;
        }});
    expect(cancelled.status == copperfin::platform::BoundedProcessStatus::cancelled &&
               cancelled.started && cancelled.process_tree_closed,
           "#4700: live cancellation should stop the owned process tree");
    expect(cancelled.elapsed_ms < 1000U,
           "#4700: cancellation should not wait for the candidate sleep to finish");
    expect(cancelled.standard_output == "before-wait" &&
               cancelled.standard_error == "diagnostic",
           "#4700: cancellation should retain exact bytes captured before tree shutdown");

    const auto output_timed_out = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {"--emit-sleep", "2000"},
        .working_directory = root_path,
        .environment = {},
        .timeout_ms = 100U,
        .poll_interval_ms = 5U,
        .stdout_limit_bytes = 1024U,
        .stderr_limit_bytes = 1024U,
        .cancellation_requested = {}});
    expect(
        output_timed_out.status == copperfin::platform::BoundedProcessStatus::timed_out &&
            output_timed_out.process_tree_closed &&
            output_timed_out.standard_output == "before-wait" &&
            output_timed_out.standard_error == "diagnostic",
        "#4700: timeout should retain bounded bytes emitted before tree shutdown");

    const auto input_timed_out = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {"--sleep", "2000"},
        .working_directory = root_path,
        .environment = {},
        .standard_input = rejected_input,
        .timeout_ms = 100U,
        .poll_interval_ms = 5U,
        .stdin_limit_bytes = static_cast<std::uint32_t>(rejected_input.size()),
        .cancellation_requested = {}});
    expect(input_timed_out.status == copperfin::platform::BoundedProcessStatus::timed_out &&
               input_timed_out.process_tree_closed && input_timed_out.elapsed_ms < 1000U,
           "#4700: timeout should stop a writer blocked by a child that does not read stdin");

    std::atomic_int input_cancellation_polls{0};
    const auto input_cancelled = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {"--sleep", "2000"},
        .working_directory = root_path,
        .environment = {},
        .standard_input = rejected_input,
        .timeout_ms = 3000U,
        .poll_interval_ms = 5U,
        .stdin_limit_bytes = static_cast<std::uint32_t>(rejected_input.size()),
        .cancellation_requested = [&input_cancellation_polls]() {
            return input_cancellation_polls.fetch_add(1) >= 2;
        }});
    expect(input_cancelled.status == copperfin::platform::BoundedProcessStatus::cancelled &&
               input_cancelled.process_tree_closed && input_cancelled.elapsed_ms < 1000U,
           "#4700: cancellation should stop a writer blocked by unread child stdin");

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

    const auto zero_output_limit = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {},
        .working_directory = root_path,
        .environment = {},
        .timeout_ms = 100U,
        .poll_interval_ms = 1U,
        .stdout_limit_bytes = 0U,
        .stderr_limit_bytes = 1024U,
        .cancellation_requested = {}});
    expect(
        zero_output_limit.status ==
                copperfin::platform::BoundedProcessStatus::invalid_request &&
            !zero_output_limit.started,
        "#4700: a zero output budget should reject before launch");

    const auto excessive_output_limit = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {},
        .working_directory = root_path,
        .environment = {},
        .timeout_ms = 100U,
        .poll_interval_ms = 1U,
        .stdout_limit_bytes = 1024U,
        .stderr_limit_bytes = 16U * 1024U * 1024U + 1U,
        .cancellation_requested = {}});
    expect(
        excessive_output_limit.status ==
                copperfin::platform::BoundedProcessStatus::invalid_request &&
            !excessive_output_limit.started,
        "#4700: an above-hard-ceiling output budget should reject before launch");

    const auto zero_input_limit = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {},
        .working_directory = root_path,
        .environment = {},
        .timeout_ms = 100U,
        .poll_interval_ms = 1U,
        .stdin_limit_bytes = 0U,
        .cancellation_requested = {}});
    expect(zero_input_limit.status ==
               copperfin::platform::BoundedProcessStatus::invalid_request &&
               !zero_input_limit.started,
           "#4700: a zero stdin budget should reject before launch");

    const auto input_above_budget = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {},
        .working_directory = root_path,
        .environment = {},
        .standard_input = "too-large",
        .timeout_ms = 100U,
        .poll_interval_ms = 1U,
        .stdin_limit_bytes = 4U,
        .cancellation_requested = {}});
    expect(input_above_budget.status ==
               copperfin::platform::BoundedProcessStatus::invalid_request &&
               !input_above_budget.started,
           "#4700: request bytes above the caller stdin budget should reject before launch");

    const auto excessive_input_limit = copperfin::platform::run_bounded_process({
        .executable_path = executable_path,
        .arguments = {},
        .working_directory = root_path,
        .environment = {},
        .timeout_ms = 100U,
        .poll_interval_ms = 1U,
        .stdin_limit_bytes = 16U * 1024U * 1024U + 1U,
        .cancellation_requested = {}});
    expect(excessive_input_limit.status ==
               copperfin::platform::BoundedProcessStatus::invalid_request &&
               !excessive_input_limit.started,
           "#4700: an above-hard-ceiling stdin budget should reject before launch");

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
