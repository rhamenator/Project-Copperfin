#include "copperfin/studio/context_editor_actions.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

bool has_action(
    const std::vector<copperfin::studio::StudioEditorActionDescriptor>& actions,
    std::string_view id) {
    for (const auto& action : actions) {
        if (action.id == id) {
            return true;
        }
    }
    return false;
}

}  // namespace

int main() {
    using copperfin::studio::StudioEditorActionKind;
    using copperfin::studio::StudioEditorSelectionContext;

    const auto& actions = copperfin::studio::studio_editor_action_registry();
    expect(actions.size() >= 7U, "#958: editor action registry should expose core context-aware actions");
    expect(std::string(copperfin::studio::studio_editor_selection_context_name(
               StudioEditorSelectionContext::visual_object)) == "visual_object",
           "#958: visual-object context token should be stable");
    expect(std::string(copperfin::studio::studio_editor_selection_context_name(
               StudioEditorSelectionContext::class_designer)) == "class_designer",
           "#1012: class-designer context token should be stable");
    expect(std::string(copperfin::studio::studio_editor_selection_context_name(
               StudioEditorSelectionContext::report_expression)) == "report_expression",
           "#958: report-expression context token should be stable");
    expect(std::string(copperfin::studio::studio_editor_selection_context_name(
               StudioEditorSelectionContext::label_expression)) == "label_expression",
           "#1011: label-expression context token should be stable");
    expect(std::string(copperfin::studio::studio_editor_selection_context_name(
               StudioEditorSelectionContext::menu_item)) == "menu_item",
           "#1013: menu-item context token should be stable");
    expect(std::string(copperfin::studio::studio_editor_action_kind_name(
               StudioEditorActionKind::expression_editor)) == "expression_editor",
           "#958: expression-editor action token should be stable");
    expect(std::string(copperfin::studio::studio_editor_action_kind_name(StudioEditorActionKind::toolbox)) ==
               "toolbox",
           "#958: toolbox action token should be stable");

    bool found_property_grid = false;
    bool found_builder = false;
    bool found_toolbox = false;
    bool found_navigator = false;

    for (const auto& action : actions) {
        expect(!std::string(action.id).empty(), "#958: each editor action should have an id");
        expect(!std::string(action.label).empty(), "#958: each editor action should have a label");
        expect(!action.contexts.empty(), "#958: each editor action should name at least one selection context");
        expect(!std::string(action.command_token).empty(), "#958: each editor action should have a command token");
        expect(!std::string(action.target_surface).empty(), "#958: each editor action should name a target surface");
        expect(!std::string(action.description).empty(), "#958: each editor action should describe its behavior");
        if (action.kind == StudioEditorActionKind::property_grid) {
            found_property_grid = true;
        }
        if (action.kind == StudioEditorActionKind::builder) {
            found_builder = true;
        }
        if (action.kind == StudioEditorActionKind::toolbox) {
            found_toolbox = true;
        }
        if (action.kind == StudioEditorActionKind::navigator) {
            found_navigator = true;
        }
    }

    expect(found_property_grid, "#958: registry should include property-grid actions");
    expect(found_builder, "#958: registry should include builder actions");
    expect(found_toolbox, "#958: registry should include toolbox actions");
    expect(found_navigator, "#958: registry should include navigation actions");
    expect(has_action(actions, "show-property-grid"), "#958: registry should include property-grid action");
    expect(has_action(actions, "edit-visual-method"), "#958: registry should include method editor action");
    expect(has_action(actions, "edit-report-expression"), "#958: registry should include expression editor action");
    expect(has_action(actions, "open-builder"), "#958: registry should include builder action");
    expect(has_action(actions, "show-toolbox"), "#958: registry should include toolbox action");

    const auto visual_actions = copperfin::studio::studio_editor_actions_for_context(
        StudioEditorSelectionContext::visual_object);
    expect(has_action(visual_actions, "show-property-grid"),
           "#958: visual-object context should expose property grid");
    expect(has_action(visual_actions, "edit-visual-method"),
           "#958: visual-object context should expose method editor");
    expect(has_action(visual_actions, "show-toolbox"), "#958: visual-object context should expose toolbox");
    expect(!has_action(visual_actions, "edit-report-expression"),
           "#958: visual-object context should exclude report expression editor");

    const auto class_actions = copperfin::studio::studio_editor_actions_for_context(
        StudioEditorSelectionContext::class_designer);
    expect(has_action(class_actions, "show-property-grid"),
           "#1012: class-designer context should expose property grid");
    expect(has_action(class_actions, "edit-visual-method"),
           "#1012: class-designer context should expose method editor");
    expect(has_action(class_actions, "open-builder"),
           "#1012: class-designer context should expose builders");
    expect(has_action(class_actions, "show-toolbox"),
           "#1012: class-designer context should expose toolbox");
    expect(!has_action(class_actions, "edit-report-expression"),
           "#1012: class-designer context should exclude report expression editor");

    const auto report_actions = copperfin::studio::studio_editor_actions_for_context(
        StudioEditorSelectionContext::report_expression);
    expect(has_action(report_actions, "edit-report-expression"),
           "#958: report-expression context should expose expression editor");
    expect(has_action(report_actions, "open-builder"), "#958: report-expression context should expose builders");
    expect(!has_action(report_actions, "edit-visual-method"),
           "#958: report-expression context should exclude method editor");

    const auto label_actions = copperfin::studio::studio_editor_actions_for_context(
        StudioEditorSelectionContext::label_expression);
    expect(has_action(label_actions, "edit-report-expression"),
           "#1011: label-expression context should expose expression editor");
    expect(has_action(label_actions, "open-builder"), "#1011: label-expression context should expose builders");
    expect(has_action(label_actions, "show-toolbox"), "#1011: label-expression context should expose report-safe toolbox");
    expect(!has_action(label_actions, "edit-visual-method"),
           "#1011: label-expression context should exclude method editor");

    const auto menu_actions = copperfin::studio::studio_editor_actions_for_context(
        StudioEditorSelectionContext::menu_item);
    expect(has_action(menu_actions, "show-property-grid"),
           "#1013: menu-item context should expose property grid");
    expect(has_action(menu_actions, "open-builder"),
           "#1013: menu-item context should expose builders");
    expect(!has_action(menu_actions, "show-toolbox"),
           "#1013: menu-item context should not expose toolbox");
    expect(!has_action(menu_actions, "edit-visual-method"),
           "#1013: menu-item context should exclude method editor");
    expect(!has_action(menu_actions, "edit-report-expression"),
           "#1013: menu-item context should exclude report expression editor");

    const auto project_actions = copperfin::studio::studio_editor_actions_for_context(
        StudioEditorSelectionContext::project_item);
    expect(has_action(project_actions, "navigate-project-item"),
           "#958: project-item context should expose project navigation");
    expect(has_action(project_actions, "open-builder"), "#958: project-item context should expose builders");
    expect(!has_action(project_actions, "show-toolbox"), "#958: project-item context should exclude toolbox");

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
