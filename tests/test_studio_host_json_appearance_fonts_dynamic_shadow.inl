void test_studio_host_json_assigns_dynamic_font_shadow_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_dynamic_font_shadow_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const std::string expression = "IIF(.T., .T., .F.)";
    const fs::path dynamic_font_shadow_path = temp_root / "dynamic_font_shadow.scx";
    write_synthetic_form_table_for_object_dynamic_font_shadow(dynamic_font_shadow_path);
    const auto dynamic_font_shadow_process = run_process_capture(
        studio_host_path,
        {
            "--path", dynamic_font_shadow_path.string(),
            "--dynamic-font-shadow-object",
            "--dynamic-font-shadow", expression,
            "--dynamic-font-shadow-target-object-name", "txtName",
            "--dynamic-font-shadow-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(dynamic_font_shadow_process.exit_code == 0,
        "#1195: host object dynamic-font-shadow assignment should exit successfully");
    expect(visual_object_property(dynamic_font_shadow_path, "one-guid", "DYNAMICFONTSHADOW") == expression &&
            visual_object_property(dynamic_font_shadow_path, "two-guid", "DYNAMICFONTSHADOW") == expression &&
            visual_object_property(dynamic_font_shadow_path, "three-guid", "DYNAMICFONTSHADOW") == ".F." &&
            visual_object_property(dynamic_font_shadow_path, "other-guid", "DYNAMICFONTSHADOW") == ".T.",
        "#1195: host object dynamic-font-shadow assignment should assign raw expression text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_dynamic_font_shadow(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--dynamic-font-shadow-object",
            "--dynamic-font-shadow", expression,
            "--dynamic-font-shadow-target-unique-id", "one-guid",
            "--dynamic-font-shadow-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1195: missing-target host object dynamic-font-shadow assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DYNAMICFONTSHADOW") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "DYNAMICFONTSHADOW") == ".T.",
        "#1195: missing-target host object dynamic-font-shadow assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_dynamic_font_shadow(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--dynamic-font-shadow-object",
            "--dynamic-font-shadow", expression,
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1195: dynamic-font-shadow-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DYNAMICFONTSHADOW") == ".F.",
        "#1195: dynamic-font-shadow-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_dynamic_font_shadow(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--dynamic-font-shadow-object",
            "--dynamic-font-shadow-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1195: dynamic-font-shadow-object without dynamic-font-shadow value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DYNAMICFONTSHADOW") == ".F.",
        "#1195: dynamic-font-shadow-object without dynamic-font-shadow value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_dynamic_font_shadow(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--dynamic-font-shadow-object",
            "--dynamic-font-shadow", expression,
            "--dynamic-font-shadow-target-unique-id", "one-guid",
            "--dynamic-font-shadow-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1195: duplicate-target host object dynamic-font-shadow assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DYNAMICFONTSHADOW") == ".F.",
        "#1195: duplicate-target host object dynamic-font-shadow assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_dynamic_font_shadow(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--dynamic-font-shadow-object",
            "--allow-output-object",
            "--dynamic-font-shadow", expression,
            "--dynamic-font-shadow-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1195: dynamic-font-shadow-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DYNAMICFONTSHADOW") == ".F.",
        "#1195: dynamic-font-shadow-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
