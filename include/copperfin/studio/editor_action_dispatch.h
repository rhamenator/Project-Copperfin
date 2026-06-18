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

[[nodiscard]] StudioEditorActionDispatchResult plan_studio_editor_action_dispatch(
    const StudioEditorActionDispatchRequest& request);

}  // namespace copperfin::studio
