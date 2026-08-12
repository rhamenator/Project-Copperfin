// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/exclusive_file.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#if !defined(_WIN32)
#include <sys/stat.h>
#endif

namespace {

int failures = 0;

void expect(const bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

class TemporaryDirectory final {
public:
    TemporaryDirectory() {
        const auto unique_value = std::chrono::steady_clock::now().time_since_epoch().count();
        path_ = std::filesystem::temp_directory_path() /
            ("copperfin-platform-exclusive-file-" + std::to_string(unique_value));
        std::filesystem::create_directories(path_);
    }

    ~TemporaryDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(path_, ignored);
    }

    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

    [[nodiscard]] const std::filesystem::path& path() const noexcept { return path_; }

private:
    std::filesystem::path path_;
};

std::string read_bytes(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

void test_create_and_refuse_replacement() {
    const TemporaryDirectory temporary;
    const auto path = temporary.path() / "snapshot.bin";
    const std::string payload("Copperfin\0verified", 18U);
    expect(copperfin::platform::write_new_durable_file(path, payload),
           "#35: exclusive file write should create a new binary file");
    expect(read_bytes(path) == payload,
           "#35: exclusive file write should preserve every byte");
    expect(!copperfin::platform::write_new_durable_file(path, "replacement"),
           "#35: exclusive file write should refuse an existing path");
    expect(read_bytes(path) == payload,
           "#35: refusal should leave existing bytes unchanged");
#if !defined(_WIN32)
    struct stat metadata {};
    expect(::stat(path.c_str(), &metadata) == 0 && (metadata.st_mode & 0777) == 0600,
           "#35: POSIX exclusive files should be owner-readable and owner-writable only");
#endif
}

void test_empty_and_invalid_destinations() {
    const TemporaryDirectory temporary;
    const auto empty_path = temporary.path() / "empty.bin";
    expect(copperfin::platform::write_new_durable_file(empty_path, {}),
           "#35: exclusive file write should durably create an empty file");
    expect(std::filesystem::file_size(empty_path) == 0U,
           "#35: empty exclusive file should remain empty");
    expect(!copperfin::platform::write_new_durable_file(
               temporary.path() / "missing" / "snapshot.bin", "bytes"),
           "#35: exclusive file write should not create parent directories");
    expect(!copperfin::platform::write_new_durable_file(temporary.path(), "bytes"),
           "#35: exclusive file write should reject a directory destination");
}

#if !defined(_WIN32)
void test_symlink_destination_is_not_followed() {
    const TemporaryDirectory temporary;
    const auto target = temporary.path() / "target.bin";
    const auto link = temporary.path() / "snapshot.bin";
    {
        std::ofstream output(target, std::ios::binary);
        output << "original";
    }
    std::filesystem::create_symlink(target.filename(), link);
    expect(!copperfin::platform::write_new_durable_file(link, "replacement"),
           "#35: POSIX exclusive file write should not follow a symlink destination");
    expect(read_bytes(target) == "original",
           "#35: symlink refusal should leave its target unchanged");
}
#endif

}  // namespace

int main() {
    test_create_and_refuse_replacement();
    test_empty_and_invalid_destinations();
#if !defined(_WIN32)
    test_symlink_destination_is_not_followed();
#endif
    if (failures == 0) {
        std::cout << "Platform exclusive-file tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
