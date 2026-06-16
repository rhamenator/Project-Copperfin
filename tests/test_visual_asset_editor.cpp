#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/visual_asset_editor.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <cmath>
#include <limits>
#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#define _getpid getpid
#endif
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

double parse_number(const std::string& text) {
    try {
        return std::stod(text);
    } catch (...) {
        return std::numeric_limits<double>::quiet_NaN();
    }
}

const copperfin::vfp::DbfRecordValue* find_record_field(
    const copperfin::vfp::DbfRecord& record,
    const std::string& field_name) {
    const auto value = std::find_if(record.values.begin(), record.values.end(), [&](const auto& candidate) {
        return candidate.field_name == field_name;
    });
    return value == record.values.end() ? nullptr : &(*value);
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

std::vector<std::uint8_t> read_file_bytes(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

void write_field_descriptor(
    std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    const std::string& name,
    char type,
    std::uint32_t field_offset,
    std::uint8_t field_length) {
    write_ascii(bytes, offset, name);
    bytes[offset + 11U] = static_cast<std::uint8_t>(type);
    write_le_u32(bytes, offset + 12U, field_offset);
    bytes[offset + 16U] = field_length;
}

void write_synthetic_direct_and_memo_asset(
    const std::filesystem::path& table_path,
    const std::filesystem::path& memo_path,
    const std::string& direct_field_name,
    const std::string& direct_field_value,
    const std::string& memo_field_name,
    const std::string& memo_field_value) {
    std::vector<std::uint8_t> table_bytes(122U, 0U);
    table_bytes[0] = 0x30U;
    table_bytes[1] = 126U;
    table_bytes[2] = 4U;
    table_bytes[3] = 7U;
    write_le_u32(table_bytes, 4U, 1U);
    write_le_u16(table_bytes, 8U, 97U);
    write_le_u16(table_bytes, 10U, 25U);
    table_bytes[28] = 0x00U;
    table_bytes[29] = 0x03U;

    write_field_descriptor(table_bytes, 32U, direct_field_name, 'C', 1U, 20U);
    write_field_descriptor(table_bytes, 64U, memo_field_name, 'M', 21U, 4U);
    table_bytes[96] = 0x0DU;

    table_bytes[97] = 0x20U;
    std::fill_n(table_bytes.begin() + 98U, 20U, static_cast<std::uint8_t>(' '));
    write_ascii(table_bytes, 98U, direct_field_value);
    write_le_u32(table_bytes, 118U, 1U);

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }

    std::vector<std::uint8_t> memo_bytes(1024U, 0U);
    write_be_u32(memo_bytes, 0U, 2U);
    write_be_u16(memo_bytes, 6U, 512U);
    memo_bytes[512 + 3] = 1U;
    write_be_u32(memo_bytes, 512 + 4, static_cast<std::uint32_t>(memo_field_value.size()));
    write_ascii(memo_bytes, 520U, memo_field_value);

    {
        std::ofstream output(memo_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }
}

struct SyntheticNamedVisualObject {
    std::string objname;
    std::string name;
    std::string unique_id;
    std::string properties;
};

void write_synthetic_named_object_asset(
    const std::filesystem::path& table_path,
    const std::filesystem::path& memo_path,
    const std::vector<SyntheticNamedVisualObject>& objects) {
    constexpr std::size_t header_length = 161U;
    constexpr std::size_t record_length = 37U;
    std::vector<std::uint8_t> table_bytes(header_length + (record_length * objects.size()) + 1U, 0U);
    table_bytes[0] = 0x30U;
    table_bytes[1] = 126U;
    table_bytes[2] = 4U;
    table_bytes[3] = 7U;
    write_le_u32(table_bytes, 4U, static_cast<std::uint32_t>(objects.size()));
    write_le_u16(table_bytes, 8U, static_cast<std::uint16_t>(header_length));
    write_le_u16(table_bytes, 10U, static_cast<std::uint16_t>(record_length));
    table_bytes[28] = 0x00U;
    table_bytes[29] = 0x03U;

    write_field_descriptor(table_bytes, 32U, "OBJNAME", 'M', 1U, 4U);
    write_field_descriptor(table_bytes, 64U, "NAME", 'C', 5U, 24U);
    write_field_descriptor(table_bytes, 96U, "UNIQUEID", 'M', 29U, 4U);
    write_field_descriptor(table_bytes, 128U, "PROPERTIES", 'M', 33U, 4U);
    table_bytes[160] = 0x0DU;
    table_bytes[header_length + (record_length * objects.size())] = 0x1AU;

    std::vector<std::uint8_t> memo_bytes(512U * (objects.size() * 3U + 2U), 0U);
    write_be_u16(memo_bytes, 6U, 512U);
    std::uint32_t next_block = 1U;
    auto write_memo = [&](const std::string& value) {
        const std::uint32_t block = next_block++;
        const std::size_t offset = static_cast<std::size_t>(block) * 512U;
        memo_bytes[offset + 3U] = 1U;
        write_be_u32(memo_bytes, offset + 4U, static_cast<std::uint32_t>(value.size()));
        write_ascii(memo_bytes, offset + 8U, value);
        return block;
    };

    for (std::size_t index = 0; index < objects.size(); ++index) {
        const auto& object = objects[index];
        const std::size_t record_offset = header_length + (record_length * index);
        table_bytes[record_offset] = 0x20U;
        if (!object.objname.empty()) {
            write_le_u32(table_bytes, record_offset + 1U, write_memo(object.objname));
        }
        std::fill_n(table_bytes.begin() + static_cast<std::ptrdiff_t>(record_offset + 5U), 24U, static_cast<std::uint8_t>(' '));
        write_ascii(table_bytes, record_offset + 5U, object.name);
        if (!object.unique_id.empty()) {
            write_le_u32(table_bytes, record_offset + 29U, write_memo(object.unique_id));
        }
        if (!object.properties.empty()) {
            write_le_u32(table_bytes, record_offset + 33U, write_memo(object.properties));
        }
    }
    write_be_u32(memo_bytes, 0U, next_block);

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }
    {
        std::ofstream output(memo_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }
}

void write_synthetic_named_direct_asset(
    const std::filesystem::path& table_path,
    const std::filesystem::path& memo_path) {
    constexpr std::size_t header_length = 129U;
    constexpr std::size_t record_length = 39U;
    constexpr std::size_t record_count = 2U;
    std::vector<std::uint8_t> table_bytes(header_length + (record_length * record_count) + 1U, 0U);
    table_bytes[0] = 0x30U;
    table_bytes[1] = 126U;
    table_bytes[2] = 4U;
    table_bytes[3] = 7U;
    write_le_u32(table_bytes, 4U, static_cast<std::uint32_t>(record_count));
    write_le_u16(table_bytes, 8U, static_cast<std::uint16_t>(header_length));
    write_le_u16(table_bytes, 10U, static_cast<std::uint16_t>(record_length));
    table_bytes[28] = 0x00U;
    table_bytes[29] = 0x03U;

    write_field_descriptor(table_bytes, 32U, "OBJNAME", 'M', 1U, 4U);
    write_field_descriptor(table_bytes, 64U, "NAME", 'C', 5U, 24U);
    write_field_descriptor(table_bytes, 96U, "HPOS", 'N', 29U, 10U);
    table_bytes[128] = 0x0DU;
    table_bytes[header_length + (record_length * record_count)] = 0x1AU;

    std::vector<std::uint8_t> memo_bytes(1536U, 0U);
    write_be_u32(memo_bytes, 0U, 3U);
    write_be_u16(memo_bytes, 6U, 512U);
    memo_bytes[512U + 3U] = 1U;
    write_be_u32(memo_bytes, 512U + 4U, 7U);
    write_ascii(memo_bytes, 520U, "cmdSave");
    memo_bytes[1024U + 3U] = 1U;
    write_be_u32(memo_bytes, 1024U + 4U, 7U);
    write_ascii(memo_bytes, 1032U, "txtName");

    const auto write_record = [&](std::size_t record_index, std::uint32_t objname_block, const std::string& name, const std::string& hpos) {
        const std::size_t record_offset = header_length + (record_length * record_index);
        table_bytes[record_offset] = 0x20U;
        write_le_u32(table_bytes, record_offset + 1U, objname_block);
        std::fill_n(table_bytes.begin() + static_cast<std::ptrdiff_t>(record_offset + 5U), 24U, static_cast<std::uint8_t>(' '));
        write_ascii(table_bytes, record_offset + 5U, name);
        std::fill_n(table_bytes.begin() + static_cast<std::ptrdiff_t>(record_offset + 29U), 10U, static_cast<std::uint8_t>(' '));
        write_ascii(table_bytes, record_offset + 29U + (10U - hpos.size()), hpos);
    };
    write_record(0U, 1U, "saveButton", "111.000");
    write_record(1U, 2U, "nameBox", "222.000");

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }
    {
        std::ofstream output(memo_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }
}

void write_synthetic_named_geometry_asset(
    const std::filesystem::path& table_path,
    const std::filesystem::path& memo_path) {
    constexpr std::size_t header_length = 193U;
    constexpr std::size_t record_length = 53U;
    constexpr std::size_t record_count = 2U;
    std::vector<std::uint8_t> table_bytes(header_length + (record_length * record_count) + 1U, 0U);
    table_bytes[0] = 0x30U;
    table_bytes[1] = 126U;
    table_bytes[2] = 4U;
    table_bytes[3] = 7U;
    write_le_u32(table_bytes, 4U, static_cast<std::uint32_t>(record_count));
    write_le_u16(table_bytes, 8U, static_cast<std::uint16_t>(header_length));
    write_le_u16(table_bytes, 10U, static_cast<std::uint16_t>(record_length));
    table_bytes[28] = 0x00U;
    table_bytes[29] = 0x03U;

    write_field_descriptor(table_bytes, 32U, "OBJNAME", 'M', 1U, 4U);
    write_field_descriptor(table_bytes, 64U, "NAME", 'C', 5U, 24U);
    write_field_descriptor(table_bytes, 96U, "UNIQUEID", 'M', 29U, 4U);
    write_field_descriptor(table_bytes, 128U, "HPOS", 'N', 33U, 10U);
    write_field_descriptor(table_bytes, 160U, "VPOS", 'N', 43U, 10U);
    table_bytes[192] = 0x0DU;
    table_bytes[header_length + (record_length * record_count)] = 0x1AU;

    std::vector<std::uint8_t> memo_bytes(2560U, 0U);
    write_be_u32(memo_bytes, 0U, 5U);
    write_be_u16(memo_bytes, 6U, 512U);
    const auto write_memo = [&](std::uint32_t block, const std::string& value) {
        const std::size_t offset = static_cast<std::size_t>(block) * 512U;
        memo_bytes[offset + 3U] = 1U;
        write_be_u32(memo_bytes, offset + 4U, static_cast<std::uint32_t>(value.size()));
        write_ascii(memo_bytes, offset + 8U, value);
    };
    write_memo(1U, "cmdSave");
    write_memo(2U, "first-guid");
    write_memo(3U, "txtName");
    write_memo(4U, "target-guid");

    const auto write_record = [&](
        std::size_t record_index,
        std::uint32_t objname_block,
        const std::string& name,
        std::uint32_t unique_id_block,
        const std::string& hpos,
        const std::string& vpos) {
        const std::size_t record_offset = header_length + (record_length * record_index);
        table_bytes[record_offset] = 0x20U;
        write_le_u32(table_bytes, record_offset + 1U, objname_block);
        std::fill_n(table_bytes.begin() + static_cast<std::ptrdiff_t>(record_offset + 5U), 24U, static_cast<std::uint8_t>(' '));
        write_ascii(table_bytes, record_offset + 5U, name);
        write_le_u32(table_bytes, record_offset + 29U, unique_id_block);
        std::fill_n(table_bytes.begin() + static_cast<std::ptrdiff_t>(record_offset + 33U), 10U, static_cast<std::uint8_t>(' '));
        write_ascii(table_bytes, record_offset + 33U + (10U - hpos.size()), hpos);
        std::fill_n(table_bytes.begin() + static_cast<std::ptrdiff_t>(record_offset + 43U), 10U, static_cast<std::uint8_t>(' '));
        write_ascii(table_bytes, record_offset + 43U + (10U - vpos.size()), vpos);
    };
    write_record(0U, 1U, "saveButton", 2U, "111.000", "211.000");
    write_record(1U, 3U, "nameBox", 4U, "222.000", "322.000");

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }
    {
        std::ofstream output(memo_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }
}

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

    fs::remove_all(temp_dir, ignored);
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
    write_ascii(table_bytes, 164U, "  7812.5");
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

}  // namespace

int main() {
    test_update_visual_object_property_rewrites_properties_memo();
    test_update_visual_object_properties_updates_selected_geometry_fields();
    test_update_visual_object_property_skips_noop_writes();
    test_update_visual_object_property_targets_selected_object_name();
    test_update_visual_object_property_targets_selected_unique_id();
    test_update_visual_object_property_matches_property_names_case_insensitively();
    test_update_visual_object_property_targets_selected_object_name_direct_field();
    test_update_visual_object_property_rewrites_direct_fields();
    test_update_visual_object_property_round_trips_added_vcx_property();
    test_update_visual_object_property_round_trips_label_and_menu_assets();
    test_update_visual_object_property_round_trips_project_and_database_assets();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
