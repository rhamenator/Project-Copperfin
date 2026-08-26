void test_studio_host_json_assigns_selected_back_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_selected_back_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path selected_back_color_path = temp_root / "selected_back_color.scx";
    write_synthetic_form_table_for_object_selected_back_color(selected_back_color_path);
    const auto selected_back_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", selected_back_color_path.string(),
            "--selected-back-color-object",
            "--selected-back-color", "8421504",
            "--selected-back-color-target-object-name", "lstCustomers",
            "--selected-back-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(selected_back_color_process.exit_code == 0,
        "#1056: host object selected-back-color assignment should exit successfully");
    expect(visual_object_property(selected_back_color_path, "one-guid", "SELECTEDBACKCOLOR") == "8421504" &&
            visual_object_property(selected_back_color_path, "two-guid", "SELECTEDBACKCOLOR") == "8421504" &&
            visual_object_property(selected_back_color_path, "three-guid", "SELECTEDBACKCOLOR") == "255" &&
            visual_object_property(selected_back_color_path, "other-guid", "SELECTEDBACKCOLOR") == "65280",
        "#1056: host object selected-back-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_selected_back_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--selected-back-color-object",
            "--selected-back-color", "8421504",
            "--selected-back-color-target-unique-id", "one-guid",
            "--selected-back-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1056: missing-target host object selected-back-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "SELECTEDBACKCOLOR") == "16777215" &&
            visual_object_property(missing_target_path, "two-guid", "SELECTEDBACKCOLOR") == "12632256",
        "#1056: missing-target host object selected-back-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_selected_back_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--selected-back-color-object",
            "--selected-back-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1056: selected-back-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "SELECTEDBACKCOLOR") == "16777215",
        "#1056: selected-back-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_selected_back_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--selected-back-color-object",
            "--selected-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1056: selected-back-color-object without selected-back-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "SELECTEDBACKCOLOR") == "16777215",
        "#1056: selected-back-color-object without selected-back-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_selected_back_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--selected-back-color-object",
            "--selected-back-color", "-1",
            "--selected-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1056: negative selected-back-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "SELECTEDBACKCOLOR") == "16777215",
        "#1056: negative selected-back-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_selected_back_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--selected-back-color-object",
            "--selected-back-color", "8421504",
            "--selected-back-color-target-unique-id", "one-guid",
            "--selected-back-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1056: duplicate-target host object selected-back-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "SELECTEDBACKCOLOR") == "16777215",
        "#1056: duplicate-target host object selected-back-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_selected_back_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--selected-back-color-object",
            "--display-value-object",
            "--selected-back-color", "8421504",
            "--selected-back-color-target-unique-id", "one-guid",
            "--display-value", "Bob",
            "--display-value-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1056: selected-back-color-object plus display-value-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "SELECTEDBACKCOLOR") == "16777215",
        "#1056: selected-back-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_selected_fore_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_selected_fore_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path selected_fore_color_path = temp_root / "selected_fore_color.scx";
    write_synthetic_form_table_for_object_selected_fore_color(selected_fore_color_path);
    const auto selected_fore_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", selected_fore_color_path.string(),
            "--selected-fore-color-object",
            "--selected-fore-color", "8421504",
            "--selected-fore-color-target-object-name", "lstCustomers",
            "--selected-fore-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(selected_fore_color_process.exit_code == 0,
        "#1057: host object selected-fore-color assignment should exit successfully");
    expect(visual_object_property(selected_fore_color_path, "one-guid", "SELECTEDFORECOLOR") == "8421504" &&
            visual_object_property(selected_fore_color_path, "two-guid", "SELECTEDFORECOLOR") == "8421504" &&
            visual_object_property(selected_fore_color_path, "three-guid", "SELECTEDFORECOLOR") == "16777215" &&
            visual_object_property(selected_fore_color_path, "other-guid", "SELECTEDFORECOLOR") == "65280",
        "#1057: host object selected-fore-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_selected_fore_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--selected-fore-color-object",
            "--selected-fore-color", "8421504",
            "--selected-fore-color-target-unique-id", "one-guid",
            "--selected-fore-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1057: missing-target host object selected-fore-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "SELECTEDFORECOLOR") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "SELECTEDFORECOLOR") == "255",
        "#1057: missing-target host object selected-fore-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_selected_fore_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--selected-fore-color-object",
            "--selected-fore-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1057: selected-fore-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "SELECTEDFORECOLOR") == "0",
        "#1057: selected-fore-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_selected_fore_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--selected-fore-color-object",
            "--selected-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1057: selected-fore-color-object without selected-fore-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "SELECTEDFORECOLOR") == "0",
        "#1057: selected-fore-color-object without selected-fore-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_selected_fore_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--selected-fore-color-object",
            "--selected-fore-color", "-1",
            "--selected-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1057: negative selected-fore-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "SELECTEDFORECOLOR") == "0",
        "#1057: negative selected-fore-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_selected_fore_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--selected-fore-color-object",
            "--selected-fore-color", "8421504",
            "--selected-fore-color-target-unique-id", "one-guid",
            "--selected-fore-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1057: duplicate-target host object selected-fore-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "SELECTEDFORECOLOR") == "0",
        "#1057: duplicate-target host object selected-fore-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_selected_fore_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--selected-fore-color-object",
            "--selected-back-color-object",
            "--selected-fore-color", "8421504",
            "--selected-fore-color-target-unique-id", "one-guid",
            "--selected-back-color", "16777215",
            "--selected-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1057: selected-fore-color-object plus selected-back-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "SELECTEDFORECOLOR") == "0",
        "#1057: selected-fore-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_selected_item_back_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_item_back_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path selected_item_back_color_path = temp_root / "selected_item_back_color.scx";
    write_synthetic_form_table_for_object_selected_item_back_color(selected_item_back_color_path);
    const auto selected_item_back_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", selected_item_back_color_path.string(),
            "--selected-item-back-color-object",
            "--selected-item-back-color", "8421504",
            "--selected-item-back-color-target-object-name", "lstCustomers",
            "--selected-item-back-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(selected_item_back_color_process.exit_code == 0,
        "#1058: host object selected-item-back-color assignment should exit successfully");
    expect(visual_object_property(selected_item_back_color_path, "one-guid", "SELECTEDITEMBACKCOLOR") == "8421504" &&
            visual_object_property(selected_item_back_color_path, "two-guid", "SELECTEDITEMBACKCOLOR") == "8421504" &&
            visual_object_property(selected_item_back_color_path, "three-guid", "SELECTEDITEMBACKCOLOR") == "255" &&
            visual_object_property(selected_item_back_color_path, "other-guid", "SELECTEDITEMBACKCOLOR") == "65280",
        "#1058: host object selected-item-back-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_selected_item_back_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--selected-item-back-color-object",
            "--selected-item-back-color", "8421504",
            "--selected-item-back-color-target-unique-id", "one-guid",
            "--selected-item-back-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1058: missing-target host object selected-item-back-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "SELECTEDITEMBACKCOLOR") == "16777215" &&
            visual_object_property(missing_target_path, "two-guid", "SELECTEDITEMBACKCOLOR") == "12632256",
        "#1058: missing-target host object selected-item-back-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_selected_item_back_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--selected-item-back-color-object",
            "--selected-item-back-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1058: selected-item-back-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "SELECTEDITEMBACKCOLOR") == "16777215",
        "#1058: selected-item-back-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_selected_item_back_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--selected-item-back-color-object",
            "--selected-item-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1058: selected-item-back-color-object without selected-item-back-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "SELECTEDITEMBACKCOLOR") == "16777215",
        "#1058: selected-item-back-color-object without selected-item-back-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_selected_item_back_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--selected-item-back-color-object",
            "--selected-item-back-color", "-1",
            "--selected-item-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1058: negative selected-item-back-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "SELECTEDITEMBACKCOLOR") == "16777215",
        "#1058: negative selected-item-back-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_selected_item_back_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--selected-item-back-color-object",
            "--selected-item-back-color", "8421504",
            "--selected-item-back-color-target-unique-id", "one-guid",
            "--selected-item-back-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1058: duplicate-target host object selected-item-back-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "SELECTEDITEMBACKCOLOR") == "16777215",
        "#1058: duplicate-target host object selected-item-back-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_selected_item_back_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--selected-item-back-color-object",
            "--selected-fore-color-object",
            "--selected-item-back-color", "8421504",
            "--selected-item-back-color-target-unique-id", "one-guid",
            "--selected-fore-color", "255",
            "--selected-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1058: selected-item-back-color-object plus selected-fore-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "SELECTEDITEMBACKCOLOR") == "16777215",
        "#1058: selected-item-back-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_selected_item_fore_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_item_fore_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path selected_item_fore_color_path = temp_root / "selected_item_fore_color.scx";
    write_synthetic_form_table_for_object_selected_item_fore_color(selected_item_fore_color_path);
    const auto selected_item_fore_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", selected_item_fore_color_path.string(),
            "--selected-item-fore-color-object",
            "--selected-item-fore-color", "8421504",
            "--selected-item-fore-color-target-object-name", "lstCustomers",
            "--selected-item-fore-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(selected_item_fore_color_process.exit_code == 0,
        "#1059: host object selected-item-fore-color assignment should exit successfully");
    expect(visual_object_property(selected_item_fore_color_path, "one-guid", "SELECTEDITEMFORECOLOR") == "8421504" &&
            visual_object_property(selected_item_fore_color_path, "two-guid", "SELECTEDITEMFORECOLOR") == "8421504" &&
            visual_object_property(selected_item_fore_color_path, "three-guid", "SELECTEDITEMFORECOLOR") == "255" &&
            visual_object_property(selected_item_fore_color_path, "other-guid", "SELECTEDITEMFORECOLOR") == "65280",
        "#1059: host object selected-item-fore-color assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_selected_item_fore_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--selected-item-fore-color-object",
            "--selected-item-fore-color", "8421504",
            "--selected-item-fore-color-target-unique-id", "one-guid",
            "--selected-item-fore-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1059: missing-target host object selected-item-fore-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "SELECTEDITEMFORECOLOR") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "SELECTEDITEMFORECOLOR") == "16777215",
        "#1059: missing-target host object selected-item-fore-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_selected_item_fore_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--selected-item-fore-color-object",
            "--selected-item-fore-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1059: selected-item-fore-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "SELECTEDITEMFORECOLOR") == "0",
        "#1059: selected-item-fore-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_selected_item_fore_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--selected-item-fore-color-object",
            "--selected-item-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1059: selected-item-fore-color-object without selected-item-fore-color should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "SELECTEDITEMFORECOLOR") == "0",
        "#1059: selected-item-fore-color-object without selected-item-fore-color should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_selected_item_fore_color(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--selected-item-fore-color-object",
            "--selected-item-fore-color", "-1",
            "--selected-item-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1059: negative selected-item-fore-color values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "SELECTEDITEMFORECOLOR") == "0",
        "#1059: negative selected-item-fore-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_selected_item_fore_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--selected-item-fore-color-object",
            "--selected-item-fore-color", "8421504",
            "--selected-item-fore-color-target-unique-id", "one-guid",
            "--selected-item-fore-color-target-object-name", "lstCustomers",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1059: duplicate-target host object selected-item-fore-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "SELECTEDITEMFORECOLOR") == "0",
        "#1059: duplicate-target host object selected-item-fore-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_selected_item_fore_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--selected-item-fore-color-object",
            "--selected-item-back-color-object",
            "--selected-item-fore-color", "8421504",
            "--selected-item-fore-color-target-unique-id", "one-guid",
            "--selected-item-back-color", "65280",
            "--selected-item-back-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1059: selected-item-fore-color-object plus selected-item-back-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "SELECTEDITEMFORECOLOR") == "0",
        "#1059: selected-item-fore-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
