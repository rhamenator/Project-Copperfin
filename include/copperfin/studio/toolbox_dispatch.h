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

[[nodiscard]] StudioToolboxDispatchResult plan_studio_toolbox_dispatch(
    const StudioToolboxDispatchRequest& request);

}  // namespace copperfin::studio
