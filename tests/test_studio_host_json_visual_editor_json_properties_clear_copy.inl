void test_studio_host_json_clears_visual_properties(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path direct_path = temp_root / "direct_clear.scx";
    write_synthetic_form_table_for_toolbox_creation(direct_path);

    const auto direct_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-clear",
            "--path", direct_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--json"
        },
        temp_root);
    expect(direct_process.exit_code == 0,
        "#1435: visual property clear JSON should exit successfully for direct fields");
    expect_contains(direct_process.stdout_text, "\"visualPropertyClear\": {",
        "#1435: visual property clear JSON should expose a clear object");
    expect_contains(direct_process.stdout_text, "\"affectedObjectCount\": 1",
        "#1435: visual property clear JSON should expose affected object counts");
    expect_contains(direct_process.stdout_text, "\"dryRun\": false",
        "#1435: visual property clear JSON should expose committed execution state");
    expect_contains(direct_process.stdout_text, "\"mutatesAsset\": true",
        "#1435: visual property clear JSON should expose mutation state");
    expect_contains(direct_process.stdout_text, "\"undoAvailable\": true",
        "#1435: visual property clear JSON should expose undo availability");
    expect(visual_object_property(direct_path, "existing-textbox-guid", "CAPTION").empty(),
        "#1435: visual property clear host command should blank direct fields");

    const fs::path memo_path = write_synthetic_form_table_for_property_rename(temp_root, "memo_clear.scx");
    const auto memo_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-clear",
            "--path", memo_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--json"
        },
        temp_root);
    expect(memo_process.exit_code == 0,
        "#1435: visual property clear JSON should exit successfully for memo-backed properties");
    expect(visual_object_property(memo_path, "existing-textbox-guid", "ControlSource").empty() &&
            visual_object_property(memo_path, "existing-textbox-guid", "Left") == "12",
        "#1435: visual property clear host command should remove memo-backed assignments and preserve unrelated properties");

    const auto missing_property_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-clear",
            "--path", memo_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "MissingProperty",
            "--json"
        },
        temp_root);
    expect(missing_property_process.exit_code == 0,
        "#1435: visual property clear JSON should preserve existing idempotent missing-property clears");
    expect_contains(missing_property_process.stdout_text, "\"visualPropertyClear\": {",
        "#1435: missing-property visual property clear JSON should expose a successful clear object");
    expect(visual_object_property(memo_path, "existing-textbox-guid", "Left") == "12",
        "#1435: missing-property visual property clear commands should not mutate unrelated properties");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-clear",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1435: visual property clear JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyClear\": null",
        "#1435: missing-path visual property clear JSON should not expose a clear object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1435: missing-path visual property clear JSON should report parser errors");

    const auto missing_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-clear",
            "--path", direct_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(missing_property_name_process.exit_code == 2,
        "#1435: visual property clear JSON should reject missing property names");
    expect_contains(missing_property_name_process.stdout_text, "No property name was provided.",
        "#1435: missing property-name visual property clear JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-clear",
            "--path", direct_path.string(),
            "--record", "-1",
            "--property-name", "CAPTION",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1435: visual property clear JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1435: invalid-record visual property clear JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-clear",
            "--path", direct_path.string(),
            "--object-name", "missingObject",
            "--property-name", "CAPTION",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1435: visual property clear JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualPropertyClear\": null",
        "#1435: unresolved visual property clear JSON should not expose a clear object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1435: unresolved visual property clear JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-clear --path <asset>",
        "#1435: usage text should expose visual property clear commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_visual_property_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_clear_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = write_synthetic_form_table_for_property_rename(temp_root, "clear_batch.scx");

    const auto clear_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-clear-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(clear_batch_process.exit_code == 0,
        "#1437: visual property clear-batch JSON should exit successfully for valid batches");
    expect_contains(clear_batch_process.stdout_text, "\"visualPropertyClearBatch\": {",
        "#1437: visual property clear-batch JSON should expose a batch clear object");
    expect_contains(clear_batch_process.stdout_text, "\"affectedObjectCount\": 2",
        "#1437: visual property clear-batch JSON should expose affected item counts");
    expect_contains(clear_batch_process.stdout_text, "\"dryRun\": false",
        "#1437: visual property clear-batch JSON should expose committed execution state");
    expect_contains(clear_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1437: visual property clear-batch JSON should expose mutation state");
    expect_contains(clear_batch_process.stdout_text, "\"undoAvailable\": true",
        "#1437: visual property clear-batch JSON should expose undo availability");
    expect(visual_object_property(form_path, "existing-textbox-guid", "ControlSource").empty() &&
            visual_object_property(form_path, "existing-textbox-guid", "Left").empty(),
        "#1437: visual property clear-batch host command should clear all requested properties");

    const fs::path rollback_path = write_synthetic_form_table_for_property_rename(temp_root, "clear_batch_rollback.scx");
    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-clear-batch",
            "--path", rollback_path.string(),
            "--property-name", "ControlSource",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--object-name", "missingObject",
            "--json"
        },
        temp_root);
    expect(rollback_process.exit_code == 4,
        "#1437: visual property clear-batch JSON should reject unresolved selected objects");
    expect_contains(rollback_process.stdout_text, "\"visualPropertyClearBatch\": null",
        "#1437: failed visual property clear-batch JSON should not expose a batch clear object");
    expect_contains(rollback_process.stdout_text, "No visual object with the requested name was found.",
        "#1437: unresolved visual property clear-batch JSON should report editor errors");
    expect(visual_object_property(rollback_path, "existing-textbox-guid", "ControlSource") == "\"customer.name\"" &&
            visual_object_property(rollback_path, "existing-textbox-guid", "Left") == "12",
        "#1437: failed visual property clear-batch commands should roll back earlier clears");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-clear-batch",
            "--property-name", "ControlSource",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1437: visual property clear-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyClearBatch\": null",
        "#1437: missing-path visual property clear-batch JSON should not expose a batch clear object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1437: missing-path visual property clear-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-clear-batch",
            "--path", form_path.string(),
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1437: visual property clear-batch JSON should reject empty batches");
    expect_contains(no_items_process.stdout_text, "No property clears were provided.",
        "#1437: empty visual property clear-batch JSON should report parser errors");

    const auto option_before_item_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-clear-batch",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--json"
        },
        temp_root);
    expect(option_before_item_process.exit_code == 2,
        "#1437: visual property clear-batch JSON should reject item options before property names");
    expect_contains(option_before_item_process.stdout_text,
        "Visual property clear batch item options require a preceding --property-name.",
        "#1437: option-before-item visual property clear-batch JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-clear-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1437: visual property clear-batch JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1437: invalid-record visual property clear-batch JSON should report parser errors");

    const auto missing_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-clear-batch",
            "--path", form_path.string(),
            "--property-name", "",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(missing_property_name_process.exit_code == 4,
        "#1437: visual property clear-batch JSON should reject missing property names");
    expect_contains(missing_property_name_process.stdout_text, "\"visualPropertyClearBatch\": null",
        "#1437: missing property-name visual property clear-batch JSON should not expose a batch clear object");
    expect_contains(missing_property_name_process.stdout_text, "No property name was provided.",
        "#1437: missing property-name visual property clear-batch JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-clear-batch --path <asset>",
        "#1437: usage text should expose visual property clear-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_copies_visual_properties(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_copy_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = write_synthetic_form_table_for_property_rename(temp_root, "copy.scx");

    const auto copy_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy",
            "--path", form_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--target-unique-id", "form-guid",
            "--target-property-name", "FormControlSource",
            "--json"
        },
        temp_root);
    expect(copy_process.exit_code == 0,
        "#1436: visual property copy JSON should exit successfully for memo-backed properties");
    expect_contains(copy_process.stdout_text, "\"visualPropertyCopy\": {",
        "#1436: visual property copy JSON should expose a copy object");
    expect_contains(copy_process.stdout_text, "\"affectedObjectCount\": 1",
        "#1436: visual property copy JSON should expose affected object counts");
    expect_contains(copy_process.stdout_text, "\"dryRun\": false",
        "#1436: visual property copy JSON should expose committed execution state");
    expect_contains(copy_process.stdout_text, "\"mutatesAsset\": true",
        "#1436: visual property copy JSON should expose mutation state");
    expect_contains(copy_process.stdout_text, "\"undoAvailable\": true",
        "#1436: visual property copy JSON should expose undo availability");
    expect(visual_object_property(form_path, "form-guid", "FormControlSource") == "\"customer.name\"" &&
            visual_object_property(form_path, "existing-textbox-guid", "ControlSource") == "\"customer.name\"",
        "#1436: visual property copy host command should copy source values and preserve source properties");

    const auto default_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy",
            "--path", form_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(default_name_process.exit_code == 0,
        "#1436: visual property copy JSON should default target property names from source properties");
    expect(visual_object_property(form_path, "form-guid", "Left") == "12",
        "#1436: visual property copy host command should default target property names from sources");

    const auto collision_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy",
            "--path", form_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--target-unique-id", "form-guid",
            "--target-property-name", "FormControlSource",
            "--json"
        },
        temp_root);
    expect(collision_process.exit_code == 4,
        "#1436: visual property copy JSON should reject target collisions by default");
    expect_contains(collision_process.stdout_text, "\"visualPropertyCopy\": null",
        "#1436: target-collision visual property copy JSON should not expose a copy object");
    expect_contains(collision_process.stdout_text, "The target object already has the requested property.",
        "#1436: target-collision visual property copy JSON should report editor errors");
    expect(visual_object_property(form_path, "form-guid", "FormControlSource") == "\"customer.name\"",
        "#1436: failed visual property copy commands should not mutate target properties");

    const auto replace_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy",
            "--path", form_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--target-unique-id", "form-guid",
            "--target-property-name", "FormControlSource",
            "--replace-existing", "true",
            "--json"
        },
        temp_root);
    expect(replace_process.exit_code == 0,
        "#1436: visual property copy JSON should allow explicit replacement");
    expect(visual_object_property(form_path, "form-guid", "FormControlSource") == "12",
        "#1436: visual property copy host command should replace target values when requested");

    const auto missing_source_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy",
            "--path", form_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "MissingProperty",
            "--target-unique-id", "form-guid",
            "--target-property-name", "CopiedMissing",
            "--json"
        },
        temp_root);
    expect(missing_source_process.exit_code == 4,
        "#1436: visual property copy JSON should reject missing source properties");
    expect_contains(missing_source_process.stdout_text, "\"visualPropertyCopy\": null",
        "#1436: missing-source visual property copy JSON should not expose a copy object");
    expect_contains(missing_source_process.stdout_text, "The source property was not found.",
        "#1436: missing-source visual property copy JSON should report editor errors");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy",
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1436: visual property copy JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyCopy\": null",
        "#1436: missing-path visual property copy JSON should not expose a copy object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1436: missing-path visual property copy JSON should report parser errors");

    const auto missing_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy",
            "--path", form_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(missing_property_name_process.exit_code == 2,
        "#1436: visual property copy JSON should reject missing property names");
    expect_contains(missing_property_name_process.stdout_text, "No property name was provided.",
        "#1436: missing property-name visual property copy JSON should report parser errors");

    const auto invalid_source_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy",
            "--path", form_path.string(),
            "--source-record", "-1",
            "--property-name", "ControlSource",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(invalid_source_record_process.exit_code == 2,
        "#1436: visual property copy JSON should reject invalid source record values");
    expect_contains(invalid_source_record_process.stdout_text,
        "The --source-record value must be a non-negative integer.",
        "#1436: invalid-source-record visual property copy JSON should report parser errors");

    const auto invalid_replace_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy",
            "--path", form_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--target-unique-id", "form-guid",
            "--replace-existing", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_replace_process.exit_code == 2,
        "#1436: visual property copy JSON should reject invalid replace-existing values");
    expect_contains(invalid_replace_process.stdout_text,
        "The --replace-existing value must be true or false.",
        "#1436: invalid replace-existing visual property copy JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy",
            "--path", form_path.string(),
            "--source-object-name", "missingObject",
            "--property-name", "ControlSource",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1436: visual property copy JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualPropertyCopy\": null",
        "#1436: unresolved visual property copy JSON should not expose a copy object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1436: unresolved visual property copy JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-copy --path <asset>",
        "#1436: usage text should expose visual property copy commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_copies_visual_property_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_copy_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = write_synthetic_form_table_for_property_rename(temp_root, "copy_batch.scx");

    const auto copy_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--target-property-name", "FormControlSource",
            "--property-name", "Left",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--target-property-name", "FormLeft",
            "--json"
        },
        temp_root);
    expect(copy_batch_process.exit_code == 0,
        "#1438: visual property copy-batch JSON should exit successfully for valid batches");
    expect_contains(copy_batch_process.stdout_text, "\"visualPropertyCopyBatch\": {",
        "#1438: visual property copy-batch JSON should expose a batch copy object");
    expect_contains(copy_batch_process.stdout_text, "\"affectedObjectCount\": 2",
        "#1438: visual property copy-batch JSON should expose affected item counts");
    expect_contains(copy_batch_process.stdout_text, "\"dryRun\": false",
        "#1438: visual property copy-batch JSON should expose committed execution state");
    expect_contains(copy_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1438: visual property copy-batch JSON should expose mutation state");
    expect_contains(copy_batch_process.stdout_text, "\"undoAvailable\": true",
        "#1438: visual property copy-batch JSON should expose undo availability");
    expect(visual_object_property(form_path, "form-guid", "FormControlSource") == "\"customer.name\"" &&
            visual_object_property(form_path, "form-guid", "FormLeft") == "12" &&
            visual_object_property(form_path, "existing-textbox-guid", "ControlSource") == "\"customer.name\"" &&
            visual_object_property(form_path, "existing-textbox-guid", "Left") == "12",
        "#1438: visual property copy-batch host command should copy all requested properties and preserve sources");

    const fs::path rollback_path = write_synthetic_form_table_for_property_rename(temp_root, "copy_batch_rollback.scx");
    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy-batch",
            "--path", rollback_path.string(),
            "--property-name", "ControlSource",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--target-property-name", "CopiedControlSource",
            "--property-name", "MissingProperty",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--target-property-name", "CopiedMissing",
            "--json"
        },
        temp_root);
    expect(rollback_process.exit_code == 4,
        "#1438: visual property copy-batch JSON should reject missing source properties");
    expect_contains(rollback_process.stdout_text, "\"visualPropertyCopyBatch\": null",
        "#1438: failed visual property copy-batch JSON should not expose a batch copy object");
    expect_contains(rollback_process.stdout_text, "The source property was not found.",
        "#1438: missing-source visual property copy-batch JSON should report editor errors");
    expect(visual_object_property(rollback_path, "form-guid", "CopiedControlSource").empty() &&
            visual_object_property(rollback_path, "existing-textbox-guid", "ControlSource") == "\"customer.name\"",
        "#1438: failed visual property copy-batch commands should roll back earlier copies");

    const auto collision_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy-batch",
            "--path", form_path.string(),
            "--property-name", "Left",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--target-property-name", "FormLeft",
            "--json"
        },
        temp_root);
    expect(collision_process.exit_code == 4,
        "#1438: visual property copy-batch JSON should reject target collisions by default");
    expect_contains(collision_process.stdout_text, "\"visualPropertyCopyBatch\": null",
        "#1438: target-collision visual property copy-batch JSON should not expose a batch copy object");
    expect_contains(collision_process.stdout_text, "The target object already has the requested property.",
        "#1438: target-collision visual property copy-batch JSON should report editor errors");

    const auto replace_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy-batch",
            "--path", form_path.string(),
            "--property-name", "Left",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--target-property-name", "FormControlSource",
            "--replace-existing", "true",
            "--json"
        },
        temp_root);
    expect(replace_process.exit_code == 0,
        "#1438: visual property copy-batch JSON should allow explicit replacement");
    expect(visual_object_property(form_path, "form-guid", "FormControlSource") == "12",
        "#1438: visual property copy-batch host command should replace targets when requested");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy-batch",
            "--property-name", "ControlSource",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1438: visual property copy-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyCopyBatch\": null",
        "#1438: missing-path visual property copy-batch JSON should not expose a batch copy object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1438: missing-path visual property copy-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy-batch",
            "--path", form_path.string(),
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1438: visual property copy-batch JSON should reject empty batches");
    expect_contains(no_items_process.stdout_text, "No property copies were provided.",
        "#1438: empty visual property copy-batch JSON should report parser errors");

    const auto option_before_item_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy-batch",
            "--path", form_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(option_before_item_process.exit_code == 2,
        "#1438: visual property copy-batch JSON should reject item options before property names");
    expect_contains(option_before_item_process.stdout_text,
        "Visual property copy batch item options require a preceding --property-name.",
        "#1438: option-before-item visual property copy-batch JSON should report parser errors");

    const auto invalid_source_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--source-record", "-1",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(invalid_source_record_process.exit_code == 2,
        "#1438: visual property copy-batch JSON should reject invalid source record values");
    expect_contains(invalid_source_record_process.stdout_text,
        "The --source-record value must be a non-negative integer.",
        "#1438: invalid-source-record visual property copy-batch JSON should report parser errors");

    const auto invalid_target_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--source-unique-id", "existing-textbox-guid",
            "--target-record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_target_record_process.exit_code == 2,
        "#1438: visual property copy-batch JSON should reject invalid target record values");
    expect_contains(invalid_target_record_process.stdout_text,
        "The --target-record value must be a non-negative integer.",
        "#1438: invalid-target-record visual property copy-batch JSON should report parser errors");

    const auto invalid_replace_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--replace-existing", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_replace_process.exit_code == 2,
        "#1438: visual property copy-batch JSON should reject invalid replace-existing values");
    expect_contains(invalid_replace_process.stdout_text,
        "The --replace-existing value must be true or false.",
        "#1438: invalid replace-existing visual property copy-batch JSON should report parser errors");

    const auto missing_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy-batch",
            "--path", form_path.string(),
            "--property-name", "",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(missing_property_name_process.exit_code == 4,
        "#1438: visual property copy-batch JSON should reject missing property names");
    expect_contains(missing_property_name_process.stdout_text, "\"visualPropertyCopyBatch\": null",
        "#1438: missing property-name visual property copy-batch JSON should not expose a batch copy object");
    expect_contains(missing_property_name_process.stdout_text, "No property name was provided.",
        "#1438: missing property-name visual property copy-batch JSON should report editor errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-copy-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--source-object-name", "missingObject",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1438: visual property copy-batch JSON should reject unresolved source objects");
    expect_contains(missing_object_process.stdout_text, "\"visualPropertyCopyBatch\": null",
        "#1438: unresolved visual property copy-batch JSON should not expose a batch copy object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1438: unresolved visual property copy-batch JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-copy-batch --path <asset>",
        "#1438: usage text should expose visual property copy-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
