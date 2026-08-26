void test_studio_host_json_reorders_visual_properties(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_reorder_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = write_synthetic_form_table_for_property_rename(temp_root, "reorder.scx");

    const auto reorder_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--placement", "first",
            "--json"
        },
        temp_root);
    expect(reorder_process.exit_code == 0,
        "#1443: visual property reorder JSON should exit successfully for memo-backed properties");
    expect_contains(reorder_process.stdout_text, "\"visualPropertyReorder\": {",
        "#1443: visual property reorder JSON should expose a reorder object");
    expect_contains(reorder_process.stdout_text, "\"affectedObjectCount\": 1",
        "#1443: visual property reorder JSON should expose affected object counts");
    expect_contains(reorder_process.stdout_text, "\"dryRun\": false",
        "#1443: visual property reorder JSON should expose committed execution state");
    expect_contains(reorder_process.stdout_text, "\"mutatesAsset\": true",
        "#1443: visual property reorder JSON should expose mutation state");
    expect_contains(reorder_process.stdout_text, "\"undoAvailable\": true",
        "#1443: visual property reorder JSON should expose undo availability");
    expect(visual_object_property_order(form_path, "existing-textbox-guid") == "Left,ControlSource" &&
            visual_object_property(form_path, "existing-textbox-guid", "Left") == "12" &&
            visual_object_property(form_path, "existing-textbox-guid", "ControlSource") == "\"customer.name\"",
        "#1443: visual property reorder host command should reorder properties and preserve values");

    const auto relative_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--placement", "before",
            "--relative-property-name", "Left",
            "--json"
        },
        temp_root);
    expect(relative_process.exit_code == 0,
        "#1443: visual property reorder JSON should support relative placements");
    expect(visual_object_property_order(form_path, "existing-textbox-guid") == "ControlSource,Left",
        "#1443: visual property reorder host command should apply relative placements");

    const auto missing_relative_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--placement", "after",
            "--relative-property-name", "MissingProperty",
            "--json"
        },
        temp_root);
    expect(missing_relative_process.exit_code == 4,
        "#1443: visual property reorder JSON should reject missing relative properties");
    expect_contains(missing_relative_process.stdout_text, "\"visualPropertyReorder\": null",
        "#1443: missing-relative visual property reorder JSON should not expose a reorder object");
    expect_contains(missing_relative_process.stdout_text, "The relative property was not found.",
        "#1443: missing-relative visual property reorder JSON should report editor errors");
    expect(visual_object_property_order(form_path, "existing-textbox-guid") == "ControlSource,Left",
        "#1443: failed visual property reorder commands should not mutate property order");

    const auto self_relative_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--placement", "before",
            "--relative-property-name", "Left",
            "--json"
        },
        temp_root);
    expect(self_relative_process.exit_code == 4,
        "#1443: visual property reorder JSON should reject self-relative placement");
    expect_contains(self_relative_process.stdout_text, "The source property cannot be positioned relative to itself.",
        "#1443: self-relative visual property reorder JSON should report editor errors");

    const fs::path direct_path = temp_root / "reorder_direct.scx";
    write_synthetic_form_table_for_toolbox_creation(direct_path);
    const auto direct_field_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", direct_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--placement", "first",
            "--json"
        },
        temp_root);
    expect(direct_field_process.exit_code == 4,
        "#1443: visual property reorder JSON should reject direct DBF-backed fields");
    expect_contains(direct_field_process.stdout_text, "\"visualPropertyReorder\": null",
        "#1443: direct-field visual property reorder JSON should not expose a reorder object");
    expect_contains(direct_field_process.stdout_text, "Direct DBF-backed fields cannot be reordered per object.",
        "#1443: direct-field visual property reorder JSON should report editor errors");
    expect(visual_object_property(direct_path, "existing-textbox-guid", "CAPTION") == "Existing",
        "#1443: direct-field visual property reorder commands should not mutate direct fields");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--placement", "first",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1443: visual property reorder JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyReorder\": null",
        "#1443: missing-path visual property reorder JSON should not expose a reorder object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1443: missing-path visual property reorder JSON should report parser errors");

    const auto missing_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--placement", "first",
            "--json"
        },
        temp_root);
    expect(missing_property_name_process.exit_code == 2,
        "#1443: visual property reorder JSON should reject missing property names");
    expect_contains(missing_property_name_process.stdout_text, "No property name was provided.",
        "#1443: missing property-name visual property reorder JSON should report parser errors");

    const auto missing_placement_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--json"
        },
        temp_root);
    expect(missing_placement_process.exit_code == 2,
        "#1443: visual property reorder JSON should reject missing placements");
    expect_contains(missing_placement_process.stdout_text, "No property placement was provided.",
        "#1443: missing-placement visual property reorder JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", form_path.string(),
            "--record", "-1",
            "--property-name", "Left",
            "--placement", "first",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1443: visual property reorder JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1443: invalid-record visual property reorder JSON should report parser errors");

    const auto unknown_placement_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--placement", "middle",
            "--json"
        },
        temp_root);
    expect(unknown_placement_process.exit_code == 4,
        "#1443: visual property reorder JSON should reject unknown placements");
    expect_contains(unknown_placement_process.stdout_text, "Unknown property placement was requested.",
        "#1443: unknown-placement visual property reorder JSON should report editor errors");

    const auto missing_relative_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--placement", "before",
            "--json"
        },
        temp_root);
    expect(missing_relative_name_process.exit_code == 4,
        "#1443: visual property reorder JSON should reject missing relative names");
    expect_contains(missing_relative_name_process.stdout_text, "No relative property name was provided.",
        "#1443: missing-relative-name visual property reorder JSON should report editor errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", form_path.string(),
            "--object-name", "missingObject",
            "--property-name", "Left",
            "--placement", "first",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1443: visual property reorder JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualPropertyReorder\": null",
        "#1443: unresolved visual property reorder JSON should not expose a reorder object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1443: unresolved visual property reorder JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-reorder --path <asset>",
        "#1443: usage text should expose visual property reorder commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
void test_studio_host_json_reorders_visual_property_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_reorder_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = write_synthetic_form_table_for_property_rename(temp_root, "reorder_batch.scx");

    const auto reorder_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", form_path.string(),
            "--property-name", "Left",
            "--placement", "first",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--placement", "after",
            "--relative-property-name", "Left",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(reorder_batch_process.exit_code == 0,
        "#1444: visual property reorder-batch JSON should exit successfully for valid batches");
    expect_contains(reorder_batch_process.stdout_text, "\"visualPropertyReorderBatch\": {",
        "#1444: visual property reorder-batch JSON should expose a batch reorder object");
    expect_contains(reorder_batch_process.stdout_text, "\"affectedObjectCount\": 2",
        "#1444: visual property reorder-batch JSON should expose affected item counts");
    expect_contains(reorder_batch_process.stdout_text, "\"dryRun\": false",
        "#1444: visual property reorder-batch JSON should expose committed execution state");
    expect_contains(reorder_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1444: visual property reorder-batch JSON should expose mutation state");
    expect_contains(reorder_batch_process.stdout_text, "\"undoAvailable\": true",
        "#1444: visual property reorder-batch JSON should expose undo availability");
    expect(visual_object_property_order(form_path, "existing-textbox-guid") == "Left,ControlSource" &&
            visual_object_property(form_path, "existing-textbox-guid", "Left") == "12" &&
            visual_object_property(form_path, "existing-textbox-guid", "ControlSource") == "\"customer.name\"",
        "#1444: visual property reorder-batch host command should reorder requested properties and preserve values");

    const fs::path rollback_path = write_synthetic_form_table_for_property_rename(temp_root, "reorder_batch_rollback.scx");
    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", rollback_path.string(),
            "--property-name", "Left",
            "--placement", "first",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--placement", "after",
            "--relative-property-name", "MissingProperty",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(rollback_process.exit_code == 4,
        "#1444: visual property reorder-batch JSON should reject missing relative properties");
    expect_contains(rollback_process.stdout_text, "\"visualPropertyReorderBatch\": null",
        "#1444: failed visual property reorder-batch JSON should not expose a batch reorder object");
    expect_contains(rollback_process.stdout_text, "The relative property was not found.",
        "#1444: missing-relative visual property reorder-batch JSON should report editor errors");
    expect(visual_object_property_order(rollback_path, "existing-textbox-guid") == "ControlSource,Left",
        "#1444: failed visual property reorder-batch commands should roll back earlier reorders");

    const fs::path direct_path = temp_root / "reorder_batch_direct.scx";
    write_synthetic_form_table_for_toolbox_creation(direct_path);
    const auto direct_field_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", direct_path.string(),
            "--property-name", "CAPTION",
            "--placement", "first",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(direct_field_process.exit_code == 4,
        "#1444: visual property reorder-batch JSON should reject direct DBF-backed fields");
    expect_contains(direct_field_process.stdout_text, "\"visualPropertyReorderBatch\": null",
        "#1444: direct-field visual property reorder-batch JSON should not expose a batch reorder object");
    expect_contains(direct_field_process.stdout_text, "Direct DBF-backed fields cannot be reordered per object.",
        "#1444: direct-field visual property reorder-batch JSON should report editor errors");

    const fs::path self_path = write_synthetic_form_table_for_property_rename(temp_root, "reorder_batch_self.scx");
    const auto self_relative_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", self_path.string(),
            "--property-name", "Left",
            "--placement", "before",
            "--relative-property-name", "Left",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(self_relative_process.exit_code == 4,
        "#1444: visual property reorder-batch JSON should reject self-relative placement");
    expect_contains(self_relative_process.stdout_text, "The source property cannot be positioned relative to itself.",
        "#1444: self-relative visual property reorder-batch JSON should report editor errors");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--property-name", "Left",
            "--placement", "first",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1444: visual property reorder-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyReorderBatch\": null",
        "#1444: missing-path visual property reorder-batch JSON should not expose a batch reorder object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1444: missing-path visual property reorder-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", form_path.string(),
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1444: visual property reorder-batch JSON should reject empty batches");
    expect_contains(no_items_process.stdout_text, "No property reorders were provided.",
        "#1444: empty visual property reorder-batch JSON should report parser errors");

    const auto option_before_item_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", form_path.string(),
            "--placement", "first",
            "--property-name", "Left",
            "--json"
        },
        temp_root);
    expect(option_before_item_process.exit_code == 2,
        "#1444: visual property reorder-batch JSON should reject item options before property names");
    expect_contains(option_before_item_process.stdout_text,
        "Visual property reorder batch item options require a preceding --property-name.",
        "#1444: option-before-item visual property reorder-batch JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", form_path.string(),
            "--property-name", "Left",
            "--placement", "first",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1444: visual property reorder-batch JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1444: invalid-record visual property reorder-batch JSON should report parser errors");

    const auto missing_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", form_path.string(),
            "--property-name", "",
            "--placement", "first",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(missing_property_name_process.exit_code == 4,
        "#1444: visual property reorder-batch JSON should reject missing property names");
    expect_contains(missing_property_name_process.stdout_text, "\"visualPropertyReorderBatch\": null",
        "#1444: missing property-name visual property reorder-batch JSON should not expose a batch reorder object");
    expect_contains(missing_property_name_process.stdout_text, "No property name was provided.",
        "#1444: missing property-name visual property reorder-batch JSON should report editor errors");

    const auto missing_placement_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", form_path.string(),
            "--property-name", "Left",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(missing_placement_process.exit_code == 4,
        "#1444: visual property reorder-batch JSON should reject missing placements");
    expect_contains(missing_placement_process.stdout_text, "\"visualPropertyReorderBatch\": null",
        "#1444: missing-placement visual property reorder-batch JSON should not expose a batch reorder object");
    expect_contains(missing_placement_process.stdout_text, "Unknown property placement was requested.",
        "#1444: missing-placement visual property reorder-batch JSON should report editor errors");

    const auto missing_relative_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", form_path.string(),
            "--property-name", "Left",
            "--placement", "before",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(missing_relative_name_process.exit_code == 4,
        "#1444: visual property reorder-batch JSON should reject missing relative names");
    expect_contains(missing_relative_name_process.stdout_text, "No relative property name was provided.",
        "#1444: missing-relative-name visual property reorder-batch JSON should report editor errors");

    const auto unknown_placement_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", form_path.string(),
            "--property-name", "Left",
            "--placement", "middle",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(unknown_placement_process.exit_code == 4,
        "#1444: visual property reorder-batch JSON should reject unknown placements");
    expect_contains(unknown_placement_process.stdout_text, "Unknown property placement was requested.",
        "#1444: unknown-placement visual property reorder-batch JSON should report editor errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", form_path.string(),
            "--property-name", "Left",
            "--placement", "first",
            "--object-name", "missingObject",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1444: visual property reorder-batch JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualPropertyReorderBatch\": null",
        "#1444: unresolved visual property reorder-batch JSON should not expose a batch reorder object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1444: unresolved visual property reorder-batch JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-reorder-batch --path <asset>",
        "#1444: usage text should expose visual property reorder-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
