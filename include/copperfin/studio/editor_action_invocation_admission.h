#pragma once

#include "copperfin/studio/context_editor_actions.h"

#include <cstddef>
#include <string>

namespace copperfin::studio {

struct StudioEditorActionInvocationAdmissionRequest {
    StudioEditorActionLaunchPlan launch_plan;
    bool admit_editor_invocation = false;
};

struct StudioEditorActionInvocationAdmissionPlan {
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
    bool editor_invocation_admitted = false;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioEditorActionInvocationAdmissionResult {
    bool ok = false;
    std::string error;
    StudioEditorActionInvocationAdmissionPlan plan;
};

[[nodiscard]] StudioEditorActionInvocationAdmissionResult plan_studio_editor_action_invocation_admission(
    const StudioEditorActionInvocationAdmissionRequest& request);

}  // namespace copperfin::studio
