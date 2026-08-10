// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "studio_host_main_support.h"

#include "copperfin/platform/path.h"

#include <cerrno>
#if defined(_WIN32)
#include <windows.h>
#else
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#if defined(__APPLE__)
#include <crt_externs.h>
#else
extern char** environ;
#endif
#endif

namespace cf_studio_host_main_detail {

namespace {

#if defined(_WIN32)
std::string windows_shell_quote(const std::string& value) {
    if (value.empty()) {
        return "\"\"";
    }

    std::string quoted;
    quoted.reserve(value.size() + 2U);
    quoted.push_back('"');
    for (const char ch : value) {
        switch (ch) {
            case '"':
                quoted += "\"\"";
                break;
            case '%':
                quoted += "%%";
                break;
            default:
                quoted.push_back(ch);
                break;
        }
    }
    quoted.push_back('"');
    return quoted;
}

std::wstring utf8_to_wide(const std::string& value) {
    if (value.empty()) {
        return {};
    }
    const int count = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.data(),
        static_cast<int>(value.size()),
        nullptr,
        0);
    if (count <= 0) {
        return {};
    }
    std::wstring result(static_cast<std::size_t>(count), L'\0');
    if (MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            value.data(),
            static_cast<int>(value.size()),
            result.data(),
            count) != count) {
        return {};
    }
    return result;
}

std::wstring quote_windows_argument(const std::wstring& value) {
    const bool needs_quotes = value.empty() || value.find_first_of(L" \t\r\n\"") != std::wstring::npos;
    if (!needs_quotes) {
        return value;
    }

    std::wstring quoted(1U, L'"');
    std::size_t backslashes = 0U;
    for (const wchar_t ch : value) {
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

}  // namespace

std::string shell_quote(const std::string& value) {
#if defined(_WIN32)
    return windows_shell_quote(value);
#else
    if (value.empty()) {
        return "''";
    }
    std::string quoted = "'";
    for (const char ch : value) {
        if (ch == '\'') {
            quoted += "'\\''";
        } else {
            quoted += ch;
        }
    }
    quoted += "'";
    return quoted;
#endif
}

std::string build_shell_command(const std::string& launch_command, const std::vector<std::string>& arguments) {
    std::string command = shell_quote(launch_command);
    for (const auto& argument : arguments) {
        command += " ";
        command += shell_quote(argument);
    }
    return command;
}

int execute_launch_command(const std::string& launch_command, const std::vector<std::string>& arguments) {
#if defined(_WIN32)
    const auto executable = utf8_to_wide(launch_command);
    if (executable.empty()) {
        return -1;
    }

    std::wstring command_line = quote_windows_argument(executable);
    for (const auto& argument : arguments) {
        command_line.push_back(L' ');
        command_line += quote_windows_argument(utf8_to_wide(argument));
    }
    std::vector<wchar_t> mutable_command_line(command_line.begin(), command_line.end());
    mutable_command_line.push_back(L'\0');

    STARTUPINFOW startup_info{};
    startup_info.cb = sizeof(startup_info);
    PROCESS_INFORMATION process_info{};
    if (CreateProcessW(
            nullptr,
            mutable_command_line.data(),
            nullptr,
            nullptr,
            FALSE,
            CREATE_UNICODE_ENVIRONMENT,
            nullptr,
            nullptr,
            &startup_info,
            &process_info) == FALSE) {
        return -1;
    }

    const DWORD wait_result = WaitForSingleObject(process_info.hProcess, INFINITE);
    DWORD process_exit_code = static_cast<DWORD>(-1);
    const BOOL exit_code_read = GetExitCodeProcess(process_info.hProcess, &process_exit_code);
    CloseHandle(process_info.hThread);
    CloseHandle(process_info.hProcess);
    if (wait_result != WAIT_OBJECT_0 || exit_code_read == FALSE) {
        return -1;
    }
    return static_cast<int>(process_exit_code);
#else
    std::vector<std::string> launch_arguments;
    launch_arguments.reserve(arguments.size() + 1U);
    launch_arguments.push_back(launch_command);
    launch_arguments.insert(launch_arguments.end(), arguments.begin(), arguments.end());

    std::vector<char*> argv;
    argv.reserve(launch_arguments.size() + 1U);
    for (auto& argument : launch_arguments) {
        argv.push_back(argument.data());
    }
    argv.push_back(nullptr);

    char* const* environment =
#if defined(__APPLE__)
        *_NSGetEnviron();
#else
        environ;
#endif
    pid_t child = 0;
    const int spawn_result = posix_spawnp(
        &child,
        launch_command.c_str(),
        nullptr,
        nullptr,
        argv.data(),
        environment);
    if (spawn_result != 0) {
        return -1;
    }

    int status = 0;
    pid_t wait_result = 0;
    do {
        wait_result = waitpid(child, &status, 0);
    } while (wait_result < 0 && errno == EINTR);
    if (wait_result != child) {
        return -1;
    }
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }
    return status;
#endif
}

}  // namespace cf_studio_host_main_detail
