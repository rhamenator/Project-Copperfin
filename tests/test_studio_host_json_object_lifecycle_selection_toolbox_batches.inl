void test_studio_host_json_creates_selection_toolbox_object_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_create_batch_json_tests";
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
            "--selection-toolbox-create-batch",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--unique-id", "selection-batch-host-first-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=First Selection Batch",
            "--toolbox-item", "commandbutton",
            "--object-name", "cmdSelectionBatchHost",
            "--unique-id", "selection-batch-host-command-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Run Selection Batch",
            "--toolbox-item", "textbox",
            "--unique-id", "selection-batch-host-second-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Second Selection Batch",
            "--json"
        },
        temp_root);
    expect(create_process.exit_code == 0,
        "#1311: selection-toolbox-create-batch JSON command should exit successfully");
    expect_contains(create_process.stdout_text, "\"status\": \"ok\"",
        "#1311: successful selection-toolbox-create-batch JSON should report ok status");
    expect_contains(create_process.stdout_text, "\"selectionToolboxCreateBatch\": {",
        "#1311: selection-toolbox-create-batch JSON should expose a stable result object");
    expect_contains(create_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1311: selection-toolbox-create-batch JSON should expose selected contexts");
    expect_contains(create_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1311: selection-toolbox-create-batch JSON should expose resolved toolbox contexts");
    expect_contains(create_process.stdout_text, "\"launchPlanOk\": true",
        "#1311: selection-toolbox-create-batch JSON should expose launch state");
    expect_contains(create_process.stdout_text, "\"batchPlanOk\": true",
        "#1311: selection-toolbox-create-batch JSON should expose nested batch-plan state");
    expect_contains(create_process.stdout_text, "\"createResult\": {",
        "#1311: selection-toolbox-create-batch JSON should expose lower-level create results");
    expect_contains(create_process.stdout_text, "\"recordIndexes\": [2, 3, 4]",
        "#1311: selection-toolbox-create-batch JSON should expose created record indexes");
    expect_contains(create_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1311: selection-toolbox-create-batch JSON should expose first generated names");
    expect_contains(create_process.stdout_text, "\"objectName\": \"cmdSelectionBatchHost\"",
        "#1311: selection-toolbox-create-batch JSON should expose explicit names");
    expect_contains(create_process.stdout_text, "\"objectName\": \"txt3\"",
        "#1311: selection-toolbox-create-batch JSON should reserve generated names");
    expect_contains(create_process.stdout_text, "\"uniqueId\": \"selection-batch-host-first-guid\"",
        "#1311: selection-toolbox-create-batch JSON should expose first unique ids");
    expect_contains(create_process.stdout_text, "\"uniqueId\": \"selection-batch-host-command-guid\"",
        "#1311: selection-toolbox-create-batch JSON should expose later unique ids");
    expect_contains(create_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1311: selection-toolbox-create-batch JSON should expose created parents");
    expect_contains(create_process.stdout_text,
        "\"createdObjectNames\": [\"txt2\", \"cmdSelectionBatchHost\", \"txt3\"]",
        "#1383: selection-toolbox-create-batch JSON should summarize created object names");
    expect_contains(create_process.stdout_text,
        "\"createdUniqueIds\": [\"selection-batch-host-first-guid\", \"selection-batch-host-command-guid\", \"selection-batch-host-second-guid\"]",
        "#1383: selection-toolbox-create-batch JSON should summarize created unique ids");
    expect_contains(create_process.stdout_text, "\"createErrors\": []",
        "#1383: successful selection-toolbox-create-batch JSON should summarize empty create errors");
    expect_contains(create_process.stdout_text, "\"className\": \"TextBox\"",
        "#1311: selection-toolbox-create-batch JSON should expose descriptor class names");
    expect_contains(create_process.stdout_text, "\"propertyValue\": \"First Selection Batch\"",
        "#1311: selection-toolbox-create-batch JSON should expose per-item field values");
    expect_contains(create_process.stdout_text, "\"dryRun\": false",
        "#1311: selection-toolbox-create-batch JSON should expose execution state");
    expect_contains(create_process.stdout_text, "\"mutatesAsset\": true",
        "#1311: selection-toolbox-create-batch JSON should expose mutation state");
    expect(visual_object_count(form_path) == before_count + 3U,
        "#1311: selection-toolbox-create-batch host command should mutate once per accepted item");

    const auto first_caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-batch-host-first-guid",
        .property_name = "CAPTION"
    });
    const auto command_caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-batch-host-command-guid",
        .property_name = "CAPTION"
    });
    const auto second_caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-batch-host-second-guid",
        .property_name = "CAPTION"
    });
    expect(first_caption.ok && first_caption.exists && first_caption.value == "First Selection Batch" &&
            command_caption.ok && command_caption.exists && command_caption.value == "Run Selection Batch" &&
            second_caption.ok && second_caption.exists && second_caption.value == "Second Selection Batch",
        "#1311: selection-toolbox-create-batch host command should persist caller fields");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch",
            "--selection-context", "report_expression",
            "--toolbox-item", "label",
            "--unique-id", "selection-batch-host-report-label-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Selection Batch",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#1311: report selection-toolbox-create-batch JSON command should exit successfully");
    expect_contains(report_process.stdout_text, "\"selectionToolboxCreateBatch\": {",
        "#2116: report selection-toolbox-create-batch JSON should expose a stable result object");
    expect_contains(report_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1311: report selection-toolbox-create-batch JSON should expose report selections");
    expect_contains(report_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1311: report selection-toolbox-create-batch JSON should resolve report contexts");
    expect_contains(report_process.stdout_text, "\"launchPlanOk\": true",
        "#2116: report selection-toolbox-create-batch JSON should expose launch state");
    expect_contains(report_process.stdout_text, "\"batchPlanOk\": true",
        "#2116: report selection-toolbox-create-batch JSON should expose nested batch-plan state");
    expect_contains(report_process.stdout_text, "\"createResult\": {",
        "#2116: report selection-toolbox-create-batch JSON should expose lower-level create results");
    expect_contains(report_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#1311: report selection-toolbox-create-batch JSON should expose generated labels");
    expect_contains(report_process.stdout_text, "\"uniqueId\": \"selection-batch-host-report-label-guid\"",
        "#1311: report selection-toolbox-create-batch JSON should expose label unique ids");
    expect_contains(report_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2116: report selection-toolbox-create-batch JSON should preserve report parent payloads");
    expect_contains(report_process.stdout_text, "\"createdObjectNames\": [\"lbl1\"]",
        "#1383: report selection-toolbox-create-batch JSON should summarize created report object names");
    expect_contains(report_process.stdout_text, "\"createdUniqueIds\": [\"selection-batch-host-report-label-guid\"]",
        "#1383: report selection-toolbox-create-batch JSON should summarize created report unique ids");
    expect_contains(report_process.stdout_text, "\"createErrors\": []",
        "#2116: report selection-toolbox-create-batch JSON should summarize empty create errors");
    expect_contains(report_process.stdout_text, "\"propertyValue\": \"Report Selection Batch\"",
        "#2116: report selection-toolbox-create-batch JSON should expose caller report fields");
    expect_contains(report_process.stdout_text, "\"dryRun\": false",
        "#2116: report selection-toolbox-create-batch JSON should expose execution state");
    expect_contains(report_process.stdout_text, "\"mutatesAsset\": true",
        "#2116: report selection-toolbox-create-batch JSON should expose mutation state");
    expect_not_contains(report_process.stdout_text, "\"className\": \"TextBox\"",
        "#1311: report selection-toolbox-create-batch JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count + 4U,
        "#2116: report selection-toolbox-create-batch host command should mutate the asset exactly once");

    const auto report_caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-batch-host-report-label-guid",
        .property_name = "CAPTION"
    });
    expect(report_caption.ok && report_caption.exists && report_caption.value == "Report Selection Batch",
        "#2116: report selection-toolbox-create-batch host command should persist report label captions");

    const auto label_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch",
            "--selection-context", "label_expression",
            "--toolbox-item", "label",
            "--unique-id", "selection-batch-label-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Label Selection Batch",
            "--json"
        },
        temp_root);
    expect(label_process.exit_code == 0,
        "#2085: label selection-toolbox-create-batch JSON command should exit successfully");
    expect_contains(label_process.stdout_text, "\"selectionToolboxCreateBatch\": {",
        "#2130: label selection-toolbox-create-batch JSON should expose a stable result object");
    expect_contains(label_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#2085: label selection-toolbox-create-batch JSON should expose label selections");
    expect_contains(label_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2085: label selection-toolbox-create-batch JSON should resolve report contexts");
    expect_contains(label_process.stdout_text, "\"launchPlanOk\": true",
        "#2130: label selection-toolbox-create-batch JSON should expose launch state");
    expect_contains(label_process.stdout_text, "\"batchPlanOk\": true",
        "#2130: label selection-toolbox-create-batch JSON should expose nested batch-plan state");
    expect_contains(label_process.stdout_text, "\"createResult\": {",
        "#2130: label selection-toolbox-create-batch JSON should expose lower-level create results");
    expect_contains(label_process.stdout_text, "\"objectName\": \"lbl2\"",
        "#2085: label selection-toolbox-create-batch JSON should expose generated labels");
    expect_contains(label_process.stdout_text, "\"uniqueId\": \"selection-batch-label-guid\"",
        "#2085: label selection-toolbox-create-batch JSON should expose label unique ids");
    expect_contains(label_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2130: label selection-toolbox-create-batch JSON should preserve label parent payloads");
    expect_contains(label_process.stdout_text, "\"createdObjectNames\": [\"lbl2\"]",
        "#2085: label selection-toolbox-create-batch JSON should summarize created label object names");
    expect_contains(label_process.stdout_text, "\"createdUniqueIds\": [\"selection-batch-label-guid\"]",
        "#2085: label selection-toolbox-create-batch JSON should summarize created label unique ids");
    expect_contains(label_process.stdout_text, "\"createErrors\": []",
        "#2130: label selection-toolbox-create-batch JSON should summarize empty create errors");
    expect_contains(label_process.stdout_text, "\"propertyValue\": \"Label Selection Batch\"",
        "#2130: label selection-toolbox-create-batch JSON should expose caller label fields");
    expect_contains(label_process.stdout_text, "\"dryRun\": false",
        "#2130: label selection-toolbox-create-batch JSON should expose execution state");
    expect_contains(label_process.stdout_text, "\"mutatesAsset\": true",
        "#2130: label selection-toolbox-create-batch JSON should expose mutation state");
    expect_not_contains(label_process.stdout_text, "\"className\": \"TextBox\"",
        "#2085: label selection-toolbox-create-batch JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count + 5U,
        "#2130: label selection-toolbox-create-batch host command should mutate the asset exactly once");

    const auto label_caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-batch-label-guid",
        .property_name = "CAPTION"
    });
    expect(label_caption.ok && label_caption.exists && label_caption.value == "Label Selection Batch",
        "#2130: label selection-toolbox-create-batch host command should persist label captions");

    const std::size_t committed_count = visual_object_count(form_path);
    const auto unavailable_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch",
            "--selection-context", "report_expression",
            "--toolbox-item", "textbox",
            "--unique-id", "selection-batch-host-report-textbox-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Should Not Exist",
            "--json"
        },
        temp_root);
    expect(unavailable_process.exit_code == 4,
        "#1311: selection-toolbox-create-batch JSON should reject unavailable selected-context items");
    expect_contains(unavailable_process.stdout_text, "\"status\": \"error\"",
        "#1311: unavailable selection-toolbox-create-batch JSON should report error status");
    expect_contains(unavailable_process.stdout_text, "\"batchPlanOk\": false",
        "#1311: unavailable selection-toolbox-create-batch JSON should expose failed batch plans");
    expect_contains(unavailable_process.stdout_text, "\"recordIndexes\": []",
        "#1311: unavailable selection-toolbox-create-batch JSON should avoid stale record indexes");
    expect_contains(unavailable_process.stdout_text,
        "The requested toolbox item is not available in the requested designer context.",
        "#1311: unavailable selection-toolbox-create-batch JSON should report planner errors");
    expect_contains(unavailable_process.stdout_text, "\"createdObjectNames\": []",
        "#1383: unavailable selection-toolbox-create-batch JSON should summarize no created object names");
    expect_contains(unavailable_process.stdout_text, "\"createdUniqueIds\": []",
        "#1383: unavailable selection-toolbox-create-batch JSON should summarize no created unique ids");
    expect_contains(unavailable_process.stdout_text,
        "\"createErrors\": [\"The requested toolbox item is not available in the requested designer context.\"",
        "#1383: unavailable selection-toolbox-create-batch JSON should summarize create errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1311: unavailable selection-toolbox-create-batch commands should not mutate assets");

    const auto unsupported_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch",
            "--selection-context", "menu_item",
            "--toolbox-item", "textbox",
            "--unique-id", "selection-batch-host-menu-guid",
            "--json"
        },
        temp_root);
    expect(unsupported_process.exit_code == 4,
        "#1311: selection-toolbox-create-batch JSON should reject unsupported selections");
    expect_contains(unsupported_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1311: unsupported selection-toolbox-create-batch JSON should preserve selected contexts");
    expect_contains(unsupported_process.stdout_text, "\"launchPlanOk\": false",
        "#1311: unsupported selection-toolbox-create-batch JSON should expose launch failures");
    expect_contains(unsupported_process.stdout_text,
        "A selection-context toolbox object batch creation plan request requires a toolbox palette.",
        "#1311: unsupported selection-toolbox-create-batch JSON should report palette errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1311: unsupported selection-toolbox-create-batch commands should not mutate assets");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-batch",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1311: selection-toolbox-create-batch JSON should reject missing paths");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1311: missing path selection-toolbox-create-batch JSON should report parser errors");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch",
            "--toolbox-item", "textbox",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1311: selection-toolbox-create-batch JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1311: missing selection selection-toolbox-create-batch JSON should report parser errors");

    const auto empty_items_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(empty_items_process.exit_code == 2,
        "#1311: selection-toolbox-create-batch JSON should reject empty item lists");
    expect_contains(empty_items_process.stdout_text, "No toolbox item ids were provided.",
        "#1311: empty selection-toolbox-create-batch item lists should report parser errors");

    const auto orphan_item_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch",
            "--selection-context", "visual_object",
            "--parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(orphan_item_option_process.exit_code == 2,
        "#1311: selection-toolbox-create-batch JSON should reject item options before items");
    expect_contains(orphan_item_option_process.stdout_text,
        "Selection toolbox batch create item options require a preceding --toolbox-item.",
        "#1311: orphan selection-toolbox-create-batch item options should report parser errors");

    const auto malformed_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(malformed_field_process.exit_code == 2,
        "#1311: selection-toolbox-create-batch JSON should reject malformed field values");
    expect_contains(malformed_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1311: malformed selection-toolbox-create-batch JSON should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1311: selection-toolbox-create-batch JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-toolbox-create-batch option: --toolbox-context",
        "#1311: unknown option selection-toolbox-create-batch JSON should report parser errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1311: parser-rejected selection-toolbox-create-batch commands should not mutate assets");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
