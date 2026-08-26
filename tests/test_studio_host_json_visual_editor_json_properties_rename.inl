void test_studio_host_json_renames_visual_properties(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_rename_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = write_synthetic_form_table_for_property_rename(temp_root, "rename.scx");

    const auto rename_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--new-property-name", "BoundControlSource",
            "--json"
        },
        temp_root);
    expect(rename_process.exit_code == 0,
        "#1441: visual property rename JSON should exit successfully for memo-backed properties");
    expect_contains(rename_process.stdout_text, "\"visualPropertyRename\": {",
        "#1441: visual property rename JSON should expose a rename object");
    expect_contains(rename_process.stdout_text, "\"affectedObjectCount\": 1",
        "#1441: visual property rename JSON should expose affected object counts");
    expect_contains(rename_process.stdout_text, "\"dryRun\": false",
        "#1441: visual property rename JSON should expose committed execution state");
    expect_contains(rename_process.stdout_text, "\"mutatesAsset\": true",
        "#1441: visual property rename JSON should expose mutation state");
    expect_contains(rename_process.stdout_text, "\"undoAvailable\": true",
        "#1441: visual property rename JSON should expose undo availability");
    expect(visual_object_property(form_path, "existing-textbox-guid", "BoundControlSource") == "\"customer.name\"" &&
            visual_object_property(form_path, "existing-textbox-guid", "ControlSource").empty() &&
            visual_object_property(form_path, "existing-textbox-guid", "Left") == "12",
        "#1441: visual property rename host command should rename values and preserve unrelated properties");

    const auto collision_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--new-property-name", "BoundControlSource",
            "--json"
        },
        temp_root);
    expect(collision_process.exit_code == 4,
        "#1441: visual property rename JSON should reject target collisions");
    expect_contains(collision_process.stdout_text, "\"visualPropertyRename\": null",
        "#1441: target-collision visual property rename JSON should not expose a rename object");
    expect_contains(collision_process.stdout_text, "The target property already exists in the selected object.",
        "#1441: target-collision visual property rename JSON should report editor errors");
    expect(visual_object_property(form_path, "existing-textbox-guid", "BoundControlSource") == "\"customer.name\"" &&
            visual_object_property(form_path, "existing-textbox-guid", "Left") == "12",
        "#1441: failed visual property rename commands should not mutate source or target properties");

    const fs::path same_name_path = write_synthetic_form_table_for_property_rename(temp_root, "rename_same.scx");
    const auto same_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--path", same_name_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--new-property-name", "controlsource",
            "--json"
        },
        temp_root);
    expect(same_name_process.exit_code == 4,
        "#1441: visual property rename JSON should reject same-name renames");
    expect_contains(same_name_process.stdout_text, "The source property cannot be renamed to itself.",
        "#1441: same-name visual property rename JSON should report editor errors");
    expect(visual_object_property(same_name_path, "existing-textbox-guid", "ControlSource") == "\"customer.name\"",
        "#1441: same-name visual property rename commands should not mutate source properties");

    const fs::path missing_source_path = write_synthetic_form_table_for_property_rename(temp_root, "rename_missing.scx");
    const auto missing_source_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--path", missing_source_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "MissingProperty",
            "--new-property-name", "RenamedMissing",
            "--json"
        },
        temp_root);
    expect(missing_source_process.exit_code == 4,
        "#1441: visual property rename JSON should reject missing source properties");
    expect_contains(missing_source_process.stdout_text, "\"visualPropertyRename\": null",
        "#1441: missing-source visual property rename JSON should not expose a rename object");
    expect_contains(missing_source_process.stdout_text, "The source property was not found.",
        "#1441: missing-source visual property rename JSON should report editor errors");

    const fs::path direct_path = temp_root / "rename_direct.scx";
    write_synthetic_form_table_for_toolbox_creation(direct_path);
    const auto direct_field_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--path", direct_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--new-property-name", "DisplayCaption",
            "--json"
        },
        temp_root);
    expect(direct_field_process.exit_code == 4,
        "#1441: visual property rename JSON should reject direct DBF-backed fields");
    expect_contains(direct_field_process.stdout_text, "\"visualPropertyRename\": null",
        "#1441: direct-field visual property rename JSON should not expose a rename object");
    expect_contains(direct_field_process.stdout_text, "Direct DBF-backed fields cannot be renamed per object.",
        "#1441: direct-field visual property rename JSON should report editor errors");
    expect(visual_object_property(direct_path, "existing-textbox-guid", "CAPTION") == "Existing",
        "#1441: direct-field visual property rename commands should not mutate direct fields");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--new-property-name", "BoundControlSource",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1441: visual property rename JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyRename\": null",
        "#1441: missing-path visual property rename JSON should not expose a rename object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1441: missing-path visual property rename JSON should report parser errors");

    const auto missing_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--path", missing_source_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--new-property-name", "BoundControlSource",
            "--json"
        },
        temp_root);
    expect(missing_property_name_process.exit_code == 2,
        "#1441: visual property rename JSON should reject missing property names");
    expect_contains(missing_property_name_process.stdout_text, "No property name was provided.",
        "#1441: missing property-name visual property rename JSON should report parser errors");

    const auto missing_new_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--path", missing_source_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--json"
        },
        temp_root);
    expect(missing_new_property_name_process.exit_code == 2,
        "#1441: visual property rename JSON should reject missing target property names");
    expect_contains(missing_new_property_name_process.stdout_text, "No target property name was provided.",
        "#1441: missing target property-name visual property rename JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--path", missing_source_path.string(),
            "--record", "-1",
            "--property-name", "ControlSource",
            "--new-property-name", "BoundControlSource",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1441: visual property rename JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1441: invalid-record visual property rename JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--path", missing_source_path.string(),
            "--object-name", "missingObject",
            "--property-name", "ControlSource",
            "--new-property-name", "BoundControlSource",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1441: visual property rename JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualPropertyRename\": null",
        "#1441: unresolved visual property rename JSON should not expose a rename object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1441: unresolved visual property rename JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-rename --path <asset>",
        "#1441: usage text should expose visual property rename commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
void test_studio_host_json_renames_visual_property_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_rename_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = write_synthetic_form_table_for_property_rename(temp_root, "rename_batch.scx");

    const auto rename_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--new-property-name", "BoundControlSource",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--new-property-name", "DisplayLeft",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(rename_batch_process.exit_code == 0,
        "#1442: visual property rename-batch JSON should exit successfully for valid batches");
    expect_contains(rename_batch_process.stdout_text, "\"visualPropertyRenameBatch\": {",
        "#1442: visual property rename-batch JSON should expose a batch rename object");
    expect_contains(rename_batch_process.stdout_text, "\"affectedObjectCount\": 2",
        "#1442: visual property rename-batch JSON should expose affected item counts");
    expect_contains(rename_batch_process.stdout_text, "\"dryRun\": false",
        "#1442: visual property rename-batch JSON should expose committed execution state");
    expect_contains(rename_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1442: visual property rename-batch JSON should expose mutation state");
    expect_contains(rename_batch_process.stdout_text, "\"undoAvailable\": true",
        "#1442: visual property rename-batch JSON should expose undo availability");
    expect(visual_object_property(form_path, "existing-textbox-guid", "BoundControlSource") == "\"customer.name\"" &&
            visual_object_property(form_path, "existing-textbox-guid", "DisplayLeft") == "12" &&
            visual_object_property(form_path, "existing-textbox-guid", "ControlSource").empty() &&
            visual_object_property(form_path, "existing-textbox-guid", "Left").empty(),
        "#1442: visual property rename-batch host command should rename all requested properties");

    const fs::path rollback_path = write_synthetic_form_table_for_property_rename(temp_root, "rename_batch_rollback.scx");
    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", rollback_path.string(),
            "--property-name", "ControlSource",
            "--new-property-name", "BoundControlSource",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "MissingProperty",
            "--new-property-name", "RenamedMissing",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(rollback_process.exit_code == 4,
        "#1442: visual property rename-batch JSON should reject missing source properties");
    expect_contains(rollback_process.stdout_text, "\"visualPropertyRenameBatch\": null",
        "#1442: failed visual property rename-batch JSON should not expose a batch rename object");
    expect_contains(rollback_process.stdout_text, "The source property was not found.",
        "#1442: missing-source visual property rename-batch JSON should report editor errors");
    expect(visual_object_property(rollback_path, "existing-textbox-guid", "BoundControlSource").empty() &&
            visual_object_property(rollback_path, "existing-textbox-guid", "ControlSource") == "\"customer.name\"",
        "#1442: failed visual property rename-batch commands should roll back earlier renames");

    const fs::path collision_path = write_synthetic_form_table_for_property_rename(temp_root, "rename_batch_collision.scx");
    const auto collision_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", collision_path.string(),
            "--property-name", "ControlSource",
            "--new-property-name", "Left",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(collision_process.exit_code == 4,
        "#1442: visual property rename-batch JSON should reject target collisions");
    expect_contains(collision_process.stdout_text, "\"visualPropertyRenameBatch\": null",
        "#1442: target-collision visual property rename-batch JSON should not expose a batch rename object");
    expect_contains(collision_process.stdout_text, "The target property already exists in the selected object.",
        "#1442: target-collision visual property rename-batch JSON should report editor errors");
    expect(visual_object_property(collision_path, "existing-textbox-guid", "ControlSource") == "\"customer.name\"" &&
            visual_object_property(collision_path, "existing-textbox-guid", "Left") == "12",
        "#1442: failed visual property rename-batch collision commands should not mutate source or target properties");

    const fs::path direct_path = temp_root / "rename_batch_direct.scx";
    write_synthetic_form_table_for_toolbox_creation(direct_path);
    const auto direct_field_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", direct_path.string(),
            "--property-name", "CAPTION",
            "--new-property-name", "DisplayCaption",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(direct_field_process.exit_code == 4,
        "#1442: visual property rename-batch JSON should reject direct DBF-backed fields");
    expect_contains(direct_field_process.stdout_text, "\"visualPropertyRenameBatch\": null",
        "#1442: direct-field visual property rename-batch JSON should not expose a batch rename object");
    expect_contains(direct_field_process.stdout_text, "Direct DBF-backed fields cannot be renamed per object.",
        "#1442: direct-field visual property rename-batch JSON should report editor errors");

    const fs::path same_name_path = write_synthetic_form_table_for_property_rename(temp_root, "rename_batch_same.scx");
    const auto same_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", same_name_path.string(),
            "--property-name", "ControlSource",
            "--new-property-name", "controlsource",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(same_name_process.exit_code == 4,
        "#1442: visual property rename-batch JSON should reject same-name renames");
    expect_contains(same_name_process.stdout_text, "The source property cannot be renamed to itself.",
        "#1442: same-name visual property rename-batch JSON should report editor errors");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--property-name", "ControlSource",
            "--new-property-name", "BoundControlSource",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1442: visual property rename-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyRenameBatch\": null",
        "#1442: missing-path visual property rename-batch JSON should not expose a batch rename object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1442: missing-path visual property rename-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", form_path.string(),
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1442: visual property rename-batch JSON should reject empty batches");
    expect_contains(no_items_process.stdout_text, "No property renames were provided.",
        "#1442: empty visual property rename-batch JSON should report parser errors");

    const auto option_before_item_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", form_path.string(),
            "--new-property-name", "BoundControlSource",
            "--property-name", "ControlSource",
            "--json"
        },
        temp_root);
    expect(option_before_item_process.exit_code == 2,
        "#1442: visual property rename-batch JSON should reject item options before property names");
    expect_contains(option_before_item_process.stdout_text,
        "Visual property rename batch item options require a preceding --property-name.",
        "#1442: option-before-item visual property rename-batch JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--new-property-name", "BoundControlSource",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1442: visual property rename-batch JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1442: invalid-record visual property rename-batch JSON should report parser errors");

    const auto missing_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", form_path.string(),
            "--property-name", "",
            "--new-property-name", "BoundControlSource",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(missing_property_name_process.exit_code == 4,
        "#1442: visual property rename-batch JSON should reject missing property names");
    expect_contains(missing_property_name_process.stdout_text, "\"visualPropertyRenameBatch\": null",
        "#1442: missing property-name visual property rename-batch JSON should not expose a batch rename object");
    expect_contains(missing_property_name_process.stdout_text, "No property name was provided.",
        "#1442: missing property-name visual property rename-batch JSON should report editor errors");

    const auto missing_new_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--new-property-name", "",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(missing_new_property_name_process.exit_code == 4,
        "#1442: visual property rename-batch JSON should reject missing target property names");
    expect_contains(missing_new_property_name_process.stdout_text, "\"visualPropertyRenameBatch\": null",
        "#1442: missing target property-name visual property rename-batch JSON should not expose a batch rename object");
    expect_contains(missing_new_property_name_process.stdout_text, "No target property name was provided.",
        "#1442: missing target property-name visual property rename-batch JSON should report editor errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--new-property-name", "BoundControlSource",
            "--object-name", "missingObject",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1442: visual property rename-batch JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualPropertyRenameBatch\": null",
        "#1442: unresolved visual property rename-batch JSON should not expose a batch rename object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1442: unresolved visual property rename-batch JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-rename-batch --path <asset>",
        "#1442: usage text should expose visual property rename-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
