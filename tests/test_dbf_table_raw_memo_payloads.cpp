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

namespace copperfin::test_dbf_table {
namespace {

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
    std::uint8_t field_length) {
    write_ascii(bytes, offset, name);
    bytes[offset + 11U] = static_cast<std::uint8_t>(type);
    write_le_u32(bytes, offset + 12U, field_offset);
    bytes[offset + 16U] = field_length;
}

}  // namespace

void test_memo_payload_that_decodes_empty_stays_empty() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_empty_memo_decode_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "empty_decode.dbf";
    const fs::path memo_path = temp_dir / "empty_decode.fpt";

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

    std::vector<std::uint8_t> memo_bytes(1024U, 0U);
    write_be_u32(memo_bytes, 0U, 2U);
    write_be_u16(memo_bytes, 6U, 512U);
    write_be_u32(memo_bytes, 516U, 4U);
    write_ascii(memo_bytes, 520U, "    ");

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

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "#3687: whitespace-only memo payloads should still parse");
    if (parse_result.ok && parse_result.table.records.size() == 1U && parse_result.table.records[0].values.size() == 1U) {
        expect(parse_result.table.records[0].values[0].display_value.empty(),
               "#3687: memo payloads that decode to empty should stay empty instead of surfacing a placeholder");
        expect(parse_result.table.records[0].values[0].memo_block_number == 1U,
               "#3687: empty-decoding memo payloads should retain structured block provenance");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_pack_memo_preserves_payloads_that_decode_empty() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_pack_empty_memo_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "pack_empty.dbf";
    const fs::path memo_path = temp_dir / "pack_empty.fpt";

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

    std::vector<std::uint8_t> memo_bytes(1024U, 0U);
    write_be_u32(memo_bytes, 0U, 2U);
    write_be_u16(memo_bytes, 6U, 512U);
    write_be_u32(memo_bytes, 516U, 4U);
    write_ascii(memo_bytes, 520U, "    ");

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

    const auto pack_result = copperfin::vfp::pack_dbf_memo_file(table_path.string());
    expect(pack_result.ok, "#3687: PACK MEMO should preserve memo payloads that decode to empty");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "#3687: packed tables with empty-decoding memo payloads should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 1U && parse_result.table.records[0].values.size() == 1U) {
        expect(parse_result.table.records[0].values[0].display_value.empty(),
               "#3687: PACK MEMO should not replace empty-decoding payloads with placeholder text");
        const auto raw_payload = copperfin::vfp::read_memo_block_raw(
            memo_path.string(),
            parse_result.table.records[0].values[0].memo_block_number);
        expect(raw_payload == std::vector<std::uint8_t>{' ', ' ', ' ', ' '},
               "#3687: PACK MEMO should preserve the original raw memo bytes when decoded text is empty");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_pack_memo_preserves_binary_picture_payloads() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_table_pack_picture_payload_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "pack_picture.dbf";
    const fs::path memo_path = temp_dir / "pack_picture.fpt";

    std::vector<std::uint8_t> table_bytes(65U + 5U + 1U, 0U);
    table_bytes[0] = 0x30U;
    write_le_u32(table_bytes, 4U, 1U);
    write_le_u16(table_bytes, 8U, 65U);
    write_le_u16(table_bytes, 10U, 5U);
    write_field_descriptor(table_bytes, 32U, "PICTURE", 'P', 1U, 4U);
    table_bytes[64U] = 0x0DU;
    table_bytes[65U] = 0x20U;
    write_le_u32(table_bytes, 66U, 1U);
    table_bytes.back() = 0x1AU;

    const std::vector<std::uint8_t> original_payload{0x00U, 0x01U, 0x41U, 0xFFU, 0x09U, 0x42U};
    std::vector<std::uint8_t> memo_bytes(1024U, 0U);
    write_be_u32(memo_bytes, 0U, 2U);
    write_be_u16(memo_bytes, 6U, 512U);
    write_be_u32(memo_bytes, 516U, static_cast<std::uint32_t>(original_payload.size()));
    std::copy(
        original_payload.begin(),
        original_payload.end(),
        memo_bytes.begin() + static_cast<std::ptrdiff_t>(520U));

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

    const auto pack_result = copperfin::vfp::pack_dbf_memo_file(table_path.string());
    expect(pack_result.ok, "#3687: PACK MEMO should preserve picture payloads without rebuilding them from display text");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "#3687: packed picture-memo tables should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 1U && parse_result.table.records[0].values.size() == 1U) {
        const auto raw_payload = copperfin::vfp::read_memo_block_raw(
            memo_path.string(),
            parse_result.table.records[0].values[0].memo_block_number);
        expect(raw_payload == original_payload,
               "#3687: PACK MEMO should preserve raw picture payload bytes exactly");
    }

    fs::remove_all(temp_dir, ignored);
}

}  // namespace copperfin::test_dbf_table
