void test_studio_host_json_assigns_highlight_back_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_highlight_back_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path highlight_back_color_path = temp_root / "highlight_back_color.scx";
    write_synthetic_form_table_for_object_highlight_back_color(highlight_back_color_path);
    const auto highlight_back_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", highlight_back_color_path.string(),
            "--highlight-back-color-object",
            "--highlight-back-color", "8421504",
            "--highlight-back-color-target-object-name", "lstCustomers",
            "--highlight-back-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(highlight_back_color_process.exit_code == 0,
        "#1064: host object highlight-back-color assignment should exit successfully");
    expect(visual_object_property(highlight_back_color_path, "one-guid", "HIGHLIGHTBACKCOLOR") == "8421504" &&
            visual_object_property(highlight_back_color_path, "two-guid", "HIGHLIGHTBACKCOLOR") == "8421504" &&
            visual_object_property(highlight_back_color_path, "three-guid", "HIGHLIGHTBACKCOLOR") == "255" &&
            visual_object_property(highlight_back_color_path, "other-guid", "HIGHLIGHTBACKCOLOR") == "65280",
        "#1064: host object highlight-back-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_highlight_back_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--highlight-back-color-object",
            "--highlight-back-color", "8421504",
            "--highlight-back-color-target-unique-id", "one-guid",
            "--highlight-back-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1064: missing-target host object highlight-back-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "HIGHLIGHTBACKCOLOR") == "16777215" &&
            visual_object_property(missing_target_path, "two-guid", "HIGHLIGHTBACKCOLOR") == "12632256",
        "#1064: missing-target host object highlight-back-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_highlight_back_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--highlight-back-color-object",
            "--highlight-back-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1064: highlight-back-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "HIGHLIGHTBACKCOLOR") == "16777215",
        "#1064: highlight-back-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_highlight_back_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--highlight-back-color-object",
            "--highlight-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1064: highlight-back-color-object without highlight-back-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "HIGHLIGHTBACKCOLOR") == "16777215",
        "#1064: highlight-back-color-object without highlight-back-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_highlight_back_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--highlight-back-color-object",
            "--highlight-back-color", "-1",
            "--highlight-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1064: negative highlight-back-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "HIGHLIGHTBACKCOLOR") == "16777215",
        "#1064: negative highlight-back-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_highlight_back_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--highlight-back-color-object",
            "--highlight-back-color", "8421504",
            "--highlight-back-color-target-unique-id", "one-guid",
            "--highlight-back-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1064: duplicate-target host object highlight-back-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "HIGHLIGHTBACKCOLOR") == "16777215",
        "#1064: duplicate-target host object highlight-back-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_highlight_back_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--highlight-back-color-object",
            "--item-fore-color-object",
            "--highlight-back-color", "8421504",
            "--highlight-back-color-target-unique-id", "one-guid",
            "--item-fore-color", "65280",
            "--item-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1064: highlight-back-color-object plus item-fore-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "HIGHLIGHTBACKCOLOR") == "16777215",
        "#1064: highlight-back-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_highlight_fore_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_highlight_fore_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path highlight_fore_color_path = temp_root / "highlight_fore_color.scx";
    write_synthetic_form_table_for_object_highlight_fore_color(highlight_fore_color_path);
    const auto highlight_fore_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", highlight_fore_color_path.string(),
            "--highlight-fore-color-object",
            "--highlight-fore-color", "8421504",
            "--highlight-fore-color-target-object-name", "lstCustomers",
            "--highlight-fore-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(highlight_fore_color_process.exit_code == 0,
        "#1065: host object highlight-fore-color assignment should exit successfully");
    expect(visual_object_property(highlight_fore_color_path, "one-guid", "HIGHLIGHTFORECOLOR") == "8421504" &&
            visual_object_property(highlight_fore_color_path, "two-guid", "HIGHLIGHTFORECOLOR") == "8421504" &&
            visual_object_property(highlight_fore_color_path, "three-guid", "HIGHLIGHTFORECOLOR") == "255" &&
            visual_object_property(highlight_fore_color_path, "other-guid", "HIGHLIGHTFORECOLOR") == "65280",
        "#1065: host object highlight-fore-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_highlight_fore_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--highlight-fore-color-object",
            "--highlight-fore-color", "8421504",
            "--highlight-fore-color-target-unique-id", "one-guid",
            "--highlight-fore-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1065: missing-target host object highlight-fore-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "HIGHLIGHTFORECOLOR") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "HIGHLIGHTFORECOLOR") == "16777215",
        "#1065: missing-target host object highlight-fore-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_highlight_fore_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--highlight-fore-color-object",
            "--highlight-fore-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1065: highlight-fore-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "HIGHLIGHTFORECOLOR") == "0",
        "#1065: highlight-fore-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_highlight_fore_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--highlight-fore-color-object",
            "--highlight-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1065: highlight-fore-color-object without highlight-fore-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "HIGHLIGHTFORECOLOR") == "0",
        "#1065: highlight-fore-color-object without highlight-fore-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_highlight_fore_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--highlight-fore-color-object",
            "--highlight-fore-color", "-1",
            "--highlight-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1065: negative highlight-fore-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "HIGHLIGHTFORECOLOR") == "0",
        "#1065: negative highlight-fore-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_highlight_fore_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--highlight-fore-color-object",
            "--highlight-fore-color", "8421504",
            "--highlight-fore-color-target-unique-id", "one-guid",
            "--highlight-fore-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1065: duplicate-target host object highlight-fore-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "HIGHLIGHTFORECOLOR") == "0",
        "#1065: duplicate-target host object highlight-fore-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_highlight_fore_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--highlight-fore-color-object",
            "--highlight-back-color-object",
            "--highlight-fore-color", "8421504",
            "--highlight-fore-color-target-unique-id", "one-guid",
            "--highlight-back-color", "65280",
            "--highlight-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1065: highlight-fore-color-object plus highlight-back-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "HIGHLIGHTFORECOLOR") == "0",
        "#1065: highlight-fore-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_back_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_back_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path back_color_path = temp_root / "back_color.scx";
    write_synthetic_form_table_for_object_back_color(back_color_path);
    const auto back_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", back_color_path.string(),
            "--back-color-object",
            "--back-color", "8421504",
            "--back-color-target-object-name", "lstCustomers",
            "--back-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(back_color_process.exit_code == 0,
        "#1066: host object back-color assignment should exit successfully");
    expect(visual_object_property(back_color_path, "one-guid", "BACKCOLOR") == "8421504" &&
            visual_object_property(back_color_path, "two-guid", "BACKCOLOR") == "8421504" &&
            visual_object_property(back_color_path, "three-guid", "BACKCOLOR") == "255" &&
            visual_object_property(back_color_path, "other-guid", "BACKCOLOR") == "65280",
        "#1066: host object back-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_back_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--back-color-object",
            "--back-color", "8421504",
            "--back-color-target-unique-id", "one-guid",
            "--back-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1066: missing-target host object back-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "BACKCOLOR") == "16777215" &&
            visual_object_property(missing_target_path, "two-guid", "BACKCOLOR") == "12632256",
        "#1066: missing-target host object back-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_back_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--back-color-object",
            "--back-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1066: back-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "BACKCOLOR") == "16777215",
        "#1066: back-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_back_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--back-color-object",
            "--back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1066: back-color-object without back-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "BACKCOLOR") == "16777215",
        "#1066: back-color-object without back-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_back_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--back-color-object",
            "--back-color", "-1",
            "--back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1066: negative back-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "BACKCOLOR") == "16777215",
        "#1066: negative back-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_back_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--back-color-object",
            "--back-color", "8421504",
            "--back-color-target-unique-id", "one-guid",
            "--back-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1066: duplicate-target host object back-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "BACKCOLOR") == "16777215",
        "#1066: duplicate-target host object back-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_back_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--back-color-object",
            "--highlight-fore-color-object",
            "--back-color", "8421504",
            "--back-color-target-unique-id", "one-guid",
            "--highlight-fore-color", "65280",
            "--highlight-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1066: back-color-object plus highlight-fore-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "BACKCOLOR") == "16777215",
        "#1066: back-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_fore_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_fore_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path fore_color_path = temp_root / "fore_color.scx";
    write_synthetic_form_table_for_object_fore_color(fore_color_path);
    const auto fore_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", fore_color_path.string(),
            "--fore-color-object",
            "--fore-color", "8421504",
            "--fore-color-target-object-name", "lstCustomers",
            "--fore-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(fore_color_process.exit_code == 0,
        "#1067: host object fore-color assignment should exit successfully");
    expect(visual_object_property(fore_color_path, "one-guid", "FORECOLOR") == "8421504" &&
            visual_object_property(fore_color_path, "two-guid", "FORECOLOR") == "8421504" &&
            visual_object_property(fore_color_path, "three-guid", "FORECOLOR") == "16777215" &&
            visual_object_property(fore_color_path, "other-guid", "FORECOLOR") == "65280",
        "#1067: host object fore-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_fore_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--fore-color-object",
            "--fore-color", "8421504",
            "--fore-color-target-unique-id", "one-guid",
            "--fore-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1067: missing-target host object fore-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "FORECOLOR") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "FORECOLOR") == "255",
        "#1067: missing-target host object fore-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_fore_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--fore-color-object",
            "--fore-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1067: fore-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "FORECOLOR") == "0",
        "#1067: fore-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_fore_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--fore-color-object",
            "--fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1067: fore-color-object without fore-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "FORECOLOR") == "0",
        "#1067: fore-color-object without fore-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_fore_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--fore-color-object",
            "--fore-color", "-1",
            "--fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1067: negative fore-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "FORECOLOR") == "0",
        "#1067: negative fore-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_fore_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--fore-color-object",
            "--fore-color", "8421504",
            "--fore-color-target-unique-id", "one-guid",
            "--fore-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1067: duplicate-target host object fore-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "FORECOLOR") == "0",
        "#1067: duplicate-target host object fore-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_fore_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--fore-color-object",
            "--back-color-object",
            "--fore-color", "8421504",
            "--fore-color-target-unique-id", "one-guid",
            "--back-color", "65280",
            "--back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1067: fore-color-object plus back-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "FORECOLOR") == "0",
        "#1067: fore-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
