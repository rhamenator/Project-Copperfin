// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_environment_support.h"
#include "test_process_capture_support.h"

#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#endif

namespace {

int failures = 0;

void expect(const bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

const std::vector<std::string>& probe_arguments() {
    static const std::vector<std::string> arguments{
        "",
        "space value",
        "quote\"value",
        "slashes\\\\tail\\",
        "line1\nline2\r\nline3",
        "&|<>%^!",
        "Espa\xC3\xB1ol-\xD0\xA0\xD1\x83\xD1\x81\xD1\x81\xD0\xBA\xD0\xB8\xD0\xB9"
    };
    return arguments;
}

int run_probe(
    const std::filesystem::path& expected_directory,
    const std::vector<std::string>& actual_arguments) {
    const auto& expected = probe_arguments();
    if (actual_arguments.size() != expected.size()) {
        return 91;
    }
    std::error_code error;
    if (!std::filesystem::equivalent(
            std::filesystem::current_path(),
            expected_directory,
            error) || error) {
        return 92;
    }
    for (std::size_t index = 0U; index < expected.size(); ++index) {
        if (actual_arguments[index] != expected[index]) {
            return static_cast<int>(100U + index);
        }
    }

#if defined(_WIN32)
    if (_setmode(_fileno(stdout), _O_BINARY) == -1 ||
        _setmode(_fileno(stderr), _O_BINARY) == -1) {
        return 93;
    }
#endif
    const std::string stdout_bytes{"stdout\r\nline\r\0tail", 18U};
    const std::string stderr_bytes{"stderr\nline\0tail", 16U};
    std::cout.write(stdout_bytes.data(), static_cast<std::streamsize>(stdout_bytes.size()));
    std::cerr.write(stderr_bytes.data(), static_cast<std::streamsize>(stderr_bytes.size()));
    return 37;
}

int run_parent(const std::filesystem::path& source_executable) {
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() /
        copperfin::test_support::path_from_utf8_string(
            "copperfin-process-capture-\xD0\xBF\xD1\x80\xD0\xBE\xD0\xB1\xD0\xB0");
    std::error_code error;
    fs::remove_all(root, error);
    error.clear();
    fs::create_directories(root, error);
    expect(!error, "#4081: Unicode process working directory should be created");

    fs::path copied_executable = root /
        copperfin::test_support::path_from_utf8_string(
            "capture-\xD1\x82\xD0\xB5\xD1\x81\xD1\x82");
#if defined(_WIN32)
    copied_executable += L".exe";
#endif
    fs::copy_file(source_executable, copied_executable, fs::copy_options::overwrite_existing, error);
    expect(!error, "#4081: process probe should copy to a Unicode executable path");
#if !defined(_WIN32)
    error.clear();
    fs::permissions(
        copied_executable,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add,
        error);
    expect(!error, "#4081: copied process probe should remain executable");
#endif

    std::vector<std::string> arguments{
        "--probe",
        copperfin::test_support::path_to_utf8_string(root)
    };
    const auto& expected_arguments = probe_arguments();
    arguments.insert(arguments.end(), expected_arguments.begin(), expected_arguments.end());
    const auto result = copperfin::test_support::run_process_capture(
        copied_executable,
        arguments,
        root);
    expect(result.started && result.launch_error == 0,
           "#4081: direct process probe should start without a shell");
    expect(result.exit_code == 37,
           "#4081: direct process probe should preserve its exact exit status");
    expect(result.stdout_text == std::string("stdout\r\nline\r\0tail", 18U),
           "#4081: direct process probe should preserve stdout bytes");
    expect(result.stderr_text == std::string("stderr\nline\0tail", 16U),
           "#4081: direct process probe should preserve stderr bytes");

    const auto normalized =
        copperfin::test_support::normalize_captured_process_line_endings(result);
    expect(normalized.stdout_text == std::string("stdout\nline\n\0tail", 17U),
           "#4080: Studio-host observation should normalize CRLF and lone CR bytes");
    expect(normalized.stderr_text == result.stderr_text,
           "#4080: Studio-host observation should preserve existing LF and binary bytes");
    expect(result.stdout_text == std::string("stdout\r\nline\r\0tail", 18U),
           "#4081: observation normalization should not mutate raw captured bytes");

    const auto missing = copperfin::test_support::run_process_capture(
        root / "missing-process",
        {},
        root);
    expect(!missing.started && missing.exit_code == -1 && missing.launch_error != 0,
           "#4081: a missing executable should report launch failure separately");

    fs::remove_all(root, error);
    return failures == 0 ? 0 : 1;
}

}  // namespace

#if defined(_WIN32)
int wmain(int argc, wchar_t** argv) {
    if (argc >= 2 && std::wstring(argv[1]) == L"--probe") {
        if (argc < 3) {
            return 91;
        }
        std::vector<std::string> actual_arguments;
        actual_arguments.reserve(static_cast<std::size_t>(argc - 3));
        for (int index = 3; index < argc; ++index) {
            actual_arguments.push_back(
                copperfin::test_support::path_to_utf8_string(std::filesystem::path(argv[index])));
        }
        return run_probe(std::filesystem::path(argv[2]), actual_arguments);
    }
    return run_parent(std::filesystem::absolute(std::filesystem::path(argv[0])));
}
#else
int main(int argc, char** argv) {
    if (argc >= 2 && std::string(argv[1]) == "--probe") {
        if (argc < 3) {
            return 91;
        }
        std::vector<std::string> actual_arguments(argv + 3, argv + argc);
        return run_probe(
            copperfin::test_support::path_from_utf8_string(argv[2]),
            actual_arguments);
    }
    return run_parent(std::filesystem::absolute(
        copperfin::test_support::path_from_utf8_string(argv[0])));
}
#endif
