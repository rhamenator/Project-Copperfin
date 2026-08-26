void test_studio_host_json_assigns_form_set_class_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_form_set_class_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path form_set_class_path = temp_root / "form_set_class.scx";
    write_synthetic_form_table_for_object_form_set_class(form_set_class_path);
    const auto form_set_class_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_set_class_path.string(),
            "--form-set-class-object",
            "--form-set-class", "BaseFormSet",
            "--form-set-class-target-object-name", "cmdSaveFormSet",
            "--form-set-class-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(form_set_class_process.exit_code == 0,
        "#1136: host object form set class assignment should exit successfully");
    expect(visual_object_property(form_set_class_path, "one-guid", "FORMSETCLASS") == "BaseFormSet" &&
            visual_object_property(form_set_class_path, "two-guid", "FORMSETCLASS") == "BaseFormSet" &&
            visual_object_property(form_set_class_path, "three-guid", "FORMSETCLASS") == "StatusFormSet" &&
            visual_object_property(form_set_class_path, "other-guid", "FORMSETCLASS") == "OtherFormSet",
        "#1136: host object form set class assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_form_set_class(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--form-set-class-object",
            "--form-set-class", "BaseFormSet",
            "--form-set-class-target-unique-id", "one-guid",
            "--form-set-class-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1136: missing-target host object form set class assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "FORMSETCLASS") == "SaveFormSet" &&
            visual_object_property(missing_target_path, "two-guid", "FORMSETCLASS") == "CancelFormSet",
        "#1136: missing-target host object form set class assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_form_set_class(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--form-set-class-object",
            "--form-set-class", "BaseFormSet",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1136: form-set-class-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "FORMSETCLASS") == "SaveFormSet",
        "#1136: form-set-class-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_form_set_class(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--form-set-class-object",
            "--form-set-class-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1136: form-set-class-object without form set class value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "FORMSETCLASS") == "SaveFormSet",
        "#1136: form-set-class-object without form set class value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_form_set_class(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--form-set-class-object",
            "--form-set-class", "BaseFormSet",
            "--form-set-class-target-unique-id", "one-guid",
            "--form-set-class-target-object-name", "cmdSaveFormSet",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1136: duplicate-target host object form set class assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "FORMSETCLASS") == "SaveFormSet",
        "#1136: duplicate-target host object form set class assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_form_set_class(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--form-set-class-object",
            "--caption-object",
            "--form-set-class", "BaseFormSet",
            "--form-set-class-target-unique-id", "one-guid",
            "--caption", "SaveFormSet Customer",
            "--caption-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1136: form-set-class-object plus caption-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "FORMSETCLASS") == "SaveFormSet",
        "#1136: form-set-class-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
void test_studio_host_json_assigns_default_file_path_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_default_file_path_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path default_file_path_path = temp_root / "default_file_path.scx";
    write_synthetic_form_table_for_object_default_file_path(default_file_path_path);
    const auto default_file_path_process = run_process_capture(
        studio_host_path,
        {
            "--path", default_file_path_path.string(),
            "--default-file-path-object",
            "--default-file-path", "Data\\Customers",
            "--default-file-path-target-object-name", "cmdDefaultPath",
            "--default-file-path-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(default_file_path_process.exit_code == 0,
        "#1137: host object default file path assignment should exit successfully");
    expect(visual_object_property(default_file_path_path, "one-guid", "DEFAULTFILEPATH") == "Data\\Customers" &&
            visual_object_property(default_file_path_path, "two-guid", "DEFAULTFILEPATH") == "Data\\Customers" &&
            visual_object_property(default_file_path_path, "three-guid", "DEFAULTFILEPATH") == "Data\\Status" &&
            visual_object_property(default_file_path_path, "other-guid", "DEFAULTFILEPATH") == "Data\\Other",
        "#1137: host object default file path assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_default_file_path(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--default-file-path-object",
            "--default-file-path", "Data\\Customers",
            "--default-file-path-target-unique-id", "one-guid",
            "--default-file-path-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1137: missing-target host object default file path assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DEFAULTFILEPATH") == "Data\\Save" &&
            visual_object_property(missing_target_path, "two-guid", "DEFAULTFILEPATH") == "Data\\Cancel",
        "#1137: missing-target host object default file path assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_default_file_path(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--default-file-path-object",
            "--default-file-path", "Data\\Customers",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1137: default-file-path-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DEFAULTFILEPATH") == "Data\\Save",
        "#1137: default-file-path-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_default_file_path(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--default-file-path-object",
            "--default-file-path-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1137: default-file-path-object without default file path value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DEFAULTFILEPATH") == "Data\\Save",
        "#1137: default-file-path-object without default file path value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_default_file_path(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--default-file-path-object",
            "--default-file-path", "Data\\Customers",
            "--default-file-path-target-unique-id", "one-guid",
            "--default-file-path-target-object-name", "cmdDefaultPath",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1137: duplicate-target host object default file path assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DEFAULTFILEPATH") == "Data\\Save",
        "#1137: duplicate-target host object default file path assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_default_file_path(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--default-file-path-object",
            "--caption-object",
            "--default-file-path", "Data\\Customers",
            "--default-file-path-target-unique-id", "one-guid",
            "--caption", "Default path",
            "--caption-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1137: default-file-path-object plus caption-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DEFAULTFILEPATH") == "Data\\Save",
        "#1137: default-file-path-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_initial_selected_alias_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_initial_selected_alias_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path initial_selected_alias_path = temp_root / "initial_selected_alias.scx";
    write_synthetic_form_table_for_object_initial_selected_alias(initial_selected_alias_path);
    const auto initial_selected_alias_process = run_process_capture(
        studio_host_path,
        {
            "--path", initial_selected_alias_path.string(),
            "--initial-selected-alias-object",
            "--initial-selected-alias", "customers",
            "--initial-selected-alias-target-object-name", "cmdInitialAlias",
            "--initial-selected-alias-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(initial_selected_alias_process.exit_code == 0,
        "#1138: host object initial selected alias assignment should exit successfully");
    expect(visual_object_property(initial_selected_alias_path, "one-guid", "INITIALSELECTEDALIAS") == "customers" &&
            visual_object_property(initial_selected_alias_path, "two-guid", "INITIALSELECTEDALIAS") == "customers" &&
            visual_object_property(initial_selected_alias_path, "three-guid", "INITIALSELECTEDALIAS") == "status_alias" &&
            visual_object_property(initial_selected_alias_path, "other-guid", "INITIALSELECTEDALIAS") == "other_alias",
        "#1138: host object initial selected alias assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_initial_selected_alias(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--initial-selected-alias-object",
            "--initial-selected-alias", "customers",
            "--initial-selected-alias-target-unique-id", "one-guid",
            "--initial-selected-alias-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1138: missing-target host object initial selected alias assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "INITIALSELECTEDALIAS") == "orders" &&
            visual_object_property(missing_target_path, "two-guid", "INITIALSELECTEDALIAS") == "payments",
        "#1138: missing-target host object initial selected alias assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_initial_selected_alias(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--initial-selected-alias-object",
            "--initial-selected-alias", "customers",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1138: initial-selected-alias-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "INITIALSELECTEDALIAS") == "orders",
        "#1138: initial-selected-alias-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_initial_selected_alias(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--initial-selected-alias-object",
            "--initial-selected-alias-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1138: initial-selected-alias-object without initial selected alias value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "INITIALSELECTEDALIAS") == "orders",
        "#1138: initial-selected-alias-object without initial selected alias value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_initial_selected_alias(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--initial-selected-alias-object",
            "--initial-selected-alias", "customers",
            "--initial-selected-alias-target-unique-id", "one-guid",
            "--initial-selected-alias-target-object-name", "cmdInitialAlias",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1138: duplicate-target host object initial selected alias assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "INITIALSELECTEDALIAS") == "orders",
        "#1138: duplicate-target host object initial selected alias assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_initial_selected_alias(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--initial-selected-alias-object",
            "--caption-object",
            "--initial-selected-alias", "customers",
            "--initial-selected-alias-target-unique-id", "one-guid",
            "--caption", "Initial alias",
            "--caption-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1138: initial-selected-alias-object plus caption-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "INITIALSELECTEDALIAS") == "orders",
        "#1138: initial-selected-alias-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_link_master_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_link_master_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path link_master_path = temp_root / "link_master.scx";
    write_synthetic_form_table_for_object_link_master(link_master_path);
    const auto link_master_process = run_process_capture(
        studio_host_path,
        {
            "--path", link_master_path.string(),
            "--link-master-object",
            "--link-master", "customer_id",
            "--link-master-target-object-name", "cmdSave",
            "--link-master-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(link_master_process.exit_code == 0,
        "#1165: host object link-master assignment should exit successfully");
    expect(visual_object_property(link_master_path, "one-guid", "LINKMASTER") == "customer_id" &&
            visual_object_property(link_master_path, "two-guid", "LINKMASTER") == "customer_id" &&
            visual_object_property(link_master_path, "three-guid", "LINKMASTER") == "status_id" &&
            visual_object_property(link_master_path, "other-guid", "LINKMASTER") == "other_id",
        "#1165: host object link-master assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_link_master(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--link-master-object",
            "--link-master", "customer_id",
            "--link-master-target-unique-id", "one-guid",
            "--link-master-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1165: missing-target host object link-master assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "LINKMASTER") == "old_customer_id" &&
            visual_object_property(missing_target_path, "two-guid", "LINKMASTER") == "old_order_id",
        "#1165: missing-target host object link-master assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_link_master(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--link-master-object",
            "--link-master", "customer_id",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1165: link-master-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "LINKMASTER") == "old_customer_id",
        "#1165: link-master-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_link_master(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--link-master-object",
            "--link-master-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1165: link-master-object without link-master value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "LINKMASTER") == "old_customer_id",
        "#1165: link-master-object without link-master value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_link_master(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--link-master-object",
            "--link-master", "customer_id",
            "--link-master-target-unique-id", "one-guid",
            "--link-master-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1165: duplicate-target host object link-master assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "LINKMASTER") == "old_customer_id",
        "#1165: duplicate-target host object link-master assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_link_master(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--link-master-object",
            "--status-bar-text-object",
            "--link-master", "customer_id",
            "--link-master-target-unique-id", "one-guid",
            "--status-bar-text", "Ready to save",
            "--status-bar-text-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1165: link-master-object plus status-bar-text-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "LINKMASTER") == "old_customer_id",
        "#1165: link-master-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_row_source_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_row_source_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path row_source_path = temp_root / "row_source.scx";
    write_synthetic_form_table_for_object_row_source(row_source_path);
    const auto row_source_process = run_process_capture(
        studio_host_path,
        {
            "--path", row_source_path.string(),
            "--row-source-object",
            "--row-source", "products.name,product_id",
            "--row-source-target-object-name", "cboCustomer",
            "--row-source-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(row_source_process.exit_code == 0,
        "#1048: host object row-source assignment should exit successfully");
    expect(visual_object_property(row_source_path, "one-guid", "ROWSOURCE") == "products.name,product_id" &&
            visual_object_property(row_source_path, "two-guid", "ROWSOURCE") == "products.name,product_id" &&
            visual_object_property(row_source_path, "three-guid", "ROWSOURCE") == "Ready" &&
            visual_object_property(row_source_path, "other-guid", "ROWSOURCE") == "states.name",
        "#1048: host object row-source assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_row_source(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--row-source-object",
            "--row-source", "products.name,product_id",
            "--row-source-target-unique-id", "one-guid",
            "--row-source-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1048: missing-target host object row-source assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "ROWSOURCE") == "customers.name,customer_id" &&
            visual_object_property(missing_target_path, "two-guid", "ROWSOURCE") == "orders.order_id,total",
        "#1048: missing-target host object row-source assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_row_source(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--row-source-object",
            "--row-source", "products.name,product_id",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1048: row-source-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "ROWSOURCE") == "customers.name,customer_id",
        "#1048: row-source-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_row_source(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--row-source-object",
            "--row-source-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1048: row-source-object without row-source value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "ROWSOURCE") == "customers.name,customer_id",
        "#1048: row-source-object without row-source value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_row_source(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--row-source-object",
            "--row-source", "products.name,product_id",
            "--row-source-target-unique-id", "one-guid",
            "--row-source-target-object-name", "cboCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1048: duplicate-target host object row-source assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "ROWSOURCE") == "customers.name,customer_id",
        "#1048: duplicate-target host object row-source assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_row_source(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--row-source-object",
            "--format-object",
            "--row-source", "products.name,product_id",
            "--row-source-target-unique-id", "one-guid",
            "--format", "!",
            "--format-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1048: row-source-object plus format-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "ROWSOURCE") == "customers.name,customer_id",
        "#1048: row-source-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
