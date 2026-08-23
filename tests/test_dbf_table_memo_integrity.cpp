// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_dbf_table_support.h"

#include "copperfin/vfp/dbf_table.h"

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

namespace copperfin::test_dbf_table {
void test_pack_memo_fails_when_referenced_payload_cannot_be_recovered() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_pack_unreadable_memo_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "pack_unreadable.dbf";
    const fs::path memo_path = temp_dir / "pack_unreadable.fpt";

    std::vector<std::uint8_t> table_bytes(65U + 5U + 1U, 0U);
    table_bytes[0] = 0x30U;
    write_le_u32(table_bytes, 4U, 1U);
    write_le_u16(table_bytes, 8U, 65U);
    write_le_u16(table_bytes, 10U, 5U);
    write_field_descriptor(table_bytes, 32U, "BODY", 'M', 1U, 4U);
    table_bytes[64U] = 0x0DU;
    table_bytes[65U] = 0x20U;
    write_le_u32(table_bytes, 66U, 1U);
    table_bytes.back() = 0x1AU;

    std::vector<std::uint8_t> memo_bytes(512U, 0U);
    write_be_u16(memo_bytes, 6U, 1U);
    write_be_u32(memo_bytes, 5U, 0x00FFFFFFU);

    {
        std::ofstream table_output(table_path, std::ios::binary);
        table_output.write(reinterpret_cast<const char*>(table_bytes.data()),
                           static_cast<std::streamsize>(table_bytes.size()));
    }
    {
        std::ofstream memo_output(memo_path, std::ios::binary);
        memo_output.write(reinterpret_cast<const char*>(memo_bytes.data()),
                          static_cast<std::streamsize>(memo_bytes.size()));
    }

    const auto original_table = read_binary_file(table_path);
    const auto original_memo = read_binary_file(memo_path);

    const auto pack_result = copperfin::vfp::pack_dbf_memo_file(table_path.string());
    expect(!pack_result.ok, "#3687: PACK MEMO should fail when a referenced memo payload cannot be recovered");
    expect(pack_result.error == "Unable to recover memo payload from the sidecar.",
           "#3687: unreadable memo repacks should surface the localized payload-recovery error");
    expect(read_binary_file(table_path) == original_table,
           "#3687: failed memo repacks should leave the original table bytes intact");
    expect(read_binary_file(memo_path) == original_memo,
           "#3687: failed memo repacks should leave the original memo sidecar intact");

    fs::remove_all(temp_dir, ignored);
}

void test_additive_memo_replace_preserves_raw_payload_and_fails_closed() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_additive_memo_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "additive.dbf";
    const fs::path memo_path = temp_dir / "additive.fpt";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "BODY", .type = 'M', .length = 4U}
    };
    const auto create_result =
        copperfin::vfp::create_dbf_table_file(table_path.string(), fields, {{"seed"}});
    expect(create_result.ok, "#3927: additive raw-memo fixture should be created");

    auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
    expect(parse_result.ok && parse_result.table.records.size() == 1U,
           "#3927: additive raw-memo fixture should expose its source block");
    if (!parse_result.ok || parse_result.table.records.size() != 1U) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    const std::vector<std::uint8_t> original_payload{0x00U, 0x01U, 'A', 0xFFU, ' '};
    expect(replace_memo_block_payload(
               memo_path,
               parse_result.table.records[0U].values[0U].memo_block_number,
               original_payload),
           "#3927: binary source memo payload should be installed");

    const std::string suffix("\0Z ", 3U);
    const auto additive_result = copperfin::vfp::replace_record_field_value_additive(
        table_path.string(), 0U, "BODY", suffix);
    expect(additive_result.ok, "#3927: additive memo replacement should append raw suffix bytes");

    parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
    expect(parse_result.ok && parse_result.table.records.size() == 1U,
           "#3927: additive memo replacement should leave the table readable");
    if (parse_result.ok && parse_result.table.records.size() == 1U) {
        std::vector<std::uint8_t> expected_payload = original_payload;
        expected_payload.insert(expected_payload.end(), suffix.begin(), suffix.end());
        expect(copperfin::vfp::read_memo_block_raw(
                   memo_path.string(),
                   parse_result.table.records[0U].values[0U].memo_block_number) == expected_payload,
               "#3927: additive memo replacement should preserve existing binary bytes exactly");
    }

    std::vector<std::uint8_t> corrupt_memo_bytes = read_binary_file(memo_path);
    corrupt_memo_bytes.resize(512U);
    expect(write_binary_file(memo_path, corrupt_memo_bytes),
           "#3927: unreadable additive memo fixture should be truncated");
    const std::vector<std::uint8_t> pre_operation_table_bytes = read_binary_file(table_path);
    const std::vector<std::uint8_t> pre_operation_memo_bytes = read_binary_file(memo_path);
    const auto failed_additive_result = copperfin::vfp::replace_record_field_value_additive(
        table_path.string(), 0U, "BODY", "unsafe");
    expect(!failed_additive_result.ok &&
               failed_additive_result.error == "Unable to recover memo payload from the sidecar.",
           "#3927: additive memo replacement should fail closed on an unreadable source block");
    expect(read_binary_file(table_path) == pre_operation_table_bytes &&
               read_binary_file(memo_path) == pre_operation_memo_bytes,
           "#3927: failed additive memo replacement should leave DBF/FPT bytes untouched");

    fs::remove_all(temp_dir, ignored);
}

}  // namespace copperfin::test_dbf_table
