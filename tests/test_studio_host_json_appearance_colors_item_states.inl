void test_studio_host_json_assigns_disabled_item_back_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_disabled_item_back_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path disabled_item_back_color_path = temp_root / "disabled_item_back_color.scx";
    write_synthetic_form_table_for_object_disabled_item_back_color(disabled_item_back_color_path);
    const auto disabled_item_back_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", disabled_item_back_color_path.string(),
            "--disabled-item-back-color-object",
            "--disabled-item-back-color", "8421504",
            "--disabled-item-back-color-target-object-name", "lstCustomers",
            "--disabled-item-back-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(disabled_item_back_color_process.exit_code == 0,
        "#1060: host object disabled-item-back-color assignment should exit successfully");
    expect(visual_object_property(disabled_item_back_color_path, "one-guid", "DISABLEDITEMBACKCOLOR") == "8421504" &&
            visual_object_property(disabled_item_back_color_path, "two-guid", "DISABLEDITEMBACKCOLOR") == "8421504" &&
            visual_object_property(disabled_item_back_color_path, "three-guid", "DISABLEDITEMBACKCOLOR") == "255" &&
            visual_object_property(disabled_item_back_color_path, "other-guid", "DISABLEDITEMBACKCOLOR") == "65280",
        "#1060: host object disabled-item-back-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_disabled_item_back_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--disabled-item-back-color-object",
            "--disabled-item-back-color", "8421504",
            "--disabled-item-back-color-target-unique-id", "one-guid",
            "--disabled-item-back-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1060: missing-target host object disabled-item-back-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DISABLEDITEMBACKCOLOR") == "16777215" &&
            visual_object_property(missing_target_path, "two-guid", "DISABLEDITEMBACKCOLOR") == "12632256",
        "#1060: missing-target host object disabled-item-back-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_disabled_item_back_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--disabled-item-back-color-object",
            "--disabled-item-back-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1060: disabled-item-back-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DISABLEDITEMBACKCOLOR") == "16777215",
        "#1060: disabled-item-back-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_disabled_item_back_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--disabled-item-back-color-object",
            "--disabled-item-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1060: disabled-item-back-color-object without disabled-item-back-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DISABLEDITEMBACKCOLOR") == "16777215",
        "#1060: disabled-item-back-color-object without disabled-item-back-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_disabled_item_back_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--disabled-item-back-color-object",
            "--disabled-item-back-color", "-1",
            "--disabled-item-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1060: negative disabled-item-back-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "DISABLEDITEMBACKCOLOR") == "16777215",
        "#1060: negative disabled-item-back-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_disabled_item_back_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--disabled-item-back-color-object",
            "--disabled-item-back-color", "8421504",
            "--disabled-item-back-color-target-unique-id", "one-guid",
            "--disabled-item-back-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1060: duplicate-target host object disabled-item-back-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DISABLEDITEMBACKCOLOR") == "16777215",
        "#1060: duplicate-target host object disabled-item-back-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_disabled_item_back_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--disabled-item-back-color-object",
            "--selected-item-fore-color-object",
            "--disabled-item-back-color", "8421504",
            "--disabled-item-back-color-target-unique-id", "one-guid",
            "--selected-item-fore-color", "65280",
            "--selected-item-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1060: disabled-item-back-color-object plus selected-item-fore-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DISABLEDITEMBACKCOLOR") == "16777215",
        "#1060: disabled-item-back-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_disabled_item_fore_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_disabled_item_fore_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path disabled_item_fore_color_path = temp_root / "disabled_item_fore_color.scx";
    write_synthetic_form_table_for_object_disabled_item_fore_color(disabled_item_fore_color_path);
    const auto disabled_item_fore_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", disabled_item_fore_color_path.string(),
            "--disabled-item-fore-color-object",
            "--disabled-item-fore-color", "8421504",
            "--disabled-item-fore-color-target-object-name", "lstCustomers",
            "--disabled-item-fore-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(disabled_item_fore_color_process.exit_code == 0,
        "#1061: host object disabled-item-fore-color assignment should exit successfully");
    expect(visual_object_property(disabled_item_fore_color_path, "one-guid", "DISABLEDITEMFORECOLOR") == "8421504" &&
            visual_object_property(disabled_item_fore_color_path, "two-guid", "DISABLEDITEMFORECOLOR") == "8421504" &&
            visual_object_property(disabled_item_fore_color_path, "three-guid", "DISABLEDITEMFORECOLOR") == "255" &&
            visual_object_property(disabled_item_fore_color_path, "other-guid", "DISABLEDITEMFORECOLOR") == "65280",
        "#1061: host object disabled-item-fore-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_disabled_item_fore_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--disabled-item-fore-color-object",
            "--disabled-item-fore-color", "8421504",
            "--disabled-item-fore-color-target-unique-id", "one-guid",
            "--disabled-item-fore-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1061: missing-target host object disabled-item-fore-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DISABLEDITEMFORECOLOR") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "DISABLEDITEMFORECOLOR") == "16777215",
        "#1061: missing-target host object disabled-item-fore-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_disabled_item_fore_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--disabled-item-fore-color-object",
            "--disabled-item-fore-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1061: disabled-item-fore-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DISABLEDITEMFORECOLOR") == "0",
        "#1061: disabled-item-fore-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_disabled_item_fore_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--disabled-item-fore-color-object",
            "--disabled-item-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1061: disabled-item-fore-color-object without disabled-item-fore-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DISABLEDITEMFORECOLOR") == "0",
        "#1061: disabled-item-fore-color-object without disabled-item-fore-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_disabled_item_fore_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--disabled-item-fore-color-object",
            "--disabled-item-fore-color", "-1",
            "--disabled-item-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1061: negative disabled-item-fore-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "DISABLEDITEMFORECOLOR") == "0",
        "#1061: negative disabled-item-fore-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_disabled_item_fore_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--disabled-item-fore-color-object",
            "--disabled-item-fore-color", "8421504",
            "--disabled-item-fore-color-target-unique-id", "one-guid",
            "--disabled-item-fore-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1061: duplicate-target host object disabled-item-fore-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DISABLEDITEMFORECOLOR") == "0",
        "#1061: duplicate-target host object disabled-item-fore-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_disabled_item_fore_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--disabled-item-fore-color-object",
            "--disabled-item-back-color-object",
            "--disabled-item-fore-color", "8421504",
            "--disabled-item-fore-color-target-unique-id", "one-guid",
            "--disabled-item-back-color", "65280",
            "--disabled-item-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1061: disabled-item-fore-color-object plus disabled-item-back-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DISABLEDITEMFORECOLOR") == "0",
        "#1061: disabled-item-fore-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_item_back_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_item_back_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path item_back_color_path = temp_root / "item_back_color.scx";
    write_synthetic_form_table_for_object_item_back_color(item_back_color_path);
    const auto item_back_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", item_back_color_path.string(),
            "--item-back-color-object",
            "--item-back-color", "8421504",
            "--item-back-color-target-object-name", "lstCustomers",
            "--item-back-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(item_back_color_process.exit_code == 0,
        "#1062: host object item-back-color assignment should exit successfully");
    expect(visual_object_property(item_back_color_path, "one-guid", "ITEMBACKCOLOR") == "8421504" &&
            visual_object_property(item_back_color_path, "two-guid", "ITEMBACKCOLOR") == "8421504" &&
            visual_object_property(item_back_color_path, "three-guid", "ITEMBACKCOLOR") == "255" &&
            visual_object_property(item_back_color_path, "other-guid", "ITEMBACKCOLOR") == "65280",
        "#1062: host object item-back-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_item_back_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--item-back-color-object",
            "--item-back-color", "8421504",
            "--item-back-color-target-unique-id", "one-guid",
            "--item-back-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1062: missing-target host object item-back-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "ITEMBACKCOLOR") == "16777215" &&
            visual_object_property(missing_target_path, "two-guid", "ITEMBACKCOLOR") == "12632256",
        "#1062: missing-target host object item-back-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_item_back_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--item-back-color-object",
            "--item-back-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1062: item-back-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "ITEMBACKCOLOR") == "16777215",
        "#1062: item-back-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_item_back_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--item-back-color-object",
            "--item-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1062: item-back-color-object without item-back-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "ITEMBACKCOLOR") == "16777215",
        "#1062: item-back-color-object without item-back-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_item_back_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--item-back-color-object",
            "--item-back-color", "-1",
            "--item-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1062: negative item-back-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "ITEMBACKCOLOR") == "16777215",
        "#1062: negative item-back-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_item_back_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--item-back-color-object",
            "--item-back-color", "8421504",
            "--item-back-color-target-unique-id", "one-guid",
            "--item-back-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1062: duplicate-target host object item-back-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "ITEMBACKCOLOR") == "16777215",
        "#1062: duplicate-target host object item-back-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_item_back_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--item-back-color-object",
            "--disabled-item-fore-color-object",
            "--item-back-color", "8421504",
            "--item-back-color-target-unique-id", "one-guid",
            "--disabled-item-fore-color", "65280",
            "--disabled-item-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1062: item-back-color-object plus disabled-item-fore-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "ITEMBACKCOLOR") == "16777215",
        "#1062: item-back-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_item_fore_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_item_fore_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path item_fore_color_path = temp_root / "item_fore_color.scx";
    write_synthetic_form_table_for_object_item_fore_color(item_fore_color_path);
    const auto item_fore_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", item_fore_color_path.string(),
            "--item-fore-color-object",
            "--item-fore-color", "8421504",
            "--item-fore-color-target-object-name", "lstCustomers",
            "--item-fore-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(item_fore_color_process.exit_code == 0,
        "#1063: host object item-fore-color assignment should exit successfully");
    expect(visual_object_property(item_fore_color_path, "one-guid", "ITEMFORECOLOR") == "8421504" &&
            visual_object_property(item_fore_color_path, "two-guid", "ITEMFORECOLOR") == "8421504" &&
            visual_object_property(item_fore_color_path, "three-guid", "ITEMFORECOLOR") == "255" &&
            visual_object_property(item_fore_color_path, "other-guid", "ITEMFORECOLOR") == "65280",
        "#1063: host object item-fore-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_item_fore_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--item-fore-color-object",
            "--item-fore-color", "8421504",
            "--item-fore-color-target-unique-id", "one-guid",
            "--item-fore-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1063: missing-target host object item-fore-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "ITEMFORECOLOR") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "ITEMFORECOLOR") == "16777215",
        "#1063: missing-target host object item-fore-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_item_fore_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--item-fore-color-object",
            "--item-fore-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1063: item-fore-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "ITEMFORECOLOR") == "0",
        "#1063: item-fore-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_item_fore_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--item-fore-color-object",
            "--item-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1063: item-fore-color-object without item-fore-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "ITEMFORECOLOR") == "0",
        "#1063: item-fore-color-object without item-fore-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_item_fore_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--item-fore-color-object",
            "--item-fore-color", "-1",
            "--item-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1063: negative item-fore-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "ITEMFORECOLOR") == "0",
        "#1063: negative item-fore-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_item_fore_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--item-fore-color-object",
            "--item-fore-color", "8421504",
            "--item-fore-color-target-unique-id", "one-guid",
            "--item-fore-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1063: duplicate-target host object item-fore-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "ITEMFORECOLOR") == "0",
        "#1063: duplicate-target host object item-fore-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_item_fore_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--item-fore-color-object",
            "--item-back-color-object",
            "--item-fore-color", "8421504",
            "--item-fore-color-target-unique-id", "one-guid",
            "--item-back-color", "65280",
            "--item-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1063: item-fore-color-object plus item-back-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "ITEMFORECOLOR") == "0",
        "#1063: item-fore-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
