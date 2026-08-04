// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

// Toolbox catalog and catalog-dispatch coverage.
void test_studio_host_json_plans_toolbox_object_creation_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_plan_catalog_json_tests";
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
            "--toolbox-create-plan-catalog",
            "--toolbox-context", "form",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Planned",
            "--json"
        },
        temp_root);
    expect(catalog_process.exit_code == 0,
        "#1244: toolbox-create-plan-catalog JSON command should exit successfully");
    expect_contains(catalog_process.stdout_text, "\"toolboxCreatePlanCatalog\": {",
        "#1244: toolbox-create-plan-catalog JSON should expose a catalog object");
    expect_contains(catalog_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1244: toolbox-create-plan-catalog JSON should expose toolbox contexts");
    expect_contains(catalog_process.stdout_text, "\"itemCount\": ",
        "#1244: toolbox-create-plan-catalog JSON should expose item counts");
    expect_contains(catalog_process.stdout_text, "\"planCount\": ",
        "#1244: toolbox-create-plan-catalog JSON should expose plan counts");
    expect_contains(catalog_process.stdout_text, "\"errorCount\": 0",
        "#1244: toolbox-create-plan-catalog JSON should expose zero error counts");
    expect_contains(catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1244: form toolbox-create-plan-catalog JSON should include textbox plans");
    expect_contains(catalog_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1244: form toolbox-create-plan-catalog JSON should include command button plans");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1244: form toolbox-create-plan-catalog JSON should expose generated textbox names");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"cmd1\"",
        "#1244: form toolbox-create-plan-catalog JSON should expose generated command names");
    expect_contains(catalog_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1244: toolbox-create-plan-catalog JSON should expose planned parent names");
    expect_contains(catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1244: toolbox-create-plan-catalog JSON should expose caller field values");
    expect_contains(catalog_process.stdout_text, "\"dryRun\": true",
        "#1244: toolbox-create-plan-catalog JSON should expose dry-run state");
    expect_contains(catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#1244: toolbox-create-plan-catalog JSON should remain non-mutating");
    expect_contains(catalog_process.stdout_text,
        "\"planReadyItemIds\": [\"label\", \"textbox\", \"editbox\", \"commandbutton\"",
        "#1374: toolbox-create-plan-catalog JSON should summarize plan-ready form items");
    expect_contains(catalog_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1374: toolbox-create-plan-catalog JSON should summarize empty blocked item ids");
    expect_contains(catalog_process.stdout_text, "\"planBlockedErrors\": []",
        "#1374: toolbox-create-plan-catalog JSON should summarize empty blocked plan errors");
    expect(visual_object_count(form_path) == before_count,
        "#1244: toolbox-create-plan-catalog host command should not mutate the visual asset");

    const auto report_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-plan-catalog",
            "--toolbox-context", "report",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Plan Catalog",
            "--json"
        },
        temp_root);
    expect(report_catalog_process.exit_code == 0,
        "#1244: report toolbox-create-plan-catalog JSON command should exit successfully");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxCreatePlanCatalog\": {",
        "#2107: report toolbox-create-plan-catalog JSON should expose a catalog object");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1244: report toolbox-create-plan-catalog JSON should expose report contexts");
    expect_contains(report_catalog_process.stdout_text, "\"planCount\": ",
        "#2107: report toolbox-create-plan-catalog JSON should expose report plan counts");
    expect_contains(report_catalog_process.stdout_text, "\"errorCount\": 0",
        "#2107: report toolbox-create-plan-catalog JSON should expose zero catalog errors");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1244: report toolbox-create-plan-catalog JSON should include label plans");
    expect_contains(report_catalog_process.stdout_text, "\"planReadyItemIds\": [\"label\"",
        "#1374: report toolbox-create-plan-catalog JSON should summarize plan-ready report items");
    expect_contains(report_catalog_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1374: report toolbox-create-plan-catalog JSON should summarize empty blocked item ids");
    expect_contains(report_catalog_process.stdout_text, "\"planBlockedErrors\": []",
        "#1374: report toolbox-create-plan-catalog JSON should summarize empty blocked plan errors");
    expect_contains(report_catalog_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#1244: report toolbox-create-plan-catalog JSON should expose generated label names");
    expect_contains(report_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2107: report toolbox-create-plan-catalog JSON should preserve report parent payloads");
    expect_contains(report_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2143: report toolbox-create-plan-catalog JSON should expose caller field names");
    expect_contains(report_catalog_process.stdout_text, "\"propertyValue\": \"Report Plan Catalog\"",
        "#2143: report toolbox-create-plan-catalog JSON should expose caller field values");
    expect_contains(report_catalog_process.stdout_text, "\"dryRun\": true",
        "#2107: report toolbox-create-plan-catalog JSON should remain dry-run");
    expect_contains(report_catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#2107: report toolbox-create-plan-catalog JSON should remain non-mutating");
    expect_not_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1244: report toolbox-create-plan-catalog JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#1244: report toolbox-create-plan-catalog host command should not mutate the visual asset");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-plan-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1244: toolbox-create-plan-catalog JSON should reject missing contexts");
    expect_contains(missing_context_process.stdout_text, "No toolbox context was provided.",
        "#1244: missing toolbox-create-plan-catalog context JSON should report parser errors");

    const auto invalid_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-plan-catalog",
            "--toolbox-context", "form",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(invalid_field_process.exit_code == 2,
        "#1244: toolbox-create-plan-catalog JSON should reject malformed field values");
    expect_contains(invalid_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1244: malformed toolbox-create-plan-catalog field values should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1244: rejected toolbox-create-plan-catalog host commands should not mutate the visual asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
void test_studio_host_json_plans_selection_toolbox_object_creation_catalog(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_create_plan_catalog_json_tests";
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
            "--selection-toolbox-create-plan-catalog",
            "--selection-context", "visual_object",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Selection Planned",
            "--json"
        },
        temp_root);
    expect(visual_catalog_process.exit_code == 0,
        "#1293: selection toolbox create-plan catalog JSON command should exit successfully");
    expect_contains(visual_catalog_process.stdout_text, "\"selectionToolboxCreatePlanCatalog\": {",
        "#1293: selection toolbox create-plan catalog JSON should expose a catalog object");
    expect_contains(visual_catalog_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1293: selection toolbox create-plan catalog JSON should expose selected Studio contexts");
    expect_contains(visual_catalog_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1293: visual selection toolbox create-plan catalog JSON should resolve form contexts");
    expect_contains(visual_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#1293: selection toolbox create-plan catalog JSON should expose launch state");
    expect_contains(visual_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1293: visual selection toolbox create-plan catalog JSON should include textbox plans");
    expect_contains(visual_catalog_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1293: visual selection toolbox create-plan catalog JSON should expose generated names");
    expect_contains(visual_catalog_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1293: selection toolbox create-plan catalog JSON should expose planned parents");
    expect_contains(visual_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1293: selection toolbox create-plan catalog JSON should expose caller field values");
    expect_contains(visual_catalog_process.stdout_text, "\"dryRun\": true",
        "#1293: selection toolbox create-plan catalog JSON should expose dry-run state");
    expect_contains(visual_catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#1293: selection toolbox create-plan catalog JSON should remain non-mutating");
    expect_contains(visual_catalog_process.stdout_text,
        "\"planReadyItemIds\": [\"label\", \"textbox\", \"editbox\", \"commandbutton\"",
        "#1375: selection toolbox create-plan catalog JSON should summarize plan-ready visual items");
    expect_contains(visual_catalog_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1375: selection toolbox create-plan catalog JSON should summarize empty blocked item ids");
    expect_contains(visual_catalog_process.stdout_text, "\"planBlockedErrors\": []",
        "#1375: selection toolbox create-plan catalog JSON should summarize empty blocked plan errors");
    expect(visual_object_count(form_path) == before_count,
        "#1293: visual selection toolbox create-plan catalog host command should not mutate assets");

    const auto report_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan-catalog",
            "--selection-context", "report_expression",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Selection Plan Catalog",
            "--json"
        },
        temp_root);
    expect(report_catalog_process.exit_code == 0,
        "#1293: report selection toolbox create-plan catalog JSON command should exit successfully");
    expect_contains(report_catalog_process.stdout_text, "\"selectionToolboxCreatePlanCatalog\": {",
        "#2111: report selection toolbox create-plan catalog JSON should expose a catalog object");
    expect_contains(report_catalog_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1293: report selection toolbox create-plan catalog JSON should expose report selections");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1293: report selection toolbox create-plan catalog JSON should resolve report contexts");
    expect_contains(report_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#2111: report selection toolbox create-plan catalog JSON should expose launch state");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1293: report selection toolbox create-plan catalog JSON should include label plans");
    expect_contains(report_catalog_process.stdout_text, "\"planReadyItemIds\": [\"label\"",
        "#1375: report selection toolbox create-plan catalog JSON should summarize plan-ready report items");
    expect_contains(report_catalog_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1375: report selection toolbox create-plan catalog JSON should summarize empty blocked item ids");
    expect_contains(report_catalog_process.stdout_text, "\"planBlockedErrors\": []",
        "#1375: report selection toolbox create-plan catalog JSON should summarize empty blocked plan errors");
    expect_contains(report_catalog_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#1293: report selection toolbox create-plan catalog JSON should expose generated labels");
    expect_contains(report_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2111: report selection toolbox create-plan catalog JSON should preserve report parent payloads");
    expect_contains(report_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2148: report selection toolbox create-plan catalog JSON should expose caller field names");
    expect_contains(report_catalog_process.stdout_text, "\"propertyValue\": \"Report Selection Plan Catalog\"",
        "#2148: report selection toolbox create-plan catalog JSON should expose caller field values");
    expect_contains(report_catalog_process.stdout_text, "\"dryRun\": true",
        "#2111: report selection toolbox create-plan catalog JSON should remain dry-run");
    expect_contains(report_catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#2111: report selection toolbox create-plan catalog JSON should remain non-mutating");
    expect_not_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1293: report selection toolbox create-plan catalog JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#1293: report selection toolbox create-plan catalog host command should not mutate assets");

    const auto label_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan-catalog",
            "--selection-context", "label_expression",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Label Selection Plan Catalog",
            "--json"
        },
        temp_root);
    expect(label_catalog_process.exit_code == 0,
        "#2080: label selection toolbox create-plan catalog JSON command should exit successfully");
    expect_contains(label_catalog_process.stdout_text, "\"selectionToolboxCreatePlanCatalog\": {",
        "#2125: label selection toolbox create-plan catalog JSON should expose a catalog object");
    expect_contains(label_catalog_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#2080: label selection toolbox create-plan catalog JSON should expose label selections");
    expect_contains(label_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2080: label selection toolbox create-plan catalog JSON should resolve report contexts");
    expect_contains(label_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#2125: label selection toolbox create-plan catalog JSON should expose launch state");
    expect_contains(label_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2080: label selection toolbox create-plan catalog JSON should include label plans");
    expect_contains(label_catalog_process.stdout_text, "\"planReadyItemIds\": [\"label\"",
        "#2080: label selection toolbox create-plan catalog JSON should summarize plan-ready label items");
    expect_contains(label_catalog_process.stdout_text, "\"planBlockedItemIds\": []",
        "#2080: label selection toolbox create-plan catalog JSON should summarize empty blocked item ids");
    expect_contains(label_catalog_process.stdout_text, "\"planBlockedErrors\": []",
        "#2080: label selection toolbox create-plan catalog JSON should summarize empty blocked plan errors");
    expect_contains(label_catalog_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2080: label selection toolbox create-plan catalog JSON should expose generated labels");
    expect_contains(label_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2125: label selection toolbox create-plan catalog JSON should preserve label parent payloads");
    expect_contains(label_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2150: label selection toolbox create-plan catalog JSON should expose caller field names");
    expect_contains(label_catalog_process.stdout_text, "\"propertyValue\": \"Label Selection Plan Catalog\"",
        "#2150: label selection toolbox create-plan catalog JSON should expose caller field values");
    expect_contains(label_catalog_process.stdout_text, "\"dryRun\": true",
        "#2125: label selection toolbox create-plan catalog JSON should remain dry-run");
    expect_contains(label_catalog_process.stdout_text, "\"mutatesAsset\": false",
        "#2125: label selection toolbox create-plan catalog JSON should remain non-mutating");
    expect_not_contains(label_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#2080: label selection toolbox create-plan catalog JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2125: label selection toolbox create-plan catalog host command should not mutate assets");

    const auto unsupported_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan-catalog",
            "--selection-context", "menu_item",
            "--json"
        },
        temp_root);
    expect(unsupported_catalog_process.exit_code == 4,
        "#1293: selection toolbox create-plan catalog JSON should reject unsupported selections");
    expect_contains(unsupported_catalog_process.stdout_text, "\"selectionToolboxCreatePlanCatalog\": null",
        "#1293: unsupported selection toolbox create-plan catalog JSON should omit catalog objects");
    expect_contains(unsupported_catalog_process.stdout_text,
        "A selection-context toolbox object creation catalog request requires a toolbox palette.",
        "#1293: unsupported selection toolbox create-plan catalog JSON should report planner errors");
    expect(visual_object_count(form_path) == before_count,
        "#1293: unsupported selection toolbox create-plan catalog host command should not mutate assets");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan-catalog",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1293: selection toolbox create-plan catalog JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1293: missing selection toolbox create-plan catalog JSON should report parser errors");

    const auto unknown_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan-catalog",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_selection_process.exit_code == 2,
        "#1293: selection toolbox create-plan catalog JSON should reject unknown selections");
    expect_contains(unknown_selection_process.stdout_text, "Unknown selection context token: unknown",
        "#1293: unknown selection toolbox create-plan catalog JSON should report parser errors");

    const auto invalid_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan-catalog",
            "--selection-context", "visual_object",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(invalid_field_process.exit_code == 2,
        "#1293: selection toolbox create-plan catalog JSON should reject malformed field values");
    expect_contains(invalid_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1293: malformed selection toolbox create-plan catalog field values should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan-catalog",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1293: selection toolbox create-plan catalog JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-toolbox-create-plan-catalog option: --toolbox-context",
        "#1293: unknown selection toolbox create-plan catalog options should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1293: rejected selection toolbox create-plan catalog host commands should not mutate assets");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_toolbox_object_creation_dispatch_catalog(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_dispatch_catalog_json_tests";
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
            "--toolbox-create-dispatch-catalog",
            "--toolbox-context", "form",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Dispatch Catalog",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(catalog_process.exit_code == 0,
        "#1254: toolbox-create-dispatch-catalog JSON command should exit successfully");
    expect_contains(catalog_process.stdout_text, "\"toolboxCreateDispatchCatalog\": {",
        "#1254: toolbox-create-dispatch-catalog JSON should expose a catalog object");
    expect_contains(catalog_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1254: toolbox-create-dispatch-catalog JSON should expose requested contexts");
    expect_contains(catalog_process.stdout_text, "\"itemCount\": ",
        "#1254: toolbox-create-dispatch-catalog JSON should expose item counts");
    expect_contains(catalog_process.stdout_text, "\"dispatchCount\": ",
        "#1254: toolbox-create-dispatch-catalog JSON should expose dispatch counts");
    expect_contains(catalog_process.stdout_text, "\"errorCount\": 0",
        "#1254: admitted toolbox-create-dispatch-catalog JSON should expose zero errors");
    expect_contains(catalog_process.stdout_text, "\"dryRun\": false",
        "#1254: admitted toolbox-create-dispatch-catalog JSON should expose non-dry-run dispatch state");
    expect_contains(catalog_process.stdout_text, "\"mutatesAsset\": true",
        "#1254: admitted toolbox-create-dispatch-catalog JSON should expose mutation intent");
    expect_contains(catalog_process.stdout_text,
        "\"dispatchReadyItemIds\": [\"label\", \"textbox\", \"editbox\", \"commandbutton\"",
        "#1376: toolbox-create-dispatch-catalog JSON should summarize dispatch-ready form items");
    expect_contains(catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1376: admitted toolbox-create-dispatch-catalog JSON should summarize empty blocked item ids");
    expect_contains(catalog_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1376: admitted toolbox-create-dispatch-catalog JSON should summarize empty blocked dispatch errors");
    expect_contains(catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1254: toolbox-create-dispatch-catalog JSON should include textbox entries");
    expect_contains(catalog_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1254: toolbox-create-dispatch-catalog JSON should include command button entries");
    expect_contains(catalog_process.stdout_text, "\"createPlanOk\": true",
        "#1254: toolbox-create-dispatch-catalog JSON should expose create plan state");
    expect_contains(catalog_process.stdout_text, "\"dispatchOk\": true",
        "#1254: toolbox-create-dispatch-catalog JSON should expose dispatch state");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1254: toolbox-create-dispatch-catalog JSON should expose generated textbox names");
    expect_contains(catalog_process.stdout_text, "\"objectName\": \"cmd1\"",
        "#1254: toolbox-create-dispatch-catalog JSON should expose generated command names");
    expect_contains(catalog_process.stdout_text, "\"dispatchArguments\": [",
        "#1254: toolbox-create-dispatch-catalog JSON should expose dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"--toolbox-create\", \"textbox\"",
        "#1254: toolbox-create-dispatch-catalog JSON should emit textbox dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"--toolbox-create\", \"commandbutton\"",
        "#1254: toolbox-create-dispatch-catalog JSON should emit command dispatch arguments");
    expect_contains(catalog_process.stdout_text, "\"--field-value\", \"CAPTION=Dispatch Catalog\"",
        "#1254: toolbox-create-dispatch-catalog JSON should preserve shared field values");
    expect_contains(catalog_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1254: toolbox-create-dispatch-catalog JSON should expose dispatch admission state");
    expect(visual_object_count(form_path) == before_count,
        "#1254: toolbox-create-dispatch-catalog host command should not mutate the visual asset");

    const auto dry_run_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-catalog",
            "--toolbox-context", "form",
            "--parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(dry_run_catalog_process.exit_code == 0,
        "#1254: non-admitted toolbox-create-dispatch-catalog JSON should return a catalog");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchCount\": 0",
        "#1254: non-admitted toolbox-create-dispatch-catalog JSON should expose zero dispatches");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1376: non-admitted toolbox-create-dispatch-catalog JSON should summarize empty ready item ids");
    expect_contains(dry_run_catalog_process.stdout_text,
        "\"dispatchBlockedItemIds\": [\"label\", \"textbox\", \"editbox\", \"commandbutton\"",
        "#1376: non-admitted toolbox-create-dispatch-catalog JSON should summarize blocked form item ids");
    expect_contains(dry_run_catalog_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"A toolbox create dispatch request requires an admitted non-dry-run create operation.\"",
        "#1376: non-admitted toolbox-create-dispatch-catalog JSON should summarize blocked dispatch errors");
    expect_contains(dry_run_catalog_process.stdout_text,
        "A toolbox create dispatch request requires an admitted non-dry-run create operation.",
        "#1254: non-admitted toolbox-create-dispatch-catalog JSON should expose per-item errors");
    expect_not_contains(dry_run_catalog_process.stdout_text, "\"--toolbox-create\"",
        "#1254: non-admitted toolbox-create-dispatch-catalog JSON should not expose stale dispatch arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1254: non-admitted toolbox-create-dispatch-catalog host command should not mutate the asset");

    const auto report_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-catalog",
            "--toolbox-context", "report",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Dispatch Catalog",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(report_catalog_process.exit_code == 0,
        "#1254: report toolbox-create-dispatch-catalog JSON command should exit successfully");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxCreateDispatchCatalog\": {",
        "#2108: report toolbox-create-dispatch-catalog JSON should expose a catalog object");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1254: report toolbox-create-dispatch-catalog JSON should expose report contexts");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchCount\": ",
        "#2108: report toolbox-create-dispatch-catalog JSON should expose report dispatch counts");
    expect_contains(report_catalog_process.stdout_text, "\"errorCount\": 0",
        "#2108: report toolbox-create-dispatch-catalog JSON should expose zero catalog errors");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1254: report toolbox-create-dispatch-catalog JSON should include label dispatches");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"",
        "#1376: report toolbox-create-dispatch-catalog JSON should summarize dispatch-ready report items");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1376: report toolbox-create-dispatch-catalog JSON should summarize empty blocked item ids");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1376: report toolbox-create-dispatch-catalog JSON should summarize empty blocked dispatch errors");
    expect_contains(report_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2108: report toolbox-create-dispatch-catalog JSON should preserve report parent payloads");
    expect_contains(report_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2145: report toolbox-create-dispatch-catalog JSON should expose caller field names");
    expect_contains(report_catalog_process.stdout_text, "\"propertyValue\": \"Report Dispatch Catalog\"",
        "#2145: report toolbox-create-dispatch-catalog JSON should expose caller field values");
    expect_contains(report_catalog_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#1254: report toolbox-create-dispatch-catalog JSON should preserve report dispatch context");
    expect_contains(report_catalog_process.stdout_text, "\"--field-value\", \"CAPTION=Report Dispatch Catalog\"",
        "#2145: report toolbox-create-dispatch-catalog JSON should preserve report dispatch field arguments");
    expect_contains(report_catalog_process.stdout_text, "\"dryRun\": false",
        "#2108: report toolbox-create-dispatch-catalog JSON should expose non-dry-run dispatch state");
    expect_contains(report_catalog_process.stdout_text, "\"mutatesAsset\": true",
        "#2108: report toolbox-create-dispatch-catalog JSON should expose mutation intent");
    expect_not_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1254: report toolbox-create-dispatch-catalog JSON should exclude form-only textbox dispatches");
    expect(visual_object_count(form_path) == before_count,
        "#2108: report toolbox-create-dispatch-catalog host command should not mutate assets");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-catalog",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1254: toolbox-create-dispatch-catalog JSON should reject missing contexts");
    expect_contains(missing_context_process.stdout_text, "No toolbox context was provided.",
        "#1254: missing toolbox-create-dispatch-catalog context JSON should report parser errors");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-catalog",
            "--toolbox-context", "form",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1254: toolbox-create-dispatch-catalog JSON should reject invalid admission tokens");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#1254: invalid toolbox-create-dispatch-catalog admission tokens should report parser errors");

    const auto invalid_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-catalog",
            "--toolbox-context", "form",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(invalid_field_process.exit_code == 2,
        "#1254: toolbox-create-dispatch-catalog JSON should reject malformed field values");
    expect_contains(invalid_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1254: malformed toolbox-create-dispatch-catalog field values should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1254: rejected toolbox-create-dispatch-catalog host commands should not mutate the visual asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_plans_selection_toolbox_object_creation_dispatch_catalog(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_create_dispatch_catalog_json_tests";
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
            "--selection-toolbox-create-dispatch-catalog",
            "--selection-context", "visual_object",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Selection Dispatch Catalog",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(visual_catalog_process.exit_code == 0,
        "#1295: selection toolbox create-dispatch catalog JSON command should exit successfully");
    expect_contains(visual_catalog_process.stdout_text, "\"selectionToolboxCreateDispatchCatalog\": {",
        "#1295: selection toolbox create-dispatch catalog JSON should expose catalog objects");
    expect_contains(visual_catalog_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1295: selection toolbox create-dispatch catalog JSON should expose selected Studio contexts");
    expect_contains(visual_catalog_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1295: visual selection toolbox create-dispatch catalog JSON should resolve form contexts");
    expect_contains(visual_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#1295: selection toolbox create-dispatch catalog JSON should expose launch state");
    expect_contains(visual_catalog_process.stdout_text, "\"dispatchCount\": ",
        "#1295: selection toolbox create-dispatch catalog JSON should expose dispatch counts");
    expect_contains(visual_catalog_process.stdout_text, "\"errorCount\": 0",
        "#1295: admitted selection toolbox create-dispatch catalog JSON should expose zero errors");
    expect_contains(visual_catalog_process.stdout_text, "\"dryRun\": false",
        "#1295: admitted selection toolbox create-dispatch catalog JSON should expose non-dry-run state");
    expect_contains(visual_catalog_process.stdout_text, "\"mutatesAsset\": true",
        "#1295: admitted selection toolbox create-dispatch catalog JSON should expose mutation intent");
    expect_contains(visual_catalog_process.stdout_text,
        "\"dispatchReadyItemIds\": [\"label\", \"textbox\", \"editbox\", \"commandbutton\"",
        "#1377: selection toolbox create-dispatch catalog JSON should summarize dispatch-ready visual items");
    expect_contains(visual_catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1377: admitted selection toolbox create-dispatch catalog JSON should summarize empty blocked item ids");
    expect_contains(visual_catalog_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1377: admitted selection toolbox create-dispatch catalog JSON should summarize empty blocked dispatch errors");
    expect_contains(visual_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1295: visual selection toolbox create-dispatch catalog JSON should include textbox entries");
    expect_contains(visual_catalog_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1295: visual selection toolbox create-dispatch catalog JSON should expose generated names");
    expect_contains(visual_catalog_process.stdout_text, "\"dispatchOk\": true",
        "#1295: admitted selection toolbox create-dispatch catalog JSON should expose dispatch state");
    expect_contains(visual_catalog_process.stdout_text, "\"--toolbox-create\", \"textbox\"",
        "#1295: admitted selection toolbox create-dispatch catalog JSON should expose create arguments");
    expect_contains(visual_catalog_process.stdout_text, "\"--field-value\", \"CAPTION=Selection Dispatch Catalog\"",
        "#1295: admitted selection toolbox create-dispatch catalog JSON should preserve field values");
    expect(visual_object_count(form_path) == before_count,
        "#1295: admitted selection toolbox create-dispatch catalog host command should not mutate assets");

    const auto dry_run_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-catalog",
            "--selection-context", "visual_object",
            "--parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(dry_run_catalog_process.exit_code == 0,
        "#1295: non-admitted selection toolbox create-dispatch catalog JSON should return catalogs");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchCount\": 0",
        "#1295: non-admitted selection toolbox create-dispatch catalog JSON should expose zero dispatches");
    expect_contains(dry_run_catalog_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1377: non-admitted selection toolbox create-dispatch catalog JSON should summarize empty ready item ids");
    expect_contains(dry_run_catalog_process.stdout_text,
        "\"dispatchBlockedItemIds\": [\"label\", \"textbox\", \"editbox\", \"commandbutton\"",
        "#1377: non-admitted selection toolbox create-dispatch catalog JSON should summarize blocked visual item ids");
    expect_contains(dry_run_catalog_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"A toolbox create dispatch request requires an admitted non-dry-run create operation.\"",
        "#1377: non-admitted selection toolbox create-dispatch catalog JSON should summarize blocked dispatch errors");
    expect_contains(dry_run_catalog_process.stdout_text,
        "A toolbox create dispatch request requires an admitted non-dry-run create operation.",
        "#1295: non-admitted selection toolbox create-dispatch catalog JSON should expose dispatch errors");
    expect_not_contains(dry_run_catalog_process.stdout_text, "\"--toolbox-create\"",
        "#1295: non-admitted selection toolbox create-dispatch catalog JSON should omit stale arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1295: non-admitted selection toolbox create-dispatch catalog host command should not mutate assets");

    const auto report_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-catalog",
            "--selection-context", "report_expression",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Selection Dispatch Catalog",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(report_catalog_process.exit_code == 0,
        "#1295: report selection toolbox create-dispatch catalog JSON command should exit successfully");
    expect_contains(report_catalog_process.stdout_text, "\"selectionToolboxCreateDispatchCatalog\": {",
        "#2112: report selection toolbox create-dispatch catalog JSON should expose a catalog object");
    expect_contains(report_catalog_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1295: report selection toolbox create-dispatch catalog JSON should expose report selections");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1295: report selection toolbox create-dispatch catalog JSON should resolve report contexts");
    expect_contains(report_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#2112: report selection toolbox create-dispatch catalog JSON should expose launch state");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchCount\": ",
        "#2112: report selection toolbox create-dispatch catalog JSON should expose dispatch counts");
    expect_contains(report_catalog_process.stdout_text, "\"errorCount\": 0",
        "#2112: report selection toolbox create-dispatch catalog JSON should expose zero catalog errors");
    expect_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1295: report selection toolbox create-dispatch catalog JSON should include label dispatches");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"",
        "#1377: report selection toolbox create-dispatch catalog JSON should summarize dispatch-ready report items");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1377: report selection toolbox create-dispatch catalog JSON should summarize empty blocked item ids");
    expect_contains(report_catalog_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1377: report selection toolbox create-dispatch catalog JSON should summarize empty blocked dispatch errors");
    expect_contains(report_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2112: report selection toolbox create-dispatch catalog JSON should preserve report parent payloads");
    expect_contains(report_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2149: report selection toolbox create-dispatch catalog JSON should expose caller field names");
    expect_contains(report_catalog_process.stdout_text, "\"propertyValue\": \"Report Selection Dispatch Catalog\"",
        "#2149: report selection toolbox create-dispatch catalog JSON should expose caller field values");
    expect_contains(report_catalog_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#1295: report selection toolbox create-dispatch catalog JSON should preserve report contexts");
    expect_contains(report_catalog_process.stdout_text,
        "\"--field-value\", \"CAPTION=Report Selection Dispatch Catalog\"",
        "#2149: report selection toolbox create-dispatch catalog JSON should preserve report dispatch field arguments");
    expect_contains(report_catalog_process.stdout_text, "\"dryRun\": false",
        "#2112: report selection toolbox create-dispatch catalog JSON should expose non-dry-run dispatch state");
    expect_contains(report_catalog_process.stdout_text, "\"mutatesAsset\": true",
        "#2112: report selection toolbox create-dispatch catalog JSON should expose mutation intent");
    expect_not_contains(report_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1295: report selection toolbox create-dispatch catalog JSON should exclude form-only textbox entries");
    expect(visual_object_count(form_path) == before_count,
        "#1295: report selection toolbox create-dispatch catalog host command should not mutate assets");

    const auto label_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-catalog",
            "--selection-context", "label_expression",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Label Selection Dispatch Catalog",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(label_catalog_process.exit_code == 0,
        "#2083: label selection toolbox create-dispatch catalog JSON command should exit successfully");
    expect_contains(label_catalog_process.stdout_text, "\"selectionToolboxCreateDispatchCatalog\": {",
        "#2126: label selection toolbox create-dispatch catalog JSON should expose a catalog object");
    expect_contains(label_catalog_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#2083: label selection toolbox create-dispatch catalog JSON should expose label selections");
    expect_contains(label_catalog_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2083: label selection toolbox create-dispatch catalog JSON should resolve report contexts");
    expect_contains(label_catalog_process.stdout_text, "\"launchPlanOk\": true",
        "#2126: label selection toolbox create-dispatch catalog JSON should expose launch state");
    expect_contains(label_catalog_process.stdout_text, "\"dispatchCount\": ",
        "#2126: label selection toolbox create-dispatch catalog JSON should expose dispatch counts");
    expect_contains(label_catalog_process.stdout_text, "\"errorCount\": 0",
        "#2126: label selection toolbox create-dispatch catalog JSON should expose zero catalog errors");
    expect_contains(label_catalog_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2083: label selection toolbox create-dispatch catalog JSON should include label dispatches");
    expect_contains(label_catalog_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"",
        "#2083: label selection toolbox create-dispatch catalog JSON should summarize dispatch-ready label items");
    expect_contains(label_catalog_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#2083: label selection toolbox create-dispatch catalog JSON should summarize empty blocked item ids");
    expect_contains(label_catalog_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#2083: label selection toolbox create-dispatch catalog JSON should summarize empty blocked dispatch errors");
    expect_contains(label_catalog_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2126: label selection toolbox create-dispatch catalog JSON should preserve label parent payloads");
    expect_contains(label_catalog_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#2151: label selection toolbox create-dispatch catalog JSON should expose caller field names");
    expect_contains(label_catalog_process.stdout_text, "\"propertyValue\": \"Label Selection Dispatch Catalog\"",
        "#2151: label selection toolbox create-dispatch catalog JSON should expose caller field values");
    expect_contains(label_catalog_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#2083: label selection toolbox create-dispatch catalog JSON should preserve report contexts");
    expect_contains(label_catalog_process.stdout_text,
        "\"--field-value\", \"CAPTION=Label Selection Dispatch Catalog\"",
        "#2151: label selection toolbox create-dispatch catalog JSON should preserve label dispatch field arguments");
    expect_contains(label_catalog_process.stdout_text, "\"dryRun\": false",
        "#2126: label selection toolbox create-dispatch catalog JSON should expose non-dry-run dispatch state");
    expect_contains(label_catalog_process.stdout_text, "\"mutatesAsset\": true",
        "#2126: label selection toolbox create-dispatch catalog JSON should expose mutation intent");
    expect_not_contains(label_catalog_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#2083: label selection toolbox create-dispatch catalog JSON should exclude form-only textbox entries");
    expect(visual_object_count(form_path) == before_count,
        "#2126: label selection toolbox create-dispatch catalog host command should not mutate assets");

    const auto unsupported_catalog_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-catalog",
            "--selection-context", "menu_item",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(unsupported_catalog_process.exit_code == 4,
        "#1295: selection toolbox create-dispatch catalog JSON should reject unsupported selections");
    expect_contains(unsupported_catalog_process.stdout_text, "\"selectionToolboxCreateDispatchCatalog\": null",
        "#1295: unsupported selection toolbox create-dispatch catalog JSON should omit catalog objects");
    expect_contains(unsupported_catalog_process.stdout_text,
        "A selection-context toolbox object creation dispatch catalog request requires a toolbox palette.",
        "#1295: unsupported selection toolbox create-dispatch catalog JSON should report planner errors");
    expect(visual_object_count(form_path) == before_count,
        "#1295: unsupported selection toolbox create-dispatch catalog host command should not mutate assets");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-catalog",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1295: selection toolbox create-dispatch catalog JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1295: missing selection toolbox create-dispatch catalog JSON should report parser errors");

    const auto unknown_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-catalog",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_selection_process.exit_code == 2,
        "#1295: selection toolbox create-dispatch catalog JSON should reject unknown selections");
    expect_contains(unknown_selection_process.stdout_text, "Unknown selection context token: unknown",
        "#1295: unknown selection toolbox create-dispatch catalog JSON should report parser errors");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-catalog",
            "--selection-context", "visual_object",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1295: selection toolbox create-dispatch catalog JSON should reject invalid admission tokens");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#1295: invalid selection toolbox create-dispatch catalog admission tokens should report parser errors");

    const auto invalid_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-catalog",
            "--selection-context", "visual_object",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(invalid_field_process.exit_code == 2,
        "#1295: selection toolbox create-dispatch catalog JSON should reject malformed field values");
    expect_contains(invalid_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1295: malformed selection toolbox create-dispatch catalog field values should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-catalog",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1295: selection toolbox create-dispatch catalog JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-toolbox-create-dispatch-catalog option: --toolbox-context",
        "#1295: unknown selection toolbox create-dispatch catalog options should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1295: rejected selection toolbox create-dispatch catalog host commands should not mutate assets");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
