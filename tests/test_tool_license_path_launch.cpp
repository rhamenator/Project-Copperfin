// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_environment_support.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#if !defined(_WIN32)
#include <sys/wait.h>
#endif

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void write_text(const std::filesystem::path& path, const std::string& text) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << text;
}

std::string read_text(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

std::string quote_command_argument(const std::string& value) {
    std::string quoted = "\"";
    for (const char ch : value) {
        if (ch == '"') {
            quoted += "\\\"";
        } else {
            quoted.push_back(ch);
        }
    }
    quoted.push_back('"');
    return quoted;
}

struct ProcessResult {
    int exit_code = -1;
    std::string stdout_text;
};

ProcessResult run_process_capture(
    const std::string& executable,
    const std::vector<std::string>& arguments,
    const std::filesystem::path& working_directory,
    const std::string& log_name) {
    namespace fs = std::filesystem;
    const fs::path stdout_path = working_directory / (log_name + ".stdout.log");
    const fs::path stderr_path = working_directory / (log_name + ".stderr.log");
    std::string command = quote_command_argument(executable);
    for (const auto& argument : arguments) {
        command += " " + quote_command_argument(argument);
    }
    command += " > " + quote_command_argument(stdout_path.string());
    command += " 2> " + quote_command_argument(stderr_path.string());

    const fs::path original_directory = fs::current_path();
    fs::current_path(working_directory);
    const int raw_exit_code = copperfin::test_support::run_shell_command(command);
    fs::current_path(original_directory);

    ProcessResult result;
#if defined(_WIN32)
    result.exit_code = raw_exit_code;
#else
    result.exit_code = raw_exit_code != -1 && WIFEXITED(raw_exit_code)
        ? WEXITSTATUS(raw_exit_code)
        : raw_exit_code;
#endif
    result.stdout_text = read_text(stdout_path);
    return result;
}

void copy_executable(
    const std::filesystem::path& source,
    const std::filesystem::path& destination) {
    std::filesystem::copy_file(
        source,
        destination,
        std::filesystem::copy_options::overwrite_existing);
#if !defined(_WIN32)
    std::error_code ignored;
    std::filesystem::permissions(
        destination,
        std::filesystem::perms::owner_exec |
            std::filesystem::perms::group_exec |
            std::filesystem::perms::others_exec,
        std::filesystem::perm_options::add,
        ignored);
#endif
}

}  // namespace

int main(int argc, char** argv) {
    namespace fs = std::filesystem;
    if (argc < 3) {
        std::cerr << "FAIL: build-host and inspect executable arguments are required\n";
        return 1;
    }

    const fs::path temp_root = fs::temp_directory_path() /
        ("copperfin_tool_license_path_launch_" +
         std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const fs::path bundle_root = temp_root / "bundle";
    const fs::path caller_root = temp_root / "caller";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(bundle_root);
    fs::create_directories(caller_root);

    const fs::path build_host = bundle_root / fs::path(argv[1]).filename();
    const fs::path inspect_host = bundle_root / fs::path(argv[2]).filename();
    copy_executable(argv[1], build_host);
    copy_executable(argv[2], inspect_host);

    const fs::path deployed_license = bundle_root / "license.cflicense";
    const fs::path caller_license = caller_root / "license.cflicense";
    write_text(deployed_license, "{\"deployed\":true}\n");
    write_text(caller_license, "{\"caller\":true}\n");

    copperfin::test_support::ScopedEnvironmentValue license_path("COPPERFIN_LICENSE_PATH");
    copperfin::test_support::ScopedEnvironmentValue search_path("PATH", false);
#if defined(_WIN32)
    copperfin::test_support::ScopedEnvironmentValue path_extensions(
        "PATHEXT",
        ".EXE;.COM;.BAT;.CMD");
    constexpr char path_separator = ';';
    const std::string build_launch_name = build_host.stem().string();
    const std::string inspect_launch_name = inspect_host.stem().string();
#else
    constexpr char path_separator = ':';
    const std::string build_launch_name = build_host.filename().string();
    const std::string inspect_launch_name = inspect_host.filename().string();
#endif
    const std::string original_path = copperfin::test_support::getenv_value("PATH");
    search_path.set(
        bundle_root.string() +
        (original_path.empty()
             ? std::string()
             : std::string(1U, path_separator) + original_path));

    const auto verify_host = [&] (const std::string& launch_name, const std::string& label) {
        const ProcessResult process = run_process_capture(
            launch_name,
            {"--license-status"},
            caller_root,
            label);
        expect(process.exit_code != 0,
               "#4900: PATH-launched " + label + " should reject the inactive license-status command");
        expect(process.stdout_text.find("status: ok") == std::string::npos &&
                   process.stdout_text.find("state:") == std::string::npos,
               "#4900: PATH-launched " + label + " should not present product-license state");
        expect(process.stdout_text.find("source_path:") == std::string::npos &&
                   process.stdout_text.find("diagnostic:") == std::string::npos,
               "#4900: PATH-launched " + label + " must not inspect deployed or caller license files");
    };

    verify_host(build_launch_name, "build-host");
    verify_host(inspect_launch_name, "inspect-host");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    } else {
        std::cerr << "fixture root: " << temp_root << "\n";
    }
    return failures == 0 ? 0 : 1;
}
