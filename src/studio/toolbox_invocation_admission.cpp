#include "copperfin/studio/toolbox_invocation_admission.h"

namespace copperfin::studio {

StudioToolboxInvocationAdmissionResult plan_studio_toolbox_invocation_admission(
    const StudioToolboxInvocationAdmissionRequest& request) {
    if (request.launch_plan.items.empty() || request.launch_plan.item_count == 0U) {
        return {
            .ok = false,
            .error = "A toolbox invocation admission request requires validated toolbox item metadata.",
            .plan = {}
        };
    }
    if (request.launch_plan.item_count != request.launch_plan.items.size()) {
        return {
            .ok = false,
            .error = "A toolbox invocation admission request requires consistent toolbox item metadata.",
            .plan = {}
        };
    }

    return {
        .ok = true,
        .error = {},
        .plan = {
            .selection_context = request.launch_plan.selection_context,
            .toolbox_context = request.launch_plan.toolbox_context,
            .asset_path = request.launch_plan.asset_path,
            .record_index = request.launch_plan.record_index,
            .object_name = request.launch_plan.object_name,
            .unique_id = request.launch_plan.unique_id,
            .item_count = request.launch_plan.item_count,
            .items = request.launch_plan.items,
            .palette_invocation_admitted = request.admit_palette_invocation,
            .dry_run = !request.admit_palette_invocation,
            .mutates_asset = false
        }
    };
}

}  // namespace copperfin::studio
