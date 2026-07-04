// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#ifndef COPPERFIN_TEST_LOCALE_CATALOG_ENVIRONMENT_SUPPORT_H
#define COPPERFIN_TEST_LOCALE_CATALOG_ENVIRONMENT_SUPPORT_H

#include "test_environment_support.h"

#include <filesystem>
#include <string>

namespace copperfin::test_support {

inline std::string find_locale_catalog_dir_from_cwd() {
    std::filesystem::path ancestor = std::filesystem::absolute(std::filesystem::current_path());
    for (;;) {
        const auto candidate = ancestor / "resources" / "locales";
        if (std::filesystem::exists(candidate)) {
            return candidate.lexically_normal().string();
        }
        const auto parent = ancestor.parent_path();
        if (parent == ancestor) {
            return candidate.lexically_normal().string();
        }
        ancestor = parent;
    }
}

struct ScopedDefaultLocaleCatalogEnvironment {
    ScopedEnvironmentValue locale;
    ScopedEnvironmentValue locale_dir;

    ScopedDefaultLocaleCatalogEnvironment()
        : locale("COPPERFIN_LOCALE"),
          locale_dir("COPPERFIN_LOCALE_DIR") {
        set_env_value("COPPERFIN_LOCALE", "en-US", true);
        set_env_value("COPPERFIN_LOCALE_DIR", find_locale_catalog_dir_from_cwd(), true);
    }
};

}  // namespace copperfin::test_support

#endif
