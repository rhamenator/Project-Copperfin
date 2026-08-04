// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/studio/toolbox_dispatch.h"
#include "copperfin/studio/toolbox_palette.h"
#include "copperfin/vfp/visual_asset_editor.h"

#include <cstddef>
#include <string>
#include <vector>

namespace copperfin::studio {

struct StudioToolboxObjectCreateRequest {
    std::string path;
    std::string toolbox_item_id;
    std::string object_name;
    std::string unique_id;
    std::string parent_name;
    bool toolbox_context_provided = false;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
};

struct StudioToolboxObjectCreateFromPaletteDispatchRequest {
    StudioToolboxDispatchPlan dispatch_plan;
    std::string toolbox_item_id;
    std::string object_name;
    std::string unique_id;
    std::string parent_name;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
};

struct StudioToolboxObjectCreatePlan {
    std::string path;
    StudioToolboxItemDescriptor toolbox_item;
    bool toolbox_context_provided = false;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::size_t target_record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string parent_name;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioToolboxObjectCreatePlanResult {
    bool ok = false;
    std::string error;
    StudioToolboxObjectCreatePlan plan;
};

struct StudioSelectionToolboxObjectCreatePlanRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string path;
    std::string toolbox_item_id;
    std::string object_name;
    std::string unique_id;
    std::string parent_name;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
};

struct StudioSelectionToolboxObjectCreatePlanResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    StudioToolboxPaletteLaunchPlanResult launch_plan;
    StudioToolboxObjectCreatePlanResult create_plan;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioSelectionToolboxObjectCreateResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    StudioToolboxPaletteLaunchPlanResult launch_plan;
    StudioSelectionToolboxObjectCreatePlanResult create_plan;
    vfp::VisualObjectCreateResult create_result;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioToolboxObjectCreateFromDispatchResult {
    bool ok = false;
    std::string error;
    StudioToolboxObjectCreatePlanResult create_plan;
    vfp::VisualObjectCreateResult create_result;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioToolboxObjectCreateBatchItem {
    std::string toolbox_item_id;
    std::string object_name;
    std::string unique_id;
    std::string parent_name;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
};

struct StudioToolboxObjectCreateBatchFromPaletteDispatchRequest {
    StudioToolboxDispatchPlan dispatch_plan;
    std::vector<StudioToolboxObjectCreateBatchItem> items;
};

struct StudioToolboxObjectCreateBatchPlan {
    std::string path;
    bool toolbox_context_provided = false;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::size_t item_count = 0;
    std::vector<StudioToolboxObjectCreatePlan> plans;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioToolboxObjectCreateBatchPlanRequest {
    std::string path;
    bool toolbox_context_provided = false;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::vector<StudioToolboxObjectCreateBatchItem> items;
};

struct StudioToolboxObjectCreateBatchPlanResult {
    bool ok = false;
    std::string error;
    StudioToolboxObjectCreateBatchPlan plan;
};

struct StudioSelectionToolboxObjectCreateBatchPlanRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string path;
    std::vector<StudioToolboxObjectCreateBatchItem> items;
};

struct StudioSelectionToolboxObjectCreateBatchPlanResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    StudioToolboxPaletteLaunchPlanResult launch_plan;
    std::size_t item_count = 0;
    std::size_t plan_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    StudioToolboxObjectCreateBatchPlanResult batch_plan;
};

struct StudioSelectionToolboxObjectCreateBatchResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    StudioToolboxPaletteLaunchPlanResult launch_plan;
    std::size_t item_count = 0;
    StudioSelectionToolboxObjectCreateBatchPlanResult batch_plan;
    vfp::VisualObjectCreateBatchResult create_result;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioToolboxObjectCreateBatchFromDispatchResult {
    bool ok = false;
    std::string error;
    StudioToolboxObjectCreateBatchPlanResult batch_plan;
    vfp::VisualObjectCreateBatchResult create_result;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioToolboxObjectCreateDispatchRequest {
    StudioToolboxObjectCreatePlan create_plan;
    bool admit_create_operation = false;
};

struct StudioToolboxObjectCreateDispatchFromPaletteDispatchRequest {
    StudioToolboxObjectCreateFromPaletteDispatchRequest create_request;
    bool admit_create_operation = false;
};

struct StudioToolboxObjectCreateDispatchPlan {
    std::string path;
    StudioToolboxItemDescriptor toolbox_item;
    bool toolbox_context_provided = false;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::size_t target_record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string parent_name;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
    std::vector<std::string> dispatch_arguments;
    bool dispatch_admitted = false;
    bool dry_run = true;
    bool executed = false;
    bool mutates_asset = false;
};

struct StudioToolboxObjectCreateDispatchResult {
    bool ok = false;
    std::string error;
    StudioToolboxObjectCreateDispatchPlan plan;
};

struct StudioSelectionToolboxObjectCreateDispatchRequest {
    StudioSelectionToolboxObjectCreatePlanRequest create_request;
    bool admit_create_operation = false;
};

struct StudioSelectionToolboxObjectCreateDispatchResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    StudioToolboxPaletteLaunchPlanResult launch_plan;
    StudioSelectionToolboxObjectCreatePlanResult create_plan;
    StudioToolboxObjectCreateDispatchResult dispatch;
    std::size_t dispatch_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioToolboxObjectCreateBatchDispatchRequest {
    StudioToolboxObjectCreateBatchPlan batch_plan;
    bool admit_create_operation = false;
};

struct StudioSelectionToolboxObjectCreateBatchDispatchRequest {
    StudioSelectionToolboxObjectCreateBatchPlanRequest batch_request;
    bool admit_create_operation = false;
};

struct StudioToolboxObjectCreateBatchDispatchFromPaletteDispatchRequest {
    StudioToolboxObjectCreateBatchFromPaletteDispatchRequest create_request;
    bool admit_create_operation = false;
};

struct StudioToolboxObjectCreateBatchDispatchPlan {
    std::string path;
    bool toolbox_context_provided = false;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::size_t item_count = 0;
    std::vector<StudioToolboxObjectCreatePlan> plans;
    std::vector<std::string> dispatch_arguments;
    bool dispatch_admitted = false;
    bool dry_run = true;
    bool executed = false;
    bool mutates_asset = false;
};

struct StudioToolboxObjectCreateBatchDispatchResult {
    bool ok = false;
    std::string error;
    StudioToolboxObjectCreateBatchDispatchPlan plan;
};

struct StudioSelectionToolboxObjectCreateBatchDispatchResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    StudioToolboxPaletteLaunchPlanResult launch_plan;
    std::size_t item_count = 0;
    StudioSelectionToolboxObjectCreateBatchPlanResult batch_plan;
    StudioToolboxObjectCreateBatchDispatchResult dispatch;
    std::size_t dispatch_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioToolboxObjectCreatePlanCatalogRequest {
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string path;
    std::string parent_name;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
};

struct StudioToolboxObjectCreatePlanCatalogEntry {
    StudioToolboxItemDescriptor toolbox_item;
    StudioToolboxObjectCreatePlanResult create_plan;
};

struct StudioToolboxObjectCreatePlanCatalogResult {
    bool ok = false;
    std::string error;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::size_t item_count = 0;
    std::size_t plan_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioToolboxObjectCreatePlanCatalogEntry> entries;
};

struct StudioSelectionToolboxObjectCreatePlanCatalogRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string path;
    std::string parent_name;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
};

struct StudioSelectionToolboxObjectCreatePlanCatalogResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    StudioToolboxPaletteLaunchPlanResult launch_plan;
    std::size_t item_count = 0;
    std::size_t plan_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioToolboxObjectCreatePlanCatalogEntry> entries;
};

struct StudioToolboxObjectCreateDispatchCatalogRequest {
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string path;
    std::string parent_name;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
    bool admit_create_operation = false;
};

struct StudioToolboxObjectCreateDispatchCatalogEntry {
    StudioToolboxItemDescriptor toolbox_item;
    StudioToolboxObjectCreatePlanResult create_plan;
    StudioToolboxObjectCreateDispatchResult dispatch;
};

struct StudioToolboxObjectCreateDispatchCatalogResult {
    bool ok = false;
    std::string error;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::size_t item_count = 0;
    std::size_t dispatch_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioToolboxObjectCreateDispatchCatalogEntry> entries;
};

struct StudioSelectionToolboxObjectCreateDispatchCatalogRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string path;
    std::string parent_name;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
    bool admit_create_operation = false;
};

struct StudioSelectionToolboxObjectCreateDispatchCatalogResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    StudioToolboxPaletteLaunchPlanResult launch_plan;
    std::size_t item_count = 0;
    std::size_t dispatch_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioToolboxObjectCreateDispatchCatalogEntry> entries;
};

struct StudioToolboxObjectCreateBatchPlanCatalogRequest {
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string path;
    std::string parent_name;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
};

struct StudioToolboxObjectCreateBatchPlanCatalogResult {
    bool ok = false;
    std::string error;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::size_t item_count = 0;
    std::size_t plan_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    StudioToolboxObjectCreateBatchPlanResult batch_plan;
};

struct StudioSelectionToolboxObjectCreateBatchPlanCatalogRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string path;
    std::string parent_name;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
};

struct StudioSelectionToolboxObjectCreateBatchPlanCatalogResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    StudioToolboxPaletteLaunchPlanResult launch_plan;
    std::size_t item_count = 0;
    std::size_t plan_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    StudioToolboxObjectCreateBatchPlanResult batch_plan;
};

struct StudioToolboxObjectCreateBatchDispatchCatalogRequest {
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string path;
    std::string parent_name;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
    bool admit_create_operation = false;
};

struct StudioToolboxObjectCreateBatchDispatchCatalogResult {
    bool ok = false;
    std::string error;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::size_t item_count = 0;
    StudioToolboxObjectCreateBatchPlanResult batch_plan;
    StudioToolboxObjectCreateBatchDispatchResult dispatch;
    std::size_t dispatch_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioSelectionToolboxObjectCreateBatchDispatchCatalogRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string path;
    std::string parent_name;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
    bool admit_create_operation = false;
};

struct StudioSelectionToolboxObjectCreateBatchDispatchCatalogResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    StudioToolboxPaletteLaunchPlanResult launch_plan;
    std::size_t item_count = 0;
    StudioToolboxObjectCreateBatchPlanResult batch_plan;
    StudioToolboxObjectCreateBatchDispatchResult dispatch;
    std::size_t dispatch_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
};

[[nodiscard]] StudioToolboxObjectCreatePlanResult plan_visual_object_from_toolbox_item(
    const StudioToolboxObjectCreateRequest& request);
[[nodiscard]] StudioSelectionToolboxObjectCreatePlanResult plan_visual_object_from_toolbox_selection(
    const StudioSelectionToolboxObjectCreatePlanRequest& request);
[[nodiscard]] StudioToolboxObjectCreatePlanResult plan_visual_object_from_toolbox_dispatch(
    const StudioToolboxObjectCreateFromPaletteDispatchRequest& request);
[[nodiscard]] StudioToolboxObjectCreateBatchPlanResult plan_visual_objects_from_toolbox_items(
    const StudioToolboxObjectCreateBatchPlanRequest& request);
[[nodiscard]] StudioSelectionToolboxObjectCreateBatchPlanResult plan_visual_objects_from_toolbox_selection(
    const StudioSelectionToolboxObjectCreateBatchPlanRequest& request);
[[nodiscard]] StudioToolboxObjectCreateBatchPlanResult plan_visual_objects_from_toolbox_dispatch(
    const StudioToolboxObjectCreateBatchFromPaletteDispatchRequest& request);
[[nodiscard]] StudioToolboxObjectCreateDispatchResult plan_visual_object_create_dispatch(
    const StudioToolboxObjectCreateDispatchRequest& request);
[[nodiscard]] StudioSelectionToolboxObjectCreateDispatchResult plan_visual_object_create_dispatch_from_toolbox_selection(
    const StudioSelectionToolboxObjectCreateDispatchRequest& request);
[[nodiscard]] StudioToolboxObjectCreateDispatchResult plan_visual_object_create_dispatch_from_toolbox_dispatch(
    const StudioToolboxObjectCreateDispatchFromPaletteDispatchRequest& request);
[[nodiscard]] StudioToolboxObjectCreateBatchDispatchResult plan_visual_object_batch_create_dispatch(
    const StudioToolboxObjectCreateBatchDispatchRequest& request);
[[nodiscard]] StudioSelectionToolboxObjectCreateBatchDispatchResult
plan_visual_object_batch_create_dispatch_from_toolbox_selection(
    const StudioSelectionToolboxObjectCreateBatchDispatchRequest& request);
[[nodiscard]] StudioToolboxObjectCreateBatchDispatchResult plan_visual_object_batch_create_dispatch_from_toolbox_dispatch(
    const StudioToolboxObjectCreateBatchDispatchFromPaletteDispatchRequest& request);
[[nodiscard]] StudioToolboxObjectCreatePlanCatalogResult plan_visual_object_catalog_from_toolbox_context(
    const StudioToolboxObjectCreatePlanCatalogRequest& request);
[[nodiscard]] StudioSelectionToolboxObjectCreatePlanCatalogResult
plan_visual_object_catalog_from_toolbox_selection(
    const StudioSelectionToolboxObjectCreatePlanCatalogRequest& request);
[[nodiscard]] StudioToolboxObjectCreateDispatchCatalogResult plan_visual_object_create_dispatch_catalog(
    const StudioToolboxObjectCreateDispatchCatalogRequest& request);
[[nodiscard]] StudioSelectionToolboxObjectCreateDispatchCatalogResult
plan_visual_object_create_dispatch_catalog_from_toolbox_selection(
    const StudioSelectionToolboxObjectCreateDispatchCatalogRequest& request);
[[nodiscard]] StudioToolboxObjectCreateBatchPlanCatalogResult plan_visual_object_batch_catalog_from_toolbox_context(
    const StudioToolboxObjectCreateBatchPlanCatalogRequest& request);
[[nodiscard]] StudioSelectionToolboxObjectCreateBatchPlanCatalogResult
plan_visual_object_batch_catalog_from_toolbox_selection(
    const StudioSelectionToolboxObjectCreateBatchPlanCatalogRequest& request);
[[nodiscard]] StudioToolboxObjectCreateBatchDispatchCatalogResult plan_visual_object_batch_create_dispatch_catalog(
    const StudioToolboxObjectCreateBatchDispatchCatalogRequest& request);
[[nodiscard]] StudioSelectionToolboxObjectCreateBatchDispatchCatalogResult
plan_visual_object_batch_create_dispatch_catalog_from_toolbox_selection(
    const StudioSelectionToolboxObjectCreateBatchDispatchCatalogRequest& request);
[[nodiscard]] vfp::VisualObjectCreateResult create_visual_object_from_toolbox_item(
    const StudioToolboxObjectCreateRequest& request);
[[nodiscard]] StudioSelectionToolboxObjectCreateResult create_visual_object_from_toolbox_selection(
    const StudioSelectionToolboxObjectCreatePlanRequest& request);
[[nodiscard]] StudioToolboxObjectCreateFromDispatchResult create_visual_object_from_toolbox_dispatch(
    const StudioToolboxObjectCreateFromPaletteDispatchRequest& request);
[[nodiscard]] StudioSelectionToolboxObjectCreateBatchResult create_visual_objects_from_toolbox_selection(
    const StudioSelectionToolboxObjectCreateBatchPlanRequest& request);
[[nodiscard]] StudioToolboxObjectCreateBatchFromDispatchResult create_visual_objects_from_toolbox_dispatch(
    const StudioToolboxObjectCreateBatchFromPaletteDispatchRequest& request);
[[nodiscard]] vfp::VisualObjectCreateBatchResult create_visual_objects_from_toolbox_items(
    const StudioToolboxObjectCreateBatchPlanRequest& request);

}  // namespace copperfin::studio
