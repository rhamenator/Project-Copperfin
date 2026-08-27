void test_studio_host_json_ungroups_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_ungroup_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path object_name_path = temp_root / "object_name.scx";
    write_synthetic_form_table_for_object_ungroup(object_name_path);
    const auto object_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", object_name_path.string(),
            "--ungroup-object",
            "--object-name", "cntGroup",
            "--json"
        },
        temp_root);
    expect(object_name_process.exit_code == 0,
        "#1029: object-name host object ungroup should exit successfully");
    expect(visual_object_is_deleted(object_name_path, "group-guid") &&
            visual_object_parent(object_name_path, "name-guid") == "frmCustomer" &&
            visual_object_parent(object_name_path, "save-guid") == "frmCustomer",
        "#1029: object-name host object ungroup should move children to the container parent and delete the container");

    const fs::path unique_id_path = temp_root / "unique_id.scx";
    write_synthetic_form_table_for_object_ungroup(unique_id_path);
    const auto unique_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", unique_id_path.string(),
            "--ungroup-object",
            "--unique-id", "group-guid",
            "--json"
        },
        temp_root);
    expect(unique_id_process.exit_code == 0,
        "#1029: unique-id host object ungroup should exit successfully");
    expect(visual_object_is_deleted(unique_id_path, "group-guid") &&
            visual_object_parent(unique_id_path, "name-guid") == "frmCustomer" &&
            visual_object_parent(unique_id_path, "save-guid") == "frmCustomer",
        "#1029: unique-id host object ungroup should move children to the container parent and delete the container");

    const fs::path root_path = temp_root / "root.scx";
    write_synthetic_form_table_for_object_ungroup(root_path);
    const auto root_process = run_process_capture(
        studio_host_path,
        {
            "--path", root_path.string(),
            "--ungroup-object",
            "--unique-id", "root-group-guid",
            "--json"
        },
        temp_root);
    expect(root_process.exit_code == 0,
        "#1029: root-level host object ungroup should exit successfully");
    expect(visual_object_is_deleted(root_path, "root-group-guid") &&
            visual_object_parent(root_path, "root-child-guid").empty(),
        "#1029: root-level host object ungroup should clear child parents and delete the container");

    const fs::path empty_path = temp_root / "empty.scx";
    write_synthetic_form_table_for_object_ungroup(empty_path);
    const auto empty_process = run_process_capture(
        studio_host_path,
        {
            "--path", empty_path.string(),
            "--ungroup-object",
            "--unique-id", "empty-guid",
            "--json"
        },
        temp_root);
    expect(empty_process.exit_code == 4,
        "#1029: empty-container host object ungroup should return command failure");
    expect(!visual_object_is_deleted(empty_path, "empty-guid") &&
            visual_object_parent(empty_path, "name-guid") == "cntGroup",
        "#1029: empty-container host object ungroup should not mutate the asset");

    const fs::path missing_path = temp_root / "missing.scx";
    write_synthetic_form_table_for_object_ungroup(missing_path);
    const auto missing_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_path.string(),
            "--ungroup-object",
            "--unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_process.exit_code == 4,
        "#1029: missing-container host object ungroup should return command failure");
    expect(!visual_object_is_deleted(missing_path, "group-guid") &&
            visual_object_parent(missing_path, "name-guid") == "cntGroup",
        "#1029: missing-container host object ungroup should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_ungroup(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--ungroup-object",
            "--reorder-object",
            "--unique-id", "group-guid",
            "--placement", "front",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1029: ungroup-object plus reorder-object requests should fail during launch parsing");
    expect(!visual_object_is_deleted(ambiguous_path, "group-guid") &&
            visual_object_parent(ambiguous_path, "name-guid") == "cntGroup",
        "#1029: ungroup-object/reorder-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
