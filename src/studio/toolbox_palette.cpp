// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/studio/toolbox_palette.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iterator>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

namespace copperfin::studio {

namespace {

bool supports_context(const StudioToolboxItemDescriptor& item, StudioToolboxContext context) {
    return std::find(item.contexts.begin(), item.contexts.end(), context) != item.contexts.end();
}

std::string lowercase_copy(std::string_view value) {
    std::string result;
    result.reserve(value.size());
    for (const char ch : value) {
        result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
    }
    return result;
}

bool contains_case_insensitive(std::string_view value, const std::string& lowered_needle) {
    if (lowered_needle.empty()) {
        return true;
    }
    return lowercase_copy(value).find(lowered_needle) != std::string::npos;
}

copperfin::localization::LocalizedCatalog toolbox_palette_catalog() {
    struct CatalogCache {
        std::filesystem::path locale_root;
        std::string locale;
        copperfin::localization::LocalizedCatalog catalog;
    };

    static std::mutex cache_mutex;
    static CatalogCache cache{
        {},
        {},
        copperfin::localization::load_catalogs(
            copperfin::localization::resolve_catalog_root(),
            copperfin::localization::default_locale)};
    const std::filesystem::path locale_root = copperfin::localization::resolve_catalog_root();
    const std::string locale = copperfin::localization::select_locale();
    std::lock_guard<std::mutex> lock(cache_mutex);
    if (cache.locale_root != locale_root || cache.locale != locale) {
        cache.locale_root = locale_root;
        cache.locale = locale;
        cache.catalog = copperfin::localization::load_catalogs(locale_root, locale);
    }
    return cache.catalog;
}

std::string toolbox_palette_text(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key) {
    return catalog.translate(key);
}

std::string toolbox_palette_text(std::string_view key) {
    return toolbox_palette_text(toolbox_palette_catalog(), key);
}

bool matches_category(const StudioToolboxItemDescriptor& item, const std::string& lowered_category) {
    return lowered_category.empty() || lowercase_copy(item.category) == lowered_category;
}

bool matches_search_text(const StudioToolboxItemDescriptor& item, const std::string& lowered_search_text) {
    return lowered_search_text.empty() ||
        contains_case_insensitive(item.id, lowered_search_text) ||
        contains_case_insensitive(item.title, lowered_search_text) ||
        contains_case_insensitive(item.category, lowered_search_text) ||
        contains_case_insensitive(item.vfp_class, lowered_search_text) ||
        contains_case_insensitive(item.base_class, lowered_search_text) ||
        contains_case_insensitive(item.description, lowered_search_text);
}

std::optional<StudioToolboxContext> toolbox_context_for_selection_context(
    StudioEditorSelectionContext context) {
    switch (context) {
        case StudioEditorSelectionContext::visual_object:
        case StudioEditorSelectionContext::visual_method:
            return StudioToolboxContext::form;
        case StudioEditorSelectionContext::container_object:
            return StudioToolboxContext::container;
        case StudioEditorSelectionContext::class_designer:
            return StudioToolboxContext::class_designer;
        case StudioEditorSelectionContext::report_expression:
        case StudioEditorSelectionContext::label_expression:
            return StudioToolboxContext::report;
        case StudioEditorSelectionContext::menu_item:
        case StudioEditorSelectionContext::project_item:
        case StudioEditorSelectionContext::data_environment:
            return std::nullopt;
    }
    return std::nullopt;
}

std::vector<StudioEditorSelectionContext> all_toolbox_palette_selection_contexts() {
    return {
        StudioEditorSelectionContext::visual_object,
        StudioEditorSelectionContext::visual_method,
        StudioEditorSelectionContext::container_object,
        StudioEditorSelectionContext::class_designer,
        StudioEditorSelectionContext::report_expression,
        StudioEditorSelectionContext::label_expression,
        StudioEditorSelectionContext::menu_item,
        StudioEditorSelectionContext::project_item,
        StudioEditorSelectionContext::data_environment
    };
}

}  // namespace

const char* studio_toolbox_context_name(StudioToolboxContext context) {
    switch (context) {
        case StudioToolboxContext::form:
            return "form";
        case StudioToolboxContext::class_designer:
            return "class_designer";
        case StudioToolboxContext::container:
            return "container";
        case StudioToolboxContext::report:
            return "report";
    }
    return "form";
}

std::vector<StudioToolboxItemDescriptor> studio_toolbox_palette_for_catalog(
    const copperfin::localization::LocalizedCatalog& catalog) {
    using Context = StudioToolboxContext;
    return {
        {
            .id = "label",
            .title = toolbox_palette_text(catalog, "Studio.Toolbox.Item.Label.Title"),
            .category = toolbox_palette_text(catalog, "Studio.Toolbox.Category.StandardControls"),
            .vfp_class = "Label",
            .base_class = "Label",
            .default_name_prefix = "lbl",
            .contexts = {Context::form, Context::class_designer, Context::container, Context::report},
            .container = false,
            .description = toolbox_palette_text(catalog, "Studio.Toolbox.Item.Label.Description")
        },
        {
            .id = "textbox",
            .title = toolbox_palette_text(catalog, "Studio.Toolbox.Item.TextBox.Title"),
            .category = toolbox_palette_text(catalog, "Studio.Toolbox.Category.StandardControls"),
            .vfp_class = "TextBox",
            .base_class = "TextBox",
            .default_name_prefix = "txt",
            .contexts = {Context::form, Context::class_designer, Context::container},
            .container = false,
            .description = toolbox_palette_text(catalog, "Studio.Toolbox.Item.TextBox.Description")
        },
        {
            .id = "editbox",
            .title = toolbox_palette_text(catalog, "Studio.Toolbox.Item.EditBox.Title"),
            .category = toolbox_palette_text(catalog, "Studio.Toolbox.Category.StandardControls"),
            .vfp_class = "EditBox",
            .base_class = "EditBox",
            .default_name_prefix = "edt",
            .contexts = {Context::form, Context::class_designer, Context::container},
            .container = false,
            .description = toolbox_palette_text(catalog, "Studio.Toolbox.Item.EditBox.Description")
        },
        {
            .id = "commandbutton",
            .title = toolbox_palette_text(catalog, "Studio.Toolbox.Item.CommandButton.Title"),
            .category = toolbox_palette_text(catalog, "Studio.Toolbox.Category.StandardControls"),
            .vfp_class = "CommandButton",
            .base_class = "CommandButton",
            .default_name_prefix = "cmd",
            .contexts = {Context::form, Context::class_designer, Context::container},
            .container = false,
            .description = toolbox_palette_text(catalog, "Studio.Toolbox.Item.CommandButton.Description")
        },
        {
            .id = "checkbox",
            .title = toolbox_palette_text(catalog, "Studio.Toolbox.Item.CheckBox.Title"),
            .category = toolbox_palette_text(catalog, "Studio.Toolbox.Category.StandardControls"),
            .vfp_class = "CheckBox",
            .base_class = "CheckBox",
            .default_name_prefix = "chk",
            .contexts = {Context::form, Context::class_designer, Context::container},
            .container = false,
            .description = toolbox_palette_text(catalog, "Studio.Toolbox.Item.CheckBox.Description")
        },
        {
            .id = "combobox",
            .title = toolbox_palette_text(catalog, "Studio.Toolbox.Item.ComboBox.Title"),
            .category = toolbox_palette_text(catalog, "Studio.Toolbox.Category.ListControls"),
            .vfp_class = "ComboBox",
            .base_class = "ComboBox",
            .default_name_prefix = "cbo",
            .contexts = {Context::form, Context::class_designer, Context::container},
            .container = false,
            .description = toolbox_palette_text(catalog, "Studio.Toolbox.Item.ComboBox.Description")
        },
        {
            .id = "listbox",
            .title = toolbox_palette_text(catalog, "Studio.Toolbox.Item.ListBox.Title"),
            .category = toolbox_palette_text(catalog, "Studio.Toolbox.Category.ListControls"),
            .vfp_class = "ListBox",
            .base_class = "ListBox",
            .default_name_prefix = "lst",
            .contexts = {Context::form, Context::class_designer, Context::container},
            .container = false,
            .description = toolbox_palette_text(catalog, "Studio.Toolbox.Item.ListBox.Description")
        },
        {
            .id = "grid",
            .title = toolbox_palette_text(catalog, "Studio.Toolbox.Item.Grid.Title"),
            .category = toolbox_palette_text(catalog, "Studio.Toolbox.Category.DataControls"),
            .vfp_class = "Grid",
            .base_class = "Grid",
            .default_name_prefix = "grd",
            .contexts = {Context::form, Context::class_designer, Context::container},
            .container = true,
            .description = toolbox_palette_text(catalog, "Studio.Toolbox.Item.Grid.Description")
        },
        {
            .id = "image",
            .title = toolbox_palette_text(catalog, "Studio.Toolbox.Item.Image.Title"),
            .category = toolbox_palette_text(catalog, "Studio.Toolbox.Category.Graphics"),
            .vfp_class = "Image",
            .base_class = "Image",
            .default_name_prefix = "img",
            .contexts = {Context::form, Context::class_designer, Context::container, Context::report},
            .container = false,
            .description = toolbox_palette_text(catalog, "Studio.Toolbox.Item.Image.Description")
        },
        {
            .id = "line",
            .title = toolbox_palette_text(catalog, "Studio.Toolbox.Item.Line.Title"),
            .category = toolbox_palette_text(catalog, "Studio.Toolbox.Category.Graphics"),
            .vfp_class = "Line",
            .base_class = "Line",
            .default_name_prefix = "lin",
            .contexts = {Context::form, Context::class_designer, Context::container, Context::report},
            .container = false,
            .description = toolbox_palette_text(catalog, "Studio.Toolbox.Item.Line.Description")
        },
        {
            .id = "shape",
            .title = toolbox_palette_text(catalog, "Studio.Toolbox.Item.Shape.Title"),
            .category = toolbox_palette_text(catalog, "Studio.Toolbox.Category.Graphics"),
            .vfp_class = "Shape",
            .base_class = "Shape",
            .default_name_prefix = "shp",
            .contexts = {Context::form, Context::class_designer, Context::container, Context::report},
            .container = false,
            .description = toolbox_palette_text(catalog, "Studio.Toolbox.Item.Shape.Description")
        },
        {
            .id = "container",
            .title = toolbox_palette_text(catalog, "Studio.Toolbox.Item.Container.Title"),
            .category = toolbox_palette_text(catalog, "Studio.Toolbox.Category.Containers"),
            .vfp_class = "Container",
            .base_class = "Container",
            .default_name_prefix = "cnt",
            .contexts = {Context::form, Context::class_designer, Context::container},
            .container = true,
            .description = toolbox_palette_text(catalog, "Studio.Toolbox.Item.Container.Description")
        },
        {
            .id = "pageframe",
            .title = toolbox_palette_text(catalog, "Studio.Toolbox.Item.PageFrame.Title"),
            .category = toolbox_palette_text(catalog, "Studio.Toolbox.Category.Containers"),
            .vfp_class = "PageFrame",
            .base_class = "PageFrame",
            .default_name_prefix = "pgf",
            .contexts = {Context::form, Context::class_designer, Context::container},
            .container = true,
            .description = toolbox_palette_text(catalog, "Studio.Toolbox.Item.PageFrame.Description")
        },
        {
            .id = "olecontrol",
            .title = toolbox_palette_text(catalog, "Studio.Toolbox.Item.OLEControl.Title"),
            .category = toolbox_palette_text(catalog, "Studio.Toolbox.Category.Interop"),
            .vfp_class = "OLEControl",
            .base_class = "OLEControl",
            .default_name_prefix = "ole",
            .contexts = {Context::form, Context::class_designer, Context::container},
            .container = false,
            .description = toolbox_palette_text(catalog, "Studio.Toolbox.Item.OLEControl.Description")
        }
    };
}

std::vector<StudioToolboxItemDescriptor> studio_toolbox_palette() {
    struct PaletteCache {
        std::filesystem::path locale_root;
        std::string locale;
        std::vector<StudioToolboxItemDescriptor> items;
    };

    const auto catalog = toolbox_palette_catalog();
    const std::filesystem::path locale_root = copperfin::localization::resolve_catalog_root();
    const std::string locale = copperfin::localization::select_locale();
    static std::mutex cache_mutex;
    static PaletteCache cache{};
    std::lock_guard<std::mutex> lock(cache_mutex);
    if (cache.locale_root != locale_root || cache.locale != locale) {
        cache.locale_root = locale_root;
        cache.locale = locale;
        cache.items = studio_toolbox_palette_for_catalog(catalog);
    }
    return cache.items;
}

std::vector<StudioToolboxItemDescriptor> studio_toolbox_items_for_context(StudioToolboxContext context) {
    std::vector<StudioToolboxItemDescriptor> filtered;
    const auto& items = studio_toolbox_palette();
    std::copy_if(items.begin(), items.end(), std::back_inserter(filtered), [&](const StudioToolboxItemDescriptor& item) {
        return supports_context(item, context);
    });
    return filtered;
}

StudioToolboxPaletteQueryResult query_studio_toolbox_palette(
    const StudioToolboxPaletteQueryRequest& request) {
    const std::string lowered_search_text = lowercase_copy(request.search_text);
    const std::string lowered_category = lowercase_copy(request.category);

    std::vector<StudioToolboxItemDescriptor> filtered;
    const auto& items = studio_toolbox_palette();
    std::copy_if(items.begin(), items.end(), std::back_inserter(filtered), [&](const StudioToolboxItemDescriptor& item) {
        return supports_context(item, request.toolbox_context) &&
            matches_category(item, lowered_category) &&
            matches_search_text(item, lowered_search_text);
    });

    const auto item_count = filtered.size();
    return {
        .ok = true,
        .error = {},
        .toolbox_context = request.toolbox_context,
        .search_text = request.search_text,
        .category = request.category,
        .item_count = item_count,
        .dry_run = true,
        .mutates_asset = false,
        .items = std::move(filtered)
    };
}

StudioToolboxPaletteLaunchPlanResult plan_studio_toolbox_palette_launch(
    const StudioToolboxPaletteLaunchRequest& request) {
    const auto toolbox_context = toolbox_context_for_selection_context(request.selection_context);
    if (!toolbox_context.has_value()) {
        return {
            .ok = false,
            .error = toolbox_palette_text("Studio.ToolboxPalette.Error.ContextUnavailable"),
            .plan = {}
        };
    }

    auto items = studio_toolbox_items_for_context(*toolbox_context);
    if (items.empty()) {
        return {
            .ok = false,
            .error = toolbox_palette_text("Studio.ToolboxPalette.Error.NoItems"),
            .plan = {}
        };
    }

    const auto item_count = items.size();
    return {
        .ok = true,
        .error = {},
        .plan = {
            .selection_context = request.selection_context,
            .toolbox_context = *toolbox_context,
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .item_count = item_count,
            .items = std::move(items)
        }
    };
}

StudioToolboxPaletteLaunchCatalogResult plan_studio_toolbox_palette_launch_catalog(
    const StudioToolboxPaletteLaunchCatalogRequest& request) {
    const auto contexts = all_toolbox_palette_selection_contexts();
    std::vector<StudioToolboxPaletteLaunchCatalogEntry> entries;
    entries.reserve(contexts.size());

    std::size_t launch_plan_count = 0U;
    std::size_t error_count = 0U;

    for (const auto context : contexts) {
        auto launch_plan = plan_studio_toolbox_palette_launch({
            .selection_context = context,
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id
        });

        if (launch_plan.ok) {
            ++launch_plan_count;
        } else {
            ++error_count;
        }

        const bool toolbox_available = launch_plan.ok;
        const std::size_t item_count = toolbox_available ? launch_plan.plan.item_count : 0U;
        const std::string error = launch_plan.error;
        entries.push_back({
            .selection_context = context,
            .toolbox_available = toolbox_available,
            .item_count = item_count,
            .error = error,
            .launch_plan = std::move(launch_plan)
        });
    }

    return {
        .ok = true,
        .error = {},
        .context_count = contexts.size(),
        .launch_plan_count = launch_plan_count,
        .error_count = error_count,
        .dry_run = true,
        .mutates_asset = false,
        .entries = std::move(entries)
    };
}

}  // namespace copperfin::studio
