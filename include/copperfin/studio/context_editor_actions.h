#pragma once

#include <string_view>
#include <vector>

namespace copperfin::studio {

enum class StudioEditorSelectionContext {
    visual_object,
    visual_method,
    container_object,
    class_designer,
    report_expression,
    label_expression,
    menu_item,
    project_item,
    data_environment
};

enum class StudioEditorActionKind {
    property_grid,
    source_editor,
    expression_editor,
    builder,
    toolbox,
    navigator
};

struct StudioEditorActionDescriptor {
    std::string_view id;
    std::string_view label;
    StudioEditorActionKind kind = StudioEditorActionKind::property_grid;
    std::vector<StudioEditorSelectionContext> contexts;
    std::string_view command_token;
    std::string_view target_surface;
    std::string_view description;
};

[[nodiscard]] const char* studio_editor_selection_context_name(StudioEditorSelectionContext context);
[[nodiscard]] const char* studio_editor_action_kind_name(StudioEditorActionKind kind);
[[nodiscard]] const std::vector<StudioEditorActionDescriptor>& studio_editor_action_registry();
[[nodiscard]] std::vector<StudioEditorActionDescriptor> studio_editor_actions_for_context(
    StudioEditorSelectionContext context);

}  // namespace copperfin::studio
