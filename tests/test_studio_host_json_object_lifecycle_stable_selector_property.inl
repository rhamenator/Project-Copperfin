void test_studio_host_json_sets_properties_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_property_selector_json_tests";
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

    const auto record_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--record", "1",
            "--property-name", "CAPTION",
            "--property-value", "RecordTarget",
            "--json"
        },
        temp_root);
    expect(record_process.exit_code == 0,
        "#1020: record-index host property edits should remain compatible");
    auto caption = caption_value();
    expect(caption.ok && caption.exists && caption.value == "RecordTarget",
        "#1020: record-index host property edits should update the selected record");

    const auto object_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--object-name", "txt1",
            "--property-name", "CAPTION",
            "--property-value", "NameTarget",
            "--json"
        },
        temp_root);
    expect(object_name_process.exit_code == 0,
        "#1020: object-name host property edits should exit successfully");
    caption = caption_value();
    expect(caption.ok && caption.exists && caption.value == "NameTarget",
        "#1020: object-name host property edits should update the named object");

    const auto unique_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "UniqueTarget",
            "--json"
        },
        temp_root);
    expect(unique_id_process.exit_code == 0,
        "#1020: unique-id host property edits should exit successfully");
    caption = caption_value();
    expect(caption.ok && caption.exists && caption.value == "UniqueTarget",
        "#1020: unique-id host property edits should update the stable selected object");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--object-name", "missingObject",
            "--property-name", "CAPTION",
            "--property-value", "ShouldNotWrite",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1020: missing object-name host property edits should return command failure");
    caption = caption_value();
    expect(caption.ok && caption.exists && caption.value == "UniqueTarget",
        "#1020: missing object-name host property edits should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
