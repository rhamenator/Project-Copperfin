// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include "copperfin/localization/localization.h"

#include <string>
#include <string_view>

namespace copperfin::runtime {

class RuntimeCatalogScope {
public:
    explicit RuntimeCatalogScope(const localization::LocalizedCatalog* catalog) noexcept;
    ~RuntimeCatalogScope();

    RuntimeCatalogScope(const RuntimeCatalogScope& other) noexcept;
    RuntimeCatalogScope& operator=(const RuntimeCatalogScope&) = delete;

private:
    const localization::LocalizedCatalog* catalog_ = nullptr;
    const localization::LocalizedCatalog* previous_ = nullptr;
};

std::string runtime_text(
    std::string_view key,
    const localization::PlaceholderMap& placeholders = {});

}  // namespace copperfin::runtime
