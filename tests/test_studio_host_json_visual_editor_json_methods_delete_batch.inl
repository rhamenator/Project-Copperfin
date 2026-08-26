void test_studio_host_json_deletes_visual_method_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_method_delete_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "method-delete-batch.scx";
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
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nFUNCTION GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC\r\nPROCEDURE Init\r\nTHIS.Enabled = .T.\r\nENDPROC"
        },
        {
            "txtName",
            "nameBox",
            "name-guid",
            "PROCEDURE LostFocus\r\nTHISFORM.ValidateName()\r\nENDPROC\r\nFUNCTION Valid\r\nRETURN .T.\r\nENDFUNC"
        },
        {
            "lblStatus",
            "statusLabel",
            "status-guid",
            "PROCEDURE Paint\r\nTHIS.Refresh()\r\nENDPROC\r\nFUNCTION RefreshValue\r\nRETURN THIS.Caption\r\nENDFUNC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1430: synthetic SCX table for visual method delete batches should be created");

    const auto method_state = [&](const std::string& unique_id, const std::string& method_name) {
        return copperfin::vfp::query_visual_object_method({
            .path = form_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .method_name = method_name
        });
    };

    const auto delete_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-delete-batch",
            "--path", form_path.string(),
            "--method-name", "Click",
            "--unique-id", "save-guid",
            "--method-name", "LostFocus",
            "--object-name", "txtName",
            "--method-name", "RefreshValue",
            "--record", "2",
            "--json"
        },
        temp_root);
    expect(delete_batch_process.exit_code == 0,
        "#1430: visual method delete-batch JSON should exit successfully for valid batches");
    expect_contains(delete_batch_process.stdout_text, "\"visualMethodDeleteBatch\": {",
        "#1430: visual method delete-batch JSON should expose a batch delete object");
    expect_contains(delete_batch_process.stdout_text, "\"affectedObjectCount\": 3",
        "#1430: visual method delete-batch JSON should expose affected item counts");
    expect_contains(delete_batch_process.stdout_text, "\"dryRun\": false",
        "#1430: visual method delete-batch JSON should expose committed execution state");
    expect_contains(delete_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1430: visual method delete-batch JSON should expose mutation state");
    expect_contains(delete_batch_process.stdout_text, "\"undoAvailable\": true",
        "#1430: visual method delete-batch JSON should expose undo availability");
    auto save_click = method_state("save-guid", "Click");
    auto name_lost_focus = method_state("name-guid", "LostFocus");
    auto status_refresh_value = method_state("status-guid", "RefreshValue");
    auto save_init = method_state("save-guid", "Init");
    auto save_get_caption = method_state("save-guid", "GetCaption");
    auto name_valid = method_state("name-guid", "Valid");
    auto status_paint = method_state("status-guid", "Paint");
    expect(save_click.ok && !save_click.exists &&
            name_lost_focus.ok && !name_lost_focus.exists &&
            status_refresh_value.ok && !status_refresh_value.exists,
        "#1430: visual method delete-batch host command should delete all requested methods");
    expect(save_init.ok && save_init.exists &&
            save_get_caption.ok && save_get_caption.exists &&
            name_valid.ok && name_valid.exists &&
            status_paint.ok && status_paint.exists,
        "#1430: visual method delete-batch host command should preserve unrelated methods");

    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-delete-batch",
            "--path", form_path.string(),
            "--method-name", "Init",
            "--unique-id", "save-guid",
            "--method-name", "MissingMethod",
            "--unique-id", "save-guid",
            "--json"
        },
        temp_root);
    expect(rollback_process.exit_code == 4,
        "#1430: visual method delete-batch JSON should reject missing methods");
    expect_contains(rollback_process.stdout_text, "\"visualMethodDeleteBatch\": null",
        "#1430: failed visual method delete-batch JSON should not expose a batch delete object");
    expect_contains(rollback_process.stdout_text, "The requested method was not found.",
        "#1430: missing-method visual method delete-batch JSON should report editor errors");
    save_init = method_state("save-guid", "Init");
    expect(save_init.ok && save_init.exists,
        "#1430: failed visual method delete-batch commands should roll back earlier deletes");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-delete-batch",
            "--method-name", "Init",
            "--unique-id", "save-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1430: visual method delete-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualMethodDeleteBatch\": null",
        "#1430: missing-path visual method delete-batch JSON should not expose a batch delete object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1430: missing-path visual method delete-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-delete-batch",
            "--path", form_path.string(),
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1430: visual method delete-batch JSON should reject empty batches");
    expect_contains(no_items_process.stdout_text, "No method deletes were provided.",
        "#1430: empty visual method delete-batch JSON should report parser errors");

    const auto option_before_item_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-delete-batch",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--method-name", "Init",
            "--json"
        },
        temp_root);
    expect(option_before_item_process.exit_code == 2,
        "#1430: visual method delete-batch JSON should reject item options before method names");
    expect_contains(option_before_item_process.stdout_text,
        "Visual method delete batch item options require a preceding --method-name.",
        "#1430: option-before-item visual method delete-batch JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-delete-batch",
            "--path", form_path.string(),
            "--method-name", "Init",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1430: visual method delete-batch JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1430: invalid-record visual method delete-batch JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-delete-batch",
            "--path", form_path.string(),
            "--method-name", "Init",
            "--object-name", "missingObject",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1430: visual method delete-batch JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualMethodDeleteBatch\": null",
        "#1430: unresolved visual method delete-batch JSON should not expose a batch delete object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1430: unresolved visual method delete-batch JSON should report editor errors");
    save_init = method_state("save-guid", "Init");
    expect(save_init.ok && save_init.exists,
        "#1430: failed visual method delete-batch selection errors should not mutate methods");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-method-delete-batch --path <asset>",
        "#1430: usage text should expose visual method delete-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
