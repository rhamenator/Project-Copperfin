// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "localized_text.h"

#include <mutex>

namespace copperfin::runtime {

namespace {

thread_local const localization::LocalizedCatalog* active_catalog = nullptr;

}  // namespace

RuntimeCatalogScope::RuntimeCatalogScope(
    const localization::LocalizedCatalog* catalog) noexcept
    : catalog_(catalog), previous_(active_catalog) {
    // A null scope preserves an outer host-selected catalog for nested sessions.
    if (catalog != nullptr) {
        active_catalog = catalog;
    }
}

RuntimeCatalogScope::RuntimeCatalogScope(const RuntimeCatalogScope& other) noexcept
    : RuntimeCatalogScope(other.catalog_) {}

RuntimeCatalogScope::~RuntimeCatalogScope() {
    active_catalog = previous_;
}

std::string runtime_text(
    std::string_view key,
    const localization::PlaceholderMap& placeholders) {
    if (active_catalog != nullptr) {
        return active_catalog->translate(key, placeholders);
    }

    struct CatalogCache {
        std::filesystem::path locale_root;
        std::string locale;
        localization::LocalizedCatalog catalog;
    };

    static std::mutex cache_mutex;
    static CatalogCache cache{
        {},
        {},
        localization::load_catalogs(localization::resolve_catalog_root(), localization::default_locale)
    };

    const std::filesystem::path locale_root = localization::resolve_catalog_root();
    const std::string locale = localization::select_locale();

    std::lock_guard<std::mutex> lock(cache_mutex);
    if (cache.locale_root != locale_root || cache.locale != locale) {
        cache.locale_root = locale_root;
        cache.locale = locale;
        cache.catalog = localization::load_catalogs(locale_root, locale);
    }
    return cache.catalog.translate(key, placeholders);
}

}  // namespace copperfin::runtime
