void test_toolbox_palette_invocation_and_dispatch_contracts() {
    using copperfin::studio::StudioEditorSelectionContext;

    const auto unsupported_toolbox_launch = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::menu_item,
        .asset_path = "menus/main.mnx",
        .record_index = 5U,
        .object_name = "FileExit",
        .unique_id = "menu-guid"
    });
    expect(!unsupported_toolbox_launch.ok &&
               unsupported_toolbox_launch.error ==
                   "The selected Studio context does not expose a toolbox palette.",
           "#2364: toolbox palette launch errors should resolve through the en-US localization catalog");
    const auto report_toolbox_launch = copperfin::studio::plan_studio_toolbox_palette_launch({
        .selection_context = StudioEditorSelectionContext::report_expression,
        .asset_path = "reports/orders.frx",
        .record_index = 2U,
        .object_name = "Expr1",
        .unique_id = "expr-guid"
    });
    const auto admitted_toolbox_invocation = copperfin::studio::plan_studio_toolbox_invocation_admission({
        .launch_plan = report_toolbox_launch.plan,
        .admit_palette_invocation = true
    });
    expect(report_toolbox_launch.ok &&
               admitted_toolbox_invocation.ok &&
               admitted_toolbox_invocation.plan.command_token == "studio.toolbox.palette.invoke" &&
               admitted_toolbox_invocation.plan.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
               admitted_toolbox_invocation.plan.item_count == 4U &&
               admitted_toolbox_invocation.plan.palette_invocation_admitted &&
               !admitted_toolbox_invocation.plan.dry_run &&
               !admitted_toolbox_invocation.plan.mutates_asset &&
               find_toolbox_item(admitted_toolbox_invocation.plan.items, "label") != nullptr &&
               find_toolbox_item(admitted_toolbox_invocation.plan.items, "textbox") == nullptr,
           "#2365: toolbox invocation admission should preserve report-safe invariant metadata");
    const auto admitted_toolbox_dispatch = copperfin::studio::plan_studio_toolbox_dispatch({
        .admission_plan = admitted_toolbox_invocation.plan
    });
    expect(admitted_toolbox_dispatch.ok &&
               admitted_toolbox_dispatch.plan.command_token == "studio.toolbox.palette.invoke" &&
               admitted_toolbox_dispatch.plan.toolbox_context == copperfin::studio::StudioToolboxContext::report &&
               admitted_toolbox_dispatch.plan.item_count == 4U &&
               admitted_toolbox_dispatch.plan.dispatch_admitted &&
               !admitted_toolbox_dispatch.plan.dry_run &&
               !admitted_toolbox_dispatch.plan.executed &&
               !admitted_toolbox_dispatch.plan.mutates_asset &&
               has_argument_pair(admitted_toolbox_dispatch.plan.dispatch_arguments, "--toolbox-context", "report") &&
               has_argument_pair(admitted_toolbox_dispatch.plan.dispatch_arguments, "--item-count", "4") &&
               find_toolbox_item(admitted_toolbox_dispatch.plan.items, "label") != nullptr &&
               find_toolbox_item(admitted_toolbox_dispatch.plan.items, "textbox") == nullptr,
           "#2366: toolbox dispatch should preserve report-safe invariant metadata and argument tokens");
    const auto unadmitted_toolbox_execution = copperfin::studio::execute_studio_toolbox_dispatch({
        .dispatch_plan = admitted_toolbox_dispatch.plan,
        .admit_execution = false,
        .executor = {}
    });
    expect(!unadmitted_toolbox_execution.ok &&
               unadmitted_toolbox_execution.error ==
                   "A toolbox dispatch execution request requires explicit execution admission." &&
               !unadmitted_toolbox_execution.executed &&
               unadmitted_toolbox_execution.dry_run,
           "#2366: toolbox dispatch execution should localize unadmitted execution errors");
    const auto launch_failure_toolbox_execution = copperfin::studio::execute_studio_toolbox_dispatch({
        .dispatch_plan = admitted_toolbox_dispatch.plan,
        .admit_execution = true,
        .executor = [](const copperfin::studio::StudioToolboxDispatchPlan&) {
            return copperfin::studio::StudioToolboxDispatchExecutionObservation{
                .launched = false,
                .exit_code = 0,
                .output = {},
                .error = {},
                .mutates_asset = false
            };
        }
    });
    expect(!launch_failure_toolbox_execution.ok &&
               launch_failure_toolbox_execution.error ==
                   "A toolbox dispatch executor did not launch the toolbox dispatch." &&
               !launch_failure_toolbox_execution.executed &&
               launch_failure_toolbox_execution.dry_run,
           "#2366: toolbox dispatch execution should localize default launch-failure prose");
    const auto non_zero_toolbox_execution = copperfin::studio::execute_studio_toolbox_dispatch({
        .dispatch_plan = admitted_toolbox_dispatch.plan,
        .admit_execution = true,
        .executor = [](const copperfin::studio::StudioToolboxDispatchPlan&) {
            return copperfin::studio::StudioToolboxDispatchExecutionObservation{
                .launched = true,
                .exit_code = 7,
                .output = {},
                .error = {},
                .mutates_asset = false
            };
        }
    });
    expect(!non_zero_toolbox_execution.ok &&
               non_zero_toolbox_execution.error ==
                   "A toolbox dispatch executor returned a non-zero exit code." &&
               non_zero_toolbox_execution.observation.launched &&
               non_zero_toolbox_execution.observation.exit_code == 7 &&
               !non_zero_toolbox_execution.executed &&
               non_zero_toolbox_execution.dry_run,
           "#2366: toolbox dispatch execution should localize default non-zero-exit prose");
    const auto unadmitted_toolbox_execution_catalog =
        copperfin::studio::plan_studio_toolbox_dispatch_execution_catalog_for_selection({
            .selection_context = StudioEditorSelectionContext::report_expression,
            .asset_path = "reports/orders.frx",
            .record_index = 2U,
            .object_name = "Expr1",
            .unique_id = "expr-guid",
            .admit_palette_invocation = true,
            .admit_execution = false
        });
    expect(unadmitted_toolbox_execution_catalog.ok &&
               unadmitted_toolbox_execution_catalog.execution_ready_count == 0U &&
               unadmitted_toolbox_execution_catalog.error_count == 4U &&
               !unadmitted_toolbox_execution_catalog.entries.empty() &&
               unadmitted_toolbox_execution_catalog.entries.front().execution_error ==
                   "A toolbox dispatch execution catalog entry requires explicit execution admission.",
           "#2366: toolbox dispatch execution catalogs should localize readiness errors without changing item counts");
    const auto dry_run_toolbox_invocation = copperfin::studio::plan_studio_toolbox_invocation_admission({
        .launch_plan = report_toolbox_launch.plan,
        .admit_palette_invocation = false
    });
    const auto dry_run_toolbox_dispatch = copperfin::studio::plan_studio_toolbox_dispatch({
        .admission_plan = dry_run_toolbox_invocation.plan
    });
    expect(!dry_run_toolbox_dispatch.ok &&
               dry_run_toolbox_dispatch.error ==
                   "A toolbox dispatch request requires an admitted non-dry-run invocation.",
           "#2366: toolbox dispatch should localize dry-run invocation errors");
    auto inconsistent_toolbox_launch = report_toolbox_launch.plan;
    inconsistent_toolbox_launch.item_count = inconsistent_toolbox_launch.items.size() + 1U;
    const auto inconsistent_toolbox_invocation =
        copperfin::studio::plan_studio_toolbox_invocation_admission({
            .launch_plan = inconsistent_toolbox_launch,
            .admit_palette_invocation = true
        });
    expect(!inconsistent_toolbox_invocation.ok &&
               inconsistent_toolbox_invocation.error ==
                   "A toolbox invocation admission request requires consistent toolbox item metadata.",
           "#2365: toolbox invocation admission should localize inconsistent item metadata errors");
    const auto unsupported_selection_toolbox_invocation_catalog =
        copperfin::studio::plan_studio_toolbox_invocation_admission_catalog_for_selection({
            .selection_context = StudioEditorSelectionContext::menu_item,
            .asset_path = "menus/main.mnx",
            .record_index = 5U,
            .object_name = "FileExit",
            .unique_id = "menu-guid",
            .admit_palette_invocation = true
        });
    expect(!unsupported_selection_toolbox_invocation_catalog.ok &&
               unsupported_selection_toolbox_invocation_catalog.error ==
                   "A selection-context toolbox invocation admission catalog request requires a toolbox palette." &&
               unsupported_selection_toolbox_invocation_catalog.item_count == 0U &&
               unsupported_selection_toolbox_invocation_catalog.admission_count == 0U &&
               unsupported_selection_toolbox_invocation_catalog.error_count == 0U &&
               unsupported_selection_toolbox_invocation_catalog.dry_run &&
               !unsupported_selection_toolbox_invocation_catalog.mutates_asset,
           "#2365: selection toolbox invocation admission catalogs should localize missing-palette errors");
}
