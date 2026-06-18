#include "copperfin/studio/editor_action_dispatch.h"

#include <string>
#include <utility>

namespace copperfin::studio {

namespace {

void append_argument(std::vector<std::string>& arguments, std::string key, std::string value) {
    arguments.push_back(std::move(key));
    arguments.push_back(std::move(value));
}

}  // namespace

StudioEditorActionDispatchResult plan_studio_editor_action_dispatch(
    const StudioEditorActionDispatchRequest& request) {
    if (request.admission_plan.action.id.empty()) {
        return {
            .ok = false,
            .error = "An editor action dispatch request requires a validated action id.",
            .plan = {}
        };
    }
    if (request.admission_plan.command_token.empty()) {
        return {
            .ok = false,
            .error = "An editor action dispatch request requires a command token.",
            .plan = {}
        };
    }
    if (!request.admission_plan.editor_invocation_admitted || request.admission_plan.dry_run) {
        return {
            .ok = false,
            .error = "An editor action dispatch request requires an admitted non-dry-run invocation.",
            .plan = {}
        };
    }

    std::vector<std::string> arguments;
    append_argument(arguments, "--command-token", request.admission_plan.command_token);
    append_argument(arguments, "--action-id", std::string(request.admission_plan.action.id));
    append_argument(arguments, "--selection-context",
        studio_editor_selection_context_name(request.admission_plan.selection_context));
    append_argument(arguments, "--target-surface", request.admission_plan.target_surface);
    append_argument(arguments, "--path", request.admission_plan.asset_path);
    append_argument(arguments, "--record", std::to_string(request.admission_plan.record_index));
    append_argument(arguments, "--object-name", request.admission_plan.object_name);
    append_argument(arguments, "--unique-id", request.admission_plan.unique_id);
    append_argument(arguments, "--symbol", request.admission_plan.symbol);
    append_argument(arguments, "--line", std::to_string(request.admission_plan.line));
    append_argument(arguments, "--column", std::to_string(request.admission_plan.column));

    return {
        .ok = true,
        .error = {},
        .plan = {
            .action = request.admission_plan.action,
            .selection_context = request.admission_plan.selection_context,
            .command_token = request.admission_plan.command_token,
            .target_surface = request.admission_plan.target_surface,
            .asset_path = request.admission_plan.asset_path,
            .record_index = request.admission_plan.record_index,
            .object_name = request.admission_plan.object_name,
            .unique_id = request.admission_plan.unique_id,
            .symbol = request.admission_plan.symbol,
            .line = request.admission_plan.line,
            .column = request.admission_plan.column,
            .dispatch_arguments = std::move(arguments),
            .dispatch_admitted = true,
            .dry_run = false,
            .executed = false,
            .mutates_asset = false
        }
    };
}

}  // namespace copperfin::studio
