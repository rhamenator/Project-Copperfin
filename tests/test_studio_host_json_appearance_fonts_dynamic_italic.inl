void test_studio_host_json_assigns_dynamic_font_italic_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_dynamic_font_italic_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const std::string expression = "IIF(.T., .T., .F.)";
    const fs::path dynamic_font_italic_path = temp_root / "dynamic_font_italic.scx";
    write_synthetic_form_table_for_object_dynamic_font_italic(dynamic_font_italic_path);
    const auto dynamic_font_italic_process = run_process_capture(
        studio_host_path,
        {
            "--path", dynamic_font_italic_path.string(),
            "--dynamic-font-italic-object",
            "--dynamic-font-italic", expression,
            "--dynamic-font-italic-target-object-name", "txtName",
            "--dynamic-font-italic-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(dynamic_font_italic_process.exit_code == 0,
        "#1191: host object dynamic-font-italic assignment should exit successfully");
    expect(visual_object_property(dynamic_font_italic_path, "one-guid", "DYNAMICFONTITALIC") == expression &&
            visual_object_property(dynamic_font_italic_path, "two-guid", "DYNAMICFONTITALIC") == expression &&
            visual_object_property(dynamic_font_italic_path, "three-guid", "DYNAMICFONTITALIC") == ".F." &&
            visual_object_property(dynamic_font_italic_path, "other-guid", "DYNAMICFONTITALIC") == ".T.",
        "#1191: host object dynamic-font-italic assignment should assign raw expression text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_dynamic_font_italic(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--dynamic-font-italic-object",
            "--dynamic-font-italic", expression,
            "--dynamic-font-italic-target-unique-id", "one-guid",
            "--dynamic-font-italic-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1191: missing-target host object dynamic-font-italic assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DYNAMICFONTITALIC") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "DYNAMICFONTITALIC") == ".T.",
        "#1191: missing-target host object dynamic-font-italic assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_dynamic_font_italic(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--dynamic-font-italic-object",
            "--dynamic-font-italic", expression,
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1191: dynamic-font-italic-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DYNAMICFONTITALIC") == ".F.",
        "#1191: dynamic-font-italic-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_dynamic_font_italic(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--dynamic-font-italic-object",
            "--dynamic-font-italic-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1191: dynamic-font-italic-object without dynamic-font-italic value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DYNAMICFONTITALIC") == ".F.",
        "#1191: dynamic-font-italic-object without dynamic-font-italic value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_dynamic_font_italic(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--dynamic-font-italic-object",
            "--dynamic-font-italic", expression,
            "--dynamic-font-italic-target-unique-id", "one-guid",
            "--dynamic-font-italic-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1191: duplicate-target host object dynamic-font-italic assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DYNAMICFONTITALIC") == ".F.",
        "#1191: duplicate-target host object dynamic-font-italic assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_dynamic_font_italic(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--dynamic-font-italic-object",
            "--allow-output-object",
            "--dynamic-font-italic", expression,
            "--dynamic-font-italic-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1191: dynamic-font-italic-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DYNAMICFONTITALIC") == ".F.",
        "#1191: dynamic-font-italic-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
