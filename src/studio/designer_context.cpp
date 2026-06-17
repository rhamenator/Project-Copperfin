#include "copperfin/studio/designer_context.h"

#include <utility>

namespace copperfin::studio {

namespace {

std::vector<StudioBuilderDescriptor> builders_for_selection_context(StudioEditorSelectionContext context) {
    switch (context) {
        case StudioEditorSelectionContext::visual_object:
        {
            auto builders = studio_builders_for_context(StudioBuilderContext::form);
            auto control_builders = studio_builders_for_context(StudioBuilderContext::control);
            builders.insert(builders.end(), control_builders.begin(), control_builders.end());
            return builders;
        }
        case StudioEditorSelectionContext::visual_method:
        case StudioEditorSelectionContext::container_object:
            return studio_builders_for_context(StudioBuilderContext::control);
        case StudioEditorSelectionContext::class_designer:
            return studio_builders_for_context(StudioBuilderContext::class_designer);
        case StudioEditorSelectionContext::report_expression:
            return studio_builders_for_context(StudioBuilderContext::report);
        case StudioEditorSelectionContext::label_expression:
            return studio_builders_for_context(StudioBuilderContext::label);
        case StudioEditorSelectionContext::menu_item:
            return studio_builders_for_context(StudioBuilderContext::menu);
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
        case StudioEditorSelectionContext::container_object:
            return studio_toolbox_items_for_context(StudioToolboxContext::container);
        case StudioEditorSelectionContext::class_designer:
            return studio_toolbox_items_for_context(StudioToolboxContext::class_designer);
        case StudioEditorSelectionContext::report_expression:
        case StudioEditorSelectionContext::label_expression:
            return studio_toolbox_items_for_context(StudioToolboxContext::report);
        case StudioEditorSelectionContext::menu_item:
            return {};
        case StudioEditorSelectionContext::project_item:
        case StudioEditorSelectionContext::data_environment:
            return {};
    }
    return {};
}

}  // namespace

StudioDesignerContextResult studio_designer_context_for_selection(const StudioDesignerContextRequest& request) {
    auto editor_actions = studio_editor_actions_for_context(request.selection_context);
    auto builders = builders_for_selection_context(request.selection_context);
    auto toolbox_items = toolbox_items_for_selection_context(request.selection_context);

    return {
        .selection_context = request.selection_context,
        .editor_action_count = editor_actions.size(),
        .builder_count = builders.size(),
        .toolbox_item_count = toolbox_items.size(),
        .editor_actions = std::move(editor_actions),
        .builders = std::move(builders),
        .toolbox_items = std::move(toolbox_items)
    };
}

}  // namespace copperfin::studio
