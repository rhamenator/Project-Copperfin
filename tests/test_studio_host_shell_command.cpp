// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "studio_host_main_support.h"

#include <filesystem>
#include <fstream>
#include <iostream>

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
#endif

}  // namespace

int main() {
    test_build_shell_command_uses_platform_quoting();
#if !defined(_WIN32)
    test_execute_launch_command_handles_paths_and_arguments_with_spaces();
#endif

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
