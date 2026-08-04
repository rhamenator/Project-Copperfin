// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

// Direct toolbox plan and selection coverage.
void test_studio_host_json_plans_toolbox_object_creation(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_plan_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-plan", "textbox",
            "--toolbox-context", "form",
            "--unique-id", "planned-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Customer",
            "--field-value", "PROPERTIES=ControlSource = \"customer.name\"",
            "--json"
        },
        temp_root);

    expect(plan_process.exit_code == 0,
        "#1242: toolbox-create-plan JSON command should exit successfully");
    expect_contains(plan_process.stdout_text, "\"status\": \"ok\"",
        "#1242: successful toolbox-create-plan JSON should report ok status");
    expect_contains(plan_process.stdout_text, "\"toolboxCreatePlan\": {",
        "#1242: toolbox-create-plan JSON should expose a stable result object");
    expect_contains(plan_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1242: toolbox-create-plan JSON should expose toolbox item ids");
    expect_contains(plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#1242: toolbox-create-plan JSON should expose descriptor class names");
    expect_contains(plan_process.stdout_text, "\"baseClassName\": \"TextBox\"",
        "#1242: toolbox-create-plan JSON should expose descriptor base class names");
    expect_contains(plan_process.stdout_text, "\"toolboxContextProvided\": true",
        "#1242: toolbox-create-plan JSON should expose requested toolbox context state");
    expect_contains(plan_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1242: toolbox-create-plan JSON should expose requested toolbox contexts");
    expect_contains(plan_process.stdout_text, "\"planReadyItemIds\": [\"textbox\"]",
        "#1405: toolbox-create-plan JSON should summarize plan-ready item ids");
    expect_contains(plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1405: toolbox-create-plan JSON should expose empty blocked item ids");
    expect_contains(plan_process.stdout_text, "\"planBlockedErrors\": []",
        "#1405: toolbox-create-plan JSON should expose empty blocked plan errors");
    expect_contains(plan_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1242: toolbox-create-plan JSON should expose target record indexes");
    expect_contains(plan_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1242: toolbox-create-plan JSON should expose generated object names");
    expect_contains(plan_process.stdout_text, "\"uniqueId\": \"planned-textbox-guid\"",
        "#1242: toolbox-create-plan JSON should expose planned unique ids");
    expect_contains(plan_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1242: toolbox-create-plan JSON should expose planned parent names");
    expect_contains(plan_process.stdout_text, "\"propertyName\": \"OBJNAME\"",
        "#1242: toolbox-create-plan JSON should expose generated field values");
    expect_contains(plan_process.stdout_text, "\"propertyValue\": \"txt2\"",
        "#1242: toolbox-create-plan JSON should expose generated object-name values");
    expect_contains(plan_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1242: toolbox-create-plan JSON should expose caller direct fields");
    expect_contains(plan_process.stdout_text, "\"propertyName\": \"PROPERTIES\"",
        "#1242: toolbox-create-plan JSON should expose caller memo fields");
    expect_contains(plan_process.stdout_text, "\"dryRun\": true",
        "#1242: toolbox-create-plan JSON should expose dry-run state");
    expect_contains(plan_process.stdout_text, "\"mutatesAsset\": false",
        "#1242: toolbox-create-plan JSON should remain non-mutating");
    expect(visual_object_count(form_path) == before_count,
        "#1242: toolbox-create-plan host command should not mutate the visual asset");

    const auto report_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-plan", "label",
            "--toolbox-context", "report",
            "--unique-id", "direct-report-plan-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Direct Report Plan",
            "--json"
        },
        temp_root);
    expect(report_plan_process.exit_code == 0,
        "#2102: report toolbox-create-plan JSON command should exit successfully");
    expect_contains(report_plan_process.stdout_text, "\"toolboxCreatePlan\": {",
        "#2102: report toolbox-create-plan JSON should expose create plans");
    expect_contains(report_plan_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2102: report toolbox-create-plan JSON should expose label plans");
    expect_contains(report_plan_process.stdout_text, "\"className\": \"Label\"",
        "#2102: report toolbox-create-plan JSON should expose label descriptor metadata");
    expect_contains(report_plan_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2102: report toolbox-create-plan JSON should preserve report contexts");
    expect_contains(report_plan_process.stdout_text, "\"planReadyItemIds\": [\"label\"]",
        "#2102: report toolbox-create-plan JSON should summarize plan-ready report item ids");
    expect_contains(report_plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#2102: report toolbox-create-plan JSON should summarize empty blocked item ids");
    expect_contains(report_plan_process.stdout_text, "\"planBlockedErrors\": []",
        "#2102: report toolbox-create-plan JSON should summarize empty plan errors");
    expect_contains(report_plan_process.stdout_text, "\"targetRecordIndex\": 2",
        "#2102: report toolbox-create-plan JSON should expose target record indexes");
    expect_contains(report_plan_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2102: report toolbox-create-plan JSON should expose generated label names");
    expect_contains(report_plan_process.stdout_text, "\"uniqueId\": \"direct-report-plan-guid\"",
        "#2102: report toolbox-create-plan JSON should preserve label unique ids");
    expect_contains(report_plan_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2102: report toolbox-create-plan JSON should preserve label parent overrides");
    expect_contains(report_plan_process.stdout_text, "\"propertyValue\": \"Direct Report Plan\"",
        "#2102: report toolbox-create-plan JSON should preserve label field values");
    expect_contains(report_plan_process.stdout_text, "\"dryRun\": true",
        "#2102: report toolbox-create-plan JSON should remain a dry-run plan");
    expect_contains(report_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#2102: report toolbox-create-plan JSON should remain non-mutating");
    expect_not_contains(report_plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#2102: report toolbox-create-plan JSON should exclude form-only TextBox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2102: report toolbox-create-plan host command should not mutate assets");

    const auto unknown_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-plan", "missing-toolbox-item",
            "--json"
        },
        temp_root);
    expect(unknown_process.exit_code == 4,
        "#1242: unknown toolbox-create-plan ids should return command failure");
    expect_contains(unknown_process.stdout_text, "\"toolboxCreatePlan\": null",
        "#1242: failed toolbox-create-plan JSON should not expose stale plans");
    expect_contains(unknown_process.stdout_text, "The requested toolbox item was not found.",
        "#1242: failed toolbox-create-plan JSON should expose clean errors");
    expect(visual_object_count(form_path) == before_count,
        "#1242: failed toolbox-create-plan host commands should not mutate the visual asset");

    const auto invalid_context_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-plan", "textbox",
            "--toolbox-context", "missing",
            "--json"
        },
        temp_root);
    expect(invalid_context_process.exit_code == 2,
        "#1242: toolbox-create-plan JSON should reject invalid toolbox contexts");
    expect_contains(invalid_context_process.stdout_text, "Unknown toolbox context token: missing",
        "#1242: invalid toolbox-create-plan context JSON should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1242: invalid toolbox-create-plan host commands should not mutate the visual asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
void test_studio_host_json_plans_selection_toolbox_object_creation(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_create_plan_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan", "textbox",
            "--selection-context", "visual_object",
            "--unique-id", "selected-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Selected Customer",
            "--field-value", "PROPERTIES=ControlSource = \"customer.name\"",
            "--json"
        },
        temp_root);

    expect(plan_process.exit_code == 0,
        "#1301: selection-toolbox-create-plan JSON command should exit successfully");
    expect_contains(plan_process.stdout_text, "\"status\": \"ok\"",
        "#1301: successful selection-toolbox-create-plan JSON should report ok status");
    expect_contains(plan_process.stdout_text, "\"selectionToolboxCreatePlan\": {",
        "#1301: selection-toolbox-create-plan JSON should expose a stable result object");
    expect_contains(plan_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1301: selection-toolbox-create-plan JSON should expose selected contexts");
    expect_contains(plan_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1301: selection-toolbox-create-plan JSON should expose resolved toolbox contexts");
    expect_contains(plan_process.stdout_text, "\"launchPlanOk\": true",
        "#1301: selection-toolbox-create-plan JSON should expose launch plan status");
    expect_contains(plan_process.stdout_text, "\"createPlanOk\": true",
        "#1301: selection-toolbox-create-plan JSON should expose create plan status");
    expect_contains(plan_process.stdout_text, "\"createPlan\": {",
        "#1301: selection-toolbox-create-plan JSON should expose nested create plans");
    expect_contains(plan_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1301: selection-toolbox-create-plan JSON should expose toolbox item ids");
    expect_contains(plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#1301: selection-toolbox-create-plan JSON should expose descriptor class names");
    expect_contains(plan_process.stdout_text, "\"toolboxContextProvided\": true",
        "#1301: selection-toolbox-create-plan JSON should expose context-filtered planning");
    expect_contains(plan_process.stdout_text, "\"planReadyItemIds\": [\"textbox\"]",
        "#1405: selection-toolbox-create-plan JSON should summarize plan-ready item ids");
    expect_contains(plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#1405: selection-toolbox-create-plan JSON should expose empty blocked item ids");
    expect_contains(plan_process.stdout_text, "\"planBlockedErrors\": []",
        "#1405: selection-toolbox-create-plan JSON should expose empty blocked plan errors");
    expect_contains(plan_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1301: selection-toolbox-create-plan JSON should expose target record indexes");
    expect_contains(plan_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1301: selection-toolbox-create-plan JSON should expose generated object names");
    expect_contains(plan_process.stdout_text, "\"uniqueId\": \"selected-textbox-guid\"",
        "#1301: selection-toolbox-create-plan JSON should expose planned unique ids");
    expect_contains(plan_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1301: selection-toolbox-create-plan JSON should expose planned parent names");
    expect_contains(plan_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1301: selection-toolbox-create-plan JSON should expose caller direct fields");
    expect_contains(plan_process.stdout_text, "\"propertyName\": \"PROPERTIES\"",
        "#1301: selection-toolbox-create-plan JSON should expose caller memo fields");
    expect_contains(plan_process.stdout_text, "\"dryRun\": true",
        "#1301: selection-toolbox-create-plan JSON should expose dry-run state");
    expect_contains(plan_process.stdout_text, "\"mutatesAsset\": false",
        "#1301: selection-toolbox-create-plan JSON should remain non-mutating");
    expect(visual_object_count(form_path) == before_count,
        "#1301: selection-toolbox-create-plan host command should not mutate the visual asset");

    const auto report_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan", "label",
            "--selection-context", "report_expression",
            "--parent-name", "DetailBand",
            "--json"
        },
        temp_root);
    expect(report_plan_process.exit_code == 0,
        "#1301: report selection-toolbox-create-plan JSON command should exit successfully");
    expect_contains(report_plan_process.stdout_text, "\"selectionToolboxCreatePlan\": {",
        "#2119: report selection-toolbox-create-plan JSON should expose a stable result object");
    expect_contains(report_plan_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1301: report selection-toolbox-create-plan JSON should expose report selections");
    expect_contains(report_plan_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1301: report selection-toolbox-create-plan JSON should expose report contexts");
    expect_contains(report_plan_process.stdout_text, "\"launchPlanOk\": true",
        "#2119: report selection-toolbox-create-plan JSON should expose launch plan status");
    expect_contains(report_plan_process.stdout_text, "\"createPlanOk\": true",
        "#2119: report selection-toolbox-create-plan JSON should expose create plan status");
    expect_contains(report_plan_process.stdout_text, "\"createPlan\": {",
        "#2119: report selection-toolbox-create-plan JSON should expose nested create plans");
    expect_contains(report_plan_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1301: report selection-toolbox-create-plan JSON should expose label plans");
    expect_contains(report_plan_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#1301: report selection-toolbox-create-plan JSON should expose generated label names");
    expect_contains(report_plan_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2119: report selection-toolbox-create-plan JSON should preserve report parent payloads");
    expect_contains(report_plan_process.stdout_text, "\"planReadyItemIds\": [\"label\"]",
        "#2119: report selection-toolbox-create-plan JSON should summarize plan-ready report item ids");
    expect_contains(report_plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#2119: report selection-toolbox-create-plan JSON should expose empty blocked item ids");
    expect_contains(report_plan_process.stdout_text, "\"planBlockedErrors\": []",
        "#2119: report selection-toolbox-create-plan JSON should expose empty blocked plan errors");
    expect_contains(report_plan_process.stdout_text, "\"dryRun\": true",
        "#2119: report selection-toolbox-create-plan JSON should expose dry-run state");
    expect_contains(report_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#2119: report selection-toolbox-create-plan JSON should remain non-mutating");
    expect_not_contains(report_plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#1301: report selection-toolbox-create-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2119: report selection-toolbox-create-plan host command should not mutate assets");

    const auto label_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan", "label",
            "--selection-context", "label_expression",
            "--parent-name", "DetailBand",
            "--json"
        },
        temp_root);
    expect(label_plan_process.exit_code == 0,
        "#2081: label selection-toolbox-create-plan JSON command should exit successfully");
    expect_contains(label_plan_process.stdout_text, "\"selectionToolboxCreatePlan\": {",
        "#2121: label selection-toolbox-create-plan JSON should expose a stable result object");
    expect_contains(label_plan_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#2081: label selection-toolbox-create-plan JSON should expose label selections");
    expect_contains(label_plan_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2081: label selection-toolbox-create-plan JSON should expose report contexts");
    expect_contains(label_plan_process.stdout_text, "\"launchPlanOk\": true",
        "#2121: label selection-toolbox-create-plan JSON should expose launch plan status");
    expect_contains(label_plan_process.stdout_text, "\"createPlanOk\": true",
        "#2121: label selection-toolbox-create-plan JSON should expose create plan status");
    expect_contains(label_plan_process.stdout_text, "\"createPlan\": {",
        "#2121: label selection-toolbox-create-plan JSON should expose nested create plans");
    expect_contains(label_plan_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2081: label selection-toolbox-create-plan JSON should expose label plans");
    expect_contains(label_plan_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2081: label selection-toolbox-create-plan JSON should expose generated label names");
    expect_contains(label_plan_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2121: label selection-toolbox-create-plan JSON should preserve label parent payloads");
    expect_contains(label_plan_process.stdout_text, "\"planReadyItemIds\": [\"label\"]",
        "#2121: label selection-toolbox-create-plan JSON should summarize plan-ready label item ids");
    expect_contains(label_plan_process.stdout_text, "\"planBlockedItemIds\": []",
        "#2121: label selection-toolbox-create-plan JSON should expose empty blocked item ids");
    expect_contains(label_plan_process.stdout_text, "\"planBlockedErrors\": []",
        "#2121: label selection-toolbox-create-plan JSON should expose empty blocked plan errors");
    expect_contains(label_plan_process.stdout_text, "\"dryRun\": true",
        "#2121: label selection-toolbox-create-plan JSON should expose dry-run state");
    expect_contains(label_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#2121: label selection-toolbox-create-plan JSON should remain non-mutating");
    expect_not_contains(label_plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#2081: label selection-toolbox-create-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2081: label selection-toolbox-create-plan host command should not mutate assets");

    const auto unavailable_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan", "textbox",
            "--selection-context", "report_expression",
            "--json"
        },
        temp_root);
    expect(unavailable_process.exit_code == 4,
        "#1301: selection-toolbox-create-plan JSON should reject unavailable context items");
    expect_contains(unavailable_process.stdout_text, "\"selectionToolboxCreatePlan\": null",
        "#1301: unavailable selection-toolbox-create-plan JSON should suppress stale plans");
    expect_contains(unavailable_process.stdout_text,
        "The requested toolbox item is not available in the requested designer context.",
        "#1301: unavailable selection-toolbox-create-plan JSON should report planner errors");

    const auto unsupported_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan", "textbox",
            "--selection-context", "menu_item",
            "--json"
        },
        temp_root);
    expect(unsupported_process.exit_code == 4,
        "#1301: selection-toolbox-create-plan JSON should reject unsupported selections");
    expect_contains(unsupported_process.stdout_text, "\"selectionToolboxCreatePlan\": null",
        "#1301: unsupported selection-toolbox-create-plan JSON should suppress stale plans");
    expect_contains(unsupported_process.stdout_text,
        "A selection-context toolbox object creation plan request requires a toolbox palette.",
        "#1301: unsupported selection-toolbox-create-plan JSON should report palette errors");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-plan", "textbox",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1301: selection-toolbox-create-plan JSON should reject missing paths");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1301: missing path selection-toolbox-create-plan JSON should report parser errors");

    const auto missing_item_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-context", "visual_object",
            "--selection-toolbox-create-plan",
            "--json"
        },
        temp_root);
    expect(missing_item_process.exit_code == 2,
        "#1301: selection-toolbox-create-plan JSON should reject missing item ids");
    expect_contains(missing_item_process.stdout_text, "Missing value for --selection-toolbox-create-plan.",
        "#1301: missing item selection-toolbox-create-plan JSON should report parser errors");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan", "textbox",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1301: selection-toolbox-create-plan JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1301: missing selection selection-toolbox-create-plan JSON should report parser errors");

    const auto unknown_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan", "textbox",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_selection_process.exit_code == 2,
        "#1301: selection-toolbox-create-plan JSON should reject unknown selections");
    expect_contains(unknown_selection_process.stdout_text, "Unknown selection context token: unknown",
        "#1301: unknown selection selection-toolbox-create-plan JSON should report parser errors");

    const auto malformed_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan", "textbox",
            "--selection-context", "visual_object",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(malformed_field_process.exit_code == 2,
        "#1301: selection-toolbox-create-plan JSON should reject malformed field values");
    expect_contains(malformed_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1301: malformed field selection-toolbox-create-plan JSON should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-plan", "textbox",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1301: selection-toolbox-create-plan JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-toolbox-create-plan option: --toolbox-context",
        "#1301: unknown option selection-toolbox-create-plan JSON should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1301: rejected selection-toolbox-create-plan host commands should not mutate the visual asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
