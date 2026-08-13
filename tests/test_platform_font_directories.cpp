// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/environment.h"
#include "copperfin/platform/font_directories.h"
#include "copperfin/platform/path.h"

#include <filesystem>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace {
int failures = 0;
void expect(const bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

void test_host_font_search_order() {
    namespace fs = std::filesystem;
    const std::vector<fs::path> roots = copperfin::platform::font_search_directories();
    expect(!roots.empty(), "#35: host font search roots should never be empty");
    expect(std::set<fs::path>(roots.begin(), roots.end()).size() == roots.size(),
           "#35: host font search roots should not contain duplicates");
#if defined(_WIN32)
    const auto windir = copperfin::platform::read_environment_variable("WINDIR");
    const fs::path expected = windir.has_value()
        ? copperfin::platform::path_from_utf8_string(*windir) / "Fonts"
        : fs::path(L"C:\\Windows\\Fonts");
    expect(roots.size() == 1U && roots.front() == expected,
           "#35: Windows should search only the configured Windows Fonts directory");
#elif defined(__APPLE__)
    expect(roots.size() >= 2U && roots[0] == fs::path("/System/Library/Fonts") &&
               roots[1] == fs::path("/Library/Fonts"),
           "#35: macOS should preserve system then local font search order");
    const auto home = copperfin::platform::read_environment_variable("HOME");
    expect(roots.size() == (home.has_value() ? 3U : 2U),
           "#35: macOS user font search should follow HOME availability");
    if (home.has_value()) {
        expect(roots[2] == copperfin::platform::path_from_utf8_string(*home) / "Library" / "Fonts",
               "#35: macOS should append the current user's font directory");
    }
#else
    expect(roots.size() >= 2U && roots[0] == fs::path("/usr/share/fonts") &&
               roots[1] == fs::path("/usr/local/share/fonts"),
           "#35: POSIX should preserve system then local font search order");
    const auto home = copperfin::platform::read_environment_variable("HOME");
    expect(roots.size() == (home.has_value() ? 4U : 2U),
           "#35: POSIX user font search should follow HOME availability");
    if (home.has_value()) {
        const fs::path home_path = copperfin::platform::path_from_utf8_string(*home);
        expect(roots[2] == home_path / ".fonts" &&
                   roots[3] == home_path / ".local" / "share" / "fonts",
               "#35: POSIX should append both current-user font directories in order");
    }
#endif
}
}  // namespace

int main() {
    test_host_font_search_order();
    if (failures == 0) {
        std::cout << "Platform font-directory tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
