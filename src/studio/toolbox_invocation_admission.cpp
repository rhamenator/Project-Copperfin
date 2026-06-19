#include "copperfin/studio/toolbox_invocation_admission.h"

#include <string>
#include <utility>

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
            .command_token = "studio.toolbox.palette.invoke",
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

StudioToolboxInvocationAdmissionCatalogResult plan_studio_toolbox_invocation_admission_catalog(
    const StudioToolboxInvocationAdmissionCatalogRequest& request) {
    auto items = studio_toolbox_items_for_context(request.toolbox_context);
    if (items.empty()) {
        return {
            .ok = false,
            .error = "A toolbox invocation admission catalog request requires validated toolbox item metadata.",
            .selection_context = request.selection_context,
            .toolbox_context = request.toolbox_context,
            .command_token = {},
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .item_count = 0U,
            .items = {},
            .invocation_admission = {},
            .admission_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false
        };
    }

    auto invocation_admission = plan_studio_toolbox_invocation_admission({
        .launch_plan = {
            .selection_context = request.selection_context,
            .toolbox_context = request.toolbox_context,
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .item_count = items.size(),
            .items = items
        },
        .admit_palette_invocation = request.admit_palette_invocation
    });

    const std::size_t admission_count = invocation_admission.ok ? 1U : 0U;
    const std::size_t error_count = invocation_admission.ok ? 0U : 1U;

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .toolbox_context = request.toolbox_context,
        .command_token = invocation_admission.ok ? invocation_admission.plan.command_token : std::string{},
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .item_count = items.size(),
        .items = std::move(items),
        .invocation_admission = std::move(invocation_admission),
        .admission_count = admission_count,
        .error_count = error_count,
        .dry_run = admission_count == 0U ? true : invocation_admission.plan.dry_run,
        .mutates_asset = admission_count == 0U ? false : invocation_admission.plan.mutates_asset
    };
}

StudioSelectionToolboxInvocationAdmissionCatalogResult
plan_studio_toolbox_invocation_admission_catalog_for_selection(
    const StudioSelectionToolboxInvocationAdmissionCatalogRequest& request) {
    auto launch_plan = plan_studio_toolbox_palette_launch({
        .selection_context = request.selection_context,
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id
    });
    if (!launch_plan.ok) {
        return {
            .ok = false,
            .error = "A selection-context toolbox invocation admission catalog request requires a toolbox palette.",
            .selection_context = request.selection_context,
            .toolbox_context = StudioToolboxContext::form,
            .command_token = {},
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .item_count = 0U,
            .items = {},
            .launch_plan = std::move(launch_plan),
            .invocation_admission = {},
            .admission_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false
        };
    }

    auto invocation_admission = plan_studio_toolbox_invocation_admission({
        .launch_plan = launch_plan.plan,
        .admit_palette_invocation = request.admit_palette_invocation
    });

    const std::size_t admission_count = invocation_admission.ok ? 1U : 0U;
    const std::size_t error_count = invocation_admission.ok ? 0U : 1U;
    const bool dry_run = admission_count == 0U ? true : invocation_admission.plan.dry_run;
    const bool mutates_asset = invocation_admission.ok ? invocation_admission.plan.mutates_asset : false;

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .toolbox_context = launch_plan.plan.toolbox_context,
        .command_token = invocation_admission.ok ? invocation_admission.plan.command_token : std::string{},
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .item_count = launch_plan.plan.item_count,
        .items = launch_plan.plan.items,
        .launch_plan = std::move(launch_plan),
        .invocation_admission = std::move(invocation_admission),
        .admission_count = admission_count,
        .error_count = error_count,
        .dry_run = dry_run,
        .mutates_asset = mutates_asset
    };
}

}  // namespace copperfin::studio
