// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
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

void test_set_visual_object_caption_assigns_text() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_caption_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "caption.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "CAPTION", .type = 'C', .length = 50U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "oneButton", "one-guid", "One"},
        {"cmdTwo", "twoButton", "two-guid", "Two"},
        {"cmdThree", "threeButton", "three-guid", "Three"},
        {"cmdOther", "otherButton", "other-guid", "Other"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#799: caption fixture should be writable");

    const auto caption_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "Caption"
        });
        expect(result.ok && result.exists, "#799: caption fixture property should be readable");
        return result.value;
    };
    const auto caption = [&](const std::string& unique_id) {
        return caption_for(table_path.string(), unique_id);
    };
    const auto caption_state = [&]() {
        return caption("one-guid") + "," +
            caption("two-guid") + "," +
            caption("three-guid") + "," +
            caption("other-guid");
    };

    auto caption_result = copperfin::vfp::set_visual_object_caption({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cmdOne", .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "two-guid"}
        },
        .caption = "Primary Action"
    });
    expect(caption_result.ok, "#799: caption assignment should support mixed selectors");
    expect(caption_result.affected_object_count == 2U,
        "#1001: successful caption assignment should report affected object count");
    expect(caption("one-guid") == "Primary Action" &&
            caption("two-guid") == "Primary Action" &&
            caption("three-guid") == "Three" &&
            caption("other-guid") == "Other",
        "#799: direct caption assignment should write raw text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#799: first caption write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#799: second caption write should remain undo-backed");
    expect(caption_state() == "One,Two,Three,Other", "#799: caption undo should restore original direct values");

    caption_result = copperfin::vfp::set_visual_object_caption({
        .path = table_path.string(),
        .objects = {
            {.record_index = 2U, .object_name = {}, .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"}
        },
        .caption = "Ready \"Now\""
    });
    expect(caption_result.ok, "#799: caption assignment should support record-index selectors");
    expect(caption("three-guid") == "Ready \"Now\"" &&
            caption("one-guid") == "Ready \"Now\"" &&
            caption("two-guid") == "Two",
        "#799: direct caption assignment should store caller text without serialized quoting");

    const std::string committed_state = caption_state();
    caption_result = copperfin::vfp::set_visual_object_caption({
        .path = table_path.string(),
        .objects = {},
        .caption = "Ignored"
    });
    expect(!caption_result.ok, "#799: caption assignment should reject empty selections");
    expect(caption_result.affected_object_count == 0U,
        "#1001: failed caption assignment should report zero affected objects");
    expect(caption_state() == committed_state, "#799: empty-selection failures should not mutate captions");

    caption_result = copperfin::vfp::set_visual_object_caption({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .caption = "Ignored"
    });
    expect(!caption_result.ok, "#799: caption assignment should reject missing selected objects");
    expect(caption_state() == committed_state, "#799: missing-object failures should not mutate captions");

    caption_result = copperfin::vfp::set_visual_object_caption({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = "cmdOne", .unique_id = {}}
        },
        .caption = "Ignored"
    });
    expect(!caption_result.ok, "#799: caption assignment should reject duplicate selected objects");
    expect(caption_state() == committed_state, "#799: duplicate-selection failures should not mutate captions");

    const fs::path blob_path = temp_dir / "caption_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cmdBlob", "blob-guid", "Caption = \"Blob\"\r\nEnabled = .T.\r\n"},
        {"cmdNoCaption", "no-caption-guid", "Enabled = .T.\r\n"},
        {"cmdOther", "other-guid", "Caption = \"Other\"\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#799: caption property-blob fixture should be writable");

    const auto blob_caption_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "Caption"
        });
    };

    caption_result = copperfin::vfp::set_visual_object_caption({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cmdNoCaption", .unique_id = {}}
        },
        .caption = "He said \"Run\""
    });
    expect(caption_result.ok, "#799: caption assignment should support existing and absent serialized properties");
    auto blob_caption = blob_caption_state("blob-guid");
    auto appended_caption = blob_caption_state("no-caption-guid");
    auto other_caption = blob_caption_state("other-guid");
    expect(blob_caption.ok && blob_caption.exists && blob_caption.value == "\"He said \"\"Run\"\"\"" &&
            appended_caption.ok && appended_caption.exists && appended_caption.value == "\"He said \"\"Run\"\"\"" &&
            other_caption.ok && other_caption.exists && other_caption.value == "\"Other\"",
        "#799: serialized caption assignment should quote text, append missing Caption, and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#799: appended serialized caption write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#799: existing serialized caption write should remain undo-backed");
    blob_caption = blob_caption_state("blob-guid");
    appended_caption = blob_caption_state("no-caption-guid");
    expect(blob_caption.ok && blob_caption.exists && blob_caption.value == "\"Blob\"" &&
            appended_caption.ok && !appended_caption.exists,
        "#799: serialized caption undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_caption.scx";
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
    expect(incomplete_create.ok, "#799: missing-Caption fixture should be writable");

    caption_result = copperfin::vfp::set_visual_object_caption({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .caption = "Ignored"
    });
    expect(!caption_result.ok, "#799: caption assignment should reject objects without a writable Caption carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_tooltip_text_assigns_text() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_tooltip_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "tooltip.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "TOOLTIPTEX", .type = 'C', .length = 60U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "oneButton", "one-guid", "One tip"},
        {"cmdTwo", "twoButton", "two-guid", "Two tip"},
        {"cmdThree", "threeButton", "three-guid", "Three tip"},
        {"cmdOther", "otherButton", "other-guid", "Other tip"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#800: tooltip fixture should be writable");

    const auto tooltip_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "ToolTipText"
        });
        expect(result.ok && result.exists, "#800: tooltip fixture property should be readable");
        return result.value;
    };
    const auto tooltip = [&](const std::string& unique_id) {
        return tooltip_for(table_path.string(), unique_id);
    };
    const auto tooltip_state = [&]() {
        return tooltip("one-guid") + "," +
            tooltip("two-guid") + "," +
            tooltip("three-guid") + "," +
            tooltip("other-guid");
    };

    auto tooltip_result = copperfin::vfp::set_visual_object_tooltip_text({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cmdOne", .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "two-guid"}
        },
        .tooltip_text = "Save the current record"
    });
    expect(tooltip_result.ok, "#800: tooltip assignment should support mixed selectors");
    expect(tooltip_result.affected_object_count == 2U,
        "#1002: successful tooltip assignment should report affected object count");
    expect(tooltip("one-guid") == "Save the current record" &&
            tooltip("two-guid") == "Save the current record" &&
            tooltip("three-guid") == "Three tip" &&
            tooltip("other-guid") == "Other tip",
        "#800: direct tooltip assignment should write raw text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#800: first tooltip write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#800: second tooltip write should remain undo-backed");
    expect(tooltip_state() == "One tip,Two tip,Three tip,Other tip",
        "#800: tooltip undo should restore original direct values");

    tooltip_result = copperfin::vfp::set_visual_object_tooltip_text({
        .path = table_path.string(),
        .objects = {
            {.record_index = 2U, .object_name = {}, .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"}
        },
        .tooltip_text = "Use \"Enter\""
    });
    expect(tooltip_result.ok, "#800: tooltip assignment should support record-index selectors");
    expect(tooltip("three-guid") == "Use \"Enter\"" &&
            tooltip("one-guid") == "Use \"Enter\"" &&
            tooltip("two-guid") == "Two tip",
        "#800: direct tooltip assignment should store caller text without serialized quoting");

    const std::string committed_state = tooltip_state();
    tooltip_result = copperfin::vfp::set_visual_object_tooltip_text({
        .path = table_path.string(),
        .objects = {},
        .tooltip_text = "Ignored"
    });
    expect(!tooltip_result.ok, "#800: tooltip assignment should reject empty selections");
    expect(tooltip_result.affected_object_count == 0U,
        "#1002: failed tooltip assignment should report zero affected objects");
    expect(tooltip_state() == committed_state, "#800: empty-selection failures should not mutate tooltips");

    tooltip_result = copperfin::vfp::set_visual_object_tooltip_text({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .tooltip_text = "Ignored"
    });
    expect(!tooltip_result.ok, "#800: tooltip assignment should reject missing selected objects");
    expect(tooltip_state() == committed_state, "#800: missing-object failures should not mutate tooltips");

    tooltip_result = copperfin::vfp::set_visual_object_tooltip_text({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = "cmdOne", .unique_id = {}}
        },
        .tooltip_text = "Ignored"
    });
    expect(!tooltip_result.ok, "#800: tooltip assignment should reject duplicate selected objects");
    expect(tooltip_state() == committed_state, "#800: duplicate-selection failures should not mutate tooltips");

    const fs::path blob_path = temp_dir / "tooltip_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cmdBlob", "blob-guid", "Caption = \"Blob\"\r\nToolTipText = \"Blob tip\"\r\n"},
        {"cmdNoTooltip", "no-tooltip-guid", "Caption = \"No tooltip\"\r\n"},
        {"cmdOther", "other-guid", "ToolTipText = \"Other tip\"\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#800: tooltip property-blob fixture should be writable");

    const auto blob_tooltip_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "ToolTipText"
        });
    };

    tooltip_result = copperfin::vfp::set_visual_object_tooltip_text({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cmdNoTooltip", .unique_id = {}}
        },
        .tooltip_text = "Press \"F1\" for help"
    });
    expect(tooltip_result.ok, "#800: tooltip assignment should support existing and absent serialized properties");
    auto blob_tooltip = blob_tooltip_state("blob-guid");
    auto appended_tooltip = blob_tooltip_state("no-tooltip-guid");
    auto other_tooltip = blob_tooltip_state("other-guid");
    expect(blob_tooltip.ok && blob_tooltip.exists && blob_tooltip.value == "\"Press \"\"F1\"\" for help\"" &&
            appended_tooltip.ok && appended_tooltip.exists && appended_tooltip.value == "\"Press \"\"F1\"\" for help\"" &&
            other_tooltip.ok && other_tooltip.exists && other_tooltip.value == "\"Other tip\"",
        "#800: serialized tooltip assignment should quote text, append missing ToolTipText, and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#800: appended serialized tooltip write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#800: existing serialized tooltip write should remain undo-backed");
    blob_tooltip = blob_tooltip_state("blob-guid");
    appended_tooltip = blob_tooltip_state("no-tooltip-guid");
    expect(blob_tooltip.ok && blob_tooltip.exists && blob_tooltip.value == "\"Blob tip\"" &&
            appended_tooltip.ok && !appended_tooltip.exists,
        "#800: serialized tooltip undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_tooltip.scx";
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
    expect(incomplete_create.ok, "#800: missing-ToolTipText fixture should be writable");

    tooltip_result = copperfin::vfp::set_visual_object_tooltip_text({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .tooltip_text = "Ignored"
    });
    expect(!tooltip_result.ok, "#800: tooltip assignment should reject objects without a writable ToolTipText carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_status_bar_text_assigns_text() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_statusbar_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "statusbar.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "STATUSBART", .type = 'C', .length = 70U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "oneButton", "one-guid", "One status"},
        {"cmdTwo", "twoButton", "two-guid", "Two status"},
        {"cmdOther", "otherButton", "other-guid", "Other status"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#801: status-bar fixture should be writable");

    const auto status_bar_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "StatusBarText"
        });
        expect(result.ok && result.exists, "#801: status-bar fixture property should be readable");
        return result.value;
    };
    const auto status_bar = [&](const std::string& unique_id) {
        return status_bar_for(table_path.string(), unique_id);
    };
    const auto status_bar_state = [&]() {
        return status_bar("one-guid") + "," +
            status_bar("two-guid") + "," +
            status_bar("other-guid");
    };

    auto status_result = copperfin::vfp::set_visual_object_status_bar_text({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cmdOne", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .status_bar_text = "Ready to save"
    });
    expect(status_result.ok, "#801: status-bar assignment should support object-name and record-index selectors");
    expect(status_result.affected_object_count == 2U,
        "#1002: successful status-bar assignment should report affected object count");
    expect(status_bar("one-guid") == "Ready to save" &&
            status_bar("two-guid") == "Ready to save" &&
            status_bar("other-guid") == "Other status",
        "#801: direct status-bar assignment should write raw text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#801: first status-bar write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#801: second status-bar write should remain undo-backed");
    expect(status_bar_state() == "One status,Two status,Other status",
        "#801: status-bar undo should restore original direct values");

    status_result = copperfin::vfp::set_visual_object_status_bar_text({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "two-guid"}
        },
        .status_bar_text = "Use \"Save\" to continue"
    });
    expect(status_result.ok, "#801: status-bar assignment should support UNIQUEID selectors");
    expect(status_bar("one-guid") == "Use \"Save\" to continue" &&
            status_bar("two-guid") == "Use \"Save\" to continue",
        "#801: direct status-bar assignment should store caller text without serialized quoting");

    const std::string committed_state = status_bar_state();
    status_result = copperfin::vfp::set_visual_object_status_bar_text({
        .path = table_path.string(),
        .objects = {},
        .status_bar_text = "Ignored"
    });
    expect(!status_result.ok, "#801: status-bar assignment should reject empty selections");
    expect(status_result.affected_object_count == 0U,
        "#1002: failed status-bar assignment should report zero affected objects");
    expect(status_bar_state() == committed_state, "#801: empty-selection failures should not mutate status-bar text");

    status_result = copperfin::vfp::set_visual_object_status_bar_text({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .status_bar_text = "Ignored"
    });
    expect(!status_result.ok, "#801: status-bar assignment should reject missing selected objects");
    expect(status_bar_state() == committed_state, "#801: missing-object failures should not mutate status-bar text");

    status_result = copperfin::vfp::set_visual_object_status_bar_text({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = "cmdOne", .unique_id = {}}
        },
        .status_bar_text = "Ignored"
    });
    expect(!status_result.ok, "#801: status-bar assignment should reject duplicate selected objects");
    expect(status_bar_state() == committed_state, "#801: duplicate-selection failures should not mutate status-bar text");

    const fs::path blob_path = temp_dir / "statusbar_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cmdBlob", "blob-guid", "StatusBarText = \"Blob status\"\r\nCaption = \"Blob\"\r\n"},
        {"cmdNoStatus", "no-status-guid", "Caption = \"No status\"\r\n"},
        {"cmdOther", "other-guid", "StatusBarText = \"Other status\"\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#801: status-bar property-blob fixture should be writable");

    const auto blob_status_bar_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "StatusBarText"
        });
    };

    status_result = copperfin::vfp::set_visual_object_status_bar_text({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cmdNoStatus", .unique_id = {}}
        },
        .status_bar_text = "Press \"Esc\" to cancel"
    });
    expect(status_result.ok, "#801: status-bar assignment should support existing and absent serialized properties");
    auto blob_status = blob_status_bar_state("blob-guid");
    auto appended_status = blob_status_bar_state("no-status-guid");
    auto other_status = blob_status_bar_state("other-guid");
    expect(blob_status.ok && blob_status.exists && blob_status.value == "\"Press \"\"Esc\"\" to cancel\"" &&
            appended_status.ok && appended_status.exists && appended_status.value == "\"Press \"\"Esc\"\" to cancel\"" &&
            other_status.ok && other_status.exists && other_status.value == "\"Other status\"",
        "#801: serialized status-bar assignment should quote text, append missing StatusBarText, and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#801: appended serialized status-bar write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#801: existing serialized status-bar write should remain undo-backed");
    blob_status = blob_status_bar_state("blob-guid");
    appended_status = blob_status_bar_state("no-status-guid");
    expect(blob_status.ok && blob_status.exists && blob_status.value == "\"Blob status\"" &&
            appended_status.ok && !appended_status.exists,
        "#801: serialized status-bar undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_statusbar.scx";
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
    expect(incomplete_create.ok, "#801: missing-StatusBarText fixture should be writable");

    status_result = copperfin::vfp::set_visual_object_status_bar_text({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .status_bar_text = "Ignored"
    });
    expect(!status_result.ok, "#801: status-bar assignment should reject objects without a writable StatusBarText carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_control_source_assigns_text() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_controlsource_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "controlsource.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "CONTROLSOU", .type = 'C', .length = 70U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtOne", "oneBox", "one-guid", "customers.name"},
        {"txtTwo", "twoBox", "two-guid", "customers.city"},
        {"txtOther", "otherBox", "other-guid", "customers.state"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#802: control-source fixture should be writable");

    const auto control_source_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "ControlSource"
        });
        expect(result.ok && result.exists, "#802: control-source fixture property should be readable");
        return result.value;
    };
    const auto control_source = [&](const std::string& unique_id) {
        return control_source_for(table_path.string(), unique_id);
    };
    const auto control_source_state = [&]() {
        return control_source("one-guid") + "," +
            control_source("two-guid") + "," +
            control_source("other-guid");
    };

    auto control_result = copperfin::vfp::set_visual_object_control_source({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "txtOne", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .control_source = "orders.total"
    });
    expect(control_result.ok, "#802: control-source assignment should support object-name and record-index selectors");
    expect(control_result.affected_object_count == 2U,
        "#1002: successful control-source assignment should report affected object count");
    expect(control_source("one-guid") == "orders.total" &&
            control_source("two-guid") == "orders.total" &&
            control_source("other-guid") == "customers.state",
        "#802: direct control-source assignment should write raw text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#802: first control-source write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#802: second control-source write should remain undo-backed");
    expect(control_source_state() == "customers.name,customers.city,customers.state",
        "#802: control-source undo should restore original direct values");

    control_result = copperfin::vfp::set_visual_object_control_source({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "two-guid"}
        },
        .control_source = "ThisForm.CurrentCustomer"
    });
    expect(control_result.ok, "#802: control-source assignment should support UNIQUEID selectors");
    expect(control_source("one-guid") == "ThisForm.CurrentCustomer" &&
            control_source("two-guid") == "ThisForm.CurrentCustomer",
        "#802: direct control-source assignment should store caller text without serialized quoting");

    const std::string committed_state = control_source_state();
    control_result = copperfin::vfp::set_visual_object_control_source({
        .path = table_path.string(),
        .objects = {},
        .control_source = "Ignored"
    });
    expect(!control_result.ok, "#802: control-source assignment should reject empty selections");
    expect(control_result.affected_object_count == 0U,
        "#1002: failed control-source assignment should report zero affected objects");
    expect(control_source_state() == committed_state, "#802: empty-selection failures should not mutate control sources");

    control_result = copperfin::vfp::set_visual_object_control_source({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .control_source = "Ignored"
    });
    expect(!control_result.ok, "#802: control-source assignment should reject missing selected objects");
    expect(control_source_state() == committed_state, "#802: missing-object failures should not mutate control sources");

    control_result = copperfin::vfp::set_visual_object_control_source({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = "txtOne", .unique_id = {}}
        },
        .control_source = "Ignored"
    });
    expect(!control_result.ok, "#802: control-source assignment should reject duplicate selected objects");
    expect(control_source_state() == committed_state, "#802: duplicate-selection failures should not mutate control sources");

    const fs::path blob_path = temp_dir / "controlsource_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", "ControlSource = \"customers.name\"\r\nCaption = \"Name\"\r\n"},
        {"txtNoSource", "no-source-guid", "Caption = \"No source\"\r\n"},
        {"txtOther", "other-guid", "ControlSource = \"customers.state\"\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#802: control-source property-blob fixture should be writable");

    const auto blob_control_source_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "ControlSource"
        });
    };

    control_result = copperfin::vfp::set_visual_object_control_source({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoSource", .unique_id = {}}
        },
        .control_source = "orders.\"total\""
    });
    expect(control_result.ok, "#802: control-source assignment should support existing and absent serialized properties");
    auto blob_source = blob_control_source_state("blob-guid");
    auto appended_source = blob_control_source_state("no-source-guid");
    auto other_source = blob_control_source_state("other-guid");
    expect(blob_source.ok && blob_source.exists && blob_source.value == "\"orders.\"\"total\"\"\"" &&
            appended_source.ok && appended_source.exists && appended_source.value == "\"orders.\"\"total\"\"\"" &&
            other_source.ok && other_source.exists && other_source.value == "\"customers.state\"",
        "#802: serialized control-source assignment should quote text, append missing ControlSource, and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#802: appended serialized control-source write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#802: existing serialized control-source write should remain undo-backed");
    blob_source = blob_control_source_state("blob-guid");
    appended_source = blob_control_source_state("no-source-guid");
    expect(blob_source.ok && blob_source.exists && blob_source.value == "\"customers.name\"" &&
            appended_source.ok && !appended_source.exists,
        "#802: serialized control-source undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_controlsource.scx";
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
    expect(incomplete_create.ok, "#802: missing-ControlSource fixture should be writable");

    control_result = copperfin::vfp::set_visual_object_control_source({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .control_source = "Ignored"
    });
    expect(!control_result.ok, "#802: control-source assignment should reject objects without a writable ControlSource carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_current_control_assigns_text() {
    test_visual_object_text_property_assigns_text(
        "#851",
        "currentcontrol",
        "CurrentControl",
        "CURRENTCONTROL",
        "current-control",
        "txtName",
        "txtCity",
        "txtState",
        "txtTotal",
        "txtCustomer",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            const std::string& value) {
            return copperfin::vfp::set_visual_object_current_control({
                .path = path,
                .objects = objects,
                .current_control = value
            });
        });
}

void test_set_visual_object_sparse_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#852",
        "sparse",
        "Sparse",
        "SPARSE",
        "sparse",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_sparse({
                .path = path,
                .objects = objects,
                .sparse = value
            });
        });
}

void test_set_visual_object_closable_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#882",
        "closable",
        "Closable",
        "CLOSABLE",
        "closable",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_closable({
                .path = path,
                .objects = objects,
                .closable = value
            });
        });
}

void test_set_visual_object_control_box_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#883",
        "control_box",
        "ControlBox",
        "CONTROLBOX",
        "control-box",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_control_box({
                .path = path,
                .objects = objects,
                .control_box = value
            });
        });
}

void test_set_visual_object_allow_output_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#915",
        "allow_output",
        "AllowOutput",
        "ALLOWOUTPUT",
        "allow-output",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_allow_output({
                .path = path,
                .objects = objects,
                .allow_output = value
            });
        });
}

void test_set_visual_object_auto_center_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#900",
        "auto_center",
        "AutoCenter",
        "AUTOCENTER",
        "auto-center",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_auto_center({
                .path = path,
                .objects = objects,
                .auto_center = value
            });
        });
}

void test_set_visual_object_auto_size_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#906",
        "auto_size",
        "AutoSize",
        "AUTOSIZE",
        "auto-size",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_auto_size({
                .path = path,
                .objects = objects,
                .auto_size = value
            });
        });
}

void test_set_visual_object_auto_release_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#910",
        "auto_release",
        "AutoRelease",
        "AUTORELEASE",
        "auto-release",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_auto_release({
                .path = path,
                .objects = objects,
                .auto_release = value
            });
        });
}

void test_set_visual_object_auto_verb_menu_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#911",
        "auto_verb_menu",
        "AutoVerbMenu",
        "AUTOVERBMENU",
        "auto-verb-menu",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_auto_verb_menu({
                .path = path,
                .objects = objects,
                .auto_verb_menu = value
            });
        });
}

void test_set_visual_object_bind_controls_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#912",
        "bind_controls",
        "BindControls",
        "BINDCONTROLS",
        "bind-controls",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_bind_controls({
                .path = path,
                .objects = objects,
                .bind_controls = value
            });
        });
}

void test_set_visual_object_clip_controls_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#907",
        "clip_controls",
        "ClipControls",
        "CLIPCONTROLS",
        "clip-controls",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_clip_controls({
                .path = path,
                .objects = objects,
                .clip_controls = value
            });
        });
}

void test_set_visual_object_dockable_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#908",
        "dockable",
        "Dockable",
        "DOCKABLE",
        "dockable",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_dockable({
                .path = path,
                .objects = objects,
                .dockable = value
            });
        });
}

void test_set_visual_object_continuous_scroll_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#909",
        "continuous_scroll",
        "ContinuousScroll",
        "CONTINUOUSSCROLL",
        "continuous-scroll",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_continuous_scroll({
                .path = path,
                .objects = objects,
                .continuous_scroll = value
            });
        });
}

void test_set_visual_object_desktop_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#901",
        "desktop",
        "Desktop",
        "DESKTOP",
        "desktop",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_desktop({
                .path = path,
                .objects = objects,
                .desktop = value
            });
        });
}

void test_set_visual_object_key_preview_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#902",
        "key_preview",
        "KeyPreview",
        "KEYPREVIEW",
        "key-preview",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_key_preview({
                .path = path,
                .objects = objects,
                .key_preview = value
            });
        });
}

void test_set_visual_object_mac_desktop_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#903",
        "mac_desktop",
        "MacDesktop",
        "MACDESKTOP",
        "Mac desktop",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_mac_desktop({
                .path = path,
                .objects = objects,
                .mac_desktop = value
            });
        });
}

void test_set_visual_object_max_button_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#889",
        "max_button",
        "MaxButton",
        "MAXBUTTON",
        "max-button",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_max_button({
                .path = path,
                .objects = objects,
                .max_button = value
            });
        });
}

void test_set_visual_object_max_height_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#950",
        "max_height",
        "MaxHeight",
        "MAXHEIGHT",
        "max-height",
        0,
        1,
        42,
        7,
        99,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_max_height({
                .path = path,
                .objects = objects,
                .max_height = value
            });
        });
}

void test_set_visual_object_max_width_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#951",
        "max_width",
        "MaxWidth",
        "MAXWIDTH",
        "max-width",
        0,
        1,
        42,
        7,
        99,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_max_width({
                .path = path,
                .objects = objects,
                .max_width = value
            });
        });
}

void test_set_visual_object_max_left_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#952",
        "max_left",
        "MaxLeft",
        "MAXLEFT",
        "max-left",
        0,
        1,
        42,
        7,
        99,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_max_left({
                .path = path,
                .objects = objects,
                .max_left = value
            });
        });
}

void test_set_visual_object_max_top_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#953",
        "max_top",
        "MaxTop",
        "MAXTOP",
        "max-top",
        0,
        1,
        42,
        7,
        99,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_max_top({
                .path = path,
                .objects = objects,
                .max_top = value
            });
        });
}

void test_set_visual_object_min_button_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#890",
        "min_button",
        "MinButton",
        "MINBUTTON",
        "min-button",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_min_button({
                .path = path,
                .objects = objects,
                .min_button = value
            });
        });
}

void test_set_visual_object_min_height_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#954",
        "min_height",
        "MinHeight",
        "MINHEIGHT",
        "min-height",
        0,
        1,
        42,
        7,
        99,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_min_height({
                .path = path,
                .objects = objects,
                .min_height = value
            });
        });
}

void test_set_visual_object_min_width_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#955",
        "min_width",
        "MinWidth",
        "MINWIDTH",
        "min-width",
        0,
        1,
        42,
        7,
        99,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_min_width({
                .path = path,
                .objects = objects,
                .min_width = value
            });
        });
}

void test_set_visual_object_movable_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#891",
        "movable",
        "Movable",
        "MOVABLE",
        "movable",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_movable({
                .path = path,
                .objects = objects,
                .movable = value
            });
        });
}

void test_set_visual_object_half_height_caption_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#895",
        "half_height_caption",
        "HalfHeightCaption",
        "HALFHEIGHTCAPTION",
        "half-height-caption",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_half_height_caption({
                .path = path,
                .objects = objects,
                .half_height_caption = value
            });
        });
}

void test_set_visual_object_mdi_form_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#896",
        "mdi_form",
        "MDIForm",
        "MDIFORM",
        "MDI form",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_mdi_form({
                .path = path,
                .objects = objects,
                .mdi_form = value
            });
        });
}

void test_set_visual_object_whats_this_button_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#897",
        "whats_this_button",
        "WhatsThisButton",
        "WHATSTHISBUTTON",
        "WhatsThis button",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_whats_this_button({
                .path = path,
                .objects = objects,
                .whats_this_button = value
            });
        });
}

void test_set_visual_object_whats_this_help_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#898",
        "whats_this_help",
        "WhatsThisHelp",
        "WHATSTHISHELP",
        "WhatsThis help",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_whats_this_help({
                .path = path,
                .objects = objects,
                .whats_this_help = value
            });
        });
}

void test_set_visual_object_whats_this_help_id_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#899",
        "whats_this_help_id",
        "WhatsThisHelpID",
        "WHATSTHISHELPID",
        "WhatsThis help ID",
        0,
        1,
        42,
        7,
        99,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_whats_this_help_id({
                .path = path,
                .objects = objects,
                .whats_this_help_id = value
            });
        });
}

void test_set_visual_object_help_context_id_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#941",
        "help_context_id",
        "HelpContextID",
        "HELPCONTEXTID",
        "help-context ID",
        0,
        1,
        42,
        7,
        99,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_help_context_id({
                .path = path,
                .objects = objects,
                .help_context_id = value
            });
        });
}

void test_set_visual_object_display_orientation_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#942",
        "display_orientation",
        "DisplayOrientation",
        "DISPLAYORIENTATION",
        "display orientation",
        0,
        1,
        42,
        7,
        99,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_display_orientation({
                .path = path,
                .objects = objects,
                .display_orientation = value
            });
        });
}

void test_set_visual_object_tab_orientation_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#943",
        "tab_orientation",
        "TabOrientation",
        "TABORIENTATION",
        "tab orientation",
        0,
        1,
        42,
        7,
        99,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_tab_orientation({
                .path = path,
                .objects = objects,
                .tab_orientation = value
            });
        });
}

void test_set_visual_object_list_item_id_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#947",
        "list_item_id",
        "ListItemID",
        "LISTITEMID",
        "list-item ID",
        0,
        1,
        42,
        7,
        99,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_list_item_id({
                .path = path,
                .objects = objects,
                .list_item_id = value
            });
        });
}

void test_set_visual_object_lock_screen_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#949",
        "lock_screen",
        "LockScreen",
        "LOCKSCREEN",
        "lock-screen",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_lock_screen({
                .path = path,
                .objects = objects,
                .lock_screen = value
            });
        });
}

void test_set_visual_object_hide_selection_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#877",
        "hide_selection",
        "HideSelection",
        "HIDESELECTION",
        "hide-selection",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_hide_selection({
                .path = path,
                .objects = objects,
                .hide_selection = value
            });
        });
}

void test_set_visual_object_allow_cell_selection_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#878",
        "allow_cell_selection",
        "AllowCellSelection",
        "ALLOWCELLSELECTION",
        "allow-cell-selection",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_allow_cell_selection({
                .path = path,
                .objects = objects,
                .allow_cell_selection = value
            });
        });
}

void test_set_visual_object_delete_mark_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#924",
        "delete_mark",
        "DeleteMark",
        "DELETEMARK",
        "delete-mark",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_delete_mark({
                .path = path,
                .objects = objects,
                .delete_mark = value
            });
        });
}

void test_set_visual_object_record_mark_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#925",
        "record_mark",
        "RecordMark",
        "RECORDMARK",
        "record-mark",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_record_mark({
                .path = path,
                .objects = objects,
                .record_mark = value
            });
        });
}

void test_set_visual_object_split_bar_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#926",
        "split_bar",
        "SplitBar",
        "SPLITBAR",
        "split-bar",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_split_bar({
                .path = path,
                .objects = objects,
                .split_bar = value
            });
        });
}

void test_set_visual_object_highlight_row_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#927",
        "highlight_row",
        "HighlightRow",
        "HIGHLIGHTROW",
        "highlight-row",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_highlight_row({
                .path = path,
                .objects = objects,
                .highlight_row = value
            });
        });
}

void test_set_visual_object_panel_link_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#932",
        "panel_link",
        "PanelLink",
        "PANELLINK",
        "panel-link",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_panel_link({
                .path = path,
                .objects = objects,
                .panel_link = value
            });
        });
}

void test_set_visual_object_allow_header_sizing_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#934",
        "allow_header_sizing",
        "AllowHeaderSizing",
        "ALLOWHEADERSIZING",
        "allow-header-sizing",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_allow_header_sizing({
                .path = path,
                .objects = objects,
                .allow_header_sizing = value
            });
        });
}

void test_set_visual_object_allow_row_sizing_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#935",
        "allow_row_sizing",
        "AllowRowSizing",
        "ALLOWROWSIZING",
        "allow-row-sizing",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_allow_row_sizing({
                .path = path,
                .objects = objects,
                .allow_row_sizing = value
            });
        });
}

void test_set_visual_object_resizable_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#940",
        "resizable",
        "Resizable",
        "RESIZABLE",
        "resizable",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_resizable({
                .path = path,
                .objects = objects,
                .resizable = value
            });
        });
}

void test_set_visual_object_add_line_feeds_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#853",
        "addlinefeeds",
        "AddLineFeeds",
        "ADDLINEFEEDS",
        "add-line-feeds",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_add_line_feeds({
                .path = path,
                .objects = objects,
                .add_line_feeds = value
            });
        });
}

void test_set_visual_object_always_on_top_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#854",
        "always_on_top",
        "AlwaysOnTop",
        "ALWAYSONTOP",
        "always-on-top",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_always_on_top({
                .path = path,
                .objects = objects,
                .always_on_top = value
            });
        });
}

void test_set_visual_object_always_on_bottom_assigns_logical_state() {
    test_visual_object_logical_property_assigns_state(
        "#855",
        "always_on_bottom",
        "AlwaysOnBottom",
        "ALWAYSONBOTTOM",
        "always-on-bottom",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            bool value) {
            return copperfin::vfp::set_visual_object_always_on_bottom({
                .path = path,
                .objects = objects,
                .always_on_bottom = value
            });
        });
}

void test_set_visual_object_style_assigns_numeric_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_style_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "style.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "STYLE", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", "0"},
        {"lstOrders", "ordersList", "orders-guid", "1"},
        {"cboOther", "otherCombo", "other-guid", "2"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#814: style fixture should be writable");

    const auto style_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "Style"
        });
        expect(result.ok && result.exists, "#814: style fixture property should be readable");
        return result.value;
    };
    const auto style = [&](const std::string& unique_id) {
        return style_for(table_path.string(), unique_id);
    };
    const auto style_state = [&]() {
        return style("customer-guid") + "," +
            style("orders-guid") + "," +
            style("other-guid");
    };

    auto style_result = copperfin::vfp::set_visual_object_style({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .style = 2
    });
    expect(style_result.ok, "#814: style assignment should support object-name and record-index selectors");
    expect(style("customer-guid") == "2" &&
            style("orders-guid") == "2" &&
            style("other-guid") == "2",
        "#814: direct style assignment should write raw numeric text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#814: first style write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#814: second style write should remain undo-backed");
    expect(style_state() == "0,1,2", "#814: style undo should restore original direct values");

    style_result = copperfin::vfp::set_visual_object_style({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .style = 1
    });
    expect(style_result.ok, "#814: style assignment should support UNIQUEID selectors");
    expect(style("customer-guid") == "1" &&
            style("orders-guid") == "1",
        "#814: direct style assignment should store unquoted numeric values");

    const std::string committed_state = style_state();
    style_result = copperfin::vfp::set_visual_object_style({
        .path = table_path.string(),
        .objects = {},
        .style = 1
    });
    expect(!style_result.ok, "#814: style assignment should reject empty selections");
    expect(style_state() == committed_state, "#814: empty-selection failures should not mutate styles");

    style_result = copperfin::vfp::set_visual_object_style({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"}
        },
        .style = -1
    });
    expect(!style_result.ok, "#814: style assignment should reject negative values");
    expect(style_state() == committed_state, "#814: negative-value failures should not mutate styles");

    style_result = copperfin::vfp::set_visual_object_style({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .style = 1
    });
    expect(!style_result.ok, "#814: style assignment should reject missing selected objects");
    expect(style_state() == committed_state, "#814: missing-object failures should not mutate styles");

    style_result = copperfin::vfp::set_visual_object_style({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .style = 1
    });
    expect(!style_result.ok, "#814: style assignment should reject duplicate selected objects");
    expect(style_state() == committed_state, "#814: duplicate-selection failures should not mutate styles");

    const fs::path blob_path = temp_dir / "style_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "Style = 0\r\nCaption = \"Customer\"\r\n"},
        {"cboNoStyle", "no-style-guid", "Caption = \"No style\"\r\n"},
        {"cboOther", "other-guid", "Style = 2\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#814: style property-blob fixture should be writable");

    const auto blob_style_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "Style"
        });
    };

    style_result = copperfin::vfp::set_visual_object_style({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoStyle", .unique_id = {}}
        },
        .style = 2
    });
    expect(style_result.ok, "#814: style assignment should support existing and absent serialized properties");
    auto blob_style = blob_style_state("blob-guid");
    auto appended_style = blob_style_state("no-style-guid");
    auto other_style = blob_style_state("other-guid");
    expect(blob_style.ok && blob_style.exists && blob_style.value == "2" &&
            appended_style.ok && appended_style.exists && appended_style.value == "2" &&
            other_style.ok && other_style.exists && other_style.value == "2",
        "#814: serialized style assignment should write unquoted numeric values and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#814: appended serialized style write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#814: existing serialized style write should remain undo-backed");
    blob_style = blob_style_state("blob-guid");
    appended_style = blob_style_state("no-style-guid");
    expect(blob_style.ok && blob_style.exists && blob_style.value == "0" &&
            appended_style.ok && !appended_style.exists,
        "#814: serialized style undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_style.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"cboA", "a-guid"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#814: missing-Style fixture should be writable");

    style_result = copperfin::vfp::set_visual_object_style({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .style = 1
    });
    expect(!style_result.ok, "#814: style assignment should reject objects without a writable Style carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_deleted_states_rolls_back_batch_failures() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_batch_delete_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "batch_delete.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid"},
        {"txtName", "nameBox", "name-guid"},
        {"lblStatus", "statusLabel", "status-guid"},
        {"dupControl", "dupOne", "dup-one-guid"},
        {"dupControl", "dupTwo", "dup-two-guid"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#753: batch deleted-state fixture should be writable");

    const auto is_deleted = [&](const std::string& unique_id) {
        const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
        expect(list_result.ok, "#753: batch deleted-state fixture should remain listable");
        const auto object = std::find_if(
            list_result.objects.begin(),
            list_result.objects.end(),
            [&](const copperfin::vfp::VisualObjectSnapshot& candidate) {
                return candidate.unique_id == unique_id;
            });
        expect(object != list_result.objects.end(), "#753: expected visual object should remain present");
        return object != list_result.objects.end() && object->deleted;
    };

    auto batch_result = copperfin::vfp::set_visual_object_deleted_states({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .deleted = true
            },
            {
                .record_index = 0U,
                .object_name = "txtName",
                .unique_id = {},
                .deleted = true
            }
        }
    });
    expect(batch_result.ok, "#753: batch deleted-state changes should support mixed selector modes");
    expect(batch_result.affected_object_count == 2U,
        "#1006: successful batch deleted-state change should report changed item count");
    expect(is_deleted("save-guid") && is_deleted("name-guid"),
        "#753: batch deleted-state changes should mark multiple selected objects deleted");
    expect(!is_deleted("status-guid"),
        "#753: batch deleted-state changes should preserve unrelated records");

    batch_result = copperfin::vfp::set_visual_object_deleted_states({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = "cmdSave",
                .unique_id = {},
                .deleted = false
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "name-guid",
                .deleted = false
            }
        }
    });
    expect(batch_result.ok, "#753: batch deleted-state changes should restore objects through the same surface");
    expect(!is_deleted("save-guid") && !is_deleted("name-guid"),
        "#753: batch deleted-state restore should clear deleted flags");

    batch_result = copperfin::vfp::set_visual_object_deleted_states({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .deleted = true
            },
            {
                .record_index = 0U,
                .object_name = "dupControl",
                .unique_id = {},
                .deleted = true
            }
        }
    });
    expect(!batch_result.ok,
        "#753: batch deleted-state changes should reject ambiguous later selections");
    expect(batch_result.affected_object_count == 0U,
        "#1006: failed batch deleted-state change should report zero affected objects");
    expect(!is_deleted("status-guid") && !is_deleted("dup-one-guid") && !is_deleted("dup-two-guid"),
        "#753: failed batch deleted-state changes should roll back earlier flag mutations");

    batch_result = copperfin::vfp::set_visual_object_deleted_states({
        .path = table_path.string(),
        .objects = {}
    });
    expect(!batch_result.ok, "#753: empty batch deleted-state requests should fail explicitly");
    expect(batch_result.affected_object_count == 0U,
        "#1006: empty batch deleted-state change should report zero affected objects");
    expect(!is_deleted("save-guid") && !is_deleted("name-guid") && !is_deleted("status-guid"),
        "#753: empty batch deleted-state requests should not mutate existing flags");

    fs::remove_all(temp_dir, ignored);
}

}  // namespace cf_test_visual_asset_editor
