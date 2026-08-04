// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

// Toolbox plan dispatch and selection-dispatch coverage.
void test_studio_host_json_plans_toolbox_object_creation_dispatch(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_dispatch_plan_json_tests";
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
            "--toolbox-create-dispatch-plan", "textbox",
            "--toolbox-context", "form",
            "--unique-id", "dispatch-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Dispatch",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(dispatch_process.exit_code == 0,
        "#1250: toolbox-create-dispatch-plan JSON command should exit successfully");
    expect_contains(dispatch_process.stdout_text, "\"toolboxCreateDispatchPlan\": {",
        "#1250: toolbox-create-dispatch-plan JSON should expose a dispatch result object");
    expect_contains(dispatch_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1250: toolbox-create-dispatch-plan JSON should expose toolbox item ids");
    expect_contains(dispatch_process.stdout_text, "\"className\": \"TextBox\"",
        "#1250: toolbox-create-dispatch-plan JSON should expose descriptor class names");
    expect_contains(dispatch_process.stdout_text, "\"toolboxContextProvided\": true",
        "#1250: toolbox-create-dispatch-plan JSON should expose requested context state");
    expect_contains(dispatch_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1250: toolbox-create-dispatch-plan JSON should expose requested contexts");
    expect_contains(dispatch_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1250: toolbox-create-dispatch-plan JSON should expose planned target record indexes");
    expect_contains(dispatch_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1250: toolbox-create-dispatch-plan JSON should expose generated object names");
    expect_contains(dispatch_process.stdout_text, "\"uniqueId\": \"dispatch-textbox-guid\"",
        "#1250: toolbox-create-dispatch-plan JSON should expose planned unique ids");
    expect_contains(dispatch_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1250: toolbox-create-dispatch-plan JSON should expose planned parent names");
    expect_contains(dispatch_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1250: toolbox-create-dispatch-plan JSON should expose planned field values");
    expect_contains(dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#1250: toolbox-create-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(dispatch_process.stdout_text, "\"--toolbox-create\", \"textbox\"",
        "#1250: toolbox-create-dispatch-plan JSON should dispatch to toolbox-create");
    expect_contains(dispatch_process.stdout_text, "\"--toolbox-context\", \"form\"",
        "#1250: toolbox-create-dispatch-plan JSON should preserve toolbox context arguments");
    expect_contains(dispatch_process.stdout_text, "\"--object-name\", \"txt2\"",
        "#1250: toolbox-create-dispatch-plan JSON should preserve object-name arguments");
    expect_contains(dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Dispatch\"",
        "#1250: toolbox-create-dispatch-plan JSON should preserve caller field-value arguments");
    expect_contains(dispatch_process.stdout_text, "\"dispatchReadyItemIds\": [\"textbox\"]",
        "#1386: toolbox-create-dispatch-plan JSON should summarize dispatch-ready item ids");
    expect_contains(dispatch_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1386: toolbox-create-dispatch-plan JSON should summarize empty blocked item ids");
    expect_contains(dispatch_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1386: successful toolbox-create-dispatch-plan JSON should summarize empty dispatch errors");
    expect_contains(dispatch_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1250: toolbox-create-dispatch-plan JSON should expose dispatch admission state");
    expect_contains(dispatch_process.stdout_text, "\"dryRun\": false",
        "#1250: toolbox-create-dispatch-plan JSON should expose non-dry-run dispatch state");
    expect_contains(dispatch_process.stdout_text, "\"executed\": false",
        "#1250: toolbox-create-dispatch-plan JSON should remain non-executing");
    expect_contains(dispatch_process.stdout_text, "\"mutatesAsset\": true",
        "#1250: toolbox-create-dispatch-plan JSON should expose mutation intent");
    expect(visual_object_count(form_path) == before_count,
        "#1250: toolbox-create-dispatch-plan host command should not mutate the visual asset");

    const auto report_dispatch_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-plan", "label",
            "--toolbox-context", "report",
            "--unique-id", "direct-report-dispatch-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Direct Report Dispatch",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(report_dispatch_process.exit_code == 0,
        "#2103: report toolbox-create-dispatch-plan JSON command should exit successfully");
    expect_contains(report_dispatch_process.stdout_text, "\"toolboxCreateDispatchPlan\": {",
        "#2103: report toolbox-create-dispatch-plan JSON should expose dispatch plans");
    expect_contains(report_dispatch_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2103: report toolbox-create-dispatch-plan JSON should expose label plans");
    expect_contains(report_dispatch_process.stdout_text, "\"className\": \"Label\"",
        "#2103: report toolbox-create-dispatch-plan JSON should expose label descriptor metadata");
    expect_contains(report_dispatch_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2103: report toolbox-create-dispatch-plan JSON should preserve report contexts");
    expect_contains(report_dispatch_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2103: report toolbox-create-dispatch-plan JSON should expose generated label names");
    expect_contains(report_dispatch_process.stdout_text, "\"uniqueId\": \"direct-report-dispatch-guid\"",
        "#2103: report toolbox-create-dispatch-plan JSON should preserve label unique ids");
    expect_contains(report_dispatch_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2103: report toolbox-create-dispatch-plan JSON should preserve label parent overrides");
    expect_contains(report_dispatch_process.stdout_text, "\"propertyValue\": \"Direct Report Dispatch\"",
        "#2103: report toolbox-create-dispatch-plan JSON should preserve label field values");
    expect_contains(report_dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#2103: report toolbox-create-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--toolbox-create\", \"label\"",
        "#2103: report toolbox-create-dispatch-plan JSON should dispatch label creates");
    expect_contains(report_dispatch_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#2103: report toolbox-create-dispatch-plan JSON should preserve report context arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--object-name\", \"lbl1\"",
        "#2103: report toolbox-create-dispatch-plan JSON should preserve generated label arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--unique-id\", \"direct-report-dispatch-guid\"",
        "#2103: report toolbox-create-dispatch-plan JSON should preserve label unique-id arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--parent-name\", \"DetailBand\"",
        "#2103: report toolbox-create-dispatch-plan JSON should preserve label parent arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Direct Report Dispatch\"",
        "#2103: report toolbox-create-dispatch-plan JSON should preserve label field arguments");
    expect_contains(report_dispatch_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"]",
        "#2103: report toolbox-create-dispatch-plan JSON should summarize dispatch-ready report item ids");
    expect_contains(report_dispatch_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#2103: report toolbox-create-dispatch-plan JSON should summarize empty blocked item ids");
    expect_contains(report_dispatch_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#2103: report toolbox-create-dispatch-plan JSON should summarize empty dispatch errors");
    expect_contains(report_dispatch_process.stdout_text, "\"executed\": false",
        "#2103: report toolbox-create-dispatch-plan JSON should remain non-executing");
    expect_not_contains(report_dispatch_process.stdout_text, "\"className\": \"TextBox\"",
        "#2103: report toolbox-create-dispatch-plan JSON should exclude form-only TextBox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2103: report toolbox-create-dispatch-plan host command should not mutate assets");

    const auto non_admitted_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-plan", "textbox",
            "--admit-create-operation", "false",
            "--json"
        },
        temp_root);
    expect(non_admitted_process.exit_code == 4,
        "#1250: toolbox-create-dispatch-plan JSON should reject non-admitted create operations");
    expect_contains(non_admitted_process.stdout_text, "\"toolboxCreateDispatchPlan\": null",
        "#1250: non-admitted toolbox-create-dispatch-plan JSON should not expose stale dispatch plans");
    expect_contains(non_admitted_process.stdout_text,
        "A toolbox create dispatch request requires an admitted non-dry-run create operation.",
        "#1250: non-admitted toolbox-create-dispatch-plan JSON should report dispatch errors");
    expect_contains(non_admitted_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1386: non-admitted toolbox-create-dispatch-plan JSON should summarize empty ready item ids");
    expect_contains(non_admitted_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1386: non-admitted toolbox-create-dispatch-plan JSON should summarize aggregate blocked state");
    expect_contains(non_admitted_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"A toolbox create dispatch request requires an admitted non-dry-run create operation.\"",
        "#1386: non-admitted toolbox-create-dispatch-plan JSON should summarize dispatch errors");
    expect_not_contains(non_admitted_process.stdout_text, "\"dispatchArguments\": [",
        "#1250: failed toolbox-create-dispatch-plan JSON should not expose stale dispatch arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1250: non-admitted toolbox-create-dispatch-plan commands should not mutate the visual asset");

    const auto unknown_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-plan", "missing-toolbox-item",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(unknown_process.exit_code == 4,
        "#1250: toolbox-create-dispatch-plan JSON should reject invalid create plans");
    expect_contains(unknown_process.stdout_text, "The requested toolbox item was not found.",
        "#1250: invalid toolbox-create-dispatch-plan create plans should report planning errors");
    expect_contains(unknown_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1386: invalid toolbox-create-dispatch-plan JSON should summarize empty ready item ids");
    expect_contains(unknown_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"The requested toolbox item was not found.\"",
        "#1386: invalid toolbox-create-dispatch-plan JSON should summarize planning errors");
    expect_not_contains(unknown_process.stdout_text, "\"dispatchArguments\": [",
        "#1250: invalid toolbox-create-dispatch-plan create plans should not expose stale arguments");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-dispatch-plan", "textbox",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1250: toolbox-create-dispatch-plan JSON should reject invalid admission tokens");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#1250: invalid toolbox-create-dispatch-plan admission tokens should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
void test_studio_host_json_plans_selection_toolbox_object_creation_dispatch(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_create_dispatch_plan_json_tests";
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
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--unique-id", "selection-dispatch-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Selection Dispatch",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(dispatch_process.exit_code == 0,
        "#1303: selection-toolbox-create-dispatch-plan JSON command should exit successfully");
    expect_contains(dispatch_process.stdout_text, "\"selectionToolboxCreateDispatchPlan\": {",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose a stable result object");
    expect_contains(dispatch_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose selected Studio contexts");
    expect_contains(dispatch_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose resolved toolbox contexts");
    expect_contains(dispatch_process.stdout_text, "\"launchPlanOk\": true",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose launch state");
    expect_contains(dispatch_process.stdout_text, "\"createPlanOk\": true",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose create-plan state");
    expect_contains(dispatch_process.stdout_text, "\"dispatchOk\": true",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose dispatch state");
    expect_contains(dispatch_process.stdout_text, "\"dispatchCount\": 1",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose dispatch counts");
    expect_contains(dispatch_process.stdout_text, "\"errorCount\": 0",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose zero errors");
    expect_contains(dispatch_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose toolbox item ids");
    expect_contains(dispatch_process.stdout_text, "\"className\": \"TextBox\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose descriptor class names");
    expect_contains(dispatch_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose planned target records");
    expect_contains(dispatch_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose generated names");
    expect_contains(dispatch_process.stdout_text, "\"uniqueId\": \"selection-dispatch-textbox-guid\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose caller unique ids");
    expect_contains(dispatch_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose parent names");
    expect_contains(dispatch_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose caller field names");
    expect_contains(dispatch_process.stdout_text, "\"propertyValue\": \"Selection Dispatch\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose caller field values");
    expect_contains(dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(dispatch_process.stdout_text, "\"--toolbox-create\", \"textbox\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should dispatch to toolbox-create");
    expect_contains(dispatch_process.stdout_text, "\"--toolbox-context\", \"form\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should preserve resolved toolbox contexts");
    expect_contains(dispatch_process.stdout_text, "\"--object-name\", \"txt2\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should preserve object-name arguments");
    expect_contains(dispatch_process.stdout_text, "\"--unique-id\", \"selection-dispatch-textbox-guid\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should preserve unique-id arguments");
    expect_contains(dispatch_process.stdout_text, "\"--parent-name\", \"frmCustomer\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should preserve parent-name arguments");
    expect_contains(dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Selection Dispatch\"",
        "#1303: selection-toolbox-create-dispatch-plan JSON should preserve field-value arguments");
    expect_contains(dispatch_process.stdout_text, "\"dispatchReadyItemIds\": [\"textbox\"]",
        "#1386: selection-toolbox-create-dispatch-plan JSON should summarize dispatch-ready item ids");
    expect_contains(dispatch_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1386: selection-toolbox-create-dispatch-plan JSON should summarize empty blocked item ids");
    expect_contains(dispatch_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1386: successful selection-toolbox-create-dispatch-plan JSON should summarize empty dispatch errors");
    expect_contains(dispatch_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1303: selection-toolbox-create-dispatch-plan JSON should expose dispatch admission state");
    expect_contains(dispatch_process.stdout_text, "\"dryRun\": false",
        "#1303: admitted selection-toolbox-create-dispatch-plan JSON should expose non-dry-run state");
    expect_contains(dispatch_process.stdout_text, "\"executed\": false",
        "#1303: selection-toolbox-create-dispatch-plan JSON should remain non-executing");
    expect_contains(dispatch_process.stdout_text, "\"mutatesAsset\": true",
        "#1303: admitted selection-toolbox-create-dispatch-plan JSON should expose mutation intent");
    expect(visual_object_count(form_path) == before_count,
        "#1303: admitted selection-toolbox-create-dispatch-plan host command should not mutate assets");

    const auto non_admitted_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--admit-create-operation", "false",
            "--json"
        },
        temp_root);
    expect(non_admitted_process.exit_code == 4,
        "#1303: selection-toolbox-create-dispatch-plan JSON should reject non-admitted creates");
    expect_contains(non_admitted_process.stdout_text, "\"createPlanOk\": true",
        "#1303: non-admitted selection-toolbox-create-dispatch-plan JSON should preserve create-plan state");
    expect_contains(non_admitted_process.stdout_text, "\"dispatchOk\": false",
        "#1303: non-admitted selection-toolbox-create-dispatch-plan JSON should expose dispatch failures");
    expect_contains(non_admitted_process.stdout_text,
        "A toolbox create dispatch request requires an admitted non-dry-run create operation.",
        "#1303: non-admitted selection-toolbox-create-dispatch-plan JSON should report dispatch errors");
    expect_contains(non_admitted_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1386: non-admitted selection-toolbox-create-dispatch-plan JSON should summarize empty ready item ids");
    expect_contains(non_admitted_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#1386: non-admitted selection-toolbox-create-dispatch-plan JSON should summarize aggregate blocked state");
    expect_contains(non_admitted_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"A toolbox create dispatch request requires an admitted non-dry-run create operation.\"",
        "#1386: non-admitted selection-toolbox-create-dispatch-plan JSON should summarize dispatch errors");
    expect_not_contains(non_admitted_process.stdout_text, "\"dispatchArguments\": [",
        "#1303: non-admitted selection-toolbox-create-dispatch-plan JSON should omit stale arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1303: non-admitted selection-toolbox-create-dispatch-plan host command should not mutate assets");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "label",
            "--selection-context", "report_expression",
            "--parent-name", "DetailBand",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#1303: report selection-toolbox-create-dispatch-plan JSON command should exit successfully");
    expect_contains(report_process.stdout_text, "\"selectionToolboxCreateDispatchPlan\": {",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should expose a stable result object");
    expect_contains(report_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1303: report selection-toolbox-create-dispatch-plan JSON should expose report selections");
    expect_contains(report_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1303: report selection-toolbox-create-dispatch-plan JSON should resolve report contexts");
    expect_contains(report_process.stdout_text, "\"launchPlanOk\": true",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should expose launch state");
    expect_contains(report_process.stdout_text, "\"createPlanOk\": true",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should expose create-plan state");
    expect_contains(report_process.stdout_text, "\"dispatchOk\": true",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should expose dispatch state");
    expect_contains(report_process.stdout_text, "\"dispatchCount\": 1",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should expose dispatch counts");
    expect_contains(report_process.stdout_text, "\"errorCount\": 0",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should expose zero errors");
    expect_contains(report_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#1303: report selection-toolbox-create-dispatch-plan JSON should expose label dispatches");
    expect_contains(report_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#1303: report selection-toolbox-create-dispatch-plan JSON should expose generated label names");
    expect_contains(report_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should preserve report parent payloads");
    expect_contains(report_process.stdout_text, "\"dispatchArguments\": [",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(report_process.stdout_text, "\"--toolbox-create\", \"label\"",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should dispatch to toolbox-create");
    expect_contains(report_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#1303: report selection-toolbox-create-dispatch-plan JSON should preserve report context arguments");
    expect_contains(report_process.stdout_text, "\"--parent-name\", \"DetailBand\"",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should preserve report parent arguments");
    expect_contains(report_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"]",
        "#1386: report selection-toolbox-create-dispatch-plan JSON should summarize dispatch-ready report item ids");
    expect_contains(report_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should summarize empty blocked item ids");
    expect_contains(report_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#1386: report selection-toolbox-create-dispatch-plan JSON should summarize empty dispatch errors");
    expect_contains(report_process.stdout_text, "\"dispatchAdmitted\": true",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should expose dispatch admission state");
    expect_contains(report_process.stdout_text, "\"dryRun\": false",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should expose non-dry-run state");
    expect_contains(report_process.stdout_text, "\"executed\": false",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should remain non-executing");
    expect_contains(report_process.stdout_text, "\"mutatesAsset\": true",
        "#2117: report selection-toolbox-create-dispatch-plan JSON should expose mutation intent");
    expect_not_contains(report_process.stdout_text, "\"className\": \"TextBox\"",
        "#1303: report selection-toolbox-create-dispatch-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2117: report selection-toolbox-create-dispatch-plan host command should not mutate assets");

    const auto label_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "label",
            "--selection-context", "label_expression",
            "--parent-name", "DetailBand",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(label_process.exit_code == 0,
        "#2082: label selection-toolbox-create-dispatch-plan JSON command should exit successfully");
    expect_contains(label_process.stdout_text, "\"selectionToolboxCreateDispatchPlan\": {",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should expose a stable result object");
    expect_contains(label_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#2082: label selection-toolbox-create-dispatch-plan JSON should expose label selections");
    expect_contains(label_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2082: label selection-toolbox-create-dispatch-plan JSON should resolve report contexts");
    expect_contains(label_process.stdout_text, "\"launchPlanOk\": true",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should expose launch state");
    expect_contains(label_process.stdout_text, "\"createPlanOk\": true",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should expose create-plan state");
    expect_contains(label_process.stdout_text, "\"dispatchOk\": true",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should expose dispatch state");
    expect_contains(label_process.stdout_text, "\"dispatchCount\": 1",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should expose dispatch counts");
    expect_contains(label_process.stdout_text, "\"errorCount\": 0",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should expose zero errors");
    expect_contains(label_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2082: label selection-toolbox-create-dispatch-plan JSON should expose label dispatches");
    expect_contains(label_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2082: label selection-toolbox-create-dispatch-plan JSON should expose generated label names");
    expect_contains(label_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should preserve label parent payloads");
    expect_contains(label_process.stdout_text, "\"dispatchArguments\": [",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(label_process.stdout_text, "\"--toolbox-create\", \"label\"",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should dispatch to toolbox-create");
    expect_contains(label_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#2082: label selection-toolbox-create-dispatch-plan JSON should preserve report context arguments");
    expect_contains(label_process.stdout_text, "\"--parent-name\", \"DetailBand\"",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should preserve label parent arguments");
    expect_contains(label_process.stdout_text, "\"dispatchReadyItemIds\": [\"label\"]",
        "#2082: label selection-toolbox-create-dispatch-plan JSON should summarize dispatch-ready label item ids");
    expect_contains(label_process.stdout_text, "\"dispatchBlockedItemIds\": []",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should summarize empty blocked item ids");
    expect_contains(label_process.stdout_text, "\"dispatchBlockedErrors\": []",
        "#2082: label selection-toolbox-create-dispatch-plan JSON should summarize empty dispatch errors");
    expect_contains(label_process.stdout_text, "\"dispatchAdmitted\": true",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should expose dispatch admission state");
    expect_contains(label_process.stdout_text, "\"dryRun\": false",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should expose non-dry-run state");
    expect_contains(label_process.stdout_text, "\"executed\": false",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should remain non-executing");
    expect_contains(label_process.stdout_text, "\"mutatesAsset\": true",
        "#2123: label selection-toolbox-create-dispatch-plan JSON should expose mutation intent");
    expect_not_contains(label_process.stdout_text, "\"className\": \"TextBox\"",
        "#2082: label selection-toolbox-create-dispatch-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2123: label selection-toolbox-create-dispatch-plan host command should not mutate assets");

    const auto unavailable_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--selection-context", "report_expression",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(unavailable_process.exit_code == 4,
        "#1303: selection-toolbox-create-dispatch-plan JSON should reject unavailable selected-context items");
    expect_contains(unavailable_process.stdout_text, "\"createPlanOk\": false",
        "#1303: unavailable selection-toolbox-create-dispatch-plan JSON should expose failed create-plan state");
    expect_contains(unavailable_process.stdout_text, "\"dispatchOk\": false",
        "#1303: unavailable selection-toolbox-create-dispatch-plan JSON should avoid stale dispatches");
    expect_contains(unavailable_process.stdout_text,
        "The requested toolbox item is not available in the requested designer context.",
        "#1303: unavailable selection-toolbox-create-dispatch-plan JSON should report planner errors");
    expect_contains(unavailable_process.stdout_text, "\"dispatchReadyItemIds\": []",
        "#1386: unavailable selection-toolbox-create-dispatch-plan JSON should summarize empty ready item ids");
    expect_contains(unavailable_process.stdout_text,
        "\"dispatchBlockedErrors\": [\"The requested toolbox item is not available in the requested designer context.\"",
        "#1386: unavailable selection-toolbox-create-dispatch-plan JSON should summarize planner errors");
    expect_not_contains(unavailable_process.stdout_text, "\"dispatchArguments\": [",
        "#1303: unavailable selection-toolbox-create-dispatch-plan JSON should omit stale arguments");

    const auto unsupported_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--selection-context", "menu_item",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(unsupported_process.exit_code == 4,
        "#1303: selection-toolbox-create-dispatch-plan JSON should reject unsupported selections");
    expect_contains(unsupported_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1303: unsupported selection-toolbox-create-dispatch-plan JSON should preserve selected contexts");
    expect_contains(unsupported_process.stdout_text,
        "A selection-context toolbox object creation plan request requires a toolbox palette.",
        "#1303: unsupported selection-toolbox-create-dispatch-plan JSON should report palette errors");
    expect_not_contains(unsupported_process.stdout_text, "\"dispatchArguments\": [",
        "#1303: unsupported selection-toolbox-create-dispatch-plan JSON should omit stale arguments");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1303: selection-toolbox-create-dispatch-plan JSON should reject missing paths");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1303: missing path selection-toolbox-create-dispatch-plan JSON should report parser errors");

    const auto missing_item_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-context", "visual_object",
            "--selection-toolbox-create-dispatch-plan",
            "--json"
        },
        temp_root);
    expect(missing_item_process.exit_code == 2,
        "#1303: selection-toolbox-create-dispatch-plan JSON should reject missing item ids");
    expect_contains(missing_item_process.stdout_text,
        "Missing value for --selection-toolbox-create-dispatch-plan.",
        "#1303: missing item selection-toolbox-create-dispatch-plan JSON should report parser errors");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1303: selection-toolbox-create-dispatch-plan JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1303: missing selection selection-toolbox-create-dispatch-plan JSON should report parser errors");

    const auto unknown_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_selection_process.exit_code == 2,
        "#1303: selection-toolbox-create-dispatch-plan JSON should reject unknown selections");
    expect_contains(unknown_selection_process.stdout_text, "Unknown selection context token: unknown",
        "#1303: unknown selection selection-toolbox-create-dispatch-plan JSON should report parser errors");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1303: selection-toolbox-create-dispatch-plan JSON should reject invalid admission tokens");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#1303: invalid selection-toolbox-create-dispatch-plan admission tokens should report parser errors");

    const auto malformed_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(malformed_field_process.exit_code == 2,
        "#1303: selection-toolbox-create-dispatch-plan JSON should reject malformed field values");
    expect_contains(malformed_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1303: malformed field selection-toolbox-create-dispatch-plan JSON should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1303: selection-toolbox-create-dispatch-plan JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-toolbox-create-dispatch-plan option: --toolbox-context",
        "#1303: unknown option selection-toolbox-create-dispatch-plan JSON should report parser errors");
    expect(visual_object_count(form_path) == before_count,
        "#1303: rejected selection-toolbox-create-dispatch-plan host commands should not mutate assets");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
