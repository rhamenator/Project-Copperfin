#include "copperfin/studio/designer_context.h"

namespace copperfin::studio {

namespace {

std::vector<StudioBuilderDescriptor> builders_for_selection_context(StudioEditorSelectionContext context) {
    switch (context) {
        case StudioEditorSelectionContext::visual_object:
        case StudioEditorSelectionContext::visual_method:
            return studio_builders_for_context(StudioBuilderContext::control);
        case StudioEditorSelectionContext::report_expression:
            return studio_builders_for_context(StudioBuilderContext::report);
        case StudioEditorSelectionContext::project_item:
            return studio_builders_for_context(StudioBuilderContext::project);
        case StudioEditorSelectionContext::data_environment:
            return studio_builders_for_context(StudioBuilderContext::data_environment);
    }
    return {};
}

std::vector<StudioToolboxItemDescriptor> toolbox_items_for_selection_context(StudioEditorSelectionContext context) {
    switch (context) {
        case StudioEditorSelectionContext::visual_object:
        case StudioEditorSelectionContext::visual_method:
            return studio_toolbox_items_for_context(StudioToolboxContext::form);
        case StudioEditorSelectionContext::report_expression:
            return studio_toolbox_items_for_context(StudioToolboxContext::report);
        case StudioEditorSelectionContext::project_item:
        case StudioEditorSelectionContext::data_environment:
            return {};
    }
    return {};
}

}  // namespace

StudioDesignerContextResult studio_designer_context_for_selection(const StudioDesignerContextRequest& request) {
    return {
        .selection_context = request.selection_context,
        .editor_actions = studio_editor_actions_for_context(request.selection_context),
        .builders = builders_for_selection_context(request.selection_context),
        .toolbox_items = toolbox_items_for_selection_context(request.selection_context)
    };
}

}  // namespace copperfin::studio
