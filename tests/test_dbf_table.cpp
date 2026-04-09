#include "copperfin/vfp/dbf_table.h"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <process.h>
#include <string>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void write_le_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
}

void write_le_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

void write_be_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>(value & 0xFFU);
}

void write_be_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>(value & 0xFFU);
}

void write_ascii(std::vector<std::uint8_t>& bytes, std::size_t offset, const std::string& value) {
    for (std::size_t index = 0; index < value.size(); ++index) {
        bytes[offset + index] = static_cast<std::uint8_t>(value[index]);
    }
}

void write_field_descriptor(
    std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    const std::string& name,
    char type,
    std::uint32_t field_offset,
    std::uint8_t field_length,
    std::uint8_t decimal_count = 0U) {
    write_ascii(bytes, offset, name);
    bytes[offset + 11U] = static_cast<std::uint8_t>(type);
    write_le_u32(bytes, offset + 12U, field_offset);
    bytes[offset + 16U] = field_length;
    bytes[offset + 17U] = decimal_count;
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
        expect(result.table.records[0].values[1].display_value == "Textbox", "character values should be trimmed");
    }

    fs::remove(table_path, ignored);
    fs::remove(memo_path, ignored);
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

}  // namespace

int main() {
    test_parse_dbf_table_with_memo_sidecar();
    test_mutate_and_append_dbf_table();
    test_create_dbf_table_file_round_trips();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
