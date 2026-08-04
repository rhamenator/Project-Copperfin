// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_visual_asset_editor_support.h"
#include "test_environment_support.h"

#include "copperfin/platform/path.h"

#include <array>

namespace cf_test_visual_asset_editor {
namespace {

struct RawAssetFixture {
    std::filesystem::path table_path;
    std::filesystem::path memo_path;
    std::vector<std::uint8_t> table_bytes;
    std::vector<std::uint8_t> memo_bytes;
    std::size_t header_length = 229U;
    std::size_t record_length = 67U;
};

constexpr std::array<std::uint8_t, 10U> opaque_a = {
    0x80U, 0x00U, 0xFEU, 'A', 't', 'a', 'i', 'l', 0x00U, 0xE1U
};
constexpr std::array<std::uint8_t, 10U> opaque_b = {
    0x81U, 0x00U, 0xFDU, 'B', 't', 'a', 'i', 'l', 0x00U, 0xE2U
};
constexpr std::array<std::uint8_t, 10U> opaque_c = {
    0x82U, 0x00U, 0xFCU, 'C', 't', 'a', 'i', 'l', 0x00U, 0xE3U
};

void write_bytes(const std::filesystem::path& path, const std::vector<std::uint8_t>& bytes) {
    std::ofstream output(path, std::ios::binary);
    output.write(
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size()));
}

void write_fixed_field(
    std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    std::size_t length,
    const std::string& value) {
    std::fill_n(
        bytes.begin() + static_cast<std::ptrdiff_t>(offset),
        static_cast<std::ptrdiff_t>(length),
        static_cast<std::uint8_t>(' '));
    std::copy(
        value.begin(),
        value.begin() + static_cast<std::ptrdiff_t>(std::min(length, value.size())),
        bytes.begin() + static_cast<std::ptrdiff_t>(offset));
}

void write_opaque_field(
    std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    const std::array<std::uint8_t, 10U>& value) {
    std::copy(value.begin(), value.end(), bytes.begin() + static_cast<std::ptrdiff_t>(offset));
}

std::vector<std::uint8_t> make_record(
    std::uint8_t deletion_mark,
    const std::string& object_name,
    const std::string& name,
    const std::string& unique_id,
    std::uint32_t memo_block,
    const std::array<std::uint8_t, 10U>& opaque,
    const std::string& parent) {
    std::vector<std::uint8_t> record(67U, 0xA5U);
    record[0] = deletion_mark;
    write_fixed_field(record, 1U, 12U, object_name);
    write_fixed_field(record, 13U, 12U, name);
    write_fixed_field(record, 25U, 16U, unique_id);
    write_le_u32(record, 41U, memo_block);
    write_opaque_field(record, 45U, opaque);
    write_fixed_field(record, 55U, 12U, parent);
    return record;
}

RawAssetFixture make_raw_fixture(const std::filesystem::path& temp_dir, const std::string& extension) {
    RawAssetFixture fixture;
    fixture.table_path = temp_dir / ("raw_preservation" + extension);
    const std::string memo_extension = extension == ".frx" ? ".frt" : ".lbt";
    fixture.memo_path = temp_dir / ("raw_preservation" + memo_extension);

    const std::vector<std::vector<std::uint8_t>> records{
        make_record(0x20U, "cmdA", "buttonA", "a-guid", 1U, opaque_a, ""),
        make_record(0x2AU, "cmdB", "buttonB", "b-guid", 2U, opaque_b, "cmdA"),
        make_record(0x20U, "cmdC", "buttonC", "c-guid", 3U, opaque_c, "cmdB")
    };

    fixture.table_bytes.assign(
        fixture.header_length + (fixture.record_length * records.size()) + 6U,
        0U);
    fixture.table_bytes[0] = 0x30U;
    fixture.table_bytes[1] = 126U;
    fixture.table_bytes[2] = 11U;
    fixture.table_bytes[3] = 23U;
    write_le_u32(fixture.table_bytes, 4U, static_cast<std::uint32_t>(records.size()));
    write_le_u16(fixture.table_bytes, 8U, static_cast<std::uint16_t>(fixture.header_length));
    write_le_u16(fixture.table_bytes, 10U, static_cast<std::uint16_t>(fixture.record_length));
    fixture.table_bytes[12] = 0x91U;
    fixture.table_bytes[13] = 0x00U;
    fixture.table_bytes[14] = 0xE7U;
    fixture.table_bytes[15] = 0x7FU;
    fixture.table_bytes[28] = 0x05U;
    fixture.table_bytes[29] = 0xC8U;
    fixture.table_bytes[30] = 0xA4U;
    fixture.table_bytes[31] = 0x00U;

    write_field_descriptor(fixture.table_bytes, 32U, "OBJNAME", 'C', 1U, 12U);
    write_field_descriptor(fixture.table_bytes, 64U, "NAME", 'C', 13U, 12U);
    write_field_descriptor(fixture.table_bytes, 96U, "UNIQUEID", 'C', 25U, 16U);
    write_field_descriptor(fixture.table_bytes, 128U, "PROPERTIES", 'M', 41U, 4U);
    write_field_descriptor(fixture.table_bytes, 160U, "OPAQUE", 'C', 45U, 10U);
    write_field_descriptor(fixture.table_bytes, 192U, "PARENT", 'C', 55U, 12U);
    fixture.table_bytes[224] = 0x0DU;
    fixture.table_bytes[225] = 0xB1U;
    fixture.table_bytes[226] = 0x00U;
    fixture.table_bytes[227] = 0xD2U;
    fixture.table_bytes[228] = 0x7EU;

    for (std::size_t index = 0U; index < records.size(); ++index) {
        std::copy(
            records[index].begin(),
            records[index].end(),
            fixture.table_bytes.begin() + static_cast<std::ptrdiff_t>(
                fixture.header_length + (index * fixture.record_length)));
    }
    const std::size_t tail_offset = fixture.header_length + (fixture.record_length * records.size());
    fixture.table_bytes[tail_offset] = 0x1AU;
    fixture.table_bytes[tail_offset + 1U] = 0xDEU;
    fixture.table_bytes[tail_offset + 2U] = 0xADU;
    fixture.table_bytes[tail_offset + 3U] = 0x00U;
    fixture.table_bytes[tail_offset + 4U] = 0xF1U;
    fixture.table_bytes[tail_offset + 5U] = 0x7FU;

    fixture.memo_bytes.assign(5U * 512U, 0U);
    write_be_u32(fixture.memo_bytes, 0U, 4U);
    write_be_u16(fixture.memo_bytes, 6U, 512U);
    const std::array<std::vector<std::uint8_t>, 3U> payloads{
        std::vector<std::uint8_t>{'A', ' ', 'm', 'e', 'm', 'o', 0x80U, 0x00U, 'a'},
        std::vector<std::uint8_t>{'B', ' ', 'm', 'e', 'm', 'o', 0x81U, 0x00U, 'b'},
        std::vector<std::uint8_t>{'C', ' ', 'm', 'e', 'm', 'o', 0x82U, 0x00U, 'c'}
    };
    for (std::size_t index = 0U; index < payloads.size(); ++index) {
        const std::size_t block_offset = (index + 1U) * 512U;
        fixture.memo_bytes[block_offset + 3U] = 1U;
        write_be_u32(
            fixture.memo_bytes,
            block_offset + 4U,
            static_cast<std::uint32_t>(payloads[index].size()));
        std::copy(
            payloads[index].begin(),
            payloads[index].end(),
            fixture.memo_bytes.begin() + static_cast<std::ptrdiff_t>(block_offset + 8U));
    }
    fixture.memo_bytes[4U * 512U + 17U] = 0xF3U;
    fixture.memo_bytes[4U * 512U + 18U] = 0x00U;
    fixture.memo_bytes[4U * 512U + 19U] = 0x9EU;

    write_bytes(fixture.table_path, fixture.table_bytes);
    write_bytes(fixture.memo_path, fixture.memo_bytes);
    return fixture;
}

RawAssetFixture make_truncated_fixture(
    const std::filesystem::path& temp_dir,
    const std::string& extension) {
    RawAssetFixture fixture = make_raw_fixture(temp_dir, extension);
    fixture.table_bytes.resize(fixture.header_length + fixture.record_length + 7U);
    write_bytes(fixture.table_path, fixture.table_bytes);
    return fixture;
}

std::vector<std::uint8_t> record_span(const std::vector<std::uint8_t>& table_bytes, std::size_t header_length, std::size_t record_length, std::size_t index) {
    const std::size_t offset = header_length + (index * record_length);
    return {
        table_bytes.begin() + static_cast<std::ptrdiff_t>(offset),
        table_bytes.begin() + static_cast<std::ptrdiff_t>(offset + record_length)
    };
}

bool header_matches_except_record_count(
    const std::vector<std::uint8_t>& original,
    const std::vector<std::uint8_t>& actual,
    std::size_t header_length) {
    if (actual.size() < header_length || original.size() < header_length) {
        return false;
    }
    for (std::size_t index = 0U; index < header_length; ++index) {
        if (index >= 4U && index < 8U) {
            continue;
        }
        if (original[index] != actual[index]) {
            return false;
        }
    }
    return true;
}

bool tail_matches(
    const std::vector<std::uint8_t>& original,
    const std::vector<std::uint8_t>& actual,
    std::size_t original_tail_offset,
    std::size_t actual_tail_offset) {
    if (original.size() < original_tail_offset || actual.size() < actual_tail_offset) {
        return false;
    }
    return std::vector<std::uint8_t>(
               original.begin() + static_cast<std::ptrdiff_t>(original_tail_offset),
               original.end()) ==
           std::vector<std::uint8_t>(
               actual.begin() + static_cast<std::ptrdiff_t>(actual_tail_offset),
               actual.end());
}

void expect_reordered_table(
    const RawAssetFixture& fixture,
    const std::vector<std::uint8_t>& actual,
    const std::vector<std::size_t>& expected_order,
    const std::string& label) {
    const std::size_t record_count = expected_order.size();
    const std::size_t original_tail_offset = fixture.header_length + (fixture.record_length * 3U);
    const std::size_t actual_tail_offset = fixture.header_length + (fixture.record_length * record_count);
    expect(actual.size() == fixture.table_bytes.size(), label + " should preserve full table size");
    expect(header_matches_except_record_count(fixture.table_bytes, actual, fixture.header_length),
           label + " should preserve header metadata and code-page/table flags");
    if (actual.size() >= actual_tail_offset + 1U) {
        for (std::size_t output_index = 0U; output_index < expected_order.size(); ++output_index) {
            expect(
                record_span(actual, fixture.header_length, fixture.record_length, output_index) ==
                    record_span(fixture.table_bytes, fixture.header_length, fixture.record_length, expected_order[output_index]),
                label + " should move complete raw record spans without rewriting fields");
        }
        expect(tail_matches(fixture.table_bytes, actual, original_tail_offset, actual_tail_offset),
               label + " should preserve bytes after the DBF records");
    }
}

void expect_appended_table(
    const RawAssetFixture& fixture,
    const std::vector<std::uint8_t>& actual,
    const std::vector<std::vector<std::uint8_t>>& expected_appends,
    const std::string& label) {
    const std::size_t original_record_count = 3U;
    const std::size_t final_record_count = original_record_count + expected_appends.size();
    expect(
        actual.size() == fixture.table_bytes.size() +
            (expected_appends.size() * fixture.record_length),
        label + " should append complete record spans only");
    expect(
        header_matches_except_record_count(
            fixture.table_bytes,
            actual,
            fixture.header_length),
        label + " should preserve header metadata except record count");
    if (actual.size() < fixture.header_length + (final_record_count * fixture.record_length)) {
        return;
    }
    for (std::size_t index = 0U; index < original_record_count; ++index) {
        expect(
            record_span(actual, fixture.header_length, fixture.record_length, index) ==
                record_span(
                    fixture.table_bytes,
                    fixture.header_length,
                    fixture.record_length,
                    index),
            label + " should preserve each existing raw record span");
    }
    for (std::size_t index = 0U; index < expected_appends.size(); ++index) {
        expect(
            record_span(
                actual,
                fixture.header_length,
                fixture.record_length,
                original_record_count + index) == expected_appends[index],
            label + " should encode the exact appended raw record span");
    }
    expect(
        tail_matches(
            fixture.table_bytes,
            actual,
            fixture.header_length + (original_record_count * fixture.record_length),
            fixture.header_length + (final_record_count * fixture.record_length)),
        label + " should preserve the table tail");
}

void test_one_extension_reorder(const std::filesystem::path& temp_dir, const std::string& extension) {
    const std::string label = "#4057 " + extension;
    auto fixture = make_raw_fixture(temp_dir, extension);
    const auto original_table = fixture.table_bytes;
    const auto original_memo = fixture.memo_bytes;

    auto result = copperfin::vfp::reorder_visual_object({
        .path = fixture.table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "a-guid",
        .placement = "front",
        .target_object_name = {},
        .target_unique_id = {}
    });
    expect(result.ok, label + " no-op reorder should succeed: " + result.error);
    expect(read_file_bytes(fixture.table_path) == original_table,
           label + " no-op reorder should preserve exact table bytes");
    expect(read_file_bytes(fixture.memo_path) == original_memo,
           label + " no-op reorder should preserve exact memo bytes");

    fixture = make_raw_fixture(temp_dir, extension);
    result = copperfin::vfp::reorder_visual_object({
        .path = fixture.table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "c-guid",
        .placement = "front",
        .target_object_name = {},
        .target_unique_id = {}
    });
    expect(result.ok, label + " non-no-op reorder should succeed: " + result.error);
    const auto reordered_table = read_file_bytes(fixture.table_path);
    expect_reordered_table(fixture, reordered_table, {2U, 0U, 1U}, label + " reorder");
    expect(read_file_bytes(fixture.memo_path) == fixture.memo_bytes,
           label + " reorder should preserve exact memo sidecar bytes");
}

void test_one_extension_duplicate(const std::filesystem::path& temp_dir, const std::string& extension) {
    const std::string label = "#4057 " + extension;
    const auto fixture = make_raw_fixture(temp_dir, extension);
    const auto original_table = fixture.table_bytes;
    const auto original_memo = fixture.memo_bytes;
    auto result = copperfin::vfp::duplicate_visual_object({
        .path = fixture.table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "b-guid",
        .new_object_name = "cmdBCopy",
        .new_name = "buttonBCopy",
        .new_unique_id = "b-copy-guid"
    });
    expect(result.ok, label + " duplicate should succeed: " + result.error);

    const auto actual = read_file_bytes(fixture.table_path);
    expect(actual.size() == original_table.size() + fixture.record_length,
           label + " duplicate should append exactly one record span");
    expect(header_matches_except_record_count(original_table, actual, fixture.header_length),
           label + " duplicate should preserve header metadata except record count");
    if (actual.size() >= fixture.header_length + (4U * fixture.record_length)) {
        for (std::size_t index = 0U; index < 3U; ++index) {
            expect(
                record_span(actual, fixture.header_length, fixture.record_length, index) ==
                    record_span(original_table, fixture.header_length, fixture.record_length, index),
                label + " duplicate should preserve every existing raw record span");
        }
        auto expected_copy = record_span(original_table, fixture.header_length, fixture.record_length, 1U);
        write_fixed_field(expected_copy, 1U, 12U, "cmdBCopy");
        write_fixed_field(expected_copy, 13U, 12U, "buttonBCopy");
        write_fixed_field(expected_copy, 25U, 16U, "b-copy-guid");
        expect(
            record_span(actual, fixture.header_length, fixture.record_length, 3U) == expected_copy,
            label + " duplicate should only change requested identity fields in the copied span");
        expect(
            tail_matches(
                original_table,
                actual,
                fixture.header_length + (3U * fixture.record_length),
                fixture.header_length + (4U * fixture.record_length)),
            label + " duplicate should preserve the table tail");
    }
    expect(read_file_bytes(fixture.memo_path) == original_memo,
           label + " duplicate should preserve exact memo sidecar bytes and payloads");
}

void test_one_extension_create(const std::filesystem::path& temp_dir, const std::string& extension) {
    const std::string label = "#4057 " + extension;
    const auto fixture = make_raw_fixture(temp_dir, extension);
    const auto original_table = fixture.table_bytes;
    const auto original_memo = fixture.memo_bytes;
    const auto result = copperfin::vfp::create_visual_object({
        .path = fixture.table_path.string(),
        .field_values = {
            {.property_name = "OBJNAME", .property_value = "cmdCreated"},
            {.property_name = "NAME", .property_value = "btnCreated"},
            {.property_name = "UNIQUEID", .property_value = "created-guid"}
        }
    });
    expect(result.ok, label + " create should succeed: " + result.error);
    const auto actual = read_file_bytes(fixture.table_path);
    expect(actual.size() == original_table.size() + fixture.record_length,
           label + " create should append one record without changing record width");
    expect(header_matches_except_record_count(original_table, actual, fixture.header_length),
           label + " create should preserve header metadata except record count");
    if (actual.size() >= fixture.header_length + (4U * fixture.record_length)) {
        for (std::size_t index = 0U; index < 3U; ++index) {
            expect(
                record_span(actual, fixture.header_length, fixture.record_length, index) ==
                    record_span(original_table, fixture.header_length, fixture.record_length, index),
                label + " create should preserve each existing raw record span");
        }
        expect(
            tail_matches(
                original_table,
                actual,
                fixture.header_length + (3U * fixture.record_length),
                fixture.header_length + (4U * fixture.record_length)),
            label + " create should preserve the table tail");
    }
    expect(read_file_bytes(fixture.memo_path) == original_memo,
           label + " create should preserve unrelated memo sidecar bytes");
}

void test_one_extension_batch_structural_edits(
    const std::filesystem::path& temp_dir,
    const std::string& extension) {
    const std::string label = "#4057 batch structural " + extension;

    {
        const auto fixture = make_raw_fixture(temp_dir, extension);
        const auto result = copperfin::vfp::duplicate_visual_objects({
            .path = fixture.table_path.string(),
            .objects = {
                {
                    .record_index = 0U,
                    .object_name = {},
                    .unique_id = "a-guid",
                    .new_object_name = "cmdACopy",
                    .new_name = "buttonACopy",
                    .new_unique_id = "a-batch-guid"
                },
                {
                    .record_index = 0U,
                    .object_name = {},
                    .unique_id = "c-guid",
                    .new_object_name = "cmdCCopy",
                    .new_name = "buttonCCopy",
                    .new_unique_id = "c-batch-guid"
                }
            }
        });
        expect(result.ok && result.record_indexes == std::vector<std::size_t>({3U, 4U}),
               label + " duplicate batch should append both copies: " + result.error);
        auto expected_a = record_span(
            fixture.table_bytes,
            fixture.header_length,
            fixture.record_length,
            0U);
        write_fixed_field(expected_a, 1U, 12U, "cmdACopy");
        write_fixed_field(expected_a, 13U, 12U, "buttonACopy");
        write_fixed_field(expected_a, 25U, 16U, "a-batch-guid");
        auto expected_c = record_span(
            fixture.table_bytes,
            fixture.header_length,
            fixture.record_length,
            2U);
        write_fixed_field(expected_c, 1U, 12U, "cmdCCopy");
        write_fixed_field(expected_c, 13U, 12U, "buttonCCopy");
        write_fixed_field(expected_c, 25U, 16U, "c-batch-guid");
        expect_appended_table(
            fixture,
            read_file_bytes(fixture.table_path),
            {expected_a, expected_c},
            label + " duplicate batch");
        expect(read_file_bytes(fixture.memo_path) == fixture.memo_bytes,
               label + " duplicate batch should preserve the exact memo sidecar");
    }

    {
        const auto fixture = make_raw_fixture(temp_dir, extension);
        const auto result = copperfin::vfp::create_visual_objects({
            .path = fixture.table_path.string(),
            .objects = {
                {.field_values = {
                    {.property_name = "OBJNAME", .property_value = "cmdNewOne"},
                    {.property_name = "NAME", .property_value = "btnNewOne"},
                    {.property_name = "UNIQUEID", .property_value = "new-one-guid"}
                }},
                {.field_values = {
                    {.property_name = "OBJNAME", .property_value = "cmdNewTwo"},
                    {.property_name = "NAME", .property_value = "btnNewTwo"},
                    {.property_name = "UNIQUEID", .property_value = "new-two-guid"}
                }}
            }
        });
        expect(result.ok && result.record_indexes == std::vector<std::size_t>({3U, 4U}),
               label + " create batch should append both records: " + result.error);
        auto expected_one = std::vector<std::uint8_t>(
            fixture.record_length,
            static_cast<std::uint8_t>(' '));
        std::fill_n(expected_one.begin() + 41, 4U, static_cast<std::uint8_t>(0U));
        write_fixed_field(expected_one, 1U, 12U, "cmdNewOne");
        write_fixed_field(expected_one, 13U, 12U, "btnNewOne");
        write_fixed_field(expected_one, 25U, 16U, "new-one-guid");
        auto expected_two = expected_one;
        write_fixed_field(expected_two, 1U, 12U, "cmdNewTwo");
        write_fixed_field(expected_two, 13U, 12U, "btnNewTwo");
        write_fixed_field(expected_two, 25U, 16U, "new-two-guid");
        expect_appended_table(
            fixture,
            read_file_bytes(fixture.table_path),
            {expected_one, expected_two},
            label + " create batch");
        expect(read_file_bytes(fixture.memo_path) == fixture.memo_bytes,
               label + " create batch should preserve the exact memo sidecar");
    }

    {
        const auto fixture = make_raw_fixture(temp_dir, extension);
        const auto result = copperfin::vfp::reorder_visual_objects({
            .path = fixture.table_path.string(),
            .objects = {
                {
                    .record_index = 0U,
                    .object_name = {},
                    .unique_id = "c-guid",
                    .placement = "front",
                    .target_object_name = {},
                    .target_unique_id = {}
                },
                {
                    .record_index = 0U,
                    .object_name = {},
                    .unique_id = "a-guid",
                    .placement = "back",
                    .target_object_name = {},
                    .target_unique_id = {}
                }
            }
        });
        expect(result.ok && result.affected_object_count == 2U,
               label + " reorder batch should succeed: " + result.error);
        expect_reordered_table(
            fixture,
            read_file_bytes(fixture.table_path),
            {2U, 1U, 0U},
            label + " reorder batch");
        expect(read_file_bytes(fixture.memo_path) == fixture.memo_bytes,
               label + " reorder batch should preserve the exact memo sidecar");
    }

    {
        const auto fixture = make_raw_fixture(temp_dir, extension);
        const auto result = copperfin::vfp::duplicate_visual_object_subtree({
            .path = fixture.table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "a-guid",
            .replacements = {
                {
                    .source_unique_id = "a-guid",
                    .new_object_name = "cmdACopy",
                    .new_name = "buttonACopy",
                    .new_unique_id = "a-tree-guid"
                },
                {
                    .source_unique_id = "b-guid",
                    .new_object_name = "cmdBCopy",
                    .new_name = "buttonBCopy",
                    .new_unique_id = "b-tree-guid"
                },
                {
                    .source_unique_id = "c-guid",
                    .new_object_name = "cmdCCopy",
                    .new_name = "buttonCCopy",
                    .new_unique_id = "c-tree-guid"
                }
            }
        });
        expect(result.ok && result.root_record_index == 3U && result.copied_count == 3U,
               label + " subtree duplicate should append the hierarchy: " + result.error);
        std::vector<std::vector<std::uint8_t>> expected_copies;
        for (std::size_t index = 0U; index < 3U; ++index) {
            expected_copies.push_back(record_span(
                fixture.table_bytes,
                fixture.header_length,
                fixture.record_length,
                index));
        }
        for (std::size_t index = 0U; index < expected_copies.size(); ++index) {
            const char letter = static_cast<char>('A' + index);
            write_fixed_field(expected_copies[index], 1U, 12U, "cmd" + std::string(1U, letter) + "Copy");
            write_fixed_field(expected_copies[index], 13U, 12U, "button" + std::string(1U, letter) + "Copy");
            write_fixed_field(expected_copies[index], 25U, 16U, std::string(1U, static_cast<char>('a' + index)) + "-tree-guid");
        }
        write_fixed_field(expected_copies[1], 55U, 12U, "cmdACopy");
        write_fixed_field(expected_copies[2], 55U, 12U, "cmdBCopy");
        expect_appended_table(
            fixture,
            read_file_bytes(fixture.table_path),
            expected_copies,
            label + " subtree duplicate");
        expect(read_file_bytes(fixture.memo_path) == fixture.memo_bytes,
               label + " subtree duplicate should preserve the exact memo sidecar");
    }
}

void test_one_extension_create_appends_raw_memo(
    const std::filesystem::path& temp_dir,
    const std::string& extension) {
    const std::string label = "#4057 memo append " + extension;
    auto fixture = make_raw_fixture(temp_dir, extension);

    // Block 4 is occupied by the fixture's opaque tail, so the append must use block 5.
    write_be_u32(fixture.memo_bytes, 0U, 5U);
    write_bytes(fixture.memo_path, fixture.memo_bytes);
    const auto original_table = fixture.table_bytes;
    const auto original_memo = fixture.memo_bytes;
    const std::string memo_value{
        static_cast<char>(0xC3U), 'm', static_cast<char>(0x00U), 'e',
        static_cast<char>(0xF1U), 'm', static_cast<char>(0x81U),
        static_cast<char>(0x00U), 'o', static_cast<char>(0xFEU),
        't', 'a', 'i', 'l'};

    const auto result = copperfin::vfp::create_visual_object({
        .path = fixture.table_path.string(),
        .field_values = {
            {.property_name = "OBJNAME", .property_value = "cmdMemoNew"},
            {.property_name = "NAME", .property_value = "btnMemoNew"},
            {.property_name = "UNIQUEID", .property_value = "memo-guid-4057"},
            {.property_name = "PROPERTIES", .property_value = memo_value}
        }
    });
    expect(result.ok, label + " create with binary memo value should succeed: " + result.error);

    const auto actual_table = read_file_bytes(fixture.table_path);
    expect(header_matches_except_record_count(original_table, actual_table, fixture.header_length),
           label + " should preserve table header metadata except record count");
    expect(actual_table.size() == original_table.size() + fixture.record_length,
           label + " should append one record");
    for (std::size_t index = 0U; index < 3U; ++index) {
        expect(
            record_span(actual_table, fixture.header_length, fixture.record_length, index) ==
                record_span(original_table, fixture.header_length, fixture.record_length, index),
            label + " should preserve each existing record span");
    }
    auto expected_record = std::vector<std::uint8_t>(fixture.record_length, static_cast<std::uint8_t>(' '));
    write_fixed_field(expected_record, 1U, 12U, "cmdMemoNew");
    write_fixed_field(expected_record, 13U, 12U, "btnMemoNew");
    write_fixed_field(expected_record, 25U, 16U, "memo-guid-4057");
    write_le_u32(expected_record, 41U, 5U);
    expect(
        record_span(actual_table, fixture.header_length, fixture.record_length, 3U) == expected_record,
        label + " should write the exact new record bytes and memo pointer");

    auto expected_memo = original_memo;
    expected_memo.resize(6U * 512U, 0U);
    const std::size_t block_offset = 5U * 512U;
    expected_memo[block_offset + 3U] = 1U;
    write_be_u32(expected_memo, block_offset + 4U, static_cast<std::uint32_t>(memo_value.size()));
    std::copy(
        memo_value.begin(),
        memo_value.end(),
        expected_memo.begin() + static_cast<std::ptrdiff_t>(block_offset + 8U));
    write_be_u32(expected_memo, 0U, 6U);
    expect(read_file_bytes(fixture.memo_path) == expected_memo,
           label + " should preserve the old memo bytes and append the exact binary payload");
}

void test_one_extension_create_write_fault(
    const std::filesystem::path& temp_dir,
    const std::string& extension,
    const std::string& stage) {
    const std::string label = "#4057 " + stage + " " + extension;
    const auto fixture = make_raw_fixture(temp_dir, extension);
    const auto original_table = read_file_bytes(fixture.table_path);
    const auto original_memo = read_file_bytes(fixture.memo_path);
    const std::string path_marker = fixture.table_path.filename().string();
    copperfin::test_support::ScopedEnvironmentValue fail_path(
        "COPPERFIN_TEST_FAIL_WRITE_PATH_CONTAINS", path_marker);
    copperfin::test_support::ScopedEnvironmentValue fail_stage(
        "COPPERFIN_TEST_FAIL_WRITE_STAGE", stage);

    const auto result = copperfin::vfp::create_visual_object({
        .path = fixture.table_path.string(),
        .field_values = {
            {.property_name = "OBJNAME", .property_value = "cmdFault"},
            {.property_name = "NAME", .property_value = "btnFault"},
            {.property_name = "UNIQUEID", .property_value = "fault-guid"},
            {.property_name = "PROPERTIES", .property_value = "fault"}
        }
    });
    expect(!result.ok, label + " should fail at the injected transaction stage");
    expect(read_file_bytes(fixture.table_path) == original_table &&
               read_file_bytes(fixture.memo_path) == original_memo,
           label + " should preserve original table and memo bytes");
    expect(!std::filesystem::exists(fixture.table_path.string() + ".cptmp") &&
               !std::filesystem::exists(fixture.table_path.string() + ".cpbak") &&
               !std::filesystem::exists(fixture.memo_path.string() + ".cptmp") &&
               !std::filesystem::exists(fixture.memo_path.string() + ".cpbak") &&
               !std::filesystem::exists(fixture.table_path.string() + ".cpcommit"),
           label + " should clean transaction artifacts");
}

void test_table_only_create_write_fault(
    const std::filesystem::path& temp_dir,
    const std::string& stage) {
    const std::string label = "#4057 table-only " + stage;
    const std::filesystem::path table_path =
        temp_dir / ("raw_table_only_" + stage + ".dbf");
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 12U},
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "OPAQUE", .type = 'C', .length = 10U}
    };
    const auto fixture_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        fields,
        {{"cmdExisting", "btnExisting", "existing-guid", "opaque-old"}});
    expect(fixture_result.ok, label + " fixture should be created");
    const auto original_table = read_file_bytes(table_path);
    const std::string path_marker = table_path.filename().string();
    copperfin::test_support::ScopedEnvironmentValue fail_path(
        "COPPERFIN_TEST_FAIL_WRITE_PATH_CONTAINS", path_marker);
    copperfin::test_support::ScopedEnvironmentValue fail_stage(
        "COPPERFIN_TEST_FAIL_WRITE_STAGE", stage);

    const auto result = copperfin::vfp::create_visual_object({
        .path = table_path.string(),
        .field_values = {
            {.property_name = "OBJNAME", .property_value = "cmdCreated"},
            {.property_name = "NAME", .property_value = "btnCreated"},
            {.property_name = "UNIQUEID", .property_value = "created-guid"},
            {.property_name = "OPAQUE", .property_value = "opaque-new"}
        }
    });
    expect(!result.ok, label + " should fail at the injected transaction stage");
    expect(read_file_bytes(table_path) == original_table,
           label + " should preserve the complete original DBF");
    expect(!std::filesystem::exists(table_path.string() + ".cptmp") &&
               !std::filesystem::exists(table_path.string() + ".cpbak") &&
               !std::filesystem::exists(table_path.string() + ".cpcommit") &&
               !std::filesystem::exists(
                   table_path.parent_path() /
                   (table_path.stem().string() + ".fpt.cptmp")) &&
               !std::filesystem::exists(
                   table_path.parent_path() /
                   (table_path.stem().string() + ".fpt.cpbak")),
           label + " should clean table-only transaction artifacts");
}

void test_table_only_recovery_keeps_committed_target(
    const std::filesystem::path& temp_dir) {
    const std::string label = "#4057 table-only committed recovery";
    const std::filesystem::path table_path = temp_dir / "raw_table_only_committed.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 12U},
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U}
    };
    auto fixture_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        fields,
        {{"cmdOld", "btnOld", "old-guid"}});
    expect(fixture_result.ok, label + " old fixture should be created");
    const auto old_table = read_file_bytes(table_path);

    fixture_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        fields,
        {{"cmdCommitted", "btnCommitted", "committed-guid"}});
    expect(fixture_result.ok, label + " committed fixture should be created");
    write_bytes(table_path.string() + ".cpbak", old_table);

    const auto result = copperfin::vfp::create_visual_object({
        .path = table_path.string(),
        .field_values = {
            {.property_name = "OBJNAME", .property_value = "cmdNext"},
            {.property_name = "NAME", .property_value = "btnNext"},
            {.property_name = "UNIQUEID", .property_value = "next-guid"}
        }
    });
    expect(result.ok, label + " should complete the next append: " + result.error);
    const auto table_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
    const auto* first_unique_id = table_result.ok && !table_result.table.records.empty()
        ? find_record_field(table_result.table.records.front(), "UNIQUEID")
        : nullptr;
    expect(table_result.ok && table_result.table.records.size() == 2U &&
               first_unique_id != nullptr &&
               first_unique_id->display_value == "committed-guid",
           label + " should keep the committed target instead of restoring its old backup");
    expect(!std::filesystem::exists(table_path.string() + ".cpbak") &&
               !std::filesystem::exists(table_path.string() + ".cptmp"),
           label + " should remove stale single-file transaction artifacts");
}

void test_table_only_dbf_ignores_ambiguous_stray_sidecars(
    const std::filesystem::path& temp_dir) {
    const std::string label = "#4057 table-only ambiguous stray sidecars";
    const std::filesystem::path table_path = temp_dir / "raw_table_only_ambiguous.dbf";
    const std::filesystem::path lower_sidecar = temp_dir / "raw_table_only_ambiguous.fpt";
    const std::filesystem::path upper_sidecar = temp_dir / "raw_table_only_ambiguous.FPT";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 12U},
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U}
    };
    const auto fixture_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        fields,
        {{"cmdExisting", "btnExisting", "existing-guid"}});
    expect(fixture_result.ok, label + " fixture should be created");
    write_bytes(lower_sidecar, {0x10U, 0x00U, 0x80U});
    write_bytes(upper_sidecar, {0x20U, 0x00U, 0xF0U});

    std::error_code error;
    const bool distinct_sidecars =
        std::filesystem::exists(lower_sidecar, error) && !error &&
        std::filesystem::exists(upper_sidecar, error) && !error &&
        !std::filesystem::equivalent(lower_sidecar, upper_sidecar, error) && !error;
    if (!distinct_sidecars) {
        return;
    }
    const auto lower_bytes = read_file_bytes(lower_sidecar);
    const auto upper_bytes = read_file_bytes(upper_sidecar);

    const auto result = copperfin::vfp::create_visual_object({
        .path = table_path.string(),
        .field_values = {
            {.property_name = "OBJNAME", .property_value = "cmdCreated"},
            {.property_name = "NAME", .property_value = "btnCreated"},
            {.property_name = "UNIQUEID", .property_value = "created-guid"}
        }
    });
    expect(result.ok, label + " should remain a table-only mutation: " + result.error);
    const auto batch_result = copperfin::vfp::duplicate_visual_objects({
        .path = table_path.string(),
        .objects = {{
            .record_index = 0U,
            .object_name = {},
            .unique_id = "created-guid",
            .new_object_name = "cmdCopy",
            .new_name = "btnCopy",
            .new_unique_id = "copy-guid"
        }}
    });
    expect(batch_result.ok && batch_result.record_indexes == std::vector<std::size_t>({2U}),
           label + " batch snapshot should also remain table-only: " + batch_result.error);
    const auto table_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 3U);
    expect(table_result.ok && table_result.table.records.size() == 3U,
           label + " should append both requested records");
    expect(read_file_bytes(lower_sidecar) == lower_bytes &&
               read_file_bytes(upper_sidecar) == upper_bytes,
           label + " should not read, rewrite, or remove unrelated FPT files");
}

void test_one_extension_malformed_no_write(const std::filesystem::path& temp_dir, const std::string& extension) {
    const std::string label = "#4057 malformed " + extension;
    {
        const auto fixture = make_truncated_fixture(temp_dir, extension);
        const auto before_table = read_file_bytes(fixture.table_path);
        const auto before_memo = read_file_bytes(fixture.memo_path);
        const auto result = copperfin::vfp::reorder_visual_object({
            .path = fixture.table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "a-guid",
            .placement = "front",
            .target_object_name = {},
            .target_unique_id = {}
        });
        expect(!result.ok, label + " reorder should reject a truncated record layout");
        expect(read_file_bytes(fixture.table_path) == before_table &&
                   read_file_bytes(fixture.memo_path) == before_memo,
               label + " reorder failure should not write either asset file");
    }
    {
        const auto fixture = make_truncated_fixture(temp_dir, extension);
        const auto before_table = read_file_bytes(fixture.table_path);
        const auto before_memo = read_file_bytes(fixture.memo_path);
        const auto result = copperfin::vfp::duplicate_visual_object({
            .path = fixture.table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "a-guid",
            .new_object_name = "cmdACopy",
            .new_name = "buttonACopy",
            .new_unique_id = "a-copy-guid"
        });
        expect(!result.ok, label + " duplicate should reject a truncated record layout");
        expect(read_file_bytes(fixture.table_path) == before_table &&
                   read_file_bytes(fixture.memo_path) == before_memo,
               label + " duplicate failure should not write either asset file");
    }
    {
        const auto fixture = make_truncated_fixture(temp_dir, extension);
        const auto before_table = read_file_bytes(fixture.table_path);
        const auto before_memo = read_file_bytes(fixture.memo_path);
        const auto result = copperfin::vfp::create_visual_object({
            .path = fixture.table_path.string(),
            .field_values = {
                {.property_name = "OBJNAME", .property_value = "cmdCreated"},
                {.property_name = "NAME", .property_value = "btnCreated"},
                {.property_name = "UNIQUEID", .property_value = "created-guid"}
            }
        });
        expect(!result.ok, label + " create should reject a truncated record layout");
        expect(read_file_bytes(fixture.table_path) == before_table &&
                   read_file_bytes(fixture.memo_path) == before_memo,
               label + " create failure should not write either asset file");
    }
}

}  // namespace

void test_visual_asset_raw_reorder_preserves_frx_frt_and_lbx_lbt_bytes() {
    const auto temp_dir = std::filesystem::temp_directory_path() /
        ("copperfin_visual_asset_raw_reorder_" + std::to_string(_getpid()));
    std::error_code ignored;
    std::filesystem::remove_all(temp_dir, ignored);
    std::filesystem::create_directories(temp_dir);
    test_one_extension_reorder(temp_dir, ".frx");
    test_one_extension_reorder(temp_dir, ".lbx");
    std::filesystem::remove_all(temp_dir, ignored);
}

void test_visual_asset_raw_duplicate_preserves_frx_frt_and_lbx_lbt_bytes() {
    const auto temp_dir = std::filesystem::temp_directory_path() /
        ("copperfin_visual_asset_raw_duplicate_" + std::to_string(_getpid()));
    std::error_code ignored;
    std::filesystem::remove_all(temp_dir, ignored);
    std::filesystem::create_directories(temp_dir);
    test_one_extension_duplicate(temp_dir, ".frx");
    test_one_extension_duplicate(temp_dir, ".lbx");
    std::filesystem::remove_all(temp_dir, ignored);
}

void test_visual_asset_raw_create_preserves_frx_frt_and_lbx_lbt_records() {
    const auto temp_dir = std::filesystem::temp_directory_path() /
        ("copperfin_visual_asset_raw_create_" + std::to_string(_getpid()));
    std::error_code ignored;
    std::filesystem::remove_all(temp_dir, ignored);
    std::filesystem::create_directories(temp_dir);
    test_one_extension_create(temp_dir, ".frx");
    test_one_extension_create(temp_dir, ".lbx");
    std::filesystem::remove_all(temp_dir, ignored);
}

void test_visual_asset_raw_batch_and_subtree_edits_preserve_bytes() {
    const auto temp_dir = std::filesystem::temp_directory_path() /
        ("copperfin_visual_asset_raw_batch_" + std::to_string(_getpid()));
    std::error_code ignored;
    std::filesystem::remove_all(temp_dir, ignored);
    std::filesystem::create_directories(temp_dir);
    test_one_extension_batch_structural_edits(temp_dir, ".frx");
    test_one_extension_batch_structural_edits(temp_dir, ".lbx");
    std::filesystem::remove_all(temp_dir, ignored);
}

void test_visual_asset_raw_malformed_layouts_fail_without_writes() {
    const auto temp_dir = std::filesystem::temp_directory_path() /
        ("copperfin_visual_asset_raw_malformed_" + std::to_string(_getpid()));
    std::error_code ignored;
    std::filesystem::remove_all(temp_dir, ignored);
    std::filesystem::create_directories(temp_dir);
    test_one_extension_malformed_no_write(temp_dir, ".frx");
    test_one_extension_malformed_no_write(temp_dir, ".lbx");
    std::filesystem::remove_all(temp_dir, ignored);
}

void test_visual_asset_raw_create_preserves_binary_memo_payloads() {
    const auto temp_dir = std::filesystem::temp_directory_path() /
        ("copperfin_visual_asset_raw_memo_" + std::to_string(_getpid()));
    std::error_code ignored;
    std::filesystem::remove_all(temp_dir, ignored);
    std::filesystem::create_directories(temp_dir);
    test_one_extension_create_appends_raw_memo(temp_dir, ".frx");
    test_one_extension_create_appends_raw_memo(temp_dir, ".lbx");
    std::filesystem::remove_all(temp_dir, ignored);
}

void test_visual_asset_raw_create_write_faults_are_atomic() {
    const auto temp_dir = std::filesystem::temp_directory_path() /
        ("copperfin_visual_asset_raw_fault_" + std::to_string(_getpid()));
    std::error_code ignored;
    std::filesystem::remove_all(temp_dir, ignored);
    std::filesystem::create_directories(temp_dir);
    for (const auto& stage : {
             std::string("sidecar-stage"),
             std::string("primary-stage"),
             std::string("first-commit"),
             std::string("second-commit"),
             std::string("rollback")
         }) {
        test_one_extension_create_write_fault(temp_dir, ".frx", stage);
        test_one_extension_create_write_fault(temp_dir, ".lbx", stage);
    }
    for (const auto& stage : {
             std::string("primary-stage"),
             std::string("first-commit"),
             std::string("rollback")
         }) {
        test_table_only_create_write_fault(temp_dir, stage);
    }
    test_table_only_recovery_keeps_committed_target(temp_dir);
    test_table_only_dbf_ignores_ambiguous_stray_sidecars(temp_dir);
    std::filesystem::remove_all(temp_dir, ignored);
}

void test_visual_asset_raw_unicode_path_transaction_round_trip() {
    const auto temp_dir = std::filesystem::temp_directory_path() /
        copperfin::platform::path_from_utf8_string(
            "copperfin_visual_asset_raw-\xD0\xBF\xD1\x83\xD1\x82\xD1\x8C");
    std::error_code ignored;
    std::filesystem::remove_all(temp_dir, ignored);
    std::filesystem::create_directories(temp_dir);
    const auto fixture = make_raw_fixture(temp_dir, ".frx");
    const auto result = copperfin::vfp::create_visual_object({
        .path = copperfin::platform::path_to_utf8_string(fixture.table_path),
        .field_values = {
            {.property_name = "OBJNAME", .property_value = "cmdUnicode"},
            {.property_name = "NAME", .property_value = "btnUnicode"},
            {.property_name = "UNIQUEID", .property_value = "unicode-guid"},
            {.property_name = "PROPERTIES", .property_value = "unicode"}
        }
    });
    expect(result.ok, "#4245 Unicode visual-asset transaction should round-trip: " + result.error);

    auto staged = fixture.table_path;
    staged += ".cptmp";
    auto backup = fixture.table_path;
    backup += ".cpbak";
    auto marker = fixture.table_path;
    marker += ".cpcommit";
    expect(!std::filesystem::exists(staged) && !std::filesystem::exists(backup) &&
               !std::filesystem::exists(marker),
           "#4245 Unicode visual-asset transaction should clean temporary artifacts");
    expect(!read_file_bytes(fixture.table_path).empty() &&
               !read_file_bytes(fixture.memo_path).empty(),
           "#4245 Unicode visual-asset transaction should preserve accessible FRX/FRT files");
    std::filesystem::remove_all(temp_dir, ignored);
}

}  // namespace cf_test_visual_asset_editor
