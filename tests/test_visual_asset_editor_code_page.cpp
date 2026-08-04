// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/visual_asset_editor.h"
#include "test_environment_support.h"
#include "test_process_capture_support.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
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
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::uint8_t>((value >> (index * 8U)) & 0xFFU);
    }
}

void write_be_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>(value & 0xFFU);
}

void write_be_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    for (std::size_t index = 0U; index < 4U; ++index) {
        bytes[offset + index] = static_cast<std::uint8_t>((value >> ((3U - index) * 8U)) & 0xFFU);
    }
}

void write_field_descriptor(
    std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    const std::string& name,
    char type,
    std::uint32_t field_offset,
    std::uint8_t field_length) {
    std::copy(name.begin(), name.end(), bytes.begin() + static_cast<std::ptrdiff_t>(offset));
    bytes[offset + 11U] = static_cast<std::uint8_t>(type);
    write_le_u32(bytes, offset + 12U, field_offset);
    bytes[offset + 16U] = field_length;
}

std::vector<std::uint8_t> read_file_bytes(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

const copperfin::vfp::DbfRecordValue* find_field(
    const copperfin::vfp::DbfRecord& record,
    const std::string& name) {
    const auto value = std::find_if(record.values.begin(), record.values.end(), [&](const auto& candidate) {
        return candidate.field_name == name;
    });
    return value == record.values.end() ? nullptr : &*value;
}

struct MarkedAssetFixture {
    std::filesystem::path table_path;
    std::filesystem::path memo_path;
    std::size_t header_length = 129U;
    std::size_t record_length = 65U;
};

MarkedAssetFixture write_marked_text_asset(
    const std::filesystem::path& root,
    const std::string& extension,
    std::uint8_t code_page_mark,
    const std::string& initial_text_bytes) {
    const std::string memo_extension = extension == ".frx" ? ".frt" : ".lbt";
    MarkedAssetFixture fixture{
        .table_path = root / ("marked_text" + extension),
        .memo_path = root / ("marked_text" + memo_extension)
    };

    std::vector<std::uint8_t> table_bytes(fixture.header_length + fixture.record_length + 1U, 0U);
    table_bytes[0U] = 0x30U;
    write_le_u32(table_bytes, 4U, 1U);
    write_le_u16(table_bytes, 8U, static_cast<std::uint16_t>(fixture.header_length));
    write_le_u16(table_bytes, 10U, static_cast<std::uint16_t>(fixture.record_length));
    table_bytes[29U] = code_page_mark;
    write_field_descriptor(table_bytes, 32U, "OBJNAME", 'C', 1U, 20U);
    write_field_descriptor(table_bytes, 64U, "CAPTION", 'C', 21U, 40U);
    write_field_descriptor(table_bytes, 96U, "PROPERTIES", 'M', 61U, 4U);
    table_bytes[128U] = 0x0DU;
    table_bytes[fixture.header_length] = 0x20U;
    const std::string object_name = "txtTitle";
    std::copy(
        object_name.begin(),
        object_name.end(),
        table_bytes.begin() + static_cast<std::ptrdiff_t>(fixture.header_length + 1U));
    std::fill_n(table_bytes.begin() + static_cast<std::ptrdiff_t>(fixture.header_length + 21U), 40U, static_cast<std::uint8_t>(' '));
    std::copy(initial_text_bytes.begin(), initial_text_bytes.end(), table_bytes.begin() + static_cast<std::ptrdiff_t>(fixture.header_length + 21U));
    write_le_u32(table_bytes, fixture.header_length + 61U, 1U);
    table_bytes.back() = 0x1AU;

    std::vector<std::uint8_t> memo_bytes(1024U, 0U);
    write_be_u32(memo_bytes, 0U, 2U);
    write_be_u16(memo_bytes, 6U, 512U);
    const std::string initial_properties = initial_text_bytes;
    memo_bytes[515U] = 1U;
    write_be_u32(memo_bytes, 516U, static_cast<std::uint32_t>(initial_properties.size()));
    std::copy(initial_properties.begin(), initial_properties.end(), memo_bytes.begin() + 520U);

    std::ofstream table_output(fixture.table_path, std::ios::binary);
    table_output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    table_output.close();
    std::ofstream memo_output(fixture.memo_path, std::ios::binary);
    memo_output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    return fixture;
}

void verify_marked_asset_round_trip(
    const std::filesystem::path& root,
    const std::string& extension,
    std::uint8_t code_page_mark,
    const std::string& initial_bytes,
    const std::string& expected_utf8,
    const std::string& replacement_bytes,
    const std::string& replacement_utf8,
    const std::string& label,
    const std::string& studio_host_path) {
    const MarkedAssetFixture fixture = write_marked_text_asset(root, extension, code_page_mark, initial_bytes);
    const auto before = copperfin::vfp::parse_dbf_table_from_file(fixture.table_path.string(), 1U);
    expect(before.ok, label + " initial asset should parse");
    if (before.ok && !before.table.records.empty()) {
        const auto* caption = find_field(before.table.records.front(), "CAPTION");
        const auto* properties = find_field(before.table.records.front(), "PROPERTIES");
        expect(caption != nullptr && caption->display_value == expected_utf8, label + " direct field should decode to UTF-8");
        expect(properties != nullptr && properties->display_value == expected_utf8,
               label + " property memo should decode to UTF-8");
    }

    const auto direct_update = copperfin::vfp::update_visual_object_property({
        .path = fixture.table_path.string(), .record_index = 0U, .object_name = {}, .unique_id = {},
        .property_name = "CAPTION", .property_value = replacement_utf8
    });
    expect(direct_update.ok, label + " direct text update should succeed: " + direct_update.error);
    const auto memo_update = copperfin::vfp::update_visual_object_property({
        .path = fixture.table_path.string(), .record_index = 0U, .object_name = {}, .unique_id = {},
        .property_name = "PROPERTIES", .property_value = replacement_utf8
    });
    expect(memo_update.ok, label + " memo text update should succeed: " + memo_update.error);

    const auto after = copperfin::vfp::parse_dbf_table_from_file(fixture.table_path.string(), 1U);
    expect(after.ok, label + " updated asset should parse");
    if (after.ok && !after.table.records.empty()) {
        const auto* caption = find_field(after.table.records.front(), "CAPTION");
        const auto* properties = find_field(after.table.records.front(), "PROPERTIES");
        expect(caption != nullptr && caption->display_value == replacement_utf8,
               label + " updated direct field should expose UTF-8");
        expect(properties != nullptr && properties->display_value == replacement_utf8,
               label + " updated property memo should expose UTF-8");
        if (properties != nullptr) {
            const auto memo_payload = copperfin::vfp::read_memo_block_raw(fixture.memo_path.string(), properties->memo_block_number);
            const std::string expected_payload = replacement_bytes;
            expect(memo_payload == std::vector<std::uint8_t>(expected_payload.begin(), expected_payload.end()),
                   label + " updated property memo should retain table code-page bytes");
        }
    }

    if (!studio_host_path.empty()) {
        const auto host_result = copperfin::test_support::normalize_captured_process_line_endings(
            copperfin::test_support::run_process_capture(
                copperfin::test_support::path_from_utf8_string(studio_host_path),
                {
                    "--path",
                    copperfin::test_support::path_to_utf8_string(fixture.table_path),
                    "--json"
                },
                root));
        expect(host_result.started && host_result.exit_code == 0,
               label + " Studio host JSON command should succeed");
        expect(host_result.stdout_text.find(replacement_utf8) != std::string::npos,
               label + " Studio host JSON should expose the UTF-8 text value");
    }

    const auto table_before_rejection = read_file_bytes(fixture.table_path);
    const auto rejected = copperfin::vfp::update_visual_object_property({
        .path = fixture.table_path.string(), .record_index = 0U, .object_name = {}, .unique_id = {},
        .property_name = "CAPTION", .property_value = "\xF0\x9F\x98\x80"
    });
    expect(!rejected.ok, label + " unrepresentable text should fail explicitly");
    expect(read_file_bytes(fixture.table_path) == table_before_rejection,
           label + " rejected text should not alter the table");
}

}  // namespace

int main(int argc, char* argv[]) {
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / "copperfin_visual_asset_code_page_tests";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);

    const std::string studio_host_path = argc == 2 ? argv[1] : std::string{};
    for (const std::string& extension : {std::string{".frx"}, std::string{".lbx"}}) {
        verify_marked_asset_round_trip(root, extension, 0x03U, "caf\xE9", "caf\xC3\xA9", "na\xEFve", "na\xC3\xAFve",
            extension + " CP1252", studio_host_path);
        verify_marked_asset_round_trip(root, extension, 0xC9U, "\xCF\xF0\xE8\xE2\xE5\xF2",
            "\xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82", "\xCC\xE8\xF0",
            "\xD0\x9C\xD0\xB8\xD1\x80", extension + " CP1251", studio_host_path);
    }

    fs::remove_all(root, ignored);
    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
