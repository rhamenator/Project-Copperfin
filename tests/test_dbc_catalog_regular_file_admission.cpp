// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/asset_inspector.h"
#include "copperfin/vfp/dbf_table.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#if !defined(_WIN32)
#include <csignal>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

std::filesystem::path unique_test_directory() {
    const auto nonce = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() /
        ("copperfin_dbc_catalog_regular_file_admission_" + std::to_string(nonce));
}

std::filesystem::path create_regular_dbc_control(const std::filesystem::path& directory) {
    const std::filesystem::path dbc_path = directory / "regular.dbc";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 0U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 0U, .length = 32U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 0U, .length = 32U, .decimal_count = 0U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Database", "RegularControl", ""},
    };
    const auto created = copperfin::vfp::create_dbf_table_file(
        dbc_path.string(), fields, records);
    expect(created.ok, "regular DBC control creation should succeed: " + created.error);
    return dbc_path;
}

void expect_regular_dbc_apis(const std::filesystem::path& dbc_path) {
    const auto procedures =
        copperfin::vfp::extract_dbc_stored_procedures_source(dbc_path.string());
    expect(procedures.ok,
           "stored-procedure extraction should preserve a valid regular DBC: " +
               procedures.error);

    const auto json = copperfin::vfp::export_database_as_json(dbc_path.string());
    expect(json.ok, "JSON export should preserve a valid regular DBC: " + json.error);
    expect(!json.json.empty(), "JSON export should produce a document for the regular control");

    const auto sql = copperfin::vfp::export_database_as_sql(dbc_path.string());
    expect(sql.ok, "SQL export should preserve a valid regular DBC: " + sql.error);
    expect(!sql.sql.empty(), "SQL export should produce a script for the regular control");
}

void test_regular_dbc_control() {
    namespace fs = std::filesystem;
    const fs::path directory = unique_test_directory();
    std::error_code ignored;
    fs::create_directories(directory, ignored);
    expect(!ignored, "regular-control directory creation should succeed");
    const fs::path dbc_path = create_regular_dbc_control(directory);

    expect_regular_dbc_apis(dbc_path);

#if !defined(_WIN32)
    const fs::path regular_symlink = directory / "regular-link.dbc";
    fs::create_symlink(dbc_path.filename(), regular_symlink, ignored);
    expect(!ignored, "regular DBC symlink creation should succeed");
    if (!ignored) {
        expect_regular_dbc_apis(regular_symlink);
    }
#endif

    fs::remove_all(directory, ignored);
}

#if !defined(_WIN32)

void expect_prompt_rejection(
    const std::string& label,
    const std::function<bool()>& rejected) {
    const pid_t child = ::fork();
    if (child < 0) {
        expect(false, label + ": fork should succeed");
        return;
    }
    if (child == 0) {
        ::alarm(30U);
        _exit(rejected() ? EXIT_SUCCESS : EXIT_FAILURE);
    }

    int status = 0;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(35);
    while (std::chrono::steady_clock::now() < deadline) {
        const pid_t waited = ::waitpid(child, &status, WNOHANG);
        if (waited == child) {
            if (WIFSIGNALED(status)) {
                expect(false, label + ": child watchdog terminated a blocked API call");
                return;
            }
            expect(WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS,
                   label + ": API should promptly return a structured rejection");
            return;
        }
        if (waited < 0) {
            expect(false, label + ": waitpid should succeed");
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    (void)::kill(child, SIGKILL);
    (void)::waitpid(child, &status, 0);
    expect(false, label + ": API blocked on an unpaired FIFO");
}

void test_fifo_dbc_paths_are_rejected_promptly() {
    namespace fs = std::filesystem;
    const fs::path directory = unique_test_directory();
    std::error_code ignored;
    fs::create_directories(directory, ignored);
    expect(!ignored, "FIFO-test directory creation should succeed");

    const fs::path fifo_path = directory / "catalog.fifo";
    const fs::path fifo_symlink = directory / "catalog-link.dbc";
    expect(::mkfifo(fifo_path.c_str(), 0600) == 0, "test FIFO creation should succeed");
    fs::create_symlink(fifo_path.filename(), fifo_symlink, ignored);
    expect(!ignored, "FIFO symlink creation should succeed");

    for (const fs::path& candidate : {fifo_path, fifo_symlink}) {
        const std::string kind = candidate == fifo_path ? "direct FIFO" : "symlink-to-FIFO";
        expect_prompt_rejection(kind + " stored-procedure extraction", [&]() {
            return !copperfin::vfp::extract_dbc_stored_procedures_source(
                        candidate.string()).ok;
        });
        expect_prompt_rejection(kind + " JSON export", [&]() {
            return !copperfin::vfp::export_database_as_json(candidate.string()).ok;
        });
        expect_prompt_rejection(kind + " SQL export", [&]() {
            return !copperfin::vfp::export_database_as_sql(candidate.string()).ok;
        });
    }

    fs::remove_all(directory, ignored);
}

#endif

}  // namespace

int main() {
    test_regular_dbc_control();
#if !defined(_WIN32)
    test_fifo_dbc_paths_are_rejected_promptly();
#endif

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
