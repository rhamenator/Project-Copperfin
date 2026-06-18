#include "copperfin/studio/toolbox_dispatch.h"

#include <string>
#include <utility>

namespace copperfin::studio {

namespace {

void append_argument(std::vector<std::string>& arguments, std::string key, std::string value) {
    arguments.push_back(std::move(key));
    arguments.push_back(std::move(value));
}

}  // namespace

StudioToolboxDispatchResult plan_studio_toolbox_dispatch(
    const StudioToolboxDispatchRequest& request) {
    if (request.admission_plan.command_token.empty()) {
        return {
            .ok = false,
            .error = "A toolbox dispatch request requires a command token.",
            .plan = {}
        };
    }
    if (!request.admission_plan.palette_invocation_admitted || request.admission_plan.dry_run) {
        return {
            .ok = false,
            .error = "A toolbox dispatch request requires an admitted non-dry-run invocation.",
            .plan = {}
        };
    }
    if (request.admission_plan.items.empty() || request.admission_plan.item_count == 0U) {
        return {
            .ok = false,
            .error = "A toolbox dispatch request requires validated toolbox item metadata.",
            .plan = {}
        };
    }
    if (request.admission_plan.item_count != request.admission_plan.items.size()) {
        return {
            .ok = false,
            .error = "A toolbox dispatch request requires consistent toolbox item metadata.",
            .plan = {}
        };
    }

    std::vector<std::string> arguments;
    append_argument(arguments, "--command-token", request.admission_plan.command_token);
    append_argument(arguments, "--selection-context",
        studio_editor_selection_context_name(request.admission_plan.selection_context));
    append_argument(arguments, "--toolbox-context", studio_toolbox_context_name(request.admission_plan.toolbox_context));
    append_argument(arguments, "--path", request.admission_plan.asset_path);
    append_argument(arguments, "--record", std::to_string(request.admission_plan.record_index));
    append_argument(arguments, "--object-name", request.admission_plan.object_name);
    append_argument(arguments, "--unique-id", request.admission_plan.unique_id);
    append_argument(arguments, "--item-count", std::to_string(request.admission_plan.item_count));

    return {
        .ok = true,
        .error = {},
        .plan = {
            .selection_context = request.admission_plan.selection_context,
            .toolbox_context = request.admission_plan.toolbox_context,
            .command_token = request.admission_plan.command_token,
            .asset_path = request.admission_plan.asset_path,
            .record_index = request.admission_plan.record_index,
            .object_name = request.admission_plan.object_name,
            .unique_id = request.admission_plan.unique_id,
            .item_count = request.admission_plan.item_count,
            .items = request.admission_plan.items,
            .dispatch_arguments = std::move(arguments),
            .dispatch_admitted = true,
            .dry_run = false,
            .executed = false,
            .mutates_asset = false
        }
    };
}

}  // namespace copperfin::studio
