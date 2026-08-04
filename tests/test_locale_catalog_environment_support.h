// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#ifndef COPPERFIN_TEST_LOCALE_CATALOG_ENVIRONMENT_SUPPORT_H
#define COPPERFIN_TEST_LOCALE_CATALOG_ENVIRONMENT_SUPPORT_H

#include "test_environment_support.h"

#include <filesystem>
#include <fstream>
#include <string>

namespace copperfin::test_support {

inline bool can_supply_default_locale_catalog(const std::filesystem::path& locale_dir) {
    if (locale_dir.empty()) {
        return false;
    }

    const std::filesystem::path catalog_path = locale_dir / "en-US" / "strings.json";
    std::error_code error;
    if (!std::filesystem::is_regular_file(catalog_path, error)) {
        return false;
    }
    std::ifstream input(catalog_path, std::ios::binary);
    return input.good();
}

inline std::filesystem::path find_locale_catalog_dir_from_cwd() {
    std::error_code error;
    std::filesystem::path ancestor = std::filesystem::current_path(error);
    if (error) {
        return (std::filesystem::path("resources") / "locales").lexically_normal();
    }
    ancestor = std::filesystem::absolute(ancestor, error);
    if (error) {
        return (std::filesystem::path("resources") / "locales").lexically_normal();
    }
    for (;;) {
        const auto candidate = ancestor / "resources" / "locales";
        if (can_supply_default_locale_catalog(candidate)) {
            return candidate.lexically_normal();
        }
        const auto parent = ancestor.parent_path();
        if (parent == ancestor) {
            return candidate.lexically_normal();
        }
        ancestor = parent;
    }
}

inline std::filesystem::path configured_test_locale_catalog_dir() {
#if defined(COPPERFIN_TEST_LOCALE_DIR)
    return path_from_utf8_string(COPPERFIN_TEST_LOCALE_DIR).lexically_normal();
#else
    return find_locale_catalog_dir_from_cwd();
#endif
}

struct ScopedTestLocaleCatalogDirectory {
    ScopedEnvironmentPath locale_dir;

    ScopedTestLocaleCatalogDirectory()
        : locale_dir("COPPERFIN_LOCALE_DIR", false) {
        if (!can_supply_default_locale_catalog(getenv_path("COPPERFIN_LOCALE_DIR"))) {
            locale_dir.set(configured_test_locale_catalog_dir());
        }
    }
};

struct ScopedDefaultLocaleCatalogEnvironment {
    ScopedEnvironmentValue locale;
    ScopedEnvironmentPath locale_dir;

    ScopedDefaultLocaleCatalogEnvironment()
        : locale("COPPERFIN_LOCALE", false),
          locale_dir("COPPERFIN_LOCALE_DIR", false) {
        locale.set("en-US");
        if (!can_supply_default_locale_catalog(getenv_path("COPPERFIN_LOCALE_DIR"))) {
            locale_dir.set(configured_test_locale_catalog_dir());
        }
    }
};

}  // namespace copperfin::test_support

#endif
