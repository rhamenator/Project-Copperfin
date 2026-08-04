// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_process_capture_support.h"

#include <atomic>
#include <cerrno>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <limits>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace copperfin::test_support {
namespace {

std::atomic_uint capture_sequence{0U};

std::string read_binary_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

struct CapturePaths {
    std::filesystem::path stdout_path;
    std::filesystem::path stderr_path;
};

CapturePaths make_capture_paths(const std::filesystem::path& working_directory) {
#if defined(_WIN32)
    const unsigned long process_id = static_cast<unsigned long>(::GetCurrentProcessId());
#else
    const unsigned long process_id = static_cast<unsigned long>(::getpid());
#endif
    const std::string stem = ".copperfin-process-" + std::to_string(process_id) + "-" +
        std::to_string(capture_sequence.fetch_add(1U));
    return {
        .stdout_path = working_directory / (stem + ".stdout"),
        .stderr_path = working_directory / (stem + ".stderr")
    };
}

void collect_and_remove_capture_files(
    const CapturePaths& paths,
    CapturedProcessResult& result) {
    std::error_code error;
    if (std::filesystem::exists(paths.stdout_path, error) && !error) {
        result.stdout_text = read_binary_file(paths.stdout_path);
    }
    error.clear();
    if (std::filesystem::exists(paths.stderr_path, error) && !error) {
        result.stderr_text = read_binary_file(paths.stderr_path);
    }
    error.clear();
    std::filesystem::remove(paths.stdout_path, error);
    error.clear();
    std::filesystem::remove(paths.stderr_path, error);
}

#if defined(_WIN32)

std::wstring utf8_to_wide(const std::string& value, int& error) {
    error = 0;
    if (value.empty()) {
        return {};
    }
    if (value.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        error = ERROR_INVALID_PARAMETER;
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
        error = static_cast<int>(::GetLastError());
        return {};
    }
    std::wstring converted(static_cast<std::size_t>(required), L'\0');
    if (::MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            value.data(),
            static_cast<int>(value.size()),
            converted.data(),
            required) != required) {
        error = static_cast<int>(::GetLastError());
        return {};
    }
    return converted;
}

std::wstring quote_windows_process_argument(const std::wstring& value) {
    std::wstring quoted(1U, L'"');
    std::size_t backslashes = 0U;
    for (const wchar_t ch : value) {
        if (ch == L'\\') {
            ++backslashes;
            continue;
        }
        if (ch == L'"') {
            quoted.append(backslashes * 2U + 1U, L'\\');
            quoted.push_back(L'"');
            backslashes = 0U;
            continue;
        }
        quoted.append(backslashes, L'\\');
        backslashes = 0U;
        quoted.push_back(ch);
    }
    quoted.append(backslashes * 2U, L'\\');
    quoted.push_back(L'"');
    return quoted;
}

CapturedProcessResult run_process_capture_windows(
    const std::filesystem::path& executable_path,
    const std::vector<std::string>& utf8_arguments,
    const std::filesystem::path& working_directory,
    const CapturePaths& paths) {
    CapturedProcessResult result;
    int conversion_error = 0;
    std::wstring command_line = quote_windows_process_argument(executable_path.native());
    for (const auto& argument : utf8_arguments) {
        const std::wstring wide_argument = utf8_to_wide(argument, conversion_error);
        if (conversion_error != 0) {
            result.launch_error = conversion_error;
            return result;
        }
        command_line.push_back(L' ');
        command_line += quote_windows_process_argument(wide_argument);
    }

    SECURITY_ATTRIBUTES security_attributes{};
    security_attributes.nLength = sizeof(security_attributes);
    security_attributes.bInheritHandle = TRUE;
    const HANDLE stdout_handle = ::CreateFileW(
        paths.stdout_path.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        &security_attributes,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    const HANDLE stderr_handle = ::CreateFileW(
        paths.stderr_path.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        &security_attributes,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    const HANDLE stdin_handle = ::CreateFileW(
        L"NUL",
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        &security_attributes,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (stdout_handle == INVALID_HANDLE_VALUE ||
        stderr_handle == INVALID_HANDLE_VALUE ||
        stdin_handle == INVALID_HANDLE_VALUE) {
        result.launch_error = static_cast<int>(::GetLastError());
        if (stdout_handle != INVALID_HANDLE_VALUE) {
            (void)::CloseHandle(stdout_handle);
        }
        if (stderr_handle != INVALID_HANDLE_VALUE) {
            (void)::CloseHandle(stderr_handle);
        }
        if (stdin_handle != INVALID_HANDLE_VALUE) {
            (void)::CloseHandle(stdin_handle);
        }
        return result;
    }

    STARTUPINFOW startup_info{};
    startup_info.cb = sizeof(startup_info);
    startup_info.dwFlags = STARTF_USESTDHANDLES;
    startup_info.hStdInput = stdin_handle;
    startup_info.hStdOutput = stdout_handle;
    startup_info.hStdError = stderr_handle;
    PROCESS_INFORMATION process_info{};
    const BOOL started = ::CreateProcessW(
        executable_path.c_str(),
        command_line.data(),
        nullptr,
        nullptr,
        TRUE,
        CREATE_NO_WINDOW,
        nullptr,
        working_directory.c_str(),
        &startup_info,
        &process_info);
    result.launch_error = started == FALSE ? static_cast<int>(::GetLastError()) : 0;
    (void)::CloseHandle(stdin_handle);
    (void)::CloseHandle(stdout_handle);
    (void)::CloseHandle(stderr_handle);
    if (started == FALSE) {
        return result;
    }

    result.started = true;
    (void)::CloseHandle(process_info.hThread);
    const DWORD wait_result = ::WaitForSingleObject(process_info.hProcess, INFINITE);
    DWORD exit_code = 1U;
    if (wait_result != WAIT_OBJECT_0 ||
        ::GetExitCodeProcess(process_info.hProcess, &exit_code) == FALSE) {
        result.launch_error = static_cast<int>(::GetLastError());
        result.exit_code = -1;
    } else {
        result.exit_code = static_cast<int>(exit_code);
    }
    (void)::CloseHandle(process_info.hProcess);
    return result;
}

#else

bool write_launch_error(const int descriptor, const int error) {
    const auto* bytes = reinterpret_cast<const std::uint8_t*>(&error);
    std::size_t written = 0U;
    while (written < sizeof(error)) {
        const ssize_t count = ::write(descriptor, bytes + written, sizeof(error) - written);
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

CapturedProcessResult run_process_capture_posix(
    const std::filesystem::path& executable_path,
    const std::vector<std::string>& arguments,
    const std::filesystem::path& working_directory,
    const CapturePaths& paths) {
    CapturedProcessResult result;
    int launch_pipe[2]{-1, -1};
    if (::pipe(launch_pipe) != 0) {
        result.launch_error = errno;
        return result;
    }
    if (::fcntl(launch_pipe[1], F_SETFD, FD_CLOEXEC) == -1) {
        result.launch_error = errno;
        (void)::close(launch_pipe[0]);
        (void)::close(launch_pipe[1]);
        return result;
    }

    const std::string executable_bytes = executable_path.string();
    std::vector<std::string> argument_storage;
    argument_storage.reserve(arguments.size() + 1U);
    argument_storage.push_back(executable_bytes);
    argument_storage.insert(argument_storage.end(), arguments.begin(), arguments.end());
    std::vector<char*> argv;
    argv.reserve(argument_storage.size() + 1U);
    for (auto& argument : argument_storage) {
        argv.push_back(argument.data());
    }
    argv.push_back(nullptr);

    const pid_t process_id = ::fork();
    if (process_id == 0) {
        (void)::close(launch_pipe[0]);
        const int stdout_descriptor = ::open(
            paths.stdout_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
        const int stderr_descriptor = ::open(
            paths.stderr_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (stdout_descriptor < 0 || stderr_descriptor < 0 ||
            ::dup2(stdout_descriptor, STDOUT_FILENO) < 0 ||
            ::dup2(stderr_descriptor, STDERR_FILENO) < 0 ||
            ::chdir(working_directory.c_str()) != 0) {
            const int child_error = errno;
            (void)write_launch_error(launch_pipe[1], child_error);
            _exit(127);
        }
        (void)::close(stdout_descriptor);
        (void)::close(stderr_descriptor);
        ::execv(executable_bytes.c_str(), argv.data());
        const int child_error = errno;
        (void)write_launch_error(launch_pipe[1], child_error);
        _exit(127);
    }
    (void)::close(launch_pipe[1]);
    if (process_id < 0) {
        result.launch_error = errno;
        (void)::close(launch_pipe[0]);
        return result;
    }

    int child_error = 0;
    auto* error_buffer = reinterpret_cast<std::uint8_t*>(&child_error);
    std::size_t error_bytes = 0U;
    int pipe_error = 0;
    while (error_bytes < sizeof(child_error)) {
        const ssize_t count = ::read(
            launch_pipe[0],
            error_buffer + error_bytes,
            sizeof(child_error) - error_bytes);
        if (count < 0 && errno == EINTR) {
            continue;
        }
        if (count < 0) {
            pipe_error = errno;
            break;
        }
        if (count == 0) {
            break;
        }
        error_bytes += static_cast<std::size_t>(count);
    }
    (void)::close(launch_pipe[0]);
    result.started = error_bytes == 0U && pipe_error == 0;
    if (result.started) {
        result.launch_error = 0;
    } else if (error_bytes == sizeof(child_error)) {
        result.launch_error = child_error;
    } else {
        result.launch_error = pipe_error != 0 ? pipe_error : EIO;
    }

    int status = 0;
    pid_t waited = -1;
    do {
        waited = ::waitpid(process_id, &status, 0);
    } while (waited < 0 && errno == EINTR);
    if (result.started && waited == process_id && WIFEXITED(status)) {
        result.exit_code = WEXITSTATUS(status);
    } else if (result.started && waited == process_id && WIFSIGNALED(status)) {
        result.exit_code = 128 + WTERMSIG(status);
    }
    return result;
}

#endif

}  // namespace

CapturedProcessResult run_process_capture(
    const std::filesystem::path& executable_path,
    const std::vector<std::string>& utf8_arguments,
    const std::filesystem::path& working_directory) {
    const std::filesystem::path absolute_executable = std::filesystem::absolute(executable_path);
    const std::filesystem::path absolute_working_directory =
        std::filesystem::absolute(working_directory);
    const CapturePaths paths = make_capture_paths(absolute_working_directory);
#if defined(_WIN32)
    CapturedProcessResult result = run_process_capture_windows(
        absolute_executable,
        utf8_arguments,
        absolute_working_directory,
        paths);
#else
    CapturedProcessResult result = run_process_capture_posix(
        absolute_executable,
        utf8_arguments,
        absolute_working_directory,
        paths);
#endif
    collect_and_remove_capture_files(paths, result);
    return result;
}

std::string normalize_captured_line_endings(const std::string_view text) {
    std::string normalized;
    normalized.reserve(text.size());
    for (std::size_t index = 0U; index < text.size(); ++index) {
        if (text[index] != '\r') {
            normalized.push_back(text[index]);
            continue;
        }
        if (index + 1U < text.size() && text[index + 1U] == '\n') {
            ++index;
        }
        normalized.push_back('\n');
    }
    return normalized;
}

CapturedProcessResult normalize_captured_process_line_endings(CapturedProcessResult result) {
    result.stdout_text = normalize_captured_line_endings(result.stdout_text);
    result.stderr_text = normalize_captured_line_endings(result.stderr_text);
    return result;
}

}  // namespace copperfin::test_support
