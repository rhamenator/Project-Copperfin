// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
void test_update_visual_object_property_rewrites_properties_memo() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "sample.scx";
    const fs::path memo_path = temp_dir / "sample.sct";

    std::vector<std::uint8_t> table_bytes(110U, 0U);
    table_bytes[0] = 0x30U;
    table_bytes[1] = 126U;
    table_bytes[2] = 4U;
    table_bytes[3] = 7U;
    write_le_u32(table_bytes, 4U, 1U);
    write_le_u16(table_bytes, 8U, 97U);
    write_le_u16(table_bytes, 10U, 13U);
    table_bytes[28] = 0x00U;
    table_bytes[29] = 0x03U;

    write_field_descriptor(table_bytes, 32U, "OBJNAME", 'M', 1U, 4U);
    write_field_descriptor(table_bytes, 64U, "PROPERTIES", 'M', 5U, 4U);
    table_bytes[96] = 0x0DU;

    table_bytes[97] = 0x20U;
    write_le_u32(table_bytes, 98U, 1U);
    write_le_u32(table_bytes, 102U, 2U);

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }

    std::vector<std::uint8_t> memo_bytes(2048U, 0U);
    write_be_u32(memo_bytes, 0U, 3U);
    write_be_u16(memo_bytes, 6U, 512U);

    memo_bytes[512 + 3] = 1U;
    write_be_u32(memo_bytes, 512 + 4, 8U);
    write_ascii(memo_bytes, 520U, "txtTitle");

    const std::string properties = "Left = 10\r\nTop = 20\r\nWidth = 40\r\nHeight = 12\r\nName = \"txtTitle\"\r\n";
    memo_bytes[1024 + 3] = 1U;
    write_be_u32(memo_bytes, 1024 + 4, static_cast<std::uint32_t>(properties.size()));
    write_ascii(memo_bytes, 1032U, properties);

    {
        std::ofstream output(memo_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }

    const auto update_result = copperfin::vfp::update_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "Left",
        .property_value = "25"
    });

    expect(update_result.ok, "update_visual_object_property should succeed for a synthetic SCX/SCT pair");
    expect(update_result.affected_object_count == 1U,
        "#1007: successful single property edit should report one affected object");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
    expect(parse_result.ok, "updated synthetic SCX/SCT should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 1U) {
        const auto& record = parse_result.table.records[0];
        bool found = false;
        for (const auto& value : record.values) {
            if (value.field_name == "PROPERTIES") {
                found = true;
                expect(value.display_value.find("Left = 25") != std::string::npos, "updated PROPERTIES memo should contain the new Left value");
            }
        }
        expect(found, "updated record should still expose the PROPERTIES field");
    }

    const auto undo_status = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_status.available, "memo-backed asset edits should leave an undo journal entry behind");
    expect(undo_status.label.find("Left") != std::string::npos, "undo label should name the edited property");

    const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "undo_visual_object_property should revert memo-backed asset edits");
    expect(undo_result.affected_object_count == 1U,
        "#1008: successful visual property undo should report one affected object");

    const auto reverted_parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
    expect(reverted_parse_result.ok, "reverted synthetic SCX/SCT should remain readable");
    if (reverted_parse_result.ok && reverted_parse_result.table.records.size() == 1U) {
        const auto& record = reverted_parse_result.table.records[0];
        for (const auto& value : record.values) {
            if (value.field_name == "PROPERTIES") {
                expect(value.display_value.find("Left = 10") != std::string::npos, "undo should restore the original Left value");
            }
        }
    }

    const auto empty_undo_status = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(!empty_undo_status.available, "undo journal should be empty after undoing the only memo-backed edit");
    const auto missing_undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(!missing_undo_result.ok, "#1008: undo should fail when no visual asset undo history is available");
    expect(missing_undo_result.affected_object_count == 0U,
        "#1008: failed visual property undo should report zero affected objects");

    fs::remove_all(temp_dir, ignored);
}

void test_update_visual_object_properties_updates_selected_geometry_fields() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_multi_property_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "geometry.scx";
    const fs::path memo_path = temp_dir / "geometry.sct";
    write_synthetic_named_geometry_asset(table_path, memo_path);

    const auto update_result = copperfin::vfp::update_visual_object_properties({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid",
        .properties = {
            {.property_name = "hpos", .property_value = "333.000"},
            {.property_name = "VPOS", .property_value = "444.000"}
        }
    });
    expect(update_result.ok, "#735: multi-property edits should update selected geometry fields");
    expect(update_result.affected_object_count == 1U,
        "#1007: successful multi-property edit should report one affected object");

    auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
    expect(parse_result.ok, "#735: multi-property geometry fixture should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 2U) {
        const auto* first_hpos = find_record_field(parse_result.table.records[0], "HPOS");
        const auto* first_vpos = find_record_field(parse_result.table.records[0], "VPOS");
        const auto* second_hpos = find_record_field(parse_result.table.records[1], "HPOS");
        const auto* second_vpos = find_record_field(parse_result.table.records[1], "VPOS");
        expect(first_hpos != nullptr && std::abs(parse_number(first_hpos->display_value) - 111.0) < 0.001,
            "#735: multi-property edits should preserve unrelated HPOS values");
        expect(first_vpos != nullptr && std::abs(parse_number(first_vpos->display_value) - 211.0) < 0.001,
            "#735: multi-property edits should preserve unrelated VPOS values");
        expect(second_hpos != nullptr && std::abs(parse_number(second_hpos->display_value) - 333.0) < 0.001,
            "#735: multi-property edits should update selected HPOS values");
        expect(second_vpos != nullptr && std::abs(parse_number(second_vpos->display_value) - 444.0) < 0.001,
            "#735: multi-property edits should update selected VPOS values");
    }

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#735: multi-property edits should keep existing per-property undo compatibility");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#735: multi-property edits should make each changed property undoable");
    parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
    expect(parse_result.ok, "#735: multi-property geometry fixture should remain readable after undo");
    if (parse_result.ok && parse_result.table.records.size() == 2U) {
        const auto* second_hpos = find_record_field(parse_result.table.records[1], "HPOS");
        const auto* second_vpos = find_record_field(parse_result.table.records[1], "VPOS");
        expect(second_hpos != nullptr && std::abs(parse_number(second_hpos->display_value) - 222.0) < 0.001,
            "#735: multi-property undo should restore selected HPOS values");
        expect(second_vpos != nullptr && std::abs(parse_number(second_vpos->display_value) - 322.0) < 0.001,
            "#735: multi-property undo should restore selected VPOS values");
    }
    const auto empty_result = copperfin::vfp::update_visual_object_properties({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid",
        .properties = {}
    });
    expect(!empty_result.ok, "#735: empty multi-property edit requests should fail explicitly");
    expect(empty_result.affected_object_count == 0U,
        "#1007: empty multi-property edit should report zero affected objects");

    fs::remove_all(temp_dir, ignored);
}

void test_update_visual_object_properties_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_multi_property_rollback_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "geometry_rollback.scx";
    const fs::path memo_path = temp_dir / "geometry_rollback.sct";
    write_synthetic_named_geometry_asset(table_path, memo_path);

    const auto update_result = copperfin::vfp::update_visual_object_properties({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid",
        .properties = {
            {.property_name = "HPOS", .property_value = "333.000"},
            {.property_name = "NOT_A_FIELD", .property_value = "444.000"}
        }
    });
    expect(!update_result.ok, "#740: failing multi-property edits should report the failed property change");
    expect(update_result.affected_object_count == 0U,
        "#1007: failed multi-property edit should report zero affected objects");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
    expect(parse_result.ok, "#740: rollback geometry fixture should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 2U) {
        const auto* second_hpos = find_record_field(parse_result.table.records[1], "HPOS");
        const auto* second_vpos = find_record_field(parse_result.table.records[1], "VPOS");
        expect(second_hpos != nullptr && std::abs(parse_number(second_hpos->display_value) - 222.0) < 0.001,
            "#740: failed multi-property edits should restore earlier successful field changes");
        expect(second_vpos != nullptr && std::abs(parse_number(second_vpos->display_value) - 322.0) < 0.001,
            "#740: failed multi-property edits should leave later untouched fields unchanged");
    }
    expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
        "#740: failed multi-property rollback should not leave extra undo history");

    fs::remove_all(temp_dir, ignored);
}

void test_update_visual_object_batch_rolls_back_failed_alignment() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_batch_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "10", "20", "Caption = \"Save\"\r\n"},
        {"txtName", "nameBox", "name-guid", "30", "40", "Caption = \"Name\"\r\n"},
        {"lblStatus", "statusLabel", "status-guid", "50", "60", "Caption = \"Status\"\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#752: batch-edit fixture should be writable");

    const auto property_value = [&](const std::string& unique_id, const std::string& property_name) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
        expect(result.ok && result.exists, "#752: batch-edit fixture property should be readable");
        return result.value;
    };

    auto batch_result = copperfin::vfp::update_visual_object_batch({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .properties = {
                    {.property_name = "HPOS", .property_value = "100"},
                    {.property_name = "VPOS", .property_value = "200"}
                }
            },
            {
                .record_index = 0U,
                .object_name = "txtName",
                .unique_id = {},
                .properties = {
                    {.property_name = "HPOS", .property_value = "100"},
                    {.property_name = "VPOS", .property_value = "300"}
                }
            }
        }
    });
    expect(batch_result.ok, "#752: batch edits should apply multi-object geometry changes");
    expect(batch_result.affected_object_count == 2U,
        "#998: successful batch edits should report affected object count");
    expect(property_value("save-guid", "HPOS") == "100" &&
            property_value("save-guid", "VPOS") == "200",
        "#752: batch edits should update UNIQUEID-selected geometry");
    expect(property_value("name-guid", "HPOS") == "100" &&
            property_value("name-guid", "VPOS") == "300",
        "#752: batch edits should update object-name-selected geometry");
    expect(property_value("status-guid", "HPOS") == "50" &&
            property_value("status-guid", "VPOS") == "60",
        "#752: batch edits should preserve unrelated records");

    const auto undo_before_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_before_failure.available,
        "#752: successful batch edits should leave normal visual undo history available");

    batch_result = copperfin::vfp::update_visual_object_batch({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = "cmdSave",
                .unique_id = {},
                .properties = {
                    {.property_name = "HPOS", .property_value = "400"}
                }
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "name-guid",
                .properties = {}
            }
        }
    });
    expect(!batch_result.ok, "#752: batch edits should fail explicitly on an empty item property list");
    expect(batch_result.affected_object_count == 0U,
        "#998: failed batch edits should report zero affected objects after rollback");
    expect(property_value("save-guid", "HPOS") == "100",
        "#752: failed batch edits should roll back earlier successful object edits");
    const auto undo_after_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_after_failure.available == undo_before_failure.available &&
            undo_after_failure.label == undo_before_failure.label,
        "#752: failed batch rollback should clean up undo entries created by the failed batch");

    batch_result = copperfin::vfp::update_visual_object_batch({
        .path = table_path.string(),
        .objects = {}
    });
    expect(!batch_result.ok, "#752: empty batch edit requests should fail explicitly");
    expect(batch_result.affected_object_count == 0U,
        "#998: empty batch edit requests should report zero affected objects");
    expect(property_value("save-guid", "HPOS") == "100" &&
            property_value("name-guid", "VPOS") == "300",
        "#752: empty batch edit requests should not mutate existing geometry");

    fs::remove_all(temp_dir, ignored);
}

void test_update_visual_object_batch_undoes_report_and_label_batches_in_single_step() {
    namespace fs = std::filesystem;
    const auto exercise_asset = [&](const std::string& stem,
                                    const std::string& table_extension,
                                    const std::string& asset_label) {
        const fs::path temp_dir = fs::temp_directory_path() /
            ("copperfin_visual_editor_report_batch_undo_" + stem + "_tests_" + std::to_string(_getpid()));
        std::error_code ignored;
        fs::remove_all(temp_dir, ignored);
        fs::create_directories(temp_dir);

        const fs::path table_path = temp_dir / ("batch" + table_extension);
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "OBJTYPE", .type = 'N', .length = 8U},
            {.name = "HPOS", .type = 'N', .length = 10U},
            {.name = "WIDTH", .type = 'N', .length = 10U},
            {.name = "EXPR", .type = 'M', .length = 4U}
        };
        const std::vector<std::vector<std::string>> records{
            {"8", "1200", "2400", "customer.company"}
        };
        const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
        expect(create_result.ok, asset_label + " batch-undo fixture should be writable");

        const auto batch_result = copperfin::vfp::update_visual_object_batch({
            .path = table_path.string(),
            .objects = {
                {
                    .record_index = 0U,
                    .object_name = {},
                    .unique_id = {},
                    .properties = {
                        {.property_name = "HPOS", .property_value = "1800"},
                        {.property_name = "WIDTH", .property_value = "3200"},
                        {.property_name = "EXPR", .property_value = "\"updated.expr\""}
                    }
                }
            }
        });
        expect(batch_result.ok, asset_label + " batch property edits should succeed");
        expect(batch_result.affected_object_count == 1U,
            asset_label + " batch property edits should report one affected object");

        auto undo_status = copperfin::vfp::query_visual_object_undo(table_path.string());
        expect(undo_status.available, asset_label + " batch property edits should expose undo");
        expect(undo_status.label.find("EXPR") != std::string::npos,
            asset_label + " batch property edits should keep the latest-property undo label");

        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, asset_label + " batch property edits should undo in a single command");

        const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
        expect(parse_result.ok, asset_label + " fixture should remain readable after the command undo");
        if (parse_result.ok && parse_result.table.records.size() == 1U) {
            const auto& record = parse_result.table.records[0];
            for (const auto& value : record.values) {
                if (value.field_name == "HPOS") {
                    expect(value.display_value == "1200", asset_label + " command undo should restore HPOS");
                }
                if (value.field_name == "WIDTH") {
                    expect(value.display_value == "2400", asset_label + " command undo should restore WIDTH");
                }
                if (value.field_name == "EXPR") {
                    expect(value.display_value == "customer.company", asset_label + " command undo should restore EXPR");
                }
            }
        }

        undo_status = copperfin::vfp::query_visual_object_undo(table_path.string());
        expect(!undo_status.available, asset_label + " command undo should consume the only report batch history entry");

        fs::remove_all(temp_dir, ignored);
    };

    exercise_asset("frx", ".frx", "Report");
    exercise_asset("lbx", ".lbx", "Label");
}

void test_update_visual_object_property_skips_noop_writes() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_noop_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path memo_table_path = temp_dir / "noop_memo.scx";
    const fs::path memo_path = temp_dir / "noop_memo.sct";
    write_synthetic_named_object_asset(memo_table_path, memo_path, {
        {
            .objname = "cmdSave",
            .name = "saveButton",
            .unique_id = "save-guid",
            .properties = "Caption = \"Save\"\r\nLeft = 10\r\n"
        }
    });
    const auto memo_table_before = read_file_bytes(memo_table_path);
    const auto memo_before = read_file_bytes(memo_path);
    const auto memo_update_result = copperfin::vfp::update_visual_object_property({
        .path = memo_table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption",
        .property_value = "\"Save\""
    });
    expect(memo_update_result.ok, "#733: unchanged memo-backed property edits should succeed as no-ops");
    expect(read_file_bytes(memo_table_path) == memo_table_before,
        "#733: unchanged memo-backed property edits should not rewrite the table bytes");
    expect(read_file_bytes(memo_path) == memo_before,
        "#733: unchanged memo-backed property edits should not rewrite the memo bytes");
    expect(!copperfin::vfp::query_visual_object_undo(memo_table_path.string()).available,
        "#733: unchanged memo-backed property edits should not create undo history");

    const fs::path direct_table_path = temp_dir / "noop_direct.scx";
    const fs::path direct_memo_path = temp_dir / "noop_direct.sct";
    write_synthetic_named_direct_asset(direct_table_path, direct_memo_path);
    const auto direct_table_before = read_file_bytes(direct_table_path);
    const auto direct_memo_before = read_file_bytes(direct_memo_path);
    const auto direct_update_result = copperfin::vfp::update_visual_object_property({
        .path = direct_table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .property_name = "HPOS",
        .property_value = "222.000"
    });
    expect(direct_update_result.ok, "#733: unchanged direct-field property edits should succeed as no-ops");
    expect(read_file_bytes(direct_table_path) == direct_table_before,
        "#733: unchanged direct-field property edits should not rewrite the table bytes");
    expect(read_file_bytes(direct_memo_path) == direct_memo_before,
        "#733: unchanged direct-field property edits should not rewrite the memo bytes");
    expect(!copperfin::vfp::query_visual_object_undo(direct_table_path.string()).available,
        "#733: unchanged direct-field property edits should not create undo history");

    fs::remove_all(temp_dir, ignored);
}

void test_update_visual_object_property_skips_noop_writes_for_report_and_label_assets() {
    namespace fs = std::filesystem;
    const auto exercise_asset = [&](const std::string& stem,
                                    const std::string& table_extension,
                                    const std::string& memo_extension,
                                    const std::string& asset_label) {
        const fs::path temp_dir = fs::temp_directory_path() /
            ("copperfin_visual_editor_noop_" + stem + "_tests_" + std::to_string(_getpid()));
        std::error_code ignored;
        fs::remove_all(temp_dir, ignored);
        fs::create_directories(temp_dir);

        const fs::path table_path = temp_dir / ("noop" + table_extension);
        const fs::path memo_path = temp_dir / ("noop" + memo_extension);

        std::vector<std::uint8_t> table_bytes(178U, 0U);
        table_bytes[0] = 0x30U;
        table_bytes[1] = 126U;
        table_bytes[2] = 4U;
        table_bytes[3] = 7U;
        write_le_u32(table_bytes, 4U, 1U);
        write_le_u16(table_bytes, 8U, 161U);
        write_le_u16(table_bytes, 10U, 17U);
        table_bytes[28] = 0x00U;
        table_bytes[29] = 0x03U;

        write_field_descriptor(table_bytes, 32U, "OBJTYPE", 'N', 1U, 2U);
        write_field_descriptor(table_bytes, 64U, "HPOS", 'N', 3U, 9U);
        write_field_descriptor(table_bytes, 96U, "GRID", 'L', 12U, 1U);
        write_field_descriptor(table_bytes, 128U, "EXPR", 'M', 13U, 4U);
        table_bytes[160] = 0x0DU;
        table_bytes[161] = 0x20U;
        write_ascii(table_bytes, 162U, " 8");
        write_ascii(table_bytes, 164U, "   7812.5");
        table_bytes[173] = 'F';
        write_le_u32(table_bytes, 174U, 1U);

        {
            std::ofstream output(table_path, std::ios::binary);
            output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
        }

        std::vector<std::uint8_t> memo_bytes(1024U, 0U);
        write_be_u32(memo_bytes, 0U, 2U);
        write_be_u16(memo_bytes, 6U, 512U);
        const std::string expr = "customer.company";
        memo_bytes[512 + 3] = 1U;
        write_be_u32(memo_bytes, 512 + 4, static_cast<std::uint32_t>(expr.size()));
        write_ascii(memo_bytes, 520U, expr);

        {
            std::ofstream output(memo_path, std::ios::binary);
            output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
        }

        const auto table_before = read_file_bytes(table_path);
        const auto memo_before = read_file_bytes(memo_path);

        const auto direct_update_result = copperfin::vfp::update_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS",
            .property_value = "7812.5"
        });
        expect(direct_update_result.ok, asset_label + " unchanged HPOS edits should succeed as no-ops");
        expect(read_file_bytes(table_path) == table_before,
            asset_label + " unchanged HPOS edits should not rewrite the table bytes");
        expect(read_file_bytes(memo_path) == memo_before,
            asset_label + " unchanged HPOS edits should not rewrite the memo bytes");
        expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
            asset_label + " unchanged HPOS edits should not create undo history");

        const auto memo_update_result = copperfin::vfp::update_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR",
            .property_value = "customer.company"
        });
        expect(memo_update_result.ok, asset_label + " unchanged EXPR edits should succeed as no-ops");
        expect(read_file_bytes(table_path) == table_before,
            asset_label + " unchanged EXPR edits should not rewrite the table bytes");
        expect(read_file_bytes(memo_path) == memo_before,
            asset_label + " unchanged EXPR edits should not rewrite the memo bytes");
        expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
            asset_label + " unchanged EXPR edits should not create undo history");

        fs::remove_all(temp_dir, ignored);
    };

    exercise_asset("frx", ".frx", ".frt", "Report");
    exercise_asset("lbx", ".lbx", ".lbt", "Label");
}

void test_update_visual_object_property_preserves_unsupported_report_and_label_metadata() {
    namespace fs = std::filesystem;
    const auto exercise_asset = [&](const std::string& stem,
                                    const std::string& table_extension,
                                    const std::string& asset_label) {
        const fs::path temp_dir = fs::temp_directory_path() /
            ("copperfin_visual_editor_preserve_unsupported_" + stem + "_tests_" + std::to_string(_getpid()));
        std::error_code ignored;
        fs::remove_all(temp_dir, ignored);
        fs::create_directories(temp_dir);

        const fs::path table_path = temp_dir / ("sample" + table_extension);
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "OBJTYPE", .type = 'N', .length = 8U},
            {.name = "HPOS", .type = 'N', .length = 10U},
            {.name = "EXPR", .type = 'M', .length = 4U},
            {.name = "USERFLAG", .type = 'C', .length = 24U},
            {.name = "USERMETA", .type = 'M', .length = 4U}
        };
        const std::vector<std::vector<std::string>> records{
            {"8", "1200", "customer.company", asset_label + "DirectMetadata", asset_label + "MemoMetadata"}
        };
        const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
        expect(create_result.ok, asset_label + " unsupported-metadata fixture should be writable");

        auto update_result = copperfin::vfp::update_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS",
            .property_value = "2400"
        });
        expect(update_result.ok, asset_label + " supported HPOS edits should succeed");

        update_result = copperfin::vfp::update_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR",
            .property_value = "\"updated.expr\""
        });
        expect(update_result.ok, asset_label + " supported EXPR edits should succeed");

        const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
        expect(parse_result.ok, asset_label + " unsupported-metadata fixture should remain readable after supported edits");
        if (parse_result.ok && parse_result.table.records.size() == 1U) {
            const auto& record = parse_result.table.records[0];
            const auto* hpos = find_record_field(record, "HPOS");
            const auto* expr = find_record_field(record, "EXPR");
            const auto* direct_metadata = find_record_field(record, "USERFLAG");
            const auto* memo_metadata = find_record_field(record, "USERMETA");
            expect(hpos != nullptr && hpos->display_value == "2400",
                asset_label + " supported HPOS edits should persist");
            expect(expr != nullptr && expr->display_value == "\"updated.expr\"",
                asset_label + " supported EXPR edits should persist");
            expect(direct_metadata != nullptr && direct_metadata->display_value == asset_label + "DirectMetadata",
                asset_label + " unsupported direct metadata should survive supported edits");
            expect(memo_metadata != nullptr && memo_metadata->display_value == asset_label + "MemoMetadata",
                asset_label + " unsupported memo metadata should survive supported edits");
        }

        auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, asset_label + " first undo should restore the supported EXPR edit");
        undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, asset_label + " second undo should restore the supported HPOS edit");

        const auto reverted_parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
        expect(reverted_parse_result.ok, asset_label + " unsupported-metadata fixture should remain readable after undo");
        if (reverted_parse_result.ok && reverted_parse_result.table.records.size() == 1U) {
            const auto& record = reverted_parse_result.table.records[0];
            const auto* hpos = find_record_field(record, "HPOS");
            const auto* expr = find_record_field(record, "EXPR");
            const auto* direct_metadata = find_record_field(record, "USERFLAG");
            const auto* memo_metadata = find_record_field(record, "USERMETA");
            expect(hpos != nullptr && hpos->display_value == "1200",
                asset_label + " undo should restore the original HPOS value");
            expect(expr != nullptr && expr->display_value == "customer.company",
                asset_label + " undo should restore the original EXPR value");
            expect(direct_metadata != nullptr && direct_metadata->display_value == asset_label + "DirectMetadata",
                asset_label + " unsupported direct metadata should survive undo");
            expect(memo_metadata != nullptr && memo_metadata->display_value == asset_label + "MemoMetadata",
                asset_label + " unsupported memo metadata should survive undo");
        }

        const auto undo_status = copperfin::vfp::query_visual_object_undo(table_path.string());
        expect(!undo_status.available, asset_label + " undo journal should be empty after restoring supported edits");

        fs::remove_all(temp_dir, ignored);
    };

    exercise_asset("frx", ".frx", "Report");
    exercise_asset("lbx", ".lbx", "Label");
}

void test_update_visual_object_property_preserves_report_and_label_sibling_rows() {
    namespace fs = std::filesystem;
    const auto exercise_asset = [&](const std::string& stem,
                                    const std::string& table_extension,
                                    const std::string& asset_label) {
        const fs::path temp_dir = fs::temp_directory_path() /
            ("copperfin_visual_editor_preserve_siblings_" + stem + "_tests_" + std::to_string(_getpid()));
        std::error_code ignored;
        fs::remove_all(temp_dir, ignored);
        fs::create_directories(temp_dir);

        const fs::path table_path = temp_dir / ("siblings" + table_extension);
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "OBJTYPE", .type = 'N', .length = 8U},
            {.name = "HPOS", .type = 'N', .length = 10U},
            {.name = "EXPR", .type = 'M', .length = 4U},
            {.name = "USERFLAG", .type = 'C', .length = 24U},
            {.name = "USERMETA", .type = 'M', .length = 4U}
        };
        const std::vector<std::vector<std::string>> records{
            {"8", "1200", "customer.company", asset_label + "TargetDirect", asset_label + "TargetMemo"},
            {"8", "2400", "shipto.city", asset_label + "SiblingDirect", asset_label + "SiblingMemo"},
            {"8", "3600", "deleted.total", asset_label + "DeletedDirect", asset_label + "DeletedMemo"}
        };
        const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
        expect(create_result.ok, asset_label + " sibling-row fixture should be writable");

        const auto delete_result = copperfin::vfp::set_record_deleted_flag(table_path.string(), 2U, true);
        expect(delete_result.ok, asset_label + " sibling-row fixture should support deleted-row setup");

        auto update_result = copperfin::vfp::update_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS",
            .property_value = "1800"
        });
        expect(update_result.ok, asset_label + " targeted HPOS edits should succeed in a multi-row fixture");

        update_result = copperfin::vfp::update_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR",
            .property_value = "\"updated.expr\""
        });
        expect(update_result.ok, asset_label + " targeted EXPR edits should succeed in a multi-row fixture");

        auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 3U);
        expect(parse_result.ok, asset_label + " sibling-row fixture should remain readable after targeted edits");
        if (parse_result.ok && parse_result.table.records.size() == 3U) {
            const auto& target_record = parse_result.table.records[0];
            const auto& sibling_record = parse_result.table.records[1];
            const auto& deleted_record = parse_result.table.records[2];
            const auto* target_hpos = find_record_field(target_record, "HPOS");
            const auto* target_expr = find_record_field(target_record, "EXPR");
            const auto* sibling_hpos = find_record_field(sibling_record, "HPOS");
            const auto* sibling_expr = find_record_field(sibling_record, "EXPR");
            const auto* deleted_hpos = find_record_field(deleted_record, "HPOS");
            const auto* deleted_expr = find_record_field(deleted_record, "EXPR");
            const auto* sibling_direct = find_record_field(sibling_record, "USERFLAG");
            const auto* sibling_memo = find_record_field(sibling_record, "USERMETA");
            const auto* deleted_direct = find_record_field(deleted_record, "USERFLAG");
            const auto* deleted_memo = find_record_field(deleted_record, "USERMETA");
            expect(target_hpos != nullptr && target_hpos->display_value == "1800",
                asset_label + " targeted direct-field edits should persist on the selected row");
            expect(target_expr != nullptr && target_expr->display_value == "\"updated.expr\"",
                asset_label + " targeted memo-backed edits should persist on the selected row");
            expect(sibling_hpos != nullptr && sibling_hpos->display_value == "2400",
                asset_label + " targeted edits should preserve live sibling HPOS values");
            expect(sibling_expr != nullptr && sibling_expr->display_value == "shipto.city",
                asset_label + " targeted edits should preserve live sibling EXPR values");
            expect(sibling_direct != nullptr && sibling_direct->display_value == asset_label + "SiblingDirect",
                asset_label + " targeted edits should preserve live sibling direct metadata");
            expect(sibling_memo != nullptr && sibling_memo->display_value == asset_label + "SiblingMemo",
                asset_label + " targeted edits should preserve live sibling memo metadata");
            expect(deleted_hpos != nullptr && deleted_hpos->display_value == "3600",
                asset_label + " targeted edits should preserve deleted sibling HPOS values");
            expect(deleted_expr != nullptr && deleted_expr->display_value == "deleted.total",
                asset_label + " targeted edits should preserve deleted sibling EXPR values");
            expect(deleted_direct != nullptr && deleted_direct->display_value == asset_label + "DeletedDirect",
                asset_label + " targeted edits should preserve deleted sibling direct metadata");
            expect(deleted_memo != nullptr && deleted_memo->display_value == asset_label + "DeletedMemo",
                asset_label + " targeted edits should preserve deleted sibling memo metadata");
            expect(!target_record.deleted && !sibling_record.deleted && deleted_record.deleted,
                asset_label + " targeted edits should preserve sibling deleted flags");
        }

        auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, asset_label + " first undo should restore the targeted EXPR edit");
        undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, asset_label + " second undo should restore the targeted HPOS edit");

        parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 3U);
        expect(parse_result.ok, asset_label + " sibling-row fixture should remain readable after undo");
        if (parse_result.ok && parse_result.table.records.size() == 3U) {
            const auto& target_record = parse_result.table.records[0];
            const auto& sibling_record = parse_result.table.records[1];
            const auto& deleted_record = parse_result.table.records[2];
            const auto* target_hpos = find_record_field(target_record, "HPOS");
            const auto* target_expr = find_record_field(target_record, "EXPR");
            const auto* sibling_hpos = find_record_field(sibling_record, "HPOS");
            const auto* sibling_expr = find_record_field(sibling_record, "EXPR");
            const auto* deleted_hpos = find_record_field(deleted_record, "HPOS");
            const auto* deleted_expr = find_record_field(deleted_record, "EXPR");
            expect(target_hpos != nullptr && target_hpos->display_value == "1200",
                asset_label + " undo should restore the targeted HPOS value");
            expect(target_expr != nullptr && target_expr->display_value == "customer.company",
                asset_label + " undo should restore the targeted EXPR value");
            expect(sibling_hpos != nullptr && sibling_hpos->display_value == "2400",
                asset_label + " undo should preserve live sibling HPOS values");
            expect(sibling_expr != nullptr && sibling_expr->display_value == "shipto.city",
                asset_label + " undo should preserve live sibling EXPR values");
            expect(deleted_hpos != nullptr && deleted_hpos->display_value == "3600",
                asset_label + " undo should preserve deleted sibling HPOS values");
            expect(deleted_expr != nullptr && deleted_expr->display_value == "deleted.total",
                asset_label + " undo should preserve deleted sibling EXPR values");
            expect(!target_record.deleted && !sibling_record.deleted && deleted_record.deleted,
                asset_label + " undo should preserve sibling deleted flags");
        }

        const auto undo_status = copperfin::vfp::query_visual_object_undo(table_path.string());
        expect(!undo_status.available, asset_label + " undo journal should be empty after restoring targeted edits");

        fs::remove_all(temp_dir, ignored);
    };

    exercise_asset("frx", ".frx", "Report");
    exercise_asset("lbx", ".lbx", "Label");
}

void test_report_and_label_asset_inspection_is_a_noop_binary_round_trip() {
    namespace fs = std::filesystem;
    const auto exercise_asset = [&](const std::string& stem,
                                    const std::string& table_extension,
                                    const std::string& memo_extension,
                                    const std::string& asset_label) {
        const fs::path temp_dir = fs::temp_directory_path() /
            ("copperfin_visual_editor_inspect_noop_" + stem + "_tests_" + std::to_string(_getpid()));
        std::error_code ignored;
        fs::remove_all(temp_dir, ignored);
        fs::create_directories(temp_dir);

        const fs::path table_path = temp_dir / ("inspect" + table_extension);
        const fs::path memo_path = temp_dir / ("inspect" + memo_extension);

        std::vector<std::uint8_t> table_bytes(178U, 0U);
        table_bytes[0] = 0x30U;
        table_bytes[1] = 126U;
        table_bytes[2] = 4U;
        table_bytes[3] = 7U;
        write_le_u32(table_bytes, 4U, 1U);
        write_le_u16(table_bytes, 8U, 161U);
        write_le_u16(table_bytes, 10U, 17U);
        table_bytes[28] = 0x00U;
        table_bytes[29] = 0x03U;

        write_field_descriptor(table_bytes, 32U, "OBJTYPE", 'N', 1U, 2U);
        write_field_descriptor(table_bytes, 64U, "HPOS", 'N', 3U, 9U);
        write_field_descriptor(table_bytes, 96U, "GRID", 'L', 12U, 1U);
        write_field_descriptor(table_bytes, 128U, "EXPR", 'M', 13U, 4U);
        table_bytes[160] = 0x0DU;
        table_bytes[161] = 0x20U;
        write_ascii(table_bytes, 162U, " 8");
        write_ascii(table_bytes, 164U, "   7812.5");
        table_bytes[173] = 'F';
        write_le_u32(table_bytes, 174U, 1U);

        {
            std::ofstream output(table_path, std::ios::binary);
            output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
        }

        std::vector<std::uint8_t> memo_bytes(1024U, 0U);
        write_be_u32(memo_bytes, 0U, 2U);
        write_be_u16(memo_bytes, 6U, 512U);
        const std::string expr = "customer.company";
        memo_bytes[512 + 3] = 1U;
        write_be_u32(memo_bytes, 512 + 4, static_cast<std::uint32_t>(expr.size()));
        write_ascii(memo_bytes, 520U, expr);

        {
            std::ofstream output(memo_path, std::ios::binary);
            output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
        }

        const auto table_before = read_file_bytes(table_path);
        const auto memo_before = read_file_bytes(memo_path);

        const auto property_result = copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR"
        });
        expect(property_result.ok && property_result.exists && property_result.value == "customer.company",
            asset_label + " property queries should succeed without mutation");

        const auto list_result = copperfin::vfp::list_visual_object_properties({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = {}
        });
        expect(list_result.ok &&
               list_result.properties.size() == 4U &&
               list_result.record_index == 0U,
            asset_label + " property listing should succeed without mutation");

        expect(read_file_bytes(table_path) == table_before,
            asset_label + " inspection should not rewrite the table bytes");
        expect(read_file_bytes(memo_path) == memo_before,
            asset_label + " inspection should not rewrite the memo bytes");
        expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
            asset_label + " inspection should not create undo history");

        fs::remove_all(temp_dir, ignored);
    };

    exercise_asset("frx", ".frx", ".frt", "Report");
    exercise_asset("lbx", ".lbx", ".lbt", "Label");
}

void test_update_visual_object_property_targets_selected_object_name() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_named_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "named.scx";
    const fs::path memo_path = temp_dir / "named.sct";
    write_synthetic_named_object_asset(table_path, memo_path, {
        {.objname = "cmdSave", .name = "saveButton", .unique_id = {}, .properties = "Caption = \"Save\"\r\nLeft = 10\r\n"},
        {.objname = "", .name = "fallbackButton", .unique_id = {}, .properties = "Caption = \"Fallback\"\r\nTop = 20\r\n"},
        {.objname = "txtName", .name = "nameBox", .unique_id = {}, .properties = "Caption = \"Name\"\r\nLeft = 30\r\n"}
    });

    auto update_result = copperfin::vfp::update_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "CMDSAVE",
        .unique_id = {},
        .property_name = "Caption",
        .property_value = "\"Persist\""
    });
    expect(update_result.ok, "#730: visual property edits should target selected objects by OBJNAME case-insensitively");

    update_result = copperfin::vfp::update_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "fallbackbutton",
        .unique_id = {},
        .property_name = "Top",
        .property_value = "44"
    });
    expect(update_result.ok, "#730: visual property edits should fall back to NAME when OBJNAME is absent");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 3U);
    expect(parse_result.ok, "#730: name-targeted edit fixture should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 3U) {
        const auto& first_record = parse_result.table.records[0];
        const auto& second_record = parse_result.table.records[1];
        const auto& third_record = parse_result.table.records[2];
        const auto first_properties = std::find_if(first_record.values.begin(), first_record.values.end(), [](const auto& value) {
            return value.field_name == "PROPERTIES";
        });
        const auto second_properties = std::find_if(second_record.values.begin(), second_record.values.end(), [](const auto& value) {
            return value.field_name == "PROPERTIES";
        });
        const auto third_properties = std::find_if(third_record.values.begin(), third_record.values.end(), [](const auto& value) {
            return value.field_name == "PROPERTIES";
        });
        expect(first_properties != first_record.values.end() &&
                first_properties->display_value.find("Caption = \"Persist\"") != std::string::npos,
            "#730: OBJNAME-targeted edits should update only the selected object's property blob");
        expect(second_properties != second_record.values.end() &&
                second_properties->display_value.find("Top = 44") != std::string::npos,
            "#730: NAME fallback edits should update the selected object's property blob");
        expect(third_properties != third_record.values.end() &&
                third_properties->display_value.find("Caption = \"Name\"") != std::string::npos,
            "#730: name-targeted edits should not update unrelated object property blobs");
    }

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#730: undo should restore the NAME fallback property edit");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#730: undo should restore the OBJNAME-targeted property edit");

    const auto missing_result = copperfin::vfp::update_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "doesNotExist",
        .unique_id = {},
        .property_name = "Caption",
        .property_value = "\"Missing\""
    });
    expect(!missing_result.ok, "#730: missing object names should fail instead of editing by record index");
    expect(missing_result.error.find("No visual object") != std::string::npos,
        "#730: missing object-name failures should explain that no object matched");

    const fs::path duplicate_table_path = temp_dir / "duplicate.scx";
    const fs::path duplicate_memo_path = temp_dir / "duplicate.sct";
    write_synthetic_named_object_asset(duplicate_table_path, duplicate_memo_path, {
        {.objname = "dupButton", .name = "firstDup", .unique_id = {}, .properties = "Caption = \"First\"\r\n"},
        {.objname = "DUPBUTTON", .name = "secondDup", .unique_id = {}, .properties = "Caption = \"Second\"\r\n"}
    });
    const auto duplicate_result = copperfin::vfp::update_visual_object_property({
        .path = duplicate_table_path.string(),
        .record_index = 0U,
        .object_name = "dupbutton",
        .unique_id = {},
        .property_name = "Caption",
        .property_value = "\"Ambiguous\""
    });
    expect(!duplicate_result.ok, "#730: ambiguous object names should fail instead of editing an arbitrary row");
    expect(duplicate_result.error.find("ambiguous") != std::string::npos,
        "#730: ambiguous object-name failures should explain the ambiguity");

    fs::remove_all(temp_dir, ignored);
}

void test_update_visual_object_property_targets_selected_unique_id() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_uniqueid_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "uniqueid.scx";
    const fs::path memo_path = temp_dir / "uniqueid.sct";
    write_synthetic_named_object_asset(table_path, memo_path, {
        {
            .objname = "dupButton",
            .name = "firstDup",
            .unique_id = "first-guid",
            .properties = "Caption = \"First\"\r\nLeft = 10\r\n"
        },
        {
            .objname = "DUPBUTTON",
            .name = "secondDup",
            .unique_id = "target-guid",
            .properties = "Caption = \"Second\"\r\nLeft = 20\r\n"
        },
        {
            .objname = "txtName",
            .name = "nameBox",
            .unique_id = "other-guid",
            .properties = "Caption = \"Name\"\r\nLeft = 30\r\n"
        }
    });

    const auto update_result = copperfin::vfp::update_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "dupbutton",
        .unique_id = " TARGET-GUID ",
        .property_name = "Caption",
        .property_value = "\"ById\""
    });
    expect(update_result.ok, "#732: UNIQUEID-targeted edits should disambiguate duplicate object names");

    auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 3U);
    expect(parse_result.ok, "#732: UNIQUEID-targeted fixture should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 3U) {
        const auto* first_properties = find_record_field(parse_result.table.records[0], "PROPERTIES");
        const auto* second_properties = find_record_field(parse_result.table.records[1], "PROPERTIES");
        const auto* third_properties = find_record_field(parse_result.table.records[2], "PROPERTIES");
        expect(first_properties != nullptr &&
                first_properties->display_value.find("Caption = \"First\"") != std::string::npos,
            "#732: UNIQUEID-targeted edits should preserve duplicate-name non-target records");
        expect(second_properties != nullptr &&
                second_properties->display_value.find("Caption = \"ById\"") != std::string::npos,
            "#732: UNIQUEID-targeted edits should update the resolved object record");
        expect(third_properties != nullptr &&
                third_properties->display_value.find("Caption = \"Name\"") != std::string::npos,
            "#732: UNIQUEID-targeted edits should preserve unrelated object records");
    }

    const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#732: UNIQUEID-targeted undo should use the resolved record index");
    parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 3U);
    expect(parse_result.ok, "#732: UNIQUEID-targeted fixture should remain readable after undo");
    if (parse_result.ok && parse_result.table.records.size() == 3U) {
        const auto* second_properties = find_record_field(parse_result.table.records[1], "PROPERTIES");
        expect(second_properties != nullptr &&
                second_properties->display_value.find("Caption = \"Second\"") != std::string::npos,
            "#732: UNIQUEID-targeted undo should restore the resolved object's original value");
    }

    const auto missing_result = copperfin::vfp::update_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "missing-guid",
        .property_name = "Caption",
        .property_value = "\"Missing\""
    });
    expect(!missing_result.ok, "#732: missing UNIQUEID selectors should fail instead of editing by record index");
    expect(missing_result.error.find("unique id") != std::string::npos,
        "#732: missing UNIQUEID failures should name the selector type");

    const fs::path duplicate_table_path = temp_dir / "duplicate_uniqueid.scx";
    const fs::path duplicate_memo_path = temp_dir / "duplicate_uniqueid.sct";
    write_synthetic_named_object_asset(duplicate_table_path, duplicate_memo_path, {
        {
            .objname = "first",
            .name = "first",
            .unique_id = "duplicate-guid",
            .properties = "Caption = \"First\"\r\n"
        },
        {
            .objname = "second",
            .name = "second",
            .unique_id = "DUPLICATE-GUID",
            .properties = "Caption = \"Second\"\r\n"
        }
    });
    const auto duplicate_result = copperfin::vfp::update_visual_object_property({
        .path = duplicate_table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "duplicate-guid",
        .property_name = "Caption",
        .property_value = "\"Ambiguous\""
    });
    expect(!duplicate_result.ok, "#732: ambiguous UNIQUEID selectors should fail instead of editing an arbitrary row");
    expect(duplicate_result.error.find("ambiguous") != std::string::npos,
        "#732: ambiguous UNIQUEID failures should explain the ambiguity");

    fs::remove_all(temp_dir, ignored);
}

void test_update_visual_object_property_matches_property_names_case_insensitively() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_property_case_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path memo_table_path = temp_dir / "property_case.scx";
    const fs::path memo_path = temp_dir / "property_case.sct";
    write_synthetic_named_object_asset(memo_table_path, memo_path, {
        {
            .objname = "cmdSave",
            .name = "saveButton",
            .unique_id = {},
            .properties = "Caption = \"Save\"\r\nLeft = 10\r\n"
        }
    });
    const auto memo_update_result = copperfin::vfp::update_visual_object_property({
        .path = memo_table_path.string(),
        .record_index = 0U,
        .object_name = "CMDSAVE",
        .unique_id = {},
        .property_name = "caption",
        .property_value = "\"Lower\""
    });
    expect(memo_update_result.ok, "#734: memo-backed property names should match case-insensitively");

    auto parse_result = copperfin::vfp::parse_dbf_table_from_file(memo_table_path.string(), 1U);
    expect(parse_result.ok, "#734: case-insensitive memo property fixture should remain readable");
    if (parse_result.ok && !parse_result.table.records.empty()) {
        const auto* properties = find_record_field(parse_result.table.records[0], "PROPERTIES");
        expect(properties != nullptr &&
                properties->display_value.find("Caption = \"Lower\"") != std::string::npos,
            "#734: memo property updates should preserve existing property-name casing");
        expect(properties != nullptr &&
                properties->display_value.find("caption = \"Lower\"") == std::string::npos,
            "#734: memo property updates should not append duplicate lower-case properties");
    }

    auto undo_result = copperfin::vfp::undo_visual_object_property(memo_table_path.string());
    expect(undo_result.ok, "#734: case-insensitive memo property undo should restore the original value");
    parse_result = copperfin::vfp::parse_dbf_table_from_file(memo_table_path.string(), 1U);
    expect(parse_result.ok, "#734: case-insensitive memo property fixture should remain readable after undo");
    if (parse_result.ok && !parse_result.table.records.empty()) {
        const auto* properties = find_record_field(parse_result.table.records[0], "PROPERTIES");
        expect(properties != nullptr &&
                properties->display_value.find("Caption = \"Save\"") != std::string::npos,
            "#734: case-insensitive memo property undo should resolve the original property name");
    }

    const fs::path direct_table_path = temp_dir / "property_case_direct.scx";
    const fs::path direct_memo_path = temp_dir / "property_case_direct.sct";
    write_synthetic_named_direct_asset(direct_table_path, direct_memo_path);
    const auto direct_update_result = copperfin::vfp::update_visual_object_property({
        .path = direct_table_path.string(),
        .record_index = 0U,
        .object_name = "TXTNAME",
        .unique_id = {},
        .property_name = "hpos",
        .property_value = "444.000"
    });
    expect(direct_update_result.ok, "#734: direct DBF-field property names should match case-insensitively");

    parse_result = copperfin::vfp::parse_dbf_table_from_file(direct_table_path.string(), 2U);
    expect(parse_result.ok, "#734: case-insensitive direct-field fixture should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 2U) {
        const auto* second_hpos = find_record_field(parse_result.table.records[1], "HPOS");
        expect(second_hpos != nullptr &&
                std::abs(parse_number(second_hpos->display_value) - 444.0) < 0.001,
            "#734: lower-case direct-field edits should update the resolved field");
    }

    undo_result = copperfin::vfp::undo_visual_object_property(direct_table_path.string());
    expect(undo_result.ok, "#734: case-insensitive direct-field undo should restore the original value");
    parse_result = copperfin::vfp::parse_dbf_table_from_file(direct_table_path.string(), 2U);
    expect(parse_result.ok, "#734: case-insensitive direct-field fixture should remain readable after undo");
    if (parse_result.ok && parse_result.table.records.size() == 2U) {
        const auto* second_hpos = find_record_field(parse_result.table.records[1], "HPOS");
        expect(second_hpos != nullptr &&
                std::abs(parse_number(second_hpos->display_value) - 222.0) < 0.001,
            "#734: case-insensitive direct-field undo should restore the resolved field");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_update_visual_object_property_targets_selected_object_name_direct_field() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_named_direct_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "named_direct.scx";
    const fs::path memo_path = temp_dir / "named_direct.sct";
    write_synthetic_named_direct_asset(table_path, memo_path);

    const auto update_result = copperfin::vfp::update_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "TXTNAME",
        .unique_id = {},
        .property_name = "HPOS",
        .property_value = "333.000"
    });
    expect(update_result.ok, "#731: object-name-targeted edits should update direct DBF fields on the selected object");

    auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
    expect(parse_result.ok, "#731: name-targeted direct-field fixture should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 2U) {
        const auto first_hpos = std::find_if(
            parse_result.table.records[0].values.begin(),
            parse_result.table.records[0].values.end(),
            [](const auto& value) {
                return value.field_name == "HPOS";
            });
        const auto second_hpos = std::find_if(
            parse_result.table.records[1].values.begin(),
            parse_result.table.records[1].values.end(),
            [](const auto& value) {
                return value.field_name == "HPOS";
            });
        expect(first_hpos != parse_result.table.records[0].values.end() &&
                std::abs(parse_number(first_hpos->display_value) - 111.0) < 0.001,
            "#731: direct-field selected-object edits should preserve unrelated object values");
        expect(second_hpos != parse_result.table.records[1].values.end() &&
                std::abs(parse_number(second_hpos->display_value) - 333.0) < 0.001,
            "#731: direct-field selected-object edits should update the resolved object record");
    }

    const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#731: direct-field selected-object undo should use the resolved record index");
    parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
    expect(parse_result.ok, "#731: direct-field selected-object fixture should remain readable after undo");
    if (parse_result.ok && parse_result.table.records.size() == 2U) {
        const auto second_hpos = std::find_if(
            parse_result.table.records[1].values.begin(),
            parse_result.table.records[1].values.end(),
            [](const auto& value) {
                return value.field_name == "HPOS";
            });
        expect(second_hpos != parse_result.table.records[1].values.end() &&
                std::abs(parse_number(second_hpos->display_value) - 222.0) < 0.001,
            "#731: direct-field selected-object undo should restore the resolved object's original value");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_update_visual_object_property_rewrites_direct_fields() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_direct_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "sample.frx";
    const fs::path memo_path = temp_dir / "sample.frt";

    std::vector<std::uint8_t> table_bytes(178U, 0U);
    table_bytes[0] = 0x30U;
    table_bytes[1] = 126U;
    table_bytes[2] = 4U;
    table_bytes[3] = 7U;
    write_le_u32(table_bytes, 4U, 1U);
    write_le_u16(table_bytes, 8U, 161U);
    write_le_u16(table_bytes, 10U, 17U);
    table_bytes[28] = 0x00U;
    table_bytes[29] = 0x03U;

    write_field_descriptor(table_bytes, 32U, "OBJTYPE", 'N', 1U, 2U);
    write_field_descriptor(table_bytes, 64U, "HPOS", 'N', 3U, 9U);
    write_field_descriptor(table_bytes, 96U, "GRID", 'L', 12U, 1U);
    write_field_descriptor(table_bytes, 128U, "EXPR", 'M', 13U, 4U);
    table_bytes[160] = 0x0DU;
    table_bytes[161] = 0x20U;
    write_ascii(table_bytes, 162U, " 8");
    write_ascii(table_bytes, 164U, "   7812.5");
    table_bytes[173] = 'F';
    write_le_u32(table_bytes, 174U, 1U);

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }

    std::vector<std::uint8_t> memo_bytes(1024U, 0U);
    write_be_u32(memo_bytes, 0U, 2U);
    write_be_u16(memo_bytes, 6U, 512U);
    const std::string expr = "customer.company";
    memo_bytes[512 + 3] = 1U;
    write_be_u32(memo_bytes, 512 + 4, static_cast<std::uint32_t>(expr.size()));
    write_ascii(memo_bytes, 520U, expr);

    {
        std::ofstream output(memo_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }

    auto update_result = copperfin::vfp::update_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "HPOS",
        .property_value = "9583.333"
    });
    expect(update_result.ok, "numeric FRX field update should succeed");

    update_result = copperfin::vfp::update_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "GRID",
        .property_value = "true"
    });
    expect(update_result.ok, "logical FRX field update should succeed");

    update_result = copperfin::vfp::update_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "EXPR",
        .property_value = "\"newexpr\""
    });
    expect(update_result.ok, "memo FRX field update should succeed");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
    expect(parse_result.ok, "updated synthetic FRX/FRT should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 1U) {
        const auto& record = parse_result.table.records[0];
        for (const auto& value : record.values) {
            if (value.field_name == "HPOS") {
                expect(value.display_value == "9583.333", "updated HPOS should be reflected in the parsed table");
            }
            if (value.field_name == "GRID") {
                expect(value.display_value == "true", "updated GRID should be reflected in the parsed table");
            }
            if (value.field_name == "EXPR") {
                expect(value.display_value == "\"newexpr\"", "updated EXPR memo should be reflected in the parsed table");
            }
        }
    }

    auto undo_status = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_status.available, "direct-field asset edits should expose an undo entry");
    expect(undo_status.label.find("EXPR") != std::string::npos, "latest undo label should name the latest edited property");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "undo should revert the latest direct-field or memo-backed report edit");
    auto after_first_undo = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
    expect(after_first_undo.ok, "asset should remain readable after the first undo");
    if (after_first_undo.ok && after_first_undo.table.records.size() == 1U) {
        const auto& record = after_first_undo.table.records[0];
        for (const auto& value : record.values) {
            if (value.field_name == "EXPR") {
                expect(value.display_value == "customer.company", "first undo should restore the original memo-backed EXPR value");
            }
            if (value.field_name == "GRID") {
                expect(value.display_value == "true", "first undo should leave earlier direct-field edits intact");
            }
        }
    }

    undo_status = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_status.available, "older undo entries should remain after undoing the latest edit");
    expect(undo_status.label.find("GRID") != std::string::npos, "undo label should walk back to the next-most-recent property");

    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "second undo should revert the logical field edit");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "third undo should revert the numeric field edit");

    const auto reverted_parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
    expect(reverted_parse_result.ok, "asset should remain readable after all direct-field undos");
    if (reverted_parse_result.ok && reverted_parse_result.table.records.size() == 1U) {
        const auto& record = reverted_parse_result.table.records[0];
        for (const auto& value : record.values) {
            if (value.field_name == "HPOS") {
                expect(std::fabs(parse_number(value.display_value) - 7812.5) < 0.0001,
                    "full undo should restore the original HPOS numerically");
            }
            if (value.field_name == "GRID") {
                expect(value.display_value == "false", "full undo should restore the original GRID logical value");
            }
            if (value.field_name == "EXPR") {
                expect(value.display_value == "customer.company", "full undo should preserve the original EXPR");
            }
        }
    }

    fs::remove_all(temp_dir, ignored);
}

void test_update_visual_object_property_round_trips_added_vcx_property() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_vcx_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "sample.vcx";
    const fs::path memo_path = temp_dir / "sample.vct";

    std::vector<std::uint8_t> table_bytes(110U, 0U);
    table_bytes[0] = 0x30U;
    table_bytes[1] = 126U;
    table_bytes[2] = 4U;
    table_bytes[3] = 7U;
    write_le_u32(table_bytes, 4U, 1U);
    write_le_u16(table_bytes, 8U, 97U);
    write_le_u16(table_bytes, 10U, 13U);
    table_bytes[28] = 0x00U;
    table_bytes[29] = 0x03U;

    write_field_descriptor(table_bytes, 32U, "OBJNAME", 'M', 1U, 4U);
    write_field_descriptor(table_bytes, 64U, "PROPERTIES", 'M', 5U, 4U);
    table_bytes[96] = 0x0DU;

    table_bytes[97] = 0x20U;
    write_le_u32(table_bytes, 98U, 1U);
    write_le_u32(table_bytes, 102U, 2U);

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }

    std::vector<std::uint8_t> memo_bytes(2048U, 0U);
    write_be_u32(memo_bytes, 0U, 3U);
    write_be_u16(memo_bytes, 6U, 512U);

    memo_bytes[512 + 3] = 1U;
    write_be_u32(memo_bytes, 512 + 4, 11U);
    write_ascii(memo_bytes, 520U, "clsCustomer");

    const std::string properties =
        "Name = \"clsCustomer\"\r\n"
        "Class = \"Custom\"\r\n";
    memo_bytes[1024 + 3] = 1U;
    write_be_u32(memo_bytes, 1024 + 4, static_cast<std::uint32_t>(properties.size()));
    write_ascii(memo_bytes, 1032U, properties);

    {
        std::ofstream output(memo_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }

    const auto update_result = copperfin::vfp::update_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "Caption",
        .property_value = "\"Customer Class\""
    });
    expect(update_result.ok, "adding a new VCX property should succeed");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
    expect(parse_result.ok, "updated synthetic VCX/VCT should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 1U) {
        const auto& record = parse_result.table.records[0];
        bool found_properties = false;
        for (const auto& value : record.values) {
            if (value.field_name == "PROPERTIES") {
                found_properties = true;
                expect(value.display_value.find("Name = \"clsCustomer\"") != std::string::npos,
                    "VCX round-trip should preserve existing serialized properties");
                expect(value.display_value.find("Caption = \"Customer Class\"") != std::string::npos,
                    "VCX round-trip should append the new serialized property");
            }
        }
        expect(found_properties, "updated VCX record should still expose the PROPERTIES field");
    }

    const auto undo_status = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_status.available, "VCX property addition should create an undo entry");
    expect(undo_status.label.find("Caption") != std::string::npos,
        "VCX undo label should name the added property");

    const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "undo should remove an added VCX property cleanly");

    const auto reverted_parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
    expect(reverted_parse_result.ok, "reverted synthetic VCX/VCT should remain readable");
    if (reverted_parse_result.ok && reverted_parse_result.table.records.size() == 1U) {
        const auto& record = reverted_parse_result.table.records[0];
        for (const auto& value : record.values) {
            if (value.field_name == "PROPERTIES") {
                expect(value.display_value.find("Caption = \"Customer Class\"") == std::string::npos,
                    "VCX undo should remove the added property from the serialized blob");
                expect(value.display_value.find("Name = \"clsCustomer\"") != std::string::npos,
                    "VCX undo should preserve pre-existing serialized properties");
            }
        }
    }

    const auto empty_undo_status = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(!empty_undo_status.available, "VCX undo journal should be empty after undoing the only added property");

    fs::remove_all(temp_dir, ignored);
}

void test_update_visual_object_property_round_trips_label_and_menu_assets() {
    namespace fs = std::filesystem;
    const auto exercise_asset = [&](const std::string& stem,
                                    const std::string& table_extension,
                                    const std::string& memo_extension,
                                    const std::string& asset_label) {
        const fs::path temp_dir = fs::temp_directory_path() /
            ("copperfin_visual_editor_" + stem + "_tests_" + std::to_string(_getpid()));
        std::error_code ignored;
        fs::remove_all(temp_dir, ignored);
        fs::create_directories(temp_dir);

        const fs::path table_path = temp_dir / ("sample" + table_extension);
        const fs::path memo_path = temp_dir / ("sample" + memo_extension);
        write_synthetic_direct_and_memo_asset(
            table_path,
            memo_path,
            "TITLE",
            asset_label + "Title",
            "EXPR",
            asset_label + "Expr");

        auto update_result = copperfin::vfp::update_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = {},
            .property_name = "TITLE",
            .property_value = asset_label + "Updated"
        });
        expect(update_result.ok, asset_label + " direct-field update should succeed");

        update_result = copperfin::vfp::update_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR",
            .property_value = "\"" + asset_label + "MemoUpdated\""
        });
        expect(update_result.ok, asset_label + " memo-field update should succeed");

        const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
        expect(parse_result.ok, asset_label + " asset should remain readable after updates");
        if (parse_result.ok && parse_result.table.records.size() == 1U) {
            const auto& record = parse_result.table.records[0];
            for (const auto& value : record.values) {
                if (value.field_name == "TITLE") {
                    expect(value.display_value == asset_label + "Updated",
                        asset_label + " direct-field value should round-trip");
                }
                if (value.field_name == "EXPR") {
                    expect(value.display_value == "\"" + asset_label + "MemoUpdated\"",
                        asset_label + " memo-field value should round-trip");
                }
            }
        }

        auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, asset_label + " first undo should restore the memo field");
        undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, asset_label + " second undo should restore the direct field");

        const auto reverted_parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
        expect(reverted_parse_result.ok, asset_label + " asset should remain readable after undo");
        if (reverted_parse_result.ok && reverted_parse_result.table.records.size() == 1U) {
            const auto& record = reverted_parse_result.table.records[0];
            for (const auto& value : record.values) {
                if (value.field_name == "TITLE") {
                    expect(value.display_value == asset_label + "Title",
                        asset_label + " undo should restore the original direct-field value");
                }
                if (value.field_name == "EXPR") {
                    expect(value.display_value == asset_label + "Expr",
                        asset_label + " undo should restore the original memo-field value");
                }
            }
        }

        const auto undo_status = copperfin::vfp::query_visual_object_undo(table_path.string());
        expect(!undo_status.available, asset_label + " undo journal should be empty after both undos");

        fs::remove_all(temp_dir, ignored);
    };

    exercise_asset("lbx", ".lbx", ".lbt", "Label");
    exercise_asset("mnx", ".mnx", ".mnt", "Menu");
}

void test_update_visual_object_property_round_trips_project_and_database_assets() {
    namespace fs = std::filesystem;
    const auto exercise_asset = [&](const std::string& stem,
                                    const std::string& table_extension,
                                    const std::string& memo_extension,
                                    const std::string& asset_label) {
        const fs::path temp_dir = fs::temp_directory_path() /
            ("copperfin_visual_editor_" + stem + "_tests_" + std::to_string(_getpid()));
        std::error_code ignored;
        fs::remove_all(temp_dir, ignored);
        fs::create_directories(temp_dir);

        const fs::path table_path = temp_dir / ("sample" + table_extension);
        const fs::path memo_path = temp_dir / ("sample" + memo_extension);
        write_synthetic_direct_and_memo_asset(
            table_path,
            memo_path,
            "TITLE",
            asset_label + "Title",
            "DETAILS",
            asset_label + "Details");

        auto update_result = copperfin::vfp::update_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = {},
            .property_name = "TITLE",
            .property_value = asset_label + "Updated"
        });
        expect(update_result.ok, asset_label + " direct-field update should succeed");

        update_result = copperfin::vfp::update_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = {},
            .property_name = "DETAILS",
            .property_value = asset_label + "MemoUpdated"
        });
        expect(update_result.ok, asset_label + " memo-field update should succeed");

        const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
        expect(parse_result.ok, asset_label + " asset should remain readable after updates");
        if (parse_result.ok && parse_result.table.records.size() == 1U) {
            const auto& record = parse_result.table.records[0];
            for (const auto& value : record.values) {
                if (value.field_name == "TITLE") {
                    expect(value.display_value == asset_label + "Updated",
                        asset_label + " direct-field value should round-trip");
                }
                if (value.field_name == "DETAILS") {
                    expect(value.display_value == asset_label + "MemoUpdated",
                        asset_label + " memo-field value should round-trip");
                }
            }
        }

        auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, asset_label + " first undo should restore the memo field");
        undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, asset_label + " second undo should restore the direct field");

        const auto reverted_parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
        expect(reverted_parse_result.ok, asset_label + " asset should remain readable after undo");
        if (reverted_parse_result.ok && reverted_parse_result.table.records.size() == 1U) {
            const auto& record = reverted_parse_result.table.records[0];
            for (const auto& value : record.values) {
                if (value.field_name == "TITLE") {
                    expect(value.display_value == asset_label + "Title",
                        asset_label + " undo should restore the original direct-field value");
                }
                if (value.field_name == "DETAILS") {
                    expect(value.display_value == asset_label + "Details",
                        asset_label + " undo should restore the original memo-field value");
                }
            }
        }

        const auto undo_status = copperfin::vfp::query_visual_object_undo(table_path.string());
        expect(!undo_status.available, asset_label + " undo journal should be empty after both undos");

        fs::remove_all(temp_dir, ignored);
    };

    exercise_asset("pjx", ".pjx", ".pjt", "Project");
    exercise_asset("dbc", ".dbc", ".dct", "Database");
}

}  // namespace cf_test_visual_asset_editor
