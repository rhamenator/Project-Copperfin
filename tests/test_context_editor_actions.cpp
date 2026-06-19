#include "copperfin/studio/context_editor_actions.h"
#include "copperfin/studio/editor_action_dispatch.h"
#include "copperfin/studio/editor_action_invocation_admission.h"

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

bool has_argument_pair(const std::vector<std::string>& arguments, const std::string& key, const std::string& value) {
    for (std::size_t index = 0U; (index + 1U) < arguments.size(); index += 2U) {
        if (arguments[index] == key && arguments[index + 1U] == value) {
            return true;
        }
    }
    return false;
}

const copperfin::studio::StudioEditorActionDispatchCatalogEntry* find_dispatch_catalog_entry(
    const std::vector<copperfin::studio::StudioEditorActionDispatchCatalogEntry>& entries,
    std::string_view id) {
    for (const auto& entry : entries) {
        if (entry.action.id == id) {
            return &entry;
        }
    }
    return nullptr;
}

const copperfin::studio::StudioEditorActionLaunchCatalogEntry* find_launch_catalog_entry(
    const std::vector<copperfin::studio::StudioEditorActionLaunchCatalogEntry>& entries,
    std::string_view id) {
    for (const auto& entry : entries) {
        if (entry.action.id == id) {
            return &entry;
        }
    }
    return nullptr;
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
               StudioEditorSelectionContext::container_object)) == "container_object",
           "#1014: container-object context token should be stable");
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

    const auto container_actions = copperfin::studio::studio_editor_actions_for_context(
        StudioEditorSelectionContext::container_object);
    expect(has_action(container_actions, "show-property-grid"),
           "#1014: container-object context should expose property grid");
    expect(has_action(container_actions, "edit-visual-method"),
           "#1014: container-object context should expose method editor");
    expect(has_action(container_actions, "open-builder"),
           "#1014: container-object context should expose builders");
    expect(has_action(container_actions, "show-toolbox"),
           "#1014: container-object context should expose toolbox");
    expect(!has_action(container_actions, "edit-report-expression"),
           "#1014: container-object context should exclude report expression editor");

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

    const auto property_plan = copperfin::studio::plan_studio_editor_action_launch({
        .selection_context = StudioEditorSelectionContext::visual_object,
        .action_id = "show-property-grid",
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "txtName",
        .unique_id = "textbox-guid",
        .symbol = "txtName",
        .line = 0U,
        .column = 0U
    });
    expect(property_plan.ok,
           "#1207: visual-object editor action launch plans should accept property-grid actions");
    expect(property_plan.plan.selection_context == StudioEditorSelectionContext::visual_object &&
               std::string(property_plan.plan.action.id) == "show-property-grid" &&
               property_plan.plan.action.kind == StudioEditorActionKind::property_grid &&
               property_plan.plan.command_token == "studio.property_grid.show" &&
               property_plan.plan.target_surface == "property-grid" &&
               property_plan.plan.asset_path == "forms/customer.scx" &&
               property_plan.plan.record_index == 1U &&
               property_plan.plan.object_name == "txtName" &&
               property_plan.plan.unique_id == "textbox-guid" &&
               property_plan.plan.symbol == "txtName",
           "#1207: editor action launch plans should preserve action and selected-object metadata");

    const auto method_plan = copperfin::studio::plan_studio_editor_action_launch({
        .selection_context = StudioEditorSelectionContext::visual_object,
        .action_id = "edit-visual-method",
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "cmdSave",
        .unique_id = "button-guid",
        .symbol = "cmdSave.Click",
        .line = 42U,
        .column = 7U
    });
    expect(method_plan.ok &&
               method_plan.plan.action.kind == StudioEditorActionKind::source_editor &&
               method_plan.plan.command_token == "studio.method_editor.open" &&
               method_plan.plan.line == 42U &&
               method_plan.plan.column == 7U,
           "#1207: visual-object editor action launch plans should accept method editor actions");

    const auto visual_launch_catalog = copperfin::studio::plan_studio_editor_action_launch_catalog({
        .selection_context = StudioEditorSelectionContext::visual_object,
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "cmdSave",
        .unique_id = "button-guid",
        .symbol = "cmdSave.Click",
        .line = 42U,
        .column = 7U
    });
    const auto* property_launch_entry = find_launch_catalog_entry(
        visual_launch_catalog.entries, "show-property-grid");
    const auto* method_launch_entry = find_launch_catalog_entry(
        visual_launch_catalog.entries, "edit-visual-method");
    expect(visual_launch_catalog.ok &&
               visual_launch_catalog.selection_context == StudioEditorSelectionContext::visual_object &&
               visual_launch_catalog.action_count == visual_actions.size() &&
               visual_launch_catalog.launch_plan_count == visual_actions.size() &&
               visual_launch_catalog.error_count == 0U &&
               visual_launch_catalog.dry_run &&
               !visual_launch_catalog.mutates_asset,
           "#1279: visual editor action launch catalogs should plan every visual action");
    expect(property_launch_entry != nullptr &&
               property_launch_entry->launch_plan.ok &&
               property_launch_entry->launch_plan.plan.selection_context ==
                   StudioEditorSelectionContext::visual_object &&
               property_launch_entry->launch_plan.plan.action.kind ==
                   StudioEditorActionKind::property_grid &&
               property_launch_entry->launch_plan.plan.command_token == "studio.property_grid.show" &&
               property_launch_entry->launch_plan.plan.target_surface == "property-grid" &&
               property_launch_entry->launch_plan.plan.asset_path == "forms/customer.scx" &&
               property_launch_entry->launch_plan.plan.record_index == 1U &&
               property_launch_entry->launch_plan.plan.object_name == "cmdSave" &&
               property_launch_entry->launch_plan.plan.unique_id == "button-guid" &&
               property_launch_entry->launch_plan.plan.symbol == "cmdSave.Click" &&
               property_launch_entry->launch_plan.plan.line == 42U &&
               property_launch_entry->launch_plan.plan.column == 7U,
           "#1279: editor action launch catalogs should preserve property-grid launch metadata");
    expect(method_launch_entry != nullptr &&
               method_launch_entry->launch_plan.ok &&
               method_launch_entry->launch_plan.plan.action.kind ==
                   StudioEditorActionKind::source_editor &&
               method_launch_entry->launch_plan.plan.command_token == "studio.method_editor.open" &&
               method_launch_entry->launch_plan.plan.target_surface == "method-editor",
           "#1279: visual editor action launch catalogs should include method editor actions");

    const auto report_launch_catalog = copperfin::studio::plan_studio_editor_action_launch_catalog({
        .selection_context = StudioEditorSelectionContext::report_expression,
        .asset_path = "reports/orders.frx",
        .record_index = 3U,
        .object_name = "Field1",
        .unique_id = "field-guid",
        .symbol = "Expr",
        .line = 5U,
        .column = 2U
    });
    const auto* report_expression_entry = find_launch_catalog_entry(
        report_launch_catalog.entries, "edit-report-expression");
    expect(report_launch_catalog.ok &&
               report_launch_catalog.action_count == report_actions.size() &&
               report_expression_entry != nullptr &&
               report_expression_entry->launch_plan.ok &&
               report_expression_entry->launch_plan.plan.action.kind ==
                   StudioEditorActionKind::expression_editor &&
               report_expression_entry->launch_plan.plan.command_token ==
                   "studio.expression_editor.open" &&
               report_expression_entry->launch_plan.plan.target_surface ==
                   "expression-editor" &&
               report_expression_entry->launch_plan.plan.asset_path == "reports/orders.frx",
           "#1279: report editor action launch catalogs should preserve expression editor metadata");

    const auto empty_launch_catalog = copperfin::studio::plan_studio_editor_action_launch_catalog({
        .selection_context = static_cast<StudioEditorSelectionContext>(999),
        .asset_path = "forms/customer.scx",
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .symbol = {},
        .line = 0U,
        .column = 0U
    });
    expect(!empty_launch_catalog.ok &&
               empty_launch_catalog.error ==
                   "An editor action launch catalog request requires at least one action." &&
               empty_launch_catalog.action_count == 0U &&
               empty_launch_catalog.launch_plan_count == 0U &&
               empty_launch_catalog.error_count == 0U &&
               empty_launch_catalog.dry_run &&
               !empty_launch_catalog.mutates_asset,
           "#1279: editor action launch catalogs should reject empty action contexts without mutation");

    const auto admitted_method_invocation = copperfin::studio::plan_studio_editor_action_invocation_admission({
        .launch_plan = method_plan.plan,
        .admit_editor_invocation = true
    });
    expect(admitted_method_invocation.ok,
           "#1217: editor action invocation admission should accept validated launch plans");
    expect(std::string(admitted_method_invocation.plan.action.id) == "edit-visual-method" &&
               admitted_method_invocation.plan.action.kind == StudioEditorActionKind::source_editor &&
               admitted_method_invocation.plan.selection_context == StudioEditorSelectionContext::visual_object &&
               admitted_method_invocation.plan.command_token == "studio.method_editor.open" &&
               admitted_method_invocation.plan.target_surface == "method-editor" &&
               admitted_method_invocation.plan.asset_path == "forms/customer.scx" &&
               admitted_method_invocation.plan.record_index == 1U &&
               admitted_method_invocation.plan.object_name == "cmdSave" &&
               admitted_method_invocation.plan.unique_id == "button-guid" &&
               admitted_method_invocation.plan.symbol == "cmdSave.Click" &&
               admitted_method_invocation.plan.line == 42U &&
               admitted_method_invocation.plan.column == 7U,
           "#1217: editor action invocation admission should preserve launch metadata");
    expect(admitted_method_invocation.plan.editor_invocation_admitted &&
               !admitted_method_invocation.plan.dry_run &&
               !admitted_method_invocation.plan.mutates_asset,
           "#1217: admitted editor action invocation plans should allow editor invocation while remaining non-mutating");

    const auto dry_run_property_invocation = copperfin::studio::plan_studio_editor_action_invocation_admission({
        .launch_plan = property_plan.plan,
        .admit_editor_invocation = false
    });
    expect(dry_run_property_invocation.ok &&
               std::string(dry_run_property_invocation.plan.action.id) == "show-property-grid" &&
               dry_run_property_invocation.plan.action.kind == StudioEditorActionKind::property_grid &&
               !dry_run_property_invocation.plan.editor_invocation_admitted &&
               dry_run_property_invocation.plan.dry_run &&
               !dry_run_property_invocation.plan.mutates_asset,
           "#1217: non-admitted editor action invocation plans should remain deterministic dry runs");

    const auto method_dispatch = copperfin::studio::plan_studio_editor_action_dispatch({
        .admission_plan = admitted_method_invocation.plan
    });
    expect(method_dispatch.ok,
           "#1225: editor action dispatch should accept admitted method editor invocations");
    expect(std::string(method_dispatch.plan.action.id) == "edit-visual-method" &&
               method_dispatch.plan.action.kind == StudioEditorActionKind::source_editor &&
               method_dispatch.plan.selection_context == StudioEditorSelectionContext::visual_object &&
               method_dispatch.plan.command_token == "studio.method_editor.open" &&
               method_dispatch.plan.target_surface == "method-editor" &&
               method_dispatch.plan.asset_path == "forms/customer.scx" &&
               method_dispatch.plan.record_index == 1U &&
               method_dispatch.plan.object_name == "cmdSave" &&
               method_dispatch.plan.unique_id == "button-guid" &&
               method_dispatch.plan.symbol == "cmdSave.Click" &&
               method_dispatch.plan.line == 42U &&
               method_dispatch.plan.column == 7U &&
               method_dispatch.plan.dispatch_admitted &&
               !method_dispatch.plan.dry_run &&
               !method_dispatch.plan.executed &&
               !method_dispatch.plan.mutates_asset,
           "#1225: editor action dispatch should preserve admission metadata without executing");
    expect(has_argument_pair(method_dispatch.plan.dispatch_arguments, "--command-token", "studio.method_editor.open") &&
               has_argument_pair(method_dispatch.plan.dispatch_arguments, "--action-id", "edit-visual-method") &&
               has_argument_pair(method_dispatch.plan.dispatch_arguments, "--selection-context", "visual_object") &&
               has_argument_pair(method_dispatch.plan.dispatch_arguments, "--target-surface", "method-editor") &&
               has_argument_pair(method_dispatch.plan.dispatch_arguments, "--path", "forms/customer.scx") &&
               has_argument_pair(method_dispatch.plan.dispatch_arguments, "--record", "1") &&
               has_argument_pair(method_dispatch.plan.dispatch_arguments, "--object-name", "cmdSave") &&
               has_argument_pair(method_dispatch.plan.dispatch_arguments, "--unique-id", "button-guid") &&
               has_argument_pair(method_dispatch.plan.dispatch_arguments, "--symbol", "cmdSave.Click") &&
               has_argument_pair(method_dispatch.plan.dispatch_arguments, "--line", "42") &&
               has_argument_pair(method_dispatch.plan.dispatch_arguments, "--column", "7"),
           "#1225: editor action dispatch should materialize a deterministic argument contract");

    const auto dry_run_dispatch = copperfin::studio::plan_studio_editor_action_dispatch({
        .admission_plan = dry_run_property_invocation.plan
    });
    expect(!dry_run_dispatch.ok &&
               dry_run_dispatch.error ==
                   "An editor action dispatch request requires an admitted non-dry-run invocation.",
           "#1225: editor action dispatch should reject dry-run admission plans");

    auto missing_command_plan = method_plan.plan;
    missing_command_plan.command_token = {};
    const auto missing_command_invocation = copperfin::studio::plan_studio_editor_action_invocation_admission({
        .launch_plan = missing_command_plan,
        .admit_editor_invocation = true
    });
    expect(!missing_command_invocation.ok,
           "#1217: editor action invocation admission should reject launch plans without command tokens");

    auto missing_action_plan_for_invocation = method_plan.plan;
    missing_action_plan_for_invocation.action = {};
    const auto missing_action_invocation = copperfin::studio::plan_studio_editor_action_invocation_admission({
        .launch_plan = missing_action_plan_for_invocation,
        .admit_editor_invocation = true
    });
    expect(!missing_action_invocation.ok,
           "#1217: editor action invocation admission should reject launch plans without action ids");

    const auto expression_plan = copperfin::studio::plan_studio_editor_action_launch({
        .selection_context = StudioEditorSelectionContext::report_expression,
        .action_id = "edit-report-expression",
        .asset_path = "reports/orders.frx",
        .record_index = 2U,
        .object_name = "Expr1",
        .unique_id = "expr-guid",
        .symbol = "Expr1.Expression",
        .line = 3U,
        .column = 11U
    });
    expect(expression_plan.ok &&
               expression_plan.plan.action.kind == StudioEditorActionKind::expression_editor &&
               expression_plan.plan.target_surface == "expression-editor",
           "#1207: report-expression editor action launch plans should accept expression editor actions");

    const auto expression_invocation = copperfin::studio::plan_studio_editor_action_invocation_admission({
        .launch_plan = expression_plan.plan,
        .admit_editor_invocation = true
    });
    const auto expression_dispatch = copperfin::studio::plan_studio_editor_action_dispatch({
        .admission_plan = expression_invocation.plan
    });
    expect(expression_dispatch.ok &&
               expression_dispatch.plan.action.kind == StudioEditorActionKind::expression_editor &&
               expression_dispatch.plan.command_token == "studio.expression_editor.open" &&
               expression_dispatch.plan.target_surface == "expression-editor" &&
               has_argument_pair(expression_dispatch.plan.dispatch_arguments, "--selection-context", "report_expression") &&
               has_argument_pair(expression_dispatch.plan.dispatch_arguments, "--path", "reports/orders.frx") &&
               has_argument_pair(expression_dispatch.plan.dispatch_arguments, "--record", "2") &&
               has_argument_pair(expression_dispatch.plan.dispatch_arguments, "--symbol", "Expr1.Expression"),
           "#1225: editor action dispatch should support admitted expression editor invocations");

    auto missing_dispatch_command_plan = admitted_method_invocation.plan;
    missing_dispatch_command_plan.command_token = {};
    const auto missing_dispatch_command = copperfin::studio::plan_studio_editor_action_dispatch({
        .admission_plan = missing_dispatch_command_plan
    });
    expect(!missing_dispatch_command.ok &&
               missing_dispatch_command.error == "An editor action dispatch request requires a command token.",
           "#1225: editor action dispatch should reject admitted plans without command tokens");

    auto missing_dispatch_action_plan = admitted_method_invocation.plan;
    missing_dispatch_action_plan.action = {};
    const auto missing_dispatch_action = copperfin::studio::plan_studio_editor_action_dispatch({
        .admission_plan = missing_dispatch_action_plan
    });
    expect(!missing_dispatch_action.ok &&
               missing_dispatch_action.error == "An editor action dispatch request requires a validated action id.",
           "#1225: editor action dispatch should reject admitted plans without action ids");

    const auto admitted_visual_dispatch_catalog =
        copperfin::studio::plan_studio_editor_action_dispatch_catalog({
            .selection_context = StudioEditorSelectionContext::visual_object,
            .asset_path = "forms/customer.scx",
            .record_index = 1U,
            .object_name = "cmdSave",
            .unique_id = "button-guid",
            .symbol = "cmdSave.Click",
            .line = 42U,
            .column = 7U,
            .admit_editor_invocations = true
        });
    expect(admitted_visual_dispatch_catalog.ok &&
               admitted_visual_dispatch_catalog.selection_context == StudioEditorSelectionContext::visual_object &&
               admitted_visual_dispatch_catalog.action_count == visual_actions.size() &&
               admitted_visual_dispatch_catalog.dispatch_count == visual_actions.size() &&
               admitted_visual_dispatch_catalog.error_count == 0U &&
               !admitted_visual_dispatch_catalog.dry_run &&
               !admitted_visual_dispatch_catalog.mutates_asset,
           "#1227: admitted editor action dispatch catalogs should dispatch every context action without mutation");
    const auto* visual_method_dispatch = find_dispatch_catalog_entry(
        admitted_visual_dispatch_catalog.entries, "edit-visual-method");
    expect(visual_method_dispatch != nullptr &&
               visual_method_dispatch->launch_plan.ok &&
               visual_method_dispatch->invocation_admission.ok &&
               visual_method_dispatch->dispatch.ok &&
               std::string(visual_method_dispatch->dispatch.plan.action.id) == "edit-visual-method" &&
               visual_method_dispatch->dispatch.plan.action.kind == StudioEditorActionKind::source_editor &&
               visual_method_dispatch->dispatch.plan.selection_context == StudioEditorSelectionContext::visual_object &&
               visual_method_dispatch->dispatch.plan.asset_path == "forms/customer.scx" &&
               visual_method_dispatch->dispatch.plan.record_index == 1U &&
               visual_method_dispatch->dispatch.plan.object_name == "cmdSave" &&
               visual_method_dispatch->dispatch.plan.unique_id == "button-guid" &&
               visual_method_dispatch->dispatch.plan.symbol == "cmdSave.Click" &&
               visual_method_dispatch->dispatch.plan.line == 42U &&
               visual_method_dispatch->dispatch.plan.column == 7U &&
               has_argument_pair(
                   visual_method_dispatch->dispatch.plan.dispatch_arguments,
                   "--selection-context",
                   "visual_object") &&
               has_argument_pair(
                   visual_method_dispatch->dispatch.plan.dispatch_arguments,
                   "--action-id",
                   "edit-visual-method"),
           "#1227: editor action dispatch catalog entries should preserve action and target metadata");
    const auto* visual_property_dispatch = find_dispatch_catalog_entry(
        admitted_visual_dispatch_catalog.entries, "show-property-grid");
    expect(visual_property_dispatch != nullptr &&
               visual_property_dispatch->dispatch.ok &&
               visual_property_dispatch->dispatch.plan.command_token == "studio.property_grid.show" &&
               visual_property_dispatch->dispatch.plan.dispatch_admitted &&
               !visual_property_dispatch->dispatch.plan.executed,
           "#1227: editor action dispatch catalogs should include property-grid dispatch contracts");

    const auto dry_run_visual_dispatch_catalog =
        copperfin::studio::plan_studio_editor_action_dispatch_catalog({
            .selection_context = StudioEditorSelectionContext::visual_object,
            .asset_path = "forms/customer.scx",
            .record_index = 1U,
            .object_name = "cmdSave",
            .unique_id = "button-guid",
            .symbol = "cmdSave.Click",
            .line = 42U,
            .column = 7U,
            .admit_editor_invocations = false
        });
    expect(dry_run_visual_dispatch_catalog.ok &&
               dry_run_visual_dispatch_catalog.action_count == visual_actions.size() &&
               dry_run_visual_dispatch_catalog.dispatch_count == 0U &&
               dry_run_visual_dispatch_catalog.error_count == visual_actions.size() &&
               dry_run_visual_dispatch_catalog.dry_run &&
               !dry_run_visual_dispatch_catalog.mutates_asset,
           "#1227: dry-run editor action dispatch catalogs should report per-action dispatch rejections");
    const auto* dry_run_property_dispatch = find_dispatch_catalog_entry(
        dry_run_visual_dispatch_catalog.entries, "show-property-grid");
    expect(dry_run_property_dispatch != nullptr &&
               dry_run_property_dispatch->launch_plan.ok &&
               dry_run_property_dispatch->invocation_admission.ok &&
               !dry_run_property_dispatch->invocation_admission.plan.editor_invocation_admitted &&
               !dry_run_property_dispatch->dispatch.ok &&
               dry_run_property_dispatch->dispatch.error ==
                   "An editor action dispatch request requires an admitted non-dry-run invocation.",
           "#1227: dry-run editor action dispatch catalog entries should preserve admission failures");

    const auto report_dispatch_catalog =
        copperfin::studio::plan_studio_editor_action_dispatch_catalog({
            .selection_context = StudioEditorSelectionContext::report_expression,
            .asset_path = "reports/orders.frx",
            .record_index = 2U,
            .object_name = "Expr1",
            .unique_id = "expr-guid",
            .symbol = "Expr1.Expression",
            .line = 3U,
            .column = 11U,
            .admit_editor_invocations = true
        });
    const auto report_dispatch_actions = copperfin::studio::studio_editor_actions_for_context(
        StudioEditorSelectionContext::report_expression);
    const auto* report_expression_dispatch = find_dispatch_catalog_entry(
        report_dispatch_catalog.entries, "edit-report-expression");
    expect(report_dispatch_catalog.ok &&
               report_dispatch_catalog.action_count == report_dispatch_actions.size() &&
               report_dispatch_catalog.dispatch_count == report_dispatch_actions.size() &&
               report_dispatch_catalog.error_count == 0U &&
               report_expression_dispatch != nullptr &&
               report_expression_dispatch->dispatch.ok &&
               report_expression_dispatch->dispatch.plan.action.kind == StudioEditorActionKind::expression_editor &&
               report_expression_dispatch->dispatch.plan.command_token == "studio.expression_editor.open" &&
               report_expression_dispatch->dispatch.plan.target_surface == "expression-editor" &&
               report_expression_dispatch->dispatch.plan.asset_path == "reports/orders.frx" &&
               report_expression_dispatch->dispatch.plan.record_index == 2U &&
               report_expression_dispatch->dispatch.plan.symbol == "Expr1.Expression" &&
               has_argument_pair(
                   report_expression_dispatch->dispatch.plan.dispatch_arguments,
                   "--selection-context",
                   "report_expression"),
           "#1227: report-expression dispatch catalogs should include expression-editor dispatch metadata");

    const auto data_plan = copperfin::studio::plan_studio_editor_action_launch({
        .selection_context = StudioEditorSelectionContext::data_environment,
        .action_id = "edit-data-environment",
        .asset_path = "forms/customer.scx",
        .record_index = 0U,
        .object_name = "Dataenvironment",
        .unique_id = "de-guid",
        .symbol = "Dataenvironment.OpenTables",
        .line = 0U,
        .column = 0U
    });
    expect(data_plan.ok &&
               data_plan.plan.command_token == "studio.data_environment.open" &&
               data_plan.plan.target_surface == "data-environment",
           "#1207: data-environment editor action launch plans should accept data-environment actions");

    const auto project_plan = copperfin::studio::plan_studio_editor_action_launch({
        .selection_context = StudioEditorSelectionContext::project_item,
        .action_id = "navigate-project-item",
        .asset_path = "apps/customer.pjx",
        .record_index = 5U,
        .object_name = {},
        .unique_id = {},
        .symbol = "forms/customer.scx",
        .line = 0U,
        .column = 0U
    });
    expect(project_plan.ok &&
               project_plan.plan.action.kind == StudioEditorActionKind::navigator &&
               project_plan.plan.command_token == "studio.project_item.navigate",
           "#1207: project-item editor action launch plans should accept navigation actions");

    const auto wrong_context_plan = copperfin::studio::plan_studio_editor_action_launch({
        .selection_context = StudioEditorSelectionContext::visual_object,
        .action_id = "edit-report-expression",
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "txtName",
        .unique_id = "textbox-guid",
        .symbol = "txtName",
        .line = 0U,
        .column = 0U
    });
    expect(!wrong_context_plan.ok,
           "#1207: editor action launch plans should reject wrong-context action ids");

    const auto missing_action_plan = copperfin::studio::plan_studio_editor_action_launch({
        .selection_context = StudioEditorSelectionContext::visual_object,
        .action_id = {},
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "txtName",
        .unique_id = "textbox-guid",
        .symbol = "txtName",
        .line = 0U,
        .column = 0U
    });
    expect(!missing_action_plan.ok,
           "#1207: editor action launch plans should reject missing action ids");

    const auto unknown_action_plan = copperfin::studio::plan_studio_editor_action_launch({
        .selection_context = StudioEditorSelectionContext::visual_object,
        .action_id = "unknown-action",
        .asset_path = "forms/customer.scx",
        .record_index = 1U,
        .object_name = "txtName",
        .unique_id = "textbox-guid",
        .symbol = "txtName",
        .line = 0U,
        .column = 0U
    });
    expect(!unknown_action_plan.ok,
           "#1207: editor action launch plans should reject unknown action ids");

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
