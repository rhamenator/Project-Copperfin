void test_studio_host_json_deletes_visual_methods(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_method_delete_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "method-delete.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nFUNCTION CanSave\r\nRETURN .T.\r\nENDFUNC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1425: synthetic SCX table for visual method delete should be created");

    const auto delete_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-delete",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--method-name", "Click",
            "--json"
        },
        temp_root);
    expect(delete_process.exit_code == 0,
        "#1425: visual method delete JSON should exit successfully for existing methods");
    expect_contains(delete_process.stdout_text, "\"visualMethodDelete\": {",
        "#1425: visual method delete JSON should expose a delete object");
    expect_contains(delete_process.stdout_text, "\"affectedObjectCount\": 1",
        "#1425: visual method delete JSON should expose affected object counts");
    expect_contains(delete_process.stdout_text, "\"dryRun\": false",
        "#1425: visual method delete JSON should expose committed execution state");
    expect_contains(delete_process.stdout_text, "\"mutatesAsset\": true",
        "#1425: visual method delete JSON should expose mutation state");
    expect_contains(delete_process.stdout_text, "\"undoAvailable\": true",
        "#1425: visual method delete JSON should expose undo availability");
    auto click_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "Click"
    });
    expect(click_method.ok && !click_method.exists,
        "#1425: visual method delete host command should remove selected methods");
    auto can_save_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "CanSave"
    });
    expect(can_save_method.ok && can_save_method.exists && can_save_method.method.source_text == "RETURN .T.",
        "#1425: visual method delete host command should preserve unrelated methods");

    const auto missing_method_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-delete",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--method-name", "MissingMethod",
            "--json"
        },
        temp_root);
    expect(missing_method_process.exit_code == 4,
        "#1425: visual method delete JSON should reject missing methods");
    expect_contains(missing_method_process.stdout_text, "\"visualMethodDelete\": null",
        "#1425: missing-method visual method delete JSON should not expose a delete object");
    expect_contains(missing_method_process.stdout_text, "The requested method was not found.",
        "#1425: missing-method visual method delete JSON should report editor errors");
    can_save_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "CanSave"
    });
    expect(can_save_method.ok && can_save_method.exists,
        "#1425: failed visual method delete commands should not mutate unrelated methods");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-delete",
            "--unique-id", "save-guid",
            "--method-name", "CanSave",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1425: visual method delete JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualMethodDelete\": null",
        "#1425: missing-path visual method delete JSON should not expose a delete object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1425: missing-path visual method delete JSON should report parser errors");

    const auto missing_method_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-delete",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--json"
        },
        temp_root);
    expect(missing_method_name_process.exit_code == 2,
        "#1425: visual method delete JSON should reject missing method names");
    expect_contains(missing_method_name_process.stdout_text, "No method name was provided.",
        "#1425: missing method-name visual method delete JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-delete",
            "--path", form_path.string(),
            "--record", "-1",
            "--method-name", "CanSave",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1425: visual method delete JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1425: invalid-record visual method delete JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-delete",
            "--path", form_path.string(),
            "--object-name", "missingObject",
            "--method-name", "CanSave",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1425: visual method delete JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualMethodDelete\": null",
        "#1425: unresolved visual method delete JSON should not expose a delete object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1425: unresolved visual method delete JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-method-delete --path <asset>",
        "#1425: usage text should expose visual method delete commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
