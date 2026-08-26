void test_studio_host_json_assigns_button_count_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_button_count_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path button_count_path = temp_root / "button_count.scx";
    write_synthetic_form_table_for_object_button_count(button_count_path);
    const auto button_count_process = run_process_capture(
        studio_host_path,
        {
            "--path", button_count_path.string(),
            "--button-count-object",
            "--button-count", "3",
            "--button-count-target-object-name", "cmdSave",
            "--button-count-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(button_count_process.exit_code == 0,
        "#1110: host object button-count assignment should exit successfully");
    expect(visual_object_property(button_count_path, "one-guid", "BUTTONCOUNT") == "3" &&
            visual_object_property(button_count_path, "two-guid", "BUTTONCOUNT") == "3" &&
            visual_object_property(button_count_path, "three-guid", "BUTTONCOUNT") == "2" &&
            visual_object_property(button_count_path, "other-guid", "BUTTONCOUNT") == "0",
        "#1110: host object button-count assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_button_count(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--button-count-object",
            "--button-count", "2",
            "--button-count-target-unique-id", "one-guid",
            "--button-count-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1110: missing-target host object button-count assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "BUTTONCOUNT") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "BUTTONCOUNT") == "1",
        "#1110: missing-target host object button-count assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_button_count(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--button-count-object",
            "--button-count", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1110: button-count-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "BUTTONCOUNT") == "0",
        "#1110: button-count-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_button_count(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--button-count-object",
            "--button-count-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1110: button-count-object without button-count value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "BUTTONCOUNT") == "0",
        "#1110: button-count-object without button-count value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_button_count(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--button-count-object",
            "--button-count", "-1",
            "--button-count-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1110: negative button-count values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "BUTTONCOUNT") == "0",
        "#1110: negative button-count values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_button_count(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--button-count-object",
            "--button-count", "2",
            "--button-count-target-unique-id", "one-guid",
            "--button-count-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1110: duplicate-target host object button-count assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "BUTTONCOUNT") == "0",
        "#1110: duplicate-target host object button-count assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_button_count(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--button-count-object",
            "--locked-object",
            "--button-count", "2",
            "--button-count-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1110: button-count-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "BUTTONCOUNT") == "0",
        "#1110: button-count-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_data_session_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_data_session_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path data_session_path = temp_root / "data_session.scx";
    write_synthetic_form_table_for_object_data_session(data_session_path);
    const auto data_session_process = run_process_capture(
        studio_host_path,
        {
            "--path", data_session_path.string(),
            "--data-session-object",
            "--data-session", "9",
            "--data-session-target-object-name", "cmdSave",
            "--data-session-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(data_session_process.exit_code == 0,
        "#1119: host object data-session assignment should exit successfully");
    expect(visual_object_property(data_session_path, "one-guid", "DATASESSION") == "9" &&
            visual_object_property(data_session_path, "two-guid", "DATASESSION") == "9" &&
            visual_object_property(data_session_path, "three-guid", "DATASESSION") == "2" &&
            visual_object_property(data_session_path, "other-guid", "DATASESSION") == "0",
        "#1119: host object data-session assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_data_session(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--data-session-object",
            "--data-session", "2",
            "--data-session-target-unique-id", "one-guid",
            "--data-session-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1119: missing-target host object data-session assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DATASESSION") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "DATASESSION") == "1",
        "#1119: missing-target host object data-session assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_data_session(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--data-session-object",
            "--data-session", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1119: data-session-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DATASESSION") == "0",
        "#1119: data-session-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_data_session(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--data-session-object",
            "--data-session-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1119: data-session-object without data-session value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DATASESSION") == "0",
        "#1119: data-session-object without data-session value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_data_session(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--data-session-object",
            "--data-session", "-1",
            "--data-session-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1119: negative data-session values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "DATASESSION") == "0",
        "#1119: negative data-session values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_data_session(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--data-session-object",
            "--data-session", "2",
            "--data-session-target-unique-id", "one-guid",
            "--data-session-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1119: duplicate-target host object data-session assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DATASESSION") == "0",
        "#1119: duplicate-target host object data-session assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_data_session(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--data-session-object",
            "--locked-object",
            "--data-session", "2",
            "--data-session-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1119: data-session-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DATASESSION") == "0",
        "#1119: data-session-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_lock_columns_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_lock_columns_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path lock_columns_path = temp_root / "lock_columns.scx";
    write_synthetic_form_table_for_object_lock_columns(lock_columns_path);
    const auto lock_columns_process = run_process_capture(
        studio_host_path,
        {
            "--path", lock_columns_path.string(),
            "--lock-columns-object",
            "--lock-columns", "9",
            "--lock-columns-target-object-name", "cmdSave",
            "--lock-columns-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(lock_columns_process.exit_code == 0,
        "#1123: host object lock-columns assignment should exit successfully");
    expect(visual_object_property(lock_columns_path, "one-guid", "LOCKCOLUMNS") == "9" &&
            visual_object_property(lock_columns_path, "two-guid", "LOCKCOLUMNS") == "9" &&
            visual_object_property(lock_columns_path, "three-guid", "LOCKCOLUMNS") == "2" &&
            visual_object_property(lock_columns_path, "other-guid", "LOCKCOLUMNS") == "0",
        "#1123: host object lock-columns assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_lock_columns(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--lock-columns-object",
            "--lock-columns", "2",
            "--lock-columns-target-unique-id", "one-guid",
            "--lock-columns-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1123: missing-target host object lock-columns assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "LOCKCOLUMNS") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "LOCKCOLUMNS") == "1",
        "#1123: missing-target host object lock-columns assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_lock_columns(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--lock-columns-object",
            "--lock-columns", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1123: lock-columns-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "LOCKCOLUMNS") == "0",
        "#1123: lock-columns-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_lock_columns(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--lock-columns-object",
            "--lock-columns-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1123: lock-columns-object without lock-columns value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "LOCKCOLUMNS") == "0",
        "#1123: lock-columns-object without lock-columns value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_lock_columns(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--lock-columns-object",
            "--lock-columns", "-1",
            "--lock-columns-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1123: negative lock-columns values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "LOCKCOLUMNS") == "0",
        "#1123: negative lock-columns values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_lock_columns(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--lock-columns-object",
            "--lock-columns", "2",
            "--lock-columns-target-unique-id", "one-guid",
            "--lock-columns-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1123: duplicate-target host object lock-columns assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "LOCKCOLUMNS") == "0",
        "#1123: duplicate-target host object lock-columns assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_lock_columns(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--lock-columns-object",
            "--locked-object",
            "--lock-columns", "2",
            "--lock-columns-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1123: lock-columns-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "LOCKCOLUMNS") == "0",
        "#1123: lock-columns-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_lock_columns_left_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_lock_columns_left_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path lock_columns_left_path = temp_root / "lock_columns_left.scx";
    write_synthetic_form_table_for_object_lock_columns_left(lock_columns_left_path);
    const auto lock_columns_left_process = run_process_capture(
        studio_host_path,
        {
            "--path", lock_columns_left_path.string(),
            "--lock-columns-left-object",
            "--lock-columns-left", "9",
            "--lock-columns-left-target-object-name", "cmdSave",
            "--lock-columns-left-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(lock_columns_left_process.exit_code == 0,
        "#1124: host object lock-columns-left assignment should exit successfully");
    expect(visual_object_property(lock_columns_left_path, "one-guid", "LOCKCOLUMNSLEFT") == "9" &&
            visual_object_property(lock_columns_left_path, "two-guid", "LOCKCOLUMNSLEFT") == "9" &&
            visual_object_property(lock_columns_left_path, "three-guid", "LOCKCOLUMNSLEFT") == "2" &&
            visual_object_property(lock_columns_left_path, "other-guid", "LOCKCOLUMNSLEFT") == "0",
        "#1124: host object lock-columns-left assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_lock_columns_left(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--lock-columns-left-object",
            "--lock-columns-left", "2",
            "--lock-columns-left-target-unique-id", "one-guid",
            "--lock-columns-left-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1124: missing-target host object lock-columns-left assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "LOCKCOLUMNSLEFT") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "LOCKCOLUMNSLEFT") == "1",
        "#1124: missing-target host object lock-columns-left assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_lock_columns_left(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--lock-columns-left-object",
            "--lock-columns-left", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1124: lock-columns-left-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "LOCKCOLUMNSLEFT") == "0",
        "#1124: lock-columns-left-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_lock_columns_left(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--lock-columns-left-object",
            "--lock-columns-left-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1124: lock-columns-left-object without lock-columns-left value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "LOCKCOLUMNSLEFT") == "0",
        "#1124: lock-columns-left-object without lock-columns-left value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_lock_columns_left(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--lock-columns-left-object",
            "--lock-columns-left", "-1",
            "--lock-columns-left-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1124: negative lock-columns-left values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "LOCKCOLUMNSLEFT") == "0",
        "#1124: negative lock-columns-left values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_lock_columns_left(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--lock-columns-left-object",
            "--lock-columns-left", "2",
            "--lock-columns-left-target-unique-id", "one-guid",
            "--lock-columns-left-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1124: duplicate-target host object lock-columns-left assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "LOCKCOLUMNSLEFT") == "0",
        "#1124: duplicate-target host object lock-columns-left assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_lock_columns_left(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--lock-columns-left-object",
            "--locked-object",
            "--lock-columns-left", "2",
            "--lock-columns-left-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1124: lock-columns-left-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "LOCKCOLUMNSLEFT") == "0",
        "#1124: lock-columns-left-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
