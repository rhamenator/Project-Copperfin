#pragma once

#include "copperfin/studio/toolbox_invocation_admission.h"

#include <cstddef>
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

[[nodiscard]] StudioToolboxDispatchResult plan_studio_toolbox_dispatch(
    const StudioToolboxDispatchRequest& request);
[[nodiscard]] StudioToolboxDispatchCatalogResult plan_studio_toolbox_dispatch_catalog(
    const StudioToolboxDispatchCatalogRequest& request);
[[nodiscard]] StudioSelectionToolboxDispatchCatalogResult plan_studio_toolbox_dispatch_catalog_for_selection(
    const StudioSelectionToolboxDispatchCatalogRequest& request);

}  // namespace copperfin::studio
