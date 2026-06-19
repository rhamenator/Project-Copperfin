#pragma once

#include "copperfin/studio/builder_invocation_admission.h"
#include "copperfin/studio/builder_registry.h"
#include "copperfin/studio/context_editor_actions.h"
#include "copperfin/studio/toolbox_palette.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace copperfin::studio {

struct StudioDesignerContextRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
};

struct StudioDesignerContextResult {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::size_t editor_action_count = 0;
    std::size_t builder_count = 0;
    std::size_t toolbox_item_count = 0;
    std::vector<StudioEditorActionDescriptor> editor_actions;
    std::vector<StudioBuilderDescriptor> builders;
    std::vector<StudioToolboxItemDescriptor> toolbox_items;
};

struct StudioSelectionBuilderLaunchRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string builder_id;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
};

struct StudioSelectionBuilderLaunchPlanResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioBuilderLaunchPlan plan;
};

struct StudioSelectionBuilderInvocationAdmissionCatalogRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool admit_ui_launches = false;
};

struct StudioSelectionBuilderInvocationAdmissionCatalogEntry {
    StudioBuilderDescriptor builder;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioSelectionBuilderLaunchPlanResult launch_plan;
    StudioBuilderInvocationAdmissionResult invocation_admission;
};

struct StudioSelectionBuilderInvocationAdmissionCatalogResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::size_t builder_count = 0;
    std::size_t admission_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioSelectionBuilderInvocationAdmissionCatalogEntry> entries;
};

[[nodiscard]] StudioDesignerContextResult studio_designer_context_for_selection(
    const StudioDesignerContextRequest& request);
[[nodiscard]] StudioSelectionBuilderLaunchPlanResult plan_studio_builder_launch_for_selection(
    const StudioSelectionBuilderLaunchRequest& request);
[[nodiscard]] StudioSelectionBuilderInvocationAdmissionCatalogResult
plan_studio_builder_invocation_admission_catalog_for_selection(
    const StudioSelectionBuilderInvocationAdmissionCatalogRequest& request);

}  // namespace copperfin::studio
