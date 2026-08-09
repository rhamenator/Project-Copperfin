// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/bounded_process.h"

#include "copperfin/platform/path.h"

#include <algorithm>
#include <atomic>
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
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace copperfin::platform {
namespace {

constexpr std::uint32_t kMaximumTransportBytes = 16U * 1024U * 1024U;

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

struct CapturedStreamState {
    std::atomic_bool limit_exceeded{false};
    std::atomic_bool stop_requested{false};
    std::atomic_int native_error{0};
};

struct InputStreamState {
    std::atomic_bool stop_requested{false};
    std::atomic_bool complete{false};
    std::atomic_int native_error{0};
};

void append_captured_bytes(
    std::string& output,
    const char* bytes,
    const std::size_t count,
    const std::uint32_t limit,
    CapturedStreamState& state) {
    const std::size_t remaining =
        static_cast<std::size_t>(limit) - output.size();
    const std::size_t accepted = std::min(count, remaining);
    output.append(bytes, accepted);
    if (accepted != count) {
        state.limit_exceeded.store(true, std::memory_order_release);
    }
}

bool apply_capture_failure(
    const CapturedStreamState& stdout_state,
    const CapturedStreamState& stderr_state,
    BoundedProcessResult& result) {
    if (stdout_state.limit_exceeded.load(std::memory_order_acquire)) {
        result.status = BoundedProcessStatus::output_limit_exceeded;
        result.error_code = "polyglot.process.stdout_limit_exceeded";
        return true;
    }
    if (stderr_state.limit_exceeded.load(std::memory_order_acquire)) {
        result.status = BoundedProcessStatus::output_limit_exceeded;
        result.error_code = "polyglot.process.stderr_limit_exceeded";
        return true;
    }
    const int stdout_error = stdout_state.native_error.load(std::memory_order_acquire);
    const int stderr_error = stderr_state.native_error.load(std::memory_order_acquire);
    if (stdout_error != 0 || stderr_error != 0) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.output_read_failed";
        result.native_error = stdout_error != 0 ? stdout_error : stderr_error;
        return true;
    }
    return false;
}

bool apply_input_failure(
    const InputStreamState& input_state,
    BoundedProcessResult& result,
    const bool require_complete = false) {
    const int input_error = input_state.native_error.load(std::memory_order_acquire);
    if (input_error == 0 &&
        (!require_complete ||
         input_state.complete.load(std::memory_order_acquire))) {
        return false;
    }
    result.status = BoundedProcessStatus::launch_failed;
    result.error_code = "polyglot.process.input_write_failed";
    result.native_error = input_error;
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

bool create_windows_capture_pipe(HANDLE& read_handle, HANDLE& write_handle) {
    SECURITY_ATTRIBUTES attributes{};
    attributes.nLength = sizeof(attributes);
    attributes.bInheritHandle = TRUE;
    if (::CreatePipe(&read_handle, &write_handle, &attributes, 0U) == FALSE) {
        return false;
    }
    if (::SetHandleInformation(read_handle, HANDLE_FLAG_INHERIT, 0U) == FALSE) {
        const DWORD error = ::GetLastError();
        (void)::CloseHandle(read_handle);
        (void)::CloseHandle(write_handle);
        read_handle = nullptr;
        write_handle = nullptr;
        ::SetLastError(error);
        return false;
    }
    return true;
}

bool create_windows_input_pipe(HANDLE& read_handle, HANDLE& write_handle) {
    SECURITY_ATTRIBUTES attributes{};
    attributes.nLength = sizeof(attributes);
    attributes.bInheritHandle = TRUE;
    if (::CreatePipe(&read_handle, &write_handle, &attributes, 0U) == FALSE) {
        return false;
    }
    if (::SetHandleInformation(write_handle, HANDLE_FLAG_INHERIT, 0U) == FALSE) {
        const DWORD error = ::GetLastError();
        (void)::CloseHandle(read_handle);
        (void)::CloseHandle(write_handle);
        read_handle = nullptr;
        write_handle = nullptr;
        ::SetLastError(error);
        return false;
    }
    return true;
}

void read_windows_capture_pipe(
    const HANDLE read_handle,
    std::string& output,
    const std::uint32_t limit,
    CapturedStreamState& state) {
    char buffer[8192];
    try {
        for (;;) {
            DWORD read = 0U;
            if (::ReadFile(
                    read_handle, buffer, static_cast<DWORD>(sizeof(buffer)),
                    &read, nullptr) == FALSE) {
                const DWORD error = ::GetLastError();
                if (error != ERROR_BROKEN_PIPE) {
                    state.native_error.store(
                        static_cast<int>(error), std::memory_order_release);
                }
                break;
            }
            if (read == 0U) {
                break;
            }
            append_captured_bytes(
                output, buffer, static_cast<std::size_t>(read), limit, state);
            if (state.limit_exceeded.load(std::memory_order_acquire)) {
                break;
            }
        }
    } catch (...) {
        state.native_error.store(
            static_cast<int>(ERROR_NOT_ENOUGH_MEMORY), std::memory_order_release);
    }
    (void)::CloseHandle(read_handle);
}

void write_windows_input_pipe(
    const HANDLE write_handle,
    const std::string_view input,
    InputStreamState& state) noexcept {
    std::size_t written = 0U;
    while (written < input.size()) {
        if (state.stop_requested.load(std::memory_order_acquire)) {
            break;
        }
        const DWORD requested = static_cast<DWORD>(std::min<std::size_t>(
            8192U, input.size() - written));
        DWORD count = 0U;
        if (::WriteFile(
                write_handle, input.data() + written, requested,
                &count, nullptr) == FALSE) {
            const DWORD error = ::GetLastError();
            if (!state.stop_requested.load(std::memory_order_acquire)) {
                state.native_error.store(
                    static_cast<int>(error == ERROR_NO_DATA ? ERROR_BROKEN_PIPE : error),
                    std::memory_order_release);
            }
            break;
        }
        if (count == 0U) {
            state.native_error.store(
                static_cast<int>(ERROR_WRITE_FAULT), std::memory_order_release);
            break;
        }
        written += static_cast<std::size_t>(count);
    }
    if (written == input.size() &&
        state.native_error.load(std::memory_order_acquire) == 0) {
        state.complete.store(true, std::memory_order_release);
    }
    (void)::CloseHandle(write_handle);
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

    HANDLE stdout_read = nullptr;
    HANDLE stdout_write = nullptr;
    HANDLE stderr_read = nullptr;
    HANDLE stderr_write = nullptr;
    HANDLE stdin_read = nullptr;
    HANDLE stdin_write = nullptr;
    if (!create_windows_capture_pipe(stdout_read, stdout_write) ||
        !create_windows_capture_pipe(stderr_read, stderr_write) ||
        !create_windows_input_pipe(stdin_read, stdin_write)) {
        const int pipe_error = static_cast<int>(::GetLastError());
        if (stdout_read != nullptr) {
            (void)::CloseHandle(stdout_read);
            (void)::CloseHandle(stdout_write);
        }
        if (stderr_read != nullptr) {
            (void)::CloseHandle(stderr_read);
            (void)::CloseHandle(stderr_write);
        }
        if (stdin_read != nullptr) {
            (void)::CloseHandle(stdin_read);
            (void)::CloseHandle(stdin_write);
        }
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.transport_pipe_failed";
        result.native_error = pipe_error;
        return result;
    }

    const HANDLE job = ::CreateJobObjectW(nullptr, nullptr);
    if (job == nullptr) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.job_create_failed";
        result.native_error = static_cast<int>(::GetLastError());
        (void)::CloseHandle(stdin_read);
        (void)::CloseHandle(stdin_write);
        (void)::CloseHandle(stdout_read);
        (void)::CloseHandle(stdout_write);
        (void)::CloseHandle(stderr_read);
        (void)::CloseHandle(stderr_write);
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
        (void)::CloseHandle(stdin_read);
        (void)::CloseHandle(stdin_write);
        (void)::CloseHandle(stdout_read);
        (void)::CloseHandle(stdout_write);
        (void)::CloseHandle(stderr_read);
        (void)::CloseHandle(stderr_write);
        return result;
    }

    STARTUPINFOEXW startup_info{};
    startup_info.StartupInfo.cb = sizeof(startup_info);
    startup_info.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
    startup_info.StartupInfo.hStdInput = stdin_read;
    startup_info.StartupInfo.hStdOutput = stdout_write;
    startup_info.StartupInfo.hStdError = stderr_write;
    SIZE_T attribute_bytes = 0U;
    const BOOL attribute_sizing =
        ::InitializeProcThreadAttributeList(nullptr, 1U, 0U, &attribute_bytes);
    const DWORD attribute_sizing_error = ::GetLastError();
    if (attribute_sizing != FALSE ||
        attribute_sizing_error != ERROR_INSUFFICIENT_BUFFER || attribute_bytes == 0U) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.output_pipe_failed";
        result.native_error = static_cast<int>(
            attribute_sizing_error == ERROR_SUCCESS
                ? ERROR_INVALID_PARAMETER
                : attribute_sizing_error);
        (void)::CloseHandle(job);
        (void)::CloseHandle(stdin_read);
        (void)::CloseHandle(stdin_write);
        (void)::CloseHandle(stdout_read);
        (void)::CloseHandle(stdout_write);
        (void)::CloseHandle(stderr_read);
        (void)::CloseHandle(stderr_write);
        return result;
    }
    std::vector<std::uint8_t> attribute_storage(attribute_bytes);
    startup_info.lpAttributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(
        attribute_storage.data());
    HANDLE inherited_handles[]{stdin_read, stdout_write, stderr_write};
    if (::InitializeProcThreadAttributeList(
            startup_info.lpAttributeList, 1U, 0U, &attribute_bytes) == FALSE ||
        ::UpdateProcThreadAttribute(
            startup_info.lpAttributeList, 0U, PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
            inherited_handles, sizeof(inherited_handles), nullptr, nullptr) == FALSE) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.output_pipe_failed";
        result.native_error = static_cast<int>(::GetLastError());
        if (startup_info.lpAttributeList != nullptr) {
            ::DeleteProcThreadAttributeList(startup_info.lpAttributeList);
        }
        (void)::CloseHandle(job);
        (void)::CloseHandle(stdin_read);
        (void)::CloseHandle(stdin_write);
        (void)::CloseHandle(stdout_read);
        (void)::CloseHandle(stdout_write);
        (void)::CloseHandle(stderr_read);
        (void)::CloseHandle(stderr_write);
        return result;
    }
    PROCESS_INFORMATION process_info{};
    const BOOL created = ::CreateProcessW(
        executable.c_str(),
        command_line.data(),
        nullptr,
        nullptr,
        TRUE,
        CREATE_NO_WINDOW | CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT |
            EXTENDED_STARTUPINFO_PRESENT,
        environment_block.data(),
        working_directory.c_str(),
        &startup_info.StartupInfo,
        &process_info);
    const DWORD create_error = created == FALSE ? ::GetLastError() : ERROR_SUCCESS;
    ::DeleteProcThreadAttributeList(startup_info.lpAttributeList);
    (void)::CloseHandle(stdin_read);
    (void)::CloseHandle(stdout_write);
    (void)::CloseHandle(stderr_write);
    if (created == FALSE) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.launch_failed";
        result.native_error = static_cast<int>(create_error);
        (void)::CloseHandle(job);
        (void)::CloseHandle(stdin_write);
        (void)::CloseHandle(stdout_read);
        (void)::CloseHandle(stderr_read);
        return result;
    }
    result.started = true;
    CapturedStreamState stdout_state;
    CapturedStreamState stderr_state;
    InputStreamState input_state;
    std::thread stdout_thread;
    std::thread stderr_thread;
    std::thread input_thread;
    const auto finish_transport = [&]() {
        input_state.stop_requested.store(true, std::memory_order_release);
        if (input_thread.joinable() &&
            !input_state.complete.load(std::memory_order_acquire)) {
            const HANDLE thread_handle = input_thread.native_handle();
            if (::CancelSynchronousIo(thread_handle) == FALSE) {
                const DWORD cancellation_error = ::GetLastError();
                if (cancellation_error != ERROR_NOT_FOUND) {
                    int expected = 0;
                    (void)input_state.native_error.compare_exchange_strong(
                        expected, static_cast<int>(cancellation_error),
                        std::memory_order_acq_rel);
                }
            }
        }
        if (input_thread.joinable()) {
            input_thread.join();
        }
        if (stdout_thread.joinable()) {
            stdout_thread.join();
        }
        if (stderr_thread.joinable()) {
            stderr_thread.join();
        }
    };
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
        (void)::CloseHandle(stdin_write);
        (void)::CloseHandle(stdout_read);
        (void)::CloseHandle(stderr_read);
        result.process_tree_closed = true;
        result.elapsed_ms = elapsed_milliseconds(started_at);
        return result;
    }
    try {
        stdout_thread = std::thread(
            read_windows_capture_pipe, stdout_read, std::ref(result.standard_output),
            request.stdout_limit_bytes, std::ref(stdout_state));
        stderr_thread = std::thread(
            read_windows_capture_pipe, stderr_read, std::ref(result.standard_error),
            request.stderr_limit_bytes, std::ref(stderr_state));
        input_thread = std::thread(
            write_windows_input_pipe, stdin_write,
            std::string_view{request.standard_input}, std::ref(input_state));
    } catch (...) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = stdout_thread.joinable() && stderr_thread.joinable()
            ? "polyglot.process.input_writer_create_failed"
            : "polyglot.process.output_reader_create_failed";
        if (::TerminateJobObject(job, 1U) == FALSE &&
            ::TerminateProcess(process_info.hProcess, 1U) == FALSE) {
            result.error_code = "polyglot.process.tree_termination_failed";
            result.native_error = static_cast<int>(::GetLastError());
        }
        (void)::CloseHandle(job);
        (void)wait_for_terminated_process(process_info.hProcess, result);
        if (!stdout_thread.joinable()) {
            (void)::CloseHandle(stdout_read);
        }
        if (!stderr_thread.joinable()) {
            (void)::CloseHandle(stderr_read);
        }
        if (!input_thread.joinable()) {
            (void)::CloseHandle(stdin_write);
        }
        finish_transport();
        (void)::CloseHandle(process_info.hThread);
        (void)::CloseHandle(process_info.hProcess);
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
        finish_transport();
        (void)apply_capture_failure(stdout_state, stderr_state, result);
        (void)apply_input_failure(input_state, result);
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
        if (apply_capture_failure(stdout_state, stderr_state, result)) {
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
        if (apply_input_failure(input_state, result)) {
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
    finish_transport();
    if (result.status == BoundedProcessStatus::exited) {
        if (!apply_capture_failure(stdout_state, stderr_state, result)) {
            (void)apply_input_failure(input_state, result, true);
        }
    }
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

bool create_posix_capture_pipe(int descriptors[2]) {
    if (::pipe(descriptors) != 0) {
        return false;
    }
    const int read_flags = ::fcntl(descriptors[0], F_GETFL, 0);
    const int write_flags = ::fcntl(descriptors[1], F_GETFD, 0);
    if (read_flags == -1 || write_flags == -1 ||
        ::fcntl(descriptors[0], F_SETFL, read_flags | O_NONBLOCK) == -1 ||
        ::fcntl(descriptors[1], F_SETFD, write_flags | FD_CLOEXEC) == -1) {
        const int pipe_error = errno;
        (void)::close(descriptors[0]);
        (void)::close(descriptors[1]);
        descriptors[0] = -1;
        descriptors[1] = -1;
        errno = pipe_error;
        return false;
    }
    return true;
}

bool create_posix_input_pipe(int descriptors[2]) {
    if (::pipe(descriptors) != 0) {
        return false;
    }
    const int read_descriptor_flags = ::fcntl(descriptors[0], F_GETFD, 0);
    const int write_status_flags = ::fcntl(descriptors[1], F_GETFL, 0);
    const int write_descriptor_flags = ::fcntl(descriptors[1], F_GETFD, 0);
    if (read_descriptor_flags == -1 || write_status_flags == -1 ||
        write_descriptor_flags == -1 ||
        ::fcntl(
            descriptors[0], F_SETFD,
            read_descriptor_flags | FD_CLOEXEC) == -1 ||
        ::fcntl(
            descriptors[1], F_SETFL,
            write_status_flags | O_NONBLOCK) == -1 ||
        ::fcntl(
            descriptors[1], F_SETFD,
            write_descriptor_flags | FD_CLOEXEC) == -1) {
        const int pipe_error = errno;
        (void)::close(descriptors[0]);
        (void)::close(descriptors[1]);
        descriptors[0] = -1;
        descriptors[1] = -1;
        errno = pipe_error;
        return false;
    }
    return true;
}

void read_posix_capture_pipe(
    const int descriptor,
    std::string& output,
    const std::uint32_t limit,
    CapturedStreamState& state) {
    char buffer[8192];
    try {
        for (;;) {
            const ssize_t count = ::read(descriptor, buffer, sizeof(buffer));
            if (count < 0 && errno == EINTR) {
                continue;
            }
            if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                if (state.stop_requested.load(std::memory_order_acquire)) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1U));
                continue;
            }
            if (count < 0) {
                state.native_error.store(errno, std::memory_order_release);
                break;
            }
            if (count == 0) {
                break;
            }
            append_captured_bytes(
                output, buffer, static_cast<std::size_t>(count), limit, state);
            if (state.limit_exceeded.load(std::memory_order_acquire)) {
                break;
            }
        }
    } catch (...) {
        state.native_error.store(ENOMEM, std::memory_order_release);
    }
    (void)::close(descriptor);
}

void write_posix_input_pipe(
    const int descriptor,
    const std::string_view input,
    InputStreamState& state) noexcept {
    sigset_t blocked_signals;
    (void)sigemptyset(&blocked_signals);
    (void)sigaddset(&blocked_signals, SIGPIPE);
    sigset_t original_mask;
    const int signal_error =
        ::pthread_sigmask(SIG_BLOCK, &blocked_signals, &original_mask);
    if (signal_error != 0) {
        state.native_error.store(signal_error, std::memory_order_release);
        (void)::close(descriptor);
        return;
    }
    sigset_t pending_before;
    if (sigpending(&pending_before) != 0) {
        state.native_error.store(errno, std::memory_order_release);
        (void)::pthread_sigmask(SIG_SETMASK, &original_mask, nullptr);
        (void)::close(descriptor);
        return;
    }
    const int sigpipe_pending_before = sigismember(&pending_before, SIGPIPE);
    if (sigpipe_pending_before < 0) {
        state.native_error.store(errno, std::memory_order_release);
        (void)::pthread_sigmask(SIG_SETMASK, &original_mask, nullptr);
        (void)::close(descriptor);
        return;
    }

    std::size_t written = 0U;
    while (written < input.size()) {
        if (state.stop_requested.load(std::memory_order_acquire)) {
            break;
        }
        const std::size_t requested = std::min<std::size_t>(
            8192U, input.size() - written);
        const ssize_t count = ::write(descriptor, input.data() + written, requested);
        if (count < 0 && errno == EINTR) {
            continue;
        }
        if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1U));
            continue;
        }
        if (count < 0) {
            const int write_error = errno;
            // A blocked SIGPIPE can remain pending when this worker exits on
            // macOS. Consume only the signal generated by this EPIPE write.
            if (write_error == EPIPE && sigpipe_pending_before == 0) {
                sigset_t pending_after;
                if (sigpending(&pending_after) == 0 &&
                    sigismember(&pending_after, SIGPIPE) == 1) {
                    int consumed_signal = 0;
                    (void)sigwait(&blocked_signals, &consumed_signal);
                }
            }
            if (!state.stop_requested.load(std::memory_order_acquire)) {
                state.native_error.store(write_error, std::memory_order_release);
            }
            break;
        }
        if (count == 0) {
            state.native_error.store(EIO, std::memory_order_release);
            break;
        }
        written += static_cast<std::size_t>(count);
    }
    const int restore_error =
        ::pthread_sigmask(SIG_SETMASK, &original_mask, nullptr);
    if (restore_error != 0) {
        int expected = 0;
        (void)state.native_error.compare_exchange_strong(
            expected, restore_error, std::memory_order_acq_rel);
    }
    if (written == input.size() &&
        state.native_error.load(std::memory_order_acquire) == 0) {
        state.complete.store(true, std::memory_order_release);
    }
    (void)::close(descriptor);
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
    int stdout_pipe[2]{-1, -1};
    int stderr_pipe[2]{-1, -1};
    int stdin_pipe[2]{-1, -1};
    if (!create_posix_capture_pipe(stdout_pipe) ||
        !create_posix_capture_pipe(stderr_pipe) ||
        !create_posix_input_pipe(stdin_pipe)) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.transport_pipe_failed";
        result.native_error = errno;
        (void)::close(launch_pipe[0]);
        (void)::close(launch_pipe[1]);
        if (stdout_pipe[0] != -1) {
            (void)::close(stdout_pipe[0]);
            (void)::close(stdout_pipe[1]);
        }
        if (stderr_pipe[0] != -1) {
            (void)::close(stderr_pipe[0]);
            (void)::close(stderr_pipe[1]);
        }
        if (stdin_pipe[0] != -1) {
            (void)::close(stdin_pipe[0]);
            (void)::close(stdin_pipe[1]);
        }
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
        (void)::close(stdout_pipe[0]);
        (void)::close(stdout_pipe[1]);
        (void)::close(stderr_pipe[0]);
        (void)::close(stderr_pipe[1]);
        (void)::close(stdin_pipe[0]);
        (void)::close(stdin_pipe[1]);
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
        (void)::close(stdout_pipe[0]);
        (void)::close(stderr_pipe[0]);
        (void)::close(stdin_pipe[1]);
        if (::dup2(stdin_pipe[0], STDIN_FILENO) == -1 ||
            ::dup2(stdout_pipe[1], STDOUT_FILENO) == -1 ||
            ::dup2(stderr_pipe[1], STDERR_FILENO) == -1 ||
            ::setpgid(0, 0) != 0 ||
            ::chdir(request.working_directory.c_str()) != 0) {
            const int child_error = errno;
            (void)write_child_error(launch_pipe[1], child_error);
            _exit(127);
        }
        (void)::close(stdin_pipe[0]);
        (void)::close(stdout_pipe[1]);
        (void)::close(stderr_pipe[1]);
        ::execve(request.executable_path.c_str(), argv.data(), environment.data());
        const int child_error = errno;
        (void)write_child_error(launch_pipe[1], child_error);
        _exit(127);
    }
    (void)::close(launch_pipe[1]);
    (void)::close(stdout_pipe[1]);
    (void)::close(stderr_pipe[1]);
    (void)::close(stdin_pipe[0]);
    if (process_id < 0) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.launch_failed";
        result.native_error = errno;
        (void)::close(launch_pipe[0]);
        (void)::close(stdout_pipe[0]);
        (void)::close(stderr_pipe[0]);
        (void)::close(stdin_pipe[1]);
        return result;
    }
    CapturedStreamState stdout_state;
    CapturedStreamState stderr_state;
    InputStreamState input_state;
    std::thread stdout_thread;
    std::thread stderr_thread;
    std::thread input_thread;
    const auto finish_transport = [&]() {
        input_state.stop_requested.store(true, std::memory_order_release);
        stdout_state.stop_requested.store(true, std::memory_order_release);
        stderr_state.stop_requested.store(true, std::memory_order_release);
        if (input_thread.joinable()) {
            input_thread.join();
        }
        if (stdout_thread.joinable()) {
            stdout_thread.join();
        }
        if (stderr_thread.joinable()) {
            stderr_thread.join();
        }
    };
    if (::setpgid(process_id, process_id) != 0 && errno != EACCES && errno != ESRCH) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = "polyglot.process.group_create_failed";
        result.native_error = errno;
        terminate_process_group(process_id);
        int ignored_status = 0;
        (void)::waitpid(process_id, &ignored_status, 0);
        (void)::close(launch_pipe[0]);
        (void)::close(stdout_pipe[0]);
        (void)::close(stderr_pipe[0]);
        (void)::close(stdin_pipe[1]);
        result.process_tree_closed = true;
        result.elapsed_ms = elapsed_milliseconds(started_at);
        return result;
    }
    try {
        stdout_thread = std::thread(
            read_posix_capture_pipe, stdout_pipe[0], std::ref(result.standard_output),
            request.stdout_limit_bytes, std::ref(stdout_state));
        stderr_thread = std::thread(
            read_posix_capture_pipe, stderr_pipe[0], std::ref(result.standard_error),
            request.stderr_limit_bytes, std::ref(stderr_state));
        input_thread = std::thread(
            write_posix_input_pipe, stdin_pipe[1],
            std::string_view{request.standard_input}, std::ref(input_state));
    } catch (...) {
        result.status = BoundedProcessStatus::launch_failed;
        result.error_code = stdout_thread.joinable() && stderr_thread.joinable()
            ? "polyglot.process.input_writer_create_failed"
            : "polyglot.process.output_reader_create_failed";
        terminate_process_group(process_id);
        int ignored_status = 0;
        (void)::waitpid(process_id, &ignored_status, 0);
        (void)::close(launch_pipe[0]);
        if (!stdout_thread.joinable()) {
            (void)::close(stdout_pipe[0]);
        }
        if (!stderr_thread.joinable()) {
            (void)::close(stderr_pipe[0]);
        }
        if (!input_thread.joinable()) {
            (void)::close(stdin_pipe[1]);
        }
        finish_transport();
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
            finish_transport();
            return result;
        }
        if (apply_capture_failure(stdout_state, stderr_state, result)) {
            terminate_process_group(process_id);
            int ignored_status = 0;
            (void)::waitpid(process_id, &ignored_status, 0);
            result.process_tree_closed = true;
            result.elapsed_ms = elapsed_milliseconds(started_at);
            (void)::close(launch_pipe[0]);
            finish_transport();
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
            finish_transport();
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
        finish_transport();
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
        if (apply_capture_failure(stdout_state, stderr_state, result)) {
            terminate_process_group(process_id);
            (void)::waitpid(process_id, &status, 0);
            break;
        }
        if (apply_input_failure(input_state, result)) {
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
    finish_transport();
    if (result.status == BoundedProcessStatus::exited) {
        if (!apply_capture_failure(stdout_state, stderr_state, result)) {
            (void)apply_input_failure(input_state, result, true);
        }
    }
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
        request.stdin_limit_bytes == 0U ||
        request.stdout_limit_bytes == 0U || request.stderr_limit_bytes == 0U ||
        request.standard_input.size() > request.stdin_limit_bytes ||
        request.stdin_limit_bytes > kMaximumTransportBytes ||
        request.stdout_limit_bytes > kMaximumTransportBytes ||
        request.stderr_limit_bytes > kMaximumTransportBytes ||
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
    case BoundedProcessStatus::output_limit_exceeded:
        return "output-limit-exceeded";
    case BoundedProcessStatus::invalid_request:
        return "invalid-request";
    case BoundedProcessStatus::launch_failed:
        return "launch-failed";
    }
    return "invalid-request";
}

}  // namespace copperfin::platform
