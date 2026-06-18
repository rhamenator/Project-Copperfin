#include "copperfin/studio/builder_dispatch.h"

#include <string>
#include <utility>

namespace copperfin::studio {

namespace {

void append_argument(std::vector<std::string>& arguments, std::string key, std::string value) {
    arguments.push_back(std::move(key));
    arguments.push_back(std::move(value));
}

}  // namespace

StudioBuilderDispatchResult plan_studio_builder_dispatch(
    const StudioBuilderDispatchRequest& request) {
    if (request.admission_plan.builder.id.empty()) {
        return {
            .ok = false,
            .error = "A builder dispatch request requires a validated builder id.",
            .plan = {}
        };
    }
    if (request.admission_plan.command_token.empty()) {
        return {
            .ok = false,
            .error = "A builder dispatch request requires a command token.",
            .plan = {}
        };
    }
    if (request.admission_plan.entry_point.empty()) {
        return {
            .ok = false,
            .error = "A builder dispatch request requires a launch entry point.",
            .plan = {}
        };
    }
    if (!request.admission_plan.ui_launch_admitted || request.admission_plan.dry_run) {
        return {
            .ok = false,
            .error = "A builder dispatch request requires an admitted non-dry-run invocation.",
            .plan = {}
        };
    }

    std::vector<std::string> arguments;
    append_argument(arguments, "--command-token", request.admission_plan.command_token);
    append_argument(arguments, "--builder-id", std::string(request.admission_plan.builder.id));
    append_argument(arguments, "--builder-context", studio_builder_context_name(request.admission_plan.context));
    append_argument(arguments, "--entry-point", request.admission_plan.entry_point);
    append_argument(arguments, "--path", request.admission_plan.asset_path);
    append_argument(arguments, "--record", std::to_string(request.admission_plan.record_index));
    append_argument(arguments, "--object-name", request.admission_plan.object_name);
    append_argument(arguments, "--unique-id", request.admission_plan.unique_id);

    return {
        .ok = true,
        .error = {},
        .plan = {
            .builder = request.admission_plan.builder,
            .context = request.admission_plan.context,
            .command_token = request.admission_plan.command_token,
            .entry_point = request.admission_plan.entry_point,
            .asset_path = request.admission_plan.asset_path,
            .record_index = request.admission_plan.record_index,
            .object_name = request.admission_plan.object_name,
            .unique_id = request.admission_plan.unique_id,
            .dispatch_arguments = std::move(arguments),
            .dispatch_admitted = true,
            .dry_run = false,
            .executed = false,
            .mutates_asset = false
        }
    };
}

}  // namespace copperfin::studio
