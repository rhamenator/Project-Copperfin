// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/access_saveastext_design.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
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
        ("copperfin_access_design_regular_file_admission_" + std::to_string(nonce));
}

void write_valid_design(const std::filesystem::path& path) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << "Version =19\r\n"
              "VersionRequired =19\r\n"
              "Checksum =1\r\n"
              "Begin Form\r\n"
              "End\r\n";
    expect(output.good(), "valid Access design fixture should be written");
}

void test_regular_and_failure_controls() {
    namespace fs = std::filesystem;
    const fs::path directory = unique_test_directory();
    std::error_code error;
    fs::create_directories(directory, error);
    expect(!error, "test directory creation should succeed");

    const fs::path regular_path = directory / "regular.txt";
    write_valid_design(regular_path);
    const auto regular =
        copperfin::vfp::parse_access_saveastext_design_from_file(regular_path.string());
    expect(regular.ok && regular.root.control_type == "Form",
           "regular Access design input should retain successful Form parsing: " + regular.error);

    const auto missing = copperfin::vfp::parse_access_saveastext_design_from_file(
        (directory / "missing.txt").string());
    expect(!missing.ok && !missing.error.empty(),
           "missing Access design input should retain a structured open failure");

    const fs::path directory_path = directory / "directory.txt";
    fs::create_directory(directory_path, error);
    expect(!error, "directory control creation should succeed");
    const auto directory_result =
        copperfin::vfp::parse_access_saveastext_design_from_file(directory_path.string());
    expect(!directory_result.ok && !directory_result.error.empty() &&
               directory_result.error != missing.error,
           "an opened directory should report a read failure, not an open failure");

#if !defined(_WIN32)
    const fs::path regular_symlink = directory / "regular-link.txt";
    fs::create_symlink(regular_path.filename(), regular_symlink, error);
    expect(!error, "regular-file symlink creation should succeed");
    if (!error) {
        const auto linked =
            copperfin::vfp::parse_access_saveastext_design_from_file(regular_symlink.string());
        expect(linked.ok && linked.root.control_type == "Form",
               "a symlink resolving to a regular Access design should remain accepted: " + linked.error);
    }
#endif

    fs::remove_all(directory, error);
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
        ::alarm(10U);
        _exit(rejected() ? EXIT_SUCCESS : EXIT_FAILURE);
    }

    int status = 0;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(12);
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

void test_fifo_paths_are_rejected_promptly() {
    namespace fs = std::filesystem;
    const fs::path directory = unique_test_directory();
    std::error_code error;
    fs::create_directories(directory, error);
    expect(!error, "FIFO-test directory creation should succeed");

    const fs::path fifo_path = directory / "design.fifo";
    const fs::path fifo_symlink = directory / "design-link.txt";
    expect(::mkfifo(fifo_path.c_str(), 0600) == 0, "test FIFO creation should succeed");
    fs::create_symlink(fifo_path.filename(), fifo_symlink, error);
    expect(!error, "FIFO symlink creation should succeed");

    for (const fs::path& candidate : {fifo_path, fifo_symlink}) {
        const std::string kind = candidate == fifo_path ? "direct FIFO" : "symlink-to-FIFO";
        expect_prompt_rejection(kind, [&]() {
            const auto result =
                copperfin::vfp::parse_access_saveastext_design_from_file(candidate.string());
            const auto directory_result =
                copperfin::vfp::parse_access_saveastext_design_from_file(directory.string());
            return !result.ok && !result.error.empty() &&
                result.error == directory_result.error;
        });
    }

    fs::remove_all(directory, error);
}

#endif

}  // namespace

int main() {
    test_regular_and_failure_controls();
#if !defined(_WIN32)
    test_fifo_paths_are_rejected_promptly();
#endif

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
