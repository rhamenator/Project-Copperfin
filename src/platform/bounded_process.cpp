// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/bounded_process.h"

#include "copperfin/platform/path.h"

#include <algorithm>
#include <chrono>
#include <cerrno>
#include <cstdint>
#include <cwctype>
#include <filesystem>
#include <limits>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#if defined(_WIN32)

#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <csignal>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace copperfin::platform {
namespace {

#if defined(_WIN32)
constexpr DWORD kTerminationWaitMilliseconds = 5000U;
#endif

using Clock = std::chrono::steady_clock;

std::uint64_t elapsed_milliseconds(const Clock::time_point started) {
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        Clock::now() - started).count();
    return elapsed <= 0 ? 0U : static_cast<std::uint64_t>(elapsed);
}

BoundedProcessResult invalid_request() {
    return {};
}

bool cancellation_requested(const BoundedProcessRequest& request) noexcept {
    if (!request.cancellation_requested) {
        return false;
    }
    try {
        return request.cancellation_requested();
    } catch (...) {
        // A callback failure must never unwind past an owned live process.
        return true;
    }
}

bool contains_nul(const std::string_view value) noexcept {
    return value.find('\0') != std::string_view::npos;
}

bool valid_arguments(const std::vector<std::string>& arguments) noexcept {
    return std::none_of(
        arguments.begin(), arguments.end(),
        [](const std::string& argument) { return contains_nul(argument); });
}

bool is_environment_name_character(const char character, const bool first) noexcept {
    return (character >= 'A' && character <= 'Z') ||
        (character >= 'a' && character <= 'z') || character == '_' ||
        (!first && character >= '0' && character <= '9');
}

bool valid_environment_variable(
    const BoundedProcessEnvironmentVariable& variable) noexcept {
    if (variable.name.empty() ||
        !is_environment_name_character(variable.name.front(), true) ||
        contains_nul(variable.value)) {
        return false;
    }
    for (std::size_t index = 1U; index < variable.name.size(); ++index) {
        if (!is_environment_name_character(variable.name[index], false)) {
            return false;
        }
    }
    return true;
}

#if defined(_WIN32)
char ascii_lower(const char character) noexcept {
    return character >= 'A' && character <= 'Z'
        ? static_cast<char>(character - 'A' + 'a')
        : character;
}
#endif

bool environment_names_equal(
    const std::string_view left,
    const std::string_view right) noexcept {
    if (left.size() != right.size()) {
        return false;
    }
#if defined(_WIN32)
    for (std::size_t index = 0U; index < left.size(); ++index) {
        if (ascii_lower(left[index]) != ascii_lower(right[index])) {
            return false;
        }
    }
    return true;
#else
    return left == right;
#endif
}

bool valid_environment(
    const std::vector<BoundedProcessEnvironmentVariable>& environment) noexcept {
    for (std::size_t index = 0U; index < environment.size(); ++index) {
        if (!valid_environment_variable(environment[index])) {
            return false;
        }
        for (std::size_t previous = 0U; previous < index; ++previous) {
            if (environment_names_equal(
                    environment[index].name, environment[previous].name)) {
                return false;
            }
        }
    }
    return true;
}

#if defined(_WIN32)

std::wstring utf8_to_wide(const std::string& value, int& native_error) {
    native_error = 0;
    if (value.empty()) {
        return {};
    }
    if (value.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        native_error = ERROR_INVALID_PARAMETER;
        return {};
    }
    const int required = ::MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.data(),
        static_cast<int>(value.size()),
        nullptr,
        0);
    if (required <= 0) {
        native_error = static_cast<int>(::GetLastError());
        return {};
    }
    std::wstring result(static_cast<std::size_t>(required), L'\0');
    if (::MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            value.data(),
            static_cast<int>(value.size()),
            result.data(),
            required) != required) {
        native_error = static_cast<int>(::GetLastError());
        return {};
    }
    return result;
}

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

bool wait_for_terminated_process(
    const HANDLE process,
    BoundedProcessResult& result) {
    const DWORD wait_result =
        ::WaitForSingleObject(process, kTerminationWaitMilliseconds);
    if (wait_result == WAIT_OBJECT_0) {
        return true;
    }
    result.status = BoundedProcessStatus::launch_failed;
    result.error_code = "polyglot.process.tree_termination_failed";
    result.native_error = wait_result == WAIT_FAILED
        ? static_cast<int>(::GetLastError())
        : static_cast<int>(WAIT_TIMEOUT);
    return false;
}

BoundedProcessResult run_windows(const BoundedProcessRequest& request) {
    BoundedProcessResult result;
    const auto started_at = Clock::now();
    int conversion_error = 0;
    const std::wstring executable = utf8_to_wide(request.executable_path, conversion_error);
    const std::wstring working_directory =
        utf8_to_wide(request.working_directory, conversion_error);
    if (conversion_error != 0 || executable.empty() || working_directory.empty()) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.launch_failed";
        result.native_error = conversion_error == 0 ? ERROR_INVALID_PARAMETER : conversion_error;
        return result;
    }

    std::wstring command_line = quote_argument(executable);
    for (const auto& argument : request.arguments) {
        const std::wstring converted = utf8_to_wide(argument, conversion_error);
        if (conversion_error != 0) {
            result.status = BoundedProcessStatus::launch_failed;
            result.error_code = "polyglot.process.launch_failed";
            result.native_error = conversion_error;
            return result;
        }
        command_line.push_back(L' ');
        command_line += quote_argument(converted);
    }

    std::vector<std::pair<std::wstring, std::wstring>> environment_entries;
    environment_entries.reserve(request.environment.size());
    for (const auto& variable : request.environment) {
        std::wstring name = utf8_to_wide(variable.name, conversion_error);
        if (conversion_error != 0) {
            result.status = BoundedProcessStatus::launch_failed;
            result.error_code = "polyglot.process.environment_invalid";
            result.native_error = conversion_error;
            return result;
        }
        std::wstring value = utf8_to_wide(variable.value, conversion_error);
        if (conversion_error != 0 && !variable.value.empty()) {
            result.status = BoundedProcessStatus::launch_failed;
            result.error_code = "polyglot.process.environment_invalid";
            result.native_error = conversion_error;
            return result;
        }
        environment_entries.emplace_back(std::move(name), std::move(value));
    }
    std::sort(
        environment_entries.begin(),
        environment_entries.end(),
        [](const auto& left, const auto& right) {
            return std::lexicographical_compare(
                left.first.begin(), left.first.end(),
                right.first.begin(), right.first.end(),
                [](const wchar_t left_character, const wchar_t right_character) {
                    return ::towlower(left_character) < ::towlower(right_character);
                });
        });
    std::vector<wchar_t> environment_block;
    for (const auto& [name, value] : environment_entries) {
        environment_block.insert(environment_block.end(), name.begin(), name.end());
        environment_block.push_back(L'=');
        environment_block.insert(environment_block.end(), value.begin(), value.end());
        environment_block.push_back(L'\0');
    }
    environment_block.push_back(L'\0');
    if (environment_entries.empty()) {
        environment_block.push_back(L'\0');
    }

    const HANDLE job = ::CreateJobObjectW(nullptr, nullptr);
    if (job == nullptr) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.job_create_failed";
        result.native_error = static_cast<int>(::GetLastError());
        return result;
    }
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
    limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if (::SetInformationJobObject(
            job,
            JobObjectExtendedLimitInformation,
            &limits,
            sizeof(limits)) == FALSE) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.job_configure_failed";
        result.native_error = static_cast<int>(::GetLastError());
        (void)::CloseHandle(job);
        return result;
    }

    STARTUPINFOW startup_info{};
    startup_info.cb = sizeof(startup_info);
    PROCESS_INFORMATION process_info{};
    const BOOL created = ::CreateProcessW(
        executable.c_str(),
        command_line.data(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_NO_WINDOW | CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT,
        environment_block.data(),
        working_directory.c_str(),
        &startup_info,
        &process_info);
    if (created == FALSE) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.launch_failed";
        result.native_error = static_cast<int>(::GetLastError());
        (void)::CloseHandle(job);
        return result;
    }
    result.started = true;
    if (::AssignProcessToJobObject(job, process_info.hProcess) == FALSE) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.job_assign_failed";
        result.native_error = static_cast<int>(::GetLastError());
        if (::TerminateProcess(process_info.hProcess, 1U) == FALSE) {
            result.native_error = static_cast<int>(::GetLastError());
            result.error_code = "polyglot.process.tree_termination_failed";
        } else {
            (void)wait_for_terminated_process(process_info.hProcess, result);
        }
        (void)::CloseHandle(process_info.hThread);
        (void)::CloseHandle(process_info.hProcess);
        (void)::CloseHandle(job);
        result.process_tree_closed = true;
        result.elapsed_ms = elapsed_milliseconds(started_at);
        return result;
    }
    if (::ResumeThread(process_info.hThread) == static_cast<DWORD>(-1)) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.resume_failed";
        result.native_error = static_cast<int>(::GetLastError());
        if (::TerminateJobObject(job, 1U) == FALSE) {
            result.native_error = static_cast<int>(::GetLastError());
            result.error_code = "polyglot.process.tree_termination_failed";
        } else {
            (void)wait_for_terminated_process(process_info.hProcess, result);
        }
        (void)::CloseHandle(process_info.hThread);
        (void)::CloseHandle(process_info.hProcess);
        (void)::CloseHandle(job);
        result.process_tree_closed = true;
        result.elapsed_ms = elapsed_milliseconds(started_at);
        return result;
    }
    (void)::CloseHandle(process_info.hThread);

    const DWORD poll_ms = std::max<DWORD>(1U, request.poll_interval_ms);
    for (;;) {
        const DWORD wait_result = ::WaitForSingleObject(process_info.hProcess, poll_ms);
        if (wait_result == WAIT_OBJECT_0) {
            DWORD exit_code = 1U;
            if (::GetExitCodeProcess(process_info.hProcess, &exit_code) == FALSE) {
                result.status = BoundedProcessStatus::launch_failed;
                result.error_code = "polyglot.process.exit_query_failed";
                result.native_error = static_cast<int>(::GetLastError());
            } else {
                result.status = BoundedProcessStatus::exited;
                result.error_code = "polyglot.process.exited";
                result.exit_code = static_cast<int>(exit_code);
            }
            break;
        }
        if (wait_result == WAIT_FAILED) {
            result.status = BoundedProcessStatus::launch_failed;
            result.error_code = "polyglot.process.wait_failed";
            result.native_error = static_cast<int>(::GetLastError());
            (void)::TerminateJobObject(job, 1U);
            result.process_tree_closed = true;
            break;
        }
        if (cancellation_requested(request)) {
            result.status = BoundedProcessStatus::cancelled;
            result.error_code = "polyglot.process.cancelled";
            if (::TerminateJobObject(job, 1U) == FALSE) {
                result.status = BoundedProcessStatus::launch_failed;
                result.error_code = "polyglot.process.tree_termination_failed";
                result.native_error = static_cast<int>(::GetLastError());
            } else {
                (void)wait_for_terminated_process(process_info.hProcess, result);
            }
            result.process_tree_closed = true;
            break;
        }
        if (elapsed_milliseconds(started_at) >= request.timeout_ms) {
            result.status = BoundedProcessStatus::timed_out;
            result.error_code = "polyglot.process.timeout";
            if (::TerminateJobObject(job, 1U) == FALSE) {
                result.status = BoundedProcessStatus::launch_failed;
                result.error_code = "polyglot.process.tree_termination_failed";
                result.native_error = static_cast<int>(::GetLastError());
            } else {
                (void)wait_for_terminated_process(process_info.hProcess, result);
            }
            result.process_tree_closed = true;
            break;
        }
    }

    // KILL_ON_JOB_CLOSE also removes descendants after an otherwise successful
    // root exit; artifact invocations never authorize persistent child jobs.
    (void)::CloseHandle(job);
    result.process_tree_closed = true;
    (void)::CloseHandle(process_info.hProcess);
    result.elapsed_ms = elapsed_milliseconds(started_at);
    return result;
}

#else

bool write_child_error(const int descriptor, const int child_error) {
    const auto* bytes = reinterpret_cast<const std::uint8_t*>(&child_error);
    std::size_t written = 0U;
    while (written < sizeof(child_error)) {
        const ssize_t count =
            ::write(descriptor, bytes + written, sizeof(child_error) - written);
        if (count < 0 && errno == EINTR) {
            continue;
        }
        if (count <= 0) {
            return false;
        }
        written += static_cast<std::size_t>(count);
    }
    return true;
}

void terminate_process_group(const pid_t process_id) {
    if (process_id <= 0) {
        return;
    }
    (void)::kill(-process_id, SIGKILL);
    (void)::kill(process_id, SIGKILL);
}

BoundedProcessResult run_posix(const BoundedProcessRequest& request) {
    BoundedProcessResult result;
    const auto started_at = Clock::now();
    int launch_pipe[2]{-1, -1};
    if (::pipe(launch_pipe) != 0) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.launch_pipe_failed";
        result.native_error = errno;
        return result;
    }
    if (::fcntl(launch_pipe[1], F_SETFD, FD_CLOEXEC) == -1) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.launch_pipe_failed";
        result.native_error = errno;
        (void)::close(launch_pipe[0]);
        (void)::close(launch_pipe[1]);
        return result;
    }
    const int read_flags = ::fcntl(launch_pipe[0], F_GETFL, 0);
    if (read_flags == -1 ||
        ::fcntl(launch_pipe[0], F_SETFL, read_flags | O_NONBLOCK) == -1) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.launch_pipe_failed";
        result.native_error = errno;
        (void)::close(launch_pipe[0]);
        (void)::close(launch_pipe[1]);
        return result;
    }

    std::vector<std::string> argument_storage;
    argument_storage.reserve(request.arguments.size() + 1U);
    argument_storage.push_back(request.executable_path);
    argument_storage.insert(
        argument_storage.end(), request.arguments.begin(), request.arguments.end());
    std::vector<char*> argv;
    argv.reserve(argument_storage.size() + 1U);
    for (auto& argument : argument_storage) {
        argv.push_back(argument.data());
    }
    argv.push_back(nullptr);
    std::vector<std::string> environment_storage;
    environment_storage.reserve(request.environment.size());
    for (const auto& variable : request.environment) {
        environment_storage.push_back(variable.name + "=" + variable.value);
    }
    std::vector<char*> environment;
    environment.reserve(environment_storage.size() + 1U);
    for (auto& variable : environment_storage) {
        environment.push_back(variable.data());
    }
    environment.push_back(nullptr);

    const pid_t process_id = ::fork();
    if (process_id == 0) {
        (void)::close(launch_pipe[0]);
        if (::setpgid(0, 0) != 0 ||
            ::chdir(request.working_directory.c_str()) != 0) {
            const int child_error = errno;
            (void)write_child_error(launch_pipe[1], child_error);
            _exit(127);
        }
        ::execve(request.executable_path.c_str(), argv.data(), environment.data());
        const int child_error = errno;
        (void)write_child_error(launch_pipe[1], child_error);
        _exit(127);
    }
    (void)::close(launch_pipe[1]);
    if (process_id < 0) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.launch_failed";
        result.native_error = errno;
        (void)::close(launch_pipe[0]);
        return result;
    }
    if (::setpgid(process_id, process_id) != 0 && errno != EACCES && errno != ESRCH) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.group_create_failed";
        result.native_error = errno;
        terminate_process_group(process_id);
        int ignored_status = 0;
        (void)::waitpid(process_id, &ignored_status, 0);
        (void)::close(launch_pipe[0]);
        result.process_tree_closed = true;
        result.elapsed_ms = elapsed_milliseconds(started_at);
        return result;
    }

    int child_error = 0;
    auto* error_bytes = reinterpret_cast<std::uint8_t*>(&child_error);
    std::size_t error_count = 0U;
    int launch_read_error = 0;
    bool launch_complete = false;
    while (error_count < sizeof(child_error)) {
        const ssize_t count = ::read(
            launch_pipe[0], error_bytes + error_count, sizeof(child_error) - error_count);
        if (count < 0 && errno == EINTR) {
            continue;
        }
        if (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            launch_read_error = errno;
            break;
        }
        if (count == 0) {
            launch_complete = true;
            break;
        }
        if (count > 0) {
            error_count += static_cast<std::size_t>(count);
            continue;
        }
        if (cancellation_requested(request)) {
            result.status = BoundedProcessStatus::cancelled;
            result.error_code = "polyglot.process.cancelled";
            terminate_process_group(process_id);
            int ignored_status = 0;
            (void)::waitpid(process_id, &ignored_status, 0);
            result.process_tree_closed = true;
            result.elapsed_ms = elapsed_milliseconds(started_at);
            (void)::close(launch_pipe[0]);
            return result;
        }
        if (elapsed_milliseconds(started_at) >= request.timeout_ms) {
            result.status = BoundedProcessStatus::timed_out;
            result.error_code = "polyglot.process.timeout";
            terminate_process_group(process_id);
            int ignored_status = 0;
            (void)::waitpid(process_id, &ignored_status, 0);
            result.process_tree_closed = true;
            result.elapsed_ms = elapsed_milliseconds(started_at);
            (void)::close(launch_pipe[0]);
            return result;
        }
        std::this_thread::sleep_for(
            std::chrono::milliseconds(std::max<std::uint32_t>(1U, request.poll_interval_ms)));
    }
    (void)::close(launch_pipe[0]);
    if (!launch_complete || error_count != 0U || launch_read_error != 0) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.launch_failed";
        result.native_error = launch_read_error != 0
            ? launch_read_error
            : (error_count == sizeof(child_error) ? child_error : EIO);
        terminate_process_group(process_id);
        int ignored_status = 0;
        (void)::waitpid(process_id, &ignored_status, 0);
        result.process_tree_closed = true;
        result.elapsed_ms = elapsed_milliseconds(started_at);
        return result;
    }
    result.started = true;

    int status = 0;
    for (;;) {
        const pid_t waited = ::waitpid(process_id, &status, WNOHANG);
        if (waited == process_id) {
            result.status = BoundedProcessStatus::exited;
            result.error_code = "polyglot.process.exited";
            if (WIFEXITED(status)) {
                result.exit_code = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                result.exit_code = 128 + WTERMSIG(status);
            }
            break;
        }
        if (waited < 0 && errno != EINTR) {
            result.status = BoundedProcessStatus::launch_failed;
            result.error_code = "polyglot.process.wait_failed";
            result.native_error = errno;
            terminate_process_group(process_id);
            (void)::waitpid(process_id, &status, 0);
            break;
        }
        if (cancellation_requested(request)) {
            result.status = BoundedProcessStatus::cancelled;
            result.error_code = "polyglot.process.cancelled";
            terminate_process_group(process_id);
            (void)::waitpid(process_id, &status, 0);
            break;
        }
        if (elapsed_milliseconds(started_at) >= request.timeout_ms) {
            result.status = BoundedProcessStatus::timed_out;
            result.error_code = "polyglot.process.timeout";
            terminate_process_group(process_id);
            (void)::waitpid(process_id, &status, 0);
            break;
        }
        std::this_thread::sleep_for(
            std::chrono::milliseconds(std::max<std::uint32_t>(1U, request.poll_interval_ms)));
    }

    // Remove helpers even when the root returned normally. A migration
    // artifact is a bounded call, never a daemon-launch boundary.
    terminate_process_group(process_id);
    result.process_tree_closed = true;
    result.elapsed_ms = elapsed_milliseconds(started_at);
    return result;
}

#endif

}  // namespace

BoundedProcessResult run_bounded_process(const BoundedProcessRequest& request) {
    if (request.executable_path.empty() || request.working_directory.empty() ||
        contains_nul(request.executable_path) ||
        contains_nul(request.working_directory) ||
        !valid_arguments(request.arguments) ||
        request.timeout_ms == 0U || request.poll_interval_ms == 0U ||
        request.poll_interval_ms > request.timeout_ms ||
        !valid_environment(request.environment)) {
        return invalid_request();
    }
    std::error_code error;
    const std::filesystem::path executable =
        copperfin::platform::path_from_utf8_string(request.executable_path);
    const std::filesystem::path working_directory =
        copperfin::platform::path_from_utf8_string(request.working_directory);
    if (!executable.is_absolute() || !working_directory.is_absolute()) {
        return invalid_request();
    }
    if (!std::filesystem::is_regular_file(executable, error) || error) {
        BoundedProcessResult result;
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.executable_unavailable";
        return result;
    }
    error.clear();
    if (!std::filesystem::is_directory(working_directory, error) || error) {
        return invalid_request();
    }
    if (cancellation_requested(request)) {
        BoundedProcessResult result;
        result.status = BoundedProcessStatus::cancelled;
        result.error_code = "polyglot.process.cancelled";
        return result;
    }
#if defined(_WIN32)
    return run_windows(request);
#else
    return run_posix(request);
#endif
}

const char* bounded_process_status_name(const BoundedProcessStatus status) noexcept {
    switch (status) {
    case BoundedProcessStatus::exited:
        return "exited";
    case BoundedProcessStatus::cancelled:
        return "cancelled";
    case BoundedProcessStatus::timed_out:
        return "timed-out";
    case BoundedProcessStatus::invalid_request:
        return "invalid-request";
    case BoundedProcessStatus::launch_failed:
        return "launch-failed";
    }
    return "invalid-request";
}

}  // namespace copperfin::platform
