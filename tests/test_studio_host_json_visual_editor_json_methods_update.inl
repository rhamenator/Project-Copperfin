void test_studio_host_json_updates_visual_methods(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_method_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "method-update.scx";
    write_synthetic_form_table_for_visual_object_list(form_path);

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-update",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--method-name", "Click",
            "--method-kind", "procedure",
            "--method-source", "THISFORM.Save(.T.)",
            "--json"
        },
        temp_root);
    expect(update_process.exit_code == 0,
        "#1424: visual method update JSON should exit successfully for existing methods");
    expect_contains(update_process.stdout_text, "\"visualMethodUpdate\": {",
        "#1424: visual method update JSON should expose an update object");
    expect_contains(update_process.stdout_text, "\"affectedObjectCount\": 1",
        "#1424: visual method update JSON should expose affected object counts");
    expect_contains(update_process.stdout_text, "\"dryRun\": false",
        "#1424: visual method update JSON should expose committed execution state");
    expect_contains(update_process.stdout_text, "\"mutatesAsset\": true",
        "#1424: visual method update JSON should expose mutation state");
    expect_contains(update_process.stdout_text, "\"undoAvailable\": true",
        "#1424: visual method update JSON should expose undo availability");
    auto click_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "Click"
    });
    expect(click_method.ok && click_method.exists && click_method.method.source_text == "THISFORM.Save(.T.)",
        "#1424: visual method update host command should persist existing method source changes");

    const auto append_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-update",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--method-name", "CanCancel",
            "--method-kind", "function",
            "--method-source", "RETURN .F.",
            "--json"
        },
        temp_root);
    expect(append_process.exit_code == 0,
        "#1424: visual method update JSON should exit successfully for appended methods");
    auto appended_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "CanCancel"
    });
    expect(appended_method.ok && appended_method.exists &&
            appended_method.method.kind == "function" &&
            appended_method.method.source_text == "RETURN .F.",
        "#1424: visual method update host command should append new function methods");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-update",
            "--unique-id", "save-guid",
            "--method-name", "Click",
            "--method-kind", "procedure",
            "--method-source", "SHOULD_NOT_WRITE",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1424: visual method update JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualMethodUpdate\": null",
        "#1424: missing-path visual method update JSON should not expose an update object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1424: missing-path visual method update JSON should report parser errors");

    const auto missing_method_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-update",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--method-kind", "procedure",
            "--method-source", "SHOULD_NOT_WRITE",
            "--json"
        },
        temp_root);
    expect(missing_method_name_process.exit_code == 2,
        "#1424: visual method update JSON should reject missing method names");
    expect_contains(missing_method_name_process.stdout_text, "No method name was provided.",
        "#1424: missing method-name visual method update JSON should report parser errors");

    const auto missing_kind_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-update",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--method-name", "Click",
            "--method-source", "SHOULD_NOT_WRITE",
            "--json"
        },
        temp_root);
    expect(missing_kind_process.exit_code == 2,
        "#1424: visual method update JSON should reject missing method kinds");
    expect_contains(missing_kind_process.stdout_text, "No method kind was provided.",
        "#1424: missing method-kind visual method update JSON should report parser errors");

    const auto missing_source_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-update",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--method-name", "Click",
            "--method-kind", "procedure",
            "--json"
        },
        temp_root);
    expect(missing_source_process.exit_code == 2,
        "#1424: visual method update JSON should reject missing source text");
    expect_contains(missing_source_process.stdout_text, "No method source was provided.",
        "#1424: missing source visual method update JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-update",
            "--path", form_path.string(),
            "--record", "-1",
            "--method-name", "Click",
            "--method-kind", "procedure",
            "--method-source", "SHOULD_NOT_WRITE",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1424: visual method update JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1424: invalid-record visual method update JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-update",
            "--path", form_path.string(),
            "--object-name", "missingObject",
            "--method-name", "Click",
            "--method-kind", "procedure",
            "--method-source", "SHOULD_NOT_WRITE",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1424: visual method update JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualMethodUpdate\": null",
        "#1424: unresolved visual method update JSON should not expose an update object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1424: unresolved visual method update JSON should report editor errors");
    click_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "Click"
    });
    expect(click_method.ok && click_method.exists && click_method.method.source_text == "THISFORM.Save(.T.)",
        "#1424: failed visual method update commands should not mutate existing methods");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-method-update --path <asset>",
        "#1424: usage text should expose visual method update commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
