// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace copperfin::localization {

constexpr std::string_view default_locale = "en-US";
constexpr std::string_view pseudo_locale = "qps-ploc";

using PlaceholderMap = std::map<std::string, std::string>;

struct CatalogLoadResult {
    bool ok = false;
    std::string error;
    std::unordered_map<std::string, std::string> entries;
};

struct LocalizedCatalog {
    std::string requested_locale;
    std::vector<std::string> fallback_chain;
    std::map<std::string, std::unordered_map<std::string, std::string>> catalogs;
    std::vector<std::string> load_errors;

    [[nodiscard]] std::optional<std::string> find(std::string_view key) const;
    [[nodiscard]] std::string translate(
        std::string_view key,
        const PlaceholderMap& placeholders = {}) const;
};

[[nodiscard]] std::string normalize_locale(std::string_view locale);
[[nodiscard]] std::vector<std::string> locale_fallback_chain(std::string_view locale);
[[nodiscard]] std::string select_locale(std::string_view explicit_locale = {});
[[nodiscard]] CatalogLoadResult parse_catalog_json(std::string_view text);
[[nodiscard]] CatalogLoadResult load_catalog_file(const std::filesystem::path& path);
[[nodiscard]] LocalizedCatalog load_catalogs(
    const std::filesystem::path& locale_root,
    std::string_view requested_locale);
[[nodiscard]] std::filesystem::path resolve_catalog_root(const std::filesystem::path& executable_path = {});
[[nodiscard]] std::string format_named_placeholders(
    std::string_view pattern,
    const PlaceholderMap& placeholders);
[[nodiscard]] std::string pseudo_localize(std::string_view pattern);

}  // namespace copperfin::localization
