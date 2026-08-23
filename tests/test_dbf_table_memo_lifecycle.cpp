// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_dbf_table_support.h"

#include "copperfin/vfp/dbf_table.h"

#include <filesystem>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#define _getpid getpid
#endif

namespace copperfin::test_dbf_table {

void test_memo_field_create_replace_and_append_round_trip() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_memo_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "notes.dbf";
    const fs::path memo_path = temp_dir / "notes.fpt";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "TITLE", .type = 'C', .length = 12U},
        {.name = "BODY", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"FIRST", "First memo body"},
        {"SECOND", "Second memo body"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "create_dbf_table_file should support memo-backed schemas");
    expect(fs::exists(memo_path), "memo-backed table creation should also create the FPT sidecar");

    auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "memo-backed created tables should remain readable");
    expect(parse_result.table.records.size() == 2U, "memo-backed created tables should persist record rows");
    if (parse_result.table.records.size() == 2U) {
        expect(parse_result.table.records[0].values[0].display_value == "FIRST", "memo-backed created character fields should persist");
        expect(parse_result.table.records[0].values[1].display_value == "First memo body", "memo-backed created memo fields should round-trip through the sidecar");
        expect(parse_result.table.records[0].values[1].memo_block_number != 0U,
               "#711: created memo rows should retain non-zero memo block provenance");
        expect(parse_result.table.records[1].values[1].display_value == "Second memo body", "later memo rows should also round-trip");
        expect(parse_result.table.records[1].values[1].memo_block_number != 0U,
               "#711: later created memo rows should retain non-zero memo block provenance");
    }

    const auto replace_result = copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "BODY", "Updated first memo");
    expect(replace_result.ok, "replace_record_field_value should support memo-backed fields");

    const auto append_result = copperfin::vfp::append_blank_record_to_file(table_path.string());
    expect(append_result.ok, "append_blank_record_to_file should support memo-backed tables");
    expect(append_result.record_count == 3U, "memo-backed append_blank_record_to_file should grow the record count");

    parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "memo-backed mutated tables should remain readable");
    expect(parse_result.table.records.size() == 3U, "memo-backed tables should expose appended records");
    if (parse_result.table.records.size() == 3U) {
        expect(parse_result.table.records[0].values[1].display_value == "Updated first memo", "memo field replacements should persist through the shared DBF layer");
        expect(parse_result.table.records[2].values[0].display_value.empty(), "blank appended character fields in memo-backed tables should start empty");
        expect(parse_result.table.records[2].values[1].display_value.empty(), "blank appended memo fields should start with an empty pointer");
        expect(parse_result.table.records[2].values[1].memo_block_number == 0U,
               "#711: blank appended memo fields should expose memo block zero");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_general_and_picture_memo_fields_round_trip() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_gp_memo_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "assets.dbf";
    const fs::path memo_path = temp_dir / "assets.fpt";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "TITLE", .type = 'C', .length = 12U},
        {.name = "GENERAL", .type = 'G', .length = 4U},
        {.name = "PICTURE", .type = 'P', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"FIRST", "General payload", "Picture payload"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "create_dbf_table_file should support G/P memo-pointer fields");
    expect(fs::exists(memo_path), "G/P-backed table creation should also create the FPT sidecar");

    auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "G/P-backed tables should remain readable after creation");
    if (parse_result.ok && parse_result.table.records.size() == 1U && parse_result.table.records[0].values.size() >= 3U) {
        expect(parse_result.table.records[0].values[1].display_value == "General payload", "created G fields should round-trip through memo sidecar storage");
        expect(parse_result.table.records[0].values[2].display_value == "Picture payload", "created P fields should round-trip through memo sidecar storage");
    }

    const auto replace_general = copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "GENERAL", "Updated general payload");
    expect(replace_general.ok, "replace_record_field_value should support G fields");

    const auto replace_picture = copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "PICTURE", "Updated picture payload");
    expect(replace_picture.ok, "replace_record_field_value should support P fields");

    const auto append_result = copperfin::vfp::append_blank_record_to_file(table_path.string());
    expect(append_result.ok, "append_blank_record_to_file should support G/P-backed tables");
    expect(append_result.record_count == 2U, "G/P-backed append_blank_record_to_file should grow the record count");

    parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "G/P-backed tables should remain readable after mutation");
    expect(parse_result.table.records.size() == 2U, "G/P-backed tables should expose appended rows");
    if (parse_result.table.records.size() == 2U) {
        expect(parse_result.table.records[0].values[1].display_value == "Updated general payload", "G field replacements should persist");
        expect(parse_result.table.records[0].values[2].display_value == "Updated picture payload", "P field replacements should persist");
        expect(parse_result.table.records[1].values[1].display_value.empty(), "blank appended G fields should start with an empty pointer");
        expect(parse_result.table.records[1].values[2].display_value.empty(), "blank appended P fields should start with an empty pointer");
    }

    fs::remove_all(temp_dir, ignored);
}

}  // namespace copperfin::test_dbf_table
