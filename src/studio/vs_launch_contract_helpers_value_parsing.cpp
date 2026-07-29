// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "vs_launch_contract_internal.h"

#include <mutex>

namespace copperfin::studio {

bool parse_size_value(const std::string& text, std::size_t& value) {
    const char* begin = text.data();
    const char* end = text.data() + text.size();
    const auto result = std::from_chars(begin, end, value);
    return result.ec == std::errc{} && result.ptr == end;
}

bool parse_int_value(const std::string& text, int& value) {
    const char* begin = text.data();
    const char* end = text.data() + text.size();
    const auto result = std::from_chars(begin, end, value);
    return result.ec == std::errc{} && result.ptr == end;
}

bool parse_double_value(const std::string& text, double& value) {
    if (text.empty() || std::isspace(static_cast<unsigned char>(text.front()))) {
        return false;
    }
    const char* begin = text.data();
    const char* end = begin + text.size();
    if (*begin == '+') {
        ++begin;
    }
    if (begin == end) {
        return false;
    }

    double parsed = 0.0;
    const auto result = std::from_chars(begin, end, parsed, std::chars_format::general);
    if (result.ec != std::errc{} || result.ptr != end || !std::isfinite(parsed)) {
        return false;
    }
    value = parsed;
    return true;
}

std::string lowercase_copy(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

std::optional<bool> parse_bool_value(std::string text) {
    text = lowercase_copy(std::move(text));
    if (text == "true" || text == ".t." || text == "t" || text == "1" || text == "yes" || text == "on") {
        return true;
    }
    if (text == "false" || text == ".f." || text == "f" || text == "0" || text == "no" || text == "off") {
        return false;
    }
    return std::nullopt;
}

std::optional<StudioEditorSelectionContext> parse_selection_context_token(std::string token) {
    token = lowercase_copy(std::move(token));
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::visual_object)) {
        return StudioEditorSelectionContext::visual_object;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::visual_method)) {
        return StudioEditorSelectionContext::visual_method;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::container_object)) {
        return StudioEditorSelectionContext::container_object;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::class_designer)) {
        return StudioEditorSelectionContext::class_designer;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::report_expression)) {
        return StudioEditorSelectionContext::report_expression;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::label_expression)) {
        return StudioEditorSelectionContext::label_expression;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::menu_item)) {
        return StudioEditorSelectionContext::menu_item;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::project_item)) {
        return StudioEditorSelectionContext::project_item;
    }
    if (token == studio_editor_selection_context_name(StudioEditorSelectionContext::data_environment)) {
        return StudioEditorSelectionContext::data_environment;
    }
    return std::nullopt;
}

const localization::LocalizedCatalog& default_launch_catalog() {
    struct CatalogCache {
        std::filesystem::path locale_root;
        std::string locale;
        localization::LocalizedCatalog catalog;
    };
    static std::mutex cache_mutex;
    static CatalogCache cache{
        {},
        {},
        localization::load_catalogs(
            localization::resolve_catalog_root(),
            localization::default_locale)};
    const auto locale_root = localization::resolve_catalog_root();
    const auto locale = localization::select_locale();
    std::lock_guard<std::mutex> lock(cache_mutex);
    if (cache.locale_root != locale_root || cache.locale != locale) {
        cache.locale_root = locale_root;
        cache.locale = locale;
        cache.catalog = localization::load_catalogs(locale_root, locale);
    }
    return cache.catalog;
}

}  // namespace copperfin::studio
