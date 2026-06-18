#pragma once

#include "copperfin/studio/editor_action_invocation_admission.h"

#include <cstddef>
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

[[nodiscard]] StudioEditorActionDispatchResult plan_studio_editor_action_dispatch(
    const StudioEditorActionDispatchRequest& request);
[[nodiscard]] StudioEditorActionDispatchCatalogResult plan_studio_editor_action_dispatch_catalog(
    const StudioEditorActionDispatchCatalogRequest& request);

}  // namespace copperfin::studio
