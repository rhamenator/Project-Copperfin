// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/localization/localization.h"
#include "copperfin/vfp/dbf_table.h"
#include "test_dbf_table_support.h"
#include "test_environment_support.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#define _getpid getpid
#endif
#include <locale>
#include <string>
#include <string_view>
#include <vector>

namespace copperfin::test_dbf_table {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

}  // namespace copperfin::test_dbf_table

namespace {

using copperfin::test_support::ScopedEnvironmentValue;
using copperfin::test_dbf_table::expect;
using copperfin::test_dbf_table::failures;
using copperfin::test_dbf_table::read_be_u16;
using copperfin::test_dbf_table::read_binary_file;
using copperfin::test_dbf_table::replace_memo_block_payload;
using copperfin::test_dbf_table::write_ascii;
using copperfin::test_dbf_table::write_be_u16;
using copperfin::test_dbf_table::write_be_u32;
using copperfin::test_dbf_table::write_binary_file;
using copperfin::test_dbf_table::write_field_descriptor;
using copperfin::test_dbf_table::write_le_u16;
using copperfin::test_dbf_table::write_le_u32;

class comma_decimal_numpunct final : public std::numpunct<char> {
protected:
    char do_decimal_point() const override { return ','; }
    char do_thousands_sep() const override { return '.'; }
    std::string do_grouping() const override { return "\3"; }
};

class scoped_global_locale {
public:
    explicit scoped_global_locale(const std::locale& replacement)
        : previous_(std::locale::global(replacement)) {}

    ~scoped_global_locale() { std::locale::global(previous_); }

    scoped_global_locale(const scoped_global_locale&) = delete;
    scoped_global_locale& operator=(const scoped_global_locale&) = delete;

private:
    std::locale previous_;
};

std::size_t count_missing_locale_keys(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view locale,
    const std::vector<std::string_view>& keys) {
    const auto locale_entries = catalog.catalogs.find(std::string(locale));
    if (locale_entries == catalog.catalogs.end()) {
        return keys.size();
    }

    std::size_t missing = 0U;
    for (const auto key : keys) {
        if (locale_entries->second.find(std::string(key)) == locale_entries->second.end()) {
            ++missing;
        }
    }
    return missing;
}

void clear_dbf_last_update_date(const std::filesystem::path& path) {
    std::vector<std::uint8_t> bytes = read_binary_file(path);
    expect(bytes.size() >= 4U, "DBF date-stamping fixture should contain a complete header");
    if (bytes.size() >= 4U) {
        bytes[1U] = 0U;
        bytes[2U] = 0U;
        bytes[3U] = 0U;
        expect(write_binary_file(path, bytes), "DBF date-stamping fixture should be writable");
    }
}

void expect_dbf_last_update_date(const std::filesystem::path& path, const std::string& label) {
    const auto result = copperfin::vfp::parse_dbf_header_from_file(path.string());
    expect(result.ok, label + " should leave a readable DBF header");
    if (result.ok) {
        expect(result.header.last_update_year != 0U &&
                   result.header.last_update_month >= 1U && result.header.last_update_month <= 12U &&
                   result.header.last_update_day >= 1U && result.header.last_update_day <= 31U,
               label + " should stamp a valid DBF last-update header date");
    }
}

std::vector<std::uint8_t> read_record_field_bytes(
    const std::filesystem::path& table_path,
    std::size_t record_index,
    const std::string& field_name) {
    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), record_index + 1U);
    if (!parse_result.ok || record_index >= parse_result.table.records.size()) {
        return {};
    }
    const auto field = std::find_if(
        parse_result.table.fields.begin(),
        parse_result.table.fields.end(),
        [&](const copperfin::vfp::DbfFieldDescriptor& candidate) {
            return candidate.name == field_name;
        });
    if (field == parse_result.table.fields.end()) {
        return {};
    }

    const std::vector<std::uint8_t> bytes = read_binary_file(table_path);
    const std::size_t field_offset =
        parse_result.table.header.header_length +
        (record_index * parse_result.table.header.record_length) +
        field->offset;
    if (field_offset + field->length > bytes.size()) {
        return {};
    }
    return {
        bytes.begin() + static_cast<std::ptrdiff_t>(field_offset),
        bytes.begin() + static_cast<std::ptrdiff_t>(field_offset + field->length)
    };
}

void test_parse_dbf_table_with_memo_sidecar() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_tests_" + std::to_string(_getpid()));
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
    write_field_descriptor(table_bytes, 64U, "OBJTYPE", 'C', 5U, 8U);
    table_bytes[96] = 0x0DU;

    table_bytes[97] = 0x20U;
    write_le_u32(table_bytes, 98U, 1U);
    write_ascii(table_bytes, 102U, "Textbox ");

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }

    std::vector<std::uint8_t> memo_bytes(1024U, 0U);
    write_be_u32(memo_bytes, 0U, 2U);
    write_be_u16(memo_bytes, 6U, 512U);
    memo_bytes[512] = 0U;
    memo_bytes[513] = 0U;
    memo_bytes[514] = 0U;
    memo_bytes[515] = 1U;
    write_be_u32(memo_bytes, 516U, 9U);
    write_ascii(memo_bytes, 520U, "txtTitle1");

    {
        std::ofstream output(memo_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }

    const auto result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(result.ok, "parse_dbf_table_from_file should succeed for a synthetic SCX/SCT pair");
    expect(result.table.fields.size() == 2U, "field descriptors should be parsed");
    expect(result.table.records.size() == 1U, "one record should be parsed");
    if (result.table.records.size() == 1U && result.table.records[0].values.size() >= 2U) {
        expect(result.table.records[0].values[0].display_value == "txtTitle1", "memo values should be decoded from the sidecar");
        expect(result.table.records[0].values[0].memo_block_number == 1U,
               "#711: decoded memo values should retain the stored memo block number");
        expect(result.table.records[0].values[1].display_value == "Textbox", "character values should be trimmed");
        expect(result.table.records[0].values[1].memo_block_number == 0U,
               "#711: non-memo fields should expose memo block zero");
    }

    fs::remove(memo_path, ignored);
    const auto missing_sidecar_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(missing_sidecar_result.ok, "#711: DBF parsing should still succeed when a memo sidecar is missing");
    if (missing_sidecar_result.ok &&
        missing_sidecar_result.table.records.size() == 1U &&
        !missing_sidecar_result.table.records[0].values.empty()) {
        expect(missing_sidecar_result.table.records[0].values[0].display_value == "<memo block 1>",
               "#711: unresolved memo blocks should keep the existing placeholder display text");
        expect(missing_sidecar_result.table.records[0].values[0].memo_block_number == 1U,
               "#711: unresolved memo blocks should retain structured block provenance");
    }

    fs::remove(table_path, ignored);
    fs::remove(temp_dir, ignored);
}

void test_mutate_and_append_dbf_table() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_write_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "people.dbf";
    std::vector<std::uint8_t> table_bytes(97U + (2U * 14U) + 1U, 0U);
    table_bytes[0] = 0x30U;
    write_le_u32(table_bytes, 4U, 2U);
    write_le_u16(table_bytes, 8U, 97U);
    write_le_u16(table_bytes, 10U, 14U);

    write_field_descriptor(table_bytes, 32U, "NAME", 'C', 1U, 10U);
    write_field_descriptor(table_bytes, 64U, "AGE", 'N', 11U, 3U);
    table_bytes[96U] = 0x0DU;

    table_bytes[97U] = 0x20U;
    write_ascii(table_bytes, 98U, "ALPHA     ");
    write_ascii(table_bytes, 108U, " 10");

    table_bytes[111U] = 0x20U;
    write_ascii(table_bytes, 112U, "BRAVO     ");
    write_ascii(table_bytes, 122U, " 20");
    table_bytes.back() = 0x1AU;

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }

    const auto replace_name = copperfin::vfp::replace_record_field_value(table_path.string(), 1U, "NAME", "BRAVOX");
    expect(replace_name.ok, "replace_record_field_value should update character fields");

    const auto replace_age = copperfin::vfp::replace_record_field_value(table_path.string(), 1U, "AGE", "21");
    expect(replace_age.ok, "replace_record_field_value should update numeric fields");

    const auto append = copperfin::vfp::append_blank_record_to_file(table_path.string());
    expect(append.ok, "append_blank_record_to_file should append a new row");
    expect(append.record_count == 3U, "append_blank_record_to_file should grow the record count");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(table_path.string(), 2U, true);
    expect(delete_result.ok, "set_record_deleted_flag should tombstone records");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "the mutated DBF should remain readable");
    expect(parse_result.table.records.size() == 3U, "mutated DBF should expose the appended record");
    if (parse_result.table.records.size() == 3U) {
        expect(parse_result.table.records[1].values[0].display_value == "BRAVOX", "character updates should persist");
        expect(parse_result.table.records[1].values[1].display_value == "21", "numeric updates should persist");
        expect(parse_result.table.records[2].deleted, "deleted-flag updates should persist");
        expect(parse_result.table.records[2].values[0].display_value.empty(), "blank appended character fields should start empty");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_create_dbf_table_file_round_trips() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_create_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "totals.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "REGION", .type = 'C', .length = 10U},
        {.name = "AMOUNT", .type = 'N', .length = 6U},
        {.name = "QTY", .type = 'N', .length = 3U}
    };
    const std::vector<std::vector<std::string>> records{
        {"EAST", "25", "3"},
        {"WEST", "8", "4"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "create_dbf_table_file should write a new DBF table");
    expect(create_result.record_count == 2U, "create_dbf_table_file should report the written record count");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "created DBF tables should round-trip through the parser");
    expect(parse_result.ok && parse_result.table.header.last_update_year != 0U &&
               parse_result.table.header.last_update_month >= 1U &&
               parse_result.table.header.last_update_month <= 12U &&
               parse_result.table.header.last_update_day >= 1U &&
               parse_result.table.header.last_update_day <= 31U,
           "created DBF tables should stamp a valid last-update header date for LUPDATE()");
    expect(parse_result.table.fields.size() == 3U, "created DBF tables should persist field descriptors");
    expect(parse_result.table.records.size() == 2U, "created DBF tables should persist record rows");
    if (parse_result.table.records.size() == 2U) {
        expect(parse_result.table.records[0].values[0].display_value == "EAST", "created character fields should persist");
        expect(parse_result.table.records[0].values[1].display_value == "25", "created numeric totals should persist");
        expect(parse_result.table.records[1].values[0].display_value == "WEST", "later created rows should persist");
        expect(parse_result.table.records[1].values[2].display_value == "4", "created numeric fields should round-trip");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_dbf_mutations_stamp_last_update_date() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_update_date_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "updates.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U}
    };
    expect(copperfin::vfp::create_dbf_table_file(table_path.string(), fields, {{"ALPHA"}, {"BRAVO"}}).ok,
           "DBF date-stamping fixture should be created");

    clear_dbf_last_update_date(table_path);
    expect(copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "NAME", "OMEGA").ok,
           "record replacement should succeed for DBF date-stamping coverage");
    expect_dbf_last_update_date(table_path, "record replacement");

    clear_dbf_last_update_date(table_path);
    expect(copperfin::vfp::append_blank_record_to_file(table_path.string()).ok,
           "append should succeed for DBF date-stamping coverage");
    expect_dbf_last_update_date(table_path, "record append");

    clear_dbf_last_update_date(table_path);
    expect(copperfin::vfp::set_record_deleted_flag(table_path.string(), 1U, true).ok,
           "delete flag update should succeed for DBF date-stamping coverage");
    expect_dbf_last_update_date(table_path, "delete flag update");

    clear_dbf_last_update_date(table_path);
    expect(copperfin::vfp::truncate_dbf_table_file(table_path.string(), 2U).ok,
           "truncate should succeed for DBF date-stamping coverage");
    expect_dbf_last_update_date(table_path, "truncate");

    clear_dbf_last_update_date(table_path);
    expect(copperfin::vfp::pack_dbf_table_file(table_path.string()).ok,
           "pack should succeed for DBF date-stamping coverage");
    expect_dbf_last_update_date(table_path, "pack");

    clear_dbf_last_update_date(table_path);
    expect(copperfin::vfp::add_dbf_table_field(
               table_path.string(), {.name = "STATUS", .type = 'C', .length = 1U}).ok,
           "schema rewrite should succeed for DBF date-stamping coverage");
    expect_dbf_last_update_date(table_path, "schema rewrite");

    clear_dbf_last_update_date(table_path);
    expect(copperfin::vfp::zap_dbf_table_file(table_path.string()).ok,
           "zap should succeed for DBF date-stamping coverage");
    expect_dbf_last_update_date(table_path, "zap");

    fs::remove_all(temp_dir, ignored);
}

void test_character_and_varchar_fields_preserve_leading_whitespace_on_write() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_leading_space_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "leading_space.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "CVAL", .type = 'C', .length = 8U},
        {.name = "VVAL", .type = 'V', .length = 9U}
    };
    const std::vector<std::vector<std::string>> records{
        {" ALPHA", " V-ONE"},
        {" BRAVO", " V-TWO"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#3676: create_dbf_table_file should preserve leading whitespace in C/V fields");

    auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "#3676: DBF with leading-space C/V values should remain readable after creation");
    if (parse_result.ok && parse_result.table.records.size() == 2U && parse_result.table.records[0].values.size() >= 2U) {
        expect(parse_result.table.records[0].values[0].display_value == " ALPHA",
               "#3676: created C fields should preserve leading whitespace");
        expect(parse_result.table.records[0].values[1].display_value == " V-ONE",
               "#3676: created V fields should preserve leading whitespace");
    }

    expect(copperfin::vfp::replace_record_field_value(table_path.string(), 1U, "CVAL", " CHARLIE").ok,
           "#3676: replace_record_field_value should preserve leading whitespace in C fields");
    expect(copperfin::vfp::replace_record_field_value(table_path.string(), 1U, "VVAL", " V-THREE").ok,
           "#3676: replace_record_field_value should preserve leading whitespace in V fields");

    parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "#3676: DBF with leading-space mutations should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 2U && parse_result.table.records[1].values.size() >= 2U) {
        expect(parse_result.table.records[1].values[0].display_value == " CHARLIE",
               "#3676: replaced C fields should preserve leading whitespace");
        expect(parse_result.table.records[1].values[1].display_value == " V-THREE",
               "#3676: replaced V fields should preserve leading whitespace");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_string_fields_store_literal_null_text() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_literal_null_text_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "literal_null_text.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "CVAL", .type = 'C', .length = 8U},
        {.name = "VVAL", .type = 'V', .length = 8U},
        {.name = "QVAL", .type = 'Q', .length = 8U}
    };
    const std::vector<std::vector<std::string>> records{
        {"null", "NULL", " null "}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#3675: create_dbf_table_file should store literal null text in string-bearing fields");

    auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "#3675: DBF with literal null text should remain readable after creation");
    if (parse_result.ok && parse_result.table.records.size() == 1U && parse_result.table.records[0].values.size() >= 3U) {
        expect(parse_result.table.records[0].values[0].display_value == "null",
               "#3675: C fields should keep literal null text");
        expect(parse_result.table.records[0].values[1].display_value == "NULL",
               "#3675: V fields should keep literal null text");
        expect(parse_result.table.records[0].values[2].display_value == " null ",
               "#3675: Q fields should keep literal null text");
    }

    expect(copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "CVAL", "NULL").ok,
           "#3675: replace_record_field_value should preserve literal NULL in C fields");
    expect(copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "VVAL", "null").ok,
           "#3675: replace_record_field_value should preserve literal null in V fields");
    expect(copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "QVAL", "NULL").ok,
           "#3675: replace_record_field_value should preserve literal NULL in Q fields");

    parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "#3675: DBF with literal null text should remain readable after replacement");
    if (parse_result.ok && parse_result.table.records.size() == 1U && parse_result.table.records[0].values.size() >= 3U) {
        expect(parse_result.table.records[0].values[0].display_value == "NULL",
               "#3675: replaced C fields should keep literal NULL text");
        expect(parse_result.table.records[0].values[1].display_value == "null",
               "#3675: replaced V fields should keep literal null text");
        expect(parse_result.table.records[0].values[2].display_value == "NULL",
               "#3675: replaced Q fields should keep literal NULL text");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_character_fields_stop_at_nul_padding() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_nul_padding_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "nul_padding.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        fields,
        {{" path.scx"}});
    expect(create_result.ok, "setup should create a character field for NUL-padding coverage");

    auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "NUL-padded character table should initially be readable");
    if (!parse_result.ok || parse_result.table.records.empty() || parse_result.table.fields.empty()) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    const auto field = std::find_if(
        parse_result.table.fields.begin(),
        parse_result.table.fields.end(),
        [](const copperfin::vfp::DbfFieldDescriptor& candidate) {
            return candidate.name == "NAME";
        });
    expect(field != parse_result.table.fields.end(), "NUL-padding fixture should expose the NAME field");
    if (field == parse_result.table.fields.end()) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    auto bytes = read_binary_file(table_path);
    const std::size_t field_offset =
        parse_result.table.header.header_length + field->offset;
    expect(field_offset + field->length <= bytes.size(), "NUL-padding fixture should contain the complete NAME field");
    if (field_offset + field->length <= bytes.size()) {
        std::fill(
            bytes.begin() + static_cast<std::ptrdiff_t>(field_offset + 9U),
            bytes.begin() + static_cast<std::ptrdiff_t>(field_offset + field->length),
            static_cast<std::uint8_t>(0U));
        expect(write_binary_file(table_path, bytes), "NUL-padding fixture should be writable");
    }

    parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "NUL-padded character table should remain readable");
    if (parse_result.ok && !parse_result.table.records.empty() && !parse_result.table.records[0].values.empty()) {
        expect(parse_result.table.records[0].values[0].display_value == " path.scx",
               "character decoding should stop at NUL padding while preserving leading spaces");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_create_dbf_table_file_rejects_duplicate_field_names() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_duplicate_field_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "duplicate_fields.dbf";
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = " name ", .type = 'N', .length = 3U}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, {});
    expect(!create_result.ok, "#3678: create_dbf_table_file should reject duplicate field names");
    expect(
        create_result.error == english_catalog.translate("Vfp.DbfTable.Error.TargetFieldExists"),
        "#3678: duplicate create_dbf_table_file field names should reuse the standard TargetFieldExists error");
    expect(!fs::exists(table_path), "#3678: duplicate field-name rejection should not leave a partial DBF on disk");

    fs::remove_all(temp_dir, ignored);
}

void test_dbf_schema_writes_enforce_field_name_boundaries() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_serialized_field_collision_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const std::string duplicate_error =
        english_catalog.translate("Vfp.DbfTable.Error.TargetFieldExists");
    const auto too_long_error = [&](const std::string& field_name) {
        return english_catalog.translate(
            "Vfp.DbfTable.Error.FreeTableFieldNameTooLong",
            {{"fieldName", field_name}, {"maxBytes", "10"}});
    };

    const fs::path boundary_path = temp_dir / "vfp_boundary_fields.dbf";
    const auto boundary_result = copperfin::vfp::create_dbf_table_file(
        boundary_path.string(),
        {
            {.name = "ABCDEFGHIJ", .type = 'C', .length = 1U},
            {.name = "KLMNOPQRST", .type = 'C', .length = 1U},
        },
        {});
    expect(boundary_result.ok, "#4028: distinct 10-byte VFP9 field names should remain valid");
    const auto boundary_parse = copperfin::vfp::parse_dbf_table_from_file(boundary_path.string(), 0U);
    expect(boundary_parse.ok && boundary_parse.table.fields.size() == 2U &&
               boundary_parse.table.fields[0U].name == "ABCDEFGHIJ" &&
               boundary_parse.table.fields[1U].name == "KLMNOPQRST",
           "#4028: VFP9 boundary-length DBF field names should round trip exactly");

    const std::string code_page_boundary_name =
        std::string("ABCDEFGHI") + static_cast<char>(0xD1);
    const fs::path code_page_boundary_path = temp_dir / "code_page_boundary_field.dbf";
    const auto code_page_boundary_result = copperfin::vfp::create_dbf_table_file(
        code_page_boundary_path.string(),
        {{.name = code_page_boundary_name, .type = 'C', .length = 1U}},
        {});
    expect(code_page_boundary_result.ok,
           "#4034: a free-table field name containing exactly 10 code-page bytes should remain valid");
    const auto code_page_boundary_parse =
        copperfin::vfp::parse_dbf_table_from_file(code_page_boundary_path.string(), 0U);
    expect(code_page_boundary_parse.ok && code_page_boundary_parse.table.fields.size() == 1U &&
               code_page_boundary_parse.table.fields[0U].name == code_page_boundary_name,
           "#4034: the writer should preserve all 10 code-page bytes in a boundary-length name");

    const fs::path overlong_path = temp_dir / "overlong_fields.dbf";
    const auto overlong_result = copperfin::vfp::create_dbf_table_file(
        overlong_path.string(),
        {
            {.name = "ABCDEFGHIJK", .type = 'M', .length = 4U},
            {.name = "OTHER", .type = 'C', .length = 1U},
        },
        {});
    expect(!overlong_result.ok,
           "#4034: create_dbf_table_file should reject names beyond the VFP9 10-byte limit");
    expect(overlong_result.error == too_long_error("ABCDEFGHIJK"),
           "#4034: over-limit field names should use the localized byte-limit error");
    expect(!fs::exists(overlong_path) && !fs::exists(overlong_path.parent_path() / "overlong_fields.fpt"),
           "#4034: over-limit memo schemas should not create DBF or FPT output");
    expect(!fs::exists(overlong_path.string() + ".cptmp") &&
               !fs::exists(overlong_path.string() + ".cpbak"),
           "#4034: direct over-limit rejection should leave no staged DBF artifacts");

    const fs::path add_path = temp_dir / "add_collision.dbf";
    const auto add_fixture = copperfin::vfp::create_dbf_table_file(
        add_path.string(),
        {
            {.name = "ABCDEFGHIJ", .type = 'C', .length = 2U},
            {.name = "OTHER", .type = 'C', .length = 2U},
        },
        {{"A", "B"}});
    expect(add_fixture.ok, "#4028: ADD collision fixture should be created");
    const std::vector<std::uint8_t> add_bytes_before = read_binary_file(add_path);
    const auto add_result = copperfin::vfp::add_dbf_table_field(
        add_path.string(),
        {.name = "abcdefghij", .type = 'C', .length = 1U});
    expect(!add_result.ok && add_result.error == duplicate_error,
           "#4028: ADD should reject a field name that collides after serialization");
    const auto overlong_add_result = copperfin::vfp::add_dbf_table_field(
        add_path.string(),
        {.name = "ABCDEFGHIJK", .type = 'C', .length = 1U});
    expect(!overlong_add_result.ok &&
               overlong_add_result.error == too_long_error("ABCDEFGHIJK"),
           "#4034: ADD should reject an over-limit field before rewriting the table");
    expect(read_binary_file(add_path) == add_bytes_before,
           "#4028/#4034: rejected ADD names should preserve the original DBF bytes");
    expect(!fs::exists(add_path.string() + ".cptmp") &&
               !fs::exists(add_path.string() + ".cpbak"),
           "#4028: rejected ADD collisions should leave no staged write artifacts");

    fs::remove_all(temp_dir, ignored);
}

void test_record_field_updates_match_descriptor_names_case_insensitively() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_field_case_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "mixed_case.dbf";
    const fs::path memo_path = temp_dir / "mixed_case.fpt";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "CuStOmEr", .type = 'C', .length = 10U},
        {.name = "NoTeS", .type = 'M', .length = 4U}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(), fields, {{"ALPHA", "First"}});
    expect(create_result.ok, "#3984: mixed-case descriptor fixture should be created");

    const std::vector<std::uint8_t> original_table_bytes = read_binary_file(table_path);
    expect(copperfin::vfp::replace_record_field_value(
               table_path.string(), 0U, "customer", "BRAVO").ok,
           "#3984: lowercase API field names should match mixed-case descriptors");
    expect(copperfin::vfp::replace_record_field_value(
               table_path.string(), 0U, "CUSTOMER", "CHARLIE").ok,
           "#3984: uppercase API field names should match mixed-case descriptors");
    expect(copperfin::vfp::replace_record_field_value(
               table_path.string(), 0U, "cUsToMeR", "DELTA").ok,
           "#3984: mixed-case API field names should match differently cased descriptors");
    expect(copperfin::vfp::replace_record_field_value_additive(
               table_path.string(), 0U, "nOtEs", "-second").ok,
           "#3984: additive memo updates should match descriptor names case-insensitively");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
    expect(parse_result.ok && parse_result.table.fields.size() == 2U &&
               parse_result.table.records.size() == 1U,
           "#3984: case-insensitively updated table should remain readable");
    if (parse_result.ok && parse_result.table.fields.size() == 2U &&
        parse_result.table.records.size() == 1U) {
        expect(parse_result.table.fields[0U].name == "CuStOmEr" &&
                   parse_result.table.fields[1U].name == "NoTeS",
               "#3984: updates should preserve on-disk descriptor spelling");
        expect(parse_result.table.records[0U].values[0U].display_value == "DELTA" &&
                   parse_result.table.records[0U].values[1U].display_value == "First-second",
               "#3984: case-insensitive ordinary and additive updates should persist values");
    }

    const std::vector<std::uint8_t> updated_table_bytes = read_binary_file(table_path);
    const std::size_t descriptor_begin = 32U;
    const std::size_t descriptor_end = descriptor_begin + (fields.size() * 32U);
    expect(original_table_bytes.size() >= descriptor_end && updated_table_bytes.size() >= descriptor_end &&
               std::equal(
                   original_table_bytes.begin() + static_cast<std::ptrdiff_t>(descriptor_begin),
                   original_table_bytes.begin() + static_cast<std::ptrdiff_t>(descriptor_end),
                   updated_table_bytes.begin() + static_cast<std::ptrdiff_t>(descriptor_begin)),
           "#3984: field updates should leave descriptor bytes unchanged");

    const std::vector<std::uint8_t> table_before_unknown = read_binary_file(table_path);
    const std::vector<std::uint8_t> memo_before_unknown = read_binary_file(memo_path);
    const auto unknown_result = copperfin::vfp::replace_record_field_value(
        table_path.string(), 0U, "missing", "DELTA");
    const auto active_catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(),
        copperfin::localization::select_locale());
    expect(!unknown_result.ok &&
               unknown_result.error == active_catalog.translate(
                   "Vfp.DbfTable.Error.TargetFieldNotFoundInTable"),
           "#3984: unknown fields should retain the localized diagnostic contract");
    expect(read_binary_file(table_path) == table_before_unknown &&
               read_binary_file(memo_path) == memo_before_unknown,
           "#3984: unknown-field rejection should leave DBF/FPT bytes unchanged");

    const fs::path ambiguous_path = temp_dir / "ambiguous.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> ambiguous_fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "OTHER", .type = 'C', .length = 10U}
    };
    expect(copperfin::vfp::create_dbf_table_file(
               ambiguous_path.string(), ambiguous_fields, {{"ALPHA", "BRAVO"}}).ok,
           "#3984: ambiguous-descriptor fixture should start as a valid DBF");
    std::vector<std::uint8_t> ambiguous_bytes = read_binary_file(ambiguous_path);
    expect(ambiguous_bytes.size() >= 75U,
           "#3984: ambiguous-descriptor fixture should contain two complete descriptors");
    if (ambiguous_bytes.size() >= 75U) {
        std::fill(
            ambiguous_bytes.begin() + 64,
            ambiguous_bytes.begin() + 75,
            static_cast<std::uint8_t>(0U));
        write_ascii(ambiguous_bytes, 64U, "name");
        expect(write_binary_file(ambiguous_path, ambiguous_bytes),
               "#3984: fixture should install a case-fold-ambiguous descriptor name");
    }
    const std::vector<std::uint8_t> ambiguous_before_write = read_binary_file(ambiguous_path);
    const auto ambiguous_result = copperfin::vfp::replace_record_field_value(
        ambiguous_path.string(), 0U, "NaMe", "DELTA");
    expect(!ambiguous_result.ok,
           "#3984: writes should fail closed when multiple descriptors case-fold to the target");
    expect(read_binary_file(ambiguous_path) == ambiguous_before_write,
           "#3984: ambiguous descriptor rejection should preserve every DBF byte");

    fs::remove_all(temp_dir, ignored);
}

void test_missing_field_diagnostic_uses_active_locale() {
    namespace fs = std::filesystem;
    expect(copperfin::localization::select_locale() == "qps-ploc",
           "#3984: isolated diagnostic probe should start in the pseudo-locale");

    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_active_locale_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "localized_missing_field.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U}
    };
    expect(copperfin::vfp::create_dbf_table_file(
               table_path.string(), fields, {{"ALPHA"}}).ok,
           "#3984: isolated diagnostic probe should create its DBF fixture");

    const auto result = copperfin::vfp::replace_record_field_value(
        table_path.string(), 0U, "missing", "BRAVO");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(), "qps-ploc");
    const auto english_catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(), "en-US");
    expect(!result.ok &&
               result.error == pseudo_catalog.translate(
                   "Vfp.DbfTable.Error.TargetFieldNotFoundInTable"),
           "#3984: missing-field writes should use the active pseudo-locale catalog");
    expect(result.error != english_catalog.translate(
               "Vfp.DbfTable.Error.TargetFieldNotFoundInTable"),
           "#3984: pseudo-locale diagnostic probe should not fall back to English");

    fs::remove_all(temp_dir, ignored);
}

void test_dbf_table_default_catalog_refreshes_when_locale_changes() {
    namespace fs = std::filesystem;
    ScopedEnvironmentValue locale_override("COPPERFIN_LOCALE");
    locale_override.set("en-US");

    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_locale_refresh_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "localized_refresh.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U}
    };
    expect(copperfin::vfp::create_dbf_table_file(
               table_path.string(), fields, {{"ALPHA"}}).ok,
           "#4360: locale-refresh fixture should create its DBF table");

    const auto english_result = copperfin::vfp::replace_record_field_value(
        table_path.string(), 0U, "missing", "BRAVO");
    locale_override.set("es-419");
    const auto spanish_result = copperfin::vfp::replace_record_field_value(
        table_path.string(), 0U, "missing", "BRAVO");
    locale_override.set("qps-ploc");
    const auto pseudo_result = copperfin::vfp::replace_record_field_value(
        table_path.string(), 0U, "missing", "BRAVO");

    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    constexpr std::string_view key = "Vfp.DbfTable.Error.TargetFieldNotFoundInTable";
    expect(!english_result.ok && english_result.error == english_catalog.translate(key),
           "#4360: default DBF-table diagnostics should begin in en-US");
    expect(!spanish_result.ok && spanish_result.error == spanish_catalog.translate(key),
           "#4360: default DBF-table diagnostics should refresh to es-419");
    expect(!pseudo_result.ok && pseudo_result.error == pseudo_catalog.translate(key),
           "#4360: default DBF-table diagnostics should refresh to qps-ploc");
    expect(english_result.error != spanish_result.error &&
               spanish_result.error != pseudo_result.error,
           "#4360: locale refresh should not reuse the prior DBF-table diagnostic text");

    fs::remove_all(temp_dir, ignored);
}

void test_schema_rewrites_preserve_raw_memo_and_unaffected_field_bytes() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_schema_raw_memo_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path base_table_path = temp_dir / "base.dbf";
    const fs::path base_memo_path = temp_dir / "base.fpt";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "KEEP", .type = 'C', .length = 8U},
        {.name = "DROP", .type = 'C', .length = 8U},
        {.name = "MEMO", .type = 'M', .length = 4U},
        {.name = "GENERAL", .type = 'G', .length = 4U},
        {.name = "PICTURE", .type = 'P', .length = 4U},
        {.name = "NULLS", .type = 'C', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"ALPHA", "REMOVE", "memo", "gen0", "p0", "x"},
        {"BRAVO", "DELETE", "memo2", "gen2", "pic2", "y"}
    };
    const auto create_result =
        copperfin::vfp::create_dbf_table_file(base_table_path.string(), fields, records);
    expect(create_result.ok, "#3891: schema-rewrite fixture should be created");

    const auto initial_parse = copperfin::vfp::parse_dbf_table_from_file(base_table_path.string(), 2U);
    expect(initial_parse.ok && initial_parse.table.records.size() == 2U,
           "#3891: schema-rewrite fixture should expose both source records");
    if (!initial_parse.ok || initial_parse.table.records.size() != 2U) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    const std::vector<std::vector<std::uint8_t>> memo_payloads{
        {0x00U, 0x01U, 0x41U, 0xFFU},
        {'D', 'E', 'L', ' '}
    };
    const std::vector<std::vector<std::uint8_t>> general_payloads{
        {'G', ' ', 0x09U, ' '},
        {0x02U, 0x00U, 'G', 0x7FU}
    };
    const std::vector<std::vector<std::uint8_t>> picture_payloads{
        {},
        {0xFFU, 0x00U, 0x10U, 'P'}
    };
    const std::vector<std::vector<std::vector<std::uint8_t>>> expected_payloads{
        memo_payloads,
        general_payloads,
        picture_payloads
    };

    for (std::size_t record_index = 0U; record_index < 2U; ++record_index) {
        for (std::size_t memo_index = 0U; memo_index < 3U; ++memo_index) {
            const std::size_t value_index = memo_index + 2U;
            expect(
                replace_memo_block_payload(
                    base_memo_path,
                    initial_parse.table.records[record_index].values[value_index].memo_block_number,
                    expected_payloads[memo_index][record_index]),
                "#3891: binary schema-rewrite memo fixture payload should be installed");
        }
    }

    std::vector<std::uint8_t> table_bytes = read_binary_file(base_table_path);
    const std::size_t null_descriptor_type_offset = 32U + (5U * 32U) + 11U;
    table_bytes[null_descriptor_type_offset] = static_cast<std::uint8_t>('0');
    const std::size_t null_field_offset = initial_parse.table.fields[5U].offset;
    const std::size_t first_record_offset = initial_parse.table.header.header_length;
    const std::size_t second_record_offset =
        first_record_offset + initial_parse.table.header.record_length;
    table_bytes[first_record_offset + null_field_offset] = 0x05U;
    table_bytes[second_record_offset] = 0x2AU;
    table_bytes[second_record_offset + null_field_offset] = 0x80U;
    expect(write_binary_file(base_table_path, table_bytes),
           "#3891: null-flag and deleted-state fixture bytes should be installed");

    const std::vector<std::uint8_t> keep_row0 = read_record_field_bytes(base_table_path, 0U, "KEEP");
    const std::vector<std::uint8_t> keep_row1 = read_record_field_bytes(base_table_path, 1U, "KEEP");
    const std::vector<std::uint8_t> drop_row0 = read_record_field_bytes(base_table_path, 0U, "DROP");
    const std::vector<std::uint8_t> drop_row1 = read_record_field_bytes(base_table_path, 1U, "DROP");
    const std::vector<std::uint8_t> null_row0 = read_record_field_bytes(base_table_path, 0U, "NULLS");
    const std::vector<std::uint8_t> null_row1 = read_record_field_bytes(base_table_path, 1U, "NULLS");

    const auto copy_fixture = [&](const std::string& stem) {
        const fs::path table_path = temp_dir / (stem + ".dbf");
        const fs::path memo_path = temp_dir / (stem + ".fpt");
        fs::copy_file(base_table_path, table_path, fs::copy_options::overwrite_existing);
        fs::copy_file(base_memo_path, memo_path, fs::copy_options::overwrite_existing);
        return table_path;
    };

    const auto verify_rewrite = [&](
        const fs::path& table_path,
        bool keeps_drop,
        bool keeps_raw_keep,
        char expected_memo_type) {
        const fs::path memo_path = table_path.parent_path() / (table_path.stem().string() + ".fpt");
        const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
        expect(parse_result.ok && parse_result.table.records.size() == 2U,
               "#3891: rewritten table should preserve record order and count");
        if (!parse_result.ok || parse_result.table.records.size() != 2U) {
            return;
        }
        expect(!parse_result.table.records[0].deleted && parse_result.table.records[1].deleted,
               "#3891: schema rewrites should preserve deleted-row markers");

        const auto find_field_index = [&](const std::string& name) {
            const auto field = std::find_if(
                parse_result.table.fields.begin(),
                parse_result.table.fields.end(),
                [&](const copperfin::vfp::DbfFieldDescriptor& candidate) {
                    return candidate.name == name;
                });
            return field == parse_result.table.fields.end()
                ? parse_result.table.fields.size()
                : static_cast<std::size_t>(std::distance(parse_result.table.fields.begin(), field));
        };

        const std::size_t keep_index = find_field_index("KEEP");
        expect(keep_index < parse_result.table.fields.size(),
               "#3891: rewritten table should retain the KEEP field");
        if (keep_index < parse_result.table.fields.size()) {
            expect(parse_result.table.records[0].values[keep_index].display_value == "ALPHA" &&
                       parse_result.table.records[1].values[keep_index].display_value == "BRAVO",
                   "#3891: schema rewrites should preserve record ordering and retained values");
        }

        const std::size_t null_index = find_field_index("NULLS");
        expect(null_index < parse_result.table.fields.size() &&
                   parse_result.table.fields[null_index].type == '0',
               "#3891: schema rewrites should retain opaque VFP null-flag field types");
        if (null_index < parse_result.table.fields.size()) {
            expect(parse_result.table.records[0].values[null_index].is_null &&
                       parse_result.table.records[1].values[null_index].is_null,
                   "#3891: schema rewrites should retain parsed null-state provenance");
        }
        expect(read_record_field_bytes(table_path, 0U, "NULLS") == null_row0 &&
                   read_record_field_bytes(table_path, 1U, "NULLS") == null_row1,
               "#3891: schema rewrites should preserve null-flag bytes exactly");

        if (keeps_raw_keep) {
            expect(read_record_field_bytes(table_path, 0U, "KEEP") == keep_row0 &&
                       read_record_field_bytes(table_path, 1U, "KEEP") == keep_row1,
                   "#3891: unaffected character field bytes should remain exact");
        }
        if (keeps_drop) {
            expect(read_record_field_bytes(table_path, 0U, "DROP") == drop_row0 &&
                       read_record_field_bytes(table_path, 1U, "DROP") == drop_row1,
                   "#3891: unaffected sibling field bytes should remain exact");
        } else {
            expect(find_field_index("DROP") == parse_result.table.fields.size(),
                   "#3891: DROP schema rewrites should remove only the requested field");
        }

        const std::vector<std::string> memo_names{"MEMO", "GENERAL", "PICTURE"};
        const std::vector<char> memo_types{expected_memo_type, 'G', 'P'};
        for (std::size_t memo_index = 0U; memo_index < memo_names.size(); ++memo_index) {
            const std::size_t field_index = find_field_index(memo_names[memo_index]);
            expect(field_index < parse_result.table.fields.size() &&
                       parse_result.table.fields[field_index].type == memo_types[memo_index],
                   "#3891: schema rewrites should retain each memo-family field type");
            if (field_index >= parse_result.table.fields.size()) {
                continue;
            }
            for (std::size_t record_index = 0U; record_index < 2U; ++record_index) {
                const std::uint32_t block_number =
                    parse_result.table.records[record_index].values[field_index].memo_block_number;
                expect(block_number != 0U,
                       "#3891: referenced empty and binary memo payloads should retain a block");
                expect(copperfin::vfp::read_memo_block_raw(memo_path.string(), block_number) ==
                           expected_payloads[memo_index][record_index],
                       "#3891: schema rewrites should preserve exact raw M/G/P payload bytes");
            }
        }
    };

    const fs::path add_table_path = copy_fixture("add");
    const auto add_result = copperfin::vfp::add_dbf_table_field(
        add_table_path.string(),
        {.name = "ADDED", .type = 'N', .length = 5U});
    expect(add_result.ok, "#3891: ADD COLUMN should preserve retained raw memo payloads");
    verify_rewrite(add_table_path, true, true, 'M');

    const fs::path drop_table_path = copy_fixture("drop");
    const auto drop_result = copperfin::vfp::drop_dbf_table_field(drop_table_path.string(), "DROP");
    expect(drop_result.ok, "#3891: DROP COLUMN should preserve retained raw memo payloads");
    verify_rewrite(drop_table_path, false, true, 'M');

    const fs::path alter_table_path = copy_fixture("alter");
    const auto alter_result = copperfin::vfp::alter_dbf_table_field(
        alter_table_path.string(),
        {.name = "KEEP", .type = 'C', .length = 12U});
    expect(alter_result.ok, "#3891: ALTER COLUMN should preserve retained raw memo payloads");
    verify_rewrite(alter_table_path, true, false, 'M');

    const fs::path alter_memo_table_path = copy_fixture("alter_memo");
    const auto alter_memo_result = copperfin::vfp::alter_dbf_table_field(
        alter_memo_table_path.string(),
        {.name = "MEMO", .type = 'G', .length = 4U});
    expect(alter_memo_result.ok,
           "#3891: ALTER COLUMN should preserve payloads when changing a memo-family field type");
    verify_rewrite(alter_memo_table_path, true, true, 'G');

    for (std::size_t operation = 0U; operation < 3U; ++operation) {
        const fs::path corrupt_table_path = copy_fixture("corrupt_" + std::to_string(operation));
        const fs::path corrupt_memo_path =
            corrupt_table_path.parent_path() / (corrupt_table_path.stem().string() + ".fpt");
        std::vector<std::uint8_t> corrupt_memo_bytes = read_binary_file(corrupt_memo_path);
        corrupt_memo_bytes.resize(512U);
        expect(write_binary_file(corrupt_memo_path, corrupt_memo_bytes),
               "#3891: unreadable memo fixture should be truncated");
        // The corrupt fixture is the authoritative pre-operation state; fail-closed
        // behavior must preserve it exactly rather than attempting a lossy repair.
        const std::vector<std::uint8_t> pre_operation_table_bytes = read_binary_file(corrupt_table_path);
        const std::vector<std::uint8_t> pre_operation_memo_bytes = read_binary_file(corrupt_memo_path);

        copperfin::vfp::DbfWriteResult result;
        if (operation == 0U) {
            result = copperfin::vfp::add_dbf_table_field(
                corrupt_table_path.string(),
                {.name = "ADDED", .type = 'N', .length = 5U});
        } else if (operation == 1U) {
            result = copperfin::vfp::drop_dbf_table_field(corrupt_table_path.string(), "DROP");
        } else {
            result = copperfin::vfp::alter_dbf_table_field(
                corrupt_table_path.string(),
                {.name = "MEMO", .type = 'G', .length = 4U});
        }

        expect(!result.ok && result.error == "Unable to recover memo payload from the sidecar.",
               "#3891: schema rewrites should fail closed when retained memo payloads are unreadable");
        expect(read_binary_file(corrupt_table_path) == pre_operation_table_bytes &&
                   read_binary_file(corrupt_memo_path) == pre_operation_memo_bytes,
               "#3891: failed schema rewrites should leave DBF/FPT bytes untouched");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_schema_rewrites_preserve_code_page_marks() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_schema_code_page_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "marked.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "NOTES", .type = 'M', .length = 4U}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(), fields, {{"", ""}});
    expect(create_result.ok, "marked schema fixture should be created");

    {
        std::fstream output(table_path, std::ios::binary | std::ios::in | std::ios::out);
        output.seekp(29U);
        output.put(static_cast<char>(0x03U));
    }
    const auto name_result = copperfin::vfp::replace_record_field_value(
        table_path.string(), 0U, "NAME", "caf\xC3\xA9");
    const auto notes_result = copperfin::vfp::replace_record_field_value(
        table_path.string(), 0U, "NOTES", "na\xC3\xAFve");
    expect(name_result.ok && notes_result.ok, "marked schema fixture text should be writable");

    const auto expect_marked_text = [&](const std::string& stage) {
        const auto header = copperfin::vfp::parse_dbf_header_from_file(table_path.string());
        const auto table = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
        expect(header.ok && header.header.code_page_mark == 0x03U,
               stage + " should preserve the CP1252 header mark");
        expect(table.ok && table.table.records.size() == 1U,
               stage + " should remain readable");
        if (table.ok && !table.table.records.empty()) {
            const auto field_value = [&](const std::string& field_name) {
                const auto value = std::find_if(
                    table.table.records.front().values.begin(),
                    table.table.records.front().values.end(),
                    [&](const copperfin::vfp::DbfRecordValue& candidate) {
                        return candidate.field_name == field_name;
                    });
                return value == table.table.records.front().values.end() ? nullptr : &*value;
            };
            const auto* name = field_value("NAME");
            const auto* notes = field_value("NOTES");
            expect(name != nullptr && name->display_value == "caf\xC3\xA9",
                   stage + " should preserve decoded character text");
            expect(notes != nullptr && notes->display_value == "na\xC3\xAFve",
                   stage + " should preserve decoded memo text");
        }
    };

    expect(copperfin::vfp::add_dbf_table_field(
               table_path.string(), {.name = "EXTRA", .type = 'C', .length = 8U}).ok,
           "ADD COLUMN should succeed on a marked table");
    expect_marked_text("ADD COLUMN");
    expect(copperfin::vfp::drop_dbf_table_field(table_path.string(), "EXTRA").ok,
           "DROP COLUMN should succeed on a marked table");
    expect_marked_text("DROP COLUMN");
    expect(copperfin::vfp::alter_dbf_table_field(
               table_path.string(), {.name = "NAME", .type = 'C', .length = 32U}).ok,
           "ALTER COLUMN should succeed on a marked table");
    expect_marked_text("ALTER COLUMN");

    fs::remove_all(temp_dir, ignored);
}

void test_indexed_table_mutations_succeed_with_production_flags_and_companions() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_index_guard_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto write_people_table = [](const fs::path& path, std::uint8_t table_flags) {
        std::vector<std::uint8_t> table_bytes(97U + (2U * 14U) + 1U, 0U);
        table_bytes[0] = 0x30U;
        write_le_u32(table_bytes, 4U, 2U);
        write_le_u16(table_bytes, 8U, 97U);
        write_le_u16(table_bytes, 10U, 14U);
        table_bytes[28U] = table_flags;

        write_field_descriptor(table_bytes, 32U, "NAME", 'C', 1U, 10U);
        write_field_descriptor(table_bytes, 64U, "AGE", 'N', 11U, 3U);
        table_bytes[96U] = 0x0DU;

        table_bytes[97U] = 0x20U;
        write_ascii(table_bytes, 98U, "ALPHA     ");
        write_ascii(table_bytes, 108U, " 10");

        table_bytes[111U] = 0x20U;
        write_ascii(table_bytes, 112U, "BRAVO     ");
        write_ascii(table_bytes, 122U, " 20");
        table_bytes.back() = 0x1AU;

        std::ofstream output(path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    };

    const fs::path flagged_table_path = temp_dir / "flagged.dbf";
    write_people_table(flagged_table_path, 0x01U);
    const auto flagged_replace = copperfin::vfp::replace_record_field_value(flagged_table_path.string(), 0U, "NAME", "OMEGA");
    expect(flagged_replace.ok, "replace_record_field_value should support DBFs marked with production indexes");

    const auto flagged_append = copperfin::vfp::append_blank_record_to_file(flagged_table_path.string());
    expect(flagged_append.ok, "append_blank_record_to_file should support DBFs marked with production indexes");
    expect(flagged_append.record_count == 3U, "indexed append should update the record count");

    const auto flagged_delete = copperfin::vfp::set_record_deleted_flag(flagged_table_path.string(), 0U, true);
    expect(flagged_delete.ok, "set_record_deleted_flag should support DELETE-style writes on production-index DBFs");

    const auto flagged_recall = copperfin::vfp::set_record_deleted_flag(flagged_table_path.string(), 0U, false);
    expect(flagged_recall.ok, "set_record_deleted_flag should support RECALL-style writes on production-index DBFs");

    const auto flagged_parse = copperfin::vfp::parse_dbf_table_from_file(flagged_table_path.string(), 5U);
    expect(flagged_parse.ok, "production-flagged DBFs should remain readable after mutation writes");
    expect(flagged_parse.table.records.size() == 3U, "production-flagged DBFs should expose appended records after mutation writes");
    if (flagged_parse.table.records.size() == 3U) {
        expect(flagged_parse.table.records[0].values[0].display_value == "OMEGA", "indexed mutation should persist REPLACE writes");
        expect(!flagged_parse.table.records[0].deleted, "RECALL should clear indexed-table tombstones");
    }

    const fs::path companion_table_path = temp_dir / "companion.dbf";
    const fs::path companion_cdx_path = temp_dir / "companion.cdx";
    write_people_table(companion_table_path, 0x00U);
    {
        std::ofstream output(companion_cdx_path, std::ios::binary);
        output << "synthetic companion index";
    }
    const auto companion_append = copperfin::vfp::append_blank_record_to_file(companion_table_path.string());
    expect(companion_append.ok, "append_blank_record_to_file should support DBFs with a same-base companion CDX");
    expect(companion_append.record_count == 3U, "companion-CDX append should update the record count");

    const auto companion_replace = copperfin::vfp::replace_record_field_value(companion_table_path.string(), 1U, "NAME", "CHARLIE");
    expect(companion_replace.ok, "replace_record_field_value should support DBFs with a same-base companion CDX");

    const auto companion_parse = copperfin::vfp::parse_dbf_table_from_file(companion_table_path.string(), 5U);
    expect(companion_parse.ok, "companion-CDX DBFs should remain readable after mutation writes");
    expect(companion_parse.table.records.size() == 3U, "companion-CDX DBFs should expose appended records after mutation writes");
    if (companion_parse.table.records.size() >= 2U) {
        expect(companion_parse.table.records[1].values[0].display_value == "CHARLIE", "companion-CDX mutation should persist REPLACE writes");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_integer_field_create_replace_and_append_round_trip() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_integer_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "numbers.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "COUNT", .type = 'I', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"ALPHA", "10"},
        {"BRAVO", "-20"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "create_dbf_table_file should support Integer (I) fields");

    auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "Integer-backed DBFs should remain readable after creation");
    expect(parse_result.table.records.size() == 2U, "Integer-backed DBFs should expose created rows");
    if (parse_result.table.records.size() == 2U) {
        expect(parse_result.table.records[0].values[1].display_value == "10", "created Integer fields should round-trip positive values");
        expect(parse_result.table.records[1].values[1].display_value == "-20", "created Integer fields should round-trip negative values");
    }

    const auto replace_result = copperfin::vfp::replace_record_field_value(table_path.string(), 1U, "COUNT", "21");
    expect(replace_result.ok, "replace_record_field_value should support Integer (I) fields");

    const auto append_result = copperfin::vfp::append_blank_record_to_file(table_path.string());
    expect(append_result.ok, "append_blank_record_to_file should support Integer (I) fields");
    expect(append_result.record_count == 3U, "Integer-backed append_blank_record_to_file should grow the record count");

    parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "Integer-backed DBFs should remain readable after mutation");
    expect(parse_result.table.records.size() == 3U, "Integer-backed DBFs should expose appended rows");
    if (parse_result.table.records.size() == 3U) {
        expect(parse_result.table.records[1].values[1].display_value == "21", "Integer field replacements should persist");
        expect(parse_result.table.records[2].values[0].display_value.empty(), "blank appended character fields beside Integer fields should start empty");
        expect(parse_result.table.records[2].values[1].display_value == "0", "blank appended Integer fields should initialize to zero");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_currency_and_datetime_field_round_trip() {
    const std::locale grouping_locale(std::locale::classic(), new comma_decimal_numpunct());
    scoped_global_locale locale_guard(grouping_locale);
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_currency_datetime_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "ledger.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "BALANCE", .type = 'Y', .length = 8U},
        {.name = "STAMP", .type = 'T', .length = 8U}
    };
    const std::vector<std::vector<std::string>> records{
        {"ALPHA", "123.4500", "julian:2460401 millis:12345"},
        {"BRAVO", "-2.5000", "julian:2460402 millis:67890"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "create_dbf_table_file should support Currency (Y) and DateTime (T) fields");

    auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "Currency/DateTime-backed DBFs should remain readable after creation");
    expect(parse_result.table.records.size() == 2U, "Currency/DateTime-backed DBFs should expose created rows");
    if (parse_result.table.records.size() == 2U) {
        expect(parse_result.table.records[0].values[1].display_value == "123.4500", "created Currency fields should preserve four-decimal formatting");
        expect(parse_result.table.records[0].values[2].display_value == "julian:2460401 millis:12345", "created DateTime fields should round-trip through the shared storage contract");
        expect(parse_result.table.records[1].values[1].display_value == "-2.5000", "negative Currency fields should round-trip");
    }

    const auto replace_currency = copperfin::vfp::replace_record_field_value(table_path.string(), 1U, "BALANCE", "42.0001");
    expect(replace_currency.ok, "replace_record_field_value should support Currency (Y) fields");

    const auto replace_datetime = copperfin::vfp::replace_record_field_value(table_path.string(), 1U, "STAMP", "julian:2460403 millis:222");
    expect(replace_datetime.ok, "replace_record_field_value should support DateTime (T) fields");

    const auto append_result = copperfin::vfp::append_blank_record_to_file(table_path.string());
    expect(append_result.ok, "append_blank_record_to_file should support Currency/DateTime-backed tables");
    expect(append_result.record_count == 3U, "Currency/DateTime append_blank_record_to_file should grow the record count");

    parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "Currency/DateTime-backed DBFs should remain readable after mutation");
    expect(parse_result.table.records.size() == 3U, "Currency/DateTime-backed DBFs should expose appended rows");
    if (parse_result.table.records.size() == 3U) {
        expect(parse_result.table.records[1].values[1].display_value == "42.0001", "Currency field replacements should persist");
        expect(parse_result.table.records[1].values[2].display_value == "julian:2460403 millis:222", "DateTime field replacements should persist");
        expect(parse_result.table.records[2].values[1].display_value == "0.0000", "blank appended Currency fields should initialize to zero");
        expect(parse_result.table.records[2].values[2].display_value == "julian:0 millis:0", "blank appended DateTime fields should initialize to zero storage");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_currency_display_ignores_grouping_locale() {
    const std::locale grouping_locale(std::locale::classic(), new comma_decimal_numpunct());
    scoped_global_locale locale_guard(grouping_locale);
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_currency_locale_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "currency_locale.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "POSITIVE", .type = 'Y', .length = 8U},
        {.name = "NEGATIVE", .type = 'Y', .length = 8U},
        {.name = "ZERO", .type = 'Y', .length = 8U}
    };
    const std::vector<std::vector<std::string>> records{
        {"12345.6789", "-23456.5000", "0.0000"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#4839: currency locale fixture should be created");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "#4839: currency locale fixture should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 1U) {
        const auto& values = parse_result.table.records[0].values;
        expect(values[0].display_value == "12345.6789", "#4839: DBF positive currency display should not group digits");
        expect(values[1].display_value == "-23456.5000", "#4839: DBF negative currency display should remain period-decimal");
        expect(values[2].display_value == "0.0000", "#4839: DBF zero currency display should preserve four fractional digits");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_double_field_create_replace_and_append_round_trip() {
    const std::locale comma_locale(std::locale::classic(), new comma_decimal_numpunct());
    scoped_global_locale locale_guard(comma_locale);
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_double_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "metrics.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "SCORE", .type = 'B', .length = 8U}
    };
    const std::vector<std::vector<std::string>> records{
        {"ALPHA", "10.5"},
        {"BRAVO", "-2.25"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "create_dbf_table_file should support Double (B) fields");

    auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "Double-backed DBFs should remain readable after creation");
    expect(parse_result.table.records.size() == 2U, "Double-backed DBFs should expose created rows");
    if (parse_result.table.records.size() == 2U) {
        expect(parse_result.table.records[0].values[1].display_value == "10.5", "created Double fields should round-trip positive values");
        expect(parse_result.table.records[1].values[1].display_value == "-2.25", "created Double fields should round-trip negative values");
    }

    const auto replace_result = copperfin::vfp::replace_record_field_value(table_path.string(), 1U, "SCORE", "21.125");
    expect(replace_result.ok, "replace_record_field_value should support Double (B) fields");

    const auto append_result = copperfin::vfp::append_blank_record_to_file(table_path.string());
    expect(append_result.ok, "append_blank_record_to_file should support Double (B) fields");
    expect(append_result.record_count == 3U, "Double-backed append_blank_record_to_file should grow the record count");

    parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "Double-backed DBFs should remain readable after mutation");
    expect(parse_result.table.records.size() == 3U, "Double-backed DBFs should expose appended rows");
    if (parse_result.table.records.size() == 3U) {
        expect(parse_result.table.records[1].values[1].display_value == "21.125", "Double field replacements should persist");
        expect(parse_result.table.records[2].values[1].display_value == "0", "blank appended Double fields should initialize to zero");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_append_blank_supports_opaque_field_layouts() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_append_guard_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "unsupported.dbf";
    std::vector<std::uint8_t> table_bytes(97U + 9U + 1U, 0U);
    table_bytes[0] = 0x30U;
    write_le_u32(table_bytes, 4U, 1U);
    write_le_u16(table_bytes, 8U, 65U);
    write_le_u16(table_bytes, 10U, 9U);

    write_field_descriptor(table_bytes, 32U, "VALUE", 'W', 1U, 8U);
    table_bytes[64U] = 0x0DU;
    table_bytes[65U] = 0x20U;
    write_ascii(table_bytes, 66U, "12345678");
    table_bytes.back() = 0x1AU;

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }

    const auto append_result = copperfin::vfp::append_blank_record_to_file(table_path.string());
    expect(append_result.ok, "append_blank_record_to_file should support opaque binary field layouts");
    expect(append_result.record_count == 2U, "opaque-layout append_blank_record_to_file should grow the record count");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "opaque-layout DBFs should remain readable after APPEND BLANK");
    expect(parse_result.table.records.size() == 2U, "opaque-layout DBFs should expose the appended row");
    if (parse_result.ok && parse_result.table.records.size() == 2U && !parse_result.table.records[1U].values.empty()) {
        expect(parse_result.table.records[1U].values[0U].display_value == "0x0000000000000000",
            "blank appended opaque fields should initialize to zero bytes");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_replace_opaque_field_round_trips_hex_payloads() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_opaque_replace_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "opaque.dbf";
    std::vector<std::uint8_t> table_bytes(65U + 9U + 1U, 0U);
    table_bytes[0] = 0x30U;
    write_le_u32(table_bytes, 4U, 1U);
    write_le_u16(table_bytes, 8U, 65U);
    write_le_u16(table_bytes, 10U, 9U);
    write_field_descriptor(table_bytes, 32U, "VALUE", 'W', 1U, 8U);
    table_bytes[64U] = 0x0DU;
    table_bytes[65U] = 0x20U;
    write_ascii(table_bytes, 66U, "12345678");
    table_bytes.back() = 0x1AU;

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }

    const auto replace_result = copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "VALUE", "0x4142434445464748");
    expect(replace_result.ok, "replace_record_field_value should support opaque field hex payloads");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "opaque field replacement should leave the DBF readable");
    expect(parse_result.table.records.size() == 1U, "opaque field replacement should preserve record count");
    if (parse_result.ok && parse_result.table.records.size() == 1U && !parse_result.table.records[0U].values.empty()) {
        expect(parse_result.table.records[0U].values[0U].display_value == "0x4142434445464748",
            "opaque field replacement should persist the written hex payload");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_parse_dbf_table_rejects_truncated_visual_asset() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_asset_validation_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "broken.scx";
    std::vector<std::uint8_t> table_bytes(32U, 0U);
    table_bytes[0] = 0x30U;
    write_le_u32(table_bytes, 4U, 1U);
    write_le_u16(table_bytes, 8U, 97U);
    write_le_u16(table_bytes, 10U, 13U);

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
    expect(!parse_result.ok, "parse_dbf_table_from_file should reject truncated visual assets");
    expect(parse_result.error == "Table file is shorter than its header length.", "truncated visual assets should report a header-length validation error");

    fs::remove_all(temp_dir, ignored);
}

void test_visual_asset_memo_sidecar_repair_round_trip() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_asset_repair_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "designer.scx";
    const fs::path memo_path = temp_dir / "designer.sct";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 12U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtTitle", "Left = 10\r\nTop = 20\r\n"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "create_dbf_table_file should support SCX/SCT-style memo assets");
    expect(fs::exists(memo_path), "SCX-backed table creation should emit the SCT sidecar");

    auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
    expect(parse_result.ok, "synthetic SCX/SCT assets should parse before repair");
    if (parse_result.ok && parse_result.table.records.size() == 1U && parse_result.table.records[0].values.size() >= 2U) {
        expect(parse_result.table.records[0].values[1].display_value.find("Left = 10") != std::string::npos,
               "synthetic SCX/SCT assets should round-trip the initial memo payload");
    }

    {
        std::vector<std::uint8_t> broken_memo(8U, 0U);
        std::ofstream output(memo_path, std::ios::binary | std::ios::trunc);
        output.write(reinterpret_cast<const char*>(broken_memo.data()), static_cast<std::streamsize>(broken_memo.size()));
    }

    const auto replace_result = copperfin::vfp::replace_record_field_value(
        table_path.string(),
        0U,
        "PROPERTIES",
        "Left = 25\r\nTop = 20\r\nWidth = 40\r\n");
    expect(replace_result.ok, "replace_record_field_value should repair an invalid SCT sidecar while updating memo-backed asset fields");

    parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
    expect(parse_result.ok, "synthetic SCX/SCT assets should parse after memo-sidecar repair");
    if (parse_result.ok && parse_result.table.records.size() == 1U && parse_result.table.records[0].values.size() >= 2U) {
        expect(parse_result.table.records[0].values[1].display_value.find("Left = 25") != std::string::npos,
               "memo-sidecar repair should preserve the updated asset memo payload");
        expect(parse_result.table.records[0].values[1].display_value.find("Width = 40") != std::string::npos,
               "memo-sidecar repair should keep the rewritten multi-line asset memo content readable");
    }

    const auto repaired_memo_bytes = read_binary_file(memo_path);
    expect(repaired_memo_bytes.size() >= 1024U, "memo-sidecar repair should rebuild a valid SCT allocation block");

    fs::remove_all(temp_dir, ignored);
}

void test_memo_replace_recovers_directory_sidecar_path() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_memo_rollback_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "rollback.dbf";
    const fs::path memo_path = temp_dir / "rollback.fpt";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "TITLE", .type = 'C', .length = 12U},
        {.name = "BODY", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"FIRST", "Initial memo payload"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "setup should create a memo-backed table for rollback validation");
    fs::remove(memo_path, ignored);
    fs::create_directories(memo_path, ignored);

    const auto replace_result = copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "BODY", "Updated payload should rollback");
    expect(replace_result.ok, "memo replacement should recover when the sidecar path is an unexpected directory");
    expect(fs::is_regular_file(memo_path), "memo replacement should restore a regular memo sidecar file");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "table should remain readable after recovering the memo sidecar path");
    if (parse_result.ok && parse_result.table.records.size() == 1U && parse_result.table.records[0].values.size() >= 2U) {
        expect(parse_result.table.records[0].values[1].display_value == "Updated payload should rollback", "memo replacement should persist the updated payload after recovery");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_replace_field_value_accepts_null_token_for_nonstring_types() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_null_token_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "nullable.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NVAL", .type = 'N', .length = 6U},
        {.name = "LVAL", .type = 'L', .length = 1U},
        {.name = "DVAL", .type = 'D', .length = 8U},
        {.name = "BVAL", .type = 'B', .length = 8U},
        {.name = "IVAL", .type = 'I', .length = 4U},
        {.name = "YVAL", .type = 'Y', .length = 8U},
        {.name = "TVAL", .type = 'T', .length = 8U}
    };
    const std::vector<std::vector<std::string>> records{
        {"123", "true", "2026-04-12", "3.5", "7", "8.1250", "julian:2460412 millis:777"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "setup should create a non-string table for NULL-token mutation tests");

    expect(copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "NVAL", "NULL").ok, "NULL token should be accepted for N fields");
    expect(copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "LVAL", "NULL").ok, "NULL token should be accepted for L fields");
    expect(copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "DVAL", "NULL").ok, "NULL token should be accepted for D fields");
    expect(copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "BVAL", "NULL").ok, "NULL token should be accepted for B fields");
    expect(copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "IVAL", "NULL").ok, "NULL token should be accepted for I fields");
    expect(copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "YVAL", "NULL").ok, "NULL token should be accepted for Y fields");
    expect(copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "TVAL", "NULL").ok, "NULL token should be accepted for T fields");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "NULL-token mutated table should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 1U && parse_result.table.records[0].values.size() >= 7U) {
        expect(parse_result.table.records[0].values[0].display_value.empty(), "N NULL token should clear numeric storage");
        expect(parse_result.table.records[0].values[1].display_value == "?", "L NULL token should set unknown logical marker");
        expect(parse_result.table.records[0].values[2].display_value.empty(), "D NULL token should clear date storage");
        expect(parse_result.table.records[0].values[3].display_value == "0", "B NULL token should zero double storage");
        expect(parse_result.table.records[0].values[4].display_value == "0", "I NULL token should zero integer storage");
        expect(parse_result.table.records[0].values[5].display_value == "0.0000", "Y NULL token should zero currency storage");
        expect(parse_result.table.records[0].values[6].display_value == "julian:0 millis:0", "T NULL token should zero datetime storage");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_varchar_and_varbinary_field_round_trip() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_vq_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "vq.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "TITLE", .type = 'C', .length = 10U},
        {.name = "VCOL", .type = 'V', .length = 9U},
        {.name = "QCOL", .type = 'Q', .length = 9U}
    };
    const std::vector<std::vector<std::string>> records{
        {"ALPHA", "V-ONE", "Q_ONE"},
        {"BRAVO", "V-TWO", "Q_TWO"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "create_dbf_table_file should support V/Q fields");

    auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "V/Q-backed DBFs should remain readable after creation");
    if (parse_result.ok && parse_result.table.records.size() == 2U && parse_result.table.records[0].values.size() >= 3U) {
        expect(parse_result.table.records[0].values[1].display_value == "V-ONE", "created V fields should round-trip");
        expect(parse_result.table.records[1].values[2].display_value == "Q_TWO", "created Q fields should round-trip");
    }

    expect(copperfin::vfp::replace_record_field_value(table_path.string(), 1U, "VCOL", "V-THREE").ok, "replace_record_field_value should support V fields");
    expect(copperfin::vfp::replace_record_field_value(table_path.string(), 1U, "QCOL", "Q_THREE").ok, "replace_record_field_value should support Q fields");

    const auto append_result = copperfin::vfp::append_blank_record_to_file(table_path.string());
    expect(append_result.ok, "append_blank_record_to_file should support V/Q-backed tables");
    expect(append_result.record_count == 3U, "V/Q-backed append_blank_record_to_file should grow the record count");

    parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "V/Q-backed DBFs should remain readable after mutation");
    expect(parse_result.table.records.size() == 3U, "V/Q-backed DBFs should expose appended rows");
    if (parse_result.table.records.size() == 3U && parse_result.table.records[1].values.size() >= 3U) {
        expect(parse_result.table.records[1].values[1].display_value == "V-THREE", "V field replacements should persist");
        expect(parse_result.table.records[1].values[2].display_value == "Q_THREE", "Q field replacements should persist");
        expect(parse_result.table.records[2].values[1].display_value.empty(), "blank appended V fields should initialize empty");
        expect(parse_result.table.records[2].values[2].display_value.empty(), "blank appended Q fields should initialize empty");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_dbf_header_record_count_exceeds_file_size_is_rejected() {
    // GAP-02: a DBF file whose header claims more records than the file
    // can physically contain must be rejected or clamped — not cause an
    // out-of-bounds read or host crash.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_header_overflow_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "overflow_header.dbf";

    // Build a valid minimal 2-record DBF and then overwrite the record-count
    // field (bytes 4-7, LE u32) with 1000 — far beyond what the file can hold.
    // Header = 97 bytes, each record = 14 bytes, so the file holds at most 2.
    std::vector<std::uint8_t> table_bytes(97U + (2U * 14U) + 1U, 0U);
    table_bytes[0] = 0x30U;
    write_le_u32(table_bytes, 4U, 1000U);   // inflated record count
    write_le_u16(table_bytes, 8U, 97U);
    write_le_u16(table_bytes, 10U, 14U);
    write_field_descriptor(table_bytes, 32U, "NAME", 'C', 1U, 10U);
    write_field_descriptor(table_bytes, 64U, "AGE",  'N', 11U, 3U);
    table_bytes[96U] = 0x0DU;
    table_bytes[97U] = 0x20U;
    write_ascii(table_bytes, 98U,  "ALPHA     ");
    write_ascii(table_bytes, 108U, " 10");
    table_bytes[111U] = 0x20U;
    write_ascii(table_bytes, 112U, "BRAVO     ");
    write_ascii(table_bytes, 122U, " 20");
    table_bytes.back() = 0x1AU;

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()),
                     static_cast<std::streamsize>(table_bytes.size()));
    }

    // Parser must not crash. It may return ok=false (rejection) or ok=true
    // with the record count clamped to what is physically present (≤2).
    const auto result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1100U);
    if (result.ok) {
        expect(result.table.records.size() <= 2U,
               "GAP-02: parser must clamp record count to physically available records");
    }
    // If !result.ok that is an acceptable safe rejection — the host is still alive.

    fs::remove_all(temp_dir, ignored);
}

void test_dbf_field_descriptor_count_exceeds_header_size_is_rejected() {
    // GAP-02: descriptor parsing must honor header_length and not consume
    // descriptor-shaped bytes beyond the declared header boundary.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_descriptor_header_bounds_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "descriptor_bounds.dbf";

    // Header claims exactly one descriptor slot (32 + 32 + 1 = 65), but file
    // physically contains two descriptor blocks.
    std::vector<std::uint8_t> table_bytes(97U, 0U);
    table_bytes[0] = 0x30U;
    write_le_u32(table_bytes, 4U, 0U);
    write_le_u16(table_bytes, 8U, 65U);
    write_le_u16(table_bytes, 10U, 14U);
    write_field_descriptor(table_bytes, 32U, "NAME", 'C', 1U, 10U);
    write_field_descriptor(table_bytes, 64U, "AGE", 'N', 11U, 3U);
    table_bytes[96U] = 0x0DU;

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()),
                     static_cast<std::streamsize>(table_bytes.size()));
    }

    const auto result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(result.ok, "GAP-02: parser should safely parse adversarial descriptor/header mismatch input");
    expect(result.table.fields.size() <= 1U,
           "GAP-02: parser must not consume descriptor bytes beyond the declared header length");

    fs::remove_all(temp_dir, ignored);
}

void test_dbf_record_width_mismatch_field_sum_is_rejected() {
    // GAP-02: table mutation paths must reject schemas whose declared
    // record width is smaller than the field layout requires.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_record_width_mismatch_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "record_width_mismatch.dbf";

    std::vector<std::uint8_t> table_bytes(97U + 5U + 1U, 0U);
    table_bytes[0] = 0x30U;
    write_le_u32(table_bytes, 4U, 1U);
    write_le_u16(table_bytes, 8U, 97U);
    write_le_u16(table_bytes, 10U, 5U);  // invalid: too short for NAME C(10) at offset 1
    write_field_descriptor(table_bytes, 32U, "NAME", 'C', 1U, 10U);
    write_field_descriptor(table_bytes, 64U, "AGE", 'N', 11U, 3U);
    table_bytes[96U] = 0x0DU;
    table_bytes[97U] = 0x20U;
    table_bytes.back() = 0x1AU;

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()),
                     static_cast<std::streamsize>(table_bytes.size()));
    }

    const auto append_result = copperfin::vfp::append_blank_record_to_file(table_path.string());
    expect(!append_result.ok,
           "GAP-02: append should reject mismatched record-width/field-layout tables");
    expect(append_result.error == "Table field layout exceeds the record size.",
           "GAP-02: mismatch rejection should report the expected layout error");

    fs::remove_all(temp_dir, ignored);
}

void test_dbf_table_record_value_errors_resolve_through_localization_catalog() {
    namespace fs = std::filesystem;
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english_catalog.translate("Vfp.DbfTable.Error.CharacterValueTooLarge") ==
            "Character value is too large for the target field.",
        "#2381: DBF table character overflow error should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Vfp.DbfTable.Error.DateTimeValueInvalid") ==
            "DateTime fields currently accept values formatted as 'julian:<day> millis:<milliseconds>'.",
        "#2381: DBF table datetime validation error should resolve through the en-US catalog");
    expect(
        spanish_catalog.translate("Vfp.DbfTable.Error.CharacterValueTooLarge") ==
            "El valor de caracteres es demasiado grande para el campo destino.",
        "#2602: DBF table character overflow error should resolve through the es-419 catalog");
    expect(
        portuguese_catalog.translate("Vfp.DbfTable.Error.DateTimeValueInvalid") ==
            "Campos DateTime atualmente aceitam valores formatados como 'julian:<day> millis:<milliseconds>'.",
        "#2602: DBF table datetime validation error should resolve through the pt-BR catalog");
    expect(
        pseudo_catalog.translate("Vfp.DbfTable.Error.CharacterValueTooLarge") !=
            english_catalog.translate("Vfp.DbfTable.Error.CharacterValueTooLarge"),
        "#2381: DBF table record/value errors should be pseudo-localizable");
    expect(
        pseudo_catalog.translate("Vfp.DbfTable.Error.CharacterValueTooLarge") ==
            copperfin::localization::pseudo_localize(
                "Character value is too large for the target field."),
        "#2602: DBF table qps-ploc record/value errors should use the pseudo-localization transform");

    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_localized_record_value_error_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "localized_record_value_error.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 5U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{{"ALPHA"}};
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#2381: localized DBF table validation fixture should be created");

    const auto replace_result =
        copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "NAME", "TOO-LONG");
    expect(!replace_result.ok, "#2381: oversized character field writes should fail");
    expect(
        replace_result.error == "Character value is too large for the target field.",
        "#2381: oversized character field writes should preserve the default localized error");

    fs::remove_all(temp_dir, ignored);
}

void test_dbf_table_creation_errors_resolve_through_localization_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap y_field{{"fieldType", "Y"}};

    expect(
        english_catalog.translate("Vfp.DbfTable.Error.CreateFieldRequired") ==
            "At least one field is required to create a DBF table.",
        "#2382: DBF table creation field-required error should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Vfp.DbfTable.Error.EightByteFieldWidthInvalid", y_field) ==
            "Y fields require a width of exactly 8 bytes.",
        "#2382: DBF table field-type width errors should preserve named placeholders");
    expect(
        pseudo_catalog.translate("Vfp.DbfTable.Error.CreateFieldRequired") !=
            english_catalog.translate("Vfp.DbfTable.Error.CreateFieldRequired"),
        "#2382: DBF table creation/open/write errors should be pseudo-localizable");

    const auto no_fields_result = copperfin::vfp::create_dbf_table_file("unused.dbf", {}, {});
    expect(!no_fields_result.ok, "#2382: DBF table creation should reject empty field lists");
    expect(
        no_fields_result.error == "At least one field is required to create a DBF table.",
        "#2382: empty field list should preserve the default localized error");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "BALANCE", .type = 'Y', .offset = 1U, .length = 4U, .decimal_count = 0U}
    };
    const auto invalid_width_result = copperfin::vfp::create_dbf_table_file("unused.dbf", fields, {{"0"}});
    expect(!invalid_width_result.ok, "#2382: DBF table creation should reject invalid Y field widths");
    expect(
        invalid_width_result.error == "Y fields require a width of exactly 8 bytes.",
        "#2382: invalid Y field width should preserve the default localized placeholder output");
}

void test_dbf_table_schema_mutation_errors_resolve_through_localization_catalog() {
    namespace fs = std::filesystem;
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english_catalog.translate("Vfp.DbfTable.Error.TargetFieldExists") ==
            "The target field already exists.",
        "#2383: DBF table duplicate-field schema error should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Vfp.DbfTable.Error.DropLastField") ==
            "Cannot drop the last field from a DBF table.",
        "#2383: DBF table last-field drop error should resolve through the en-US catalog");
    expect(
        pseudo_catalog.translate("Vfp.DbfTable.Error.TargetFieldExists") !=
            english_catalog.translate("Vfp.DbfTable.Error.TargetFieldExists"),
        "#2383: DBF table schema mutation errors should be pseudo-localizable");

    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_localized_schema_error_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "localized_schema_error.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, {{"ALPHA"}});
    expect(create_result.ok, "#2383: localized schema validation fixture should be created");

    const auto duplicate_result = copperfin::vfp::add_dbf_table_field(
        table_path.string(),
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U});
    expect(!duplicate_result.ok, "#2383: duplicate DBF fields should be rejected");
    expect(
        duplicate_result.error == "The target field already exists.",
        "#2383: duplicate DBF fields should preserve the default localized error");

    const auto drop_result = copperfin::vfp::drop_dbf_table_field(table_path.string(), "NAME");
    expect(!drop_result.ok, "#2383: dropping the last DBF field should be rejected");
    expect(
        drop_result.error == "Cannot drop the last field from a DBF table.",
        "#2383: last-field drop should preserve the default localized error");

    fs::remove_all(temp_dir, ignored);
}

void test_dbf_table_record_replacement_errors_resolve_through_localization_catalog() {
    namespace fs = std::filesystem;
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english_catalog.translate("Vfp.DbfTable.Error.TargetFieldNotFoundInTable") ==
            "The target field was not found in the table.",
        "#2384: DBF table replacement missing-field error should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Vfp.DbfTable.Error.MemoSidecarPathMissing") ==
            "No memo sidecar path could be inferred for the table.",
        "#2384: DBF table replacement memo-sidecar-path error should resolve through the en-US catalog");
    expect(
        pseudo_catalog.translate("Vfp.DbfTable.Error.TargetFieldNotFoundInTable") !=
            english_catalog.translate("Vfp.DbfTable.Error.TargetFieldNotFoundInTable"),
        "#2384: DBF table append/replacement errors should be pseudo-localizable");

    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_localized_replacement_error_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "localized_replacement_error.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, {{"ALPHA"}});
    expect(create_result.ok, "#2384: localized replacement validation fixture should be created");

    const auto missing_field_result =
        copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "MISSING", "BRAVO");
    expect(!missing_field_result.ok, "#2384: missing replacement fields should be rejected");
    expect(
        missing_field_result.error == "The target field was not found in the table.",
        "#2384: missing replacement fields should preserve the default localized error");

    fs::remove_all(temp_dir, ignored);
}

void test_dbf_table_row_header_errors_resolve_through_localization_catalog() {
    namespace fs = std::filesystem;
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english_catalog.translate("Vfp.DbfTable.Error.RequestedRecordCountTooLarge") ==
            "Requested record count exceeds current table size.",
        "#2385: DBF table requested-count error should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Vfp.DbfTable.Error.TableHeaderTruncated") ==
            "Table header is truncated.",
        "#2385: DBF table header truncation error should resolve through the en-US catalog");
    expect(
        pseudo_catalog.translate("Vfp.DbfTable.Error.RequestedRecordCountTooLarge") !=
            english_catalog.translate("Vfp.DbfTable.Error.RequestedRecordCountTooLarge"),
        "#2385: DBF table row/header mutation errors should be pseudo-localizable");

    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_localized_row_header_error_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "localized_row_header_error.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, {{"ALPHA"}});
    expect(create_result.ok, "#2385: localized row/header validation fixture should be created");

    const auto truncate_result = copperfin::vfp::truncate_dbf_table_file(table_path.string(), 2U);
    expect(!truncate_result.ok, "#2385: truncation should reject record counts above current table size");
    expect(
        truncate_result.error == "Requested record count exceeds current table size.",
        "#2385: too-large truncate count should preserve the default localized error");

    fs::remove_all(temp_dir, ignored);
}

void test_memo_sidecar_version_mismatch_is_diagnosed() {
    // GAP-02: malformed memo sidecar headers must not crash parsing.
    // If payload decoding fails, the reader should surface a stable placeholder.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_memo_version_mismatch_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "memo_mismatch.scx";
    const fs::path memo_path = temp_dir / "memo_mismatch.sct";

    std::vector<std::uint8_t> table_bytes(97U + 5U + 1U, 0U);
    table_bytes[0] = 0x30U;
    write_le_u32(table_bytes, 4U, 1U);
    write_le_u16(table_bytes, 8U, 97U);
    write_le_u16(table_bytes, 10U, 5U);
    write_field_descriptor(table_bytes, 32U, "OBJNAME", 'M', 1U, 4U);
    write_field_descriptor(table_bytes, 64U, "OBJTYPE", 'C', 5U, 1U);
    table_bytes[96U] = 0x0DU;
    table_bytes[97U] = 0x20U;
    write_le_u32(table_bytes, 98U, 1U);
    table_bytes[102U] = static_cast<std::uint8_t>('X');
    table_bytes.back() = 0x1AU;

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()),
                     static_cast<std::streamsize>(table_bytes.size()));
    }

    // Create a deliberately malformed sidecar: valid size but unusable block metadata.
    std::vector<std::uint8_t> memo_bytes(512U, 0U);
    write_be_u16(memo_bytes, 6U, 1U);  // pathological block size
    write_be_u32(memo_bytes, 5U, 0x00FFFFFFU);  // exaggerated payload length for block 1
    {
        std::ofstream output(memo_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()),
                     static_cast<std::streamsize>(memo_bytes.size()));
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok,
           "GAP-02: malformed memo sidecar metadata should not crash DBF parsing");
    if (parse_result.ok && !parse_result.table.records.empty() && !parse_result.table.records[0].values.empty()) {
        expect(parse_result.table.records[0].values[0].display_value.find("<memo block 1>") != std::string::npos,
               "GAP-02: unreadable memo payload should surface a stable placeholder diagnostic");
        expect(parse_result.table.records[0].values[0].memo_block_number == 1U,
               "#711: malformed memo payloads should retain structured block provenance");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_memo_sidecar_zero_block_size_fails_closed() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_zero_block_size_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "memo_zero_block_size.scx";
    const fs::path memo_path = temp_dir / "memo_zero_block_size.sct";

    std::vector<std::uint8_t> table_bytes(97U + 5U + 1U, 0U);
    table_bytes[0] = 0x30U;
    write_le_u32(table_bytes, 4U, 1U);
    write_le_u16(table_bytes, 8U, 97U);
    write_le_u16(table_bytes, 10U, 5U);
    write_field_descriptor(table_bytes, 32U, "OBJNAME", 'M', 1U, 4U);
    write_field_descriptor(table_bytes, 64U, "OBJTYPE", 'C', 5U, 1U);
    table_bytes[96U] = 0x0DU;
    table_bytes[97U] = 0x20U;
    write_le_u32(table_bytes, 98U, 1U);
    table_bytes[102U] = static_cast<std::uint8_t>('X');
    table_bytes.back() = 0x1AU;

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()),
                     static_cast<std::streamsize>(table_bytes.size()));
    }

    std::vector<std::uint8_t> memo_bytes(512U, 0U);
    write_be_u32(memo_bytes, 0U, 2U);
    write_be_u16(memo_bytes, 6U, 0U);
    {
        std::ofstream output(memo_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()),
                     static_cast<std::streamsize>(memo_bytes.size()));
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok,
           "#3687: zero-block-size memo sidecars should fail closed without crashing DBF parsing");
    if (parse_result.ok && !parse_result.table.records.empty() && !parse_result.table.records[0].values.empty()) {
        expect(parse_result.table.records[0].values[0].display_value.find("<memo block 1>") != std::string::npos,
               "#3687: zero-block-size memo sidecars should surface the stable unresolved memo placeholder");
        expect(parse_result.table.records[0].values[0].memo_block_number == 1U,
               "#3687: zero-block-size memo sidecars should retain structured memo provenance");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_dbf_table_locale_catalog_parity() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::vector<std::string_view> keys{
        "Vfp.DbfTable.Error.CharacterValueTooLarge",
        "Vfp.DbfTable.Error.CreateFieldRequired",
        "Vfp.DbfTable.Error.CreateUnsupportedFieldType",
        "Vfp.DbfTable.Error.CurrencyValueInvalid",
        "Vfp.DbfTable.Error.DateTimeValueInvalid",
        "Vfp.DbfTable.Error.DateValueInvalid",
        "Vfp.DbfTable.Error.DoubleFieldWidthInvalid",
        "Vfp.DbfTable.Error.DoubleValueInvalid",
        "Vfp.DbfTable.Error.DropLastField",
        "Vfp.DbfTable.Error.EightByteFieldWidthInvalid",
        "Vfp.DbfTable.Error.FieldLengthRequired",
        "Vfp.DbfTable.Error.FieldNameRequired",
        "Vfp.DbfTable.Error.FreeTableFieldNameTooLong",
        "Vfp.DbfTable.Error.HeaderLengthExceedsFile",
        "Vfp.DbfTable.Error.IntegerFieldWidthInvalid",
        "Vfp.DbfTable.Error.IntegerValueInvalid",
        "Vfp.DbfTable.Error.IntegerValueTooLarge",
        "Vfp.DbfTable.Error.LogicalValueInvalid",
        "Vfp.DbfTable.Error.MemoSidecarPathMissing",
        "Vfp.DbfTable.Error.MemoFieldWidthTooSmall",
        "Vfp.DbfTable.Error.ReadMemoPayloadFailed",
        "Vfp.DbfTable.Error.NumericValueTooLarge",
        "Vfp.DbfTable.Error.OpaqueValueInvalid",
        "Vfp.DbfTable.Error.OpenTableFailed",
        "Vfp.DbfTable.Error.RecordDataTruncated",
        "Vfp.DbfTable.Error.RecordFieldCountMismatch",
        "Vfp.DbfTable.Error.RecordIndexOutOfRange",
        "Vfp.DbfTable.Error.RecordLayoutExceedsSize",
        "Vfp.DbfTable.Error.RequestedRecordCountTooLarge",
        "Vfp.DbfTable.Error.TableDataTruncated",
        "Vfp.DbfTable.Error.TableHeaderTruncated",
        "Vfp.DbfTable.Error.TargetFieldExists",
        "Vfp.DbfTable.Error.TargetFieldNotFound",
        "Vfp.DbfTable.Error.TargetFieldNotFoundInTable",
        "Vfp.DbfTable.Error.VqFieldWidthTooSmall",
        "Vfp.DbfTable.Error.VqValueTooLarge",
        "Vfp.DbfTable.Error.WriteMemoSidecarFailed",
        "Vfp.DbfTable.Error.WriteTableFailed"};

    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", keys) == 0U,
        "#2602: es-419 should define every remaining Vfp.DbfTable localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", keys) == 0U,
        "#2602: pt-BR should define every remaining Vfp.DbfTable localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", keys) == 0U,
        "#2602: qps-ploc should define every remaining Vfp.DbfTable localization key");
}

void test_dbf_field_name_without_null_terminator_is_tolerated() {
    // GAP-02: DBF descriptors with full 11-byte field names and no null
    // terminator should parse without crashing or truncating record access.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_field_name_11byte_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "field_name_11.dbf";

    std::vector<std::uint8_t> table_bytes(65U + 12U + 1U, 0U);
    table_bytes[0] = 0x30U;
    write_le_u32(table_bytes, 4U, 1U);
    write_le_u16(table_bytes, 8U, 65U);
    write_le_u16(table_bytes, 10U, 12U);
    write_field_descriptor(table_bytes, 32U, "ELEVENCHARS", 'C', 1U, 10U);
    table_bytes[64U] = 0x0DU;
    table_bytes[65U] = 0x20U;
    write_ascii(table_bytes, 66U, "ALPHA     ");
    table_bytes.back() = 0x1AU;

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()),
                     static_cast<std::streamsize>(table_bytes.size()));
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok,
           "GAP-02: full-width field names without null terminator should parse safely");
    expect(parse_result.table.fields.size() == 1U,
           "GAP-02: parser should preserve the descriptor when no null terminator exists");
    if (parse_result.ok && parse_result.table.fields.size() == 1U) {
        expect(!parse_result.table.fields[0].name.empty(),
               "GAP-02: parsed field name should remain non-empty");
    }
    if (parse_result.ok && parse_result.table.records.size() == 1U && !parse_result.table.records[0].values.empty()) {
        expect(parse_result.table.records[0].values[0].display_value == "ALPHA",
               "GAP-02: record values should remain readable with full-width field names");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_currency_field_boundary_values() {
    // GAP-01: currency fields should accept and round-trip near int64
    // scaled bounds used by Visual FoxPro storage.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_currency_boundary_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "currency_bounds.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "ID", .type = 'N', .length = 3U},
        {.name = "BALANCE", .type = 'Y', .length = 8U}
    };
    const std::vector<std::vector<std::string>> records{{"1", "0"}, {"2", "0"}};

    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "GAP-01: setup should create a currency-backed table");

    const auto replace_max = copperfin::vfp::replace_record_field_value(
        table_path.string(), 0U, "BALANCE", "922337203685477.5807");
    expect(replace_max.ok,
           "GAP-01: max positive currency boundary should be accepted");

    const auto replace_min = copperfin::vfp::replace_record_field_value(
        table_path.string(), 1U, "BALANCE", "-922337203685477.5808");
    expect(replace_min.ok,
           "#4866: minimum negative currency boundary should be accepted");

    const auto boundary_bytes = read_binary_file(table_path);
    const auto reject_positive_overflow = copperfin::vfp::replace_record_field_value(
        table_path.string(), 0U, "BALANCE", "922337203685477.5808");
    expect(!reject_positive_overflow.ok,
           "#4866: one scaled unit above the positive currency boundary should be rejected");
    expect(read_binary_file(table_path) == boundary_bytes,
           "#4866: rejected positive currency overflow should leave the DBF byte-for-byte unchanged");

    const auto reject_negative_overflow = copperfin::vfp::replace_record_field_value(
        table_path.string(), 1U, "BALANCE", "-922337203685477.5809");
    expect(!reject_negative_overflow.ok,
           "#4866: one scaled unit below the negative currency boundary should be rejected");
    expect(read_binary_file(table_path) == boundary_bytes,
           "#4866: rejected negative currency overflow should leave the DBF byte-for-byte unchanged");

    const fs::path rejected_positive_create_path = temp_dir / "rejected_positive.dbf";
    const auto reject_positive_create = copperfin::vfp::create_dbf_table_file(
        rejected_positive_create_path.string(), fields, {{"1", "922337203685477.5808"}});
    expect(!reject_positive_create.ok && !fs::exists(rejected_positive_create_path),
           "#4866: positive currency overflow should not materialize a new DBF");

    const fs::path rejected_negative_create_path = temp_dir / "rejected_negative.dbf";
    const auto reject_negative_create = copperfin::vfp::create_dbf_table_file(
        rejected_negative_create_path.string(), fields, {{"1", "-922337203685477.5809"}});
    expect(!reject_negative_create.ok && !fs::exists(rejected_negative_create_path),
           "#4866: negative currency overflow should not materialize a new DBF");

    const std::locale comma_locale(std::locale::classic(), new comma_decimal_numpunct());
    scoped_global_locale locale_guard(comma_locale);
    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "GAP-01: currency boundary table should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 2U &&
        parse_result.table.records[0].values.size() >= 2U &&
        parse_result.table.records[1].values.size() >= 2U) {
        expect(parse_result.table.records[0].values[1].display_value == "922337203685477.5807",
               "#4839: positive currency boundary should ignore host digit grouping");
        expect(parse_result.table.records[1].values[1].display_value == "-922337203685477.5808",
               "#4866: exact negative currency minimum should ignore host digit grouping");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_nan_inf_in_double_field_round_trip_behavior() {
    // GAP-01: double-field special values should never crash the parser
    // and should produce deterministic display output after write/read.
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_double_nan_inf_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "double_specials.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "ID", .type = 'N', .length = 3U},
        {.name = "VALUE", .type = 'B', .length = 8U}
    };
    const std::vector<std::vector<std::string>> records{{"1", "0"}, {"2", "0"}, {"3", "0"}};

    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "GAP-01: setup should create a double-backed table");

    const auto write_nan = copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "VALUE", "nan");
    const auto write_pos_inf = copperfin::vfp::replace_record_field_value(table_path.string(), 1U, "VALUE", "inf");
    const auto write_neg_inf = copperfin::vfp::replace_record_field_value(table_path.string(), 2U, "VALUE", "-inf");
    expect(write_nan.ok, "GAP-01: writing NaN into a double field should be accepted");
    expect(write_pos_inf.ok, "GAP-01: writing +INF into a double field should be accepted");
    expect(write_neg_inf.ok, "GAP-01: writing -INF into a double field should be accepted");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok,
           "GAP-01: table with NaN/INF double payloads should parse without crashing");
    if (parse_result.ok && parse_result.table.records.size() == 3U &&
        parse_result.table.records[0].values.size() >= 2U &&
        parse_result.table.records[1].values.size() >= 2U &&
        parse_result.table.records[2].values.size() >= 2U) {
        expect(!parse_result.table.records[0].values[1].display_value.empty(),
               "GAP-01: NaN display output should be non-empty");
        expect(!parse_result.table.records[1].values[1].display_value.empty(),
               "GAP-01: +INF display output should be non-empty");
        expect(!parse_result.table.records[2].values[1].display_value.empty(),
               "GAP-01: -INF display output should be non-empty");
    }

    fs::remove_all(temp_dir, ignored);
}

    void test_replace_write_failure_leaves_original_dbf_intact() {
        // GAP-03: staged DBF write failures must preserve the original table bytes.
        namespace fs = std::filesystem;
        const fs::path temp_dir = fs::temp_directory_path() /
         ("copperfin_dbf_replace_write_failure_tests_" + std::to_string(_getpid()));
        std::error_code ignored;
        fs::remove_all(temp_dir, ignored);
        fs::create_directories(temp_dir);

        const fs::path table_path = temp_dir / "replace_fail.dbf";
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
         {.name = "NAME", .type = 'C', .length = 10U},
         {.name = "AGE", .type = 'N', .length = 3U}
        };
        const std::vector<std::vector<std::string>> records{{"ALPHA", "10"}};
        expect(copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records).ok,
            "GAP-03: setup should create a DBF table for staged-write rollback validation");

        const auto original_bytes = read_binary_file(table_path);

        const auto replace_result = [&]() {
            ScopedEnvironmentValue fail_path("COPPERFIN_TEST_FAIL_WRITE_PATH_CONTAINS", ".dbf");
            ScopedEnvironmentValue fail_stage("COPPERFIN_TEST_FAIL_WRITE_STAGE", "before-promote");
            return copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "NAME", "BRAVO");
        }();

        expect(!replace_result.ok,
            "GAP-03: injected DBF write failure should surface as a failed replace operation");
        expect(replace_result.error == "Unable to write table file.",
            "GAP-03: injected DBF write failure should report the table write error");

        const auto final_bytes = read_binary_file(table_path);
        expect(final_bytes == original_bytes,
            "GAP-03: table bytes should be preserved when staged DBF promote fails");
        expect(!fs::exists(table_path.string() + ".cptmp"),
            "GAP-03: staged failure should not leak DBF temp files");
        expect(!fs::exists(table_path.string() + ".cpbak"),
            "GAP-03: staged failure should not leak DBF backup files");

        fs::remove_all(temp_dir, ignored);
    }

    void test_memo_sidecar_write_failure_leaves_dbf_header_consistent() {
        // GAP-03: if memo sidecar write fails after DBF write, rollback must restore
        // both table content and sidecar consistency.
        namespace fs = std::filesystem;
        const fs::path temp_dir = fs::temp_directory_path() /
         ("copperfin_dbf_memo_write_failure_tests_" + std::to_string(_getpid()));
        std::error_code ignored;
        fs::remove_all(temp_dir, ignored);
        fs::create_directories(temp_dir);

        const fs::path table_path = temp_dir / "memo_fail.dbf";
        const fs::path memo_path = temp_dir / "memo_fail.fpt";
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
         {.name = "TITLE", .type = 'C', .length = 10U},
         {.name = "BODY", .type = 'M', .length = 4U}
        };
        const std::vector<std::vector<std::string>> records{{"ONE", "Original payload"}};
        expect(copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records).ok,
            "GAP-03: setup should create memo-backed DBF for rollback validation");

        const auto replace_result = [&]() {
            ScopedEnvironmentValue fail_path("COPPERFIN_TEST_FAIL_WRITE_PATH_CONTAINS", ".fpt");
            ScopedEnvironmentValue fail_stage("COPPERFIN_TEST_FAIL_WRITE_STAGE", "temp-open");
            return copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "BODY", "Updated payload");
        }();

        expect(!replace_result.ok,
            "GAP-03: injected memo write failure should surface as a failed replace operation");
        expect(replace_result.error == "Unable to write memo sidecar.",
            "GAP-03: injected memo write failure should report the memo-sidecar write error");

        const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
        expect(parse_result.ok,
            "GAP-03: table should remain readable after memo sidecar write rollback");
        if (parse_result.ok && parse_result.table.records.size() == 1U && parse_result.table.records[0].values.size() >= 2U) {
         expect(parse_result.table.records[0].values[1].display_value == "Original payload",
             "GAP-03: memo rollback should preserve original payload content");
        }
        expect(!fs::exists(memo_path.string() + ".cptmp"),
            "GAP-03: failed memo write should not leak memo temp files");
        expect(!fs::exists(memo_path.string() + ".cpbak"),
            "GAP-03: failed memo write should not leak memo backup files");

        fs::remove_all(temp_dir, ignored);
    }

    void test_staged_write_rollback_removes_temp_and_preserves_original() {
        // GAP-03: staged-write rollback should preserve original on-disk state and
        // clean temp/backup artifacts.
        namespace fs = std::filesystem;
        const fs::path temp_dir = fs::temp_directory_path() /
         ("copperfin_dbf_staged_rollback_tests_" + std::to_string(_getpid()));
        std::error_code ignored;
        fs::remove_all(temp_dir, ignored);
        fs::create_directories(temp_dir);

        const fs::path table_path = temp_dir / "rollback.dbf";
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
         {.name = "NAME", .type = 'C', .length = 10U},
         {.name = "AGE", .type = 'N', .length = 3U}
        };
        const std::vector<std::vector<std::string>> records{{"ALPHA", "10"}, {"BRAVO", "20"}};
        expect(copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records).ok,
            "GAP-03: setup should create table for staged-write rollback checks");

        const auto before = read_binary_file(table_path);
        const auto result = [&]() {
            ScopedEnvironmentValue fail_path("COPPERFIN_TEST_FAIL_WRITE_PATH_CONTAINS", "rollback.dbf");
            ScopedEnvironmentValue fail_stage("COPPERFIN_TEST_FAIL_WRITE_STAGE", "before-promote");
            return copperfin::vfp::replace_record_field_value(table_path.string(), 1U, "AGE", "21");
        }();

        expect(!result.ok,
            "GAP-03: injected staged promote failure should return a failed write result");

        const auto after = read_binary_file(table_path);
        expect(after == before,
            "GAP-03: staged rollback should preserve original table bytes exactly");
        expect(!fs::exists(table_path.string() + ".cptmp"),
            "GAP-03: staged rollback should remove DBF temp artifacts");
        expect(!fs::exists(table_path.string() + ".cpbak"),
            "GAP-03: staged rollback should remove DBF backup artifacts");

        fs::remove_all(temp_dir, ignored);
    }

    void test_dbf_with_zero_record_length_is_rejected() {
        // DBF header validation: record_length==0 must be rejected.
        namespace fs = std::filesystem;
        const fs::path temp_dir = fs::temp_directory_path() /
         ("copperfin_dbf_zero_record_length_tests_" + std::to_string(_getpid()));
        std::error_code ignored;
        fs::remove_all(temp_dir, ignored);
        fs::create_directories(temp_dir);

        const fs::path table_path = temp_dir / "zero_record_len.dbf";
        std::vector<std::uint8_t> bytes(32U, 0U);
        bytes[0] = 0x30U;
        write_le_u16(bytes, 8U, 32U);
        write_le_u16(bytes, 10U, 0U);
        {
         std::ofstream output(table_path, std::ios::binary);
         output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        }

        const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
        expect(!parse_result.ok,
            "DBF header validation should reject zero record length");

        fs::remove_all(temp_dir, ignored);
    }

    void test_dbf_with_header_shorter_than_minimum_is_rejected() {
        // DBF header validation: header_length<32 must fail DBF-family header validation.
        namespace fs = std::filesystem;
        const fs::path temp_dir = fs::temp_directory_path() /
         ("copperfin_dbf_short_header_tests_" + std::to_string(_getpid()));
        std::error_code ignored;
        fs::remove_all(temp_dir, ignored);
        fs::create_directories(temp_dir);

        const fs::path table_path = temp_dir / "short_header.dbf";
        std::vector<std::uint8_t> bytes(32U, 0U);
        bytes[0] = 0x30U;
        write_le_u16(bytes, 8U, 31U);
        write_le_u16(bytes, 10U, 1U);
        {
         std::ofstream output(table_path, std::ios::binary);
         output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        }

        const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
        expect(!parse_result.ok,
            "DBF header validation should reject header length below 32 bytes");

        fs::remove_all(temp_dir, ignored);
    }

    void test_dbf_header_claim_beyond_file_size_is_rejected() {
        // DBF header validation: header_length claims beyond physical file bytes must be rejected.
        namespace fs = std::filesystem;
        const fs::path temp_dir = fs::temp_directory_path() /
         ("copperfin_dbf_header_claim_beyond_file_tests_" + std::to_string(_getpid()));
        std::error_code ignored;
        fs::remove_all(temp_dir, ignored);
        fs::create_directories(temp_dir);

        const fs::path table_path = temp_dir / "header_claim_beyond_file.dbf";
        std::vector<std::uint8_t> bytes(64U, 0U);
        bytes[0] = 0x30U;
        write_le_u16(bytes, 8U, 97U);
        write_le_u16(bytes, 10U, 14U);
        {
         std::ofstream output(table_path, std::ios::binary);
         output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        }

        const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
        expect(!parse_result.ok,
            "DBF header validation should reject header claims beyond file size");
        expect(parse_result.error == "Table file is shorter than its header length.",
            "DBF header validation should report header-length truncation errors");

        fs::remove_all(temp_dir, ignored);
    }

void test_staged_write_temp_artifacts_are_cleaned_up() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_staged_write_cleanup_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "cleanup.dbf";
    const fs::path memo_path = temp_dir / "cleanup.fpt";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "TITLE", .type = 'C', .length = 12U},
        {.name = "BODY", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{{"FIRST", "Payload"}};

    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "setup should create memo-backed table for staged-write cleanup tests");

    const auto replace_result = copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "BODY", "Payload after staged write");
    expect(replace_result.ok, "memo replacement should succeed under staged-write path");

    expect(!fs::exists(table_path.string() + ".cptmp"), "staged DBF write should remove temporary files");
    expect(!fs::exists(table_path.string() + ".cpbak"), "staged DBF write should remove backup files");
    expect(!fs::exists(memo_path.string() + ".cptmp"), "staged memo write should remove temporary files");
    expect(!fs::exists(memo_path.string() + ".cpbak"), "staged memo write should remove backup files");

    fs::remove_all(temp_dir, ignored);
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) == "--active-locale-missing-field") {
        test_missing_field_diagnostic_uses_active_locale();
        if (failures != 0) {
            std::cerr << failures << " test(s) failed.\n";
            return EXIT_FAILURE;
        }
        std::cout << "All tests passed.\n";
        return EXIT_SUCCESS;
    }

    test_parse_dbf_table_with_memo_sidecar();
    test_mutate_and_append_dbf_table();
    test_create_dbf_table_file_round_trips();
    test_dbf_mutations_stamp_last_update_date();
    test_character_and_varchar_fields_preserve_leading_whitespace_on_write();
    test_string_fields_store_literal_null_text();
    test_character_fields_stop_at_nul_padding();
    test_create_dbf_table_file_rejects_duplicate_field_names();
    test_dbf_schema_writes_enforce_field_name_boundaries();
    test_record_field_updates_match_descriptor_names_case_insensitively();
    copperfin::test_dbf_table::test_memo_field_create_replace_and_append_round_trip();
    copperfin::test_dbf_table::test_general_and_picture_memo_fields_round_trip();
    copperfin::test_dbf_table::test_memo_payload_that_decodes_empty_stays_empty();
    copperfin::test_dbf_table::test_pack_memo_preserves_payloads_that_decode_empty();
    copperfin::test_dbf_table::test_pack_memo_preserves_binary_picture_payloads();
    copperfin::test_dbf_table::test_pack_memo_fails_when_referenced_payload_cannot_be_recovered();
    copperfin::test_dbf_table::test_additive_memo_replace_preserves_raw_payload_and_fails_closed();
    test_schema_rewrites_preserve_raw_memo_and_unaffected_field_bytes();
    test_schema_rewrites_preserve_code_page_marks();
    test_indexed_table_mutations_succeed_with_production_flags_and_companions();
    test_integer_field_create_replace_and_append_round_trip();
    test_currency_and_datetime_field_round_trip();
    test_currency_display_ignores_grouping_locale();
    test_double_field_create_replace_and_append_round_trip();
    test_append_blank_supports_opaque_field_layouts();
    test_replace_opaque_field_round_trips_hex_payloads();
    test_parse_dbf_table_rejects_truncated_visual_asset();
    test_visual_asset_memo_sidecar_repair_round_trip();
    test_memo_replace_recovers_directory_sidecar_path();
    test_replace_field_value_accepts_null_token_for_nonstring_types();
    test_varchar_and_varbinary_field_round_trip();
    test_dbf_header_record_count_exceeds_file_size_is_rejected();
    test_dbf_field_descriptor_count_exceeds_header_size_is_rejected();
    test_dbf_record_width_mismatch_field_sum_is_rejected();
    test_dbf_table_record_value_errors_resolve_through_localization_catalog();
    test_dbf_table_default_catalog_refreshes_when_locale_changes();
    test_dbf_table_creation_errors_resolve_through_localization_catalog();
    test_dbf_table_schema_mutation_errors_resolve_through_localization_catalog();
    test_dbf_table_record_replacement_errors_resolve_through_localization_catalog();
    test_dbf_table_row_header_errors_resolve_through_localization_catalog();
    test_memo_sidecar_version_mismatch_is_diagnosed();
    test_memo_sidecar_zero_block_size_fails_closed();
    test_dbf_table_locale_catalog_parity();
    test_dbf_field_name_without_null_terminator_is_tolerated();
    test_currency_field_boundary_values();
    test_nan_inf_in_double_field_round_trip_behavior();
    test_replace_write_failure_leaves_original_dbf_intact();
    test_memo_sidecar_write_failure_leaves_dbf_header_consistent();
    test_staged_write_rollback_removes_temp_and_preserves_original();
    test_dbf_with_zero_record_length_is_rejected();
    test_dbf_with_header_shorter_than_minimum_is_rejected();
    test_dbf_header_claim_beyond_file_size_is_rejected();
    test_staged_write_temp_artifacts_are_cleaned_up();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
