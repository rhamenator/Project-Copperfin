void test_studio_host_json_assigns_dynamic_font_strikethru_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_dynamic_font_strikethru_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const std::string expression = "IIF(.T., .T., .F.)";
    const fs::path dynamic_font_strikethru_path = temp_root / "dynamic_font_strikethru.scx";
    write_synthetic_form_table_for_object_dynamic_font_strikethru(dynamic_font_strikethru_path);
    const auto dynamic_font_strikethru_process = run_process_capture(
        studio_host_path,
        {
            "--path", dynamic_font_strikethru_path.string(),
            "--dynamic-font-strikethru-object",
            "--dynamic-font-strikethru", expression,
            "--dynamic-font-strikethru-target-object-name", "txtName",
            "--dynamic-font-strikethru-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(dynamic_font_strikethru_process.exit_code == 0,
        "#1193: host object dynamic-font-strikethru assignment should exit successfully");
    expect(visual_object_property(dynamic_font_strikethru_path, "one-guid", "DYNAMICFONTSTRIKETHRU") == expression &&
            visual_object_property(dynamic_font_strikethru_path, "two-guid", "DYNAMICFONTSTRIKETHRU") == expression &&
            visual_object_property(dynamic_font_strikethru_path, "three-guid", "DYNAMICFONTSTRIKETHRU") == ".F." &&
            visual_object_property(dynamic_font_strikethru_path, "other-guid", "DYNAMICFONTSTRIKETHRU") == ".T.",
        "#1193: host object dynamic-font-strikethru assignment should assign raw expression text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_dynamic_font_strikethru(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--dynamic-font-strikethru-object",
            "--dynamic-font-strikethru", expression,
            "--dynamic-font-strikethru-target-unique-id", "one-guid",
            "--dynamic-font-strikethru-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1193: missing-target host object dynamic-font-strikethru assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DYNAMICFONTSTRIKETHRU") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "DYNAMICFONTSTRIKETHRU") == ".T.",
        "#1193: missing-target host object dynamic-font-strikethru assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_dynamic_font_strikethru(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--dynamic-font-strikethru-object",
            "--dynamic-font-strikethru", expression,
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1193: dynamic-font-strikethru-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DYNAMICFONTSTRIKETHRU") == ".F.",
        "#1193: dynamic-font-strikethru-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_dynamic_font_strikethru(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--dynamic-font-strikethru-object",
            "--dynamic-font-strikethru-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1193: dynamic-font-strikethru-object without dynamic-font-strikethru value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DYNAMICFONTSTRIKETHRU") == ".F.",
        "#1193: dynamic-font-strikethru-object without dynamic-font-strikethru value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_dynamic_font_strikethru(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--dynamic-font-strikethru-object",
            "--dynamic-font-strikethru", expression,
            "--dynamic-font-strikethru-target-unique-id", "one-guid",
            "--dynamic-font-strikethru-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1193: duplicate-target host object dynamic-font-strikethru assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DYNAMICFONTSTRIKETHRU") == ".F.",
        "#1193: duplicate-target host object dynamic-font-strikethru assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_dynamic_font_strikethru(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--dynamic-font-strikethru-object",
            "--allow-output-object",
            "--dynamic-font-strikethru", expression,
            "--dynamic-font-strikethru-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1193: dynamic-font-strikethru-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DYNAMICFONTSTRIKETHRU") == ".F.",
        "#1193: dynamic-font-strikethru-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
