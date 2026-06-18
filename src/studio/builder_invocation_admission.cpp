#include "copperfin/studio/builder_invocation_admission.h"

namespace copperfin::studio {

StudioBuilderInvocationAdmissionResult plan_studio_builder_invocation_admission(
    const StudioBuilderInvocationAdmissionRequest& request) {
    if (request.launch_plan.builder.id.empty()) {
        return {
            .ok = false,
            .error = "A builder invocation admission request requires a validated builder id.",
            .plan = {}
        };
    }
    if (request.launch_plan.entry_point.empty()) {
        return {
            .ok = false,
            .error = "A builder invocation admission request requires a launch entry point.",
            .plan = {}
        };
    }

    return {
        .ok = true,
        .error = {},
        .plan = {
            .builder = request.launch_plan.builder,
            .context = request.launch_plan.context,
            .command_token = "studio.builder.invoke",
            .entry_point = request.launch_plan.entry_point,
            .asset_path = request.launch_plan.asset_path,
            .record_index = request.launch_plan.record_index,
            .object_name = request.launch_plan.object_name,
            .unique_id = request.launch_plan.unique_id,
            .ui_launch_admitted = request.admit_ui_launch,
            .dry_run = !request.admit_ui_launch,
            .mutates_asset = false
        }
    };
}

}  // namespace copperfin::studio
