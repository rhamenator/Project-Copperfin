void test_studio_host_json_creates_selection_toolbox_objects(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_create_json_tests";
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
            "--selection-toolbox-create", "textbox",
            "--selection-context", "visual_object",
            "--unique-id", "selection-created-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Selection Created",
            "--field-value", "PROPERTIES=ControlSource = \"customer.name\"",
            "--json"
        },
        temp_root);

    if (create_process.exit_code != 0) {
        std::cerr << "studio host selection-toolbox-create stdout:\n" << create_process.stdout_text << "\n";
        std::cerr << "studio host selection-toolbox-create stderr:\n" << create_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(create_process.exit_code == 0,
        "#1309: selection-toolbox-create JSON command should exit successfully");
    expect_contains(create_process.stdout_text, "\"status\": \"ok\"",
        "#1309: successful selection-toolbox-create JSON should report ok status");
    expect_contains(create_process.stdout_text, "\"selectionToolboxCreate\": {",
        "#1309: selection-toolbox-create JSON should expose a stable result object");
    expect_contains(create_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1309: selection-toolbox-create JSON should expose selected contexts");
    expect_contains(create_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1309: selection-toolbox-create JSON should expose resolved toolbox contexts");
    expect_contains(create_process.stdout_text, "\"launchPlanOk\": true",
        "#1309: selection-toolbox-create JSON should expose launch state");
    expect_contains(create_process.stdout_text, "\"createPlanOk\": true",
        "#1309: selection-toolbox-create JSON should expose nested create-plan state");
    expect_contains(create_process.stdout_text, "\"createResult\": {",
        "#1309: selection-toolbox-create JSON should expose lower-level create results");
    expect_contains(create_process.stdout_text, "\"recordIndex\": 2",
        "#1309: selection-toolbox-create JSON should expose appended record index");
    expect_contains(create_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1309: selection-toolbox-create JSON should expose generated object names");
    expect_contains(create_process.stdout_text, "\"uniqueId\": \"selection-created-textbox-guid\"",
        "#1309: selection-toolbox-create JSON should expose created unique ids");
    expect_contains(create_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1309: selection-toolbox-create JSON should expose created parents");
    expect_contains(create_process.stdout_text, "\"createdObjectNames\": [\"txt2\"]",
        "#1382: selection-toolbox-create JSON should summarize created object names");
    expect_contains(create_process.stdout_text, "\"createdUniqueIds\": [\"selection-created-textbox-guid\"]",
        "#1382: selection-toolbox-create JSON should summarize created unique ids");
    expect_contains(create_process.stdout_text, "\"createErrors\": []",
        "#1382: successful selection-toolbox-create JSON should summarize empty create errors");
    expect_contains(create_process.stdout_text, "\"className\": \"TextBox\"",
        "#1309: selection-toolbox-create JSON should expose descriptor metadata");
    expect_contains(create_process.stdout_text, "\"propertyValue\": \"Selection Created\"",
        "#1309: selection-toolbox-create JSON should expose caller direct fields");
    expect_contains(create_process.stdout_text, "\"dryRun\": false",
        "#1309: selection-toolbox-create JSON should expose execution state");
    expect_contains(create_process.stdout_text, "\"mutatesAsset\": true",
        "#1309: selection-toolbox-create JSON should expose mutation state");
    expect(visual_object_count(form_path) == before_count + 1U,
        "#1309: selection-toolbox-create host command should mutate the asset exactly once");

    const auto caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-created-textbox-guid",
        .property_name = "CAPTION"
    });
    const auto control_source = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-created-textbox-guid",
        .property_name = "ControlSource"
    });
    expect(caption.ok && caption.exists && caption.value == "Selection Created" &&
            control_source.ok && control_source.exists && control_source.value == "\"customer.name\"",
        "#1309: selection-toolbox-create host command should persist caller fields");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create", "label",
            "--selection-context", "report_expression",
            "--unique-id", "selection-report-label-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Selection",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#1309: report selection-toolbox-create JSON command should exit successfully");
    expect_contains(report_process.stdout_text, "\"selectionToolboxCreate\": {",
        "#2115: report selection-toolbox-create JSON should expose a stable result object");
    expect_contains(report_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1309: report selection-toolbox-create JSON should expose report selections");
    expect_contains(report_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1309: report selection-toolbox-create JSON should resolve report contexts");
    expect_contains(report_process.stdout_text, "\"launchPlanOk\": true",
        "#2115: report selection-toolbox-create JSON should expose launch state");
    expect_contains(report_process.stdout_text, "\"createPlanOk\": true",
        "#2115: report selection-toolbox-create JSON should expose nested create-plan state");
    expect_contains(report_process.stdout_text, "\"createResult\": {",
        "#2115: report selection-toolbox-create JSON should expose lower-level create results");
    expect_contains(report_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#1309: report selection-toolbox-create JSON should expose generated label names");
    expect_contains(report_process.stdout_text, "\"uniqueId\": \"selection-report-label-guid\"",
        "#1309: report selection-toolbox-create JSON should expose report label unique ids");
    expect_contains(report_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2115: report selection-toolbox-create JSON should preserve report parent payloads");
    expect_contains(report_process.stdout_text, "\"createdObjectNames\": [\"lbl1\"]",
        "#1382: report selection-toolbox-create JSON should summarize created report object names");
    expect_contains(report_process.stdout_text, "\"createdUniqueIds\": [\"selection-report-label-guid\"]",
        "#1382: report selection-toolbox-create JSON should summarize created report unique ids");
    expect_contains(report_process.stdout_text, "\"createErrors\": []",
        "#2115: report selection-toolbox-create JSON should summarize empty create errors");
    expect_contains(report_process.stdout_text, "\"propertyValue\": \"Report Selection\"",
        "#2115: report selection-toolbox-create JSON should expose caller report fields");
    expect_contains(report_process.stdout_text, "\"dryRun\": false",
        "#2115: report selection-toolbox-create JSON should expose execution state");
    expect_contains(report_process.stdout_text, "\"mutatesAsset\": true",
        "#2115: report selection-toolbox-create JSON should expose mutation state");
    expect_not_contains(report_process.stdout_text, "\"className\": \"TextBox\"",
        "#1309: report selection-toolbox-create JSON should exclude form-only textbox metadata");
    expect(visual_object_count(form_path) == before_count + 2U,
        "#1309: report selection-toolbox-create host command should mutate the asset exactly once");

    const auto report_caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-report-label-guid",
        .property_name = "CAPTION"
    });
    expect(report_caption.ok && report_caption.exists && report_caption.value == "Report Selection",
        "#2115: report selection-toolbox-create host command should persist report label captions");

    const auto label_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create", "label",
            "--selection-context", "label_expression",
            "--unique-id", "selection-label-expression-label-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Label Selection",
            "--json"
        },
        temp_root);
    expect(label_process.exit_code == 0,
        "#2084: label selection-toolbox-create JSON command should exit successfully");
    expect_contains(label_process.stdout_text, "\"selectionToolboxCreate\": {",
        "#2129: label selection-toolbox-create JSON should expose a stable result object");
    expect_contains(label_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#2084: label selection-toolbox-create JSON should expose label selections");
    expect_contains(label_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2084: label selection-toolbox-create JSON should resolve report contexts");
    expect_contains(label_process.stdout_text, "\"launchPlanOk\": true",
        "#2129: label selection-toolbox-create JSON should expose launch state");
    expect_contains(label_process.stdout_text, "\"createPlanOk\": true",
        "#2129: label selection-toolbox-create JSON should expose nested create-plan state");
    expect_contains(label_process.stdout_text, "\"createResult\": {",
        "#2129: label selection-toolbox-create JSON should expose lower-level create results");
    expect_contains(label_process.stdout_text, "\"objectName\": \"lbl2\"",
        "#2084: label selection-toolbox-create JSON should expose generated label names");
    expect_contains(label_process.stdout_text, "\"uniqueId\": \"selection-label-expression-label-guid\"",
        "#2084: label selection-toolbox-create JSON should expose label unique ids");
    expect_contains(label_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2129: label selection-toolbox-create JSON should preserve label parent payloads");
    expect_contains(label_process.stdout_text, "\"createdObjectNames\": [\"lbl2\"]",
        "#2084: label selection-toolbox-create JSON should summarize created label object names");
    expect_contains(label_process.stdout_text, "\"createdUniqueIds\": [\"selection-label-expression-label-guid\"]",
        "#2084: label selection-toolbox-create JSON should summarize created label unique ids");
    expect_contains(label_process.stdout_text, "\"createErrors\": []",
        "#2129: label selection-toolbox-create JSON should summarize empty create errors");
    expect_contains(label_process.stdout_text, "\"propertyValue\": \"Label Selection\"",
        "#2129: label selection-toolbox-create JSON should expose caller label fields");
    expect_contains(label_process.stdout_text, "\"dryRun\": false",
        "#2129: label selection-toolbox-create JSON should expose execution state");
    expect_contains(label_process.stdout_text, "\"mutatesAsset\": true",
        "#2129: label selection-toolbox-create JSON should expose mutation state");
    expect_not_contains(label_process.stdout_text, "\"className\": \"TextBox\"",
        "#2084: label selection-toolbox-create JSON should exclude form-only textbox metadata");
    expect(visual_object_count(form_path) == before_count + 3U,
        "#2129: label selection-toolbox-create host command should mutate the asset exactly once");

    const auto label_caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-label-expression-label-guid",
        .property_name = "CAPTION"
    });
    expect(label_caption.ok && label_caption.exists && label_caption.value == "Label Selection",
        "#2129: label selection-toolbox-create host command should persist label captions");

    const std::size_t committed_count = visual_object_count(form_path);
    const auto unavailable_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create", "textbox",
            "--selection-context", "report_expression",
            "--unique-id", "selection-report-textbox-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Should Not Exist",
            "--json"
        },
        temp_root);
    expect(unavailable_process.exit_code == 4,
        "#1309: selection-toolbox-create JSON should reject unavailable selected-context items");
    expect_contains(unavailable_process.stdout_text, "\"status\": \"error\"",
        "#1309: unavailable selection-toolbox-create JSON should report error status");
    expect_contains(unavailable_process.stdout_text, "\"createPlanOk\": false",
        "#1309: unavailable selection-toolbox-create JSON should expose failed create-plan state");
    expect_contains(unavailable_process.stdout_text, "\"createResult\": {",
        "#1309: unavailable selection-toolbox-create JSON should expose clean create-result state");
    expect_contains(unavailable_process.stdout_text,
        "The requested toolbox item is not available in the requested designer context.",
        "#1309: unavailable selection-toolbox-create JSON should report planner errors");
    expect_contains(unavailable_process.stdout_text, "\"objectName\": \"\"",
        "#1309: unavailable selection-toolbox-create JSON should avoid stale object names");
    expect_contains(unavailable_process.stdout_text, "\"createdObjectNames\": []",
        "#1382: unavailable selection-toolbox-create JSON should summarize no created object names");
    expect_contains(unavailable_process.stdout_text, "\"createdUniqueIds\": []",
        "#1382: unavailable selection-toolbox-create JSON should summarize no created unique ids");
    expect_contains(unavailable_process.stdout_text,
        "\"createErrors\": [\"The requested toolbox item is not available in the requested designer context.\"",
        "#1382: unavailable selection-toolbox-create JSON should summarize create errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1309: unavailable selection-toolbox-create commands should not mutate the asset");

    const auto unsupported_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create", "textbox",
            "--selection-context", "menu_item",
            "--unique-id", "selection-menu-textbox-guid",
            "--json"
        },
        temp_root);
    expect(unsupported_process.exit_code == 4,
        "#1309: selection-toolbox-create JSON should reject unsupported selections");
    expect_contains(unsupported_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1309: unsupported selection-toolbox-create JSON should preserve selected contexts");
    expect_contains(unsupported_process.stdout_text, "\"launchPlanOk\": false",
        "#1309: unsupported selection-toolbox-create JSON should expose launch failures");
    expect_contains(unsupported_process.stdout_text,
        "A selection-context toolbox object creation plan request requires a toolbox palette.",
        "#1309: unsupported selection-toolbox-create JSON should report palette errors");
    expect_contains(unsupported_process.stdout_text, "\"objectName\": \"\"",
        "#1309: unsupported selection-toolbox-create JSON should avoid stale object names");
    expect(visual_object_count(form_path) == committed_count,
        "#1309: unsupported selection-toolbox-create commands should not mutate the asset");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create", "textbox",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1309: selection-toolbox-create JSON should reject missing paths");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1309: missing path selection-toolbox-create JSON should report parser errors");

    const auto missing_item_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(missing_item_process.exit_code == 2,
        "#1309: selection-toolbox-create JSON should reject missing item values");
    expect_contains(missing_item_process.stdout_text, "Missing value for --selection-toolbox-create.",
        "#1309: missing item selection-toolbox-create JSON should report parser errors");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create", "textbox",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1309: selection-toolbox-create JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1309: missing selection selection-toolbox-create JSON should report parser errors");

    const auto unknown_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create", "textbox",
            "--selection-context", "unknown_context",
            "--json"
        },
        temp_root);
    expect(unknown_selection_process.exit_code == 2,
        "#1309: selection-toolbox-create JSON should reject unknown selections");
    expect_contains(unknown_selection_process.stdout_text, "Unknown selection context token: unknown_context",
        "#1309: unknown selection selection-toolbox-create JSON should report parser errors");

    const auto malformed_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create", "textbox",
            "--selection-context", "visual_object",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(malformed_field_process.exit_code == 2,
        "#1309: selection-toolbox-create JSON should reject malformed field values");
    expect_contains(malformed_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1309: malformed selection-toolbox-create JSON should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create", "textbox",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1309: selection-toolbox-create JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-toolbox-create option: --toolbox-context",
        "#1309: unknown option selection-toolbox-create JSON should report parser errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1309: parser-rejected selection-toolbox-create commands should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
