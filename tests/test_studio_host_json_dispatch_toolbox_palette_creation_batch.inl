void test_studio_host_json_creates_toolbox_object_batches_from_palette_dispatch(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_batch_from_dispatch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto create_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "visual_object",
            "--record", "0",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--toolbox-item", "textbox",
            "--create-unique-id", "first-dispatch-host-batch-guid",
            "--field-value", "CAPTION=First Dispatch Host Batch",
            "--toolbox-item", "commandbutton",
            "--create-object-name", "cmdDispatchHostBatch",
            "--create-unique-id", "dispatch-host-batch-command-guid",
            "--create-parent-name", "cntToolbar",
            "--field-value", "CAPTION=Run Dispatch Host Batch",
            "--toolbox-item", "textbox",
            "--create-unique-id", "second-dispatch-host-batch-guid",
            "--field-value", "CAPTION=Second Dispatch Host Batch",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    if (create_process.exit_code != 0) {
        std::cerr << "studio host toolbox-create-batch-from-dispatch stdout:\n"
                  << create_process.stdout_text << "\n";
        std::cerr << "studio host toolbox-create-batch-from-dispatch stderr:\n"
                  << create_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }
    expect(create_process.exit_code == 0,
        "#1315: toolbox-create-batch-from-dispatch JSON command should exit successfully");
    expect_contains(create_process.stdout_text, "\"status\": \"ok\"",
        "#1315: successful toolbox-create-batch-from-dispatch JSON should report ok status");
    expect_contains(create_process.stdout_text, "\"toolboxCreateBatchFromDispatch\": {",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose a stable result object");
    expect_contains(create_process.stdout_text, "\"batchPlanOk\": true",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose batch-plan state");
    expect_contains(create_process.stdout_text, "\"createResult\": {",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose lower-level create results");
    expect_contains(create_process.stdout_text, "\"toolboxContextProvided\": true",
        "#1315: toolbox-create-batch-from-dispatch JSON should use dispatch toolbox contexts");
    expect_contains(create_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1315: toolbox-create-batch-from-dispatch JSON should resolve form contexts");
    expect_contains(create_process.stdout_text, "\"itemCount\": 3",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose item counts");
    expect_contains(create_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose textbox plans");
    expect_contains(create_process.stdout_text, "\"toolboxItemId\": \"commandbutton\"",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose command-button plans");
    expect_contains(create_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose first target indexes");
    expect_contains(create_process.stdout_text, "\"targetRecordIndex\": 4",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose later target indexes");
    expect_contains(create_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose generated names");
    expect_contains(create_process.stdout_text, "\"objectName\": \"txt3\"",
        "#1315: toolbox-create-batch-from-dispatch JSON should reserve generated names");
    expect_contains(create_process.stdout_text, "\"objectName\": \"cmdDispatchHostBatch\"",
        "#1315: toolbox-create-batch-from-dispatch JSON should preserve explicit names");
    expect_contains(create_process.stdout_text, "\"recordIndexes\": [2, 3, 4]",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose created record indexes");
    expect_contains(create_process.stdout_text, "\"createdObjects\": [",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose created object metadata");
    expect_contains(create_process.stdout_text, "\"uniqueId\": \"first-dispatch-host-batch-guid\"",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose created unique ids");
    expect_contains(create_process.stdout_text, "\"parentName\": \"cntToolbar\"",
        "#1315: toolbox-create-batch-from-dispatch JSON should preserve parent overrides");
    expect_contains(create_process.stdout_text,
        "\"createdObjectNames\": [\"txt2\", \"cmdDispatchHostBatch\", \"txt3\"]",
        "#1385: toolbox-create-batch-from-dispatch JSON should summarize created object names");
    expect_contains(create_process.stdout_text,
        "\"createdUniqueIds\": [\"first-dispatch-host-batch-guid\", \"dispatch-host-batch-command-guid\", \"second-dispatch-host-batch-guid\"]",
        "#1385: toolbox-create-batch-from-dispatch JSON should summarize created unique ids");
    expect_contains(create_process.stdout_text, "\"createErrors\": []",
        "#1385: successful toolbox-create-batch-from-dispatch JSON should summarize empty create errors");
    expect_contains(create_process.stdout_text, "\"dryRun\": false",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose execution state");
    expect_contains(create_process.stdout_text, "\"mutatesAsset\": true",
        "#1315: toolbox-create-batch-from-dispatch JSON should expose mutation state");
    expect(visual_object_count(form_path) == before_count + 3U,
        "#1315: toolbox-create-batch-from-dispatch host command should append all batch objects");
    expect(visual_object_property(form_path, "first-dispatch-host-batch-guid", "CAPTION") ==
            "First Dispatch Host Batch" &&
            visual_object_property(form_path, "dispatch-host-batch-command-guid", "CAPTION") ==
            "Run Dispatch Host Batch" &&
            visual_object_property(form_path, "second-dispatch-host-batch-guid", "CAPTION") ==
            "Second Dispatch Host Batch",
        "#1315: toolbox-create-batch-from-dispatch host command should persist per-item fields");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "report_expression",
            "--object-name", "DetailBand",
            "--toolbox-item", "label",
            "--create-unique-id", "dispatch-host-batch-report-label-guid",
            "--field-value", "CAPTION=Batch Report Label",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#2138: report toolbox-create-batch-from-dispatch JSON command should exit successfully");
    expect_contains(report_process.stdout_text, "\"toolboxCreateBatchFromDispatch\": {",
        "#2138: report toolbox-create-batch-from-dispatch JSON should expose a stable result object");
    expect_contains(report_process.stdout_text, "\"batchPlanOk\": true",
        "#2138: report toolbox-create-batch-from-dispatch JSON should expose batch-plan state");
    expect_contains(report_process.stdout_text, "\"createResult\": {",
        "#2138: report toolbox-create-batch-from-dispatch JSON should expose lower-level create results");
    expect_contains(report_process.stdout_text, "\"toolboxContextProvided\": true",
        "#2138: report toolbox-create-batch-from-dispatch JSON should use dispatch toolbox contexts");
    expect_contains(report_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2138: report toolbox-create-batch-from-dispatch JSON should resolve report contexts");
    expect_contains(report_process.stdout_text, "\"itemCount\": 1",
        "#2138: report toolbox-create-batch-from-dispatch JSON should expose report batch item counts");
    expect_contains(report_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2138: report toolbox-create-batch-from-dispatch JSON should expose label batch plans");
    expect_contains(report_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2138: report toolbox-create-batch-from-dispatch JSON should expose generated label names");
    expect_contains(report_process.stdout_text, "\"createdObjectNames\": [\"lbl1\"]",
        "#2138: report toolbox-create-batch-from-dispatch JSON should summarize created report object names");
    expect_contains(report_process.stdout_text, "\"createdUniqueIds\": [\"dispatch-host-batch-report-label-guid\"]",
        "#2138: report toolbox-create-batch-from-dispatch JSON should summarize created report unique ids");
    expect_contains(report_process.stdout_text, "\"createErrors\": []",
        "#2138: report toolbox-create-batch-from-dispatch JSON should summarize empty create errors");
    expect_contains(report_process.stdout_text, "\"propertyValue\": \"Batch Report Label\"",
        "#2138: report toolbox-create-batch-from-dispatch JSON should preserve report label field values");
    expect_contains(report_process.stdout_text, "\"dryRun\": false",
        "#2138: report toolbox-create-batch-from-dispatch JSON should expose execution state");
    expect_contains(report_process.stdout_text, "\"mutatesAsset\": true",
        "#2138: report toolbox-create-batch-from-dispatch JSON should expose mutation state");
    expect_not_contains(report_process.stdout_text, "\"className\": \"TextBox\"",
        "#2138: report toolbox-create-batch-from-dispatch JSON should exclude form-only textbox metadata");
    expect(visual_object_count(form_path) == before_count + 4U,
        "#2138: report toolbox-create-batch-from-dispatch host command should append one object");
    expect(visual_object_property(form_path, "dispatch-host-batch-report-label-guid", "CAPTION") ==
            "Batch Report Label",
        "#2138: report toolbox-create-batch-from-dispatch host command should persist caller fields");

    const auto label_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "label_expression",
            "--object-name", "DetailBand",
            "--toolbox-item", "label",
            "--create-unique-id", "dispatch-host-batch-label-guid",
            "--create-parent-name", "DetailBand",
            "--field-value", "CAPTION=Batch Label",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(label_process.exit_code == 0,
        "#2132: label toolbox-create-batch-from-dispatch JSON command should exit successfully");
    expect_contains(label_process.stdout_text, "\"toolboxCreateBatchFromDispatch\": {",
        "#2132: label toolbox-create-batch-from-dispatch JSON should expose a stable result object");
    expect_contains(label_process.stdout_text, "\"batchPlanOk\": true",
        "#2132: label toolbox-create-batch-from-dispatch JSON should expose batch-plan state");
    expect_contains(label_process.stdout_text, "\"createResult\": {",
        "#2132: label toolbox-create-batch-from-dispatch JSON should expose lower-level create results");
    expect_contains(label_process.stdout_text, "\"toolboxContextProvided\": true",
        "#2132: label toolbox-create-batch-from-dispatch JSON should use dispatch toolbox contexts");
    expect_contains(label_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2132: label toolbox-create-batch-from-dispatch JSON should resolve report contexts");
    expect_contains(label_process.stdout_text, "\"itemCount\": 1",
        "#2132: label toolbox-create-batch-from-dispatch JSON should expose label batch item counts");
    expect_contains(label_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2132: label toolbox-create-batch-from-dispatch JSON should expose label batch plans");
    expect_contains(label_process.stdout_text, "\"objectName\": \"lbl2\"",
        "#2132: label toolbox-create-batch-from-dispatch JSON should expose generated label names");
    expect_contains(label_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2132: label toolbox-create-batch-from-dispatch JSON should preserve label parent overrides");
    expect_contains(label_process.stdout_text, "\"propertyValue\": \"Batch Label\"",
        "#2132: label toolbox-create-batch-from-dispatch JSON should preserve label field values");
    expect_contains(label_process.stdout_text, "\"createdObjectNames\": [\"lbl2\"]",
        "#2132: label toolbox-create-batch-from-dispatch JSON should summarize created label object names");
    expect_contains(label_process.stdout_text, "\"createdUniqueIds\": [\"dispatch-host-batch-label-guid\"]",
        "#2132: label toolbox-create-batch-from-dispatch JSON should summarize created label unique ids");
    expect_contains(label_process.stdout_text, "\"createErrors\": []",
        "#2132: label toolbox-create-batch-from-dispatch JSON should summarize empty create errors");
    expect_contains(label_process.stdout_text, "\"dryRun\": false",
        "#2132: label toolbox-create-batch-from-dispatch JSON should expose execution state");
    expect_contains(label_process.stdout_text, "\"mutatesAsset\": true",
        "#2132: label toolbox-create-batch-from-dispatch JSON should expose mutation state");
    expect_not_contains(label_process.stdout_text, "\"className\": \"TextBox\"",
        "#2132: label toolbox-create-batch-from-dispatch JSON should exclude form-only textbox metadata");
    expect(visual_object_count(form_path) == before_count + 5U,
        "#2132: label toolbox-create-batch-from-dispatch host command should append one object");
    expect(visual_object_property(form_path, "dispatch-host-batch-label-guid", "CAPTION") == "Batch Label",
        "#2132: label toolbox-create-batch-from-dispatch host command should persist caller fields");

    const std::size_t committed_count = visual_object_count(form_path);
    const auto non_admitted_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "visual_object",
            "--object-name", "frmCustomer",
            "--toolbox-item", "textbox",
            "--json"
        },
        temp_root);
    expect(non_admitted_process.exit_code == 4,
        "#1315: toolbox-create-batch-from-dispatch JSON should reject non-admitted dispatches");
    expect_contains(non_admitted_process.stdout_text, "\"batchPlan\": null",
        "#1315: non-admitted toolbox-create-batch-from-dispatch JSON should not expose stale batch plans");
    expect_contains(non_admitted_process.stdout_text,
        "A toolbox dispatch request requires an admitted non-dry-run invocation.",
        "#1315: non-admitted toolbox-create-batch-from-dispatch JSON should report dispatch errors");
    expect_contains(non_admitted_process.stdout_text, "\"createdObjectNames\": []",
        "#1385: non-admitted toolbox-create-batch-from-dispatch JSON should summarize no created object names");
    expect_contains(non_admitted_process.stdout_text, "\"createdUniqueIds\": []",
        "#1385: non-admitted toolbox-create-batch-from-dispatch JSON should summarize no created unique ids");
    expect_contains(non_admitted_process.stdout_text,
        "\"createErrors\": [\"A toolbox dispatch request requires an admitted non-dry-run invocation.\"",
        "#1385: non-admitted toolbox-create-batch-from-dispatch JSON should summarize create errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1315: non-admitted toolbox-create-batch-from-dispatch commands should not mutate assets");

    const auto unavailable_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "report_expression",
            "--object-name", "DetailBand",
            "--toolbox-item", "textbox",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(unavailable_process.exit_code == 4,
        "#1315: toolbox-create-batch-from-dispatch JSON should reject unavailable dispatch items");
    expect_contains(unavailable_process.stdout_text, "\"batchPlan\": null",
        "#1315: unavailable toolbox-create-batch-from-dispatch JSON should not expose stale batch plans");
    expect_contains(unavailable_process.stdout_text,
        "The requested toolbox item is not available in the admitted toolbox dispatch.",
        "#1315: unavailable toolbox-create-batch-from-dispatch JSON should report availability errors");
    expect_contains(unavailable_process.stdout_text, "\"createdObjectNames\": []",
        "#1385: unavailable toolbox-create-batch-from-dispatch JSON should summarize no created object names");
    expect_contains(unavailable_process.stdout_text, "\"createdUniqueIds\": []",
        "#1385: unavailable toolbox-create-batch-from-dispatch JSON should summarize no created unique ids");
    expect_contains(unavailable_process.stdout_text,
        "\"createErrors\": [\"The requested toolbox item is not available in the admitted toolbox dispatch.\"",
        "#1385: unavailable toolbox-create-batch-from-dispatch JSON should summarize create errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1315: unavailable toolbox-create-batch-from-dispatch commands should not mutate assets");

    const auto duplicate_identity_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "visual_object",
            "--object-name", "frmCustomer",
            "--toolbox-item", "textbox",
            "--create-unique-id", "duplicate-dispatch-host-batch-guid",
            "--toolbox-item", "commandbutton",
            "--create-unique-id", "duplicate-dispatch-host-batch-guid",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(duplicate_identity_process.exit_code == 4,
        "#1315: toolbox-create-batch-from-dispatch JSON should reject duplicate planned identities");
    expect_contains(duplicate_identity_process.stdout_text, "\"batchPlanOk\": false",
        "#1315: duplicate toolbox-create-batch-from-dispatch JSON should report failed batch planning");
    expect(visual_object_count(form_path) == committed_count,
        "#1315: duplicate toolbox-create-batch-from-dispatch commands should not mutate assets");

    const auto invalid_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "visual_object",
            "--object-name", "frmCustomer",
            "--toolbox-item", "textbox",
            "--create-unique-id", "invalid-field-dispatch-host-batch-guid",
            "--field-value", "UNKNOWN=Invalid",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(invalid_field_process.exit_code == 4,
        "#1315: toolbox-create-batch-from-dispatch JSON should reject lower-layer invalid fields");
    expect_contains(invalid_field_process.stdout_text, "\"batchPlanOk\": true",
        "#1315: invalid-field toolbox-create-batch-from-dispatch JSON should preserve successful planning");
    expect_contains(invalid_field_process.stdout_text,
        "The requested field was not found in the asset.",
        "#1315: invalid-field toolbox-create-batch-from-dispatch JSON should report lower-layer failures");
    expect_contains(invalid_field_process.stdout_text, "\"createdObjectNames\": []",
        "#1385: invalid-field toolbox-create-batch-from-dispatch JSON should summarize no created object names");
    expect_contains(invalid_field_process.stdout_text,
        "\"createErrors\": [\"The requested field was not found in the asset.\"",
        "#1385: invalid-field toolbox-create-batch-from-dispatch JSON should summarize create errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1315: invalid-field toolbox-create-batch-from-dispatch commands should not partially mutate assets");

    const auto missing_items_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(missing_items_process.exit_code == 2,
        "#1315: toolbox-create-batch-from-dispatch JSON should reject empty item lists");
    expect_contains(missing_items_process.stdout_text, "No toolbox item ids were provided.",
        "#1315: empty toolbox-create-batch-from-dispatch JSON should report parser errors");

    const auto orphan_item_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "visual_object",
            "--create-parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(orphan_item_option_process.exit_code == 2,
        "#1315: toolbox-create-batch-from-dispatch JSON should reject item options before items");
    expect_contains(orphan_item_option_process.stdout_text,
        "Toolbox batch item options require a preceding --toolbox-item.",
        "#1315: orphan toolbox-create-batch-from-dispatch item options should report parser errors");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--admit-palette-invocation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1315: toolbox-create-batch-from-dispatch JSON should reject invalid admission tokens");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-palette-invocation value must be true or false.",
        "#1315: invalid toolbox-create-batch-from-dispatch admission tokens should report parser errors");

    const auto malformed_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--field-value", "BROKEN",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(malformed_field_process.exit_code == 2,
        "#1315: toolbox-create-batch-from-dispatch JSON should reject malformed field values");
    expect_contains(malformed_field_process.stdout_text,
        "Toolbox field values must use name=value syntax.",
        "#1315: malformed toolbox-create-batch-from-dispatch field values should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch-from-dispatch",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1315: toolbox-create-batch-from-dispatch JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown toolbox-create-batch-from-dispatch option: --toolbox-context",
        "#1315: unknown toolbox-create-batch-from-dispatch options should report parser errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1315: rejected toolbox-create-batch-from-dispatch commands should not mutate assets");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
