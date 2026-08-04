// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/vfp/dbf_table.h"
#include "test_environment_support.h"
#include "test_process_capture_support.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2 || argv[1] == nullptr || std::string(argv[1]).empty()) {
        std::cerr << "Studio host executable path is required\n";
        return EXIT_FAILURE;
    }

    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / "copperfin_studio_host_deleted_record_localization";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path table_path = root / "deleted_record.dbf";
    const auto created = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        {{.name = "NAME", .type = 'C', .length = 20U}},
        {{"Ada"}});
    expect(created.ok, "deleted-record fixture should be writable");
    const auto parsed = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
    expect(parsed.ok && !parsed.table.records.empty(), "deleted-record fixture should be readable");
    if (parsed.ok && !parsed.table.records.empty()) {
        std::fstream table(table_path, std::ios::binary | std::ios::in | std::ios::out);
        table.seekp(static_cast<std::streamoff>(parsed.table.header.header_length));
        table.put('*');
        expect(table.good(), "deleted-record fixture should mark its record deleted");
    }

    copperfin::test_support::ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
    copperfin::test_support::ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR");
    locale_dir.set(copperfin::test_support::path_from_utf8_string(COPPERFIN_TEST_LOCALE_DIR));
    const auto process = copperfin::test_support::normalize_captured_process_line_endings(
        copperfin::test_support::run_process_capture(
            copperfin::test_support::path_from_utf8_string(argv[1]),
            {"--path", copperfin::test_support::path_to_utf8_string(table_path)},
            root));
    expect(process.started && process.exit_code == 0,
           "Studio host deleted-record text preview should succeed: " + process.stderr_text);
    expect(process.stdout_text.find("record[0] [!!") != std::string::npos,
           "qps-ploc deleted-record preview should localize the deleted label; output: " + process.stdout_text);
    expect(process.stdout_text.find("record[0] deleted\n") == std::string::npos,
           "qps-ploc deleted-record preview should not emit the bare English label; output: " + process.stdout_text);

    fs::remove_all(root, ignored);
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
