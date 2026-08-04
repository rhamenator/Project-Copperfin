// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/studio/builder_dispatch.h"
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

struct StudioSelectionBuilderLaunchCatalogRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
};

struct StudioSelectionBuilderLaunchCatalogEntry {
    StudioBuilderDescriptor builder;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioSelectionBuilderLaunchPlanResult launch_plan;
};

struct StudioSelectionBuilderLaunchCatalogResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::size_t builder_count = 0;
    std::size_t launch_plan_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioSelectionBuilderLaunchCatalogEntry> entries;
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

struct StudioSelectionBuilderDispatchCatalogRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool admit_ui_launches = false;
};

struct StudioSelectionBuilderDispatchCatalogEntry {
    StudioBuilderDescriptor builder;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioSelectionBuilderLaunchPlanResult launch_plan;
    StudioBuilderInvocationAdmissionResult invocation_admission;
    StudioBuilderDispatchResult dispatch;
};

struct StudioSelectionBuilderDispatchCatalogResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::size_t builder_count = 0;
    std::size_t dispatch_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioSelectionBuilderDispatchCatalogEntry> entries;
};

struct StudioSelectionBuilderDispatchExecutionCatalogRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool admit_ui_launches = false;
    bool admit_execution = false;
};

struct StudioSelectionBuilderDispatchExecutionCatalogEntry {
    StudioBuilderDescriptor builder;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioSelectionBuilderLaunchPlanResult launch_plan;
    StudioBuilderInvocationAdmissionResult invocation_admission;
    StudioBuilderDispatchResult dispatch;
    bool execution_admitted = false;
    bool execution_ready = false;
    std::string execution_error;
};

struct StudioSelectionBuilderDispatchExecutionCatalogResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::size_t builder_count = 0;
    std::size_t execution_ready_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioSelectionBuilderDispatchExecutionCatalogEntry> entries;
};

[[nodiscard]] StudioDesignerContextResult studio_designer_context_for_selection(
    const StudioDesignerContextRequest& request);
[[nodiscard]] StudioSelectionBuilderLaunchPlanResult plan_studio_builder_launch_for_selection(
    const StudioSelectionBuilderLaunchRequest& request);
[[nodiscard]] StudioSelectionBuilderLaunchCatalogResult
plan_studio_builder_launch_catalog_for_selection(
    const StudioSelectionBuilderLaunchCatalogRequest& request);
[[nodiscard]] StudioSelectionBuilderInvocationAdmissionCatalogResult
plan_studio_builder_invocation_admission_catalog_for_selection(
    const StudioSelectionBuilderInvocationAdmissionCatalogRequest& request);
[[nodiscard]] StudioSelectionBuilderDispatchCatalogResult
plan_studio_builder_dispatch_catalog_for_selection(
    const StudioSelectionBuilderDispatchCatalogRequest& request);
[[nodiscard]] StudioSelectionBuilderDispatchExecutionCatalogResult
plan_studio_builder_dispatch_execution_catalog_for_selection(
    const StudioSelectionBuilderDispatchExecutionCatalogRequest& request);

}  // namespace copperfin::studio
