// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/font_directories.h"

#include "copperfin/platform/environment.h"
#include "copperfin/platform/path.h"

namespace copperfin::platform {

std::vector<std::filesystem::path> font_search_directories() {
    namespace fs = std::filesystem;
    std::vector<fs::path> roots;
#if defined(_WIN32)
    if (const auto windir = read_environment_variable("WINDIR"); windir.has_value()) {
        roots.emplace_back(path_from_utf8_string(*windir) / "Fonts");
    }
    if (roots.empty()) {
        roots.emplace_back(L"C:\\Windows\\Fonts");
    }
#elif defined(__APPLE__)
    roots.emplace_back("/System/Library/Fonts");
    roots.emplace_back("/Library/Fonts");
    if (const auto home = read_environment_variable("HOME"); home.has_value()) {
        roots.emplace_back(path_from_utf8_string(*home) / "Library" / "Fonts");
    }
#else
    roots.emplace_back("/usr/share/fonts");
    roots.emplace_back("/usr/local/share/fonts");
    if (const auto home = read_environment_variable("HOME"); home.has_value()) {
        const fs::path home_path = path_from_utf8_string(*home);
        roots.emplace_back(home_path / ".fonts");
        roots.emplace_back(home_path / ".local" / "share" / "fonts");
    }
#endif
    return roots;
}

}  // namespace copperfin::platform
