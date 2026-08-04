// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/platform/path.h"
#include "test_process_capture_support.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

std::string json_escape_for_search(std::string_view value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (const char ch : value) {
        if (ch == '\\' || ch == '"') {
            escaped.push_back('\\');
        }
        escaped.push_back(ch);
    }
    return escaped;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_utf8_arguments <copperfin_studio_host>\n";
        return 2;
    }

    const std::filesystem::path temp_root =
        std::filesystem::temp_directory_path() / "copperfin_studio_host_utf8_args";
    const std::string unicode_component = "copperfin_studio_\xC3\xA9_\xE6\xBC\xA2";
    const std::filesystem::path project_root = temp_root /
        copperfin::platform::path_from_utf8_string(unicode_component);
    const std::filesystem::path source_path = project_root / "main.prg";
    std::error_code error;
    std::filesystem::remove_all(temp_root, error);
    std::filesystem::create_directories(project_root, error);
    expect(!error, "standalone Studio Unicode argument test should create its fixture directory");
    if (error) {
        return 1;
    }

    {
        std::ofstream source(source_path, std::ios::binary | std::ios::trunc);
        source << "* Unicode argument regression\nRETURN .T.\n";
        expect(source.good(), "standalone Studio Unicode argument test should write its PRG fixture");
    }

    const std::string source_text = copperfin::platform::path_to_utf8_string(source_path);
    const auto process = copperfin::test_support::normalize_captured_process_line_endings(
        copperfin::test_support::run_process_capture(
            argv[1],
            {"--path", source_text, "--json"},
            project_root));
    expect(process.started,
           "standalone Studio should start when launched with a UTF-8 path argument");
    expect(process.exit_code == 0,
           "standalone Studio should open a PRG whose path contains non-ASCII characters");
    expect(process.stdout_text.find("\"status\": \"ok\"") != std::string::npos,
           "standalone Studio should return its normal JSON success contract for a UTF-8 path");
    expect(process.stdout_text.find(json_escape_for_search(source_text)) != std::string::npos,
           "standalone Studio JSON should preserve the UTF-8 document path");

    std::filesystem::remove_all(temp_root, error);
    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }
    std::cout << "All tests passed.\n";
    return 0;
}
