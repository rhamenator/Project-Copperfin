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

StudioDesignerDispatchCatalogResult plan_studio_designer_dispatch_catalog(
    const StudioDesignerDispatchCatalogRequest& request) {
    auto admission_catalog = plan_studio_designer_invocation_admission_catalog({
        .asset_path = request.asset_path,
        .record_index = request.record_index,
        .object_name = request.object_name,
        .unique_id = request.unique_id,
        .symbol = request.symbol,
        .line = request.line,
        .column = request.column,
        .admit_editor_invocations = request.admit_editor_invocations,
        .admit_builder_invocations = request.admit_builder_invocations,
        .admit_toolbox_invocation = request.admit_toolbox_invocation
    });
    if (!admission_catalog.ok) {
        return {
            .ok = false,
            .error = admission_catalog.error,
            .context_count = 0U,
            .contexts = {}
        };
    }

    std::vector<StudioDesignerDispatchCatalogEntry> entries;
    entries.reserve(admission_catalog.contexts.size());
    for (auto& admission_entry : admission_catalog.contexts) {
        StudioDesignerDispatchResult dispatch{};
        if (admission_entry.invocation_admission.ok) {
            dispatch = plan_studio_designer_dispatch({
                .invocation_admission_plan = admission_entry.invocation_admission.plan
            });
        } else {
            dispatch = {
                .ok = false,
                .error = admission_entry.invocation_admission.error,
                .plan = {}
            };
        }

        const auto& plan = dispatch.plan;
        entries.push_back({
            .selection_context = admission_entry.selection_context,
            .editor_action_dispatch_count = dispatch.ok ? plan.editor_action_dispatch_count : 0U,
            .builder_dispatch_count = dispatch.ok ? plan.builder_dispatch_count : 0U,
            .toolbox_dispatch_count = dispatch.ok ? plan.toolbox_dispatch_count : 0U,
            .dispatch_count = dispatch.ok ? plan.dispatch_count : 0U,
            .error_count = dispatch.ok ? plan.error_count : 1U,
            .dry_run = dispatch.ok ? plan.dry_run : true,
            .mutates_asset = dispatch.ok ? plan.mutates_asset : false,
            .dispatch = std::move(dispatch)
        });
    }

    return {
        .ok = true,
        .error = {},
        .context_count = admission_catalog.context_count,
        .contexts = std::move(entries)
    };
}

}  // namespace copperfin::studio
