// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace copperfin::test_dbf_table {

extern int failures;

void expect(bool condition, const std::string& message);

void write_le_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value);
void write_le_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value);
void write_be_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value);
void write_be_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value);
void write_ascii(std::vector<std::uint8_t>& bytes, std::size_t offset, const std::string& value);
void write_field_descriptor(
    std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    const std::string& name,
    char type,
    std::uint32_t field_offset,
    std::uint8_t field_length,
    std::uint8_t decimal_count = 0U);
std::vector<std::uint8_t> read_binary_file(const std::filesystem::path& path);
bool write_binary_file(const std::filesystem::path& path, const std::vector<std::uint8_t>& bytes);
std::uint16_t read_be_u16(const std::vector<std::uint8_t>& bytes, std::size_t offset);
bool replace_memo_block_payload(
    const std::filesystem::path& memo_path,
    std::uint32_t block_number,
    const std::vector<std::uint8_t>& payload);

void test_memo_field_create_replace_and_append_round_trip();
void test_general_and_picture_memo_fields_round_trip();
void test_memo_payload_that_decodes_empty_stays_empty();
void test_pack_memo_preserves_payloads_that_decode_empty();
void test_pack_memo_preserves_binary_picture_payloads();
void test_pack_memo_fails_when_referenced_payload_cannot_be_recovered();
void test_additive_memo_replace_preserves_raw_payload_and_fails_closed();
void test_schema_rewrites_preserve_raw_memo_and_unaffected_field_bytes();

}  // namespace copperfin::test_dbf_table
