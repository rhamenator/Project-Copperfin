#include "copperfin/studio/editor_action_invocation_admission.h"

namespace copperfin::studio {

StudioEditorActionInvocationAdmissionResult plan_studio_editor_action_invocation_admission(
    const StudioEditorActionInvocationAdmissionRequest& request) {
    if (request.launch_plan.action.id.empty()) {
        return {
            .ok = false,
            .error = "An editor action invocation admission request requires a validated action id.",
            .plan = {}
        };
    }
    if (request.launch_plan.command_token.empty()) {
        return {
            .ok = false,
            .error = "An editor action invocation admission request requires a command token.",
            .plan = {}
        };
    }

    return {
        .ok = true,
        .error = {},
        .plan = {
            .action = request.launch_plan.action,
            .selection_context = request.launch_plan.selection_context,
            .command_token = request.launch_plan.command_token,
            .target_surface = request.launch_plan.target_surface,
            .asset_path = request.launch_plan.asset_path,
            .record_index = request.launch_plan.record_index,
            .object_name = request.launch_plan.object_name,
            .unique_id = request.launch_plan.unique_id,
            .symbol = request.launch_plan.symbol,
            .line = request.launch_plan.line,
            .column = request.launch_plan.column,
            .editor_invocation_admitted = request.admit_editor_invocation,
            .dry_run = !request.admit_editor_invocation,
            .mutates_asset = false
        }
    };
}

}  // namespace copperfin::studio
