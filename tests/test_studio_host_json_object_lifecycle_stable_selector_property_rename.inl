void test_studio_host_json_renames_properties_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_property_rename_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path object_name_path = write_synthetic_form_table_for_property_rename(temp_root, "object_name.scx");
    const auto object_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", object_name_path.string(),
            "--rename-property",
            "--object-name", "txt1",
            "--property-name", "ControlSource",
            "--new-property-name", "InputSource",
            "--json"
        },
        temp_root);
    expect(object_name_process.exit_code == 0,
        "#1022: object-name host property renames should exit successfully");
    auto renamed_property = copperfin::vfp::query_visual_object_property({
        .path = object_name_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "existing-textbox-guid",
        .property_name = "InputSource"
    });
    expect(renamed_property.ok && renamed_property.exists && renamed_property.value == "\"customer.name\"",
        "#1022: object-name host property renames should create the target memo-backed property");
    auto source_property = copperfin::vfp::query_visual_object_property({
        .path = object_name_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "existing-textbox-guid",
        .property_name = "ControlSource"
    });
    expect(source_property.ok && !source_property.exists,
        "#1022: object-name host property renames should remove the source memo-backed property");

    const fs::path unique_id_path = write_synthetic_form_table_for_property_rename(temp_root, "unique_id.scx");
    const auto unique_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", unique_id_path.string(),
            "--rename-property",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--new-property-name", "InputSource",
            "--json"
        },
        temp_root);
    expect(unique_id_process.exit_code == 0,
        "#1022: unique-id host property renames should exit successfully");
    renamed_property = copperfin::vfp::query_visual_object_property({
        .path = unique_id_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "existing-textbox-guid",
        .property_name = "InputSource"
    });
    expect(renamed_property.ok && renamed_property.exists && renamed_property.value == "\"customer.name\"",
        "#1022: unique-id host property renames should create the target memo-backed property");

    const fs::path missing_path = write_synthetic_form_table_for_property_rename(temp_root, "missing.scx");
    const auto missing_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_path.string(),
            "--rename-property",
            "--object-name", "missingObject",
            "--property-name", "ControlSource",
            "--new-property-name", "InputSource",
            "--json"
        },
        temp_root);
    expect(missing_process.exit_code == 4,
        "#1022: missing object-name host property renames should return command failure");
    source_property = copperfin::vfp::query_visual_object_property({
        .path = missing_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "existing-textbox-guid",
        .property_name = "ControlSource"
    });
    expect(source_property.ok && source_property.exists && source_property.value == "\"customer.name\"",
        "#1022: missing object-name host property renames should not mutate the asset");

    const fs::path ambiguous_path = write_synthetic_form_table_for_property_rename(temp_root, "ambiguous.scx");
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--set-property",
            "--rename-property",
            "--property-name", "ControlSource",
            "--property-value", "Ambiguous",
            "--new-property-name", "InputSource",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1022: ambiguous set/rename property requests should fail during launch parsing");
    source_property = copperfin::vfp::query_visual_object_property({
        .path = ambiguous_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "existing-textbox-guid",
        .property_name = "ControlSource"
    });
    expect(source_property.ok && source_property.exists && source_property.value == "\"customer.name\"",
        "#1022: ambiguous set/rename property requests should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
