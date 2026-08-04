// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/localization/localization.h"
#include "copperfin/studio/context_editor_actions.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace copperfin::studio {

enum class StudioToolboxContext {
    form,
    class_designer,
    container,
    report
};

struct StudioToolboxItemDescriptor {
    std::string_view id;
    std::string title;
    std::string category;
    std::string_view vfp_class;
    std::string_view base_class;
    std::string_view default_name_prefix;
    std::vector<StudioToolboxContext> contexts;
    bool container = false;
    std::string description;
};

struct StudioToolboxPaletteLaunchRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
};

struct StudioToolboxPaletteLaunchPlan {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::size_t item_count = 0;
    std::vector<StudioToolboxItemDescriptor> items;
};

struct StudioToolboxPaletteQueryRequest {
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string search_text;
    std::string category;
};

struct StudioToolboxPaletteQueryResult {
    bool ok = false;
    std::string error;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string search_text;
    std::string category;
    std::size_t item_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioToolboxItemDescriptor> items;
};

struct StudioToolboxPaletteLaunchPlanResult {
    bool ok = false;
    std::string error;
    StudioToolboxPaletteLaunchPlan plan;
};

struct StudioToolboxPaletteLaunchCatalogRequest {
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
};

struct StudioToolboxPaletteLaunchCatalogEntry {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    bool toolbox_available = false;
    std::size_t item_count = 0;
    std::string error;
    StudioToolboxPaletteLaunchPlanResult launch_plan;
};

struct StudioToolboxPaletteLaunchCatalogResult {
    bool ok = false;
    std::string error;
    std::size_t context_count = 0;
    std::size_t launch_plan_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioToolboxPaletteLaunchCatalogEntry> entries;
};

[[nodiscard]] const char* studio_toolbox_context_name(StudioToolboxContext context);
[[nodiscard]] std::vector<StudioToolboxItemDescriptor> studio_toolbox_palette_for_catalog(
    const localization::LocalizedCatalog& catalog);
[[nodiscard]] std::vector<StudioToolboxItemDescriptor> studio_toolbox_palette();
[[nodiscard]] std::vector<StudioToolboxItemDescriptor> studio_toolbox_items_for_context(StudioToolboxContext context);
[[nodiscard]] StudioToolboxPaletteQueryResult query_studio_toolbox_palette(
    const StudioToolboxPaletteQueryRequest& request);
[[nodiscard]] StudioToolboxPaletteLaunchPlanResult plan_studio_toolbox_palette_launch(
    const StudioToolboxPaletteLaunchRequest& request);
[[nodiscard]] StudioToolboxPaletteLaunchCatalogResult plan_studio_toolbox_palette_launch_catalog(
    const StudioToolboxPaletteLaunchCatalogRequest& request);

}  // namespace copperfin::studio
