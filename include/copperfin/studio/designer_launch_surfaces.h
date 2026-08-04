// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "copperfin/studio/designer_context.h"

#include <cstddef>
#include <string>
#include <vector>

namespace copperfin::studio {

struct StudioDesignerLaunchSurfaceRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string symbol;
    std::size_t line = 0;
    std::size_t column = 0;
};

struct StudioDesignerLaunchSurfacePlan {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string symbol;
    std::size_t line = 0;
    std::size_t column = 0;
    std::size_t editor_action_launch_plan_count = 0;
    std::size_t builder_launch_plan_count = 0;
    bool toolbox_available = false;
    std::size_t toolbox_item_count = 0;
    std::string toolbox_error;
    std::vector<StudioEditorActionLaunchPlanResult> editor_action_launch_plans;
    std::vector<StudioSelectionBuilderLaunchPlanResult> builder_launch_plans;
    StudioToolboxPaletteLaunchPlanResult toolbox_palette_launch_plan;
};

struct StudioDesignerLaunchSurfacePlanResult {
    bool ok = false;
    std::string error;
    StudioDesignerLaunchSurfacePlan plan;
};

struct StudioDesignerLaunchSurfaceCatalogRequest {
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string symbol;
    std::size_t line = 0;
    std::size_t column = 0;
};

struct StudioDesignerLaunchSurfaceCatalogEntry {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::size_t editor_action_launch_plan_count = 0;
    std::size_t builder_launch_plan_count = 0;
    bool toolbox_available = false;
    std::size_t toolbox_item_count = 0;
    std::string toolbox_error;
    StudioDesignerLaunchSurfacePlanResult launch_surface_plan;
};

struct StudioDesignerLaunchSurfaceCatalogResult {
    bool ok = false;
    std::string error;
    std::size_t context_count = 0;
    std::vector<StudioDesignerLaunchSurfaceCatalogEntry> contexts;
};

[[nodiscard]] StudioDesignerLaunchSurfacePlanResult plan_studio_designer_launch_surfaces(
    const StudioDesignerLaunchSurfaceRequest& request);
[[nodiscard]] StudioDesignerLaunchSurfaceCatalogResult plan_studio_designer_launch_surface_catalog(
    const StudioDesignerLaunchSurfaceCatalogRequest& request);

}  // namespace copperfin::studio
