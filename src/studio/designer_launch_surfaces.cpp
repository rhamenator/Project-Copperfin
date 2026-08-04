// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/studio/designer_launch_surfaces.h"

#include <string>
#include <utility>

namespace copperfin::studio {

namespace {

std::vector<StudioEditorSelectionContext> all_launch_surface_selection_contexts() {
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

StudioDesignerLaunchSurfacePlanResult plan_studio_designer_launch_surfaces(
    const StudioDesignerLaunchSurfaceRequest& request) {
    const auto designer_context = studio_designer_context_for_selection({
        .selection_context = request.selection_context
    });

    std::vector<StudioEditorActionLaunchPlanResult> editor_action_launch_plans;
    editor_action_launch_plans.reserve(designer_context.editor_actions.size());
    for (const auto& action : designer_context.editor_actions) {
        editor_action_launch_plans.push_back(plan_studio_editor_action_launch({
            .selection_context = request.selection_context,
            .action_id = std::string(action.id),
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .symbol = request.symbol,
            .line = request.line,
            .column = request.column
        }));
    }

    std::vector<StudioSelectionBuilderLaunchPlanResult> builder_launch_plans;
    builder_launch_plans.reserve(designer_context.builders.size());
    for (const auto& builder : designer_context.builders) {
        builder_launch_plans.push_back(plan_studio_builder_launch_for_selection({
            .selection_context = request.selection_context,
            .builder_id = std::string(builder.id),
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id
        }));
    }

    auto toolbox_palette_launch_plan = plan_studio_toolbox_palette_launch({
        .selection_context = request.selection_context,
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id
    });
    const bool toolbox_available = toolbox_palette_launch_plan.ok;
    const std::size_t toolbox_item_count = toolbox_available ? toolbox_palette_launch_plan.plan.item_count : 0U;
    const std::string toolbox_error = toolbox_palette_launch_plan.error;

    return {
        .ok = true,
        .error = {},
        .plan = {
            .selection_context = request.selection_context,
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .symbol = request.symbol,
            .line = request.line,
            .column = request.column,
            .editor_action_launch_plan_count = editor_action_launch_plans.size(),
            .builder_launch_plan_count = builder_launch_plans.size(),
            .toolbox_available = toolbox_available,
            .toolbox_item_count = toolbox_item_count,
            .toolbox_error = toolbox_error,
            .editor_action_launch_plans = std::move(editor_action_launch_plans),
            .builder_launch_plans = std::move(builder_launch_plans),
            .toolbox_palette_launch_plan = std::move(toolbox_palette_launch_plan)
        }
    };
}

StudioDesignerLaunchSurfaceCatalogResult plan_studio_designer_launch_surface_catalog(
    const StudioDesignerLaunchSurfaceCatalogRequest& request) {
    std::vector<StudioDesignerLaunchSurfaceCatalogEntry> entries;
    const auto contexts = all_launch_surface_selection_contexts();
    entries.reserve(contexts.size());

    for (const auto context : contexts) {
        auto launch_surface_plan = plan_studio_designer_launch_surfaces({
            .selection_context = context,
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .symbol = request.symbol,
            .line = request.line,
            .column = request.column
        });
        const auto& plan = launch_surface_plan.plan;
        entries.push_back({
            .selection_context = context,
            .editor_action_launch_plan_count = plan.editor_action_launch_plan_count,
            .builder_launch_plan_count = plan.builder_launch_plan_count,
            .toolbox_available = plan.toolbox_available,
            .toolbox_item_count = plan.toolbox_item_count,
            .toolbox_error = plan.toolbox_error,
            .launch_surface_plan = std::move(launch_surface_plan)
        });
    }

    const auto context_count = entries.size();
    return {
        .ok = true,
        .error = {},
        .context_count = context_count,
        .contexts = std::move(entries)
    };
}

}  // namespace copperfin::studio
