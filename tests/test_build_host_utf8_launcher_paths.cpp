// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/path.h"
#include "copperfin/vfp/dbf_table.h"
#include "test_environment_support.h"
#include "test_process_capture_support.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr int skip_return_code = 77;
int failures = 0;

[[maybe_unused]] void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

[[maybe_unused]] std::string line_value(const std::string& text, const std::string& key) {
    std::string line;
    std::istringstream input(text);
    const std::string prefix = key + ": ";
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.rfind(prefix, 0U) == 0U) {
            return line.substr(prefix.size());
        }
    }
    return {};
}

[[maybe_unused]] void write_utf8_project(
    const std::filesystem::path& project_path,
    const std::filesystem::path& project_dir) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "TYPE", .type = 'C', .length = 1U},
        {.name = "KEY", .type = 'C', .length = 32U},
        {.name = "HOMEDIR", .type = 'C', .length = 200U},
        {.name = "OUTFILE", .type = 'C', .length = 200U},
        {.name = "NAME", .type = 'C', .length = 200U},
        {.name = "MAINPROG", .type = 'L', .length = 1U}
    };
    const auto result = copperfin::vfp::create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(project_path),
        fields,
        {{"H", "Utf8 Launcher Project", copperfin::platform::path_to_utf8_string(project_dir),
          "Utf8Launcher.exe", "", "false"},
         {"K", "", "", "", "main.prg", "true"}});
    expect(result.ok, "UTF-8 launcher project fixture should be writable");
}

}  // namespace

int main(int argc, char** argv) {
#if !defined(_WIN32)
    (void)argc;
    (void)argv;
    std::cout << "SKIP: generated .NET launcher UTF-8 path coverage is Windows-only\n";
    return skip_return_code;
#else
    if (argc < 2 || argv[1] == nullptr || std::string(argv[1]).empty()) {
        std::cout << "SKIP: build-host executable argument is required\n";
        return skip_return_code;
    }

    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() /
        copperfin::platform::path_from_utf8_string(
            "copperfin_build_host_utf8_launcher_tests-\xC3\xA9");
    const fs::path project_dir = temp_root /
        copperfin::platform::path_from_utf8_string("proyecto-\xE6\x97\xA5");
    const fs::path output_root = temp_root /
        copperfin::platform::path_from_utf8_string("paquete-\xD0\x9F\xD1\x83\xD1\x82\xD1\x8C");
    const fs::path project_path = project_dir / "launcher.pjx";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    fs::create_directories(output_root);

    const fs::path tool_dir = temp_root /
        copperfin::platform::path_from_utf8_string("tool-\xD0\xBF\xD1\x83\xD1\x82\xD1\x8C");
    fs::create_directories(tool_dir);
    copperfin::test_support::ScopedEnvironmentPath locale_root(
        "COPPERFIN_LOCALE_DIR",
        copperfin::platform::path_from_utf8_string(COPPERFIN_TEST_LOCALE_DIR));
    const fs::path source_build_host_path =
        copperfin::platform::path_from_utf8_string(argv[1]);
    const fs::path source_runtime_host_path = source_build_host_path.parent_path() /
        "copperfin_runtime_host.exe";
    const fs::path source_launcher_guard_path = source_build_host_path.parent_path() /
        "copperfin_launcher_guard.exe";
    const fs::path build_host_path = tool_dir / "copperfin_build_host.exe";
    const fs::path runtime_host_path = tool_dir / "copperfin_runtime_host.exe";
    const fs::path launcher_guard_path = tool_dir / "copperfin_launcher_guard.exe";
    std::error_code copy_error;
    fs::copy_file(source_build_host_path, build_host_path,
                  fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "UTF-8 launcher test should copy the build host into a Unicode directory");
    copy_error.clear();
    fs::copy_file(source_runtime_host_path, runtime_host_path,
                  fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "UTF-8 launcher test should copy the runtime host beside the build host");
    copy_error.clear();
    fs::copy_file(source_launcher_guard_path, launcher_guard_path,
                  fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "UTF-8 launcher test should copy the launcher guard beside the build host");

    std::ofstream source(project_dir / "main.prg", std::ios::binary | std::ios::trunc);
    source << "RETURN\n";
    write_utf8_project(project_path, project_dir);

    const auto process = copperfin::test_support::run_process_capture(
        build_host_path,
        {"build", "--project", copperfin::platform::path_to_utf8_string(project_path),
         "--output-dir", copperfin::platform::path_to_utf8_string(output_root),
         "--emit-dotnet-launcher"},
        temp_root);
    if (process.exit_code != 0) {
        std::cerr << "build-host UTF-8 launcher stdout:\n" << process.stdout_text
                  << "\nbuild-host UTF-8 launcher stderr:\n" << process.stderr_text << "\n";
    }
    expect(process.exit_code == 0,
           "Windows build host should publish a launcher from a UTF-8 project/output path");
    expect(process.stdout_text.find("status: ok") != std::string::npos,
           "UTF-8 launcher publish should preserve the machine-readable success status");

    const std::string reported_launcher = line_value(process.stdout_text, "launcher.output");
    expect(!reported_launcher.empty(),
           "UTF-8 launcher publish should report its configured output path");
    if (!reported_launcher.empty()) {
        const fs::path launcher_path =
            copperfin::platform::path_from_utf8_string(reported_launcher);
        expect(fs::exists(launcher_path),
               "UTF-8 launcher publish should materialize the reported output path");
        expect(copperfin::platform::path_to_utf8_string(launcher_path).find(
                   copperfin::platform::path_to_utf8_string(output_root)) != std::string::npos,
               "UTF-8 launcher output should remain under the UTF-8 output root");
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
        return 0;
    }
    std::cerr << failures << " UTF-8 launcher test(s) failed\n";
    std::cerr << "fixture root: " << temp_root << "\n";
    return EXIT_FAILURE;
#endif
}
