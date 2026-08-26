void test_studio_host_json_assigns_partition_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_partition_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path partition_path = temp_root / "partition.scx";
    write_synthetic_form_table_for_object_partition(partition_path);
    const auto partition_process = run_process_capture(
        studio_host_path,
        {
            "--path", partition_path.string(),
            "--partition-object",
            "--partition", "9",
            "--partition-target-object-name", "cmdSave",
            "--partition-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(partition_process.exit_code == 0,
        "#1128: host object partition assignment should exit successfully");
    expect(visual_object_property(partition_path, "one-guid", "PARTITION") == "9" &&
            visual_object_property(partition_path, "two-guid", "PARTITION") == "9" &&
            visual_object_property(partition_path, "three-guid", "PARTITION") == "2" &&
            visual_object_property(partition_path, "other-guid", "PARTITION") == "0",
        "#1128: host object partition assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_partition(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--partition-object",
            "--partition", "2",
            "--partition-target-unique-id", "one-guid",
            "--partition-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1128: missing-target host object partition assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "PARTITION") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "PARTITION") == "1",
        "#1128: missing-target host object partition assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_partition(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--partition-object",
            "--partition", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1128: partition-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "PARTITION") == "0",
        "#1128: partition-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_partition(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--partition-object",
            "--partition-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1128: partition-object without partition value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "PARTITION") == "0",
        "#1128: partition-object without partition value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_partition(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--partition-object",
            "--partition", "-1",
            "--partition-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1128: negative partition values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "PARTITION") == "0",
        "#1128: negative partition values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_partition(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--partition-object",
            "--partition", "2",
            "--partition-target-unique-id", "one-guid",
            "--partition-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1128: duplicate-target host object partition assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "PARTITION") == "0",
        "#1128: duplicate-target host object partition assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_partition(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--partition-object",
            "--locked-object",
            "--partition", "2",
            "--partition-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1128: partition-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "PARTITION") == "0",
        "#1128: partition-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
void test_studio_host_json_assigns_record_source_type_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_record_source_type_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path record_source_type_path = temp_root / "record_source_type.scx";
    write_synthetic_form_table_for_object_record_source_type(record_source_type_path);
    const auto record_source_type_process = run_process_capture(
        studio_host_path,
        {
            "--path", record_source_type_path.string(),
            "--record-source-type-object",
            "--record-source-type", "9",
            "--record-source-type-target-object-name", "cmdSave",
            "--record-source-type-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(record_source_type_process.exit_code == 0,
        "#1129: host object record-source-type assignment should exit successfully");
    expect(visual_object_property(record_source_type_path, "one-guid", "RECORDSOURCETYPE") == "9" &&
            visual_object_property(record_source_type_path, "two-guid", "RECORDSOURCETYPE") == "9" &&
            visual_object_property(record_source_type_path, "three-guid", "RECORDSOURCETYPE") == "2" &&
            visual_object_property(record_source_type_path, "other-guid", "RECORDSOURCETYPE") == "0",
        "#1129: host object record-source-type assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_record_source_type(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--record-source-type-object",
            "--record-source-type", "2",
            "--record-source-type-target-unique-id", "one-guid",
            "--record-source-type-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1129: missing-target host object record-source-type assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "RECORDSOURCETYPE") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "RECORDSOURCETYPE") == "1",
        "#1129: missing-target host object record-source-type assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_record_source_type(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--record-source-type-object",
            "--record-source-type", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1129: record-source-type-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "RECORDSOURCETYPE") == "0",
        "#1129: record-source-type-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_record_source_type(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--record-source-type-object",
            "--record-source-type-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1129: record-source-type-object without record-source-type value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "RECORDSOURCETYPE") == "0",
        "#1129: record-source-type-object without record-source-type value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_record_source_type(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--record-source-type-object",
            "--record-source-type", "-1",
            "--record-source-type-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1129: negative record-source-type values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "RECORDSOURCETYPE") == "0",
        "#1129: negative record-source-type values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_record_source_type(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--record-source-type-object",
            "--record-source-type", "2",
            "--record-source-type-target-unique-id", "one-guid",
            "--record-source-type-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1129: duplicate-target host object record-source-type assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "RECORDSOURCETYPE") == "0",
        "#1129: duplicate-target host object record-source-type assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_record_source_type(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--record-source-type-object",
            "--locked-object",
            "--record-source-type", "2",
            "--record-source-type-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1129: record-source-type-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "RECORDSOURCETYPE") == "0",
        "#1129: record-source-type-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_column_order_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_column_order_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path column_order_path = temp_root / "column_order.scx";
    write_synthetic_form_table_for_object_column_order(column_order_path);
    const auto column_order_process = run_process_capture(
        studio_host_path,
        {
            "--path", column_order_path.string(),
            "--column-order-object",
            "--column-order", "9",
            "--column-order-target-object-name", "cmdSave",
            "--column-order-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(column_order_process.exit_code == 0,
        "#1131: host object column-order assignment should exit successfully");
    expect(visual_object_property(column_order_path, "one-guid", "COLUMNORDER") == "9" &&
            visual_object_property(column_order_path, "two-guid", "COLUMNORDER") == "9" &&
            visual_object_property(column_order_path, "three-guid", "COLUMNORDER") == "2" &&
            visual_object_property(column_order_path, "other-guid", "COLUMNORDER") == "0",
        "#1131: host object column-order assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_column_order(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--column-order-object",
            "--column-order", "2",
            "--column-order-target-unique-id", "one-guid",
            "--column-order-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1131: missing-target host object column-order assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "COLUMNORDER") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "COLUMNORDER") == "1",
        "#1131: missing-target host object column-order assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_column_order(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--column-order-object",
            "--column-order", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1131: column-order-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "COLUMNORDER") == "0",
        "#1131: column-order-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_column_order(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--column-order-object",
            "--column-order-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1131: column-order-object without column-order value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "COLUMNORDER") == "0",
        "#1131: column-order-object without column-order value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_column_order(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--column-order-object",
            "--column-order", "-1",
            "--column-order-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1131: negative column-order values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "COLUMNORDER") == "0",
        "#1131: negative column-order values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_column_order(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--column-order-object",
            "--column-order", "2",
            "--column-order-target-unique-id", "one-guid",
            "--column-order-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1131: duplicate-target host object column-order assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "COLUMNORDER") == "0",
        "#1131: duplicate-target host object column-order assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_column_order(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--column-order-object",
            "--locked-object",
            "--column-order", "2",
            "--column-order-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1131: column-order-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "COLUMNORDER") == "0",
        "#1131: column-order-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_child_order_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_child_order_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path child_order_path = temp_root / "child_order.scx";
    write_synthetic_form_table_for_object_child_order(child_order_path);
    const auto child_order_process = run_process_capture(
        studio_host_path,
        {
            "--path", child_order_path.string(),
            "--child-order-object",
            "--child-order", "9",
            "--child-order-target-object-name", "cmdSave",
            "--child-order-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(child_order_process.exit_code == 0,
        "#1133: host object child-order assignment should exit successfully");
    expect(visual_object_property(child_order_path, "one-guid", "CHILDORDER") == "9" &&
            visual_object_property(child_order_path, "two-guid", "CHILDORDER") == "9" &&
            visual_object_property(child_order_path, "three-guid", "CHILDORDER") == "2" &&
            visual_object_property(child_order_path, "other-guid", "CHILDORDER") == "0",
        "#1133: host object child-order assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_child_order(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--child-order-object",
            "--child-order", "2",
            "--child-order-target-unique-id", "one-guid",
            "--child-order-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1133: missing-target host object child-order assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "CHILDORDER") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "CHILDORDER") == "1",
        "#1133: missing-target host object child-order assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_child_order(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--child-order-object",
            "--child-order", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1133: child-order-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "CHILDORDER") == "0",
        "#1133: child-order-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_child_order(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--child-order-object",
            "--child-order-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1133: child-order-object without child-order value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "CHILDORDER") == "0",
        "#1133: child-order-object without child-order value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_child_order(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--child-order-object",
            "--child-order", "-1",
            "--child-order-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1133: negative child-order values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "CHILDORDER") == "0",
        "#1133: negative child-order values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_child_order(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--child-order-object",
            "--child-order", "2",
            "--child-order-target-unique-id", "one-guid",
            "--child-order-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1133: duplicate-target host object child-order assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "CHILDORDER") == "0",
        "#1133: duplicate-target host object child-order assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_child_order(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--child-order-object",
            "--locked-object",
            "--child-order", "2",
            "--child-order-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1133: child-order-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "CHILDORDER") == "0",
        "#1133: child-order-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_record_source_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_record_source_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path record_source_path = temp_root / "record_source.scx";
    write_synthetic_form_table_for_object_record_source(record_source_path);
    const auto record_source_process = run_process_capture(
        studio_host_path,
        {
            "--path", record_source_path.string(),
            "--record-source-object",
            "--record-source", "customers",
            "--record-source-target-object-name", "cmdSave",
            "--record-source-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(record_source_process.exit_code == 0,
        "#1130: host object record source assignment should exit successfully");
    expect(visual_object_property(record_source_path, "one-guid", "RECORDSOURCE") == "customers" &&
            visual_object_property(record_source_path, "two-guid", "RECORDSOURCE") == "customers" &&
            visual_object_property(record_source_path, "three-guid", "RECORDSOURCE") == "Ready" &&
            visual_object_property(record_source_path, "other-guid", "RECORDSOURCE") == "Other",
        "#1130: host object record source assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_record_source(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--record-source-object",
            "--record-source", "customers",
            "--record-source-target-unique-id", "one-guid",
            "--record-source-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1130: missing-target host object record source assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "RECORDSOURCE") == "Save" &&
            visual_object_property(missing_target_path, "two-guid", "RECORDSOURCE") == "Cancel",
        "#1130: missing-target host object record source assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_record_source(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--record-source-object",
            "--record-source", "customers",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1130: record-source-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "RECORDSOURCE") == "Save",
        "#1130: record-source-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_record_source(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--record-source-object",
            "--record-source-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1130: record-source-object without record source value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "RECORDSOURCE") == "Save",
        "#1130: record-source-object without record source value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_record_source(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--record-source-object",
            "--record-source", "customers",
            "--record-source-target-unique-id", "one-guid",
            "--record-source-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1130: duplicate-target host object record source assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "RECORDSOURCE") == "Save",
        "#1130: duplicate-target host object record source assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_record_source(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--record-source-object",
            "--caption-object",
            "--record-source", "customers",
            "--record-source-target-unique-id", "one-guid",
            "--caption", "Save Customer",
            "--caption-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1130: record-source-object plus caption-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "RECORDSOURCE") == "Save",
        "#1130: record-source-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
