// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "runtime_pipeline_support.h"

#include <cerrno>
#include <filesystem>
#include <string>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace copperfin::runtime {

namespace runtime_pipeline_detail {

#if defined(_WIN32)
std::wstring native_wrapper_utf8_to_wide(std::string_view value) {
    return copperfin::platform::path_from_utf8_string(value).native();
}

std::wstring quote_native_wrapper_windows_argument(const std::wstring& argument) {
    const bool needs_quotes = argument.empty() ||
        argument.find_first_of(L" \t\r\n\"") != std::wstring::npos;
    if (!needs_quotes) {
        return argument;
    }

    std::wstring quoted(1U, L'"');
    std::size_t backslashes = 0U;
    for (const wchar_t ch : argument) {
        if (ch == L'\\') {
            ++backslashes;
            continue;
        }
        if (ch == L'"') {
            quoted.append(backslashes * 2U + 1U, L'\\');
        } else {
            quoted.append(backslashes, L'\\');
        }
        quoted.push_back(ch);
        backslashes = 0U;
    }
    quoted.append(backslashes * 2U, L'\\');
    quoted.push_back(L'"');
    return quoted;
}
#endif

NativeWrapperProcessResult run_native_wrapper_process(
    const std::string& executable,
    const std::vector<std::string>& arguments,
    const std::filesystem::path& output_log_path) {
#if defined(_WIN32)
    std::wstring command_line = quote_native_wrapper_windows_argument(
        native_wrapper_utf8_to_wide(executable));
    for (const auto& argument : arguments) {
        command_line.push_back(L' ');
        command_line += quote_native_wrapper_windows_argument(
            native_wrapper_utf8_to_wide(argument));
    }
    std::vector<wchar_t> mutable_command_line(command_line.begin(), command_line.end());
    mutable_command_line.push_back(L'\0');

    SECURITY_ATTRIBUTES security_attributes{};
    security_attributes.nLength = sizeof(security_attributes);
    security_attributes.bInheritHandle = TRUE;
    HANDLE output_handle = ::CreateFileW(
        output_log_path.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        &security_attributes,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (output_handle == INVALID_HANDLE_VALUE) {
        return {};
    }

    STARTUPINFOW startup_info{};
    startup_info.cb = sizeof(startup_info);
    startup_info.dwFlags = STARTF_USESTDHANDLES;
    startup_info.hStdInput = ::GetStdHandle(STD_INPUT_HANDLE);
    startup_info.hStdOutput = output_handle;
    startup_info.hStdError = output_handle;
    PROCESS_INFORMATION process_info{};
    const BOOL process_created = ::CreateProcessW(
        nullptr,
        mutable_command_line.data(),
        nullptr,
        nullptr,
        TRUE,
        CREATE_UNICODE_ENVIRONMENT,
        nullptr,
        nullptr,
        &startup_info,
        &process_info);
    if (process_created == FALSE) {
        (void)::CloseHandle(output_handle);
        return {};
    }

    (void)::WaitForSingleObject(process_info.hProcess, INFINITE);
    DWORD process_exit_code = static_cast<DWORD>(-1);
    (void)::GetExitCodeProcess(process_info.hProcess, &process_exit_code);
    (void)::CloseHandle(process_info.hThread);
    (void)::CloseHandle(process_info.hProcess);
    (void)::CloseHandle(output_handle);
    return {true, static_cast<int>(process_exit_code)};
#else
    std::vector<std::string> argument_values;
    argument_values.reserve(arguments.size() + 1U);
    argument_values.push_back(executable);
    argument_values.insert(argument_values.end(), arguments.begin(), arguments.end());
    std::vector<char*> argument_pointers;
    argument_pointers.reserve(argument_values.size() + 1U);
    for (auto& argument : argument_values) {
        argument_pointers.push_back(argument.data());
    }
    argument_pointers.push_back(nullptr);

    const pid_t child_process = ::fork();
    if (child_process < 0) {
        return {};
    }
    if (child_process == 0) {
        const int output_descriptor = ::open(
            output_log_path.c_str(),
            O_WRONLY | O_CREAT | O_TRUNC,
            0666);
        if (output_descriptor < 0 ||
            ::dup2(output_descriptor, STDOUT_FILENO) < 0 ||
            ::dup2(output_descriptor, STDERR_FILENO) < 0) {
            if (output_descriptor >= 0) {
                (void)::close(output_descriptor);
            }
            ::_exit(126);
        }
        (void)::close(output_descriptor);
        ::execvp(argument_pointers[0], argument_pointers.data());
        ::_exit(127);
    }

    int child_status = 0;
    pid_t waited_process = 0;
    do {
        waited_process = ::waitpid(child_process, &child_status, 0);
    } while (waited_process < 0 && errno == EINTR);
    if (waited_process != child_process) {
        return {};
    }
    if (WIFEXITED(child_status)) {
        return {true, WEXITSTATUS(child_status)};
    }
    if (WIFSIGNALED(child_status)) {
        return {true, 128 + WTERMSIG(child_status)};
    }
    return {true, -1};
#endif
}

}  // namespace runtime_pipeline_detail

}  // namespace copperfin::runtime
