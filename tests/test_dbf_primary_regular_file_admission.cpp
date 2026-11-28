// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_header.h"
#include "copperfin/vfp/dbf_table.h"
#include "prg_engine_test_support.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

#if !defined(_WIN32)
#include <csignal>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace {

namespace fs = std::filesystem;
using namespace copperfin::test_support;

fs::path unique_test_directory() {
    const auto nonce = std::chrono::steady_clock::now().time_since_epoch().count();
    return fs::temp_directory_path() /
        ("copperfin_dbf_primary_regular_file_admission_" + std::to_string(nonce));
}

bool use_is_rejected(const fs::path& directory, const fs::path& table_path) {
    const fs::path program_path = directory / "use_primary.prg";
    write_text(
        program_path,
        "USE '" + table_path.filename().string() + "'\n"
        "? RECCOUNT()\n"
        "USE\n"
        "RETURN\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(program_path, directory, false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    return !state.completed;
}

void expect_regular_controls(const fs::path& directory, const fs::path& table_path) {
    const auto table = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 10U);
    expect(table.ok && table.table.records.size() == 1U,
           "regular DBF table parsing should preserve one record: " + table.error);
    const auto zero_records = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 0U);
    expect(zero_records.ok && zero_records.table.records.empty(),
           "regular DBF max_records=0 parsing should remain valid: " + zero_records.error);
    const auto header = copperfin::vfp::parse_dbf_header_from_file(table_path.string());
    expect(header.ok, "regular DBF header parsing should remain valid: " + header.error);
    const auto fields = copperfin::vfp::parse_dbf_fields_from_file(table_path.string());
    expect(fields.ok && fields.fields.size() == 1U,
           "regular DBF fields-only parsing should remain valid: " + fields.error);

    const fs::path program_path = directory / "use_regular.prg";
    write_text(
        program_path,
        "USE '" + table_path.filename().string() + "'\n"
        "? RECCOUNT()\n"
        "USE\n"
        "RETURN\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(program_path, directory, false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PRG USE should preserve a regular DBF: " + state.message);
}

void test_regular_dbf_controls() {
    const fs::path directory = unique_test_directory();
    std::error_code ignored;
    fs::create_directories(directory, ignored);
    expect(!ignored, "regular-control directory creation should succeed");
    const fs::path table_path = directory / "regular.dbf";
    write_simple_dbf(table_path, {"ONE"});
    expect_regular_controls(directory, table_path);

#if !defined(_WIN32)
    const fs::path regular_symlink = directory / "regular-link.dbf";
    fs::create_symlink(table_path.filename(), regular_symlink, ignored);
    expect(!ignored, "regular DBF symlink creation should succeed");
    if (!ignored) {
        expect_regular_controls(directory, regular_symlink);
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

void test_fifo_primary_paths_are_rejected_promptly() {
    const fs::path directory = unique_test_directory();
    std::error_code ignored;
    fs::create_directories(directory, ignored);
    expect(!ignored, "FIFO-test directory creation should succeed");

    const fs::path fifo_path = directory / "direct.dbf";
    const fs::path fifo_symlink = directory / "linked.dbf";
    expect(::mkfifo(fifo_path.c_str(), 0600) == 0, "test FIFO creation should succeed");
    fs::create_symlink(fifo_path.filename(), fifo_symlink, ignored);
    expect(!ignored, "FIFO symlink creation should succeed");

    for (const fs::path& candidate : {fifo_path, fifo_symlink}) {
        const std::string kind = candidate == fifo_path ? "direct FIFO" : "symlink-to-FIFO";
        expect_prompt_rejection(kind + " table parse", [&]() {
            return !copperfin::vfp::parse_dbf_table_from_file(candidate.string(), 10U).ok;
        });
        expect_prompt_rejection(kind + " max_records=0 table parse", [&]() {
            return !copperfin::vfp::parse_dbf_table_from_file(candidate.string(), 0U).ok;
        });
        expect_prompt_rejection(kind + " header parse", [&]() {
            return !copperfin::vfp::parse_dbf_header_from_file(candidate.string()).ok;
        });
        expect_prompt_rejection(kind + " fields-only parse", [&]() {
            return !copperfin::vfp::parse_dbf_fields_from_file(candidate.string()).ok;
        });
        expect_prompt_rejection(kind + " PRG USE", [&]() {
            return use_is_rejected(directory, candidate);
        });
    }

    fs::remove_all(directory, ignored);
}

#else

void test_windows_nonregular_control() {
    const fs::path directory = unique_test_directory();
    std::error_code ignored;
    fs::create_directories(directory / "directory.dbf", ignored);
    expect(!ignored, "Windows nonregular-control directory creation should succeed");
    const fs::path candidate = directory / "directory.dbf";
    expect(!copperfin::vfp::parse_dbf_table_from_file(candidate.string(), 0U).ok,
           "Windows table parser should reject a directory primary path");
    expect(!copperfin::vfp::parse_dbf_header_from_file(candidate.string()).ok,
           "Windows header parser should reject a directory primary path");
    expect(!copperfin::vfp::parse_dbf_fields_from_file(candidate.string()).ok,
           "Windows fields-only parser should reject a directory primary path");
    expect(use_is_rejected(directory, candidate),
           "Windows PRG USE should reject a directory primary path");
    fs::remove_all(directory, ignored);
}

#endif

}  // namespace

int main() {
    test_regular_dbf_controls();
#if !defined(_WIN32)
    test_fifo_primary_paths_are_rejected_promptly();
#else
    test_windows_nonregular_control();
#endif

    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
