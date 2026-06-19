#include "copperfin/studio/context_editor_actions.h"

#include <algorithm>
#include <string>

namespace copperfin::studio {

namespace {

bool supports_context(const StudioEditorActionDescriptor& action, StudioEditorSelectionContext context) {
    return std::find(action.contexts.begin(), action.contexts.end(), context) != action.contexts.end();
}

}  // namespace

const char* studio_editor_selection_context_name(StudioEditorSelectionContext context) {
    switch (context) {
        case StudioEditorSelectionContext::visual_object:
            return "visual_object";
        case StudioEditorSelectionContext::visual_method:
            return "visual_method";
        case StudioEditorSelectionContext::container_object:
            return "container_object";
        case StudioEditorSelectionContext::class_designer:
            return "class_designer";
        case StudioEditorSelectionContext::report_expression:
            return "report_expression";
        case StudioEditorSelectionContext::label_expression:
            return "label_expression";
        case StudioEditorSelectionContext::menu_item:
            return "menu_item";
        case StudioEditorSelectionContext::project_item:
            return "project_item";
        case StudioEditorSelectionContext::data_environment:
            return "data_environment";
    }
    return "visual_object";
}

const char* studio_editor_action_kind_name(StudioEditorActionKind kind) {
    switch (kind) {
        case StudioEditorActionKind::property_grid:
            return "property_grid";
        case StudioEditorActionKind::source_editor:
            return "source_editor";
        case StudioEditorActionKind::expression_editor:
            return "expression_editor";
        case StudioEditorActionKind::builder:
            return "builder";
        case StudioEditorActionKind::toolbox:
            return "toolbox";
        case StudioEditorActionKind::navigator:
            return "navigator";
    }
    return "property_grid";
}

const std::vector<StudioEditorActionDescriptor>& studio_editor_action_registry() {
    using Context = StudioEditorSelectionContext;
    using Kind = StudioEditorActionKind;

    static const std::vector<StudioEditorActionDescriptor> actions = {
        {
            .id = "show-property-grid",
            .label = "Properties",
            .kind = Kind::property_grid,
            .contexts = {
                Context::visual_object,
                Context::container_object,
                Context::class_designer,
                Context::report_expression,
                Context::label_expression,
                Context::menu_item,
                Context::project_item
            },
            .command_token = "studio.property_grid.show",
            .target_surface = "property-grid",
            .description = "Show the selected object's direct and memo-backed VFP properties."
        },
        {
            .id = "edit-visual-method",
            .label = "Edit Method",
            .kind = Kind::source_editor,
            .contexts = {
                Context::visual_object,
                Context::visual_method,
                Context::container_object,
                Context::class_designer
            },
            .command_token = "studio.method_editor.open",
            .target_surface = "method-editor",
            .description = "Open the selected visual object's PROCEDURE/FUNCTION source in a method editor."
        },
        {
            .id = "edit-report-expression",
            .label = "Edit Expression",
            .kind = Kind::expression_editor,
            .contexts = {Context::report_expression, Context::label_expression},
            .command_token = "studio.expression_editor.open",
            .target_surface = "expression-editor",
            .description = "Open FRX/LBX expression text with report/label source provenance."
        },
        {
            .id = "open-builder",
            .label = "Builder",
            .kind = Kind::builder,
            .contexts = {
                Context::visual_object,
                Context::container_object,
                Context::class_designer,
                Context::report_expression,
                Context::label_expression,
                Context::menu_item,
                Context::project_item,
                Context::data_environment
            },
            .command_token = "studio.builder.open_for_context",
            .target_surface = "builder-registry",
            .description = "Open the context-filtered VFP-compatible builder or wizard list."
        },
        {
            .id = "show-toolbox",
            .label = "Toolbox",
            .kind = Kind::toolbox,
            .contexts = {
                Context::visual_object,
                Context::container_object,
                Context::class_designer,
                Context::report_expression,
                Context::label_expression
            },
            .command_token = "studio.toolbox.show_for_context",
            .target_surface = "toolbox-palette",
            .description = "Show toolbox items relevant to the active visual designer context."
        },
        {
            .id = "edit-data-environment",
            .label = "Data Environment",
            .kind = Kind::builder,
            .contexts = {Context::visual_object, Context::report_expression, Context::label_expression, Context::data_environment},
            .command_token = "studio.data_environment.open",
            .target_surface = "data-environment",
            .description = "Open data-environment bindings for forms, reports, and selected data-context entries."
        },
        {
            .id = "navigate-project-item",
            .label = "Go To Project Item",
            .kind = Kind::navigator,
            .contexts = {Context::project_item},
            .command_token = "studio.project_item.navigate",
            .target_surface = "project-explorer",
            .description = "Navigate from the active designer/editor selection back to the owning PJX/PJT item."
        }
    };

    return actions;
}

std::vector<StudioEditorActionDescriptor> studio_editor_actions_for_context(StudioEditorSelectionContext context) {
    std::vector<StudioEditorActionDescriptor> filtered;
    const auto& actions = studio_editor_action_registry();
    std::copy_if(actions.begin(), actions.end(), std::back_inserter(filtered), [&](const StudioEditorActionDescriptor& action) {
        return supports_context(action, context);
    });
    return filtered;
}

StudioEditorActionLaunchPlanResult plan_studio_editor_action_launch(
    const StudioEditorActionLaunchRequest& request) {
    if (request.action_id.empty()) {
        return {
            .ok = false,
            .error = "An editor action launch request requires an action id.",
            .plan = {}
        };
    }

    const auto actions = studio_editor_actions_for_context(request.selection_context);
    const auto action = std::find_if(
        actions.begin(),
        actions.end(),
        [&](const StudioEditorActionDescriptor& candidate) {
            return candidate.id == request.action_id;
        });

    if (action == actions.end()) {
        return {
            .ok = false,
            .error = "The requested editor action is not available for the selected Studio context.",
            .plan = {}
        };
    }

    return {
        .ok = true,
        .error = {},
        .plan = {
            .action = *action,
            .selection_context = request.selection_context,
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .symbol = request.symbol,
            .line = request.line,
            .column = request.column,
            .command_token = std::string(action->command_token),
            .target_surface = std::string(action->target_surface)
        }
    };
}

StudioEditorActionLaunchCatalogResult plan_studio_editor_action_launch_catalog(
    const StudioEditorActionLaunchCatalogRequest& request) {
    const auto actions = studio_editor_actions_for_context(request.selection_context);
    if (actions.empty()) {
        return {
            .ok = false,
            .error = "An editor action launch catalog request requires at least one action.",
            .selection_context = request.selection_context,
            .action_count = 0U,
            .launch_plan_count = 0U,
            .error_count = 0U,
            .dry_run = true,
            .mutates_asset = false,
            .entries = {}
        };
    }

    std::vector<StudioEditorActionLaunchCatalogEntry> entries;
    entries.reserve(actions.size());
    std::size_t launch_plan_count = 0U;
    std::size_t error_count = 0U;

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

        if (launch_plan.ok) {
            ++launch_plan_count;
        } else {
            ++error_count;
        }

        entries.push_back({
            .action = action,
            .launch_plan = std::move(launch_plan)
        });
    }

    return {
        .ok = true,
        .error = {},
        .selection_context = request.selection_context,
        .action_count = actions.size(),
        .launch_plan_count = launch_plan_count,
        .error_count = error_count,
        .dry_run = true,
        .mutates_asset = false,
        .entries = std::move(entries)
    };
}

}  // namespace copperfin::studio
