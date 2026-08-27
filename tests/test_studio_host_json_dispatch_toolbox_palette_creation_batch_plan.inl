void test_studio_host_json_plans_toolbox_object_creation_batch_dispatches_from_palette_dispatch(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_batch_dispatch_from_dispatch_plan_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto batch_dispatch_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--selection-context", "visual_object",
            "--record", "0",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--toolbox-item", "textbox",
            "--create-unique-id", "first-batch-dispatch-source-guid",
            "--field-value", "CAPTION=First Batch Dispatch",
            "--toolbox-item", "commandbutton",
            "--create-object-name", "cmdBatchDispatchSource",
            "--create-unique-id", "batch-dispatch-source-command-guid",
            "--create-parent-name", "cntToolbar",
            "--field-value", "CAPTION=Run Batch Dispatch",
            "--toolbox-item", "textbox",
            "--create-unique-id", "second-batch-dispatch-source-guid",
            "--field-value", "CAPTION=Second Batch Dispatch",
            "--admit-palette-invocation", "true",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(batch_dispatch_process.exit_code == 0,
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON command should exit successfully");
    expect_contains(batch_dispatch_process.stdout_text, "\"toolboxCreateBatchDispatchPlan\": {",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose batch dispatch plans");
    expect_contains(batch_dispatch_process.stdout_text, "\"toolboxContextProvided\": true",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should use dispatch toolbox contexts");
    expect_contains(batch_dispatch_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should resolve form contexts");
    expect_contains(batch_dispatch_process.stdout_text, "\"itemCount\": 3",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose item counts");
    expect_contains(batch_dispatch_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose textbox plans");
    expect_contains(batch_dispatch_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose command-button plans");
    expect_contains(batch_dispatch_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose first target indexes");
    expect_contains(batch_dispatch_process.stdout_text, "\"targetRecordIndex\": 4",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose later target indexes");
    expect_contains(batch_dispatch_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose generated names");
    expect_contains(batch_dispatch_process.stdout_text, "\"objectName\": \"txt3\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should reserve generated names");
    expect_contains(batch_dispatch_process.stdout_text, "\"objectName\": \"cmdBatchDispatchSource\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve explicit names");
    expect_contains(batch_dispatch_process.stdout_text, "\"uniqueId\": \"first-batch-dispatch-source-guid\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose unique ids");
    expect_contains(batch_dispatch_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should default selected parents");
    expect_contains(batch_dispatch_process.stdout_text, "\"parentName\": \"cntToolbar\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve parent overrides");
    expect_contains(batch_dispatch_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose field values");
    expect_contains(batch_dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(batch_dispatch_process.stdout_text, "\"--toolbox-create-batch\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should dispatch to batch create");
    expect_contains(batch_dispatch_process.stdout_text, "\"--toolbox-context\", \"form\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve toolbox contexts");
    expect_contains(batch_dispatch_process.stdout_text, "\"--toolbox-item\", \"textbox\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve textbox items");
    expect_contains(batch_dispatch_process.stdout_text, "\"--object-name\", \"txt2\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve generated names");
    expect_contains(batch_dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=First Batch Dispatch\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve first field values");
    expect_contains(batch_dispatch_process.stdout_text, "\"--toolbox-item\", \"commandbutton\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve command items");
    expect_contains(batch_dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Run Batch Dispatch\"",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve command field values");
    expect_contains(batch_dispatch_process.stdout_text, "\"dispatchAdmitted\": true",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose dispatch admission state");
    expect_contains(batch_dispatch_process.stdout_text, "\"dryRun\": false",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose non-dry-run dispatch state");
    expect_contains(batch_dispatch_process.stdout_text, "\"executed\": false",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should remain non-executing");
    expect_contains(batch_dispatch_process.stdout_text, "\"mutatesAsset\": true",
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose mutation intent");
    expect(visual_object_count(form_path) == before_count,
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan host command should not mutate the visual asset");

    const auto report_batch_dispatch_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--selection-context", "report_expression",
            "--object-name", "DetailBand",
            "--toolbox-item", "label",
            "--create-unique-id", "dispatch-report-batch-dispatch-guid",
            "--create-parent-name", "DetailBand",
            "--field-value", "CAPTION=Dispatch Report Batch Plan",
            "--admit-palette-invocation", "true",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(report_batch_dispatch_process.exit_code == 0,
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON command should exit successfully");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"toolboxCreateBatchDispatchPlan\": {",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose stable batch dispatch plans");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"toolboxContextProvided\": true",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should use dispatch toolbox contexts");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should resolve report contexts");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"itemCount\": 1",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose report batch item counts");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose label batch dispatch plans");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose generated label names");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"uniqueId\": \"dispatch-report-batch-dispatch-guid\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose label unique ids");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose label parent overrides");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"propertyValue\": \"Dispatch Report Batch Plan\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose label field values");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"--toolbox-create-batch\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should dispatch to batch create");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve report context arguments");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"--toolbox-item\", \"label\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve label item arguments");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"--object-name\", \"lbl1\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve label object-name arguments");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"--unique-id\", \"dispatch-report-batch-dispatch-guid\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve label unique-id arguments");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"--parent-name\", \"DetailBand\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve label parent arguments");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Dispatch Report Batch Plan\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve label field arguments");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"dispatchAdmitted\": true",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose dispatch admission state");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"dryRun\": false",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose non-dry-run state");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"executed\": false",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should remain non-executing");
    expect_contains(report_batch_dispatch_process.stdout_text, "\"mutatesAsset\": true",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose mutation intent");
    expect_not_contains(report_batch_dispatch_process.stdout_text, "\"className\": \"TextBox\"",
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2136: report toolbox-create-batch-dispatch-from-dispatch-plan host command should not mutate assets");

    const auto label_batch_dispatch_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--selection-context", "label_expression",
            "--object-name", "DetailBand",
            "--toolbox-item", "label",
            "--create-unique-id", "dispatch-label-batch-dispatch-guid",
            "--create-parent-name", "DetailBand",
            "--field-value", "CAPTION=Dispatch Label Batch Plan",
            "--admit-palette-invocation", "true",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(label_batch_dispatch_process.exit_code == 0,
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON command should exit successfully");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"toolboxCreateBatchDispatchPlan\": {",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose stable batch dispatch plans");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"toolboxContextProvided\": true",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should use dispatch toolbox contexts");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should resolve report contexts");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"itemCount\": 1",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose label batch item counts");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose label batch dispatch plans");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose generated label names");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"uniqueId\": \"dispatch-label-batch-dispatch-guid\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose label unique ids");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose label parent overrides");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"propertyValue\": \"Dispatch Label Batch Plan\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose label field values");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"dispatchArguments\": [",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose dispatch arguments");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"--toolbox-create-batch\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should dispatch to batch create");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"--toolbox-context\", \"report\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve report context arguments");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"--toolbox-item\", \"label\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve label item arguments");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"--object-name\", \"lbl1\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve label object-name arguments");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"--unique-id\", \"dispatch-label-batch-dispatch-guid\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve label unique-id arguments");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"--parent-name\", \"DetailBand\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve label parent arguments");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"--field-value\", \"CAPTION=Dispatch Label Batch Plan\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should preserve label field arguments");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"dispatchAdmitted\": true",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose dispatch admission state");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"dryRun\": false",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose non-dry-run state");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"executed\": false",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should remain non-executing");
    expect_contains(label_batch_dispatch_process.stdout_text, "\"mutatesAsset\": true",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should expose mutation intent");
    expect_not_contains(label_batch_dispatch_process.stdout_text, "\"className\": \"TextBox\"",
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2134: label toolbox-create-batch-dispatch-from-dispatch-plan host command should not mutate assets");

    const auto non_admitted_palette_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--selection-context", "visual_object",
            "--object-name", "frmCustomer",
            "--toolbox-item", "textbox",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(non_admitted_palette_process.exit_code == 4,
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should reject non-admitted dispatches");
    expect_contains(non_admitted_palette_process.stdout_text, "\"toolboxCreateBatchDispatchPlan\": null",
        "#1267: non-admitted batch dispatch JSON should not expose stale plans");
    expect_contains(non_admitted_palette_process.stdout_text,
        "A toolbox dispatch request requires an admitted non-dry-run invocation.",
        "#1267: non-admitted batch dispatch JSON should report dispatch errors");
    expect_not_contains(non_admitted_palette_process.stdout_text, "\"--toolbox-create-batch\"",
        "#1267: non-admitted batch dispatch JSON should not expose stale dispatch arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1267: non-admitted batch dispatch commands should not mutate assets");

    const auto non_admitted_create_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--selection-context", "visual_object",
            "--object-name", "frmCustomer",
            "--toolbox-item", "textbox",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(non_admitted_create_process.exit_code == 4,
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should reject non-admitted create operations");
    expect_contains(non_admitted_create_process.stdout_text, "\"toolboxCreateBatchDispatchPlan\": null",
        "#1267: non-admitted create batch dispatch JSON should not expose stale plans");
    expect_contains(non_admitted_create_process.stdout_text,
        "A toolbox batch create dispatch request requires an admitted non-dry-run create operation.",
        "#1267: non-admitted create batch dispatch JSON should report dispatch errors");
    expect_not_contains(non_admitted_create_process.stdout_text, "\"--toolbox-create-batch\"",
        "#1267: non-admitted create batch dispatch JSON should not expose stale dispatch arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1267: non-admitted create batch dispatch commands should not mutate assets");

    const auto unavailable_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--selection-context", "report_expression",
            "--object-name", "DetailBand",
            "--toolbox-item", "textbox",
            "--admit-palette-invocation", "true",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(unavailable_process.exit_code == 4,
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should reject unavailable dispatch items");
    expect_contains(unavailable_process.stdout_text, "\"toolboxCreateBatchDispatchPlan\": null",
        "#1267: unavailable batch dispatch JSON should not expose stale plans");
    expect_contains(unavailable_process.stdout_text,
        "The requested toolbox item is not available in the admitted toolbox dispatch.",
        "#1267: unavailable batch dispatch JSON should report availability errors");
    expect_not_contains(unavailable_process.stdout_text, "\"--toolbox-create-batch\"",
        "#1267: unavailable batch dispatch JSON should not expose stale dispatch arguments");
    expect(visual_object_count(form_path) == before_count,
        "#1267: unavailable batch dispatch commands should not mutate assets");

    const auto missing_items_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "true",
            "--admit-create-operation", "true",
            "--json"
        },
        temp_root);
    expect(missing_items_process.exit_code == 2,
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should reject empty item lists");
    expect_contains(missing_items_process.stdout_text, "No toolbox item ids were provided.",
        "#1267: empty toolbox-create-batch-dispatch-from-dispatch-plan JSON should report parser errors");

    const auto orphan_item_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--selection-context", "visual_object",
            "--create-parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(orphan_item_option_process.exit_code == 2,
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should reject item options before items");
    expect_contains(orphan_item_option_process.stdout_text,
        "Toolbox batch item options require a preceding --toolbox-item.",
        "#1267: orphan toolbox-create-batch-dispatch-from-dispatch-plan item options should report parser errors");

    const auto invalid_create_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-dispatch-from-dispatch-plan",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--admit-palette-invocation", "true",
            "--admit-create-operation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_create_admission_process.exit_code == 2,
        "#1267: toolbox-create-batch-dispatch-from-dispatch-plan JSON should reject invalid create admission tokens");
    expect_contains(invalid_create_admission_process.stdout_text,
        "The --admit-create-operation value must be true or false.",
        "#1267: invalid create admission JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
