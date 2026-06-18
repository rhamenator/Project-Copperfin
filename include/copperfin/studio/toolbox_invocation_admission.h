#pragma once

#include "copperfin/studio/toolbox_palette.h"

#include <cstddef>
#include <string>
#include <vector>

namespace copperfin::studio {

struct StudioToolboxInvocationAdmissionRequest {
    StudioToolboxPaletteLaunchPlan launch_plan;
    bool admit_palette_invocation = false;
};

struct StudioToolboxInvocationAdmissionPlan {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::size_t item_count = 0;
    std::vector<StudioToolboxItemDescriptor> items;
    bool palette_invocation_admitted = false;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioToolboxInvocationAdmissionResult {
    bool ok = false;
    std::string error;
    StudioToolboxInvocationAdmissionPlan plan;
};

[[nodiscard]] StudioToolboxInvocationAdmissionResult plan_studio_toolbox_invocation_admission(
    const StudioToolboxInvocationAdmissionRequest& request);

}  // namespace copperfin::studio
