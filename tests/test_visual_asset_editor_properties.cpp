// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
void test_update_visual_object_property_preserves_equals_for_blank_property_values() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_blank_equals_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "blank_equals.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtBox", "blank-guid", "Caption = \"Hello\"\r\nFormat = \r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "blank-value property fixture should be writable");

    const auto update_result = copperfin::vfp::update_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "blank-guid",
        .property_name = "Caption",
        .property_value = "\"World\""
    });
    expect(update_result.ok, "updating an unrelated property should succeed");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
    expect(parse_result.ok && parse_result.table.records.size() == 1U,
        "updated blank-equals fixture should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 1U) {
        const auto* properties = find_record_field(parse_result.table.records[0], "PROPERTIES");
        expect(properties != nullptr, "updated record should retain the PROPERTIES field");
        if (properties != nullptr) {
            expect(properties->display_value.find("Format =") != std::string::npos,
                "re-serializing the PROPERTIES blob should not drop the '=' for properties with a blank value "
                "(matching the sibling report-settings serializer, which always preserves it)");
        }
    }

    fs::remove_all(temp_dir, ignored);
}

void test_update_visual_object_report_settings_property_preserves_comment_lines() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_settings_comment_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path report_path = temp_dir / "settings_comment.frx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "* Header = comment text\r\nORIENTATION=1", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "report-settings comment-line fixture should be writable");

    const auto update_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "ORIENTATION",
        .property_value = "2"
    });
    expect(update_result.ok, "updating the real ORIENTATION setting should succeed");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(report_path.string(), 1U);
    expect(parse_result.ok && parse_result.table.records.size() == 1U,
        "updated report-settings comment-line fixture should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 1U) {
        const auto* expr = find_record_field(parse_result.table.records[0], "EXPR");
        expect(expr != nullptr, "updated record should retain the EXPR field");
        if (expr != nullptr) {
            expect(expr->display_value.find("* Header = comment text") != std::string::npos,
                "a settings comment line containing '=' with non-empty trailing text must survive re-serialization "
                "verbatim, not be misclassified as a real property assignment (its name starts with '*', the VFP "
                "comment marker) and reformatted as \"* Header=comment text\"");
            expect(expr->display_value.find("ORIENTATION=2") != std::string::npos,
                "the real ORIENTATION assignment should be updated to the new value");
        }
    }

    fs::remove_all(temp_dir, ignored);
}

void test_report_settings_bottom_margin_memo_round_trips() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_botmargin_memo_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto run_round_trip = [&](const fs::path& asset_path, const fs::path& memo_path) {
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "OBJTYPE", .type = 'N', .length = 8U},
            {.name = "OBJCODE", .type = 'N', .length = 8U},
            {.name = "EXPR", .type = 'M', .length = 4U},
            {.name = "UNIQUEID", .type = 'C', .length = 40U}
        };
        const std::vector<std::vector<std::string>> records{
            {"1", "53", "* retain this comment\r\nCUSTOMSETTING=preserve\r\nBOTMARGIN=20\r\nGRIDV=4", "settings-guid"}
        };
        const auto create_result = copperfin::vfp::create_dbf_table_file(asset_path.string(), fields, records);
        expect(create_result.ok, "#3920: memo-backed BOTMARGIN fixture should be writable");
        expect(fs::exists(memo_path), "#3920: report/label fixture should create its memo sidecar");

        auto query = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "BOTMARGIN"
        });
        expect(query.ok && query.exists && !query.direct_field && query.value == "20",
               "#3920: EXPR-only BOTMARGIN should be readable before mutation");

        const auto clear_result = copperfin::vfp::clear_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "BOTMARGIN"
        });
        expect(clear_result.ok, "#3920: EXPR-only BOTMARGIN should clear successfully");

        query = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "BOTMARGIN"
        });
        expect(query.ok && !query.exists && !query.direct_field,
               "#3920: cleared BOTMARGIN should remain a writable known memo setting after reopen");

        auto parsed = copperfin::vfp::parse_dbf_table_from_file(asset_path.string(), 1U);
        expect(parsed.ok && parsed.table.records.size() == 1U,
               "#3920: cleared report/label settings should reopen");
        if (parsed.ok && parsed.table.records.size() == 1U) {
            const auto* expr = find_record_field(parsed.table.records[0], "EXPR");
            expect(expr != nullptr, "#3920: cleared settings should retain EXPR");
            if (expr != nullptr) {
                expect(expr->display_value.find("BOTMARGIN") == std::string::npos,
                       "#3920: clear should remove only the BOTMARGIN assignment");
                expect(expr->display_value.find("* retain this comment") != std::string::npos &&
                           expr->display_value.find("CUSTOMSETTING=preserve") != std::string::npos &&
                           expr->display_value.find("GRIDV=4") != std::string::npos,
                       "#3920: clear should preserve comments, unsupported settings, and sibling settings");
            }
        }

        const auto cleared_table_bytes = read_file_bytes(asset_path);
        const auto cleared_memo_bytes = read_file_bytes(memo_path);
        const auto repeat_clear = copperfin::vfp::clear_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "BOTMARGIN"
        });
        expect(repeat_clear.ok, "#3920: clearing an absent known BOTMARGIN should be a no-op");
        expect(read_file_bytes(asset_path) == cleared_table_bytes && read_file_bytes(memo_path) == cleared_memo_bytes,
               "#3920: repeated clear should not rewrite primary or memo bytes");

        const auto update_result = copperfin::vfp::update_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "BOTMARGIN",
            .property_value = "35"
        });
        expect(update_result.ok, "#3920: cleared BOTMARGIN should re-materialize in EXPR");
        query = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "BOTMARGIN"
        });
        expect(query.ok && query.exists && !query.direct_field && query.value == "35",
               "#3920: re-materialized BOTMARGIN should survive reopen");

        parsed = copperfin::vfp::parse_dbf_table_from_file(asset_path.string(), 1U);
        if (parsed.ok && parsed.table.records.size() == 1U) {
            const auto* expr = find_record_field(parsed.table.records[0], "EXPR");
            expect(expr != nullptr &&
                       expr->display_value.find("BOTMARGIN=35") != std::string::npos &&
                       expr->display_value.find("CUSTOMSETTING=preserve") != std::string::npos,
                   "#3920: re-set should preserve unsupported memo content");
        }

        const auto updated_table_bytes = read_file_bytes(asset_path);
        const auto updated_memo_bytes = read_file_bytes(memo_path);
        const auto repeat_update = copperfin::vfp::update_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = "BOTMARGIN",
            .property_value = "35"
        });
        expect(repeat_update.ok, "#3920: assigning the current BOTMARGIN should be a no-op");
        expect(read_file_bytes(asset_path) == updated_table_bytes && read_file_bytes(memo_path) == updated_memo_bytes,
               "#3920: repeated update should not rewrite primary or memo bytes");
    };

    run_round_trip(temp_dir / "botmargin.frx", temp_dir / "botmargin.frt");
    run_round_trip(temp_dir / "botmargin.lbx", temp_dir / "botmargin.lbt");

    fs::remove_all(temp_dir, ignored);
}

void test_report_settings_fallback_root_gridv_round_trips() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_fallback_root_settings_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto run_round_trip = [&](const fs::path& asset_path) {
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "OBJTYPE", .type = 'N', .length = 8U},
            {.name = "EXPR", .type = 'M', .length = 4U},
            {.name = "UNIQUEID", .type = 'C', .length = 40U}
        };
        const auto create_result = copperfin::vfp::create_dbf_table_file(
            asset_path.string(), fields, {{"1", "GRIDV=1\r\nGRIDH=0", "fallback-settings-guid"}});
        expect(create_result.ok, "#4059: fallback settings fixture should be writable");

        const auto update_result = copperfin::vfp::update_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "fallback-settings-guid",
            .property_name = "GRIDV",
            .property_value = "7"
        });
        expect(update_result.ok,
               "#4059: exposed fallback-root GRIDV should be writable");

        const auto query = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "fallback-settings-guid",
            .property_name = "GRIDV"
        });
        expect(query.ok && query.exists && !query.direct_field && query.value == "7",
               "#4059: fallback-root GRIDV should round-trip through the memo settings path");
    };

    run_round_trip(temp_dir / "fallback_settings.frx");
    run_round_trip(temp_dir / "fallback_settings.lbx");
    fs::remove_all(temp_dir, ignored);
}

void test_report_settings_topmargin_and_tag_memo_round_trips() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_topmargin_tag_settings_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto query = [](const fs::path& asset_path, const std::string& property_name) {
        return copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "settings-guid",
            .property_name = property_name
        });
    };
    const auto run_round_trip = [&](const fs::path& asset_path) {
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "OBJTYPE", .type = 'N', .length = 8U},
            {.name = "OBJCODE", .type = 'N', .length = 8U},
            {.name = "EXPR", .type = 'M', .length = 4U},
            {.name = "UNIQUEID", .type = 'C', .length = 40U}
        };
        const auto create_result = copperfin::vfp::create_dbf_table_file(
            asset_path.string(), fields, {{"1", "53", "CUSTOMSETTING=preserve\r\nTOPMARGIN=10\r\nGRIDV=4", "settings-guid"}});
        expect(create_result.ok, "#4060: memo-backed settings fixture should be writable");

        auto topmargin = query(asset_path, "TOPMARGIN");
        auto tag = query(asset_path, "TAG");
        expect(topmargin.ok && topmargin.exists && topmargin.value == "10",
               "#4060: existing TOPMARGIN should be readable through the memo settings path");
        expect(tag.ok && !tag.exists && !tag.direct_field,
               "#4060: absent TAG should remain an admitted memo setting");

        const auto clear_topmargin = copperfin::vfp::clear_visual_object_property({
            .path = asset_path.string(), .record_index = 0U, .object_name = {}, .unique_id = "settings-guid", .property_name = "TOPMARGIN"});
        expect(clear_topmargin.ok, "#4060: existing TOPMARGIN should clear successfully");
        topmargin = query(asset_path, "TOPMARGIN");
        expect(topmargin.ok && !topmargin.exists,
               "#4060: cleared TOPMARGIN should stay writable after reopen");

        const auto restore_topmargin = copperfin::vfp::update_visual_object_property({
            .path = asset_path.string(), .record_index = 0U, .object_name = {}, .unique_id = "settings-guid", .property_name = "TOPMARGIN", .property_value = "20"});
        const auto create_tag = copperfin::vfp::update_visual_object_property({
            .path = asset_path.string(), .record_index = 0U, .object_name = {}, .unique_id = "settings-guid", .property_name = "TAG", .property_value = "CustomerId"});
        expect(restore_topmargin.ok && create_tag.ok,
               "#4060: TOPMARGIN and absent TAG should be materialized in EXPR");

        const auto clear_tag = copperfin::vfp::clear_visual_object_property({
            .path = asset_path.string(), .record_index = 0U, .object_name = {}, .unique_id = "settings-guid", .property_name = "TAG"});
        const auto restore_tag = copperfin::vfp::update_visual_object_property({
            .path = asset_path.string(), .record_index = 0U, .object_name = {}, .unique_id = "settings-guid", .property_name = "TAG", .property_value = "ClientId"});
        expect(clear_tag.ok && restore_tag.ok,
               "#4060: TAG should clear and re-add through the memo settings path");

        topmargin = query(asset_path, "TOPMARGIN");
        tag = query(asset_path, "TAG");
        expect(topmargin.ok && topmargin.exists && topmargin.value == "20" &&
                   tag.ok && tag.exists && tag.value == "ClientId",
               "#4060: restored TOPMARGIN and TAG should survive reopen");
        const auto parsed = copperfin::vfp::parse_dbf_table_from_file(asset_path.string(), 1U);
        if (parsed.ok && parsed.table.records.size() == 1U) {
            const auto* expr = find_record_field(parsed.table.records[0], "EXPR");
            expect(expr != nullptr && expr->display_value.find("CUSTOMSETTING=preserve") != std::string::npos &&
                       expr->display_value.find("GRIDV=4") != std::string::npos,
                   "#4060: TOPMARGIN/TAG edits must preserve unrelated EXPR assignments");
        }
    };

    run_round_trip(temp_dir / "topmargin_tag.frx");
    run_round_trip(temp_dir / "topmargin_tag.lbx");
    fs::remove_all(temp_dir, ignored);
}

void test_fractional_report_section_moves_follow_layout_membership() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_fractional_section_move_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto run_move = [&](const fs::path& asset_path) {
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "OBJTYPE", .type = 'N', .length = 8U},
            {.name = "VPOS", .type = 'N', .length = 12U, .decimal_count = 2U},
            {.name = "HEIGHT", .type = 'N', .length = 12U, .decimal_count = 2U},
            {.name = "UNIQUEID", .type = 'C', .length = 40U}
        };
        const auto create_result = copperfin::vfp::create_dbf_table_file(
            asset_path.string(), fields, {
                {"9", "0.60", "10.60", "first-section"},
                {"9", "10.60", "10.60", "second-section"},
                {"5", "10.40", "1.00", "boundary-object"}
            });
        expect(create_result.ok, "#4061: fractional section fixture should be writable");

        const auto move_result = copperfin::vfp::update_visual_object_property({
            .path = asset_path.string(),
            .record_index = 1U,
            .object_name = {},
            .unique_id = "second-section",
            .property_name = "VPOS",
            .property_value = "20.60"
        });
        expect(move_result.ok,
               "#4061: moving the displayed containing section should include its fractional-boundary object");
        const auto object_top = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 2U,
            .object_name = {},
            .unique_id = "boundary-object",
            .property_name = "VPOS"
        });
        expect(object_top.ok && object_top.exists && object_top.value == "20.4",
               "#4061: fractional section move should preserve the object's layout membership and relative top after reopen");
    };

    run_move(temp_dir / "fractional_section_move.frx");
    run_move(temp_dir / "fractional_section_move.lbx");
    fs::remove_all(temp_dir, ignored);
}

void test_report_label_character_field_writes_preserve_leading_spaces() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_character_spacing_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto run_round_trip = [&](const fs::path& asset_path) {
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "OBJTYPE", .type = 'N', .length = 8U},
            {.name = "UNIQUEID", .type = 'C', .length = 40U},
            {.name = "TAG", .type = 'C', .length = 8U}
        };
        const auto create_result = copperfin::vfp::create_dbf_table_file(
            asset_path.string(), fields, {{"5", "spacing-guid", " PRIOR "}});
        expect(create_result.ok, "#4062: report/label character-spacing fixture should be writable");

        const auto query_tag = [&]() {
            return copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 0U,
                .object_name = {},
                .unique_id = "spacing-guid",
                .property_name = "TAG"
            });
        };

        auto tag = query_tag();
        expect(tag.ok && tag.exists && tag.value == " PRIOR",
            "#4062: parsed FRX/LBX character fields should retain significant leading spaces");

        auto update_result = copperfin::vfp::update_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "spacing-guid",
            .property_name = "TAG",
            .property_value = " ALPHA  "
        });
        expect(update_result.ok, "#4062: direct report/label character-field updates should succeed");
        tag = query_tag();
        expect(tag.ok && tag.exists && tag.value == " ALPHA",
            "#4062: direct report/label character-field edits should preserve leading spaces and remove storage padding");

        const auto undo_result = copperfin::vfp::undo_visual_object_property(asset_path.string());
        expect(undo_result.ok, "#4062: undo should restore direct report/label character fields");
        tag = query_tag();
        expect(tag.ok && tag.exists && tag.value == " PRIOR",
            "#4062: undo snapshots should restore significant leading spaces after reopening");

        update_result = copperfin::vfp::update_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "spacing-guid",
            .property_name = "TAG",
            .property_value = " BETA  "
        });
        expect(update_result.ok, "#4062: subsequent direct report/label character-field edits should succeed");
        tag = query_tag();
        expect(tag.ok && tag.exists && tag.value == " BETA",
            "#4062: subsequent direct character-field edits should preserve leading spaces after undo");

        const auto overflow_result = copperfin::vfp::update_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "spacing-guid",
            .property_name = "TAG",
            .property_value = " TOO-LONG"
        });
        expect(!overflow_result.ok,
            "#4062: fixed-width report/label character fields should reject significant overflow");
        tag = query_tag();
        expect(tag.ok && tag.exists && tag.value == " BETA",
            "#4062: rejected character-field overflow should not truncate or mutate the stored value");
    };

    run_round_trip(temp_dir / "character_spacing.frx");
    run_round_trip(temp_dir / "character_spacing.lbx");
    fs::remove_all(temp_dir, ignored);
}

void test_query_visual_object_property_reads_selected_values() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_query_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path memo_table_path = temp_dir / "query.scx";
    const fs::path memo_path = temp_dir / "query.sct";
    write_synthetic_named_object_asset(memo_table_path, memo_path, {
        {
            .objname = "cmdSave",
            .name = "saveButton",
            .unique_id = "save-guid",
            .properties = "Caption = \"Save\"\r\nLeft = 10\r\n"
        },
        {
            .objname = "txtName",
            .name = "nameBox",
            .unique_id = "target-guid",
            .properties = "Caption = \"Name\"\r\nLeft = 30\r\n"
        }
    });

    auto query_result = copperfin::vfp::query_visual_object_property({
        .path = memo_table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = " TARGET-GUID ",
        .property_name = "caption"
    });
    expect(query_result.ok, "#736: visual property queries should support UNIQUEID selectors");
    expect(query_result.exists, "#736: visual property queries should report existing memo-backed properties");
    expect(!query_result.direct_field, "#736: visual property queries should identify memo-backed properties");
    expect(query_result.record_index == 1U, "#739: UNIQUEID property queries should report the resolved record index");
    expect(query_result.property_name == "Caption", "#736: visual property queries should return the stored memo property name");
    expect(query_result.value == "\"Name\"", "#736: visual property queries should return the selected memo property value");
    expect(!copperfin::vfp::query_visual_object_undo(memo_table_path.string()).available,
        "#736: visual property queries should not create undo history");

    query_result = copperfin::vfp::query_visual_object_property({
        .path = memo_table_path.string(),
        .record_index = 0U,
        .object_name = "cmdSave",
        .unique_id = {},
        .property_name = "MissingProp"
    });
    expect(query_result.ok, "#736: missing memo-backed property queries should report cleanly");
    expect(!query_result.exists, "#736: missing memo-backed property queries should not be marked existing");
    expect(!query_result.direct_field, "#736: missing memo-backed property queries should not be direct fields");
    expect(query_result.property_name == "MissingProp", "#736: missing property queries should echo the requested property name");
    expect(query_result.value.empty(), "#736: missing property queries should return an empty value");

    const fs::path direct_table_path = temp_dir / "query_geometry.scx";
    const fs::path direct_memo_path = temp_dir / "query_geometry.sct";
    write_synthetic_named_geometry_asset(direct_table_path, direct_memo_path);
    query_result = copperfin::vfp::query_visual_object_property({
        .path = direct_table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .property_name = "hpos"
    });
    expect(query_result.ok, "#736: visual property queries should support object-name selectors");
    expect(query_result.exists, "#736: visual property queries should report existing direct fields");
    expect(query_result.direct_field, "#736: visual property queries should identify direct fields");
    expect(query_result.record_index == 1U, "#739: object-name property queries should report the resolved record index");
    expect(query_result.property_name == "HPOS", "#736: visual property queries should return the stored direct field name");
    expect(std::abs(parse_number(query_result.value) - 222.0) < 0.001,
        "#736: visual property queries should return the selected direct-field value");
    expect(!copperfin::vfp::query_visual_object_undo(direct_table_path.string()).available,
        "#736: direct-field queries should not create undo history");

    fs::remove_all(temp_dir, ignored);
}

void test_clear_visual_object_property_resets_selected_values() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_clear_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "clear.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "111", "Caption = \"Save\"\r\nLeft = 10\r\n"},
        {"txtName", "nameBox", "name-guid", "222", "Caption = \"Name\"\r\nLeft = 30\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#766: property-clear fixture should be writable");

    auto clear_result = copperfin::vfp::clear_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .property_name = "MissingProp"
    });
    expect(clear_result.ok, "#766: clearing missing memo-backed properties should succeed as a no-op");
    expect(clear_result.affected_object_count == 1U,
        "#1005: successful property clear should report one affected object");
    expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
        "#766: missing memo-backed property clears should not create undo history");

    clear_result = copperfin::vfp::clear_visual_object_property({
        .path = table_path.string(),
        .record_index = 1U,
        .object_name = {},
        .unique_id = {},
        .property_name = "hpos"
    });
    expect(clear_result.ok, "#766: property clear should support record-index direct-field selection");

    clear_result = copperfin::vfp::clear_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "caption"
    });
    expect(clear_result.ok, "#766: property clear should support UNIQUEID memo-backed selection");

    auto hpos_query = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .property_name = "HPOS"
    });
    expect(hpos_query.ok && hpos_query.exists && hpos_query.direct_field && hpos_query.value.empty(),
        "#766: direct-field clears should write an empty value through the direct field path");

    auto caption_query = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption"
    });
    expect(caption_query.ok && !caption_query.exists,
        "#766: memo-backed clears should remove the assignment instead of leaving an empty value");

    auto left_query = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Left"
    });
    auto other_caption_query = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .property_name = "Caption"
    });
    expect(left_query.ok && left_query.exists && left_query.value == "10" &&
            other_caption_query.ok && other_caption_query.exists && other_caption_query.value == "\"Name\"",
        "#766: property clear should preserve unrelated memo assignments and unrelated objects");

    clear_result = copperfin::vfp::clear_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = " "
    });
    expect(!clear_result.ok, "#766: property clear should reject empty property names");
    expect(clear_result.affected_object_count == 0U,
        "#1005: failed property clear should report zero affected objects");

    clear_result = copperfin::vfp::clear_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "missing-guid",
        .property_name = "Caption"
    });
    expect(!clear_result.ok, "#766: property clear should reject missing selected objects");

    const fs::path no_properties_path = temp_dir / "clear_no_properties.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> no_properties_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> no_properties_records{
        {"cmdNoProps", "no-props-guid"}
    };
    const auto no_properties_create = copperfin::vfp::create_dbf_table_file(
        no_properties_path.string(),
        no_properties_fields,
        no_properties_records);
    expect(no_properties_create.ok, "#766: missing-PROPERTIES fixture should be writable");
    clear_result = copperfin::vfp::clear_visual_object_property({
        .path = no_properties_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "no-props-guid",
        .property_name = "Caption"
    });
    expect(!clear_result.ok, "#766: property clear should reject missing PROPERTIES fields for memo-backed clears");

    const fs::path unsupported_path = temp_dir / "clear_unsupported.dbf";
    const auto unsupported_create = copperfin::vfp::create_dbf_table_file(
        unsupported_path.string(),
        no_properties_fields,
        no_properties_records);
    expect(unsupported_create.ok, "#766: unsupported asset fixture should be writable");
    clear_result = copperfin::vfp::clear_visual_object_property({
        .path = unsupported_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "no-props-guid",
        .property_name = "Caption"
    });
    expect(!clear_result.ok, "#766: property clear should reject unsupported asset paths for memo-backed clears");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#766: undo should restore cleared memo-backed assignments");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#766: undo should restore cleared direct fields");

    caption_query = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption"
    });
    hpos_query = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .property_name = "HPOS"
    });
    expect(caption_query.ok && caption_query.exists && caption_query.value == "\"Save\"" &&
            hpos_query.ok && hpos_query.exists && hpos_query.value == "222",
        "#766: undo should restore direct and memo-backed cleared property values");

    fs::remove_all(temp_dir, ignored);
}

void test_clear_visual_object_properties_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_clear_batch_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "clear_batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "111", "211", "Caption = \"Save\"\r\nLeft = 10\r\n"},
        {"txtName", "nameBox", "name-guid", "222", "322", "Caption = \"Name\"\r\nLeft = 30\r\n"},
        {"lblStatus", "statusLabel", "status-guid", "333", "433", "Caption = \"Status\"\r\nLeft = 50\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#771: property-clear-batch fixture should be writable");

    const auto property_state = [&](const std::string& unique_id, const std::string& property_name) {
        return copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
    };

    auto batch_result = copperfin::vfp::clear_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .property_name = "hpos"
            },
            {
                .record_index = 0U,
                .object_name = "txtName",
                .unique_id = {},
                .property_name = "Caption"
            },
            {
                .record_index = 2U,
                .object_name = {},
                .unique_id = {},
                .property_name = "MissingMemo"
            }
        }
    });
    expect(batch_result.ok, "#771: batch property clears should support mixed selectors and missing memo no-ops");
    expect(batch_result.affected_object_count == 3U,
        "#1005: successful batch property clear should report affected item count");

    auto save_hpos = property_state("save-guid", "HPOS");
    auto name_caption = property_state("name-guid", "Caption");
    auto status_left = property_state("status-guid", "Left");
    expect(save_hpos.ok && save_hpos.exists && save_hpos.direct_field && save_hpos.value.empty() &&
            name_caption.ok && !name_caption.exists &&
            status_left.ok && status_left.exists && status_left.value == "50",
        "#771: batch clears should clear direct fields, remove memo assignments, and preserve unrelated assignments");

    const auto undo_before_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_before_failure.available,
        "#771: successful batch clears should leave normal visual undo history available");

    batch_result = copperfin::vfp::clear_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = "VPOS"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "missing-guid",
                .property_name = "Caption"
            }
        }
    });
    expect(!batch_result.ok, "#771: batch property clears should fail when a later selection is missing");
    expect(batch_result.affected_object_count == 0U,
        "#1005: failed batch property clear should report zero affected objects");
    auto status_vpos = property_state("status-guid", "VPOS");
    expect(status_vpos.ok && status_vpos.exists && status_vpos.value == "433",
        "#771: failed batch clears should roll back earlier direct-field clears");
    const auto undo_after_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_after_failure.available == undo_before_failure.available &&
            undo_after_failure.label == undo_before_failure.label,
        "#771: failed batch rollback should clean up undo entries created by the failed batch");

    batch_result = copperfin::vfp::clear_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = "Left"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = " "
            }
        }
    });
    expect(!batch_result.ok, "#771: batch property clears should reject empty property names");
    status_left = property_state("status-guid", "Left");
    expect(status_left.ok && status_left.exists && status_left.value == "50",
        "#771: empty-name batch failures should roll back earlier memo clears");

    batch_result = copperfin::vfp::clear_visual_object_properties({
        .path = table_path.string(),
        .properties = {}
    });
    expect(!batch_result.ok, "#771: empty batch clear requests should fail explicitly");
    expect(batch_result.affected_object_count == 0U,
        "#1005: empty batch property clear should report zero affected objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#771: undo should restore memo clears from successful batches");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#771: undo should restore direct-field clears from successful batches");

    save_hpos = property_state("save-guid", "HPOS");
    name_caption = property_state("name-guid", "Caption");
    expect(save_hpos.ok && save_hpos.exists && save_hpos.value == "111" &&
            name_caption.ok && name_caption.exists && name_caption.value == "\"Name\"",
        "#771: successful batch clear undo should restore original direct and memo-backed values");

    fs::remove_all(temp_dir, ignored);
}

void test_copy_visual_object_property_between_selected_objects() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_property_copy_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "property_copy.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "111", "Caption = \"Save\"\r\nLeft = 10\r\n"},
        {"txtName", "nameBox", "name-guid", "222", "Caption = \"Name\"\r\nTop = 30\r\n"},
        {"lblOther", "otherLabel", "other-guid", "333", "Caption = \"Other\"\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#767: property-copy fixture should be writable");

    auto copy_result = copperfin::vfp::copy_visual_object_property({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "save-guid",
        .source_property_name = "hpos",
        .target_record_index = 0U,
        .target_object_name = "txtName",
        .target_unique_id = {},
        .target_property_name = {},
        .replace_existing = true
    });
    expect(copy_result.ok, "#767: property copy should support UNIQUEID source, object-name target, and direct-field replacement");
    expect(copy_result.affected_object_count == 1U,
        "#1005: successful property copy should report one affected object");

    copy_result = copperfin::vfp::copy_visual_object_property({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = "cmdSave",
        .source_unique_id = {},
        .source_property_name = "caption",
        .target_record_index = 1U,
        .target_object_name = {},
        .target_unique_id = {},
        .target_property_name = "CopiedCaption",
        .replace_existing = false
    });
    expect(copy_result.ok, "#767: property copy should support object-name source, record-index target, and target renames");

    auto target_hpos = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .property_name = "HPOS"
    });
    auto target_copied_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "CopiedCaption"
    });
    expect(target_hpos.ok && target_hpos.exists && target_hpos.value == "111" &&
            target_copied_caption.ok && target_copied_caption.exists && target_copied_caption.value == "\"Save\"",
        "#767: property copy should persist direct-field and memo-backed target values");

    auto source_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption"
    });
    auto target_top = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "Top"
    });
    auto other_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "other-guid",
        .property_name = "Caption"
    });
    expect(source_caption.ok && source_caption.exists && source_caption.value == "\"Save\"" &&
            target_top.ok && target_top.exists && target_top.value == "30" &&
            other_caption.ok && other_caption.exists && other_caption.value == "\"Other\"",
        "#767: property copy should preserve source values, unrelated target assignments, and unrelated objects");

    copy_result = copperfin::vfp::copy_visual_object_property({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "save-guid",
        .source_property_name = "Caption",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "name-guid",
        .target_property_name = "Caption",
        .replace_existing = false
    });
    expect(!copy_result.ok, "#767: property copy should reject target collisions without replacement");
    expect(copy_result.affected_object_count == 0U,
        "#1005: failed property copy should report zero affected objects");

    copy_result = copperfin::vfp::copy_visual_object_property({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "save-guid",
        .source_property_name = "MissingProp",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "name-guid",
        .target_property_name = "MissingCopy",
        .replace_existing = false
    });
    expect(!copy_result.ok, "#767: property copy should reject missing source properties");

    copy_result = copperfin::vfp::copy_visual_object_property({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "save-guid",
        .source_property_name = " ",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "name-guid",
        .target_property_name = "EmptySource",
        .replace_existing = false
    });
    expect(!copy_result.ok, "#767: property copy should reject empty source property names");

    copy_result = copperfin::vfp::copy_visual_object_property({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "save-guid",
        .source_property_name = "Caption",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "name-guid",
        .target_property_name = " ",
        .replace_existing = false
    });
    expect(!copy_result.ok, "#767: property copy should reject empty requested target property names");

    const fs::path unsupported_path = temp_dir / "property_copy_unsupported.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> unsupported_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> unsupported_records{
        {"cmdSource", "source-guid", "111"},
        {"txtTarget", "target-guid", "222"}
    };
    const auto unsupported_create = copperfin::vfp::create_dbf_table_file(
        unsupported_path.string(),
        unsupported_fields,
        unsupported_records);
    expect(unsupported_create.ok, "#767: unsupported property-copy fixture should be writable");
    copy_result = copperfin::vfp::copy_visual_object_property({
        .path = unsupported_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_property_name = "HPOS",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_property_name = "Caption",
        .replace_existing = false
    });
    expect(!copy_result.ok, "#767: property copy should reject unsupported target property paths");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#767: undo should restore copied memo-backed properties");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#767: undo should restore copied direct fields");

    target_hpos = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .property_name = "HPOS"
    });
    target_copied_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "CopiedCaption"
    });
    expect(target_hpos.ok && target_hpos.exists && target_hpos.value == "222" &&
            target_copied_caption.ok && !target_copied_caption.exists,
        "#767: undo should restore direct fields and remove copied memo-backed assignments");

    fs::remove_all(temp_dir, ignored);
}

void test_copy_visual_object_properties_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_property_copy_batch_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "property_copy_batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "111", "211", "Caption = \"Save\"\r\nLeft = 10\r\n"},
        {"txtName", "nameBox", "name-guid", "222", "322", "Caption = \"Name\"\r\nTop = 30\r\n"},
        {"lblStatus", "statusLabel", "status-guid", "333", "433", "Caption = \"Status\"\r\nLeft = 50\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#774: property-copy-batch fixture should be writable");

    const auto property_state = [&](const std::string& unique_id, const std::string& property_name) {
        return copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
    };

    auto batch_result = copperfin::vfp::copy_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "hpos",
                .target_record_index = 0U,
                .target_object_name = "txtName",
                .target_unique_id = {},
                .target_property_name = {},
                .replace_existing = true
            },
            {
                .source_record_index = 0U,
                .source_object_name = "cmdSave",
                .source_unique_id = {},
                .source_property_name = "Caption",
                .target_record_index = 2U,
                .target_object_name = {},
                .target_unique_id = {},
                .target_property_name = "CopiedCaption",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "status-guid",
                .source_property_name = "Left",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "name-guid",
                .target_property_name = "StatusLeft",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "VPOS",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "status-guid",
                .target_property_name = "VPOS",
                .replace_existing = true
            }
        }
    });
    expect(batch_result.ok, "#774: batch property copy should support mixed selectors, direct fields, memo properties, target renames, and replacement");
    expect(batch_result.affected_object_count == 4U,
        "#1005: successful batch property copy should report affected item count");

    auto name_hpos = property_state("name-guid", "HPOS");
    auto status_copied_caption = property_state("status-guid", "CopiedCaption");
    auto name_status_left = property_state("name-guid", "StatusLeft");
    auto status_vpos = property_state("status-guid", "VPOS");
    auto save_caption = property_state("save-guid", "Caption");
    expect(name_hpos.ok && name_hpos.exists && name_hpos.value == "111" &&
            status_copied_caption.ok && status_copied_caption.exists && status_copied_caption.value == "\"Save\"" &&
            name_status_left.ok && name_status_left.exists && name_status_left.value == "50" &&
            status_vpos.ok && status_vpos.exists && status_vpos.value == "211" &&
            save_caption.ok && save_caption.exists && save_caption.value == "\"Save\"",
        "#774: batch property copy should persist target values while preserving sources and unrelated assignments");

    const auto undo_before_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_before_failure.available,
        "#774: successful batch copies should leave normal visual undo history available");

    batch_result = copperfin::vfp::copy_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "Left",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "status-guid",
                .target_property_name = "TempLeft",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "Caption",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "name-guid",
                .target_property_name = "Caption",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#774: batch property copy should reject target collisions");
    expect(batch_result.affected_object_count == 0U,
        "#1005: failed batch property copy should report zero affected objects");
    auto status_temp_left = property_state("status-guid", "TempLeft");
    expect(status_temp_left.ok && !status_temp_left.exists,
        "#774: target-collision failures should roll back earlier memo copy targets");

    batch_result = copperfin::vfp::copy_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "Left",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "status-guid",
                .target_property_name = "TempLeft",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "Missing",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "name-guid",
                .target_property_name = "MissingCopy",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#774: batch property copy should reject missing source properties");
    status_temp_left = property_state("status-guid", "TempLeft");
    expect(status_temp_left.ok && !status_temp_left.exists,
        "#774: missing-source failures should roll back earlier memo copy targets");

    batch_result = copperfin::vfp::copy_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "Left",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "status-guid",
                .target_property_name = "TempLeft",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = " ",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "name-guid",
                .target_property_name = "EmptySource",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#774: batch property copy should reject empty source names");
    status_temp_left = property_state("status-guid", "TempLeft");
    expect(status_temp_left.ok && !status_temp_left.exists,
        "#774: empty-source failures should roll back earlier memo copy targets");

    batch_result = copperfin::vfp::copy_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "Left",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "status-guid",
                .target_property_name = "TempLeft",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "Caption",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "name-guid",
                .target_property_name = " ",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#774: batch property copy should reject empty target names");
    status_temp_left = property_state("status-guid", "TempLeft");
    expect(status_temp_left.ok && !status_temp_left.exists,
        "#774: empty-target failures should roll back earlier memo copy targets");

    const auto undo_after_failures = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_after_failures.available == undo_before_failure.available &&
            undo_after_failures.label == undo_before_failure.label,
        "#774: failed batch copy rollbacks should preserve prior undo history");

    batch_result = copperfin::vfp::copy_visual_object_properties({
        .path = table_path.string(),
        .properties = {}
    });
    expect(!batch_result.ok, "#774: empty batch copy requests should fail explicitly");
    expect(batch_result.affected_object_count == 0U,
        "#1005: empty batch property copy should report zero affected objects");

    for (int index = 0; index < 4; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#774: undo should restore each successful batch copy target");
    }

    name_hpos = property_state("name-guid", "HPOS");
    status_copied_caption = property_state("status-guid", "CopiedCaption");
    name_status_left = property_state("name-guid", "StatusLeft");
    status_vpos = property_state("status-guid", "VPOS");
    expect(name_hpos.ok && name_hpos.exists && name_hpos.value == "222" &&
            status_copied_caption.ok && !status_copied_caption.exists &&
            name_status_left.ok && !name_status_left.exists &&
            status_vpos.ok && status_vpos.exists && status_vpos.value == "433",
        "#774: successful batch copy undo should restore original target state");

    fs::remove_all(temp_dir, ignored);
}

void test_move_visual_object_property_between_selected_objects() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_property_move_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "property_move.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "111", "Caption = \"Save\"\r\nLeft = 10\r\n"},
        {"txtName", "nameBox", "name-guid", "222", "Caption = \"Name\"\r\nTop = 30\r\n"},
        {"lblOther", "otherLabel", "other-guid", "333", "Caption = \"Other\"\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#768: property-move fixture should be writable");

    auto move_result = copperfin::vfp::move_visual_object_property({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "save-guid",
        .source_property_name = "hpos",
        .target_record_index = 0U,
        .target_object_name = "txtName",
        .target_unique_id = {},
        .target_property_name = {},
        .replace_existing = true
    });
    expect(move_result.ok, "#768: property move should support UNIQUEID source, object-name target, and direct-field replacement");
    expect(move_result.affected_object_count == 1U,
        "#1005: successful property move should report one affected object");

    move_result = copperfin::vfp::move_visual_object_property({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = "cmdSave",
        .source_unique_id = {},
        .source_property_name = "caption",
        .target_record_index = 1U,
        .target_object_name = {},
        .target_unique_id = {},
        .target_property_name = "MovedCaption",
        .replace_existing = false
    });
    expect(move_result.ok, "#768: property move should support object-name source, record-index target, and target renames");

    auto source_hpos = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "HPOS"
    });
    auto target_hpos = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .property_name = "HPOS"
    });
    auto source_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption"
    });
    auto moved_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "MovedCaption"
    });
    expect(source_hpos.ok && source_hpos.exists && source_hpos.value.empty() &&
            target_hpos.ok && target_hpos.exists && target_hpos.value == "111",
        "#768: direct-field moves should write target value and clear source value");
    expect(source_caption.ok && !source_caption.exists &&
            moved_caption.ok && moved_caption.exists && moved_caption.value == "\"Save\"",
        "#768: memo-backed moves should create target assignment and remove source assignment");

    auto target_top = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "Top"
    });
    auto other_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "other-guid",
        .property_name = "Caption"
    });
    expect(target_top.ok && target_top.exists && target_top.value == "30" &&
            other_caption.ok && other_caption.exists && other_caption.value == "\"Other\"",
        "#768: property move should preserve unrelated target assignments and unrelated objects");

    move_result = copperfin::vfp::move_visual_object_property({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "name-guid",
        .source_property_name = "Caption",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "other-guid",
        .target_property_name = "Caption",
        .replace_existing = false
    });
    expect(!move_result.ok, "#768: property move should reject target collisions without replacement");
    expect(move_result.affected_object_count == 0U,
        "#1005: failed property move should report zero affected objects");
    auto target_caption_after_failed_copy = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "Caption"
    });
    expect(target_caption_after_failed_copy.ok &&
            target_caption_after_failed_copy.exists &&
            target_caption_after_failed_copy.value == "\"Name\"",
        "#768: failed target copies should leave the source property intact");

    move_result = copperfin::vfp::move_visual_object_property({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "name-guid",
        .source_property_name = "Caption",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "name-guid",
        .target_property_name = {},
        .replace_existing = true
    });
    expect(!move_result.ok, "#768: property move should reject same-object same-property moves");

    move_result = copperfin::vfp::move_visual_object_property({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "name-guid",
        .source_property_name = "MissingProp",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "other-guid",
        .target_property_name = "MissingCopy",
        .replace_existing = false
    });
    expect(!move_result.ok, "#768: property move should reject missing source properties");

    move_result = copperfin::vfp::move_visual_object_property({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "name-guid",
        .source_property_name = " ",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "other-guid",
        .target_property_name = "EmptySource",
        .replace_existing = false
    });
    expect(!move_result.ok, "#768: property move should reject empty source property names");

    move_result = copperfin::vfp::move_visual_object_property({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "name-guid",
        .source_property_name = "Caption",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "other-guid",
        .target_property_name = " ",
        .replace_existing = false
    });
    expect(!move_result.ok, "#768: property move should reject empty requested target property names");

    const fs::path unsupported_path = temp_dir / "property_move_unsupported.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> unsupported_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> unsupported_records{
        {"cmdSource", "source-guid", "111"},
        {"txtTarget", "target-guid", "222"}
    };
    const auto unsupported_create = copperfin::vfp::create_dbf_table_file(
        unsupported_path.string(),
        unsupported_fields,
        unsupported_records);
    expect(unsupported_create.ok, "#768: unsupported property-move fixture should be writable");
    move_result = copperfin::vfp::move_visual_object_property({
        .path = unsupported_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_property_name = "HPOS",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_property_name = "Caption",
        .replace_existing = false
    });
    expect(!move_result.ok, "#768: property move should reject unsupported target property paths");

    for (int index = 0; index < 4; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#768: undo should restore each copy/clear step from successful moves");
    }

    source_hpos = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "HPOS"
    });
    target_hpos = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .property_name = "HPOS"
    });
    source_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption"
    });
    moved_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "MovedCaption"
    });
    expect(source_hpos.ok && source_hpos.exists && source_hpos.value == "111" &&
            target_hpos.ok && target_hpos.exists && target_hpos.value == "222" &&
            source_caption.ok && source_caption.exists && source_caption.value == "\"Save\"" &&
            moved_caption.ok && !moved_caption.exists,
        "#768: undo should restore moved source and target property state");

    fs::remove_all(temp_dir, ignored);
}

void test_move_visual_object_properties_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_property_move_batch_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "property_move_batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "111", "211", "Caption = \"Save\"\r\nLeft = 10\r\n"},
        {"txtName", "nameBox", "name-guid", "222", "322", "Caption = \"Name\"\r\nTop = 30\r\n"},
        {"lblStatus", "statusLabel", "status-guid", "333", "433", "Caption = \"Status\"\r\nLeft = 50\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#775: property-move-batch fixture should be writable");

    const auto property_state = [&](const std::string& unique_id, const std::string& property_name) {
        return copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
    };

    auto batch_result = copperfin::vfp::move_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "hpos",
                .target_record_index = 0U,
                .target_object_name = "txtName",
                .target_unique_id = {},
                .target_property_name = {},
                .replace_existing = true
            },
            {
                .source_record_index = 0U,
                .source_object_name = "cmdSave",
                .source_unique_id = {},
                .source_property_name = "Caption",
                .target_record_index = 2U,
                .target_object_name = {},
                .target_unique_id = {},
                .target_property_name = "MovedCaption",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "status-guid",
                .source_property_name = "Left",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "name-guid",
                .target_property_name = "StatusLeft",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "VPOS",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "status-guid",
                .target_property_name = "VPOS",
                .replace_existing = true
            }
        }
    });
    expect(batch_result.ok, "#775: batch property move should support mixed selectors, direct fields, memo properties, target renames, and replacement");
    expect(batch_result.affected_object_count == 4U,
        "#1005: successful batch property move should report affected item count");

    auto save_hpos = property_state("save-guid", "HPOS");
    auto save_vpos = property_state("save-guid", "VPOS");
    auto name_hpos = property_state("name-guid", "HPOS");
    auto status_vpos = property_state("status-guid", "VPOS");
    auto save_caption = property_state("save-guid", "Caption");
    auto status_moved_caption = property_state("status-guid", "MovedCaption");
    auto status_left = property_state("status-guid", "Left");
    auto name_status_left = property_state("name-guid", "StatusLeft");
    auto name_top = property_state("name-guid", "Top");
    expect(save_hpos.ok && save_hpos.exists && save_hpos.value.empty() &&
            save_vpos.ok && save_vpos.exists && save_vpos.value.empty() &&
            name_hpos.ok && name_hpos.exists && name_hpos.value == "111" &&
            status_vpos.ok && status_vpos.exists && status_vpos.value == "211",
        "#775: direct-field batch moves should write target values and clear source values");
    expect(save_caption.ok && !save_caption.exists &&
            status_moved_caption.ok && status_moved_caption.exists && status_moved_caption.value == "\"Save\"" &&
            status_left.ok && !status_left.exists &&
            name_status_left.ok && name_status_left.exists && name_status_left.value == "50" &&
            name_top.ok && name_top.exists && name_top.value == "30",
        "#775: memo-backed batch moves should create renamed targets, clear sources, and preserve unrelated assignments");

    const auto undo_before_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_before_failure.available,
        "#775: successful batch moves should leave normal visual undo history available");

    batch_result = copperfin::vfp::move_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "Left",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "status-guid",
                .target_property_name = "TempLeft",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "name-guid",
                .source_property_name = "Caption",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "status-guid",
                .target_property_name = "Caption",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#775: batch property move should reject target collisions");
    expect(batch_result.affected_object_count == 0U,
        "#1005: failed batch property move should report zero affected objects");
    auto status_temp_left = property_state("status-guid", "TempLeft");
    auto save_left = property_state("save-guid", "Left");
    expect(status_temp_left.ok && !status_temp_left.exists &&
            save_left.ok && save_left.exists && save_left.value == "10",
        "#775: target-collision failures should roll back earlier moved memo targets and source clears");

    batch_result = copperfin::vfp::move_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "Left",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "status-guid",
                .target_property_name = "TempLeft",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "Missing",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "name-guid",
                .target_property_name = "MissingMove",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#775: batch property move should reject missing source properties");
    status_temp_left = property_state("status-guid", "TempLeft");
    save_left = property_state("save-guid", "Left");
    expect(status_temp_left.ok && !status_temp_left.exists &&
            save_left.ok && save_left.exists && save_left.value == "10",
        "#775: missing-source failures should roll back earlier moved memo targets and source clears");

    batch_result = copperfin::vfp::move_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "Left",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "status-guid",
                .target_property_name = "TempLeft",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = " ",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "name-guid",
                .target_property_name = "EmptySource",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#775: batch property move should reject empty source names");
    status_temp_left = property_state("status-guid", "TempLeft");
    save_left = property_state("save-guid", "Left");
    expect(status_temp_left.ok && !status_temp_left.exists &&
            save_left.ok && save_left.exists && save_left.value == "10",
        "#775: empty-source failures should roll back earlier moved memo targets and source clears");

    batch_result = copperfin::vfp::move_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "Left",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "status-guid",
                .target_property_name = "TempLeft",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "name-guid",
                .source_property_name = "Caption",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "status-guid",
                .target_property_name = " ",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#775: batch property move should reject empty target names");
    status_temp_left = property_state("status-guid", "TempLeft");
    save_left = property_state("save-guid", "Left");
    expect(status_temp_left.ok && !status_temp_left.exists &&
            save_left.ok && save_left.exists && save_left.value == "10",
        "#775: empty-target failures should roll back earlier moved memo targets and source clears");

    batch_result = copperfin::vfp::move_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "name-guid",
                .source_property_name = "Caption",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "name-guid",
                .target_property_name = {},
                .replace_existing = true
            }
        }
    });
    expect(!batch_result.ok, "#775: batch property move should reject same-object same-property moves");
    auto name_caption = property_state("name-guid", "Caption");
    expect(name_caption.ok && name_caption.exists && name_caption.value == "\"Name\"",
        "#775: same-object same-property failures should preserve the source property");

    const auto undo_after_failures = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_after_failures.available == undo_before_failure.available &&
            undo_after_failures.label == undo_before_failure.label,
        "#775: failed batch move rollbacks should preserve prior undo history");

    batch_result = copperfin::vfp::move_visual_object_properties({
        .path = table_path.string(),
        .properties = {}
    });
    expect(!batch_result.ok, "#775: empty batch move requests should fail explicitly");
    expect(batch_result.affected_object_count == 0U,
        "#1005: empty batch property move should report zero affected objects");

    for (int index = 0; index < 8; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#775: undo should restore each copy/clear step from successful batch moves");
    }

    save_hpos = property_state("save-guid", "HPOS");
    save_vpos = property_state("save-guid", "VPOS");
    save_caption = property_state("save-guid", "Caption");
    save_left = property_state("save-guid", "Left");
    name_hpos = property_state("name-guid", "HPOS");
    name_status_left = property_state("name-guid", "StatusLeft");
    status_vpos = property_state("status-guid", "VPOS");
    status_moved_caption = property_state("status-guid", "MovedCaption");
    status_left = property_state("status-guid", "Left");
    expect(save_hpos.ok && save_hpos.exists && save_hpos.value == "111" &&
            save_vpos.ok && save_vpos.exists && save_vpos.value == "211" &&
            save_caption.ok && save_caption.exists && save_caption.value == "\"Save\"" &&
            save_left.ok && save_left.exists && save_left.value == "10" &&
            name_hpos.ok && name_hpos.exists && name_hpos.value == "222" &&
            name_status_left.ok && !name_status_left.exists &&
            status_vpos.ok && status_vpos.exists && status_vpos.value == "433" &&
            status_moved_caption.ok && !status_moved_caption.exists &&
            status_left.ok && status_left.exists && status_left.value == "50",
        "#775: successful batch move undo should restore original source and target state");

    fs::remove_all(temp_dir, ignored);
}

void test_list_visual_object_properties_reads_selected_surface() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_list_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "list.scx";
    const fs::path memo_path = temp_dir / "list.sct";
    write_synthetic_named_object_asset(table_path, memo_path, {
        {
            .objname = "cmdSave",
            .name = "saveButton",
            .unique_id = "save-guid",
            .properties = "Caption = \"Save\"\r\nLeft = 10\r\n"
        },
        {
            .objname = "txtName",
            .name = "nameBox",
            .unique_id = "target-guid",
            .properties = "Caption = \"Name\"\r\nLeft = 30\r\n"
        }
    });

    const auto list_result = copperfin::vfp::list_visual_object_properties({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid"
    });
    expect(list_result.ok, "#737: visual property lists should support selected-object UNIQUEID selectors");
    expect(list_result.record_index == 1U, "#739: visual property lists should report the resolved selected record index");

    const auto* name = find_property_snapshot(list_result.properties, "NAME");
    const auto* unique_id = find_property_snapshot(list_result.properties, "UNIQUEID");
    const auto* properties = find_property_snapshot(list_result.properties, "PROPERTIES");
    const auto* caption = find_property_snapshot(list_result.properties, "Caption");
    const auto* left = find_property_snapshot(list_result.properties, "Left");
    expect(name != nullptr && name->direct_field && name->value == "nameBox",
        "#737: visual property lists should include selected direct DBF fields");
    expect(name != nullptr && name->field_type == 'C' && name->source_line_index == static_cast<std::size_t>(-1),
        "#738: direct property list entries should expose DBF field type and missing source-line metadata");
    expect(unique_id != nullptr && unique_id->direct_field && unique_id->value == "target-guid",
        "#737: visual property lists should include selected memo-backed DBF identity fields as direct entries");
    expect(unique_id != nullptr && unique_id->field_type == 'M',
        "#738: memo-backed direct DBF fields should preserve their DBF field type in property listings");
    expect(properties == nullptr,
        "#737: visual property lists should not duplicate the raw PROPERTIES carrier field");
    expect(caption != nullptr && !caption->direct_field && caption->value == "\"Name\"",
        "#737: visual property lists should include parsed memo-backed Caption assignments");
    expect(caption != nullptr && caption->field_type == '\0' && caption->source_line_index == 0U,
        "#738: memo-backed property list entries should expose parsed source-line metadata without DBF field type");
    expect(left != nullptr && !left->direct_field && left->value == "30",
        "#737: visual property lists should include parsed memo-backed Left assignments");
    expect(left != nullptr && left->source_line_index == 1U,
        "#738: later memo-backed property list entries should retain their parsed source line index");

    const auto unfiltered_search = copperfin::vfp::filter_visual_object_properties({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid",
        .search_text = {}
    });
    expect(unfiltered_search.ok &&
            unfiltered_search.record_index == list_result.record_index &&
            unfiltered_search.property_count == list_result.properties.size() &&
            unfiltered_search.properties.size() == list_result.properties.size() &&
            unfiltered_search.dry_run &&
            !unfiltered_search.mutates_asset,
        "#1412: unfiltered visual property searches should preserve the selected property list without mutation");

    const auto name_search = copperfin::vfp::filter_visual_object_properties({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid",
        .search_text = "caption"
    });
    expect(name_search.ok &&
            name_search.property_count == 1U &&
            find_property_snapshot(name_search.properties, "Caption") != nullptr,
        "#1412: visual property searches should match property names case-insensitively");

    const auto value_search = copperfin::vfp::filter_visual_object_properties({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid",
        .search_text = "namebox"
    });
    expect(value_search.ok &&
            value_search.property_count == 1U &&
            find_property_snapshot(value_search.properties, "NAME") != nullptr,
        "#1412: visual property searches should match direct property values case-insensitively");

    const auto memo_search = copperfin::vfp::filter_visual_object_properties({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid",
        .search_text = "memo"
    });
    expect(memo_search.ok &&
            find_property_snapshot(memo_search.properties, "Caption") != nullptr &&
            find_property_snapshot(memo_search.properties, "Left") != nullptr &&
            find_property_snapshot(memo_search.properties, "NAME") == nullptr,
        "#1412: visual property searches should match memo-backed property metadata without direct fields");

    const auto empty_search = copperfin::vfp::filter_visual_object_properties({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid",
        .search_text = "does-not-exist"
    });
    expect(empty_search.ok &&
            empty_search.property_count == 0U &&
            empty_search.properties.empty(),
        "#1412: visual property searches should return empty successful results for unmatched text");

    expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
        "#737: visual property lists should not create undo history");

    fs::remove_all(temp_dir, ignored);
}

}  // namespace cf_test_visual_asset_editor
