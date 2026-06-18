#include "copperfin/studio/designer_dispatch.h"

#include <utility>

namespace copperfin::studio {

StudioDesignerDispatchResult plan_studio_designer_dispatch(
    const StudioDesignerDispatchRequest& request) {
    const auto& admission_plan = request.invocation_admission_plan;

    std::vector<StudioEditorActionDispatchResult> editor_dispatches;
    editor_dispatches.reserve(admission_plan.editor_action_invocations.size());
    for (const auto& admission : admission_plan.editor_action_invocations) {
        if (!admission.ok) {
            editor_dispatches.push_back({
                .ok = false,
                .error = admission.error,
                .plan = {}
            });
            continue;
        }
        editor_dispatches.push_back(plan_studio_editor_action_dispatch({
            .admission_plan = admission.plan
        }));
    }

    std::vector<StudioBuilderDispatchResult> builder_dispatches;
    builder_dispatches.reserve(admission_plan.builder_invocations.size());
    for (const auto& admission : admission_plan.builder_invocations) {
        if (!admission.ok) {
            builder_dispatches.push_back({
                .ok = false,
                .error = admission.error,
                .plan = {}
            });
            continue;
        }
        builder_dispatches.push_back(plan_studio_builder_dispatch({
            .admission_plan = admission.plan
        }));
    }

    StudioToolboxDispatchResult toolbox_dispatch{};
    if (admission_plan.toolbox_invocation.ok) {
        toolbox_dispatch = plan_studio_toolbox_dispatch({
            .admission_plan = admission_plan.toolbox_invocation.plan
        });
    } else {
        toolbox_dispatch = {
            .ok = false,
            .error = admission_plan.toolbox_invocation.error,
            .plan = {}
        };
    }

    const std::size_t input_surface_count = admission_plan.editor_action_invocations.size() +
                                            admission_plan.builder_invocations.size() +
                                            (admission_plan.toolbox_invocation.ok ? 1U : 0U);
    if (input_surface_count == 0U) {
        return {
            .ok = false,
            .error = "A designer dispatch request requires at least one invocation admission.",
            .plan = {}
        };
    }

    std::size_t dispatch_count = 0U;
    std::size_t error_count = 0U;
    bool dry_run = true;
    bool mutates_asset = false;

    auto summarize_dispatch = [&](const auto& dispatch) {
        if (dispatch.ok) {
            ++dispatch_count;
            dry_run = dry_run && dispatch.plan.dry_run;
            mutates_asset = mutates_asset || dispatch.plan.mutates_asset;
        } else {
            ++error_count;
        }
    };
    for (const auto& dispatch : editor_dispatches) {
        summarize_dispatch(dispatch);
    }
    for (const auto& dispatch : builder_dispatches) {
        summarize_dispatch(dispatch);
    }
    summarize_dispatch(toolbox_dispatch);

    return {
        .ok = true,
        .error = {},
        .plan = {
            .selection_context = admission_plan.selection_context,
            .asset_path = admission_plan.asset_path,
            .record_index = admission_plan.record_index,
            .object_name = admission_plan.object_name,
            .unique_id = admission_plan.unique_id,
            .symbol = admission_plan.symbol,
            .line = admission_plan.line,
            .column = admission_plan.column,
            .editor_action_dispatch_count = editor_dispatches.size(),
            .builder_dispatch_count = builder_dispatches.size(),
            .toolbox_dispatch_count = toolbox_dispatch.ok ? 1U : 0U,
            .dispatch_count = dispatch_count,
            .error_count = error_count,
            .editor_action_dispatches = std::move(editor_dispatches),
            .builder_dispatches = std::move(builder_dispatches),
            .toolbox_dispatch = std::move(toolbox_dispatch),
            .dry_run = dry_run,
            .mutates_asset = mutates_asset
        }
    };
}

}  // namespace copperfin::studio
