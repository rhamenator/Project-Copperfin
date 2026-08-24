// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_dbf_table_support.h"

#include "copperfin/vfp/dbf_table.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#define _getpid getpid
#endif

namespace {

std::vector<std::uint8_t> read_binary_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
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

}  // namespace

namespace copperfin::test_dbf_table {

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

}  // namespace copperfin::test_dbf_table
