// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/disk_space.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

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
            ("copperfin-platform-disk-space-" + std::to_string(unique_value));
        std::filesystem::create_directories(path_);
    }

    ~TemporaryDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(path_, ignored);
    }

    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

    [[nodiscard]] const std::filesystem::path& path() const noexcept {
        return path_;
    }

private:
    std::filesystem::path path_;
};

void test_existing_directory_and_file() {
    const TemporaryDirectory temporary;
    const std::filesystem::path file_path = temporary.path() / "probe.bin";
    {
        std::ofstream output(file_path, std::ios::binary);
        output << "Copperfin";
    }

    const auto available = copperfin::platform::available_disk_bytes(temporary.path());
    expect(available.has_value() && *available > 0U,
           "existing directory should report positive available disk bytes");

    const auto directory_unit =
        copperfin::platform::disk_allocation_unit_bytes(temporary.path());
    const auto file_unit = copperfin::platform::disk_allocation_unit_bytes(file_path);
    expect(directory_unit.has_value() && *directory_unit > 0U,
           "existing directory should report a positive allocation-unit size");
    expect(file_unit == directory_unit,
           "file and containing directory should report the same allocation-unit size");
}

void test_missing_path_fails_closed() {
    const TemporaryDirectory temporary;
    const std::filesystem::path missing = temporary.path() / "missing";
    expect(!copperfin::platform::available_disk_bytes(missing).has_value(),
           "missing path should not report available disk bytes");
    expect(!copperfin::platform::disk_allocation_unit_bytes(missing).has_value(),
           "missing path should not report an allocation-unit size");
}

}  // namespace

int main() {
    test_existing_directory_and_file();
    test_missing_path_fails_closed();
    if (failures == 0) {
        std::cout << "Platform disk-space tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
