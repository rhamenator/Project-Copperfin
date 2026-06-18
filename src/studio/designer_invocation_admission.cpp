#include "copperfin/studio/designer_invocation_admission.h"

#include <utility>

namespace copperfin::studio {

StudioDesignerInvocationAdmissionResult plan_studio_designer_invocation_admission(
    const StudioDesignerInvocationAdmissionRequest& request) {
    std::vector<StudioEditorActionInvocationAdmissionResult> editor_action_invocations;
    editor_action_invocations.reserve(request.launch_surface_plan.editor_action_launch_plans.size());
    for (const auto& launch_result : request.launch_surface_plan.editor_action_launch_plans) {
        if (!launch_result.ok) {
            editor_action_invocations.push_back({
                .ok = false,
                .error = launch_result.error,
                .plan = {}
            });
            continue;
        }
        editor_action_invocations.push_back(plan_studio_editor_action_invocation_admission({
            .launch_plan = launch_result.plan,
            .admit_editor_invocation = request.admit_editor_invocations
        }));
    }

    std::vector<StudioBuilderInvocationAdmissionResult> builder_invocations;
    builder_invocations.reserve(request.launch_surface_plan.builder_launch_plans.size());
    for (const auto& launch_result : request.launch_surface_plan.builder_launch_plans) {
        if (!launch_result.ok) {
            builder_invocations.push_back({
                .ok = false,
                .error = launch_result.error,
                .plan = {}
            });
            continue;
        }
        builder_invocations.push_back(plan_studio_builder_invocation_admission({
            .launch_plan = launch_result.plan,
            .admit_ui_launch = request.admit_builder_invocations
        }));
    }

    StudioToolboxInvocationAdmissionResult toolbox_invocation{};
    if (request.launch_surface_plan.toolbox_palette_launch_plan.ok) {
        toolbox_invocation = plan_studio_toolbox_invocation_admission({
            .launch_plan = request.launch_surface_plan.toolbox_palette_launch_plan.plan,
            .admit_palette_invocation = request.admit_toolbox_invocation
        });
    } else {
        toolbox_invocation = {
            .ok = false,
            .error = request.launch_surface_plan.toolbox_palette_launch_plan.error,
            .plan = {}
        };
    }

    std::size_t valid_surface_count = 0U;
    bool dry_run = true;
    bool mutates_asset = false;
    for (const auto& admission : editor_action_invocations) {
        if (admission.ok) {
            ++valid_surface_count;
            dry_run = dry_run && admission.plan.dry_run;
            mutates_asset = mutates_asset || admission.plan.mutates_asset;
        }
    }
    for (const auto& admission : builder_invocations) {
        if (admission.ok) {
            ++valid_surface_count;
            dry_run = dry_run && admission.plan.dry_run;
            mutates_asset = mutates_asset || admission.plan.mutates_asset;
        }
    }
    if (toolbox_invocation.ok) {
        ++valid_surface_count;
        dry_run = dry_run && toolbox_invocation.plan.dry_run;
        mutates_asset = mutates_asset || toolbox_invocation.plan.mutates_asset;
    }

    if (valid_surface_count == 0U) {
        return {
            .ok = false,
            .error = "A designer invocation admission request requires at least one validated launch surface.",
            .plan = {}
        };
    }

    return {
        .ok = true,
        .error = {},
        .plan = {
            .selection_context = request.launch_surface_plan.selection_context,
            .asset_path = request.launch_surface_plan.asset_path,
            .record_index = request.launch_surface_plan.record_index,
            .object_name = request.launch_surface_plan.object_name,
            .unique_id = request.launch_surface_plan.unique_id,
            .symbol = request.launch_surface_plan.symbol,
            .line = request.launch_surface_plan.line,
            .column = request.launch_surface_plan.column,
            .editor_action_invocation_count = editor_action_invocations.size(),
            .builder_invocation_count = builder_invocations.size(),
            .toolbox_available = toolbox_invocation.ok,
            .toolbox_item_count = toolbox_invocation.ok ? toolbox_invocation.plan.item_count : 0U,
            .toolbox_error = toolbox_invocation.ok ? std::string{} : toolbox_invocation.error,
            .editor_action_invocations = std::move(editor_action_invocations),
            .builder_invocations = std::move(builder_invocations),
            .toolbox_invocation = std::move(toolbox_invocation),
            .dry_run = dry_run,
            .mutates_asset = mutates_asset
        }
    };
}

}  // namespace copperfin::studio
