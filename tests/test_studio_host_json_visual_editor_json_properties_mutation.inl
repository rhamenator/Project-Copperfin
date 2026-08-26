void test_studio_host_json_updates_visual_property_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_update_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "update_batch.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);

    const auto update_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "BatchCaption",
            "--property-name", "ToolTipText",
            "--property-value", "Hover text",
            "--json"
        },
        temp_root);
    expect(update_batch_process.exit_code == 0,
        "#1446: visual property update-batch JSON should exit successfully for valid batches");
    expect_contains(update_batch_process.stdout_text, "\"visualPropertyUpdateBatch\": {",
        "#1446: visual property update-batch JSON should expose a batch update object");
    expect_contains(update_batch_process.stdout_text, "\"affectedObjectCount\": 1",
        "#1446: visual property update-batch JSON should expose affected object counts");
    expect_contains(update_batch_process.stdout_text, "\"dryRun\": false",
        "#1446: visual property update-batch JSON should expose committed execution state");
    expect_contains(update_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1446: visual property update-batch JSON should expose mutation state");
    expect_contains(update_batch_process.stdout_text, "\"undoAvailable\": true",
        "#1446: visual property update-batch JSON should expose undo availability");
    expect(visual_object_property(form_path, "existing-textbox-guid", "CAPTION") == "BatchCaption" &&
            visual_object_property(form_path, "existing-textbox-guid", "ToolTipText") == "Hover text",
        "#1446: visual property update-batch host command should update direct and memo-backed properties");

    const fs::path rollback_path = temp_root / "update_batch_rollback.scx";
    write_synthetic_form_table_for_toolbox_creation(rollback_path);
    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", rollback_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "ShouldRollback",
            "--property-name", "",
            "--property-value", "Noop",
            "--json"
        },
        temp_root);
    expect(rollback_process.exit_code == 4,
        "#1446: visual property update-batch JSON should reject missing property names");
    expect_contains(rollback_process.stdout_text, "\"visualPropertyUpdateBatch\": null",
        "#1446: failed visual property update-batch JSON should not expose a batch update object");
    expect_contains(rollback_process.stdout_text, "No property name was provided.",
        "#1446: missing-property visual property update-batch JSON should report editor errors");
    expect(visual_object_property(rollback_path, "existing-textbox-guid", "CAPTION") == "Existing",
        "#1446: failed visual property update-batch commands should roll back earlier property changes");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "MissingPath",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1446: visual property update-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyUpdateBatch\": null",
        "#1446: missing-path visual property update-batch JSON should not expose a batch update object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1446: missing-path visual property update-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1446: visual property update-batch JSON should reject empty batches");
    expect_contains(no_items_process.stdout_text, "No property changes were provided.",
        "#1446: empty visual property update-batch JSON should report parser errors");

    const auto option_before_item_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-value", "NoProperty",
            "--property-name", "CAPTION",
            "--json"
        },
        temp_root);
    expect(option_before_item_process.exit_code == 2,
        "#1446: visual property update-batch JSON should reject item options before property names");
    expect_contains(option_before_item_process.stdout_text,
        "Visual property update batch item options require a preceding --property-name.",
        "#1446: option-before-item visual property update-batch JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", form_path.string(),
            "--record", "-1",
            "--property-name", "CAPTION",
            "--property-value", "BadRecord",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1446: visual property update-batch JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1446: invalid-record visual property update-batch JSON should report parser errors");

    const fs::path missing_object_path = temp_root / "update_batch_missing_object.scx";
    write_synthetic_form_table_for_toolbox_creation(missing_object_path);
    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", missing_object_path.string(),
            "--object-name", "missingObject",
            "--property-name", "CAPTION",
            "--property-value", "ShouldNotWrite",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1446: visual property update-batch JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualPropertyUpdateBatch\": null",
        "#1446: unresolved visual property update-batch JSON should not expose a batch update object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1446: unresolved visual property update-batch JSON should report editor errors");
    expect(visual_object_property(missing_object_path, "existing-textbox-guid", "CAPTION") == "Existing",
        "#1446: unresolved visual property update-batch commands should not mutate properties");

    const fs::path alias_path = temp_root / "update_batch_alias.scx";
    write_synthetic_form_table_for_toolbox_creation(alias_path);
    const auto alias_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-property-update-batch",
            "--path", alias_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "AliasCaption",
            "--json"
        },
        temp_root);
    expect(alias_process.exit_code == 0,
        "#1446: visual object property update-batch alias should remain accepted");
    expect(visual_object_property(alias_path, "existing-textbox-guid", "CAPTION") == "AliasCaption",
        "#1446: visual object property update-batch alias should update selected properties");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-update-batch --path <asset>",
        "#1446: usage text should expose visual property update-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_properties_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_property_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);

    auto caption_value = [&]() {
        return copperfin::vfp::query_visual_object_property({
            .path = form_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "existing-textbox-guid",
            .property_name = "CAPTION"
        });
    };

    const auto object_name_clear_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--clear-property",
            "--object-name", "txt1",
            "--property-name", "CAPTION",
            "--json"
        },
        temp_root);
    expect(object_name_clear_process.exit_code == 0,
        "#1021: object-name host property clears should exit successfully");
    auto caption = caption_value();
    expect(caption.ok && caption.exists && caption.value.empty(),
        "#1021: object-name host property clears should empty direct-field properties");

    const auto set_before_unique_clear = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "BeforeUniqueClear",
            "--json"
        },
        temp_root);
    expect(set_before_unique_clear.exit_code == 0,
        "#1021: clear-property setup should be able to restore a direct-field value");

    const auto unique_id_clear_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--clear-property",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--json"
        },
        temp_root);
    expect(unique_id_clear_process.exit_code == 0,
        "#1021: unique-id host property clears should exit successfully");
    caption = caption_value();
    expect(caption.ok && caption.exists && caption.value.empty(),
        "#1021: unique-id host property clears should empty direct-field properties");

    const auto set_before_missing_clear = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "BeforeMissingClear",
            "--json"
        },
        temp_root);
    expect(set_before_missing_clear.exit_code == 0,
        "#1021: missing-clear setup should be able to restore a direct-field value");

    const auto missing_clear_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--clear-property",
            "--object-name", "missingObject",
            "--property-name", "CAPTION",
            "--json"
        },
        temp_root);
    expect(missing_clear_process.exit_code == 4,
        "#1021: missing object-name host property clears should return command failure");
    caption = caption_value();
    expect(caption.ok && caption.exists && caption.value == "BeforeMissingClear",
        "#1021: missing object-name host property clears should not mutate the asset");

    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--clear-property",
            "--property-name", "CAPTION",
            "--property-value", "Ambiguous",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1021: ambiguous set/clear property requests should fail during launch parsing");
    caption = caption_value();
    expect(caption.ok && caption.exists && caption.value == "BeforeMissingClear",
        "#1021: ambiguous set/clear property requests should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
