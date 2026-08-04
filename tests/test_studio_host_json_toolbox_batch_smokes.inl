// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

// Batch toolbox creation and selection coverage.
void test_studio_host_json_plans_toolbox_object_creation_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_batch_plan_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto batch_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-plan",
            "--toolbox-context", "form",
            "--toolbox-item", "textbox",
            "--unique-id", "first-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=First",
            "--toolbox-item", "textbox",
            "--unique-id", "second-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Second",
            "--toolbox-item", "commandbutton",
            "--object-name", "cmdRun",
            "--unique-id", "command-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Run",
            "--json"
        },
        temp_root);
    expect(batch_plan_process.exit_code == 0,
        "#1246: toolbox-create-batch-plan JSON command should exit successfully");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxCreateBatchPlan\": {",
        "#1246: toolbox-create-batch-plan JSON should expose a batch plan object");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxContextProvided\": true",
        "#1246: toolbox-create-batch-plan JSON should expose requested context state");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1246: toolbox-create-batch-plan JSON should expose requested contexts");
    expect_contains(batch_plan_process.stdout_text, "\"itemCount\": 3",
        "#1246: toolbox-create-batch-plan JSON should expose batch item counts");
    expect_contains(batch_plan_process.stdout_text,
        "\"planReadyItemIds\": [\"textbox\", \"textbox\", \"commandbutton\"]",
        "#1404: toolbox-create-batch-plan JSON should summarize plan-ready item ids");
    expect_contains(batch_plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1404: toolbox-create-batch-plan JSON should expose empty blocked item ids");
    expect_contains(batch_plan_process.stdout_text, "\"planBlockedErrors\": []",
        "#1404: toolbox-create-batch-plan JSON should expose empty blocked plan errors");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1246: toolbox-create-batch-plan JSON should expose textbox descriptors");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1246: toolbox-create-batch-plan JSON should expose command button descriptors");
    expect_contains(batch_plan_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1246: toolbox-create-batch-plan JSON should expose first append target indexes");
    expect_contains(batch_plan_process.stdout_text, "\"targetRecordIndex\": 4",
        "#1246: toolbox-create-batch-plan JSON should expose later append target indexes");
    expect_contains(batch_plan_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1246: toolbox-create-batch-plan JSON should expose first generated names");
    expect_contains(batch_plan_process.stdout_text, "\"objectName\": \"txt3\"",
        "#1246: toolbox-create-batch-plan JSON should reserve generated names across the batch");
    expect_contains(batch_plan_process.stdout_text, "\"objectName\": \"cmdRun\"",
        "#1246: toolbox-create-batch-plan JSON should preserve explicit names");
    expect_contains(batch_plan_process.stdout_text, "\"uniqueId\": \"first-textbox-guid\"",
        "#1246: toolbox-create-batch-plan JSON should expose per-item unique ids");
    expect_contains(batch_plan_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1246: toolbox-create-batch-plan JSON should expose per-item parents");
    expect_contains(batch_plan_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1246: toolbox-create-batch-plan JSON should expose per-item field values");
    expect_contains(batch_plan_process.stdout_text, "\"dryRun\": true",
        "#1246: toolbox-create-batch-plan JSON should expose dry-run state");
    expect_contains(batch_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#1246: toolbox-create-batch-plan JSON should remain non-mutating");
    expect(visual_object_count(form_path) == before_count,
        "#1246: toolbox-create-batch-plan host command should not mutate the visual asset");

    const auto report_batch_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-plan",
            "--toolbox-context", "report",
            "--toolbox-item", "label",
            "--unique-id", "direct-report-batch-plan-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Direct Report Plan",
            "--json"
        },
        temp_root);
    expect(report_batch_plan_process.exit_code == 0,
        "#2101: report toolbox-create-batch-plan JSON command should exit successfully");
    expect_contains(report_batch_plan_process.stdout_text, "\"toolboxCreateBatchPlan\": {",
        "#2101: report toolbox-create-batch-plan JSON should expose batch plans");
    expect_contains(report_batch_plan_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2101: report toolbox-create-batch-plan JSON should preserve report contexts");
    expect_contains(report_batch_plan_process.stdout_text, "\"itemCount\": 1",
        "#2101: report toolbox-create-batch-plan JSON should expose report item counts");
    expect_contains(report_batch_plan_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2101: report toolbox-create-batch-plan JSON should expose label plans");
    expect_contains(report_batch_plan_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2101: report toolbox-create-batch-plan JSON should expose generated label names");
    expect_contains(report_batch_plan_process.stdout_text, "\"uniqueId\": \"direct-report-batch-plan-guid\"",
        "#2101: report toolbox-create-batch-plan JSON should preserve label unique ids");
    expect_contains(report_batch_plan_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2101: report toolbox-create-batch-plan JSON should preserve label parent overrides");
    expect_contains(report_batch_plan_process.stdout_text, "\"propertyValue\": \"Direct Report Plan\"",
        "#2101: report toolbox-create-batch-plan JSON should preserve label field values");
    expect_contains(report_batch_plan_process.stdout_text, "\"planReadyItemIds\": [\"label\"]",
        "#2101: report toolbox-create-batch-plan JSON should summarize plan-ready report item ids");
    expect_contains(report_batch_plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#2101: report toolbox-create-batch-plan JSON should summarize empty blocked item ids");
    expect_contains(report_batch_plan_process.stdout_text, "\"planBlockedErrors\": []",
        "#2101: report toolbox-create-batch-plan JSON should summarize empty plan errors");
    expect_contains(report_batch_plan_process.stdout_text, "\"dryRun\": true",
        "#2101: report toolbox-create-batch-plan JSON should remain a dry-run plan");
    expect_contains(report_batch_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#2101: report toolbox-create-batch-plan JSON should remain non-mutating");
    expect_not_contains(report_batch_plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#2101: report toolbox-create-batch-plan JSON should exclude form-only TextBox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2101: report toolbox-create-batch-plan host command should not mutate assets");

    const auto missing_items_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-plan",
            "--json"
        },
        temp_root);
    expect(missing_items_process.exit_code == 2,
        "#1246: toolbox-create-batch-plan JSON should reject empty item lists");
    expect_contains(missing_items_process.stdout_text, "No toolbox item ids were provided.",
        "#1246: empty toolbox-create-batch-plan item lists should report parser errors");

    const auto orphan_item_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-plan",
            "--parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(orphan_item_option_process.exit_code == 2,
        "#1246: toolbox-create-batch-plan JSON should reject item options before items");
    expect_contains(orphan_item_option_process.stdout_text,
        "Toolbox batch item options require a preceding --toolbox-item.",
        "#1246: orphan toolbox-create-batch-plan item options should report parser errors");

    const auto malformed_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-plan",
            "--toolbox-item", "textbox",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(malformed_field_process.exit_code == 2,
        "#1246: toolbox-create-batch-plan JSON should reject malformed field values");
    expect_contains(malformed_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1246: malformed toolbox-create-batch-plan field values should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1246: rejected toolbox-create-batch-plan host commands should not mutate the visual asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
void test_studio_host_json_plans_selection_toolbox_object_creation_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_create_batch_plan_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto batch_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--unique-id", "selection-first-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=First Selection",
            "--toolbox-item", "commandbutton",
            "--object-name", "cmdSelection",
            "--unique-id", "selection-command-guid",
            "--parent-name", "cntToolbar",
            "--field-value", "CAPTION=Run Selection",
            "--toolbox-item", "textbox",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Second Selection",
            "--json"
        },
        temp_root);
    expect(batch_plan_process.exit_code == 0,
        "#1305: selection-toolbox-create-batch-plan JSON command should exit successfully");
    expect_contains(batch_plan_process.stdout_text, "\"selectionToolboxCreateBatchPlan\": {",
        "#1305: selection-toolbox-create-batch-plan JSON should expose a stable result object");
    expect_contains(batch_plan_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1305: selection-toolbox-create-batch-plan JSON should expose selected Studio contexts");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1305: selection-toolbox-create-batch-plan JSON should expose resolved toolbox contexts");
    expect_contains(batch_plan_process.stdout_text, "\"launchPlanOk\": true",
        "#1305: selection-toolbox-create-batch-plan JSON should expose launch state");
    expect_contains(batch_plan_process.stdout_text, "\"itemCount\": 3",
        "#1305: selection-toolbox-create-batch-plan JSON should expose item counts");
    expect_contains(batch_plan_process.stdout_text, "\"planCount\": 1",
        "#1305: selection-toolbox-create-batch-plan JSON should expose plan counts");
    expect_contains(batch_plan_process.stdout_text, "\"errorCount\": 0",
        "#1305: selection-toolbox-create-batch-plan JSON should expose zero errors");
    expect_contains(batch_plan_process.stdout_text,
        "\"planReadyItemIds\": [\"textbox\", \"commandbutton\", \"textbox\"]",
        "#1404: selection-toolbox-create-batch-plan JSON should summarize plan-ready item ids");
    expect_contains(batch_plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1404: selection-toolbox-create-batch-plan JSON should expose empty blocked item ids");
    expect_contains(batch_plan_process.stdout_text, "\"planBlockedErrors\": []",
        "#1404: selection-toolbox-create-batch-plan JSON should expose empty blocked plan errors");
    expect_contains(batch_plan_process.stdout_text, "\"batchPlanOk\": true",
        "#1305: selection-toolbox-create-batch-plan JSON should expose nested batch-plan state");
    expect_contains(batch_plan_process.stdout_text, "\"batchPlan\": {",
        "#1305: selection-toolbox-create-batch-plan JSON should expose nested batch plans");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1305: selection-toolbox-create-batch-plan JSON should expose textbox descriptors");
    expect_contains(batch_plan_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1305: selection-toolbox-create-batch-plan JSON should expose command button descriptors");
    expect_contains(batch_plan_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1305: selection-toolbox-create-batch-plan JSON should expose first target records");
    expect_contains(batch_plan_process.stdout_text, "\"targetRecordIndex\": 4",
        "#1305: selection-toolbox-create-batch-plan JSON should expose later target records");
    expect_contains(batch_plan_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1305: selection-toolbox-create-batch-plan JSON should expose first generated names");
    expect_contains(batch_plan_process.stdout_text, "\"objectName\": \"txt3\"",
        "#1305: selection-toolbox-create-batch-plan JSON should reserve generated names across the batch");
    expect_contains(batch_plan_process.stdout_text, "\"objectName\": \"cmdSelection\"",
        "#1305: selection-toolbox-create-batch-plan JSON should preserve explicit object names");
    expect_contains(batch_plan_process.stdout_text, "\"uniqueId\": \"selection-first-textbox-guid\"",
        "#1305: selection-toolbox-create-batch-plan JSON should expose per-item unique ids");
    expect_contains(batch_plan_process.stdout_text, "\"parentName\": \"cntToolbar\"",
        "#1305: selection-toolbox-create-batch-plan JSON should expose per-item parent names");
    expect_contains(batch_plan_process.stdout_text, "\"propertyValue\": \"Second Selection\"",
        "#1305: selection-toolbox-create-batch-plan JSON should preserve per-item field values");
    expect_contains(batch_plan_process.stdout_text, "\"dryRun\": true",
        "#1305: selection-toolbox-create-batch-plan JSON should expose dry-run state");
    expect_contains(batch_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#1305: selection-toolbox-create-batch-plan JSON should remain non-mutating");
    expect(visual_object_count(form_path) == before_count,
        "#1305: selection-toolbox-create-batch-plan host command should not mutate assets");

    const auto report_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan",
            "--selection-context", "report_expression",
            "--toolbox-item", "label",
            "--unique-id", "selection-report-label-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Selection",
            "--json"
        },
        temp_root);
    expect(report_plan_process.exit_code == 0,
        "#1305: report selection-toolbox-create-batch-plan JSON command should exit successfully");
    expect_contains(report_plan_process.stdout_text, "\"selectionToolboxCreateBatchPlan\": {",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose a stable result object");
    expect_contains(report_plan_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1305: report selection-toolbox-create-batch-plan JSON should expose report selections");
    expect_contains(report_plan_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1305: report selection-toolbox-create-batch-plan JSON should resolve report contexts");
    expect_contains(report_plan_process.stdout_text, "\"launchPlanOk\": true",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose launch state");
    expect_contains(report_plan_process.stdout_text, "\"itemCount\": 1",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose report item counts");
    expect_contains(report_plan_process.stdout_text, "\"planCount\": 1",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose plan counts");
    expect_contains(report_plan_process.stdout_text, "\"errorCount\": 0",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose zero errors");
    expect_contains(report_plan_process.stdout_text, "\"batchPlanOk\": true",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose batch-plan state");
    expect_contains(report_plan_process.stdout_text, "\"batchPlan\": {",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose nested batch plans");
    expect_contains(report_plan_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1305: report selection-toolbox-create-batch-plan JSON should expose label plans");
    expect_contains(report_plan_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#1305: report selection-toolbox-create-batch-plan JSON should expose generated label names");
    expect_contains(report_plan_process.stdout_text, "\"uniqueId\": \"selection-report-label-guid\"",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose caller unique ids");
    expect_contains(report_plan_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2120: report selection-toolbox-create-batch-plan JSON should preserve report parent payloads");
    expect_contains(report_plan_process.stdout_text, "\"propertyValue\": \"Report Selection\"",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose caller report fields");
    expect_contains(report_plan_process.stdout_text, "\"planReadyItemIds\": [\"label\"]",
        "#2120: report selection-toolbox-create-batch-plan JSON should summarize plan-ready report item ids");
    expect_contains(report_plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose empty blocked item ids");
    expect_contains(report_plan_process.stdout_text, "\"planBlockedErrors\": []",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose empty blocked plan errors");
    expect_contains(report_plan_process.stdout_text, "\"dryRun\": true",
        "#2120: report selection-toolbox-create-batch-plan JSON should expose dry-run state");
    expect_contains(report_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#2120: report selection-toolbox-create-batch-plan JSON should remain non-mutating");
    expect_not_contains(report_plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#1305: report selection-toolbox-create-batch-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2120: report selection-toolbox-create-batch-plan host command should not mutate assets");

    const auto label_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan",
            "--selection-context", "label_expression",
            "--toolbox-item", "label",
            "--unique-id", "selection-label-batch-plan-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Label Selection",
            "--json"
        },
        temp_root);
    expect(label_plan_process.exit_code == 0,
        "#2087: label selection-toolbox-create-batch-plan JSON command should exit successfully");
    expect_contains(label_plan_process.stdout_text, "\"selectionToolboxCreateBatchPlan\": {",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose a stable result object");
    expect_contains(label_plan_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#2087: label selection-toolbox-create-batch-plan JSON should expose label selections");
    expect_contains(label_plan_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2087: label selection-toolbox-create-batch-plan JSON should resolve report contexts");
    expect_contains(label_plan_process.stdout_text, "\"launchPlanOk\": true",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose launch state");
    expect_contains(label_plan_process.stdout_text, "\"itemCount\": 1",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose label item counts");
    expect_contains(label_plan_process.stdout_text, "\"planCount\": 1",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose plan counts");
    expect_contains(label_plan_process.stdout_text, "\"errorCount\": 0",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose zero errors");
    expect_contains(label_plan_process.stdout_text, "\"batchPlanOk\": true",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose batch-plan state");
    expect_contains(label_plan_process.stdout_text, "\"batchPlan\": {",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose nested batch plans");
    expect_contains(label_plan_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2087: label selection-toolbox-create-batch-plan JSON should expose label plans");
    expect_contains(label_plan_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2087: label selection-toolbox-create-batch-plan JSON should expose generated label names");
    expect_contains(label_plan_process.stdout_text, "\"uniqueId\": \"selection-label-batch-plan-guid\"",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose caller unique ids");
    expect_contains(label_plan_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2122: label selection-toolbox-create-batch-plan JSON should preserve label parent payloads");
    expect_contains(label_plan_process.stdout_text, "\"propertyValue\": \"Label Selection\"",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose caller label fields");
    expect_contains(label_plan_process.stdout_text, "\"planReadyItemIds\": [\"label\"]",
        "#2122: label selection-toolbox-create-batch-plan JSON should summarize plan-ready label item ids");
    expect_contains(label_plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose empty blocked item ids");
    expect_contains(label_plan_process.stdout_text, "\"planBlockedErrors\": []",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose empty blocked plan errors");
    expect_contains(label_plan_process.stdout_text, "\"dryRun\": true",
        "#2122: label selection-toolbox-create-batch-plan JSON should expose dry-run state");
    expect_contains(label_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#2122: label selection-toolbox-create-batch-plan JSON should remain non-mutating");
    expect_not_contains(label_plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#2087: label selection-toolbox-create-batch-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2087: label selection-toolbox-create-batch-plan host command should not mutate assets");

    const auto unavailable_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan",
            "--selection-context", "report_expression",
            "--toolbox-item", "textbox",
            "--json"
        },
        temp_root);
    expect(unavailable_plan_process.exit_code == 4,
        "#1305: selection-toolbox-create-batch-plan JSON should reject unavailable selected-context items");
    expect_contains(unavailable_plan_process.stdout_text, "\"batchPlanOk\": false",
        "#1305: unavailable selection-toolbox-create-batch-plan JSON should expose failed batch-plan state");
    expect_contains(unavailable_plan_process.stdout_text, "\"planReadyItemIds\": []",
        "#1404: unavailable selection-toolbox-create-batch-plan JSON should expose empty ready item ids");
    expect_contains(unavailable_plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1404: unavailable selection-toolbox-create-batch-plan JSON should not fabricate blocked item ids");
    expect_contains(unavailable_plan_process.stdout_text,
        "\"planBlockedErrors\": [\"The requested toolbox item is not available in the requested designer context.\"]",
        "#1404: unavailable selection-toolbox-create-batch-plan JSON should summarize blocked plan errors");
    expect_contains(unavailable_plan_process.stdout_text,
        "The requested toolbox item is not available in the requested designer context.",
        "#1305: unavailable selection-toolbox-create-batch-plan JSON should report planner errors");
    expect_not_contains(unavailable_plan_process.stdout_text, "\"plans\": [",
        "#1305: unavailable selection-toolbox-create-batch-plan JSON should omit stale plans");

    const auto unsupported_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan",
            "--selection-context", "menu_item",
            "--toolbox-item", "textbox",
            "--json"
        },
        temp_root);
    expect(unsupported_plan_process.exit_code == 4,
        "#1305: selection-toolbox-create-batch-plan JSON should reject unsupported selections");
    expect_contains(unsupported_plan_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1305: unsupported selection-toolbox-create-batch-plan JSON should preserve selected contexts");
    expect_contains(unsupported_plan_process.stdout_text,
        "A selection-context toolbox object batch creation plan request requires a toolbox palette.",
        "#1305: unsupported selection-toolbox-create-batch-plan JSON should report palette errors");
    expect_not_contains(unsupported_plan_process.stdout_text, "\"plans\": [",
        "#1305: unsupported selection-toolbox-create-batch-plan JSON should omit stale plans");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan",
            "--toolbox-item", "textbox",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1305: selection-toolbox-create-batch-plan JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1305: missing selection selection-toolbox-create-batch-plan JSON should report parser errors");

    const auto missing_items_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(missing_items_process.exit_code == 2,
        "#1305: selection-toolbox-create-batch-plan JSON should reject empty item lists");
    expect_contains(missing_items_process.stdout_text, "No toolbox item ids were provided.",
        "#1305: empty selection-toolbox-create-batch-plan item lists should report parser errors");

    const auto orphan_item_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan",
            "--selection-context", "visual_object",
            "--parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(orphan_item_option_process.exit_code == 2,
        "#1305: selection-toolbox-create-batch-plan JSON should reject item options before items");
    expect_contains(orphan_item_option_process.stdout_text,
        "Selection toolbox batch item options require a preceding --toolbox-item.",
        "#1305: orphan selection-toolbox-create-batch-plan item options should report parser errors");

    const auto malformed_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(malformed_field_process.exit_code == 2,
        "#1305: selection-toolbox-create-batch-plan JSON should reject malformed field values");
    expect_contains(malformed_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1305: malformed selection-toolbox-create-batch-plan field values should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1305: selection-toolbox-create-batch-plan JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-toolbox-create-batch-plan option: --toolbox-context",
        "#1305: unknown option selection-toolbox-create-batch-plan JSON should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1305: rejected selection-toolbox-create-batch-plan host commands should not mutate assets");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
