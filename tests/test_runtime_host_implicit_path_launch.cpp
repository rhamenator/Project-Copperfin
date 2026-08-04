// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_environment_support.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
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
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << text;
}

std::string read_text(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return std::string(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>());
}

std::string quote_command_argument(const std::string& value) {
    std::string quoted = "\"";
    for (const char ch : value) {
        if (ch == '\"') {
            quoted += "\\\"";
        } else {
            quoted.push_back(ch);
        }
    }
    quoted.push_back('\"');
    return quoted;
}

std::string quote_manifest_value(const std::filesystem::path& value) {
    std::string escaped;
    for (const char ch : value.string()) {
        if (ch == '\\') {
            escaped += "\\\\";
        } else {
            escaped.push_back(ch);
        }
    }
    return escaped;
}

struct ProcessResult {
    int exit_code = -1;
    std::string stdout_text;
    std::string stderr_text;
};

ProcessResult run_process_capture(
    const std::string& executable,
    const std::vector<std::string>& arguments,
    const std::filesystem::path& working_directory,
    std::string_view log_name) {
    namespace fs = std::filesystem;

    const fs::path stdout_path = working_directory / (std::string(log_name) + ".stdout.log");
    const fs::path stderr_path = working_directory / (std::string(log_name) + ".stderr.log");
    std::string command = quote_command_argument(executable);
    for (const std::string& argument : arguments) {
        command += " ";
        command += quote_command_argument(argument);
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
    result.stderr_text = read_text(stderr_path);
    return result;
}

}  // namespace

int main(int argc, char** argv) {
    namespace fs = std::filesystem;
    if (argc < 2) {
        std::cerr << "FAIL: runtime host executable path argument is required\n";
        return 1;
    }

    const fs::path temp_root = fs::temp_directory_path() /
        ("copperfin_runtime_host_path_launch_" +
         std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path deployed_runtime_host = temp_root / fs::path(argv[1]).filename();
    fs::copy_file(argv[1], deployed_runtime_host, fs::copy_options::overwrite_existing);
#if !defined(_WIN32)
    fs::permissions(
        deployed_runtime_host,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add,
        ignored);
#endif

    const fs::path release_source = temp_root / "release_main.prg";
    const fs::path debug_source = temp_root / "debug_main.prg";
    write_text(release_source, "LOCAL cMode\ncMode = 'release'\nRETURN\n");
    write_text(debug_source, "LOCAL cMode\ncMode = 'debug'\nRETURN\n");

    const std::string package_root = quote_manifest_value(temp_root);
    write_text(
        temp_root / "app.cfmanifest",
        "manifest_version=1\n"
        "project_title=PathLaunchRelease\n"
        "package_root=" + package_root + "\n"
        "content_root=" + package_root + "\n"
        "working_directory=" + package_root + "\n"
        "startup_item=release_main.prg\n"
        "startup_source=release_main.prg\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");
    write_text(
        temp_root / "app.cfdebug",
        "debug_manifest_version=2\n"
        "project_title=PathLaunchDebug\n"
        "package_root=" + package_root + "\n"
        "content_root=" + package_root + "\n"
        "working_directory=" + package_root + "\n"
        "startup_item=debug_main.prg\n"
        "startup_source=debug_main.prg\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    const fs::path caller_root = temp_root / "unrelated-caller";
    const fs::path deployed_license_path = temp_root / "license.cflicense";
    const fs::path caller_license_path = caller_root / "license.cflicense";
    const fs::path deployed_locale_root = temp_root / "share" / "copperfin" / "locales" / "en-US";
    const fs::path caller_locale_root = caller_root / "resources" / "locales" / "en-US";
    fs::create_directories(deployed_locale_root);
    fs::create_directories(caller_locale_root);
    write_text(deployed_license_path, "{\"deployed\":true}\n");
    write_text(caller_license_path, "{\"caller\":true}\n");
    write_text(
        deployed_locale_root / "strings.json",
        "{\"RuntimeHost.Prefix.Error\":\"error: \","
        "\"RuntimeHost.Error.ManifestNotFound\":\"DEPLOYED_LOCALE_ROOT\"}\n");
    write_text(
        caller_locale_root / "strings.json",
        "{\"RuntimeHost.Prefix.Error\":\"error: \","
        "\"RuntimeHost.Error.ManifestNotFound\":\"CALLER_LOCALE_ROOT\"}\n");

    copperfin::test_support::ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR");
    copperfin::test_support::ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "en-US");
    copperfin::test_support::ScopedEnvironmentValue license_path("COPPERFIN_LICENSE_PATH");
    copperfin::test_support::ScopedEnvironmentValue search_path("PATH", false);
#if defined(_WIN32)
    copperfin::test_support::ScopedEnvironmentValue path_extensions(
        "PATHEXT",
        ".EXE;.COM;.BAT;.CMD");
    constexpr char path_separator = ';';
    const std::string launch_name = deployed_runtime_host.stem().string();
#else
    constexpr char path_separator = ':';
    const std::string launch_name = deployed_runtime_host.filename().string();
#endif
    const std::string original_path = copperfin::test_support::getenv_value("PATH");
    search_path.set(
        deployed_runtime_host.parent_path().string() +
        (original_path.empty()
             ? std::string()
             : std::string(1U, path_separator) + original_path));

    const ProcessResult debug_process = run_process_capture(
        launch_name,
        {"--debug", "--debug-command", "break:add:2", "--debug-command", "continue"},
        caller_root,
        "debug");
    expect(debug_process.exit_code == 0,
           "#4013: PATH-launched debug should resolve adjacent app.cfdebug");
    expect(debug_process.stdout_text.find("debug_main.prg:2") != std::string::npos,
           "#4013: PATH-launched debug should prefer the debug startup identity");

    const ProcessResult runtime_process = run_process_capture(
        launch_name,
        {},
        caller_root,
        "runtime");
    expect(runtime_process.exit_code == 0,
           "#4013: PATH-launched runtime should resolve adjacent app.cfmanifest");
    expect(runtime_process.stdout_text.find("project.title: PathLaunchRelease") != std::string::npos,
           "#4013: non-debug PATH launch should select the release manifest title");
    expect(runtime_process.stdout_text.find("startup.item: release_main.prg") != std::string::npos,
           "#4013: non-debug PATH launch should select the release startup identity");

    const ProcessResult license_process = run_process_capture(
        launch_name,
        {"--license-status"},
        caller_root,
        "license-status");
    expect(license_process.exit_code == 2,
           "#4900: PATH-launched runtime host should reject the inactive license-status command");
    expect(license_process.stdout_text.find("status: ok") == std::string::npos &&
               license_process.stdout_text.find("state:") == std::string::npos,
           "#4900: PATH-launched runtime host should not present product-license state");
    expect(license_process.stdout_text.find("source_path:") == std::string::npos &&
               license_process.stdout_text.find("diagnostic:") == std::string::npos,
           "#4900: runtime host must not inspect deployed or caller license files");

    const ProcessResult localized_failure = run_process_capture(
        launch_name,
        {"--manifest", (caller_root / "missing.cfmanifest").string()},
        caller_root,
        "localized-failure");
    expect(localized_failure.exit_code == 3,
           "#4013: explicit missing manifests should preserve exit code 3");
    expect(localized_failure.stdout_text.find("status: error") != std::string::npos,
           "#4013: localized failures should preserve invariant status fields");
    expect(localized_failure.stdout_text.find("DEPLOYED_LOCALE_ROOT") != std::string::npos &&
               localized_failure.stdout_text.find("CALLER_LOCALE_ROOT") == std::string::npos,
           "#4013: localized prose should bind to the running image, not caller CWD");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    } else {
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    if (failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    std::cout << "All runtime-host implicit PATH-launch tests passed\n";
    return 0;
}
