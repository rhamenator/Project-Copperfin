// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "studio_host_main_support.h"

#include "copperfin/platform/path.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#if defined(_WIN32)
#include <windows.h>
#else
#include <csignal>
#include <unistd.h>
#endif

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void test_build_shell_command_uses_platform_quoting() {
#if defined(_WIN32)
    expect(
        cf_studio_host_main_detail::build_shell_command(
            R"(C:\Program Files\Copperfin Tool\tool.exe)",
            {"alpha beta", "100%", R"(say "hi")"}) ==
            "\"C:\\Program Files\\Copperfin Tool\\tool.exe\" \"alpha beta\" \"100%%\" \"say \"\"hi\"\"\"",
        "#3674: Windows Studio-host executed-command strings should use cmd-safe double-quote formatting");
#else
    expect(
        cf_studio_host_main_detail::build_shell_command(
            "/tmp/copperfin tool",
            {"alpha beta", "it's"}) ==
            "'/tmp/copperfin tool' 'alpha beta' 'it'\\''s'",
        "#3674: POSIX Studio-host executed-command strings should preserve the existing single-quote shell formatting");
#endif
}

#if !defined(_WIN32)
void test_execute_launch_command_handles_paths_and_arguments_with_spaces() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_shell_command_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path script_path = temp_dir / "launcher with spaces.sh";
    const fs::path output_path = temp_dir / "captured args.txt";
    {
        std::ofstream output(script_path, std::ios::binary);
        output << "#!/bin/sh\n";
        output << "printf '%s\\n%s\\n' \"$1\" \"$2\" > \"$3\"\n";
    }
    fs::permissions(
        script_path,
        fs::perms::owner_exec | fs::perms::owner_read | fs::perms::owner_write,
        fs::perm_options::replace,
        ignored);

    const int exit_code = cf_studio_host_main_detail::execute_launch_command(
        script_path.string(),
        {"alpha beta", "literal%value", output_path.string()});
    expect(exit_code == 0,
           "#3674: Studio-host direct launch execution should succeed for executable paths containing spaces");

    std::ifstream input(output_path, std::ios::binary);
    const std::string captured{
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
    expect(captured == "alpha beta\nliteral%value\n",
           "#3674: Studio-host direct launch execution should preserve spaced arguments without shell splitting");

    fs::remove_all(temp_dir, ignored);
}

volatile std::sig_atomic_t alarm_signal_count = 0;

void record_alarm_signal(int) {
    alarm_signal_count = 1;
}

void test_execute_launch_command_retries_interrupted_wait() {
    const auto previous_handler = std::signal(SIGALRM, record_alarm_signal);
    alarm_signal_count = 0;
    (void)ualarm(100000U, 0U);

    const int exit_code = cf_studio_host_main_detail::execute_launch_command(
        "/bin/sh",
        {"-c", "sleep 1"});

    (void)ualarm(0U, 0U);
    std::signal(SIGALRM, previous_handler);

    expect(alarm_signal_count != 0,
           "#4325: POSIX Studio-host wait regression should interrupt the parent wait");
    expect(exit_code == 0,
           "#4325: POSIX Studio-host should retry an EINTR wait and reap the child successfully");
}

void test_execute_launch_command_distinguishes_spawn_failure_from_exit_127() {
    const int missing_exit_code = cf_studio_host_main_detail::execute_launch_command(
        "/copperfin/missing/studio-host-tool",
        {});
    expect(missing_exit_code == -1,
           "#196: POSIX Studio-host should report a missing executable as a launch failure");

    const int child_exit_code = cf_studio_host_main_detail::execute_launch_command(
        "/bin/sh",
        {"-c", "exit 127"});
    expect(child_exit_code == 127,
           "#196: POSIX Studio-host should preserve an actual child exit code of 127");
}
#else
void test_execute_launch_command_preserves_unicode_paths_and_percent_arguments() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / L"copperfin_studio_host_\u00E9";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    std::wstring executable_buffer(32768U, L'\0');
    const DWORD executable_length = GetModuleFileNameW(
        nullptr,
        executable_buffer.data(),
        static_cast<DWORD>(executable_buffer.size()));
    expect(executable_length > 0U && executable_length < executable_buffer.size(),
           "#4279: Windows Studio-host test should resolve its executable path through the wide API");
    if (executable_length == 0U || executable_length >= executable_buffer.size()) {
        fs::remove_all(temp_dir, ignored);
        return;
    }
    executable_buffer.resize(executable_length);

    const fs::path fixture_path = temp_dir / L"studio-host-launch-fixture.exe";
    std::error_code copy_error;
    fs::copy_file(
        fs::path(executable_buffer),
        fixture_path,
        fs::copy_options::overwrite_existing,
        copy_error);
    expect(!copy_error,
           "#4279: Windows Studio-host test should copy a launch fixture into a Unicode directory");
    if (copy_error) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    const int exit_code = cf_studio_host_main_detail::execute_launch_command(
        copperfin::platform::path_to_utf8_string(fixture_path),
        {"--fixture", "literal%value"});
    expect(exit_code == 17,
           "#4279: Windows Studio-host direct launch should preserve Unicode executable paths and percent arguments");

    fs::remove_all(temp_dir, ignored);
}
#endif

}  // namespace

int main(int argc, char** argv) {
#if defined(_WIN32)
    if (argc == 3 && std::string(argv[1]) == "--fixture" && std::string(argv[2]) == "literal%value") {
        return 17;
    }
#else
    (void)argc;
    (void)argv;
#endif

    test_build_shell_command_uses_platform_quoting();
#if !defined(_WIN32)
    test_execute_launch_command_handles_paths_and_arguments_with_spaces();
    test_execute_launch_command_retries_interrupted_wait();
    test_execute_launch_command_distinguishes_spawn_failure_from_exit_127();
#else
    test_execute_launch_command_preserves_unicode_paths_and_percent_arguments();
#endif

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
