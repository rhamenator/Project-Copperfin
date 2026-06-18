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

StudioEditorActionDispatchCatalogResult plan_studio_editor_action_dispatch_catalog(
    const StudioEditorActionDispatchCatalogRequest& request) {
    const auto actions = studio_editor_actions_for_context(request.selection_context);
    if (actions.empty()) {
        return {
            .ok = false,
            .error = "An editor action dispatch catalog request requires at least one context action.",
            .selection_context = request.selection_context,
            .action_count = 0U,
            .dispatch_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    std::vector<StudioEditorActionDispatchCatalogEntry> entries;
    entries.reserve(actions.size());
    std::size_t dispatch_count = 0U;
    std::size_t error_count = 0U;
    bool dry_run = true;
    bool mutates_asset = false;

    for (const auto& action : actions) {
        auto launch_plan = plan_studio_editor_action_launch({
            .selection_context = request.selection_context,
            .action_id = std::string(action.id),
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .symbol = request.symbol,
            .line = request.line,
            .column = request.column
        });

        StudioEditorActionInvocationAdmissionResult invocation_admission{};
        StudioEditorActionDispatchResult dispatch{};
        if (launch_plan.ok) {
            invocation_admission = plan_studio_editor_action_invocation_admission({
                .launch_plan = launch_plan.plan,
                .admit_editor_invocation = request.admit_editor_invocations
            });
        } else {
            invocation_admission = {
                .ok = false,
                .error = launch_plan.error,
                .plan = {}
            };
        }

        if (invocation_admission.ok) {
            dispatch = plan_studio_editor_action_dispatch({
                .admission_plan = invocation_admission.plan
            });
        } else {
            dispatch = {
                .ok = false,
                .error = invocation_admission.error,
                .plan = {}
            };
        }

        if (dispatch.ok) {
            ++dispatch_count;
            dry_run = dry_run && dispatch.plan.dry_run;
            mutates_asset = mutates_asset || dispatch.plan.mutates_asset;
        } else {
            ++error_count;
        }

        entries.push_back({
            .action = action,
            .launch_plan = std::move(launch_plan),
            .invocation_admission = std::move(invocation_admission),
            .dispatch = std::move(dispatch)
        });
    }

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .action_count = entries.size(),
        .dispatch_count = dispatch_count,
        .error_count = error_count,
        .dry_run = dispatch_count == 0U ? true : dry_run,
        .mutates_asset = mutates_asset,
        .entries = std::move(entries)
    };
}

}  // namespace copperfin::studio
