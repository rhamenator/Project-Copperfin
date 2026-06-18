#pragma once

#include "copperfin/studio/builder_registry.h"

#include <cstddef>
#include <string>

namespace copperfin::studio {

struct StudioBuilderInvocationAdmissionRequest {
    StudioBuilderLaunchPlan launch_plan;
    bool admit_ui_launch = false;
};

struct StudioBuilderInvocationAdmissionPlan {
    StudioBuilderDescriptor builder;
    StudioBuilderContext context = StudioBuilderContext::form;
    std::string command_token;
    std::string entry_point;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool ui_launch_admitted = false;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioBuilderInvocationAdmissionResult {
    bool ok = false;
    std::string error;
    StudioBuilderInvocationAdmissionPlan plan;
};

[[nodiscard]] StudioBuilderInvocationAdmissionResult plan_studio_builder_invocation_admission(
    const StudioBuilderInvocationAdmissionRequest& request);

}  // namespace copperfin::studio
