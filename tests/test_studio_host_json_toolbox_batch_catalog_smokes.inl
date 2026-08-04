// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

// Batch toolbox catalog planning and dispatch coverage.
void test_studio_host_json_plans_toolbox_object_creation_batch_plan_catalog(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_batch_plan_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-plan-catalog",
            "--toolbox-context", "form",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Batch Plan Catalog",
            "--json"
        },
        temp_root);
    expect(catalog_process.exit_code == 0,
        "#1258: toolbox-create-batch-plan-catalog JSON command should exit successfully");
    expect_contains(catalog_process.stdout_text, "\"toolboxCreateBatchPlanCatalog\": {",
        "#1258: toolbox-create-batch-plan-catalog JSON should expose a catalog object");
    expect_contains(catalog_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1258: toolbox-create-batch-plan-catalog JSON should expose requested contexts");
    expect_contains(catalog_process.stdout_text, "\"planCount\": 1",
        "#1258: toolbox-create-batch-plan-catalog JSON should expose one batch plan");
    expect_contains(catalog_process.stdout_text, "\"errorCount\": 0",
        "#1258: toolbox-create-batch-plan-catalog JSON should expose zero errors");
    expect_contains(catalog_process.stdout_text, "\"batchPlanOk\": true",
        "#1258: toolbox-create-batch-plan-catalog JSON should expose batch plan state");
    expect_contains(catalog_process.stdout_text, "\"batchPlan\": {",
        "#1258: toolbox-create-batch-plan-catalog JSON should expose nested batch plans");
    expect_contains(catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1258: toolbox-create-batch-plan-catalog JSON should include textbox plans");
    expect_contains(catalog_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1258: toolbox-create-batch-plan-catalog JSON should include command button plans");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1258: toolbox-create-batch-plan-catalog JSON should expose generated textbox names");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"cmd1\"",
        "#1258: toolbox-create-batch-plan-catalog JSON should expose generated command names");
    expect_contains(catalog_process.stdout_text, "\"propertyValue\": \"Batch Plan Catalog\"",
        "#1258: toolbox-create-batch-plan-catalog JSON should preserve shared field values");
    expect_contains(catalog_process.stdout_text, "\"dryRun\": true",
        "#1258: toolbox-create-batch-plan-catalog JSON should expose dry-run state");
    expect_contains(catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#1258: toolbox-create-batch-plan-catalog JSON should remain non-mutating");
    expect_contains(catalog_process.stdout_text,
        "\"planReadyItemIds\": [\"label\", \"textbox\", \"editbox\", \"commandbutton\"",
        "#1378: toolbox-create-batch-plan-catalog JSON should summarize plan-ready form items");
    expect_contains(catalog_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1378: toolbox-create-batch-plan-catalog JSON should summarize empty blocked item ids");
    expect_contains(catalog_process.stdout_text, "\"planBlockedErrors\": []",
        "#1378: toolbox-create-batch-plan-catalog JSON should summarize empty blocked plan errors");
    expect(visual_object_count(form_path) == before_count,
        "#1258: toolbox-create-batch-plan-catalog host command should not mutate the visual asset");

    const auto report_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-plan-catalog",
            "--toolbox-context", "report",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Batch Plan Catalog",
            "--json"
        },
        temp_root);
    expect(report_catalog_process.exit_code == 0,
        "#1258: report toolbox-create-batch-plan-catalog JSON command should exit successfully");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxCreateBatchPlanCatalog\": {",
        "#2109: report toolbox-create-batch-plan-catalog JSON should expose a catalog object");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1258: report toolbox-create-batch-plan-catalog JSON should expose report contexts");
    expect_contains(report_catalog_process.stdout_text, "\"planCount\": 1",
        "#2109: report toolbox-create-batch-plan-catalog JSON should expose one report batch plan");
    expect_contains(report_catalog_process.stdout_text, "\"errorCount\": 0",
        "#2109: report toolbox-create-batch-plan-catalog JSON should expose zero catalog errors");
    expect_contains(report_catalog_process.stdout_text, "\"batchPlan\": {",
        "#2109: report toolbox-create-batch-plan-catalog JSON should expose nested batch plans");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1258: report toolbox-create-batch-plan-catalog JSON should include label plans");
    expect_contains(report_catalog_process.stdout_text, "\"planReadyItemIds\": [\"label\"",
        "#1378: report toolbox-create-batch-plan-catalog JSON should summarize plan-ready report items");
    expect_contains(report_catalog_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1378: report toolbox-create-batch-plan-catalog JSON should summarize empty blocked item ids");
    expect_contains(report_catalog_process.stdout_text, "\"planBlockedErrors\": []",
        "#1378: report toolbox-create-batch-plan-catalog JSON should summarize empty blocked plan errors");
    expect_contains(report_catalog_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2109: report toolbox-create-batch-plan-catalog JSON should expose generated label names");
    expect_contains(report_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2109: report toolbox-create-batch-plan-catalog JSON should preserve report parent payloads");
    expect_contains(report_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2146: report toolbox-create-batch-plan-catalog JSON should expose caller field names");
    expect_contains(report_catalog_process.stdout_text, "\"propertyValue\": \"Report Batch Plan Catalog\"",
        "#2146: report toolbox-create-batch-plan-catalog JSON should expose caller field values");
    expect_contains(report_catalog_process.stdout_text, "\"dryRun\": true",
        "#2109: report toolbox-create-batch-plan-catalog JSON should remain dry-run");
    expect_contains(report_catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#2109: report toolbox-create-batch-plan-catalog JSON should remain non-mutating");
    expect_not_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1258: report toolbox-create-batch-plan-catalog JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2109: report toolbox-create-batch-plan-catalog host command should not mutate assets");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-plan-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1258: toolbox-create-batch-plan-catalog JSON should reject missing contexts");
    expect_contains(missing_context_process.stdout_text, "No toolbox context was provided.",
        "#1258: missing toolbox-create-batch-plan-catalog context JSON should report parser errors");

    const auto invalid_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-plan-catalog",
            "--toolbox-context", "form",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(invalid_field_process.exit_code == 2,
        "#1258: toolbox-create-batch-plan-catalog JSON should reject malformed field values");
    expect_contains(invalid_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1258: malformed toolbox-create-batch-plan-catalog field values should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1258: rejected toolbox-create-batch-plan-catalog host commands should not mutate the visual asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
void test_studio_host_json_plans_selection_toolbox_object_creation_batch_plan_catalog(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_create_batch_plan_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto visual_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan-catalog",
            "--selection-context", "visual_object",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Selection Batch Plan",
            "--json"
        },
        temp_root);
    expect(visual_catalog_process.exit_code == 0,
        "#1297: selection toolbox batch create-plan catalog JSON command should exit successfully");
    expect_contains(visual_catalog_process.stdout_text, "\"selectionToolboxCreateBatchPlanCatalog\": {",
        "#1297: selection toolbox batch create-plan catalog JSON should expose catalog objects");
    expect_contains(visual_catalog_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1297: selection toolbox batch create-plan catalog JSON should expose selected Studio contexts");
    expect_contains(visual_catalog_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1297: visual selection toolbox batch create-plan catalog JSON should resolve form contexts");
    expect_contains(visual_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#1297: selection toolbox batch create-plan catalog JSON should expose launch state");
    expect_contains(visual_catalog_process.stdout_text, "\"planCount\": 1",
        "#1297: selection toolbox batch create-plan catalog JSON should expose one batch plan");
    expect_contains(visual_catalog_process.stdout_text, "\"errorCount\": 0",
        "#1297: selection toolbox batch create-plan catalog JSON should expose zero errors");
    expect_contains(visual_catalog_process.stdout_text, "\"batchPlanOk\": true",
        "#1297: selection toolbox batch create-plan catalog JSON should expose batch plan state");
    expect_contains(visual_catalog_process.stdout_text, "\"batchPlan\": {",
        "#1297: selection toolbox batch create-plan catalog JSON should expose nested batch plans");
    expect_contains(visual_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1297: visual selection toolbox batch create-plan catalog JSON should include textbox plans");
    expect_contains(visual_catalog_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1297: visual selection toolbox batch create-plan catalog JSON should include command button plans");
    expect_contains(visual_catalog_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1297: visual selection toolbox batch create-plan catalog JSON should expose generated textbox names");
    expect_contains(visual_catalog_process.stdout_text, "\"objectName\": \"cmd1\"",
        "#1297: visual selection toolbox batch create-plan catalog JSON should expose generated command names");
    expect_contains(visual_catalog_process.stdout_text, "\"propertyValue\": \"Selection Batch Plan\"",
        "#1297: selection toolbox batch create-plan catalog JSON should preserve shared field values");
    expect_contains(visual_catalog_process.stdout_text, "\"dryRun\": true",
        "#1297: selection toolbox batch create-plan catalog JSON should expose dry-run state");
    expect_contains(visual_catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#1297: selection toolbox batch create-plan catalog JSON should remain non-mutating");
    expect_contains(visual_catalog_process.stdout_text,
        "\"planReadyItemIds\": [\"label\", \"textbox\", \"editbox\", \"commandbutton\"",
        "#1379: selection toolbox batch create-plan catalog JSON should summarize plan-ready visual items");
    expect_contains(visual_catalog_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1379: selection toolbox batch create-plan catalog JSON should summarize empty blocked item ids");
    expect_contains(visual_catalog_process.stdout_text, "\"planBlockedErrors\": []",
        "#1379: selection toolbox batch create-plan catalog JSON should summarize empty blocked plan errors");
    expect(visual_object_count(form_path) == before_count,
        "#1297: visual selection toolbox batch create-plan catalog host command should not mutate assets");

    const auto report_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan-catalog",
            "--selection-context", "report_expression",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Selection Batch Plan Catalog",
            "--json"
        },
        temp_root);
    expect(report_catalog_process.exit_code == 0,
        "#1297: report selection toolbox batch create-plan catalog JSON command should exit successfully");
    expect_contains(report_catalog_process.stdout_text, "\"selectionToolboxCreateBatchPlanCatalog\": {",
        "#2113: report selection toolbox batch create-plan catalog JSON should expose catalog objects");
    expect_contains(report_catalog_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1297: report selection toolbox batch create-plan catalog JSON should expose report selections");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1297: report selection toolbox batch create-plan catalog JSON should resolve report contexts");
    expect_contains(report_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#2113: report selection toolbox batch create-plan catalog JSON should expose launch state");
    expect_contains(report_catalog_process.stdout_text, "\"planCount\": 1",
        "#2113: report selection toolbox batch create-plan catalog JSON should expose one report batch plan");
    expect_contains(report_catalog_process.stdout_text, "\"errorCount\": 0",
        "#2113: report selection toolbox batch create-plan catalog JSON should expose zero catalog errors");
    expect_contains(report_catalog_process.stdout_text, "\"batchPlan\": {",
        "#2113: report selection toolbox batch create-plan catalog JSON should expose nested batch plans");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1297: report selection toolbox batch create-plan catalog JSON should include label plans");
    expect_contains(report_catalog_process.stdout_text, "\"planReadyItemIds\": [\"label\"",
        "#1379: report selection toolbox batch create-plan catalog JSON should summarize plan-ready report items");
    expect_contains(report_catalog_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1379: report selection toolbox batch create-plan catalog JSON should summarize empty blocked item ids");
    expect_contains(report_catalog_process.stdout_text, "\"planBlockedErrors\": []",
        "#1379: report selection toolbox batch create-plan catalog JSON should summarize empty blocked plan errors");
    expect_contains(report_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2113: report selection toolbox batch create-plan catalog JSON should preserve report parent payloads");
    expect_contains(report_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2152: report selection toolbox batch create-plan catalog JSON should expose caller field names");
    expect_contains(report_catalog_process.stdout_text, "\"propertyValue\": \"Report Selection Batch Plan Catalog\"",
        "#2152: report selection toolbox batch create-plan catalog JSON should expose caller field values");
    expect_contains(report_catalog_process.stdout_text, "\"dryRun\": true",
        "#2113: report selection toolbox batch create-plan catalog JSON should remain dry-run");
    expect_contains(report_catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#2113: report selection toolbox batch create-plan catalog JSON should remain non-mutating");
    expect_not_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1297: report selection toolbox batch create-plan catalog JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#1297: report selection toolbox batch create-plan catalog host command should not mutate assets");

    const auto label_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan-catalog",
            "--selection-context", "label_expression",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Label Selection Batch Plan Catalog",
            "--json"
        },
        temp_root);
    expect(label_catalog_process.exit_code == 0,
        "#2089: label selection toolbox batch create-plan catalog JSON command should exit successfully");
    expect_contains(label_catalog_process.stdout_text, "\"selectionToolboxCreateBatchPlanCatalog\": {",
        "#2127: label selection toolbox batch create-plan catalog JSON should expose catalog objects");
    expect_contains(label_catalog_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#2089: label selection toolbox batch create-plan catalog JSON should expose label selections");
    expect_contains(label_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2089: label selection toolbox batch create-plan catalog JSON should resolve report contexts");
    expect_contains(label_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#2127: label selection toolbox batch create-plan catalog JSON should expose launch state");
    expect_contains(label_catalog_process.stdout_text, "\"planCount\": 1",
        "#2127: label selection toolbox batch create-plan catalog JSON should expose one label batch plan");
    expect_contains(label_catalog_process.stdout_text, "\"errorCount\": 0",
        "#2127: label selection toolbox batch create-plan catalog JSON should expose zero catalog errors");
    expect_contains(label_catalog_process.stdout_text, "\"batchPlan\": {",
        "#2127: label selection toolbox batch create-plan catalog JSON should expose nested batch plans");
    expect_contains(label_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2089: label selection toolbox batch create-plan catalog JSON should include label plans");
    expect_contains(label_catalog_process.stdout_text, "\"planReadyItemIds\": [\"label\"",
        "#2089: label selection toolbox batch create-plan catalog JSON should summarize plan-ready label items");
    expect_contains(label_catalog_process.stdout_text, "\"planBlockedItemIds\": []",
        "#2089: label selection toolbox batch create-plan catalog JSON should summarize empty blocked item ids");
    expect_contains(label_catalog_process.stdout_text, "\"planBlockedErrors\": []",
        "#2089: label selection toolbox batch create-plan catalog JSON should summarize empty blocked plan errors");
    expect_contains(label_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2127: label selection toolbox batch create-plan catalog JSON should preserve label parent payloads");
    expect_contains(label_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2154: label selection toolbox batch create-plan catalog JSON should expose caller field names");
    expect_contains(label_catalog_process.stdout_text, "\"propertyValue\": \"Label Selection Batch Plan Catalog\"",
        "#2154: label selection toolbox batch create-plan catalog JSON should expose caller field values");
    expect_contains(label_catalog_process.stdout_text, "\"dryRun\": true",
        "#2127: label selection toolbox batch create-plan catalog JSON should remain dry-run");
    expect_contains(label_catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#2127: label selection toolbox batch create-plan catalog JSON should remain non-mutating");
    expect_not_contains(label_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#2089: label selection toolbox batch create-plan catalog JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2127: label selection toolbox batch create-plan catalog host command should not mutate assets");

    const auto unsupported_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan-catalog",
            "--selection-context", "menu_item",
            "--json"
        },
        temp_root);
    expect(unsupported_catalog_process.exit_code == 4,
        "#1297: selection toolbox batch create-plan catalog JSON should reject unsupported selections");
    expect_contains(unsupported_catalog_process.stdout_text, "\"selectionToolboxCreateBatchPlanCatalog\": null",
        "#1297: unsupported selection toolbox batch create-plan catalog JSON should omit catalog objects");
    expect_contains(unsupported_catalog_process.stdout_text,
        "A selection-context toolbox object batch creation catalog request requires a toolbox palette.",
        "#1297: unsupported selection toolbox batch create-plan catalog JSON should report planner errors");
    expect(visual_object_count(form_path) == before_count,
        "#1297: unsupported selection toolbox batch create-plan catalog host command should not mutate assets");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan-catalog",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1297: selection toolbox batch create-plan catalog JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1297: missing selection toolbox batch create-plan catalog JSON should report parser errors");

    const auto unknown_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan-catalog",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_selection_process.exit_code == 2,
        "#1297: selection toolbox batch create-plan catalog JSON should reject unknown selections");
    expect_contains(unknown_selection_process.stdout_text, "Unknown selection context token: unknown",
        "#1297: unknown selection toolbox batch create-plan catalog JSON should report parser errors");

    const auto invalid_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan-catalog",
            "--selection-context", "visual_object",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(invalid_field_process.exit_code == 2,
        "#1297: selection toolbox batch create-plan catalog JSON should reject malformed field values");
    expect_contains(invalid_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1297: malformed selection toolbox batch create-plan catalog field values should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-plan-catalog",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1297: selection toolbox batch create-plan catalog JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-toolbox-create-batch-plan-catalog option: --toolbox-context",
        "#1297: unknown selection toolbox batch create-plan catalog options should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1297: rejected selection toolbox batch create-plan catalog host commands should not mutate assets");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_toolbox_object_creation_batch_dispatch_catalog(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_batch_dispatch_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-catalog",
            "--toolbox-context", "form",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Batch Dispatch Catalog",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(catalog_process.exit_code == 0,
        "#1256: toolbox-create-batch-dispatch-catalog JSON command should exit successfully");
    expect_contains(catalog_process.stdout_text, "\"toolboxCreateBatchDispatchCatalog\": {",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should expose a catalog object");
    expect_contains(catalog_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should expose requested contexts");
    expect_contains(catalog_process.stdout_text, "\"dispatchCount\": 1",
        "#1256: admitted toolbox-create-batch-dispatch-catalog JSON should expose one batch dispatch");
    expect_contains(catalog_process.stdout_text, "\"errorCount\": 0",
        "#1256: admitted toolbox-create-batch-dispatch-catalog JSON should expose zero errors");
    expect_contains(catalog_process.stdout_text, "\"batchPlanOk\": true",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should expose batch plan state");
    expect_contains(catalog_process.stdout_text, "\"batchPlan\": {",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should expose nested batch plans");
    expect_contains(catalog_process.stdout_text, "\"dispatchOk\": true",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should expose dispatch state");
    expect_contains(catalog_process.stdout_text, "\"dispatch\": {",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should expose nested dispatch plans");
    expect_contains(catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should include textbox plans");
    expect_contains(catalog_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should include command button plans");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should expose generated textbox names");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"cmd1\"",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should expose generated command names");
    expect_contains(catalog_process.stdout_text, "\"dispatchArguments\": [",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should expose dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"--toolbox-create-batch\"",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should dispatch to toolbox-create-batch");
    expect_contains(catalog_process.stdout_text, "\"--toolbox-item\", \"textbox\"",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should include textbox dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"--toolbox-item\", \"commandbutton\"",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should include command dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"--field-value\", \"CAPTION=Batch Dispatch Catalog\"",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should preserve shared field values");
    expect_contains(catalog_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1256: toolbox-create-batch-dispatch-catalog JSON should expose dispatch admission state");
    expect_contains(catalog_process.stdout_text, "\"dryRun\": false",
        "#1256: admitted toolbox-create-batch-dispatch-catalog JSON should expose non-dry-run state");
    expect_contains(catalog_process.stdout_text, "\"mutatesAsset\": true",
        "#1256: admitted toolbox-create-batch-dispatch-catalog JSON should expose mutation intent");
    expect_contains(catalog_process.stdout_text,
        "\"dispatchReadyItemIds\": [\"label\", \"textbox\", \"editbox\", \"commandbutton\"",
        "#1380: toolbox-create-batch-dispatch-catalog JSON should summarize dispatch-ready form items");
    expect_contains(catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1380: admitted toolbox-create-batch-dispatch-catalog JSON should summarize empty blocked item ids");
    expect_contains(catalog_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1380: admitted toolbox-create-batch-dispatch-catalog JSON should summarize empty blocked dispatch errors");
    expect(visual_object_count(form_path) == before_count,
        "#1256: toolbox-create-batch-dispatch-catalog host command should not mutate the visual asset");

    const auto dry_run_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-catalog",
            "--toolbox-context", "form",
            "--parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(dry_run_catalog_process.exit_code == 0,
        "#1256: non-admitted toolbox-create-batch-dispatch-catalog JSON should return a catalog");
    expect_contains(dry_run_catalog_process.stdout_text, "\"batchPlanOk\": true",
        "#1256: non-admitted toolbox-create-batch-dispatch-catalog JSON should preserve batch plans");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchCount\": 0",
        "#1256: non-admitted toolbox-create-batch-dispatch-catalog JSON should expose zero dispatches");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1380: non-admitted toolbox-create-batch-dispatch-catalog JSON should summarize empty ready item ids");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1380: non-admitted toolbox-create-batch-dispatch-catalog JSON should summarize aggregate blocked state without fabricated item ids");
    expect_contains(dry_run_catalog_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"A toolbox batch create dispatch request requires an admitted non-dry-run create operation.\"",
        "#1380: non-admitted toolbox-create-batch-dispatch-catalog JSON should summarize blocked dispatch errors");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatch\": null",
        "#1256: non-admitted toolbox-create-batch-dispatch-catalog JSON should not expose stale dispatch plans");
    expect_contains(dry_run_catalog_process.stdout_text,
        "A toolbox batch create dispatch request requires an admitted non-dry-run create operation.",
        "#1256: non-admitted toolbox-create-batch-dispatch-catalog JSON should expose dispatch errors");
    expect_not_contains(dry_run_catalog_process.stdout_text, "\"--toolbox-create-batch\"",
        "#1256: non-admitted toolbox-create-batch-dispatch-catalog JSON should not expose stale arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1256: non-admitted toolbox-create-batch-dispatch-catalog host command should not mutate the asset");

    const auto report_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-catalog",
            "--toolbox-context", "report",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Batch Dispatch Catalog",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(report_catalog_process.exit_code == 0,
        "#1256: report toolbox-create-batch-dispatch-catalog JSON command should exit successfully");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxCreateBatchDispatchCatalog\": {",
        "#2110: report toolbox-create-batch-dispatch-catalog JSON should expose a catalog object");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1256: report toolbox-create-batch-dispatch-catalog JSON should expose report contexts");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchCount\": 1",
        "#2110: report toolbox-create-batch-dispatch-catalog JSON should expose one report batch dispatch");
    expect_contains(report_catalog_process.stdout_text, "\"errorCount\": 0",
        "#2110: report toolbox-create-batch-dispatch-catalog JSON should expose zero catalog errors");
    expect_contains(report_catalog_process.stdout_text, "\"batchPlan\": {",
        "#2110: report toolbox-create-batch-dispatch-catalog JSON should expose nested batch plans");
    expect_contains(report_catalog_process.stdout_text, "\"dispatch\": {",
        "#2110: report toolbox-create-batch-dispatch-catalog JSON should expose nested dispatch plans");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1256: report toolbox-create-batch-dispatch-catalog JSON should include label plans");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"",
        "#1380: report toolbox-create-batch-dispatch-catalog JSON should summarize dispatch-ready report items");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1380: report toolbox-create-batch-dispatch-catalog JSON should summarize empty blocked item ids");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1380: report toolbox-create-batch-dispatch-catalog JSON should summarize empty blocked dispatch errors");
    expect_contains(report_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2110: report toolbox-create-batch-dispatch-catalog JSON should preserve report parent payloads");
    expect_contains(report_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2147: report toolbox-create-batch-dispatch-catalog JSON should expose caller field names");
    expect_contains(report_catalog_process.stdout_text, "\"propertyValue\": \"Report Batch Dispatch Catalog\"",
        "#2147: report toolbox-create-batch-dispatch-catalog JSON should expose caller field values");
    expect_contains(report_catalog_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#1256: report toolbox-create-batch-dispatch-catalog JSON should preserve report dispatch context");
    expect_contains(report_catalog_process.stdout_text,
        "\"--field-value\", \"CAPTION=Report Batch Dispatch Catalog\"",
        "#2147: report toolbox-create-batch-dispatch-catalog JSON should preserve report dispatch field arguments");
    expect_contains(report_catalog_process.stdout_text, "\"dryRun\": false",
        "#2110: report toolbox-create-batch-dispatch-catalog JSON should expose non-dry-run dispatch state");
    expect_contains(report_catalog_process.stdout_text, "\"mutatesAsset\": true",
        "#2110: report toolbox-create-batch-dispatch-catalog JSON should expose mutation intent");
    expect_not_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1256: report toolbox-create-batch-dispatch-catalog JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2110: report toolbox-create-batch-dispatch-catalog host command should not mutate assets");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1256: toolbox-create-batch-dispatch-catalog JSON should reject missing contexts");
    expect_contains(missing_context_process.stdout_text, "No toolbox context was provided.",
        "#1256: missing toolbox-create-batch-dispatch-catalog context JSON should report parser errors");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-catalog",
            "--toolbox-context", "form",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1256: toolbox-create-batch-dispatch-catalog JSON should reject invalid admission tokens");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#1256: invalid toolbox-create-batch-dispatch-catalog admission tokens should report parser errors");

    const auto invalid_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-catalog",
            "--toolbox-context", "form",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(invalid_field_process.exit_code == 2,
        "#1256: toolbox-create-batch-dispatch-catalog JSON should reject malformed field values");
    expect_contains(invalid_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1256: malformed toolbox-create-batch-dispatch-catalog field values should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1256: rejected toolbox-create-batch-dispatch-catalog host commands should not mutate the visual asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_selection_toolbox_object_creation_batch_dispatch_catalog(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_selection_toolbox_create_batch_dispatch_catalog_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--selection-context", "visual_object",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Selection Batch Dispatch",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(catalog_process.exit_code == 0,
        "#1299: selection toolbox batch dispatch catalog JSON command should exit successfully");
    expect_contains(catalog_process.stdout_text, "\"selectionToolboxCreateBatchDispatchCatalog\": {",
        "#1299: selection toolbox batch dispatch catalog JSON should expose a catalog object");
    expect_contains(catalog_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1299: selection toolbox batch dispatch catalog JSON should expose selected contexts");
    expect_contains(catalog_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1299: selection toolbox batch dispatch catalog JSON should expose resolved toolbox contexts");
    expect_contains(catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#1299: selection toolbox batch dispatch catalog JSON should expose launch metadata");
    expect_contains(catalog_process.stdout_text, "\"dispatchCount\": 1",
        "#1299: admitted selection toolbox batch dispatch catalog JSON should expose one dispatch");
    expect_contains(catalog_process.stdout_text, "\"errorCount\": 0",
        "#1299: admitted selection toolbox batch dispatch catalog JSON should expose zero errors");
    expect_contains(catalog_process.stdout_text, "\"batchPlanOk\": true",
        "#1299: selection toolbox batch dispatch catalog JSON should expose batch plan state");
    expect_contains(catalog_process.stdout_text, "\"batchPlan\": {",
        "#1299: selection toolbox batch dispatch catalog JSON should expose nested batch plans");
    expect_contains(catalog_process.stdout_text, "\"dispatchOk\": true",
        "#1299: selection toolbox batch dispatch catalog JSON should expose dispatch state");
    expect_contains(catalog_process.stdout_text, "\"dispatch\": {",
        "#1299: selection toolbox batch dispatch catalog JSON should expose nested dispatch plans");
    expect_contains(catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1299: selection toolbox batch dispatch catalog JSON should include textbox plans");
    expect_contains(catalog_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1299: selection toolbox batch dispatch catalog JSON should include command button plans");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1299: selection toolbox batch dispatch catalog JSON should expose generated textbox names");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"cmd1\"",
        "#1299: selection toolbox batch dispatch catalog JSON should expose generated command names");
    expect_contains(catalog_process.stdout_text, "\"dispatchArguments\": [",
        "#1299: selection toolbox batch dispatch catalog JSON should expose dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"--toolbox-create-batch\"",
        "#1299: selection toolbox batch dispatch catalog JSON should dispatch to toolbox-create-batch");
    expect_contains(catalog_process.stdout_text, "\"--toolbox-item\", \"textbox\"",
        "#1299: selection toolbox batch dispatch catalog JSON should include textbox dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"--toolbox-item\", \"commandbutton\"",
        "#1299: selection toolbox batch dispatch catalog JSON should include command dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"--field-value\", \"CAPTION=Selection Batch Dispatch\"",
        "#1299: selection toolbox batch dispatch catalog JSON should preserve shared field values");
    expect_contains(catalog_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1299: selection toolbox batch dispatch catalog JSON should expose dispatch admission state");
    expect_contains(catalog_process.stdout_text, "\"dryRun\": false",
        "#1299: admitted selection toolbox batch dispatch catalog JSON should expose non-dry-run state");
    expect_contains(catalog_process.stdout_text, "\"mutatesAsset\": true",
        "#1299: admitted selection toolbox batch dispatch catalog JSON should expose mutation intent");
    expect_contains(catalog_process.stdout_text,
        "\"dispatchReadyItemIds\": [\"label\", \"textbox\", \"editbox\", \"commandbutton\"",
        "#1381: selection toolbox batch dispatch catalog JSON should summarize dispatch-ready visual items");
    expect_contains(catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1381: admitted selection toolbox batch dispatch catalog JSON should summarize empty blocked item ids");
    expect_contains(catalog_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1381: admitted selection toolbox batch dispatch catalog JSON should summarize empty blocked dispatch errors");
    expect(visual_object_count(form_path) == before_count,
        "#1299: selection toolbox batch dispatch catalog host command should not mutate the visual asset");

    const auto dry_run_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--selection-context", "visual_object",
            "--parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(dry_run_catalog_process.exit_code == 0,
        "#1299: non-admitted selection toolbox batch dispatch catalog JSON should return a catalog");
    expect_contains(dry_run_catalog_process.stdout_text, "\"batchPlanOk\": true",
        "#1299: non-admitted selection toolbox batch dispatch catalog JSON should preserve batch plans");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchOk\": false",
        "#1299: non-admitted selection toolbox batch dispatch catalog JSON should expose dispatch failure");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchCount\": 0",
        "#1299: non-admitted selection toolbox batch dispatch catalog JSON should expose zero dispatches");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1381: non-admitted selection toolbox batch dispatch catalog JSON should summarize empty ready item ids");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1381: non-admitted selection toolbox batch dispatch catalog JSON should summarize aggregate blocked state without fabricated item ids");
    expect_contains(dry_run_catalog_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"A toolbox batch create dispatch request requires an admitted non-dry-run create operation.\"",
        "#1381: non-admitted selection toolbox batch dispatch catalog JSON should summarize blocked dispatch errors");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatch\": null",
        "#1299: non-admitted selection toolbox batch dispatch catalog JSON should not expose stale dispatch plans");
    expect_contains(dry_run_catalog_process.stdout_text,
        "A toolbox batch create dispatch request requires an admitted non-dry-run create operation.",
        "#1299: non-admitted selection toolbox batch dispatch catalog JSON should expose dispatch errors");
    expect_not_contains(dry_run_catalog_process.stdout_text, "\"--toolbox-create-batch\"",
        "#1299: non-admitted selection toolbox batch dispatch catalog JSON should not expose stale arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1299: non-admitted selection toolbox batch dispatch catalog host command should not mutate the asset");

    const auto report_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--selection-context", "report_expression",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Selection Batch Dispatch Catalog",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(report_catalog_process.exit_code == 0,
        "#1299: report selection toolbox batch dispatch catalog JSON command should exit successfully");
    expect_contains(report_catalog_process.stdout_text, "\"selectionToolboxCreateBatchDispatchCatalog\": {",
        "#2114: report selection toolbox batch dispatch catalog JSON should expose catalog objects");
    expect_contains(report_catalog_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1299: report selection toolbox batch dispatch catalog JSON should expose report selections");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1299: report selection toolbox batch dispatch catalog JSON should expose report contexts");
    expect_contains(report_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#2114: report selection toolbox batch dispatch catalog JSON should expose launch state");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchCount\": 1",
        "#2114: report selection toolbox batch dispatch catalog JSON should expose one report batch dispatch");
    expect_contains(report_catalog_process.stdout_text, "\"errorCount\": 0",
        "#2114: report selection toolbox batch dispatch catalog JSON should expose zero catalog errors");
    expect_contains(report_catalog_process.stdout_text, "\"batchPlanOk\": true",
        "#2114: report selection toolbox batch dispatch catalog JSON should expose batch plan state");
    expect_contains(report_catalog_process.stdout_text, "\"batchPlan\": {",
        "#2114: report selection toolbox batch dispatch catalog JSON should expose nested batch plans");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchOk\": true",
        "#2114: report selection toolbox batch dispatch catalog JSON should expose dispatch state");
    expect_contains(report_catalog_process.stdout_text, "\"dispatch\": {",
        "#2114: report selection toolbox batch dispatch catalog JSON should expose nested dispatch plans");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1299: report selection toolbox batch dispatch catalog JSON should include label plans");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"",
        "#1381: report selection toolbox batch dispatch catalog JSON should summarize dispatch-ready report items");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1381: report selection toolbox batch dispatch catalog JSON should summarize empty blocked item ids");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1381: report selection toolbox batch dispatch catalog JSON should summarize empty blocked dispatch errors");
    expect_contains(report_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2114: report selection toolbox batch dispatch catalog JSON should preserve report parent payloads");
    expect_contains(report_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2153: report selection toolbox batch dispatch catalog JSON should expose caller field names");
    expect_contains(report_catalog_process.stdout_text, "\"propertyValue\": \"Report Selection Batch Dispatch Catalog\"",
        "#2153: report selection toolbox batch dispatch catalog JSON should expose caller field values");
    expect_contains(report_catalog_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#1299: report selection toolbox batch dispatch catalog JSON should preserve report dispatch context");
    expect_contains(report_catalog_process.stdout_text,
        "\"--field-value\", \"CAPTION=Report Selection Batch Dispatch Catalog\"",
        "#2153: report selection toolbox batch dispatch catalog JSON should preserve report dispatch field arguments");
    expect_contains(report_catalog_process.stdout_text, "\"dryRun\": false",
        "#2114: report selection toolbox batch dispatch catalog JSON should expose non-dry-run dispatch state");
    expect_contains(report_catalog_process.stdout_text, "\"mutatesAsset\": true",
        "#2114: report selection toolbox batch dispatch catalog JSON should expose mutation intent");
    expect_not_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1299: report selection toolbox batch dispatch catalog JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#1299: report selection toolbox batch dispatch catalog host command should not mutate the asset");

    const auto label_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--selection-context", "label_expression",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Label Selection Batch Dispatch Catalog",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(label_catalog_process.exit_code == 0,
        "#2088: label selection toolbox batch dispatch catalog JSON command should exit successfully");
    expect_contains(label_catalog_process.stdout_text, "\"selectionToolboxCreateBatchDispatchCatalog\": {",
        "#2128: label selection toolbox batch dispatch catalog JSON should expose catalog objects");
    expect_contains(label_catalog_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#2088: label selection toolbox batch dispatch catalog JSON should expose label selections");
    expect_contains(label_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2088: label selection toolbox batch dispatch catalog JSON should expose report contexts");
    expect_contains(label_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#2128: label selection toolbox batch dispatch catalog JSON should expose launch state");
    expect_contains(label_catalog_process.stdout_text, "\"dispatchCount\": 1",
        "#2128: label selection toolbox batch dispatch catalog JSON should expose one label batch dispatch");
    expect_contains(label_catalog_process.stdout_text, "\"errorCount\": 0",
        "#2128: label selection toolbox batch dispatch catalog JSON should expose zero catalog errors");
    expect_contains(label_catalog_process.stdout_text, "\"batchPlanOk\": true",
        "#2128: label selection toolbox batch dispatch catalog JSON should expose batch plan state");
    expect_contains(label_catalog_process.stdout_text, "\"batchPlan\": {",
        "#2128: label selection toolbox batch dispatch catalog JSON should expose nested batch plans");
    expect_contains(label_catalog_process.stdout_text, "\"dispatchOk\": true",
        "#2128: label selection toolbox batch dispatch catalog JSON should expose dispatch state");
    expect_contains(label_catalog_process.stdout_text, "\"dispatch\": {",
        "#2128: label selection toolbox batch dispatch catalog JSON should expose nested dispatch plans");
    expect_contains(label_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2088: label selection toolbox batch dispatch catalog JSON should include label plans");
    expect_contains(label_catalog_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"",
        "#2088: label selection toolbox batch dispatch catalog JSON should summarize dispatch-ready label items");
    expect_contains(label_catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#2088: label selection toolbox batch dispatch catalog JSON should summarize empty blocked item ids");
    expect_contains(label_catalog_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#2088: label selection toolbox batch dispatch catalog JSON should summarize empty blocked dispatch errors");
    expect_contains(label_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2128: label selection toolbox batch dispatch catalog JSON should preserve label parent payloads");
    expect_contains(label_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2155: label selection toolbox batch dispatch catalog JSON should expose caller field names");
    expect_contains(label_catalog_process.stdout_text, "\"propertyValue\": \"Label Selection Batch Dispatch Catalog\"",
        "#2155: label selection toolbox batch dispatch catalog JSON should expose caller field values");
    expect_contains(label_catalog_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#2088: label selection toolbox batch dispatch catalog JSON should preserve report dispatch context");
    expect_contains(label_catalog_process.stdout_text,
        "\"--field-value\", \"CAPTION=Label Selection Batch Dispatch Catalog\"",
        "#2155: label selection toolbox batch dispatch catalog JSON should preserve label dispatch field arguments");
    expect_contains(label_catalog_process.stdout_text, "\"dryRun\": false",
        "#2128: label selection toolbox batch dispatch catalog JSON should expose non-dry-run dispatch state");
    expect_contains(label_catalog_process.stdout_text, "\"mutatesAsset\": true",
        "#2128: label selection toolbox batch dispatch catalog JSON should expose mutation intent");
    expect_not_contains(label_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#2088: label selection toolbox batch dispatch catalog JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2128: label selection toolbox batch dispatch catalog host command should not mutate the asset");

    const auto unsupported_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--selection-context", "menu_item",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(unsupported_selection_process.exit_code == 4,
        "#1299: selection toolbox batch dispatch catalog JSON should reject unsupported selections");
    expect_contains(unsupported_selection_process.stdout_text,
        "\"selectionToolboxCreateBatchDispatchCatalog\": null",
        "#1299: unsupported selection toolbox batch dispatch catalog JSON should suppress stale payloads");
    expect_contains(unsupported_selection_process.stdout_text,
        "A selection-context toolbox object batch creation dispatch catalog request requires a toolbox palette.",
        "#1299: unsupported selection toolbox batch dispatch catalog JSON should report palette errors");
    expect(visual_object_count(form_path) == before_count,
        "#1299: unsupported selection toolbox batch dispatch catalog host command should not mutate the asset");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1299: selection toolbox batch dispatch catalog JSON should reject missing paths");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1299: missing path selection toolbox batch dispatch catalog JSON should report parser errors");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1299: selection toolbox batch dispatch catalog JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1299: missing selection toolbox batch dispatch catalog JSON should report parser errors");

    const auto unknown_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_selection_process.exit_code == 2,
        "#1299: selection toolbox batch dispatch catalog JSON should reject unknown selections");
    expect_contains(unknown_selection_process.stdout_text, "Unknown selection context token: unknown",
        "#1299: unknown selection toolbox batch dispatch catalog JSON should report parser errors");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--selection-context", "visual_object",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1299: selection toolbox batch dispatch catalog JSON should reject invalid admission tokens");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#1299: invalid selection toolbox batch dispatch catalog admission tokens should report parser errors");

    const auto invalid_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--selection-context", "visual_object",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(invalid_field_process.exit_code == 2,
        "#1299: selection toolbox batch dispatch catalog JSON should reject malformed field values");
    expect_contains(invalid_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1299: malformed selection toolbox batch dispatch catalog field values should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch-dispatch-catalog",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1299: selection toolbox batch dispatch catalog JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-toolbox-create-batch-dispatch-catalog option: --toolbox-context",
        "#1299: unknown selection toolbox batch dispatch catalog options should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1299: rejected selection toolbox batch dispatch catalog host commands should not mutate the visual asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
