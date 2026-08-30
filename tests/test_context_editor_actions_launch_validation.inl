void test_editor_action_launch_contexts_and_rejections() {
    using copperfin::studio::StudioEditorActionKind;
    using copperfin::studio::StudioEditorSelectionContext;

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

    const auto data_property_plan = copperfin::studio::plan_studio_editor_action_launch({
        .selection_context = StudioEditorSelectionContext::data_environment,
        .action_id = "show-property-grid",
        .asset_path = "forms/customer.scx",
        .record_index = 0U,
        .object_name = "Dataenvironment",
        .unique_id = "de-guid",
        .symbol = "Dataenvironment",
        .line = 0U,
        .column = 0U
    });
    expect(data_property_plan.ok &&
               data_property_plan.plan.action.kind == StudioEditorActionKind::property_grid &&
               data_property_plan.plan.command_token == "studio.property_grid.show" &&
               data_property_plan.plan.target_surface == "property-grid",
           "#1410: data-environment editor action launch plans should accept property-grid actions");

    const auto menu_command_plan = copperfin::studio::plan_studio_editor_action_launch({
        .selection_context = StudioEditorSelectionContext::menu_item,
        .action_id = "edit-menu-command",
        .asset_path = "menus/main.mnx",
        .record_index = 5U,
        .object_name = "FileExit",
        .unique_id = "menu-guid",
        .symbol = "FileExit.Command",
        .line = 4U,
        .column = 2U
    });
    expect(menu_command_plan.ok &&
               menu_command_plan.plan.action.kind == StudioEditorActionKind::source_editor &&
               menu_command_plan.plan.command_token == "studio.menu_command_editor.open" &&
               menu_command_plan.plan.target_surface == "menu-command-editor" &&
               menu_command_plan.plan.asset_path == "menus/main.mnx" &&
               menu_command_plan.plan.record_index == 5U &&
               menu_command_plan.plan.symbol == "FileExit.Command",
           "#1413: menu-item editor action launch plans should accept menu command editor actions");

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
    expect(
        wrong_context_plan.error ==
            "The requested editor action is not available for the selected Studio context.",
        "#2646: wrong-context editor action launch errors should resolve through the en-US localization catalog");

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
    expect(
        missing_action_plan.error == "An editor action launch request requires an action id.",
        "#2646: missing-action editor action launch errors should resolve through the en-US localization catalog");

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
    expect(
        unknown_action_plan.error ==
            "The requested editor action is not available for the selected Studio context.",
        "#2646: unknown-action editor action launch errors should resolve through the en-US localization catalog");
}
