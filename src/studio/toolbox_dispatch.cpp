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

StudioToolboxDispatchCatalogResult plan_studio_toolbox_dispatch_catalog(
    const StudioToolboxDispatchCatalogRequest& request) {
    auto invocation_catalog = plan_studio_toolbox_invocation_admission_catalog({
        .selection_context = StudioEditorSelectionContext::visual_object,
        .toolbox_context = request.toolbox_context,
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .admit_palette_invocation = request.admit_palette_invocation
    });
    if (!invocation_catalog.ok) {
        return {
            .ok = false,
            .error = invocation_catalog.error,
            .toolbox_context = request.toolbox_context,
            .command_token = {},
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .item_count = 0U,
            .items = {},
            .invocation_admission = {},
            .dispatch = {},
            .dispatch_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false
        };
    }

    StudioToolboxDispatchResult dispatch{};
    if (invocation_catalog.invocation_admission.ok) {
        dispatch = plan_studio_toolbox_dispatch({
            .admission_plan = invocation_catalog.invocation_admission.plan
        });
    } else {
        dispatch = {
            .ok = false,
            .error = invocation_catalog.invocation_admission.error,
            .plan = {}
        };
    }

    const std::size_t dispatch_count = dispatch.ok ? 1U : 0U;
    const std::size_t error_count = dispatch.ok ? 0U : 1U;
    const bool dry_run = dispatch_count == 0U ? true : dispatch.plan.dry_run;
    const bool mutates_asset = dispatch.ok ? dispatch.plan.mutates_asset : false;

    return {
        .ok = true,
        .error = {},
        .toolbox_context = request.toolbox_context,
        .command_token = invocation_catalog.command_token,
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .item_count = invocation_catalog.item_count,
        .items = std::move(invocation_catalog.items),
        .invocation_admission = std::move(invocation_catalog.invocation_admission),
        .dispatch = std::move(dispatch),
        .dispatch_count = dispatch_count,
        .error_count = error_count,
        .dry_run = dry_run,
        .mutates_asset = mutates_asset
    };
}

}  // namespace copperfin::studio
