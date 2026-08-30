void test_studio_host_json_creates_toolbox_objects(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);

    const auto create_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create", "textbox",
            "--unique-id", "created-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Customer",
            "--field-value", "PROPERTIES=ControlSource = \"customer.name\"",
            "--json"
        },
        temp_root);

    if (create_process.exit_code != 0) {
        std::cerr << "studio host toolbox-create stdout:\n" << create_process.stdout_text << "\n";
        std::cerr << "studio host toolbox-create stderr:\n" << create_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(create_process.exit_code == 0, "#1018: toolbox-create JSON command should exit successfully");
    expect_contains(create_process.stdout_text, "\"status\": \"ok\"",
                    "#1018: successful toolbox-create JSON should report ok status");
    expect_contains(create_process.stdout_text, "\"toolboxCreate\": {",
                    "#1018: toolbox-create JSON should use a stable result object");
    expect_contains(create_process.stdout_text, "\"ok\": true",
                    "#1018: toolbox-create JSON should expose result success");
    expect_contains(create_process.stdout_text, "\"recordIndex\": 2",
                    "#1018: toolbox-create JSON should expose appended record index");
    expect_contains(create_process.stdout_text, "\"objectName\": \"txt2\"",
                    "#1018: toolbox-create JSON should expose generated object name");
    expect_contains(create_process.stdout_text, "\"uniqueId\": \"created-textbox-guid\"",
                    "#1018: toolbox-create JSON should expose created unique id");
    expect_contains(create_process.stdout_text, "\"parentName\": \"frmCustomer\"",
                    "#1018: toolbox-create JSON should expose created parent name");
    expect_contains(create_process.stdout_text, "\"createdObjectNames\": [\"txt2\"]",
                    "#1382: toolbox-create JSON should summarize created object names");
    expect_contains(create_process.stdout_text, "\"createdUniqueIds\": [\"created-textbox-guid\"]",
                    "#1382: toolbox-create JSON should summarize created unique ids");
    expect_contains(create_process.stdout_text, "\"createErrors\": []",
                    "#1382: successful toolbox-create JSON should summarize empty create errors");

    const auto caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "created-textbox-guid",
        .property_name = "CAPTION"
    });
    expect(caption.ok && caption.exists && caption.value == "Customer",
        "#1018: toolbox-create host command should propagate extra direct fields");

    const auto control_source = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "created-textbox-guid",
        .property_name = "ControlSource"
    });
    expect(control_source.ok && control_source.exists && control_source.value == "\"customer.name\"",
        "#1018: toolbox-create host command should propagate extra memo fields");

    const std::size_t object_count_before_failure = visual_object_count(form_path);
    const auto failure_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create", "missing-toolbox-item",
            "--unique-id", "should-not-exist",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Should Not Exist",
            "--json"
        },
        temp_root);

    expect(failure_process.exit_code == 4, "#1018: unknown toolbox ids should return a command failure exit code");
    expect_contains(failure_process.stdout_text, "\"status\": \"error\"",
                    "#1018: failed toolbox-create JSON should report error status");
    expect_contains(failure_process.stdout_text, "\"ok\": false",
                    "#1018: failed toolbox-create JSON should expose result failure");
    expect_contains(failure_process.stdout_text, "\"error\": \"The requested toolbox item was not found.\"",
                    "#1018: failed toolbox-create JSON should expose clean error text");
    expect_contains(failure_process.stdout_text, "\"objectName\": \"\"",
                    "#1018: failed toolbox-create JSON should not report stale object names");
    expect_contains(failure_process.stdout_text, "\"createdObjectNames\": []",
                    "#1382: failed toolbox-create JSON should summarize no created object names");
    expect_contains(failure_process.stdout_text, "\"createdUniqueIds\": []",
                    "#1382: failed toolbox-create JSON should summarize no created unique ids");
    expect_contains(failure_process.stdout_text,
                    "\"createErrors\": [\"The requested toolbox item was not found.\"",
                    "#1382: failed toolbox-create JSON should summarize create errors");
    expect(visual_object_count(form_path) == object_count_before_failure,
        "#1018: failed toolbox-create host commands should not mutate the asset");

    const auto report_label_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create", "label",
            "--toolbox-context", "report",
            "--unique-id", "report-label-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Total",
            "--json"
        },
        temp_root);

    expect(report_label_process.exit_code == 0,
        "#1019: report-compatible toolbox items should create through host JSON when report context is requested");
    expect_contains(report_label_process.stdout_text, "\"objectName\": \"lbl1\"",
                    "#1019: report-compatible toolbox creates should expose generated label names");
    expect_contains(report_label_process.stdout_text, "\"uniqueId\": \"report-label-guid\"",
                    "#1019: report-compatible toolbox creates should expose created unique ids");
    expect_contains(report_label_process.stdout_text, "\"parentName\": \"DetailBand\"",
                    "#2104: report-compatible toolbox creates should preserve report label parents");
    expect_contains(report_label_process.stdout_text, "\"createdObjectNames\": [\"lbl1\"]",
                    "#1382: report toolbox-create JSON should summarize created report object names");
    expect_contains(report_label_process.stdout_text, "\"createdUniqueIds\": [\"report-label-guid\"]",
                    "#1382: report toolbox-create JSON should summarize created report unique ids");
    expect_not_contains(report_label_process.stdout_text, "\"className\": \"TextBox\"",
                    "#2104: report toolbox-create JSON should exclude form-only TextBox metadata");
    expect(visual_object_count(form_path) == object_count_before_failure + 1U,
        "#2104: report toolbox-create host command should mutate the asset exactly once");

    const auto report_label_caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "report-label-guid",
        .property_name = "CAPTION"
    });
    expect(report_label_caption.ok && report_label_caption.exists && report_label_caption.value == "Total",
        "#2104: report toolbox-create host command should persist caller report label fields");

    const std::size_t object_count_before_context_failure = visual_object_count(form_path);
    const auto report_textbox_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create", "textbox",
            "--toolbox-context", "report",
            "--unique-id", "report-textbox-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Should Not Exist",
            "--json"
        },
        temp_root);

    expect(report_textbox_process.exit_code == 4,
        "#1019: report-incompatible toolbox items should fail through host JSON when report context is requested");
    expect_contains(report_textbox_process.stdout_text, "\"status\": \"error\"",
                    "#1019: context-filtered toolbox failures should report JSON error status");
    expect_contains(
        report_textbox_process.stdout_text,
        "\"error\": \"The requested toolbox item is not available in the requested designer context.\"",
        "#1019: context-filtered toolbox failures should expose clean error text");
    expect(visual_object_count(form_path) == object_count_before_context_failure,
        "#1019: context-filtered toolbox failures should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
