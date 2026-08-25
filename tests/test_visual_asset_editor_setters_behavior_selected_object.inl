void test_set_visual_object_deleted_state_targets_selected_object() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_deleted_state_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "deleted_state.scx";
    const fs::path memo_path = temp_dir / "deleted_state.sct";
    write_synthetic_named_geometry_asset(table_path, memo_path);

    auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#741: visual object deleted-state edits should support UNIQUEID selection");
    expect(delete_result.affected_object_count == 1U,
        "#1006: successful deleted-state edit should report one affected object");

    auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
    expect(parse_result.ok, "#741: deleted-state fixture should remain readable after delete");
    if (parse_result.ok && parse_result.table.records.size() == 2U) {
        expect(!parse_result.table.records[0].deleted,
            "#741: selected delete should preserve unrelated records");
        expect(parse_result.table.records[1].deleted,
            "#741: selected delete should mark the resolved record deleted");
    }
    auto query_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid",
        .property_name = "HPOS"
    });
    expect(query_result.ok && query_result.record_deleted,
        "#742: property queries should report deleted state for the resolved selected object");
    auto list_result = copperfin::vfp::list_visual_object_properties({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid"
    });
    expect(list_result.ok && list_result.record_deleted,
        "#742: property listings should report deleted state for the resolved selected object");

    auto restore_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .deleted = false
    });
    expect(restore_result.ok, "#741: visual object deleted-state edits should support object-name restore");
    expect(restore_result.affected_object_count == 1U,
        "#1006: successful deleted-state restore should report one affected object");

    parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
    expect(parse_result.ok, "#741: deleted-state fixture should remain readable after restore");
    if (parse_result.ok && parse_result.table.records.size() == 2U) {
        expect(!parse_result.table.records[0].deleted,
            "#741: selected restore should preserve unrelated records");
        expect(!parse_result.table.records[1].deleted,
            "#741: selected restore should clear the resolved record deleted flag");
    }
    query_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .property_name = "HPOS"
    });
    expect(query_result.ok && !query_result.record_deleted,
        "#742: property queries should report restored live state for the resolved selected object");
    list_result = copperfin::vfp::list_visual_object_properties({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {}
    });
    expect(list_result.ok && !list_result.record_deleted,
        "#742: property listings should report restored live state for the resolved selected object");

    delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "missing-guid",
        .deleted = true
    });
    expect(!delete_result.ok, "#741: missing selected objects should not mutate deleted state");
    expect(delete_result.affected_object_count == 0U,
        "#1006: failed deleted-state edit should report zero affected objects");

    parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
    expect(parse_result.ok, "#741: deleted-state fixture should remain readable after failed selection");
    if (parse_result.ok && parse_result.table.records.size() == 2U) {
        expect(!parse_result.table.records[0].deleted && !parse_result.table.records[1].deleted,
            "#741: failed deleted-state selection should preserve all record flags");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_tab_order_assigns_sequential_indexes() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_tab_order_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "tab_order.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "TABINDEX", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "oneButton", "one-guid", "10"},
        {"cmdTwo", "twoButton", "two-guid", "20"},
        {"cmdThree", "threeButton", "three-guid", "30"},
        {"cmdOther", "otherButton", "other-guid", "99"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#793: tab-order fixture should be writable");

    const auto tab_index = [&](const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "TABINDEX"
        });
        expect(result.ok && result.exists, "#793: tab-order fixture property should be readable");
        return result.value;
    };
    const auto tab_state = [&]() {
        return tab_index("one-guid") + "," +
            tab_index("two-guid") + "," +
            tab_index("three-guid") + "," +
            tab_index("other-guid");
    };

    auto tab_result = copperfin::vfp::set_visual_object_tab_order({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "two-guid"},
            {.record_index = 0U, .object_name = "cmdOne", .unique_id = {}},
            {.record_index = 2U, .object_name = {}, .unique_id = {}}
        },
        .starting_tab_index = 5
    });
    expect(tab_result.ok, "#793: tab order should support mixed selectors in caller order");
    expect(tab_result.affected_object_count == 3U,
        "#1000: successful tab-order assignment should report affected object count");
    expect(tab_index("two-guid") == "5" &&
            tab_index("one-guid") == "6" &&
            tab_index("three-guid") == "7" &&
            tab_index("other-guid") == "99",
        "#793: tab order should assign sequential indexes and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#793: first tab-order write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#793: second tab-order write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#793: third tab-order write should remain undo-backed");
    expect(tab_state() == "10,20,30,99", "#793: tab-order undo should restore original indexes");

    tab_result = copperfin::vfp::set_visual_object_tab_order({
        .path = table_path.string(),
        .objects = {
            {.record_index = 2U, .object_name = {}, .unique_id = {}},
            {.record_index = 0U, .object_name = "cmdOne", .unique_id = {}}
        },
        .starting_tab_index = 0
    });
    expect(tab_result.ok, "#793: tab order should support zero-based assignment");
    expect(tab_index("three-guid") == "0" &&
            tab_index("one-guid") == "1" &&
            tab_index("two-guid") == "20",
        "#793: zero-based tab order should use caller-provided ordering");

    const std::string committed_state = tab_state();
    tab_result = copperfin::vfp::set_visual_object_tab_order({
        .path = table_path.string(),
        .objects = {},
        .starting_tab_index = 1
    });
    expect(!tab_result.ok, "#793: tab order should reject empty selections");
    expect(tab_result.affected_object_count == 0U,
        "#1000: failed tab-order assignment should report zero affected objects");
    expect(tab_state() == committed_state, "#793: empty-selection failures should not mutate tab indexes");

    tab_result = copperfin::vfp::set_visual_object_tab_order({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"}
        },
        .starting_tab_index = -1
    });
    expect(!tab_result.ok, "#793: tab order should reject negative starting indexes");
    expect(tab_state() == committed_state, "#793: negative-start failures should not mutate tab indexes");

    tab_result = copperfin::vfp::set_visual_object_tab_order({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .starting_tab_index = 1
    });
    expect(!tab_result.ok, "#793: tab order should reject missing selected objects");
    expect(tab_state() == committed_state, "#793: missing-object failures should not mutate tab indexes");

    tab_result = copperfin::vfp::set_visual_object_tab_order({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = "cmdOne", .unique_id = {}}
        },
        .starting_tab_index = 1
    });
    expect(!tab_result.ok, "#793: tab order should reject duplicate selected objects");
    expect(tab_state() == committed_state, "#793: duplicate-selection failures should not mutate tab indexes");

    const fs::path incomplete_path = temp_dir / "missing_tabindex.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"cmdA", "a-guid"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#793: missing-TABINDEX fixture should be writable");

    tab_result = copperfin::vfp::set_visual_object_tab_order({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .starting_tab_index = 1
    });
    expect(!tab_result.ok, "#793: tab order should reject missing TABINDEX fields");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_tab_stop_assigns_logical_state() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_tab_stop_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "tab_stop.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "TABSTOP", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "oneButton", "one-guid", ".T."},
        {"cmdTwo", "twoButton", "two-guid", ".T."},
        {"cmdThree", "threeButton", "three-guid", ".F."},
        {"cmdOther", "otherButton", "other-guid", ".T."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#794: tab-stop fixture should be writable");

    const auto tab_stop = [&](const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "TABSTOP"
        });
        expect(result.ok && result.exists, "#794: tab-stop fixture property should be readable");
        return result.value;
    };
    const auto tab_stop_state = [&]() {
        return tab_stop("one-guid") + "," +
            tab_stop("two-guid") + "," +
            tab_stop("three-guid") + "," +
            tab_stop("other-guid");
    };

    auto tab_result = copperfin::vfp::set_visual_object_tab_stop({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cmdOne", .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "two-guid"}
        },
        .tab_stop = false
    });
    expect(tab_result.ok, "#794: tab-stop assignment should support mixed selectors");
    expect(tab_result.affected_object_count == 2U,
        "#1000: successful tab-stop assignment should report affected object count");
    expect(tab_stop("one-guid") == ".F." &&
            tab_stop("two-guid") == ".F." &&
            tab_stop("three-guid") == ".F." &&
            tab_stop("other-guid") == ".T.",
        "#794: tab-stop false assignment should preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#794: first tab-stop write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#794: second tab-stop write should remain undo-backed");
    expect(tab_stop_state() == ".T.,.T.,.F.,.T.", "#794: tab-stop undo should restore original states");

    tab_result = copperfin::vfp::set_visual_object_tab_stop({
        .path = table_path.string(),
        .objects = {
            {.record_index = 2U, .object_name = {}, .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"}
        },
        .tab_stop = true
    });
    expect(tab_result.ok, "#794: tab-stop true assignment should support record-index selectors");
    expect(tab_stop("three-guid") == ".T." &&
            tab_stop("one-guid") == ".T." &&
            tab_stop("two-guid") == ".T.",
        "#794: tab-stop true assignment should use FoxPro logical formatting");

    const std::string committed_state = tab_stop_state();
    tab_result = copperfin::vfp::set_visual_object_tab_stop({
        .path = table_path.string(),
        .objects = {},
        .tab_stop = true
    });
    expect(!tab_result.ok, "#794: tab-stop assignment should reject empty selections");
    expect(tab_result.affected_object_count == 0U,
        "#1000: failed tab-stop assignment should report zero affected objects");
    expect(tab_stop_state() == committed_state, "#794: empty-selection failures should not mutate tab-stop states");

    tab_result = copperfin::vfp::set_visual_object_tab_stop({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .tab_stop = false
    });
    expect(!tab_result.ok, "#794: tab-stop assignment should reject missing selected objects");
    expect(tab_stop_state() == committed_state, "#794: missing-object failures should not mutate tab-stop states");

    tab_result = copperfin::vfp::set_visual_object_tab_stop({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = "cmdOne", .unique_id = {}}
        },
        .tab_stop = false
    });
    expect(!tab_result.ok, "#794: tab-stop assignment should reject duplicate selected objects");
    expect(tab_stop_state() == committed_state, "#794: duplicate-selection failures should not mutate tab-stop states");

    const fs::path incomplete_path = temp_dir / "missing_tabstop.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"cmdA", "a-guid"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#794: missing-TABSTOP fixture should be writable");

    tab_result = copperfin::vfp::set_visual_object_tab_stop({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .tab_stop = true
    });
    expect(!tab_result.ok, "#794: tab-stop assignment should reject missing TABSTOP fields");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_visibility_assigns_logical_state() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_visibility_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "visibility.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "VISIBLE", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "oneButton", "one-guid", ".T."},
        {"cmdTwo", "twoButton", "two-guid", ".T."},
        {"cmdThree", "threeButton", "three-guid", ".F."},
        {"cmdOther", "otherButton", "other-guid", ".T."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#795: visibility fixture should be writable");

    const auto visibility = [&](const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "VISIBLE"
        });
        expect(result.ok && result.exists, "#795: visibility fixture property should be readable");
        return result.value;
    };
    const auto visibility_state = [&]() {
        return visibility("one-guid") + "," +
            visibility("two-guid") + "," +
            visibility("three-guid") + "," +
            visibility("other-guid");
    };

    auto visibility_result = copperfin::vfp::set_visual_object_visibility({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cmdOne", .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "two-guid"}
        },
        .visible = false
    });
    expect(visibility_result.ok, "#795: visibility assignment should support mixed selectors");
    expect(visibility_result.affected_object_count == 2U,
        "#1001: successful visibility assignment should report affected object count");
    expect(visibility("one-guid") == ".F." &&
            visibility("two-guid") == ".F." &&
            visibility("three-guid") == ".F." &&
            visibility("other-guid") == ".T.",
        "#795: visibility false assignment should preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#795: first visibility write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#795: second visibility write should remain undo-backed");
    expect(visibility_state() == ".T.,.T.,.F.,.T.", "#795: visibility undo should restore original states");

    visibility_result = copperfin::vfp::set_visual_object_visibility({
        .path = table_path.string(),
        .objects = {
            {.record_index = 2U, .object_name = {}, .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"}
        },
        .visible = true
    });
    expect(visibility_result.ok, "#795: visibility true assignment should support record-index selectors");
    expect(visibility("three-guid") == ".T." &&
            visibility("one-guid") == ".T." &&
            visibility("two-guid") == ".T.",
        "#795: visibility true assignment should use FoxPro logical formatting");

    const std::string committed_state = visibility_state();
    visibility_result = copperfin::vfp::set_visual_object_visibility({
        .path = table_path.string(),
        .objects = {},
        .visible = true
    });
    expect(!visibility_result.ok, "#795: visibility assignment should reject empty selections");
    expect(visibility_result.affected_object_count == 0U,
        "#1001: failed visibility assignment should report zero affected objects");
    expect(visibility_state() == committed_state, "#795: empty-selection failures should not mutate visibility states");

    visibility_result = copperfin::vfp::set_visual_object_visibility({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .visible = false
    });
    expect(!visibility_result.ok, "#795: visibility assignment should reject missing selected objects");
    expect(visibility_state() == committed_state, "#795: missing-object failures should not mutate visibility states");

    visibility_result = copperfin::vfp::set_visual_object_visibility({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = "cmdOne", .unique_id = {}}
        },
        .visible = false
    });
    expect(!visibility_result.ok, "#795: visibility assignment should reject duplicate selected objects");
    expect(visibility_state() == committed_state, "#795: duplicate-selection failures should not mutate visibility states");

    const fs::path incomplete_path = temp_dir / "missing_visible.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"cmdA", "a-guid"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#795: missing-VISIBLE fixture should be writable");

    visibility_result = copperfin::vfp::set_visual_object_visibility({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .visible = true
    });
    expect(!visibility_result.ok, "#795: visibility assignment should reject missing VISIBLE fields");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_enabled_assigns_logical_state() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_enabled_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "enabled.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "ENABLED", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "oneButton", "one-guid", ".T."},
        {"cmdTwo", "twoButton", "two-guid", ".T."},
        {"cmdThree", "threeButton", "three-guid", ".F."},
        {"cmdOther", "otherButton", "other-guid", ".T."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#796: enabled fixture should be writable");

    const auto enabled_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "ENABLED"
        });
        expect(result.ok && result.exists, "#796: enabled fixture property should be readable");
        return result.value;
    };
    const auto enabled = [&](const std::string& unique_id) {
        return enabled_for(table_path.string(), unique_id);
    };
    const auto enabled_state = [&]() {
        return enabled("one-guid") + "," +
            enabled("two-guid") + "," +
            enabled("three-guid") + "," +
            enabled("other-guid");
    };

    auto enabled_result = copperfin::vfp::set_visual_object_enabled({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cmdOne", .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "two-guid"}
        },
        .enabled = false
    });
    expect(enabled_result.ok, "#796: enabled assignment should support mixed selectors");
    expect(enabled_result.affected_object_count == 2U,
        "#1001: successful enabled assignment should report affected object count");
    expect(enabled("one-guid") == ".F." &&
            enabled("two-guid") == ".F." &&
            enabled("three-guid") == ".F." &&
            enabled("other-guid") == ".T.",
        "#796: enabled false assignment should preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#796: first enabled write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#796: second enabled write should remain undo-backed");
    expect(enabled_state() == ".T.,.T.,.F.,.T.", "#796: enabled undo should restore original states");

    enabled_result = copperfin::vfp::set_visual_object_enabled({
        .path = table_path.string(),
        .objects = {
            {.record_index = 2U, .object_name = {}, .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"}
        },
        .enabled = true
    });
    expect(enabled_result.ok, "#796: enabled true assignment should support record-index selectors");
    expect(enabled("three-guid") == ".T." &&
            enabled("one-guid") == ".T." &&
            enabled("two-guid") == ".T.",
        "#796: enabled true assignment should use FoxPro logical formatting");

    const std::string committed_state = enabled_state();
    enabled_result = copperfin::vfp::set_visual_object_enabled({
        .path = table_path.string(),
        .objects = {},
        .enabled = true
    });
    expect(!enabled_result.ok, "#796: enabled assignment should reject empty selections");
    expect(enabled_result.affected_object_count == 0U,
        "#1001: failed enabled assignment should report zero affected objects");
    expect(enabled_state() == committed_state, "#796: empty-selection failures should not mutate enabled states");

    enabled_result = copperfin::vfp::set_visual_object_enabled({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .enabled = false
    });
    expect(!enabled_result.ok, "#796: enabled assignment should reject missing selected objects");
    expect(enabled_state() == committed_state, "#796: missing-object failures should not mutate enabled states");

    enabled_result = copperfin::vfp::set_visual_object_enabled({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = "cmdOne", .unique_id = {}}
        },
        .enabled = false
    });
    expect(!enabled_result.ok, "#796: enabled assignment should reject duplicate selected objects");
    expect(enabled_state() == committed_state, "#796: duplicate-selection failures should not mutate enabled states");

    const fs::path blob_path = temp_dir / "enabled_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cmdBlob", "blob-guid", "Caption = \"Blob\"\r\nEnabled = .T.\r\n"},
        {"cmdNoEnabled", "no-enabled-guid", "Caption = \"No Enabled\"\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#796: enabled property-blob fixture should be writable");

    enabled_result = copperfin::vfp::set_visual_object_enabled({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"}
        },
        .enabled = false
    });
    expect(enabled_result.ok, "#796: enabled assignment should support existing serialized properties");
    expect(enabled_for(blob_path.string(), "blob-guid") == ".F.",
        "#796: serialized enabled assignment should preserve property lookup");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#796: serialized enabled write should remain undo-backed");
    expect(enabled_for(blob_path.string(), "blob-guid") == ".T.",
        "#796: serialized enabled undo should restore original property value");

    enabled_result = copperfin::vfp::set_visual_object_enabled({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "no-enabled-guid"}
        },
        .enabled = true
    });
    expect(!enabled_result.ok, "#796: enabled assignment should reject missing serialized ENABLED properties");

    const fs::path incomplete_path = temp_dir / "missing_enabled.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"cmdA", "a-guid"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#796: missing-ENABLED fixture should be writable");

    enabled_result = copperfin::vfp::set_visual_object_enabled({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .enabled = true
    });
    expect(!enabled_result.ok, "#796: enabled assignment should reject missing ENABLED fields");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_read_only_assigns_logical_state() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_read_only_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "readonly.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "READONLY", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtOne", "oneBox", "one-guid", ".F."},
        {"txtTwo", "twoBox", "two-guid", ".F."},
        {"txtThree", "threeBox", "three-guid", ".T."},
        {"txtOther", "otherBox", "other-guid", ".F."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#797: read-only fixture should be writable");

    const auto read_only_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "READONLY"
        });
        expect(result.ok && result.exists, "#797: read-only fixture property should be readable");
        return result.value;
    };
    const auto read_only = [&](const std::string& unique_id) {
        return read_only_for(table_path.string(), unique_id);
    };
    const auto read_only_state = [&]() {
        return read_only("one-guid") + "," +
            read_only("two-guid") + "," +
            read_only("three-guid") + "," +
            read_only("other-guid");
    };

    auto read_only_result = copperfin::vfp::set_visual_object_read_only({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "txtOne", .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "two-guid"}
        },
        .read_only = true
    });
    expect(read_only_result.ok, "#797: read-only assignment should support mixed selectors");
    expect(read_only_result.affected_object_count == 2U,
        "#1001: successful read-only assignment should report affected object count");
    expect(read_only("one-guid") == ".T." &&
            read_only("two-guid") == ".T." &&
            read_only("three-guid") == ".T." &&
            read_only("other-guid") == ".F.",
        "#797: read-only true assignment should preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#797: first read-only write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#797: second read-only write should remain undo-backed");
    expect(read_only_state() == ".F.,.F.,.T.,.F.", "#797: read-only undo should restore original states");

    read_only_result = copperfin::vfp::set_visual_object_read_only({
        .path = table_path.string(),
        .objects = {
            {.record_index = 2U, .object_name = {}, .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"}
        },
        .read_only = false
    });
    expect(read_only_result.ok, "#797: editable assignment should support record-index selectors");
    expect(read_only("three-guid") == ".F." &&
            read_only("one-guid") == ".F." &&
            read_only("two-guid") == ".F.",
        "#797: editable assignment should use FoxPro logical formatting");

    const std::string committed_state = read_only_state();
    read_only_result = copperfin::vfp::set_visual_object_read_only({
        .path = table_path.string(),
        .objects = {},
        .read_only = true
    });
    expect(!read_only_result.ok, "#797: read-only assignment should reject empty selections");
    expect(read_only_result.affected_object_count == 0U,
        "#1001: failed read-only assignment should report zero affected objects");
    expect(read_only_state() == committed_state, "#797: empty-selection failures should not mutate read-only states");

    read_only_result = copperfin::vfp::set_visual_object_read_only({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .read_only = true
    });
    expect(!read_only_result.ok, "#797: read-only assignment should reject missing selected objects");
    expect(read_only_state() == committed_state, "#797: missing-object failures should not mutate read-only states");

    read_only_result = copperfin::vfp::set_visual_object_read_only({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = "txtOne", .unique_id = {}}
        },
        .read_only = true
    });
    expect(!read_only_result.ok, "#797: read-only assignment should reject duplicate selected objects");
    expect(read_only_state() == committed_state, "#797: duplicate-selection failures should not mutate read-only states");

    const fs::path blob_path = temp_dir / "readonly_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", "Caption = \"Blob\"\r\nReadOnly = .F.\r\n"},
        {"txtNoReadOnly", "no-readonly-guid", "Caption = \"No ReadOnly\"\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#797: read-only property-blob fixture should be writable");

    read_only_result = copperfin::vfp::set_visual_object_read_only({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"}
        },
        .read_only = true
    });
    expect(read_only_result.ok, "#797: read-only assignment should support existing serialized properties");
    expect(read_only_for(blob_path.string(), "blob-guid") == ".T.",
        "#797: serialized read-only assignment should preserve property lookup");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#797: serialized read-only write should remain undo-backed");
    expect(read_only_for(blob_path.string(), "blob-guid") == ".F.",
        "#797: serialized read-only undo should restore original property value");

    read_only_result = copperfin::vfp::set_visual_object_read_only({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "no-readonly-guid"}
        },
        .read_only = true
    });
    expect(!read_only_result.ok, "#797: read-only assignment should reject missing serialized READONLY properties");

    const fs::path incomplete_path = temp_dir / "missing_readonly.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"txtA", "a-guid"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#797: missing-READONLY fixture should be writable");

    read_only_result = copperfin::vfp::set_visual_object_read_only({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .read_only = true
    });
    expect(!read_only_result.ok, "#797: read-only assignment should reject missing READONLY fields");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_locked_assigns_logical_state() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_locked_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "locked.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "LOCKED", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtOne", "oneBox", "one-guid", ".F."},
        {"txtTwo", "twoBox", "two-guid", ".F."},
        {"txtThree", "threeBox", "three-guid", ".T."},
        {"txtOther", "otherBox", "other-guid", ".F."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#798: locked fixture should be writable");

    const auto locked_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "LOCKED"
        });
        expect(result.ok && result.exists, "#798: locked fixture property should be readable");
        return result.value;
    };
    const auto locked = [&](const std::string& unique_id) {
        return locked_for(table_path.string(), unique_id);
    };
    const auto locked_state = [&]() {
        return locked("one-guid") + "," +
            locked("two-guid") + "," +
            locked("three-guid") + "," +
            locked("other-guid");
    };

    auto locked_result = copperfin::vfp::set_visual_object_locked({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "txtOne", .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "two-guid"}
        },
        .locked = true
    });
    expect(locked_result.ok, "#798: locked assignment should support mixed selectors");
    expect(locked_result.affected_object_count == 2U,
        "#1001: successful locked assignment should report affected object count");
    expect(locked("one-guid") == ".T." &&
            locked("two-guid") == ".T." &&
            locked("three-guid") == ".T." &&
            locked("other-guid") == ".F.",
        "#798: locked true assignment should preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#798: first locked write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#798: second locked write should remain undo-backed");
    expect(locked_state() == ".F.,.F.,.T.,.F.", "#798: locked undo should restore original states");

    locked_result = copperfin::vfp::set_visual_object_locked({
        .path = table_path.string(),
        .objects = {
            {.record_index = 2U, .object_name = {}, .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"}
        },
        .locked = false
    });
    expect(locked_result.ok, "#798: unlocked assignment should support record-index selectors");
    expect(locked("three-guid") == ".F." &&
            locked("one-guid") == ".F." &&
            locked("two-guid") == ".F.",
        "#798: unlocked assignment should use FoxPro logical formatting");

    const std::string committed_state = locked_state();
    locked_result = copperfin::vfp::set_visual_object_locked({
        .path = table_path.string(),
        .objects = {},
        .locked = true
    });
    expect(!locked_result.ok, "#798: locked assignment should reject empty selections");
    expect(locked_result.affected_object_count == 0U,
        "#1001: failed locked assignment should report zero affected objects");
    expect(locked_state() == committed_state, "#798: empty-selection failures should not mutate locked states");

    locked_result = copperfin::vfp::set_visual_object_locked({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .locked = true
    });
    expect(!locked_result.ok, "#798: locked assignment should reject missing selected objects");
    expect(locked_state() == committed_state, "#798: missing-object failures should not mutate locked states");

    locked_result = copperfin::vfp::set_visual_object_locked({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = "txtOne", .unique_id = {}}
        },
        .locked = true
    });
    expect(!locked_result.ok, "#798: locked assignment should reject duplicate selected objects");
    expect(locked_state() == committed_state, "#798: duplicate-selection failures should not mutate locked states");

    const fs::path blob_path = temp_dir / "locked_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", "Caption = \"Blob\"\r\nLocked = .F.\r\n"},
        {"txtNoLocked", "no-locked-guid", "Caption = \"No Locked\"\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#798: locked property-blob fixture should be writable");

    locked_result = copperfin::vfp::set_visual_object_locked({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"}
        },
        .locked = true
    });
    expect(locked_result.ok, "#798: locked assignment should support existing serialized properties");
    expect(locked_for(blob_path.string(), "blob-guid") == ".T.",
        "#798: serialized locked assignment should preserve property lookup");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#798: serialized locked write should remain undo-backed");
    expect(locked_for(blob_path.string(), "blob-guid") == ".F.",
        "#798: serialized locked undo should restore original property value");

    locked_result = copperfin::vfp::set_visual_object_locked({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "no-locked-guid"}
        },
        .locked = true
    });
    expect(!locked_result.ok, "#798: locked assignment should reject missing serialized LOCKED properties");

    const fs::path incomplete_path = temp_dir / "missing_locked.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"txtA", "a-guid"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#798: missing-LOCKED fixture should be writable");

    locked_result = copperfin::vfp::set_visual_object_locked({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .locked = true
    });
    expect(!locked_result.ok, "#798: locked assignment should reject missing LOCKED fields");

    fs::remove_all(temp_dir, ignored);
}
