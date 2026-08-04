// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/studio/toolbox_invocation_admission.h"

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace copperfin::studio {

struct StudioToolboxDispatchRequest {
    StudioToolboxInvocationAdmissionPlan admission_plan;
};

struct StudioToolboxDispatchPlan {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string command_token;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::size_t item_count = 0;
    std::vector<StudioToolboxItemDescriptor> items;
    std::vector<std::string> dispatch_arguments;
    bool dispatch_admitted = false;
    bool dry_run = true;
    bool executed = false;
    bool mutates_asset = false;
};

struct StudioToolboxDispatchResult {
    bool ok = false;
    std::string error;
    StudioToolboxDispatchPlan plan;
};

struct StudioToolboxDispatchCatalogRequest {
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool admit_palette_invocation = false;
};

struct StudioToolboxDispatchCatalogResult {
    bool ok = false;
    std::string error;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string command_token;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::size_t item_count = 0;
    std::vector<StudioToolboxItemDescriptor> items;
    StudioToolboxInvocationAdmissionResult invocation_admission;
    StudioToolboxDispatchResult dispatch;
    std::size_t dispatch_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioSelectionToolboxDispatchCatalogRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool admit_palette_invocation = false;
};

struct StudioSelectionToolboxDispatchCatalogResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string command_token;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::size_t item_count = 0;
    std::vector<StudioToolboxItemDescriptor> items;
    StudioToolboxPaletteLaunchPlanResult launch_plan;
    StudioToolboxInvocationAdmissionResult invocation_admission;
    StudioToolboxDispatchResult dispatch;
    std::size_t dispatch_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioToolboxDispatchExecutionObservation {
    bool launched = false;
    int exit_code = 0;
    std::string output;
    std::string error;
    bool mutates_asset = false;
};

using StudioToolboxDispatchExecutor =
    std::function<StudioToolboxDispatchExecutionObservation(const StudioToolboxDispatchPlan&)>;

struct StudioToolboxDispatchExecutionRequest {
    StudioToolboxDispatchPlan dispatch_plan;
    bool admit_execution = false;
    StudioToolboxDispatchExecutor executor;
};

struct StudioToolboxDispatchExecutionResult {
    bool ok = false;
    std::string error;
    StudioToolboxDispatchPlan dispatch_plan;
    StudioToolboxDispatchExecutionObservation observation;
    bool execution_admitted = false;
    bool executed = false;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioToolboxDispatchExecutionCatalogRequest {
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool admit_palette_invocation = false;
    bool admit_execution = false;
};

struct StudioToolboxDispatchExecutionCatalogEntry {
    StudioToolboxItemDescriptor item;
    bool execution_admitted = false;
    bool execution_ready = false;
    std::string execution_error;
};

struct StudioToolboxDispatchExecutionCatalogResult {
    bool ok = false;
    std::string error;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string command_token;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::size_t item_count = 0;
    std::vector<StudioToolboxItemDescriptor> items;
    StudioToolboxInvocationAdmissionResult invocation_admission;
    StudioToolboxDispatchResult dispatch;
    std::size_t execution_ready_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioToolboxDispatchExecutionCatalogEntry> entries;
};

struct StudioSelectionToolboxDispatchExecutionCatalogRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool admit_palette_invocation = false;
    bool admit_execution = false;
};

struct StudioSelectionToolboxDispatchExecutionCatalogResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string command_token;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::size_t item_count = 0;
    std::vector<StudioToolboxItemDescriptor> items;
    StudioToolboxPaletteLaunchPlanResult launch_plan;
    StudioToolboxInvocationAdmissionResult invocation_admission;
    StudioToolboxDispatchResult dispatch;
    std::size_t execution_ready_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioToolboxDispatchExecutionCatalogEntry> entries;
};

[[nodiscard]] StudioToolboxDispatchResult plan_studio_toolbox_dispatch(
    const StudioToolboxDispatchRequest& request);
[[nodiscard]] StudioToolboxDispatchCatalogResult plan_studio_toolbox_dispatch_catalog(
    const StudioToolboxDispatchCatalogRequest& request);
[[nodiscard]] StudioSelectionToolboxDispatchCatalogResult plan_studio_toolbox_dispatch_catalog_for_selection(
    const StudioSelectionToolboxDispatchCatalogRequest& request);
[[nodiscard]] StudioToolboxDispatchExecutionResult execute_studio_toolbox_dispatch(
    const StudioToolboxDispatchExecutionRequest& request);
[[nodiscard]] StudioToolboxDispatchExecutionCatalogResult plan_studio_toolbox_dispatch_execution_catalog(
    const StudioToolboxDispatchExecutionCatalogRequest& request);
[[nodiscard]] StudioSelectionToolboxDispatchExecutionCatalogResult
plan_studio_toolbox_dispatch_execution_catalog_for_selection(
    const StudioSelectionToolboxDispatchExecutionCatalogRequest& request);

}  // namespace copperfin::studio
