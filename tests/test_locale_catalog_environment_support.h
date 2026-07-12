// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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

struct ScopedDefaultLocaleCatalogEnvironment {
    ScopedEnvironmentValue locale;
    ScopedEnvironmentPath locale_dir;

    ScopedDefaultLocaleCatalogEnvironment()
        : locale("COPPERFIN_LOCALE", false),
          locale_dir("COPPERFIN_LOCALE_DIR", false) {
        locale.set("en-US");
        if (!can_supply_default_locale_catalog(getenv_path("COPPERFIN_LOCALE_DIR"))) {
            locale_dir.set(find_locale_catalog_dir_from_cwd());
        }
    }
};

}  // namespace copperfin::test_support

#endif
