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

const copperfin::vfp::VisualObjectPropertySnapshot* find_property_snapshot(
    const std::vector<copperfin::vfp::VisualObjectPropertySnapshot>& properties,
    const std::string& property_name) {
    const auto value = std::find_if(properties.begin(), properties.end(), [&](const auto& candidate) {
        return candidate.property_name == property_name;
    });
    return value == properties.end() ? nullptr : &(*value);
}

const copperfin::vfp::VisualObjectMethodSnapshot* find_method_snapshot(
    const std::vector<copperfin::vfp::VisualObjectMethodSnapshot>& methods,
    const std::string& method_name) {
    const auto value = std::find_if(methods.begin(), methods.end(), [&](const auto& candidate) {
        return candidate.method_name == method_name;
    });
    return value == methods.end() ? nullptr : &(*value);
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

void test_rename_visual_object_memo_property_updates_selected_object() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_property_rename_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "property_rename.scx";
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
        {"dupObj", "dupName", "dup-guid", "333", "Caption = \"First\"\r\ncaption = \"Second\"\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#769: property-rename fixture should be writable");

    auto rename_result = copperfin::vfp::rename_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "caption",
        .new_property_name = "DisplayCaption"
    });
    expect(rename_result.ok, "#769: property rename should support UNIQUEID selection and case-insensitive source matching");

    rename_result = copperfin::vfp::rename_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .property_name = "Top",
        .new_property_name = "TopOffset"
    });
    expect(rename_result.ok, "#769: property rename should support object-name selection");

    rename_result = copperfin::vfp::rename_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "Left",
        .new_property_name = "LeftOffset"
    });
    expect(rename_result.ok, "#769: property rename should support record-index selection");

    auto display_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "DisplayCaption"
    });
    auto old_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption"
    });
    auto left_offset = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "LeftOffset"
    });
    auto top_offset = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "TopOffset"
    });
    auto unrelated_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "Caption"
    });
    expect(display_caption.ok && display_caption.exists && display_caption.value == "\"Save\"" &&
            old_caption.ok && !old_caption.exists &&
            left_offset.ok && left_offset.exists && left_offset.value == "10" &&
            top_offset.ok && top_offset.exists && top_offset.value == "30" &&
            unrelated_caption.ok && unrelated_caption.exists && unrelated_caption.value == "\"Name\"",
        "#769: property rename should preserve values and unrelated assignments while removing old assignment names");

    rename_result = copperfin::vfp::rename_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "HPOS",
        .new_property_name = "HPosition"
    });
    expect(!rename_result.ok, "#769: property rename should reject direct DBF-backed fields");

    rename_result = copperfin::vfp::rename_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "MissingProp",
        .new_property_name = "MissingRenamed"
    });
    expect(!rename_result.ok, "#769: property rename should reject missing source properties");

    rename_result = copperfin::vfp::rename_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "DisplayCaption",
        .new_property_name = "LeftOffset"
    });
    expect(!rename_result.ok, "#769: property rename should reject target-name collisions");

    rename_result = copperfin::vfp::rename_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "DisplayCaption",
        .new_property_name = "displaycaption"
    });
    expect(!rename_result.ok, "#769: property rename should reject same-name renames case-insensitively");

    rename_result = copperfin::vfp::rename_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = " ",
        .new_property_name = "EmptySource"
    });
    expect(!rename_result.ok, "#769: property rename should reject empty source names");

    rename_result = copperfin::vfp::rename_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "DisplayCaption",
        .new_property_name = " "
    });
    expect(!rename_result.ok, "#769: property rename should reject empty target names");

    rename_result = copperfin::vfp::rename_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dup-guid",
        .property_name = "caption",
        .new_property_name = "DuplicateCaption"
    });
    expect(!rename_result.ok, "#769: property rename should reject duplicate source assignments");

    display_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "DisplayCaption"
    });
    left_offset = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "LeftOffset"
    });
    auto duplicate_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dup-guid",
        .property_name = "DuplicateCaption"
    });
    expect(display_caption.ok && display_caption.exists && display_caption.value == "\"Save\"" &&
            left_offset.ok && left_offset.exists && left_offset.value == "10" &&
            duplicate_caption.ok && !duplicate_caption.exists,
        "#769: rejected property renames should not mutate selected object properties");

    const fs::path no_properties_path = temp_dir / "property_rename_missing_properties.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> no_properties_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> no_properties_records{
        {"cmdSave", "save-guid", "111"}
    };
    const auto no_properties_create = copperfin::vfp::create_dbf_table_file(
        no_properties_path.string(),
        no_properties_fields,
        no_properties_records);
    expect(no_properties_create.ok, "#769: missing-PROPERTIES fixture should be writable");
    rename_result = copperfin::vfp::rename_visual_object_property({
        .path = no_properties_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption",
        .new_property_name = "DisplayCaption"
    });
    expect(!rename_result.ok, "#769: property rename should reject objects without PROPERTIES memo fields");

    for (int index = 0; index < 3; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#769: undo should restore each successful memo property rename");
    }

    display_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "DisplayCaption"
    });
    old_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption"
    });
    left_offset = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "LeftOffset"
    });
    top_offset = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "TopOffset"
    });
    auto original_top = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "Top"
    });
    expect(display_caption.ok && !display_caption.exists &&
            old_caption.ok && old_caption.exists && old_caption.value == "\"Save\"" &&
            left_offset.ok && !left_offset.exists &&
            top_offset.ok && !top_offset.exists &&
            original_top.ok && original_top.exists && original_top.value == "30",
        "#769: undo should restore original memo property names and values");

    fs::remove_all(temp_dir, ignored);
}

void test_reorder_visual_object_memo_properties_within_selected_object() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_property_reorder_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "property_reorder.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "111", "Caption = \"Save\"\r\nLeft = 10\r\nTop = 20\r\nWidth = 80\r\n"},
        {"txtName", "nameBox", "name-guid", "222", "Caption = \"Name\"\r\nLeft = 30\r\n"},
        {"dupObj", "dupName", "dup-guid", "333", "Caption = \"First\"\r\ncaption = \"Second\"\r\nAnchor = 0\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#770: property-reorder fixture should be writable");

    const auto memo_property_order = [&](const std::string& unique_id) {
        std::vector<std::string> names;
        const auto properties = copperfin::vfp::list_visual_object_properties({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id
        });
        if (!properties.ok) {
            return names;
        }
        for (const auto& property : properties.properties) {
            if (!property.direct_field) {
                names.push_back(property.property_name);
            }
        }
        return names;
    };
    const auto order_is = [](const std::vector<std::string>& names, std::initializer_list<const char*> expected) {
        if (names.size() != expected.size()) {
            return false;
        }
        auto name_it = names.begin();
        auto expected_it = expected.begin();
        for (; name_it != names.end(); ++name_it, ++expected_it) {
            if (*name_it != *expected_it) {
                return false;
            }
        }
        return true;
    };

    auto reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "width",
        .placement = "first",
        .relative_property_name = {}
    });
    expect(reorder_result.ok, "#770: property reorder should support UNIQUEID selection and first placement");
    expect(order_is(memo_property_order("save-guid"), {"Width", "Caption", "Left", "Top"}),
        "#770: first placement should move the requested memo property to the start");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdSave",
        .unique_id = {},
        .property_name = "WIDTH",
        .placement = "last",
        .relative_property_name = {}
    });
    expect(reorder_result.ok, "#770: property reorder should support object-name selection and last placement");
    expect(order_is(memo_property_order("save-guid"), {"Caption", "Left", "Top", "Width"}),
        "#770: last placement should move the requested memo property to the end");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "Caption",
        .placement = "after",
        .relative_property_name = "Top"
    });
    expect(reorder_result.ok, "#770: property reorder should support record-index selection and after placement");
    expect(order_is(memo_property_order("save-guid"), {"Left", "Top", "Caption", "Width"}),
        "#770: after placement should move the requested memo property after the relative property");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption",
        .placement = "before",
        .relative_property_name = "Left"
    });
    expect(reorder_result.ok, "#770: property reorder should support before placement");
    expect(order_is(memo_property_order("save-guid"), {"Caption", "Left", "Top", "Width"}),
        "#770: before placement should move the requested memo property before the relative property");

    auto caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption"
    });
    auto left = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Left"
    });
    auto width = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Width"
    });
    expect(caption.ok && caption.exists && caption.value == "\"Save\"" &&
            left.ok && left.exists && left.value == "10" &&
            width.ok && width.exists && width.value == "80",
        "#770: property reorder should preserve memo property names and values");
    expect(order_is(memo_property_order("name-guid"), {"Caption", "Left"}),
        "#770: property reorder should preserve unrelated object PROPERTIES memos");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "HPOS",
        .placement = "first",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject direct DBF-backed source fields");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption",
        .placement = "before",
        .relative_property_name = "HPOS"
    });
    expect(!reorder_result.ok, "#770: property reorder should reject direct DBF-backed relative fields");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Missing",
        .placement = "first",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject missing source properties");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption",
        .placement = "before",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject missing relative properties for before placement");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption",
        .placement = "after",
        .relative_property_name = "caption"
    });
    expect(!reorder_result.ok, "#770: property reorder should reject self-relative before/after placement");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption",
        .placement = "middle",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject unknown placements");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = " ",
        .placement = "first",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject empty source names");

    expect(order_is(memo_property_order("save-guid"), {"Caption", "Left", "Top", "Width"}),
        "#770: failed property reorders should not mutate the PROPERTIES memo");

    for (int index = 0; index < 4; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#770: undo should restore each successful property reorder");
    }
    expect(order_is(memo_property_order("save-guid"), {"Caption", "Left", "Top", "Width"}),
        "#770: undo should restore original memo property ordering");

    const fs::path duplicate_path = temp_dir / "property_reorder_duplicate.scx";
    const auto duplicate_create_result = copperfin::vfp::create_dbf_table_file(
        duplicate_path.string(),
        fields,
        {{"dupObj", "dupName", "dup-guid", "333", "Caption = \"First\"\r\ncaption = \"Second\"\r\nAnchor = 0\r\n"}});
    expect(duplicate_create_result.ok, "#770: duplicate property-reorder fixture should be writable");
    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = duplicate_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dup-guid",
        .property_name = "CAPTION",
        .placement = "first",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject duplicate source assignments as ambiguous");
    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = duplicate_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dup-guid",
        .property_name = "Anchor",
        .placement = "before",
        .relative_property_name = "Caption"
    });
    expect(!reorder_result.ok, "#770: property reorder should reject duplicate relative assignments as ambiguous");

    const fs::path no_properties_path = temp_dir / "property_reorder_no_properties.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> no_properties_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U}
    };
    const auto no_properties_create_result = copperfin::vfp::create_dbf_table_file(
        no_properties_path.string(),
        no_properties_fields,
        {{"cmdNoProps", "no-props-guid"}});
    expect(no_properties_create_result.ok, "#770: missing-PROPERTIES property-reorder fixture should be writable");
    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = no_properties_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "no-props-guid",
        .property_name = "Caption",
        .placement = "first",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject missing PROPERTIES fields");

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
    expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
        "#737: visual property lists should not create undo history");

    fs::remove_all(temp_dir, ignored);
}

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

    parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
    expect(parse_result.ok, "#741: deleted-state fixture should remain readable after failed selection");
    if (parse_result.ok && parse_result.table.records.size() == 2U) {
        expect(!parse_result.table.records[0].deleted && !parse_result.table.records[1].deleted,
            "#741: failed deleted-state selection should preserve all record flags");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_list_visual_objects_reads_selection_outline() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_object_outline_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "outline.scx";
    const fs::path memo_path = temp_dir / "outline.sct";
    write_synthetic_named_object_asset(table_path, memo_path, {
        {
            .objname = "cmdSave",
            .name = "saveButton",
            .unique_id = "save-guid",
            .properties = "Caption = \"Save\"\r\n"
        },
        {
            .objname = "",
            .name = "fallbackButton",
            .unique_id = "fallback-guid",
            .properties = "Caption = \"Fallback\"\r\n"
        }
    });

    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "fallback-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#743: object outline fixtures should allow marking one row deleted");

    const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok, "#743: visual object outlines should list DBF-family visual assets");
    expect(list_result.objects.size() == 2U, "#743: visual object outlines should include each object record");
    if (list_result.objects.size() == 2U) {
        expect(list_result.objects[0].record_index == 0U && !list_result.objects[0].deleted,
            "#743: visual object outlines should expose live record identity");
        expect(list_result.objects[0].object_name == "cmdSave",
            "#743: visual object outlines should prefer OBJNAME over NAME");
        expect(list_result.objects[0].unique_id == "save-guid",
            "#743: visual object outlines should expose stable UNIQUEID values");
        expect(list_result.objects[0].caption == "\"Save\"",
            "#745: visual object outlines should expose parsed Caption values");
        expect(list_result.objects[1].record_index == 1U && list_result.objects[1].deleted,
            "#743: visual object outlines should keep deleted records visible");
        expect(list_result.objects[1].object_name == "fallbackButton",
            "#743: visual object outlines should fall back to NAME when OBJNAME is absent");
        expect(list_result.objects[1].unique_id == "fallback-guid",
            "#743: visual object outlines should expose stable UNIQUEID values on fallback-name rows");
        expect(list_result.objects[1].caption == "\"Fallback\"",
            "#745: visual object outlines should expose parsed Caption values on fallback-name rows");
    }
    expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
        "#743: visual object outlining should not create undo history");

    fs::remove_all(temp_dir, ignored);
}

void test_list_visual_objects_reads_hierarchy_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_object_outline_metadata_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "outline_metadata.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "PARENT", .type = 'C', .length = 16U},
        {.name = "CLASS", .type = 'C', .length = 16U},
        {.name = "BASECLASS", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "Page1", "cmdButton", "CommandButton"},
        {"txtName", "nameBox", "name-guid", "", "", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#744: object outline metadata fixture should be writable");

    const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok, "#744: visual object outlines should list metadata fixtures");
    expect(list_result.objects.size() == 2U, "#744: visual object outlines should preserve row count with metadata fields");
    if (list_result.objects.size() == 2U) {
        expect(list_result.objects[0].parent_name == "Page1",
            "#744: visual object outlines should expose parent/container names");
        expect(list_result.objects[0].class_name == "cmdButton",
            "#744: visual object outlines should expose class names");
        expect(list_result.objects[0].baseclass_name == "CommandButton",
            "#744: visual object outlines should expose baseclass names");
        expect(list_result.objects[1].parent_name.empty() &&
                list_result.objects[1].class_name.empty() &&
                list_result.objects[1].baseclass_name.empty(),
            "#744: missing hierarchy/class metadata should remain empty");
        expect(list_result.objects[1].caption.empty(),
            "#745: visual object outlines should keep captions empty when no Caption property exists");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_list_visual_object_methods_reads_selected_methods() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_list_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "methods.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nFUNCTION GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC"
        },
        {"txtName", "nameBox", "name-guid", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#746: method-list fixture should be writable");

    auto method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok, "#746: visual object method lists should support UNIQUEID selection");
    expect(method_result.record_index == 0U && !method_result.record_deleted,
        "#746: visual object method lists should expose resolved record metadata");
    expect(method_result.methods.size() == 2U,
        "#746: visual object method lists should parse PROCEDURE and FUNCTION declarations");
    if (method_result.methods.size() == 2U) {
        expect(method_result.methods[0].method_name == "Click" &&
                method_result.methods[0].kind == "procedure" &&
                method_result.methods[0].source_text == "THISFORM.Save()",
            "#746: visual object method lists should parse procedure names and source bodies");
        expect(method_result.methods[0].source_line_index == 0U &&
                method_result.methods[0].source_memo_block_number != 0U,
            "#746: visual object method lists should expose procedure source-line and memo-block metadata");
        expect(method_result.methods[1].method_name == "GetCaption" &&
                method_result.methods[1].kind == "function" &&
                method_result.methods[1].source_text == "RETURN THIS.Caption",
            "#746: visual object method lists should parse function names and source bodies");
        expect(method_result.methods[1].source_line_index == 3U &&
                method_result.methods[1].source_memo_block_number == method_result.methods[0].source_memo_block_number,
            "#746: later methods should retain declaration line indexes and source memo block metadata");
    }
    expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
        "#746: visual object method listing should not create undo history");

    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {}
    });
    expect(method_result.ok, "#746: visual object method lists should support object-name selection");
    expect(method_result.record_index == 1U && method_result.methods.empty(),
        "#746: missing or empty METHODS fields should return an empty method list successfully");

    fs::remove_all(temp_dir, ignored);
}

void test_query_visual_object_method_reads_one_selected_method() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_query_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_query.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nfUnCtIoN GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC"
        },
        {"txtName", "nameBox", "name-guid", "PROCEDURE LostFocus\r\nTHISFORM.ValidateName()\r\nENDPROC"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#762: method-query fixture should be writable");

    auto query_result = copperfin::vfp::query_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "click"
    });
    expect(query_result.ok &&
            query_result.exists &&
            query_result.record_index == 0U &&
            !query_result.record_deleted &&
            query_result.method.method_name == "Click" &&
            query_result.method.kind == "procedure" &&
            query_result.method.source_text == "THISFORM.Save()" &&
            query_result.method.source_line_index == 0U &&
            query_result.method.source_memo_block_number != 0U,
        "#762: method query should return one procedure with resolved record and source metadata");

    query_result = copperfin::vfp::query_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdSave",
        .unique_id = {},
        .method_name = "GetCaption"
    });
    expect(query_result.ok &&
            query_result.exists &&
            query_result.method.method_name == "GetCaption" &&
            query_result.method.kind == "function" &&
            query_result.method.source_text == "RETURN THIS.Caption",
        "#762: method query should support object-name selection and function declarations");

    query_result = copperfin::vfp::query_visual_object_method({
        .path = table_path.string(),
        .record_index = 1U,
        .object_name = {},
        .unique_id = {},
        .method_name = "LostFocus"
    });
    expect(query_result.ok &&
            query_result.exists &&
            query_result.record_index == 1U &&
            query_result.method.method_name == "LostFocus" &&
            query_result.method.source_text == "THISFORM.ValidateName()",
        "#762: method query should support direct record-index selection");

    query_result = copperfin::vfp::query_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "Missing"
    });
    expect(query_result.ok && !query_result.exists && query_result.record_index == 0U,
        "#762: missing methods should return a successful not-found result");

    expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
        "#762: method query should not create undo history");

    const fs::path duplicate_path = temp_dir / "method_query_duplicate.scx";
    const std::vector<std::vector<std::string>> duplicate_records{
        {
            "cmdDup",
            "dupButton",
            "dup-guid",
            "PROCEDURE Click\r\nRETURN 1\r\nENDPROC\r\nPROCEDURE click\r\nRETURN 2\r\nENDPROC"
        }
    };
    const auto duplicate_create_result = copperfin::vfp::create_dbf_table_file(
        duplicate_path.string(),
        fields,
        duplicate_records);
    expect(duplicate_create_result.ok, "#762: duplicate method-query fixture should be writable");
    query_result = copperfin::vfp::query_visual_object_method({
        .path = duplicate_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dup-guid",
        .method_name = "CLICK"
    });
    expect(!query_result.ok, "#762: duplicate matching method names should fail as ambiguous");

    query_result = copperfin::vfp::query_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "   "
    });
    expect(!query_result.ok, "#762: empty method names should fail explicitly");

    const fs::path no_methods_path = temp_dir / "method_query_no_methods.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> no_methods_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> no_methods_records{
        {"cmdNoMethods", "no-methods-guid"}
    };
    const auto no_methods_create_result = copperfin::vfp::create_dbf_table_file(
        no_methods_path.string(),
        no_methods_fields,
        no_methods_records);
    expect(no_methods_create_result.ok, "#762: missing-METHODS fixture should be writable");
    query_result = copperfin::vfp::query_visual_object_method({
        .path = no_methods_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "no-methods-guid",
        .method_name = "Click"
    });
    expect(!query_result.ok, "#762: missing METHODS fields should fail explicitly");

    fs::remove_all(temp_dir, ignored);
}

void test_update_visual_object_method_updates_and_appends_methods() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_edit_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_edit.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC"
        },
        {"txtName", "nameBox", "name-guid", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#747: method-edit fixture should be writable");

    auto update_result = copperfin::vfp::update_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "click",
        .method_kind = "procedure",
        .source_text = "THISFORM.Save(.T.)"
    });
    expect(update_result.ok, "#747: method edits should update existing selected-object methods case-insensitively");

    update_result = copperfin::vfp::update_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "GetCaption",
        .method_kind = "function",
        .source_text = "RETURN THIS.Caption"
    });
    expect(update_result.ok, "#747: method edits should append missing selected-object methods");

    auto method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok, "#747: updated method fixture should remain readable");
    const auto* click = find_method_snapshot(method_result.methods, "Click");
    const auto* get_caption = find_method_snapshot(method_result.methods, "GetCaption");
    expect(click != nullptr && click->source_text == "THISFORM.Save(.T.)",
        "#747: method edits should replace existing method bodies while preserving declaration names");
    expect(get_caption != nullptr && get_caption->kind == "function" && get_caption->source_text == "RETURN THIS.Caption",
        "#747: method edits should append missing methods with requested kind and source");

    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {}
    });
    expect(method_result.ok && method_result.methods.empty(),
        "#747: selected-object method edits should not mutate unrelated object records");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#747: undo should restore the appended method edit");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#747: undo should restore the replaced method edit");

    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok, "#747: method fixture should remain readable after undo");
    click = find_method_snapshot(method_result.methods, "Click");
    get_caption = find_method_snapshot(method_result.methods, "GetCaption");
    expect(click != nullptr && click->source_text == "THISFORM.Save()",
        "#747: undo should restore original method source text");
    expect(get_caption == nullptr,
        "#747: undo should remove methods appended by the edit API");

    fs::remove_all(temp_dir, ignored);
}

void test_delete_visual_object_method_removes_selected_methods() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_delete_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_delete.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nFUNCTION GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC"
        },
        {"txtName", "nameBox", "name-guid", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#748: method-delete fixture should be writable");

    auto delete_result = copperfin::vfp::delete_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "click"
    });
    expect(delete_result.ok, "#748: method deletes should remove existing selected-object methods case-insensitively");

    auto method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok, "#748: method-delete fixture should remain readable after delete");
    expect(find_method_snapshot(method_result.methods, "Click") == nullptr,
        "#748: method deletes should remove the full selected method block");
    expect(find_method_snapshot(method_result.methods, "GetCaption") != nullptr,
        "#748: method deletes should preserve unrelated methods in the same METHODS memo");

    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {}
    });
    expect(method_result.ok && method_result.methods.empty(),
        "#748: selected-object method deletes should not mutate unrelated object records");

    delete_result = copperfin::vfp::delete_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "DoesNotExist"
    });
    expect(!delete_result.ok, "#748: missing method deletes should fail explicitly");

    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok &&
            find_method_snapshot(method_result.methods, "Click") == nullptr &&
            find_method_snapshot(method_result.methods, "GetCaption") != nullptr,
        "#748: missing method deletes should not mutate the METHODS memo");

    const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#748: undo should restore the deleted method block");
    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok &&
            find_method_snapshot(method_result.methods, "Click") != nullptr &&
            find_method_snapshot(method_result.methods, "GetCaption") != nullptr,
        "#748: undo should restore deleted methods while preserving unrelated methods");

    fs::remove_all(temp_dir, ignored);
}

void test_rename_visual_object_method_updates_declarations() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_rename_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_rename.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nFUNCTION GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC"
        },
        {
            "txtName",
            "nameBox",
            "name-guid",
            "PROCEDURE LostFocus\r\nTHISFORM.ValidateName()\r\nENDPROC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#761: method-rename fixture should be writable");

    auto rename_result = copperfin::vfp::rename_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "click",
        .new_method_name = "SaveClick"
    });
    expect(rename_result.ok, "#761: method rename should update procedure declarations case-insensitively");

    rename_result = copperfin::vfp::rename_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdSave",
        .unique_id = {},
        .method_name = "GetCaption",
        .new_method_name = "BuildCaption"
    });
    expect(rename_result.ok, "#761: method rename should update function declarations by object-name selection");

    auto method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok, "#761: renamed method fixture should remain readable");
    const auto* save_click = find_method_snapshot(method_result.methods, "SaveClick");
    const auto* build_caption = find_method_snapshot(method_result.methods, "BuildCaption");
    expect(save_click != nullptr &&
            save_click->kind == "procedure" &&
            save_click->source_text == "THISFORM.Save()",
        "#761: procedure rename should preserve kind and body text");
    expect(build_caption != nullptr &&
            build_caption->kind == "function" &&
            build_caption->source_text == "RETURN THIS.Caption",
        "#761: function rename should preserve kind and body text");
    expect(find_method_snapshot(method_result.methods, "Click") == nullptr &&
            find_method_snapshot(method_result.methods, "GetCaption") == nullptr,
        "#761: method rename should remove the old declaration names");

    auto other_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid"
    });
    expect(other_result.ok && find_method_snapshot(other_result.methods, "LostFocus") != nullptr,
        "#761: method rename should preserve unrelated object METHODS memos");

    rename_result = copperfin::vfp::rename_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "SaveClick",
        .new_method_name = "BuildCaption"
    });
    expect(!rename_result.ok, "#761: method rename should reject target method collisions");

    rename_result = copperfin::vfp::rename_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "DoesNotExist",
        .new_method_name = "Missing"
    });
    expect(!rename_result.ok, "#761: method rename should reject missing source methods");

    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok &&
            find_method_snapshot(method_result.methods, "SaveClick") != nullptr &&
            find_method_snapshot(method_result.methods, "BuildCaption") != nullptr,
        "#761: failed method renames should not mutate the METHODS memo");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#761: undo should restore function rename");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#761: undo should restore procedure rename");

    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok &&
            find_method_snapshot(method_result.methods, "Click") != nullptr &&
            find_method_snapshot(method_result.methods, "GetCaption") != nullptr &&
            find_method_snapshot(method_result.methods, "SaveClick") == nullptr &&
            find_method_snapshot(method_result.methods, "BuildCaption") == nullptr,
        "#761: undo should restore previous METHODS memo declarations");

    fs::remove_all(temp_dir, ignored);
}

void test_copy_visual_object_method_between_selected_objects() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_copy_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_copy.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSource",
            "sourceButton",
            "source-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nFUNCTION GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC"
        },
        {
            "txtTarget",
            "targetBox",
            "target-guid",
            "PROCEDURE Existing\r\nTHISFORM.Old()\r\nENDPROC\r\nFUNCTION Refresh\r\nRETURN .F.\r\nENDFUNC"
        },
        {
            "lblOther",
            "otherLabel",
            "other-guid",
            "PROCEDURE Other\r\nTHISFORM.Other()\r\nENDPROC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#763: method-copy fixture should be writable");

    auto copy_result = copperfin::vfp::copy_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "click",
        .target_record_index = 0U,
        .target_object_name = "txtTarget",
        .target_unique_id = {},
        .target_method_name = {},
        .replace_existing = false
    });
    expect(copy_result.ok, "#763: method copy should copy procedures by UNIQUEID source and object-name target");

    copy_result = copperfin::vfp::copy_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = "cmdSource",
        .source_unique_id = {},
        .source_method_name = "GetCaption",
        .target_record_index = 1U,
        .target_object_name = {},
        .target_unique_id = {},
        .target_method_name = "CaptionText",
        .replace_existing = false
    });
    expect(copy_result.ok, "#763: method copy should support record-index targets and target method renames");

    auto target_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid"
    });
    expect(target_methods.ok, "#763: target method fixture should remain readable after copies");
    const auto* copied_click = find_method_snapshot(target_methods.methods, "Click");
    const auto* copied_caption = find_method_snapshot(target_methods.methods, "CaptionText");
    const auto* existing = find_method_snapshot(target_methods.methods, "Existing");
    expect(copied_click != nullptr &&
            copied_click->kind == "procedure" &&
            copied_click->source_text == "THISFORM.Save()",
        "#763: copied procedures should preserve source body and kind");
    expect(copied_caption != nullptr &&
            copied_caption->kind == "function" &&
            copied_caption->source_text == "RETURN THIS.Caption",
        "#763: copied functions should append using the source kind and requested target name");
    expect(existing != nullptr && existing->source_text == "THISFORM.Old()",
        "#763: method copy should preserve unrelated target methods");

    auto source_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid"
    });
    expect(source_methods.ok &&
            find_method_snapshot(source_methods.methods, "Click") != nullptr &&
            find_method_snapshot(source_methods.methods, "GetCaption") != nullptr,
        "#763: method copy should not mutate source methods");

    auto other_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "other-guid"
    });
    expect(other_methods.ok && find_method_snapshot(other_methods.methods, "Other") != nullptr,
        "#763: method copy should preserve unrelated object methods");

    copy_result = copperfin::vfp::copy_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Click",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = {},
        .replace_existing = false
    });
    expect(!copy_result.ok, "#763: method copy should reject target collisions without replacement");

    copy_result = copperfin::vfp::copy_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Click",
        .target_record_index = 1U,
        .target_object_name = {},
        .target_unique_id = {},
        .target_method_name = "Refresh",
        .replace_existing = true
    });
    expect(copy_result.ok, "#763: method copy should allow explicit replacement");

    target_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid"
    });
    const auto* refresh = target_methods.ok ? find_method_snapshot(target_methods.methods, "Refresh") : nullptr;
    expect(refresh != nullptr &&
            refresh->kind == "function" &&
            refresh->source_text == "THISFORM.Save()",
        "#763: replacing existing target methods should preserve the target declaration kind");

    copy_result = copperfin::vfp::copy_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Missing",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = "Missing",
        .replace_existing = false
    });
    expect(!copy_result.ok, "#763: method copy should reject missing source methods");

    copy_result = copperfin::vfp::copy_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = " ",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = "EmptySource",
        .replace_existing = false
    });
    expect(!copy_result.ok, "#763: method copy should reject empty source method names");

    copy_result = copperfin::vfp::copy_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Click",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = " ",
        .replace_existing = false
    });
    expect(!copy_result.ok, "#763: method copy should reject empty requested target method names");

    copy_result = copperfin::vfp::copy_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Click",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "source-guid",
        .target_method_name = {},
        .replace_existing = false
    });
    expect(!copy_result.ok, "#763: same-object method copy should reject implicit overwrite without replacement");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#763: undo should restore replaced target methods");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#763: undo should remove copied renamed function methods");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#763: undo should remove copied procedure methods");

    target_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid"
    });
    refresh = target_methods.ok ? find_method_snapshot(target_methods.methods, "Refresh") : nullptr;
    expect(target_methods.ok &&
            find_method_snapshot(target_methods.methods, "Click") == nullptr &&
            find_method_snapshot(target_methods.methods, "CaptionText") == nullptr &&
            refresh != nullptr &&
            refresh->source_text == "RETURN .F.",
        "#763: undo should restore the target METHODS memo to its original method set");

    const fs::path duplicate_path = temp_dir / "method_copy_duplicate.scx";
    const std::vector<std::vector<std::string>> duplicate_records{
        {
            "cmdDup",
            "dupButton",
            "dup-guid",
            "PROCEDURE Click\r\nRETURN 1\r\nENDPROC\r\nPROCEDURE click\r\nRETURN 2\r\nENDPROC"
        },
        {"txtTarget", "targetBox", "target-guid", ""}
    };
    const auto duplicate_create_result = copperfin::vfp::create_dbf_table_file(
        duplicate_path.string(),
        fields,
        duplicate_records);
    expect(duplicate_create_result.ok, "#763: duplicate method-copy fixture should be writable");
    copy_result = copperfin::vfp::copy_visual_object_method({
        .path = duplicate_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "dup-guid",
        .source_method_name = "CLICK",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = {},
        .replace_existing = false
    });
    expect(!copy_result.ok, "#763: method copy should reject duplicate source declarations as ambiguous");

    const fs::path no_methods_path = temp_dir / "method_copy_no_methods.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> no_methods_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> no_methods_records{
        {"cmdSource", "source-guid"},
        {"txtTarget", "target-guid"}
    };
    const auto no_methods_create_result = copperfin::vfp::create_dbf_table_file(
        no_methods_path.string(),
        no_methods_fields,
        no_methods_records);
    expect(no_methods_create_result.ok, "#763: missing-METHODS method-copy fixture should be writable");
    copy_result = copperfin::vfp::copy_visual_object_method({
        .path = no_methods_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Click",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = {},
        .replace_existing = false
    });
    expect(!copy_result.ok, "#763: method copy should reject missing METHODS fields");

    fs::remove_all(temp_dir, ignored);
}

void test_move_visual_object_method_between_selected_objects() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_move_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_move.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSource",
            "sourceButton",
            "source-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nFUNCTION GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC"
        },
        {
            "txtTarget",
            "targetBox",
            "target-guid",
            "PROCEDURE Existing\r\nTHISFORM.Old()\r\nENDPROC\r\nFUNCTION Refresh\r\nRETURN .F.\r\nENDFUNC"
        },
        {
            "lblOther",
            "otherLabel",
            "other-guid",
            "PROCEDURE Other\r\nTHISFORM.Other()\r\nENDPROC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#764: method-move fixture should be writable");

    auto move_result = copperfin::vfp::move_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "click",
        .target_record_index = 0U,
        .target_object_name = "txtTarget",
        .target_unique_id = {},
        .target_method_name = {},
        .replace_existing = false
    });
    expect(move_result.ok, "#764: method move should move procedures by UNIQUEID source and object-name target");

    auto source_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid"
    });
    auto target_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid"
    });
    expect(source_methods.ok &&
            find_method_snapshot(source_methods.methods, "Click") == nullptr &&
            find_method_snapshot(source_methods.methods, "GetCaption") != nullptr,
        "#764: method move should delete only the moved source method");
    const auto* moved_click = target_methods.ok ? find_method_snapshot(target_methods.methods, "Click") : nullptr;
    expect(moved_click != nullptr &&
            moved_click->kind == "procedure" &&
            moved_click->source_text == "THISFORM.Save()" &&
            find_method_snapshot(target_methods.methods, "Existing") != nullptr,
        "#764: method move should append the moved procedure while preserving target methods");

    move_result = copperfin::vfp::move_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = "cmdSource",
        .source_unique_id = {},
        .source_method_name = "GetCaption",
        .target_record_index = 1U,
        .target_object_name = {},
        .target_unique_id = {},
        .target_method_name = "CaptionText",
        .replace_existing = false
    });
    expect(move_result.ok, "#764: method move should support record-index targets and target method renames");

    source_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid"
    });
    target_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid"
    });
    const auto* moved_caption = target_methods.ok ? find_method_snapshot(target_methods.methods, "CaptionText") : nullptr;
    expect(source_methods.ok && source_methods.methods.empty(),
        "#764: moving the final source method should leave the source METHODS memo empty");
    expect(moved_caption != nullptr &&
            moved_caption->kind == "function" &&
            moved_caption->source_text == "RETURN THIS.Caption",
        "#764: renamed function moves should preserve source body and function kind");

    auto other_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "other-guid"
    });
    expect(other_methods.ok && find_method_snapshot(other_methods.methods, "Other") != nullptr,
        "#764: method move should preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#764: undo should restore the moved function source deletion");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#764: undo should remove the moved function target copy");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#764: undo should restore the moved procedure source deletion");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#764: undo should remove the moved procedure target copy");

    source_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid"
    });
    target_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid"
    });
    expect(source_methods.ok &&
            find_method_snapshot(source_methods.methods, "Click") != nullptr &&
            find_method_snapshot(source_methods.methods, "GetCaption") != nullptr &&
            target_methods.ok &&
            find_method_snapshot(target_methods.methods, "Click") == nullptr &&
            find_method_snapshot(target_methods.methods, "CaptionText") == nullptr,
        "#764: undo should restore source and target METHODS memos after moves");

    move_result = copperfin::vfp::move_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Click",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = "Existing",
        .replace_existing = false
    });
    expect(!move_result.ok, "#764: method move should reject target collisions without replacement");
    source_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid"
    });
    expect(source_methods.ok && find_method_snapshot(source_methods.methods, "Click") != nullptr,
        "#764: failed target copies should leave the source method intact");

    move_result = copperfin::vfp::move_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Click",
        .target_record_index = 1U,
        .target_object_name = {},
        .target_unique_id = {},
        .target_method_name = "Refresh",
        .replace_existing = true
    });
    expect(move_result.ok, "#764: method move should allow explicit replacement");
    target_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid"
    });
    const auto* refresh = target_methods.ok ? find_method_snapshot(target_methods.methods, "Refresh") : nullptr;
    expect(refresh != nullptr &&
            refresh->kind == "function" &&
            refresh->source_text == "THISFORM.Save()",
        "#764: replacing target methods during move should preserve target declaration kind");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#764: undo should restore replacement source deletion");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#764: undo should restore replacement target body");

    move_result = copperfin::vfp::move_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Click",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "source-guid",
        .target_method_name = {},
        .replace_existing = false
    });
    expect(!move_result.ok, "#764: method move should reject same-source implicit overwrites");

    move_result = copperfin::vfp::move_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = " ",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = "EmptySource",
        .replace_existing = false
    });
    expect(!move_result.ok, "#764: method move should reject empty source method names");

    move_result = copperfin::vfp::move_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Click",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = " ",
        .replace_existing = false
    });
    expect(!move_result.ok, "#764: method move should reject empty requested target method names");

    move_result = copperfin::vfp::move_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Missing",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = "Missing",
        .replace_existing = false
    });
    expect(!move_result.ok, "#764: method move should reject missing source methods");

    const fs::path duplicate_path = temp_dir / "method_move_duplicate.scx";
    const std::vector<std::vector<std::string>> duplicate_records{
        {
            "cmdDup",
            "dupButton",
            "dup-guid",
            "PROCEDURE Click\r\nRETURN 1\r\nENDPROC\r\nPROCEDURE click\r\nRETURN 2\r\nENDPROC"
        },
        {"txtTarget", "targetBox", "target-guid", ""}
    };
    const auto duplicate_create_result = copperfin::vfp::create_dbf_table_file(
        duplicate_path.string(),
        fields,
        duplicate_records);
    expect(duplicate_create_result.ok, "#764: duplicate method-move fixture should be writable");
    move_result = copperfin::vfp::move_visual_object_method({
        .path = duplicate_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "dup-guid",
        .source_method_name = "CLICK",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = {},
        .replace_existing = false
    });
    expect(!move_result.ok, "#764: method move should reject duplicate source declarations as ambiguous");

    const fs::path no_methods_path = temp_dir / "method_move_no_methods.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> no_methods_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> no_methods_records{
        {"cmdSource", "source-guid"},
        {"txtTarget", "target-guid"}
    };
    const auto no_methods_create_result = copperfin::vfp::create_dbf_table_file(
        no_methods_path.string(),
        no_methods_fields,
        no_methods_records);
    expect(no_methods_create_result.ok, "#764: missing-METHODS method-move fixture should be writable");
    move_result = copperfin::vfp::move_visual_object_method({
        .path = no_methods_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Click",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = {},
        .replace_existing = false
    });
    expect(!move_result.ok, "#764: method move should reject missing METHODS fields");

    fs::remove_all(temp_dir, ignored);
}

void test_reorder_visual_object_methods_within_selected_object() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_reorder_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_reorder.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSource",
            "sourceButton",
            "source-guid",
            "PROCEDURE Alpha\r\nTHISFORM.Alpha()\r\nENDPROC\r\nFUNCTION Bravo\r\nRETURN THIS.Caption\r\nENDFUNC\r\nPROCEDURE Charlie\r\nTHISFORM.Charlie()\r\nENDPROC"
        },
        {
            "txtOther",
            "otherBox",
            "other-guid",
            "PROCEDURE Other\r\nTHISFORM.Other()\r\nENDPROC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#765: method-reorder fixture should be writable");

    const auto method_order = [&](const std::string& unique_id) {
        std::vector<std::string> names;
        const auto methods = copperfin::vfp::list_visual_object_methods({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id
        });
        if (!methods.ok) {
            return names;
        }
        for (const auto& method : methods.methods) {
            names.push_back(method.method_name);
        }
        return names;
    };
    const auto order_is = [](const std::vector<std::string>& names, std::initializer_list<const char*> expected) {
        if (names.size() != expected.size()) {
            return false;
        }
        auto name_it = names.begin();
        auto expected_it = expected.begin();
        for (; name_it != names.end(); ++name_it, ++expected_it) {
            if (*name_it != *expected_it) {
                return false;
            }
        }
        return true;
    };

    auto reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid",
        .method_name = "charlie",
        .placement = "first",
        .relative_method_name = {}
    });
    expect(reorder_result.ok, "#765: method reorder should support UNIQUEID selection and first placement");
    expect(order_is(method_order("source-guid"), {"Charlie", "Alpha", "Bravo"}),
        "#765: first placement should move the requested method to the start");

    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdSource",
        .unique_id = {},
        .method_name = "CHARLIE",
        .placement = "last",
        .relative_method_name = {}
    });
    expect(reorder_result.ok, "#765: method reorder should support object-name selection and last placement");
    expect(order_is(method_order("source-guid"), {"Alpha", "Bravo", "Charlie"}),
        "#765: last placement should move the requested method to the end");

    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .method_name = "Alpha",
        .placement = "after",
        .relative_method_name = "Charlie"
    });
    expect(reorder_result.ok, "#765: method reorder should support record-index selection and after placement");
    expect(order_is(method_order("source-guid"), {"Bravo", "Charlie", "Alpha"}),
        "#765: after placement should move the requested method after the relative method");

    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid",
        .method_name = "Alpha",
        .placement = "before",
        .relative_method_name = "Bravo"
    });
    expect(reorder_result.ok, "#765: method reorder should support before placement");
    expect(order_is(method_order("source-guid"), {"Alpha", "Bravo", "Charlie"}),
        "#765: before placement should move the requested method before the relative method");

    const auto methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid"
    });
    const auto* alpha = methods.ok ? find_method_snapshot(methods.methods, "Alpha") : nullptr;
    const auto* bravo = methods.ok ? find_method_snapshot(methods.methods, "Bravo") : nullptr;
    const auto* charlie = methods.ok ? find_method_snapshot(methods.methods, "Charlie") : nullptr;
    expect(alpha != nullptr &&
            alpha->kind == "procedure" &&
            alpha->source_text == "THISFORM.Alpha()" &&
            bravo != nullptr &&
            bravo->kind == "function" &&
            bravo->source_text == "RETURN THIS.Caption" &&
            charlie != nullptr &&
            charlie->kind == "procedure" &&
            charlie->source_text == "THISFORM.Charlie()",
        "#765: method reorder should preserve method names, kinds, and source bodies");

    expect(order_is(method_order("other-guid"), {"Other"}),
        "#765: method reorder should preserve unrelated object METHODS memos");

    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid",
        .method_name = "Missing",
        .placement = "first",
        .relative_method_name = {}
    });
    expect(!reorder_result.ok, "#765: method reorder should reject missing methods");

    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid",
        .method_name = "Alpha",
        .placement = "before",
        .relative_method_name = {}
    });
    expect(!reorder_result.ok, "#765: method reorder should reject missing relative methods for before placement");

    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid",
        .method_name = "Alpha",
        .placement = "after",
        .relative_method_name = "alpha"
    });
    expect(!reorder_result.ok, "#765: method reorder should reject self-relative before/after placement");

    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid",
        .method_name = "Alpha",
        .placement = "middle",
        .relative_method_name = {}
    });
    expect(!reorder_result.ok, "#765: method reorder should reject unknown placements");

    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid",
        .method_name = " ",
        .placement = "first",
        .relative_method_name = {}
    });
    expect(!reorder_result.ok, "#765: method reorder should reject empty method names");

    expect(order_is(method_order("source-guid"), {"Alpha", "Bravo", "Charlie"}),
        "#765: failed method reorders should not mutate the METHODS memo");

    for (int index = 0; index < 4; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#765: undo should restore each successful method reorder");
    }
    expect(order_is(method_order("source-guid"), {"Alpha", "Bravo", "Charlie"}),
        "#765: undo should restore original method ordering");

    const fs::path duplicate_path = temp_dir / "method_reorder_duplicate.scx";
    const std::vector<std::vector<std::string>> duplicate_records{
        {
            "cmdDup",
            "dupButton",
            "dup-guid",
            "PROCEDURE Click\r\nRETURN 1\r\nENDPROC\r\nPROCEDURE click\r\nRETURN 2\r\nENDPROC\r\nPROCEDURE Anchor\r\nRETURN 3\r\nENDPROC"
        }
    };
    const auto duplicate_create_result = copperfin::vfp::create_dbf_table_file(
        duplicate_path.string(),
        fields,
        duplicate_records);
    expect(duplicate_create_result.ok, "#765: duplicate method-reorder fixture should be writable");
    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = duplicate_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dup-guid",
        .method_name = "CLICK",
        .placement = "first",
        .relative_method_name = {}
    });
    expect(!reorder_result.ok, "#765: method reorder should reject duplicate source declarations as ambiguous");
    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = duplicate_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dup-guid",
        .method_name = "Anchor",
        .placement = "before",
        .relative_method_name = "Click"
    });
    expect(!reorder_result.ok, "#765: method reorder should reject duplicate relative declarations as ambiguous");

    const fs::path no_methods_path = temp_dir / "method_reorder_no_methods.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> no_methods_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> no_methods_records{
        {"cmdNoMethods", "no-methods-guid"}
    };
    const auto no_methods_create_result = copperfin::vfp::create_dbf_table_file(
        no_methods_path.string(),
        no_methods_fields,
        no_methods_records);
    expect(no_methods_create_result.ok, "#765: missing-METHODS method-reorder fixture should be writable");
    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = no_methods_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "no-methods-guid",
        .method_name = "Click",
        .placement = "first",
        .relative_method_name = {}
    });
    expect(!reorder_result.ok, "#765: method reorder should reject missing METHODS fields");

    fs::remove_all(temp_dir, ignored);
}

void test_duplicate_visual_object_appends_identity_safe_copy() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_duplicate_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "duplicate.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U},
        {.name = "CLASS", .type = 'C', .length = 20U},
        {.name = "BASECLASS", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "frmMain",
            "commandbutton",
            "commandbutton",
            "Caption = \"Save\"\r\nLeft = 12\r\n",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC"
        },
        {
            "txtName",
            "nameBox",
            "name-guid",
            "frmMain",
            "textbox",
            "textbox",
            "Caption = \"Name\"\r\n",
            ""
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#749: duplicate fixture should be writable");

    auto duplicate_result = copperfin::vfp::duplicate_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdSave",
        .unique_id = {},
        .new_object_name = "cmdSaveCopy",
        .new_name = "saveButtonCopy",
        .new_unique_id = "save-copy-guid"
    });
    expect(duplicate_result.ok && duplicate_result.record_index == 2U,
        "#749: selected-object duplication should append a live copy at the next record index");

    auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok && list_result.objects.size() == 3U,
        "#749: duplication should append exactly one visual object row");
    if (list_result.ok && list_result.objects.size() == 3U) {
        expect(list_result.objects[0].object_name == "cmdSave" &&
                list_result.objects[0].unique_id == "save-guid",
            "#749: duplication should preserve the original selected object");
        expect(!list_result.objects[2].deleted &&
                list_result.objects[2].object_name == "cmdSaveCopy" &&
                list_result.objects[2].unique_id == "save-copy-guid" &&
                list_result.objects[2].parent_name == "frmMain" &&
                list_result.objects[2].class_name == "commandbutton" &&
                list_result.objects[2].baseclass_name == "commandbutton" &&
                list_result.objects[2].caption == "\"Save\"",
            "#749: duplicated visual objects should expose replacement identity and preserved metadata");
    }

    auto property_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-copy-guid",
        .property_name = "Left"
    });
    expect(property_result.ok && property_result.exists && property_result.value == "12",
        "#749: duplicated visual objects should preserve memo-backed properties");

    auto method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-copy-guid"
    });
    expect(method_result.ok && find_method_snapshot(method_result.methods, "Click") != nullptr,
        "#749: duplicated visual objects should preserve METHODS memo content");

    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#749: duplicate fixture should support marking an existing object deleted");

    duplicate_result = copperfin::vfp::duplicate_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .new_object_name = "cmdOther",
        .new_name = "otherButton",
        .new_unique_id = "name-guid"
    });
    expect(!duplicate_result.ok,
        "#749: duplicate identity checks should reject collisions with deleted records");

    list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok && list_result.objects.size() == 3U && list_result.objects[1].deleted,
        "#749: failed duplicate requests should not mutate object count or deleted flags");

    fs::remove_all(temp_dir, ignored);
}

void test_create_visual_object_appends_toolbox_field_values() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_create_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "create.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U},
        {.name = "CLASS", .type = 'C', .length = 20U},
        {.name = "BASECLASS", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "frmMain",
            "commandbutton",
            "commandbutton",
            "Caption = \"Save\"\r\n",
            ""
        },
        {
            "txtName",
            "nameBox",
            "name-guid",
            "frmMain",
            "textbox",
            "textbox",
            "Caption = \"Name\"\r\n",
            ""
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#750: create fixture should be writable");

    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#750: create fixture should support deleted-row preservation setup");

    auto create_object_result = copperfin::vfp::create_visual_object({
        .path = table_path.string(),
        .field_values = {
            {.property_name = "OBJNAME", .property_value = "chkActive"},
            {.property_name = "NAME", .property_value = "activeCheck"},
            {.property_name = "UNIQUEID", .property_value = "active-guid"},
            {.property_name = "PARENT", .property_value = "frmMain"},
            {.property_name = "CLASS", .property_value = "checkbox"},
            {.property_name = "BASECLASS", .property_value = "checkbox"},
            {.property_name = "PROPERTIES", .property_value = "Caption = \"Active\"\r\nLeft = 24\r\n"},
            {.property_name = "METHODS", .property_value = "PROCEDURE Click\r\nTHIS.Value = !THIS.Value\r\nENDPROC"}
        }
    });
    expect(create_object_result.ok && create_object_result.record_index == 2U,
        "#750: toolbox creates should append a live object row at the next record index");

    auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok && list_result.objects.size() == 3U,
        "#750: toolbox creates should append exactly one object");
    if (list_result.ok && list_result.objects.size() == 3U) {
        expect(list_result.objects[0].object_name == "cmdSave" &&
                list_result.objects[0].unique_id == "save-guid",
            "#750: toolbox creates should preserve existing live records");
        expect(list_result.objects[1].deleted && list_result.objects[1].unique_id == "name-guid",
            "#750: toolbox creates should preserve existing deleted-row flags");
        expect(!list_result.objects[2].deleted &&
                list_result.objects[2].object_name == "chkActive" &&
                list_result.objects[2].unique_id == "active-guid" &&
                list_result.objects[2].parent_name == "frmMain" &&
                list_result.objects[2].class_name == "checkbox" &&
                list_result.objects[2].baseclass_name == "checkbox" &&
                list_result.objects[2].caption == "\"Active\"",
            "#750: created objects should expose initialized identity, hierarchy, class, and caption metadata");
    }

    auto property_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "active-guid",
        .property_name = "Left"
    });
    expect(property_result.ok && property_result.exists && property_result.value == "24",
        "#750: toolbox creates should initialize memo-backed properties");

    auto method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "active-guid"
    });
    expect(method_result.ok && find_method_snapshot(method_result.methods, "Click") != nullptr,
        "#750: toolbox creates should initialize METHODS memo content");

    create_object_result = copperfin::vfp::create_visual_object({
        .path = table_path.string(),
        .field_values = {
            {.property_name = "OBJNAME", .property_value = "chkOther"},
            {.property_name = "NAME", .property_value = "otherCheck"},
            {.property_name = "UNIQUEID", .property_value = "name-guid"}
        }
    });
    expect(!create_object_result.ok,
        "#750: toolbox creates should reject identity collisions with deleted rows");

    create_object_result = copperfin::vfp::create_visual_object({
        .path = table_path.string(),
        .field_values = {
            {.property_name = "OBJNAME", .property_value = "chkOther"},
            {.property_name = "UNKNOWN", .property_value = "value"}
        }
    });
    expect(!create_object_result.ok,
        "#750: toolbox creates should reject unknown requested fields");

    list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok && list_result.objects.size() == 3U && list_result.objects[1].deleted,
        "#750: failed toolbox creates should not mutate object count or deleted flags");

    fs::remove_all(temp_dir, ignored);
}

void test_reparent_visual_object_updates_container_parent() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_reparent_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "reparent.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U},
        {.name = "CLASS", .type = 'C', .length = 20U},
        {.name = "BASECLASS", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmMain", "mainForm", "form-guid", "", "form", "form", "Caption = \"Main\"\r\n"},
        {"cntMain", "mainContainer", "container-guid", "frmMain", "container", "container", ""},
        {"cmdSave", "saveButton", "save-guid", "frmMain", "commandbutton", "commandbutton", ""},
        {"txtName", "nameBox", "name-guid", "frmMain", "textbox", "textbox", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#751: reparent fixture should be writable");

    auto reparent_result = copperfin::vfp::reparent_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .parent_object_name = "cntMain",
        .parent_unique_id = {},
        .clear_parent = false
    });
    expect(reparent_result.ok, "#751: reparent should support UNIQUEID source and object-name parent selection");

    auto parent_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "PARENT"
    });
    expect(parent_result.ok && parent_result.value == "cntMain",
        "#751: reparent should write the resolved parent object name");

    parent_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "PARENT"
    });
    expect(parent_result.ok && parent_result.value == "frmMain",
        "#751: reparent should preserve unrelated object parent fields");

    const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#751: reparent should route through visual property undo");
    parent_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "PARENT"
    });
    expect(parent_result.ok && parent_result.value == "frmMain",
        "#751: undo should restore the previous parent");

    reparent_result = copperfin::vfp::reparent_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdSave",
        .unique_id = {},
        .parent_object_name = "cntMain",
        .parent_unique_id = {},
        .clear_parent = false
    });
    expect(reparent_result.ok, "#751: reparent should support object-name source selection");

    reparent_result = copperfin::vfp::reparent_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .parent_object_name = {},
        .parent_unique_id = "form-guid",
        .clear_parent = false
    });
    expect(reparent_result.ok, "#751: reparent should support UNIQUEID parent selection");
    parent_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "PARENT"
    });
    expect(parent_result.ok && parent_result.value == "frmMain",
        "#751: UNIQUEID parent selection should write the target object's OBJNAME");

    reparent_result = copperfin::vfp::reparent_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .parent_object_name = {},
        .parent_unique_id = {},
        .clear_parent = true
    });
    expect(reparent_result.ok, "#751: reparent should support clearing parent for root-level placement");
    parent_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "PARENT"
    });
    expect(parent_result.ok && parent_result.value.empty(),
        "#751: clear-parent reparent should blank the parent field");

    reparent_result = copperfin::vfp::reparent_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .parent_object_name = {},
        .parent_unique_id = "save-guid",
        .clear_parent = false
    });
    expect(!reparent_result.ok, "#751: reparent should reject self-parenting");

    reparent_result = copperfin::vfp::reparent_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .parent_object_name = "missingParent",
        .parent_unique_id = {},
        .clear_parent = false
    });
    expect(!reparent_result.ok, "#751: reparent should reject missing parent selectors");

    parent_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "PARENT"
    });
    expect(parent_result.ok && parent_result.value.empty(),
        "#751: failed reparent requests should not mutate the selected object's parent");

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
    expect(property_value("save-guid", "HPOS") == "100" &&
            property_value("name-guid", "VPOS") == "300",
        "#752: empty batch edit requests should not mutate existing geometry");

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
    expect(!is_deleted("status-guid") && !is_deleted("dup-one-guid") && !is_deleted("dup-two-guid"),
        "#753: failed batch deleted-state changes should roll back earlier flag mutations");

    batch_result = copperfin::vfp::set_visual_object_deleted_states({
        .path = table_path.string(),
        .objects = {}
    });
    expect(!batch_result.ok, "#753: empty batch deleted-state requests should fail explicitly");
    expect(!is_deleted("save-guid") && !is_deleted("name-guid") && !is_deleted("status-guid"),
        "#753: empty batch deleted-state requests should not mutate existing flags");

    fs::remove_all(temp_dir, ignored);
}

void test_rename_visual_object_updates_identity_safely() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_rename_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "rename.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "frmMain"},
        {"txtName", "nameBox", "name-guid", "frmMain"},
        {"oldDeleted", "deletedName", "deleted-guid", "frmMain"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#754: rename fixture should be writable");

    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "deleted-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#754: rename fixture should support deleted-row collision setup");

    auto rename_result = copperfin::vfp::rename_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .update_object_name = true,
        .new_object_name = "cmdCommit",
        .update_name = true,
        .new_name = "commitButton",
        .update_unique_id = true,
        .new_unique_id = "commit-guid"
    });
    expect(rename_result.ok, "#754: rename should update selected object identity fields together");

    auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok && list_result.objects.size() == 3U,
        "#754: renamed visual asset should remain listable");
    if (list_result.ok && list_result.objects.size() == 3U) {
        expect(list_result.objects[0].object_name == "cmdCommit" &&
                list_result.objects[0].unique_id == "commit-guid",
            "#754: rename should expose updated OBJNAME and UNIQUEID");
        expect(list_result.objects[1].object_name == "txtName" &&
                list_result.objects[1].unique_id == "name-guid",
            "#754: rename should preserve unrelated object identity");
        expect(list_result.objects[2].deleted && list_result.objects[2].unique_id == "deleted-guid",
            "#754: rename should preserve deleted-row identity metadata");
    }

    auto property_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "commit-guid",
        .property_name = "NAME"
    });
    expect(property_result.ok && property_result.value == "commitButton",
        "#754: rename should update NAME and keep UNIQUEID selection usable");

    rename_result = copperfin::vfp::rename_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdCommit",
        .unique_id = {},
        .update_object_name = false,
        .new_object_name = {},
        .update_name = false,
        .new_name = {},
        .update_unique_id = true,
        .new_unique_id = "deleted-guid"
    });
    expect(!rename_result.ok,
        "#754: rename should reject identity collisions with deleted rows");

    rename_result = copperfin::vfp::rename_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdCommit",
        .unique_id = {},
        .update_object_name = false,
        .new_object_name = {},
        .update_name = false,
        .new_name = {},
        .update_unique_id = false,
        .new_unique_id = {}
    });
    expect(!rename_result.ok, "#754: empty rename requests should fail explicitly");

    property_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdCommit",
        .unique_id = {},
        .property_name = "UNIQUEID"
    });
    expect(property_result.ok && property_result.value == "commit-guid",
        "#754: failed rename requests should not mutate selected identity fields");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#754: rename should route UNIQUEID through existing undo");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#754: rename should route NAME through existing undo");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#754: rename should route OBJNAME through existing undo");

    list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok && list_result.objects.size() == 3U &&
            list_result.objects[0].object_name == "cmdSave" &&
            list_result.objects[0].unique_id == "save-guid",
        "#754: undo should restore renamed identity fields");

    fs::remove_all(temp_dir, ignored);
}

void test_reorder_visual_object_updates_z_order() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_reorder_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "reorder.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdA", "buttonA", "a-guid", "Caption = \"A\"\r\n"},
        {"cmdB", "buttonB", "b-guid", "Caption = \"B\"\r\n"},
        {"cmdC", "buttonC", "c-guid", "Caption = \"C\"\r\n"},
        {"cmdD", "buttonD", "d-guid", "Caption = \"D\"\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#755: reorder fixture should be writable");
    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "c-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#755: reorder fixture should support deleted-row preservation setup");

    const auto order_string = [&]() {
        const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
        expect(list_result.ok, "#755: reordered visual asset should remain listable");
        std::string value;
        for (const auto& object : list_result.objects) {
            if (!value.empty()) {
                value += ",";
            }
            value += object.unique_id;
            if (object.deleted) {
                value += "*";
            }
        }
        return value;
    };

    auto reorder_result = copperfin::vfp::reorder_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "c-guid",
        .placement = "front",
        .target_object_name = {},
        .target_unique_id = {}
    });
    expect(reorder_result.ok, "#755: reorder should support front placement by UNIQUEID");
    expect(order_string() == "c-guid*,a-guid,b-guid,d-guid",
        "#755: front placement should move the selected record to the front and preserve deleted flags");

    reorder_result = copperfin::vfp::reorder_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdA",
        .unique_id = {},
        .placement = "back",
        .target_object_name = {},
        .target_unique_id = {}
    });
    expect(reorder_result.ok, "#755: reorder should support back placement by object name after indexes change");
    expect(order_string() == "c-guid*,b-guid,d-guid,a-guid",
        "#755: back placement should move the selected record to the back");

    reorder_result = copperfin::vfp::reorder_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "a-guid",
        .placement = "before",
        .target_object_name = "cmdB",
        .target_unique_id = {}
    });
    expect(reorder_result.ok, "#755: reorder should support before-target placement by object-name target");
    expect(order_string() == "c-guid*,a-guid,b-guid,d-guid",
        "#755: before placement should insert the selected record before the resolved target");

    reorder_result = copperfin::vfp::reorder_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "b-guid",
        .placement = "after",
        .target_object_name = {},
        .target_unique_id = "d-guid"
    });
    expect(reorder_result.ok, "#755: reorder should support after-target placement by UNIQUEID target");
    expect(order_string() == "c-guid*,a-guid,d-guid,b-guid",
        "#755: after placement should insert the selected record after the resolved target");

    auto property_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "a-guid",
        .property_name = "Caption"
    });
    expect(property_result.ok && property_result.value == "\"A\"",
        "#755: reorder should preserve memo-backed field values");

    reorder_result = copperfin::vfp::reorder_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "b-guid",
        .placement = "before",
        .target_object_name = {},
        .target_unique_id = "b-guid"
    });
    expect(!reorder_result.ok, "#755: reorder should reject self-targeted relative moves");

    reorder_result = copperfin::vfp::reorder_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "b-guid",
        .placement = "after",
        .target_object_name = "missingObject",
        .target_unique_id = {}
    });
    expect(!reorder_result.ok, "#755: reorder should reject missing target selectors");

    reorder_result = copperfin::vfp::reorder_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "b-guid",
        .placement = "sideways",
        .target_object_name = {},
        .target_unique_id = {}
    });
    expect(!reorder_result.ok, "#755: reorder should reject unsupported placements");
    expect(order_string() == "c-guid*,a-guid,d-guid,b-guid",
        "#755: failed reorder requests should not mutate record order");

    fs::remove_all(temp_dir, ignored);
}

void test_list_visual_object_children_filters_immediate_children() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_children_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "children.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U},
        {.name = "CLASS", .type = 'C', .length = 20U},
        {.name = "BASECLASS", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"", "mainForm", "form-guid", "", "form", "form", "Caption = \"Main\"\r\n"},
        {"cmdSave", "saveButton", "save-guid", "mainForm", "commandbutton", "commandbutton", "Caption = \"Save\"\r\n"},
        {"txtName", "nameBox", "name-guid", "mainForm", "textbox", "textbox", "Caption = \"Name\"\r\n"},
        {"lblNested", "nestedLabel", "nested-guid", "cmdSave", "label", "label", "Caption = \"Nested\"\r\n"},
        {"cmdOther", "otherButton", "other-guid", "", "commandbutton", "commandbutton", ""},
        {"", "", "nameless-guid", "", "custom", "custom", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#756: children fixture should be writable");
    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#756: children fixture should support deleted child setup");

    auto children_result = copperfin::vfp::list_visual_object_children({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "form-guid"
    });
    expect(children_result.ok &&
            children_result.parent_record_index == 0U &&
            children_result.parent_name == "mainForm" &&
            children_result.children.size() == 2U,
        "#756: child listing should support UNIQUEID parent selection and fallback parent NAME resolution");
    if (children_result.ok && children_result.children.size() == 2U) {
        expect(children_result.children[0].unique_id == "save-guid" &&
                !children_result.children[0].deleted &&
                children_result.children[0].parent_name == "mainForm" &&
                children_result.children[0].caption == "\"Save\"",
            "#756: child listing should include live immediate children with outline metadata");
        expect(children_result.children[1].unique_id == "name-guid" &&
                children_result.children[1].deleted &&
                children_result.children[1].parent_name == "mainForm" &&
                children_result.children[1].caption == "\"Name\"",
            "#756: child listing should keep deleted immediate children visible");
    }
    const auto has_grandchild = std::find_if(
        children_result.children.begin(),
        children_result.children.end(),
        [](const copperfin::vfp::VisualObjectSnapshot& child) {
            return child.unique_id == "nested-guid";
        });
    expect(has_grandchild == children_result.children.end(),
        "#756: child listing should exclude grandchildren");

    children_result = copperfin::vfp::list_visual_object_children({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "mainForm",
        .unique_id = {}
    });
    expect(children_result.ok && children_result.children.size() == 2U,
        "#756: child listing should support fallback NAME parent selection");

    children_result = copperfin::vfp::list_visual_object_children({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "missing-guid"
    });
    expect(!children_result.ok, "#756: child listing should fail explicitly for missing parents");

    children_result = copperfin::vfp::list_visual_object_children({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "nameless-guid"
    });
    expect(!children_result.ok, "#756: child listing should fail explicitly for nameless parent rows");

    expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
        "#756: child listing should not create undo history");

    fs::remove_all(temp_dir, ignored);
}

void test_list_visual_object_descendants_walks_container_tree() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_descendants_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "descendants.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U},
        {.name = "CLASS", .type = 'C', .length = 20U},
        {.name = "BASECLASS", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"", "mainForm", "form-guid", "", "form", "form", "Caption = \"Main\"\r\n"},
        {"cntA", "containerA", "a-guid", "mainForm", "container", "container", "Caption = \"A\"\r\n"},
        {"txtName", "nameBox", "name-guid", "cntA", "textbox", "textbox", "Caption = \"Name\"\r\n"},
        {"lblNested", "nestedLabel", "nested-guid", "txtName", "label", "label", "Caption = \"Nested\"\r\n"},
        {"dupContainer", "dupOne", "dup-one-guid", "mainForm", "container", "container", ""},
        {"dupContainer", "dupTwo", "dup-two-guid", "mainForm", "container", "container", ""},
        {"dupChild", "dupChildName", "dup-child-guid", "dupContainer", "label", "label", ""},
        {"cmdOther", "otherButton", "other-guid", "", "commandbutton", "commandbutton", ""},
        {"", "", "nameless-guid", "", "custom", "custom", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#757: descendants fixture should be writable");
    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#757: descendants fixture should support deleted descendant setup");

    auto descendants_result = copperfin::vfp::list_visual_object_descendants({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "form-guid"
    });
    expect(descendants_result.ok &&
            descendants_result.parent_record_index == 0U &&
            descendants_result.parent_name == "mainForm" &&
            descendants_result.descendants.size() == 6U,
        "#757: descendants should support UNIQUEID parent selection and fallback parent NAME resolution");
    if (descendants_result.ok && descendants_result.descendants.size() == 6U) {
        expect(descendants_result.descendants[0].object.unique_id == "a-guid" &&
                descendants_result.descendants[0].depth == 1U,
            "#757: descendants should list immediate children first with depth one");
        expect(descendants_result.descendants[1].object.unique_id == "name-guid" &&
                descendants_result.descendants[1].depth == 2U &&
                descendants_result.descendants[1].object.deleted,
            "#757: descendants should include deleted nested descendants with depth metadata");
        expect(descendants_result.descendants[2].object.unique_id == "nested-guid" &&
                descendants_result.descendants[2].depth == 3U,
            "#757: descendants should walk grandchildren in pre-order");
        expect(descendants_result.descendants[3].object.unique_id == "dup-one-guid" &&
                descendants_result.descendants[4].object.unique_id == "dup-child-guid" &&
                descendants_result.descendants[4].depth == 2U &&
                descendants_result.descendants[5].object.unique_id == "dup-two-guid",
            "#757: descendants should protect duplicate parent-name traversal from duplicate child entries");
    }
    const auto sibling = std::find_if(
        descendants_result.descendants.begin(),
        descendants_result.descendants.end(),
        [](const copperfin::vfp::VisualObjectDescendantSnapshot& descendant) {
            return descendant.object.unique_id == "other-guid";
        });
    expect(sibling == descendants_result.descendants.end(),
        "#757: descendants should exclude sibling/root-level objects outside the selected parent");

    descendants_result = copperfin::vfp::list_visual_object_descendants({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "mainForm",
        .unique_id = {}
    });
    expect(descendants_result.ok && descendants_result.descendants.size() == 6U,
        "#757: descendants should support fallback NAME parent selection");

    descendants_result = copperfin::vfp::list_visual_object_descendants({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "missing-guid"
    });
    expect(!descendants_result.ok, "#757: descendants should fail explicitly for missing parents");

    descendants_result = copperfin::vfp::list_visual_object_descendants({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "nameless-guid"
    });
    expect(!descendants_result.ok, "#757: descendants should fail explicitly for nameless parent rows");

    expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
        "#757: descendant listing should not create undo history");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_subtree_deleted_state_updates_descendants() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_subtree_delete_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "subtree_delete.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cntMain", "mainContainer", "container-guid", ""},
        {"cmdSave", "saveButton", "save-guid", "cntMain"},
        {"txtName", "nameBox", "name-guid", "cntMain"},
        {"lblNested", "nestedLabel", "nested-guid", "txtName"},
        {"cmdOther", "otherButton", "other-guid", ""},
        {"", "", "nameless-guid", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#758: subtree deleted-state fixture should be writable");
    const auto initial_delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .deleted = true
    });
    expect(initial_delete_result.ok, "#758: subtree fixture should support existing deleted descendants");

    const auto is_deleted = [&](const std::string& unique_id) {
        const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
        expect(list_result.ok, "#758: subtree fixture should remain listable");
        const auto object = std::find_if(
            list_result.objects.begin(),
            list_result.objects.end(),
            [&](const copperfin::vfp::VisualObjectSnapshot& candidate) {
                return candidate.unique_id == unique_id;
            });
        expect(object != list_result.objects.end(), "#758: expected subtree object should remain present");
        return object != list_result.objects.end() && object->deleted;
    };

    auto subtree_result = copperfin::vfp::set_visual_object_subtree_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "container-guid",
        .deleted = true
    });
    expect(subtree_result.ok, "#758: subtree deleted-state changes should support UNIQUEID source selection");
    expect(is_deleted("container-guid") &&
            is_deleted("save-guid") &&
            is_deleted("name-guid") &&
            is_deleted("nested-guid"),
        "#758: subtree delete should mark the selected root and all descendants deleted");
    expect(!is_deleted("other-guid"),
        "#758: subtree delete should preserve unrelated root/sibling rows");

    subtree_result = copperfin::vfp::set_visual_object_subtree_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cntMain",
        .unique_id = {},
        .deleted = false
    });
    expect(subtree_result.ok, "#758: subtree deleted-state changes should support object-name source selection");
    expect(!is_deleted("container-guid") &&
            !is_deleted("save-guid") &&
            !is_deleted("name-guid") &&
            !is_deleted("nested-guid"),
        "#758: subtree restore should clear deleted flags on root and descendants");

    subtree_result = copperfin::vfp::set_visual_object_subtree_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "missing-guid",
        .deleted = true
    });
    expect(!subtree_result.ok, "#758: subtree delete should fail explicitly for missing source selections");

    subtree_result = copperfin::vfp::set_visual_object_subtree_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "nameless-guid",
        .deleted = true
    });
    expect(!subtree_result.ok, "#758: subtree delete should fail explicitly for nameless source rows");

    expect(!is_deleted("container-guid") &&
            !is_deleted("save-guid") &&
            !is_deleted("name-guid") &&
            !is_deleted("nested-guid") &&
            !is_deleted("other-guid") &&
            !is_deleted("nameless-guid"),
        "#758: failed subtree deleted-state requests should not mutate existing flags");

    fs::remove_all(temp_dir, ignored);
}

void test_list_visual_object_ancestors_walks_parent_chain() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_ancestors_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U},
        {.name = "CLASS", .type = 'C', .length = 20U},
        {.name = "BASECLASS", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };

    const fs::path table_path = temp_dir / "ancestors.scx";
    const std::vector<std::vector<std::string>> records{
        {"frmMain", "mainForm", "form-guid", "", "form", "form", "Caption = \"Main\"\r\n"},
        {"cntMain", "mainContainer", "container-guid", "frmMain", "container", "container", "Caption = \"Container\"\r\n"},
        {"pgDetails", "detailsPage", "page-guid", "cntMain", "page", "page", "Caption = \"Details\"\r\n"},
        {"cmdSave", "saveButton", "save-guid", "pgDetails", "commandbutton", "commandbutton", "Caption = \"Save\"\r\n"},
        {"cmdOther", "otherButton", "other-guid", "", "commandbutton", "commandbutton", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#759: ancestors fixture should be writable");

    auto ancestors_result = copperfin::vfp::list_visual_object_ancestors({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(ancestors_result.ok &&
            ancestors_result.record_index == 3U &&
            ancestors_result.ancestors.size() == 3U,
        "#759: ancestors should support UNIQUEID source selection");
    if (ancestors_result.ok && ancestors_result.ancestors.size() == 3U) {
        expect(ancestors_result.ancestors[0].object.unique_id == "page-guid" &&
                ancestors_result.ancestors[0].depth == 1U &&
                ancestors_result.ancestors[0].object.caption == "\"Details\"",
            "#759: ancestors should list the immediate parent first with outline metadata");
        expect(ancestors_result.ancestors[1].object.unique_id == "container-guid" &&
                ancestors_result.ancestors[1].depth == 2U,
            "#759: ancestors should include intermediate containers with depth metadata");
        expect(ancestors_result.ancestors[2].object.unique_id == "form-guid" &&
                ancestors_result.ancestors[2].depth == 3U,
            "#759: ancestors should walk upward to the root object");
    }

    ancestors_result = copperfin::vfp::list_visual_object_ancestors({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdSave",
        .unique_id = {}
    });
    expect(ancestors_result.ok && ancestors_result.ancestors.size() == 3U,
        "#759: ancestors should support object-name source selection");

    ancestors_result = copperfin::vfp::list_visual_object_ancestors({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "form-guid"
    });
    expect(ancestors_result.ok && ancestors_result.ancestors.empty(),
        "#759: root objects should return an empty successful ancestor list");

    ancestors_result = copperfin::vfp::list_visual_object_ancestors({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "missing-guid"
    });
    expect(!ancestors_result.ok, "#759: ancestors should fail explicitly for missing source objects");

    const fs::path ambiguous_path = temp_dir / "ambiguous_ancestor.scx";
    const std::vector<std::vector<std::string>> ambiguous_records{
        {"dupParent", "parentOne", "parent-one-guid", "", "container", "container", ""},
        {"dupParent", "parentTwo", "parent-two-guid", "", "container", "container", ""},
        {"cmdChild", "childButton", "child-guid", "dupParent", "commandbutton", "commandbutton", ""}
    };
    const auto ambiguous_create_result = copperfin::vfp::create_dbf_table_file(
        ambiguous_path.string(),
        fields,
        ambiguous_records);
    expect(ambiguous_create_result.ok, "#759: ambiguous ancestor fixture should be writable");
    ancestors_result = copperfin::vfp::list_visual_object_ancestors({
        .path = ambiguous_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "child-guid"
    });
    expect(!ancestors_result.ok, "#759: ancestors should reject ambiguous parent names");

    const fs::path cycle_path = temp_dir / "cycle_ancestor.scx";
    const std::vector<std::vector<std::string>> cycle_records{
        {"cntA", "containerA", "a-guid", "cntB", "container", "container", ""},
        {"cntB", "containerB", "b-guid", "cntA", "container", "container", ""}
    };
    const auto cycle_create_result = copperfin::vfp::create_dbf_table_file(
        cycle_path.string(),
        fields,
        cycle_records);
    expect(cycle_create_result.ok, "#759: cycle ancestor fixture should be writable");
    ancestors_result = copperfin::vfp::list_visual_object_ancestors({
        .path = cycle_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "a-guid"
    });
    expect(!ancestors_result.ok, "#759: ancestors should reject parent cycles");

    expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
        "#759: ancestor listing should not create undo history");

    fs::remove_all(temp_dir, ignored);
}

void test_duplicate_visual_object_subtree_rewrites_copied_parents() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_subtree_duplicate_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "subtree_duplicate.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 20U},
        {.name = "BASECLASS", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmMain", "mainForm", "form-guid", "", "form", "form", "Caption = \"Main\"\r\n", ""},
        {"cntMain", "mainContainer", "container-guid", "frmMain", "container", "container", "Caption = \"Container\"\r\n", ""},
        {"cmdSave", "saveButton", "save-guid", "cntMain", "commandbutton", "commandbutton", "Caption = \"Save\"\r\nLeft = 10\r\n", "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC"},
        {"txtName", "nameBox", "name-guid", "cntMain", "textbox", "textbox", "Caption = \"Name\"\r\n", ""},
        {"lblNested", "nestedLabel", "nested-guid", "txtName", "label", "label", "Caption = \"Nested\"\r\n", ""},
        {"cmdOther", "otherButton", "other-guid", "", "commandbutton", "commandbutton", "", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#760: subtree duplicate fixture should be writable");
    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#760: subtree duplicate fixture should support deleted descendant setup");

    auto duplicate_result = copperfin::vfp::duplicate_visual_object_subtree({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "container-guid",
        .replacements = {
            {.source_unique_id = "container-guid", .new_object_name = "cntCopy", .new_name = "mainContainerCopy", .new_unique_id = "container-copy-guid"},
            {.source_unique_id = "save-guid", .new_object_name = "cmdSaveCopy", .new_name = "saveButtonCopy", .new_unique_id = "save-copy-guid"},
            {.source_unique_id = "name-guid", .new_object_name = "txtNameCopy", .new_name = "nameBoxCopy", .new_unique_id = "name-copy-guid"},
            {.source_unique_id = "nested-guid", .new_object_name = "lblNestedCopy", .new_name = "nestedLabelCopy", .new_unique_id = "nested-copy-guid"}
        }
    });
    expect(duplicate_result.ok &&
            duplicate_result.root_record_index == 6U &&
            duplicate_result.copied_count == 4U,
        "#760: subtree duplicate should append root and descendants in pre-order");

    auto objects_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(objects_result.ok && objects_result.objects.size() == 10U,
        "#760: subtree duplicate should append the copied subtree without removing source rows");
    const auto find_object = [&](const std::string& unique_id) {
        return std::find_if(
            objects_result.objects.begin(),
            objects_result.objects.end(),
            [&](const copperfin::vfp::VisualObjectSnapshot& object) {
                return object.unique_id == unique_id;
            });
    };
    if (objects_result.ok) {
        const auto copied_root = find_object("container-copy-guid");
        const auto copied_save = find_object("save-copy-guid");
        const auto copied_name = find_object("name-copy-guid");
        const auto copied_nested = find_object("nested-copy-guid");
        expect(copied_root != objects_result.objects.end() &&
                copied_root->object_name == "cntCopy" &&
                copied_root->parent_name == "frmMain" &&
                !copied_root->deleted,
            "#760: subtree duplicate should preserve root parent and replacement identity");
        expect(copied_save != objects_result.objects.end() &&
                copied_save->parent_name == "cntCopy" &&
                copied_save->caption == "\"Save\"",
            "#760: subtree duplicate should rewrite copied child parent names and preserve memo properties");
        expect(copied_name != objects_result.objects.end() &&
                copied_name->parent_name == "cntCopy" &&
                copied_name->deleted,
            "#760: subtree duplicate should preserve deleted state for copied descendants");
        expect(copied_nested != objects_result.objects.end() &&
                copied_nested->parent_name == "txtNameCopy",
            "#760: subtree duplicate should rewrite grandchild parent names to copied parent identities");
        expect(find_object("container-guid") != objects_result.objects.end() &&
                find_object("other-guid") != objects_result.objects.end(),
            "#760: subtree duplicate should preserve source and unrelated rows");
    }

    auto method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-copy-guid"
    });
    expect(method_result.ok && find_method_snapshot(method_result.methods, "Click") != nullptr,
        "#760: subtree duplicate should preserve copied METHODS memo content");

    const auto object_count_after_success = objects_result.objects.size();
    duplicate_result = copperfin::vfp::duplicate_visual_object_subtree({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cntMain",
        .unique_id = {},
        .replacements = {
            {.source_unique_id = "container-guid", .new_object_name = "cntCollision", .new_name = "collisionContainer", .new_unique_id = "container-copy-guid"},
            {.source_unique_id = "save-guid", .new_object_name = "cmdCollision", .new_name = "collisionButton", .new_unique_id = "collision-save-guid"},
            {.source_unique_id = "name-guid", .new_object_name = "txtCollision", .new_name = "collisionName", .new_unique_id = "collision-name-guid"},
            {.source_unique_id = "nested-guid", .new_object_name = "lblCollision", .new_name = "collisionNested", .new_unique_id = "collision-nested-guid"}
        }
    });
    expect(!duplicate_result.ok,
        "#760: subtree duplicate should reject replacement identities colliding with existing rows");

    duplicate_result = copperfin::vfp::duplicate_visual_object_subtree({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "container-guid",
        .replacements = {
            {.source_unique_id = "container-guid", .new_object_name = "cntIncomplete", .new_name = "incompleteContainer", .new_unique_id = "incomplete-container-guid"}
        }
    });
    expect(!duplicate_result.ok,
        "#760: subtree duplicate should reject missing replacement identity data for copied descendants");

    objects_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(objects_result.ok && objects_result.objects.size() == object_count_after_success,
        "#760: failed subtree duplicate requests should not mutate object count");

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
    test_update_visual_object_properties_rolls_back_failed_batches();
    test_query_visual_object_property_reads_selected_values();
    test_clear_visual_object_property_resets_selected_values();
    test_copy_visual_object_property_between_selected_objects();
    test_move_visual_object_property_between_selected_objects();
    test_rename_visual_object_memo_property_updates_selected_object();
    test_reorder_visual_object_memo_properties_within_selected_object();
    test_list_visual_object_properties_reads_selected_surface();
    test_set_visual_object_deleted_state_targets_selected_object();
    test_list_visual_objects_reads_selection_outline();
    test_list_visual_objects_reads_hierarchy_metadata();
    test_list_visual_object_methods_reads_selected_methods();
    test_query_visual_object_method_reads_one_selected_method();
    test_update_visual_object_method_updates_and_appends_methods();
    test_delete_visual_object_method_removes_selected_methods();
    test_rename_visual_object_method_updates_declarations();
    test_copy_visual_object_method_between_selected_objects();
    test_move_visual_object_method_between_selected_objects();
    test_reorder_visual_object_methods_within_selected_object();
    test_duplicate_visual_object_appends_identity_safe_copy();
    test_create_visual_object_appends_toolbox_field_values();
    test_reparent_visual_object_updates_container_parent();
    test_update_visual_object_batch_rolls_back_failed_alignment();
    test_set_visual_object_deleted_states_rolls_back_batch_failures();
    test_rename_visual_object_updates_identity_safely();
    test_reorder_visual_object_updates_z_order();
    test_list_visual_object_children_filters_immediate_children();
    test_list_visual_object_descendants_walks_container_tree();
    test_set_visual_object_subtree_deleted_state_updates_descendants();
    test_list_visual_object_ancestors_walks_parent_chain();
    test_duplicate_visual_object_subtree_rewrites_copied_parents();
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
