#pragma once

#include <cstddef>
#include <string>
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

struct StudioEditorActionLaunchRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string action_id;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string symbol;
    std::size_t line = 0;
    std::size_t column = 0;
};

struct StudioEditorActionLaunchPlan {
    StudioEditorActionDescriptor action;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string symbol;
    std::size_t line = 0;
    std::size_t column = 0;
    std::string command_token;
    std::string target_surface;
};

struct StudioEditorActionLaunchPlanResult {
    bool ok = false;
    std::string error;
    StudioEditorActionLaunchPlan plan;
};

[[nodiscard]] const char* studio_editor_selection_context_name(StudioEditorSelectionContext context);
[[nodiscard]] const char* studio_editor_action_kind_name(StudioEditorActionKind kind);
[[nodiscard]] const std::vector<StudioEditorActionDescriptor>& studio_editor_action_registry();
[[nodiscard]] std::vector<StudioEditorActionDescriptor> studio_editor_actions_for_context(
    StudioEditorSelectionContext context);
[[nodiscard]] StudioEditorActionLaunchPlanResult plan_studio_editor_action_launch(
    const StudioEditorActionLaunchRequest& request);

}  // namespace copperfin::studio
