// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/studio/editor_action_invocation_admission.h"

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace copperfin::studio {

struct StudioEditorActionDispatchRequest {
    StudioEditorActionInvocationAdmissionPlan admission_plan;
};

struct StudioEditorActionDispatchPlan {
    StudioEditorActionDescriptor action;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string command_token;
    std::string target_surface;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string symbol;
    std::size_t line = 0;
    std::size_t column = 0;
    std::vector<std::string> dispatch_arguments;
    bool dispatch_admitted = false;
    bool dry_run = true;
    bool executed = false;
    bool mutates_asset = false;
};

struct StudioEditorActionDispatchResult {
    bool ok = false;
    std::string error;
    StudioEditorActionDispatchPlan plan;
};

struct StudioEditorActionDispatchCatalogRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string symbol;
    std::size_t line = 0;
    std::size_t column = 0;
    bool admit_editor_invocations = false;
};

struct StudioEditorActionDispatchCatalogEntry {
    StudioEditorActionDescriptor action;
    StudioEditorActionLaunchPlanResult launch_plan;
    StudioEditorActionInvocationAdmissionResult invocation_admission;
    StudioEditorActionDispatchResult dispatch;
};

struct StudioEditorActionDispatchCatalogResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::size_t action_count = 0;
    std::size_t dispatch_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioEditorActionDispatchCatalogEntry> entries;
};

struct StudioEditorActionDispatchExecutionObservation {
    bool launched = false;
    int exit_code = 0;
    std::string output;
    std::string error;
    bool mutates_asset = false;
};

using StudioEditorActionDispatchExecutor =
    std::function<StudioEditorActionDispatchExecutionObservation(const StudioEditorActionDispatchPlan&)>;

struct StudioEditorActionDispatchExecutionRequest {
    StudioEditorActionDispatchPlan dispatch_plan;
    bool admit_execution = false;
    StudioEditorActionDispatchExecutor executor;
};

struct StudioEditorActionDispatchExecutionResult {
    bool ok = false;
    std::string error;
    StudioEditorActionDispatchPlan dispatch_plan;
    StudioEditorActionDispatchExecutionObservation observation;
    bool execution_admitted = false;
    bool executed = false;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioEditorActionDispatchExecutionCatalogRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string symbol;
    std::size_t line = 0;
    std::size_t column = 0;
    bool admit_editor_invocations = false;
    bool admit_execution = false;
};

struct StudioEditorActionDispatchExecutionCatalogEntry {
    StudioEditorActionDescriptor action;
    StudioEditorActionLaunchPlanResult launch_plan;
    StudioEditorActionInvocationAdmissionResult invocation_admission;
    StudioEditorActionDispatchResult dispatch;
    bool execution_admitted = false;
    bool execution_ready = false;
    std::string execution_error;
};

struct StudioEditorActionDispatchExecutionCatalogResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::size_t action_count = 0;
    std::size_t execution_ready_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioEditorActionDispatchExecutionCatalogEntry> entries;
};

[[nodiscard]] StudioEditorActionDispatchResult plan_studio_editor_action_dispatch(
    const StudioEditorActionDispatchRequest& request);
[[nodiscard]] StudioEditorActionDispatchCatalogResult plan_studio_editor_action_dispatch_catalog(
    const StudioEditorActionDispatchCatalogRequest& request);
[[nodiscard]] StudioEditorActionDispatchExecutionResult execute_studio_editor_action_dispatch(
    const StudioEditorActionDispatchExecutionRequest& request);
[[nodiscard]] StudioEditorActionDispatchExecutionCatalogResult
plan_studio_editor_action_dispatch_execution_catalog(
    const StudioEditorActionDispatchExecutionCatalogRequest& request);

}  // namespace copperfin::studio
