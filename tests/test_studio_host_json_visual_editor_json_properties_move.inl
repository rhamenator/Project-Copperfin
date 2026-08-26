void test_studio_host_json_moves_visual_properties(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_move_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = write_synthetic_form_table_for_property_rename(temp_root, "move.scx");

    const auto move_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", form_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--target-unique-id", "form-guid",
            "--target-property-name", "FormControlSource",
            "--json"
        },
        temp_root);
    expect(move_process.exit_code == 0,
        "#1439: visual property move JSON should exit successfully for memo-backed properties");
    expect_contains(move_process.stdout_text, "\"visualPropertyMove\": {",
        "#1439: visual property move JSON should expose a move object");
    expect_contains(move_process.stdout_text, "\"affectedObjectCount\": 1",
        "#1439: visual property move JSON should expose affected object counts");
    expect_contains(move_process.stdout_text, "\"dryRun\": false",
        "#1439: visual property move JSON should expose committed execution state");
    expect_contains(move_process.stdout_text, "\"mutatesAsset\": true",
        "#1439: visual property move JSON should expose mutation state");
    expect_contains(move_process.stdout_text, "\"undoAvailable\": true",
        "#1439: visual property move JSON should expose undo availability");
    expect(visual_object_property(form_path, "form-guid", "FormControlSource") == "\"customer.name\"" &&
            visual_object_property(form_path, "existing-textbox-guid", "ControlSource").empty() &&
            visual_object_property(form_path, "existing-textbox-guid", "Left") == "12",
        "#1439: visual property move host command should move values, clear sources, and preserve unrelated properties");

    const fs::path default_path = write_synthetic_form_table_for_property_rename(temp_root, "move_default.scx");
    const auto default_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", default_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(default_name_process.exit_code == 0,
        "#1439: visual property move JSON should default target property names from source properties");
    expect(visual_object_property(default_path, "form-guid", "Left") == "12" &&
            visual_object_property(default_path, "existing-textbox-guid", "Left").empty(),
        "#1439: visual property move host command should default target property names and clear sources");

    const auto collision_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", form_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--target-unique-id", "form-guid",
            "--target-property-name", "FormControlSource",
            "--json"
        },
        temp_root);
    expect(collision_process.exit_code == 4,
        "#1439: visual property move JSON should reject target collisions by default");
    expect_contains(collision_process.stdout_text, "\"visualPropertyMove\": null",
        "#1439: target-collision visual property move JSON should not expose a move object");
    expect_contains(collision_process.stdout_text, "The target object already has the requested property.",
        "#1439: target-collision visual property move JSON should report editor errors");
    expect(visual_object_property(form_path, "form-guid", "FormControlSource") == "\"customer.name\"" &&
            visual_object_property(form_path, "existing-textbox-guid", "Left") == "12",
        "#1439: failed visual property move commands should not mutate source or target properties");

    const auto replace_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
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
        "#1439: visual property move JSON should allow explicit replacement");
    expect(visual_object_property(form_path, "form-guid", "FormControlSource") == "12" &&
            visual_object_property(form_path, "existing-textbox-guid", "Left").empty(),
        "#1439: visual property move host command should replace targets and clear sources when requested");

    const fs::path self_path = write_synthetic_form_table_for_property_rename(temp_root, "move_self.scx");
    const auto self_move_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", self_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--target-unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(self_move_process.exit_code == 4,
        "#1439: visual property move JSON should reject self-moves");
    expect_contains(self_move_process.stdout_text, "\"visualPropertyMove\": null",
        "#1439: self-move visual property move JSON should not expose a move object");
    expect_contains(self_move_process.stdout_text, "The source property cannot be moved onto itself.",
        "#1439: self-move visual property move JSON should report editor errors");
    expect(visual_object_property(self_path, "existing-textbox-guid", "ControlSource") == "\"customer.name\"",
        "#1439: self-move visual property move commands should not mutate source properties");

    const fs::path missing_source_path = write_synthetic_form_table_for_property_rename(temp_root, "move_missing.scx");
    const auto missing_source_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", missing_source_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "MissingProperty",
            "--target-unique-id", "form-guid",
            "--target-property-name", "CopiedMissing",
            "--json"
        },
        temp_root);
    expect(missing_source_process.exit_code == 4,
        "#1439: visual property move JSON should reject missing source properties");
    expect_contains(missing_source_process.stdout_text, "\"visualPropertyMove\": null",
        "#1439: missing-source visual property move JSON should not expose a move object");
    expect_contains(missing_source_process.stdout_text, "The source property was not found.",
        "#1439: missing-source visual property move JSON should report editor errors");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1439: visual property move JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyMove\": null",
        "#1439: missing-path visual property move JSON should not expose a move object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1439: missing-path visual property move JSON should report parser errors");

    const auto missing_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", missing_source_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(missing_property_name_process.exit_code == 2,
        "#1439: visual property move JSON should reject missing property names");
    expect_contains(missing_property_name_process.stdout_text, "No property name was provided.",
        "#1439: missing property-name visual property move JSON should report parser errors");

    const auto invalid_source_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", missing_source_path.string(),
            "--source-record", "-1",
            "--property-name", "ControlSource",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(invalid_source_record_process.exit_code == 2,
        "#1439: visual property move JSON should reject invalid source record values");
    expect_contains(invalid_source_record_process.stdout_text,
        "The --source-record value must be a non-negative integer.",
        "#1439: invalid-source-record visual property move JSON should report parser errors");

    const auto invalid_target_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", missing_source_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--target-record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_target_record_process.exit_code == 2,
        "#1439: visual property move JSON should reject invalid target record values");
    expect_contains(invalid_target_record_process.stdout_text,
        "The --target-record value must be a non-negative integer.",
        "#1439: invalid-target-record visual property move JSON should report parser errors");

    const auto invalid_replace_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", missing_source_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--target-unique-id", "form-guid",
            "--replace-existing", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_replace_process.exit_code == 2,
        "#1439: visual property move JSON should reject invalid replace-existing values");
    expect_contains(invalid_replace_process.stdout_text,
        "The --replace-existing value must be true or false.",
        "#1439: invalid replace-existing visual property move JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", missing_source_path.string(),
            "--source-object-name", "missingObject",
            "--property-name", "ControlSource",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1439: visual property move JSON should reject unresolved source objects");
    expect_contains(missing_object_process.stdout_text, "\"visualPropertyMove\": null",
        "#1439: unresolved visual property move JSON should not expose a move object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1439: unresolved visual property move JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-move --path <asset>",
        "#1439: usage text should expose visual property move commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
void test_studio_host_json_moves_visual_property_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_move_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = write_synthetic_form_table_for_property_rename(temp_root, "move_batch.scx");

    const auto move_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
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
    expect(move_batch_process.exit_code == 0,
        "#1440: visual property move-batch JSON should exit successfully for valid batches");
    expect_contains(move_batch_process.stdout_text, "\"visualPropertyMoveBatch\": {",
        "#1440: visual property move-batch JSON should expose a batch move object");
    expect_contains(move_batch_process.stdout_text, "\"affectedObjectCount\": 2",
        "#1440: visual property move-batch JSON should expose affected item counts");
    expect_contains(move_batch_process.stdout_text, "\"dryRun\": false",
        "#1440: visual property move-batch JSON should expose committed execution state");
    expect_contains(move_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1440: visual property move-batch JSON should expose mutation state");
    expect_contains(move_batch_process.stdout_text, "\"undoAvailable\": true",
        "#1440: visual property move-batch JSON should expose undo availability");
    expect(visual_object_property(form_path, "form-guid", "FormControlSource") == "\"customer.name\"" &&
            visual_object_property(form_path, "form-guid", "FormLeft") == "12" &&
            visual_object_property(form_path, "existing-textbox-guid", "ControlSource").empty() &&
            visual_object_property(form_path, "existing-textbox-guid", "Left").empty(),
        "#1440: visual property move-batch host command should move all requested properties and clear sources");

    const fs::path rollback_path = write_synthetic_form_table_for_property_rename(temp_root, "move_batch_rollback.scx");
    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", rollback_path.string(),
            "--property-name", "ControlSource",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--target-property-name", "MovedControlSource",
            "--property-name", "MissingProperty",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--target-property-name", "MovedMissing",
            "--json"
        },
        temp_root);
    expect(rollback_process.exit_code == 4,
        "#1440: visual property move-batch JSON should reject missing source properties");
    expect_contains(rollback_process.stdout_text, "\"visualPropertyMoveBatch\": null",
        "#1440: failed visual property move-batch JSON should not expose a batch move object");
    expect_contains(rollback_process.stdout_text, "The source property was not found.",
        "#1440: missing-source visual property move-batch JSON should report editor errors");
    expect(visual_object_property(rollback_path, "form-guid", "MovedControlSource").empty() &&
            visual_object_property(rollback_path, "existing-textbox-guid", "ControlSource") == "\"customer.name\"",
        "#1440: failed visual property move-batch commands should roll back earlier moves");

    const fs::path collision_path = write_synthetic_form_table_for_property_rename(temp_root, "move_batch_collision.scx");
    const auto seed_result = copperfin::vfp::copy_visual_object_property({
        .path = collision_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "existing-textbox-guid",
        .source_property_name = "ControlSource",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "form-guid",
        .target_property_name = "FormControlSource",
        .replace_existing = false
    });
    expect(seed_result.ok,
        "#1440: synthetic visual property move-batch collision fixture should seed target properties");

    const auto collision_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", collision_path.string(),
            "--property-name", "Left",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--target-property-name", "FormControlSource",
            "--json"
        },
        temp_root);
    expect(collision_process.exit_code == 4,
        "#1440: visual property move-batch JSON should reject target collisions by default");
    expect_contains(collision_process.stdout_text, "\"visualPropertyMoveBatch\": null",
        "#1440: target-collision visual property move-batch JSON should not expose a batch move object");
    expect_contains(collision_process.stdout_text, "The target object already has the requested property.",
        "#1440: target-collision visual property move-batch JSON should report editor errors");
    expect(visual_object_property(collision_path, "form-guid", "FormControlSource") == "\"customer.name\"" &&
            visual_object_property(collision_path, "existing-textbox-guid", "Left") == "12",
        "#1440: failed visual property move-batch collision commands should not mutate source or target properties");

    const auto replace_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", collision_path.string(),
            "--property-name", "Left",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--target-property-name", "FormControlSource",
            "--replace-existing", "true",
            "--json"
        },
        temp_root);
    expect(replace_process.exit_code == 0,
        "#1440: visual property move-batch JSON should allow explicit replacement");
    expect(visual_object_property(collision_path, "form-guid", "FormControlSource") == "12" &&
            visual_object_property(collision_path, "existing-textbox-guid", "Left").empty(),
        "#1440: visual property move-batch host command should replace targets and clear sources when requested");

    const fs::path self_path = write_synthetic_form_table_for_property_rename(temp_root, "move_batch_self.scx");
    const auto self_move_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", self_path.string(),
            "--property-name", "ControlSource",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(self_move_process.exit_code == 4,
        "#1440: visual property move-batch JSON should reject self-moves");
    expect_contains(self_move_process.stdout_text, "\"visualPropertyMoveBatch\": null",
        "#1440: self-move visual property move-batch JSON should not expose a batch move object");
    expect_contains(self_move_process.stdout_text, "The source property cannot be moved onto itself.",
        "#1440: self-move visual property move-batch JSON should report editor errors");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--property-name", "ControlSource",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1440: visual property move-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyMoveBatch\": null",
        "#1440: missing-path visual property move-batch JSON should not expose a batch move object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1440: missing-path visual property move-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", form_path.string(),
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1440: visual property move-batch JSON should reject empty batches");
    expect_contains(no_items_process.stdout_text, "No property moves were provided.",
        "#1440: empty visual property move-batch JSON should report parser errors");

    const auto option_before_item_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", form_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(option_before_item_process.exit_code == 2,
        "#1440: visual property move-batch JSON should reject item options before property names");
    expect_contains(option_before_item_process.stdout_text,
        "Visual property move batch item options require a preceding --property-name.",
        "#1440: option-before-item visual property move-batch JSON should report parser errors");

    const auto invalid_source_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--source-record", "-1",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(invalid_source_record_process.exit_code == 2,
        "#1440: visual property move-batch JSON should reject invalid source record values");
    expect_contains(invalid_source_record_process.stdout_text,
        "The --source-record value must be a non-negative integer.",
        "#1440: invalid-source-record visual property move-batch JSON should report parser errors");

    const auto invalid_target_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--source-unique-id", "existing-textbox-guid",
            "--target-record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_target_record_process.exit_code == 2,
        "#1440: visual property move-batch JSON should reject invalid target record values");
    expect_contains(invalid_target_record_process.stdout_text,
        "The --target-record value must be a non-negative integer.",
        "#1440: invalid-target-record visual property move-batch JSON should report parser errors");

    const auto invalid_replace_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--replace-existing", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_replace_process.exit_code == 2,
        "#1440: visual property move-batch JSON should reject invalid replace-existing values");
    expect_contains(invalid_replace_process.stdout_text,
        "The --replace-existing value must be true or false.",
        "#1440: invalid replace-existing visual property move-batch JSON should report parser errors");

    const auto missing_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", form_path.string(),
            "--property-name", "",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(missing_property_name_process.exit_code == 4,
        "#1440: visual property move-batch JSON should reject missing property names");
    expect_contains(missing_property_name_process.stdout_text, "\"visualPropertyMoveBatch\": null",
        "#1440: missing property-name visual property move-batch JSON should not expose a batch move object");
    expect_contains(missing_property_name_process.stdout_text, "No property name was provided.",
        "#1440: missing property-name visual property move-batch JSON should report editor errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--source-object-name", "missingObject",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1440: visual property move-batch JSON should reject unresolved source objects");
    expect_contains(missing_object_process.stdout_text, "\"visualPropertyMoveBatch\": null",
        "#1440: unresolved visual property move-batch JSON should not expose a batch move object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1440: unresolved visual property move-batch JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-move-batch --path <asset>",
        "#1440: usage text should expose visual property move-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
