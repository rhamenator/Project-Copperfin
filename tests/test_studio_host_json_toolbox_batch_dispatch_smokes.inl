// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

// Batch toolbox dispatch and selection-dispatch coverage.
void test_studio_host_json_plans_toolbox_object_creation_batch_dispatch(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_batch_dispatch_plan_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto dispatch_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-plan",
            "--toolbox-context", "form",
            "--toolbox-item", "textbox",
            "--unique-id", "first-dispatch-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=First Dispatch",
            "--toolbox-item", "commandbutton",
            "--object-name", "cmdDispatch",
            "--unique-id", "dispatch-command-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Dispatch Command",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(dispatch_process.exit_code == 0,
        "#1252: toolbox-create-batch-dispatch-plan JSON command should exit successfully");
    expect_contains(dispatch_process.stdout_text, "\"toolboxCreateBatchDispatchPlan\": {",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose a dispatch result object");
    expect_contains(dispatch_process.stdout_text, "\"toolboxContextProvided\": true",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose requested context state");
    expect_contains(dispatch_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose requested contexts");
    expect_contains(dispatch_process.stdout_text, "\"itemCount\": 2",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose item counts");
    expect_contains(dispatch_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose first toolbox item metadata");
    expect_contains(dispatch_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose later toolbox item metadata");
    expect_contains(dispatch_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose first target indexes");
    expect_contains(dispatch_process.stdout_text, "\"targetRecordIndex\": 3",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose later target indexes");
    expect_contains(dispatch_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose generated names");
    expect_contains(dispatch_process.stdout_text, "\"objectName\": \"cmdDispatch\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should preserve explicit names");
    expect_contains(dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(dispatch_process.stdout_text, "\"--toolbox-create-batch\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should dispatch to toolbox-create-batch");
    expect_contains(dispatch_process.stdout_text, "\"--toolbox-item\", \"textbox\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should include first toolbox item arguments");
    expect_contains(dispatch_process.stdout_text, "\"--object-name\", \"txt2\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should include generated object-name arguments");
    expect_contains(dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=First Dispatch\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should include first field-value arguments");
    expect_contains(dispatch_process.stdout_text, "\"--toolbox-item\", \"commandbutton\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should include later toolbox item arguments");
    expect_contains(dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Dispatch Command\"",
        "#1252: toolbox-create-batch-dispatch-plan JSON should include later field-value arguments");
    expect_contains(dispatch_process.stdout_text, "\"dispatchReadyItemIds\": [\"textbox\", \"commandbutton\"]",
        "#1387: toolbox-create-batch-dispatch-plan JSON should summarize dispatch-ready item ids");
    expect_contains(dispatch_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1387: toolbox-create-batch-dispatch-plan JSON should summarize empty blocked item ids");
    expect_contains(dispatch_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1387: successful toolbox-create-batch-dispatch-plan JSON should summarize empty dispatch errors");
    expect_contains(dispatch_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose dispatch admission state");
    expect_contains(dispatch_process.stdout_text, "\"dryRun\": false",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose non-dry-run dispatch state");
    expect_contains(dispatch_process.stdout_text, "\"executed\": false",
        "#1252: toolbox-create-batch-dispatch-plan JSON should remain non-executing");
    expect_contains(dispatch_process.stdout_text, "\"mutatesAsset\": true",
        "#1252: toolbox-create-batch-dispatch-plan JSON should expose mutation intent");
    expect(visual_object_count(form_path) == before_count,
        "#1252: toolbox-create-batch-dispatch-plan host command should not mutate the visual asset");

    const auto report_dispatch_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-plan",
            "--toolbox-context", "report",
            "--toolbox-item", "label",
            "--unique-id", "direct-report-batch-dispatch-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Direct Report Dispatch",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(report_dispatch_process.exit_code == 0,
        "#2100: report toolbox-create-batch-dispatch-plan JSON command should exit successfully");
    expect_contains(report_dispatch_process.stdout_text, "\"toolboxCreateBatchDispatchPlan\": {",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should expose dispatch plans");
    expect_contains(report_dispatch_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should preserve report contexts");
    expect_contains(report_dispatch_process.stdout_text, "\"itemCount\": 1",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should expose report item counts");
    expect_contains(report_dispatch_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should expose label plans");
    expect_contains(report_dispatch_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should expose generated label names");
    expect_contains(report_dispatch_process.stdout_text, "\"uniqueId\": \"direct-report-batch-dispatch-guid\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should preserve label unique ids");
    expect_contains(report_dispatch_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should preserve label parent overrides");
    expect_contains(report_dispatch_process.stdout_text, "\"propertyValue\": \"Direct Report Dispatch\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should preserve label field values");
    expect_contains(report_dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--toolbox-create-batch\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should dispatch to toolbox-create-batch");
    expect_contains(report_dispatch_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should preserve report context arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--toolbox-item\", \"label\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should preserve label item arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--object-name\", \"lbl1\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should preserve generated label arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--unique-id\", \"direct-report-batch-dispatch-guid\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should preserve label unique-id arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--parent-name\", \"DetailBand\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should preserve label parent arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Direct Report Dispatch\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should preserve label field arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"]",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should summarize dispatch-ready report item ids");
    expect_contains(report_dispatch_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should summarize empty blocked item ids");
    expect_contains(report_dispatch_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should summarize empty dispatch errors");
    expect_contains(report_dispatch_process.stdout_text, "\"executed\": false",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should remain non-executing");
    expect_not_contains(report_dispatch_process.stdout_text, "\"className\": \"TextBox\"",
        "#2100: report toolbox-create-batch-dispatch-plan JSON should exclude form-only TextBox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2100: report toolbox-create-batch-dispatch-plan host command should not mutate assets");

    const auto non_admitted_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-plan",
            "--toolbox-item", "textbox",
            "--admit-create-operation", "false",
            "--json"
        },
        temp_root);
    expect(non_admitted_process.exit_code == 4,
        "#1252: toolbox-create-batch-dispatch-plan JSON should reject non-admitted create operations");
    expect_contains(non_admitted_process.stdout_text, "\"toolboxCreateBatchDispatchPlan\": null",
        "#1252: non-admitted toolbox-create-batch-dispatch-plan JSON should not expose stale dispatch plans");
    expect_contains(non_admitted_process.stdout_text,
        "A toolbox batch create dispatch request requires an admitted non-dry-run create operation.",
        "#1252: non-admitted toolbox-create-batch-dispatch-plan JSON should report dispatch errors");
    expect_contains(non_admitted_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1387: non-admitted toolbox-create-batch-dispatch-plan JSON should summarize empty ready item ids");
    expect_contains(non_admitted_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1387: non-admitted toolbox-create-batch-dispatch-plan JSON should summarize aggregate blocked state");
    expect_contains(non_admitted_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"A toolbox batch create dispatch request requires an admitted non-dry-run create operation.\"",
        "#1387: non-admitted toolbox-create-batch-dispatch-plan JSON should summarize dispatch errors");
    expect_not_contains(non_admitted_process.stdout_text, "\"dispatchArguments\": [",
        "#1252: failed toolbox-create-batch-dispatch-plan JSON should not expose stale dispatch arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1252: non-admitted toolbox-create-batch-dispatch-plan commands should not mutate the asset");

    const auto invalid_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-plan",
            "--toolbox-context", "report",
            "--toolbox-item", "textbox",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(invalid_plan_process.exit_code == 4,
        "#1252: toolbox-create-batch-dispatch-plan JSON should reject invalid batch plans");
    expect_contains(invalid_plan_process.stdout_text,
        "The requested toolbox item is not available in the requested designer context.",
        "#1252: invalid toolbox-create-batch-dispatch-plan batch plans should report planning errors");
    expect_contains(invalid_plan_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1387: invalid toolbox-create-batch-dispatch-plan JSON should summarize empty ready item ids");
    expect_contains(invalid_plan_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"The requested toolbox item is not available in the requested designer context.\"",
        "#1387: invalid toolbox-create-batch-dispatch-plan JSON should summarize planning errors");
    expect_not_contains(invalid_plan_process.stdout_text, "\"dispatchArguments\": [",
        "#1252: invalid toolbox-create-batch-dispatch-plan batch plans should not expose stale arguments");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-plan",
            "--toolbox-item", "textbox",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1252: toolbox-create-batch-dispatch-plan JSON should reject invalid admission tokens");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#1252: invalid toolbox-create-batch-dispatch-plan admission tokens should report parser errors");

    const auto orphan_item_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-plan",
            "--parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(orphan_item_option_process.exit_code == 2,
        "#1252: toolbox-create-batch-dispatch-plan JSON should reject item options before items");
    expect_contains(orphan_item_option_process.stdout_text,
        "Toolbox batch item options require a preceding --toolbox-item.",
        "#1252: orphan toolbox-create-batch-dispatch-plan item options should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
void test_studio_host_json_plans_selection_toolbox_object_creation_batch_dispatch(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_create_batch_dispatch_plan_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto dispatch_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--unique-id", "selection-dispatch-first-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=First Selection Dispatch",
            "--toolbox-item", "commandbutton",
            "--object-name", "cmdSelectionDispatch",
            "--unique-id", "selection-dispatch-command-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Run Selection Dispatch",
            "--toolbox-item", "textbox",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Second Selection Dispatch",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(dispatch_process.exit_code == 0,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON command should exit successfully");
    expect_contains(dispatch_process.stdout_text, "\"selectionToolboxCreateBatchDispatchPlan\": {",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose a stable result object");
    expect_contains(dispatch_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose selected contexts");
    expect_contains(dispatch_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose resolved toolbox contexts");
    expect_contains(dispatch_process.stdout_text, "\"launchPlanOk\": true",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose launch state");
    expect_contains(dispatch_process.stdout_text, "\"itemCount\": 3",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose item counts");
    expect_contains(dispatch_process.stdout_text, "\"dispatchCount\": 1",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose dispatch counts");
    expect_contains(dispatch_process.stdout_text, "\"errorCount\": 0",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose zero errors");
    expect_contains(dispatch_process.stdout_text, "\"batchPlanOk\": true",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose nested batch-plan state");
    expect_contains(dispatch_process.stdout_text, "\"dispatchOk\": true",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose nested dispatch state");
    expect_contains(dispatch_process.stdout_text, "\"batchPlan\": {",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose nested batch plans");
    expect_contains(dispatch_process.stdout_text, "\"dispatch\": {",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose nested dispatch plans");
    expect_contains(dispatch_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose textbox descriptors");
    expect_contains(dispatch_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose command button descriptors");
    expect_contains(dispatch_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose first target records");
    expect_contains(dispatch_process.stdout_text, "\"targetRecordIndex\": 4",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose later target records");
    expect_contains(dispatch_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose first generated names");
    expect_contains(dispatch_process.stdout_text, "\"objectName\": \"txt3\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reserve generated names across the batch");
    expect_contains(dispatch_process.stdout_text, "\"objectName\": \"cmdSelectionDispatch\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should preserve explicit object names");
    expect_contains(dispatch_process.stdout_text, "\"uniqueId\": \"selection-dispatch-first-textbox-guid\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose per-item unique ids");
    expect_contains(dispatch_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose per-item parent names");
    expect_contains(dispatch_process.stdout_text, "\"propertyValue\": \"Second Selection Dispatch\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should preserve per-item field values");
    expect_contains(dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(dispatch_process.stdout_text, "\"--toolbox-create-batch\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should dispatch to batch creation");
    expect_contains(dispatch_process.stdout_text, "\"--toolbox-context\", \"form\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should include resolved contexts");
    expect_contains(dispatch_process.stdout_text, "\"--object-name\", \"txt2\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should include generated object-name args");
    expect_contains(dispatch_process.stdout_text, "\"--unique-id\", \"selection-dispatch-first-textbox-guid\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should include unique id args");
    expect_contains(dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Run Selection Dispatch\"",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should include field-value args");
    expect_contains(dispatch_process.stdout_text,
        "\"dispatchReadyItemIds\": [\"textbox\", \"commandbutton\", \"textbox\"]",
        "#1387: selection-toolbox-create-batch-dispatch-plan JSON should summarize dispatch-ready item ids");
    expect_contains(dispatch_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1387: selection-toolbox-create-batch-dispatch-plan JSON should summarize empty blocked item ids");
    expect_contains(dispatch_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1387: successful selection-toolbox-create-batch-dispatch-plan JSON should summarize empty dispatch errors");
    expect_contains(dispatch_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose admission state");
    expect_contains(dispatch_process.stdout_text, "\"dryRun\": false",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose non-dry-run dispatch state");
    expect_contains(dispatch_process.stdout_text, "\"executed\": false",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should remain non-executing");
    expect_contains(dispatch_process.stdout_text, "\"mutatesAsset\": true",
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should expose mutation intent");
    expect(visual_object_count(form_path) == before_count,
        "#1307: selection-toolbox-create-batch-dispatch-plan host command should not mutate assets");

    const auto non_admitted_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--admit-create-operation", "false",
            "--json"
        },
        temp_root);
    expect(non_admitted_process.exit_code == 4,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reject non-admitted operations");
    expect_contains(non_admitted_process.stdout_text, "\"batchPlanOk\": true",
        "#1307: non-admitted selection-toolbox-create-batch-dispatch-plan JSON should keep batch-plan evidence");
    expect_contains(non_admitted_process.stdout_text, "\"dispatchOk\": false",
        "#1307: non-admitted selection-toolbox-create-batch-dispatch-plan JSON should expose dispatch failures");
    expect_contains(non_admitted_process.stdout_text,
        "A toolbox batch create dispatch request requires an admitted non-dry-run create operation.",
        "#1307: non-admitted selection-toolbox-create-batch-dispatch-plan JSON should report dispatch errors");
    expect_contains(non_admitted_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1387: non-admitted selection-toolbox-create-batch-dispatch-plan JSON should summarize empty ready item ids");
    expect_contains(non_admitted_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1387: non-admitted selection-toolbox-create-batch-dispatch-plan JSON should summarize aggregate blocked state");
    expect_contains(non_admitted_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"A toolbox batch create dispatch request requires an admitted non-dry-run create operation.\"",
        "#1387: non-admitted selection-toolbox-create-batch-dispatch-plan JSON should summarize dispatch errors");
    expect_contains(non_admitted_process.stdout_text, "\"dispatch\": null",
        "#1307: non-admitted selection-toolbox-create-batch-dispatch-plan JSON should omit stale dispatch plans");
    expect_not_contains(non_admitted_process.stdout_text, "\"dispatchArguments\": [",
        "#1307: non-admitted selection-toolbox-create-batch-dispatch-plan JSON should omit stale arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1307: non-admitted selection-toolbox-create-batch-dispatch-plan commands should not mutate assets");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "report_expression",
            "--toolbox-item", "label",
            "--unique-id", "selection-report-dispatch-label-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Selection Dispatch",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#1307: report selection-toolbox-create-batch-dispatch-plan JSON command should exit successfully");
    expect_contains(report_process.stdout_text, "\"selectionToolboxCreateBatchDispatchPlan\": {",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose a stable result object");
    expect_contains(report_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1307: report selection-toolbox-create-batch-dispatch-plan JSON should expose report selections");
    expect_contains(report_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1307: report selection-toolbox-create-batch-dispatch-plan JSON should resolve report contexts");
    expect_contains(report_process.stdout_text, "\"launchPlanOk\": true",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose launch state");
    expect_contains(report_process.stdout_text, "\"itemCount\": 1",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose report item counts");
    expect_contains(report_process.stdout_text, "\"dispatchCount\": 1",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose dispatch counts");
    expect_contains(report_process.stdout_text, "\"errorCount\": 0",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose zero errors");
    expect_contains(report_process.stdout_text, "\"batchPlanOk\": true",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose batch-plan state");
    expect_contains(report_process.stdout_text, "\"dispatchOk\": true",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose dispatch state");
    expect_contains(report_process.stdout_text, "\"batchPlan\": {",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose nested batch plans");
    expect_contains(report_process.stdout_text, "\"dispatch\": {",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose nested dispatch plans");
    expect_contains(report_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1307: report selection-toolbox-create-batch-dispatch-plan JSON should expose label plans");
    expect_contains(report_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#1307: report selection-toolbox-create-batch-dispatch-plan JSON should expose generated label names");
    expect_contains(report_process.stdout_text, "\"uniqueId\": \"selection-report-dispatch-label-guid\"",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose caller unique ids");
    expect_contains(report_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should preserve report parent payloads");
    expect_contains(report_process.stdout_text, "\"propertyValue\": \"Report Selection Dispatch\"",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose caller report fields");
    expect_contains(report_process.stdout_text, "\"dispatchArguments\": [",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(report_process.stdout_text, "\"--toolbox-create-batch\"",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should dispatch to batch creation");
    expect_contains(report_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#1307: report selection-toolbox-create-batch-dispatch-plan JSON should dispatch resolved report contexts");
    expect_contains(report_process.stdout_text, "\"--parent-name\", \"DetailBand\"",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should preserve report parent arguments");
    expect_contains(report_process.stdout_text, "\"--field-value\", \"CAPTION=Report Selection Dispatch\"",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should preserve report field arguments");
    expect_contains(report_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"]",
        "#1387: report selection-toolbox-create-batch-dispatch-plan JSON should summarize dispatch-ready report item ids");
    expect_contains(report_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should summarize empty blocked item ids");
    expect_contains(report_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1387: report selection-toolbox-create-batch-dispatch-plan JSON should summarize empty dispatch errors");
    expect_contains(report_process.stdout_text, "\"dispatchAdmitted\": true",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose admission state");
    expect_contains(report_process.stdout_text, "\"dryRun\": false",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose non-dry-run dispatch state");
    expect_contains(report_process.stdout_text, "\"executed\": false",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should remain non-executing");
    expect_contains(report_process.stdout_text, "\"mutatesAsset\": true",
        "#2118: report selection-toolbox-create-batch-dispatch-plan JSON should expose mutation intent");
    expect_not_contains(report_process.stdout_text, "\"className\": \"TextBox\"",
        "#1307: report selection-toolbox-create-batch-dispatch-plan JSON should exclude form-only textboxes");
    expect(visual_object_count(form_path) == before_count,
        "#2118: report selection-toolbox-create-batch-dispatch-plan host command should not mutate assets");

    const auto label_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "label_expression",
            "--toolbox-item", "label",
            "--unique-id", "selection-batch-dispatch-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Label Selection Dispatch",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(label_process.exit_code == 0,
        "#2086: label selection-toolbox-create-batch-dispatch-plan JSON command should exit successfully");
    expect_contains(label_process.stdout_text, "\"selectionToolboxCreateBatchDispatchPlan\": {",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose a stable result object");
    expect_contains(label_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#2086: label selection-toolbox-create-batch-dispatch-plan JSON should expose label selections");
    expect_contains(label_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2086: label selection-toolbox-create-batch-dispatch-plan JSON should resolve report contexts");
    expect_contains(label_process.stdout_text, "\"launchPlanOk\": true",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose launch state");
    expect_contains(label_process.stdout_text, "\"itemCount\": 1",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose label item counts");
    expect_contains(label_process.stdout_text, "\"dispatchCount\": 1",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose dispatch counts");
    expect_contains(label_process.stdout_text, "\"errorCount\": 0",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose zero errors");
    expect_contains(label_process.stdout_text, "\"batchPlanOk\": true",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose batch-plan state");
    expect_contains(label_process.stdout_text, "\"dispatchOk\": true",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose dispatch state");
    expect_contains(label_process.stdout_text, "\"batchPlan\": {",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose nested batch plans");
    expect_contains(label_process.stdout_text, "\"dispatch\": {",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose nested dispatch plans");
    expect_contains(label_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2086: label selection-toolbox-create-batch-dispatch-plan JSON should expose label plans");
    expect_contains(label_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2086: label selection-toolbox-create-batch-dispatch-plan JSON should expose generated label names");
    expect_contains(label_process.stdout_text, "\"uniqueId\": \"selection-batch-dispatch-guid\"",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose caller unique ids");
    expect_contains(label_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should preserve label parent payloads");
    expect_contains(label_process.stdout_text, "\"propertyValue\": \"Label Selection Dispatch\"",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose caller label fields");
    expect_contains(label_process.stdout_text, "\"dispatchArguments\": [",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(label_process.stdout_text, "\"--toolbox-create-batch\"",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should dispatch to batch creation");
    expect_contains(label_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#2086: label selection-toolbox-create-batch-dispatch-plan JSON should dispatch resolved report contexts");
    expect_contains(label_process.stdout_text, "\"--parent-name\", \"DetailBand\"",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should preserve label parent arguments");
    expect_contains(label_process.stdout_text, "\"--field-value\", \"CAPTION=Label Selection Dispatch\"",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should preserve label field arguments");
    expect_contains(label_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"]",
        "#2086: label selection-toolbox-create-batch-dispatch-plan JSON should summarize dispatch-ready label item ids");
    expect_contains(label_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should summarize empty blocked item ids");
    expect_contains(label_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#2086: label selection-toolbox-create-batch-dispatch-plan JSON should summarize empty dispatch errors");
    expect_contains(label_process.stdout_text, "\"dispatchAdmitted\": true",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose admission state");
    expect_contains(label_process.stdout_text, "\"dryRun\": false",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose non-dry-run dispatch state");
    expect_contains(label_process.stdout_text, "\"executed\": false",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should remain non-executing");
    expect_contains(label_process.stdout_text, "\"mutatesAsset\": true",
        "#2124: label selection-toolbox-create-batch-dispatch-plan JSON should expose mutation intent");
    expect_not_contains(label_process.stdout_text, "\"className\": \"TextBox\"",
        "#2086: label selection-toolbox-create-batch-dispatch-plan JSON should exclude form-only textboxes");
    expect(visual_object_count(form_path) == before_count,
        "#2124: label selection-toolbox-create-batch-dispatch-plan host command should not mutate assets");

    const auto unavailable_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "report_expression",
            "--toolbox-item", "textbox",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(unavailable_process.exit_code == 4,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reject unavailable selected-context items");
    expect_contains(unavailable_process.stdout_text, "\"batchPlanOk\": false",
        "#1307: unavailable selection-toolbox-create-batch-dispatch-plan JSON should expose failed batch plans");
    expect_contains(unavailable_process.stdout_text, "\"dispatchOk\": false",
        "#1307: unavailable selection-toolbox-create-batch-dispatch-plan JSON should expose no dispatch");
    expect_contains(unavailable_process.stdout_text,
        "The requested toolbox item is not available in the requested designer context.",
        "#1307: unavailable selection-toolbox-create-batch-dispatch-plan JSON should report planner errors");
    expect_contains(unavailable_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1387: unavailable selection-toolbox-create-batch-dispatch-plan JSON should summarize empty ready item ids");
    expect_contains(unavailable_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"The requested toolbox item is not available in the requested designer context.\"",
        "#1387: unavailable selection-toolbox-create-batch-dispatch-plan JSON should summarize planner errors");
    expect_not_contains(unavailable_process.stdout_text, "\"dispatchArguments\": [",
        "#1307: unavailable selection-toolbox-create-batch-dispatch-plan JSON should omit stale dispatch args");

    const auto unsupported_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "menu_item",
            "--toolbox-item", "textbox",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(unsupported_process.exit_code == 4,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reject unsupported selections");
    expect_contains(unsupported_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1307: unsupported selection-toolbox-create-batch-dispatch-plan JSON should preserve selections");
    expect_contains(unsupported_process.stdout_text, "\"launchPlanOk\": false",
        "#1307: unsupported selection-toolbox-create-batch-dispatch-plan JSON should expose launch failures");
    expect_contains(unsupported_process.stdout_text,
        "A selection-context toolbox object batch creation plan request requires a toolbox palette.",
        "#1307: unsupported selection-toolbox-create-batch-dispatch-plan JSON should report palette errors");
    expect_not_contains(unsupported_process.stdout_text, "\"dispatchArguments\": [",
        "#1307: unsupported selection-toolbox-create-batch-dispatch-plan JSON should omit stale dispatch args");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reject missing paths");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1307: missing path selection-toolbox-create-batch-dispatch-plan JSON should report parser errors");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--toolbox-item", "textbox",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1307: missing selection selection-toolbox-create-batch-dispatch-plan JSON should report parser errors");

    const auto missing_items_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(missing_items_process.exit_code == 2,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reject empty item lists");
    expect_contains(missing_items_process.stdout_text, "No toolbox item ids were provided.",
        "#1307: empty selection-toolbox-create-batch-dispatch-plan item lists should report parser errors");

    const auto orphan_item_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "visual_object",
            "--parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(orphan_item_option_process.exit_code == 2,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reject item options before items");
    expect_contains(orphan_item_option_process.stdout_text,
        "Selection toolbox batch dispatch item options require a preceding --toolbox-item.",
        "#1307: orphan selection-toolbox-create-batch-dispatch-plan item options should report parser errors");

    const auto malformed_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(malformed_field_process.exit_code == 2,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reject malformed field values");
    expect_contains(malformed_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1307: malformed selection-toolbox-create-batch-dispatch-plan field values should report parser errors");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reject invalid admission booleans");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#1307: invalid selection-toolbox-create-batch-dispatch-plan admission values should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-plan",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1307: selection-toolbox-create-batch-dispatch-plan JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-toolbox-create-batch-dispatch-plan option: --toolbox-context",
        "#1307: unknown option selection-toolbox-create-batch-dispatch-plan JSON should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1307: rejected selection-toolbox-create-batch-dispatch-plan host commands should not mutate assets");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
