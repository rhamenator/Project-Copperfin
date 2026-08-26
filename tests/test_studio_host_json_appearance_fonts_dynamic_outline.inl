void test_studio_host_json_assigns_dynamic_font_outline_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_dynamic_font_outline_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const std::string expression = "IIF(.T., .T., .F.)";
    const fs::path dynamic_font_outline_path = temp_root / "dynamic_font_outline.scx";
    write_synthetic_form_table_for_object_dynamic_font_outline(dynamic_font_outline_path);
    const auto dynamic_font_outline_process = run_process_capture(
        studio_host_path,
        {
            "--path", dynamic_font_outline_path.string(),
            "--dynamic-font-outline-object",
            "--dynamic-font-outline", expression,
            "--dynamic-font-outline-target-object-name", "txtName",
            "--dynamic-font-outline-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(dynamic_font_outline_process.exit_code == 0,
        "#1194: host object dynamic-font-outline assignment should exit successfully");
    expect(visual_object_property(dynamic_font_outline_path, "one-guid", "DYNAMICFONTOUTLINE") == expression &&
            visual_object_property(dynamic_font_outline_path, "two-guid", "DYNAMICFONTOUTLINE") == expression &&
            visual_object_property(dynamic_font_outline_path, "three-guid", "DYNAMICFONTOUTLINE") == ".F." &&
            visual_object_property(dynamic_font_outline_path, "other-guid", "DYNAMICFONTOUTLINE") == ".T.",
        "#1194: host object dynamic-font-outline assignment should assign raw expression text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_dynamic_font_outline(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--dynamic-font-outline-object",
            "--dynamic-font-outline", expression,
            "--dynamic-font-outline-target-unique-id", "one-guid",
            "--dynamic-font-outline-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1194: missing-target host object dynamic-font-outline assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DYNAMICFONTOUTLINE") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "DYNAMICFONTOUTLINE") == ".T.",
        "#1194: missing-target host object dynamic-font-outline assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_dynamic_font_outline(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--dynamic-font-outline-object",
            "--dynamic-font-outline", expression,
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1194: dynamic-font-outline-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DYNAMICFONTOUTLINE") == ".F.",
        "#1194: dynamic-font-outline-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_dynamic_font_outline(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--dynamic-font-outline-object",
            "--dynamic-font-outline-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1194: dynamic-font-outline-object without dynamic-font-outline value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DYNAMICFONTOUTLINE") == ".F.",
        "#1194: dynamic-font-outline-object without dynamic-font-outline value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_dynamic_font_outline(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--dynamic-font-outline-object",
            "--dynamic-font-outline", expression,
            "--dynamic-font-outline-target-unique-id", "one-guid",
            "--dynamic-font-outline-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1194: duplicate-target host object dynamic-font-outline assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DYNAMICFONTOUTLINE") == ".F.",
        "#1194: duplicate-target host object dynamic-font-outline assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_dynamic_font_outline(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--dynamic-font-outline-object",
            "--allow-output-object",
            "--dynamic-font-outline", expression,
            "--dynamic-font-outline-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1194: dynamic-font-outline-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DYNAMICFONTOUTLINE") == ".F.",
        "#1194: dynamic-font-outline-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
