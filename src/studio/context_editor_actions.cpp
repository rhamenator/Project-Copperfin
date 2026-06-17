#include "copperfin/studio/context_editor_actions.h"

#include <algorithm>

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
        case StudioEditorSelectionContext::report_expression:
            return "report_expression";
        case StudioEditorSelectionContext::label_expression:
            return "label_expression";
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
            .contexts = {Context::visual_object, Context::report_expression, Context::label_expression, Context::project_item},
            .command_token = "studio.property_grid.show",
            .target_surface = "property-grid",
            .description = "Show the selected object's direct and memo-backed VFP properties."
        },
        {
            .id = "edit-visual-method",
            .label = "Edit Method",
            .kind = Kind::source_editor,
            .contexts = {Context::visual_object, Context::visual_method},
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
                Context::report_expression,
                Context::label_expression,
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
            .contexts = {Context::visual_object, Context::report_expression, Context::label_expression},
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

}  // namespace copperfin::studio
