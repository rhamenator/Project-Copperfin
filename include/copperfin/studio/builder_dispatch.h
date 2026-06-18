#pragma once

#include "copperfin/studio/builder_invocation_admission.h"

#include <cstddef>
#include <string>
#include <vector>

namespace copperfin::studio {

struct StudioBuilderDispatchRequest {
    StudioBuilderInvocationAdmissionPlan admission_plan;
};

struct StudioBuilderDispatchPlan {
    StudioBuilderDescriptor builder;
    StudioBuilderContext context = StudioBuilderContext::form;
    std::string command_token;
    std::string entry_point;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::vector<std::string> dispatch_arguments;
    bool dispatch_admitted = false;
    bool dry_run = true;
    bool executed = false;
    bool mutates_asset = false;
};

struct StudioBuilderDispatchResult {
    bool ok = false;
    std::string error;
    StudioBuilderDispatchPlan plan;
};

[[nodiscard]] StudioBuilderDispatchResult plan_studio_builder_dispatch(
    const StudioBuilderDispatchRequest& request);

}  // namespace copperfin::studio
