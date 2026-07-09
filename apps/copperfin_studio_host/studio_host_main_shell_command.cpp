// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "studio_host_main_support.h"

#include <cerrno>
#if defined(_WIN32)
#include <process.h>
#else
#include <sys/wait.h>
#include <unistd.h>
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
    std::vector<std::string> launch_arguments;
    launch_arguments.reserve(arguments.size() + 1U);
    launch_arguments.push_back(launch_command);
    launch_arguments.insert(launch_arguments.end(), arguments.begin(), arguments.end());

    std::vector<const char*> argv;
    argv.reserve(launch_arguments.size() + 1U);
    for (const auto& argument : launch_arguments) {
        argv.push_back(argument.c_str());
    }
    argv.push_back(nullptr);

#if defined(_WIN32)
    const intptr_t exit_code = _spawnvp(_P_WAIT, launch_command.c_str(), const_cast<char* const*>(argv.data()));
    if (exit_code == -1) {
        return -1;
    }
    return static_cast<int>(exit_code);
#else
    const pid_t child = fork();
    if (child == 0) {
        execvp(launch_command.c_str(), const_cast<char* const*>(argv.data()));
        _exit(127);
    }
    if (child < 0) {
        return -1;
    }

    int status = 0;
    if (waitpid(child, &status, 0) != child) {
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
