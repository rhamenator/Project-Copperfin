void test_studio_host_json_assigns_dynamic_font_bold_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_dynamic_font_bold_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const std::string expression = "IIF(.T., .T., .F.)";
    const fs::path dynamic_font_bold_path = temp_root / "dynamic_font_bold.scx";
    write_synthetic_form_table_for_object_dynamic_font_bold(dynamic_font_bold_path);
    const auto dynamic_font_bold_process = run_process_capture(
        studio_host_path,
        {
            "--path", dynamic_font_bold_path.string(),
            "--dynamic-font-bold-object",
            "--dynamic-font-bold", expression,
            "--dynamic-font-bold-target-object-name", "txtName",
            "--dynamic-font-bold-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(dynamic_font_bold_process.exit_code == 0,
        "#1190: host object dynamic-font-bold assignment should exit successfully");
    expect(visual_object_property(dynamic_font_bold_path, "one-guid", "DYNAMICFONTBOLD") == expression &&
            visual_object_property(dynamic_font_bold_path, "two-guid", "DYNAMICFONTBOLD") == expression &&
            visual_object_property(dynamic_font_bold_path, "three-guid", "DYNAMICFONTBOLD") == ".F." &&
            visual_object_property(dynamic_font_bold_path, "other-guid", "DYNAMICFONTBOLD") == ".T.",
        "#1190: host object dynamic-font-bold assignment should assign raw expression text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_dynamic_font_bold(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--dynamic-font-bold-object",
            "--dynamic-font-bold", expression,
            "--dynamic-font-bold-target-unique-id", "one-guid",
            "--dynamic-font-bold-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1190: missing-target host object dynamic-font-bold assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DYNAMICFONTBOLD") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "DYNAMICFONTBOLD") == ".T.",
        "#1190: missing-target host object dynamic-font-bold assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_dynamic_font_bold(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--dynamic-font-bold-object",
            "--dynamic-font-bold", expression,
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1190: dynamic-font-bold-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DYNAMICFONTBOLD") == ".F.",
        "#1190: dynamic-font-bold-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_dynamic_font_bold(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--dynamic-font-bold-object",
            "--dynamic-font-bold-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1190: dynamic-font-bold-object without dynamic-font-bold value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DYNAMICFONTBOLD") == ".F.",
        "#1190: dynamic-font-bold-object without dynamic-font-bold value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_dynamic_font_bold(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--dynamic-font-bold-object",
            "--dynamic-font-bold", expression,
            "--dynamic-font-bold-target-unique-id", "one-guid",
            "--dynamic-font-bold-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1190: duplicate-target host object dynamic-font-bold assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DYNAMICFONTBOLD") == ".F.",
        "#1190: duplicate-target host object dynamic-font-bold assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_dynamic_font_bold(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--dynamic-font-bold-object",
            "--allow-output-object",
            "--dynamic-font-bold", expression,
            "--dynamic-font-bold-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1190: dynamic-font-bold-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DYNAMICFONTBOLD") == ".F.",
        "#1190: dynamic-font-bold-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
