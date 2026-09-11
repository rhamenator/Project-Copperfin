// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/vfp/dbf_header.h"

#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::vfp {

struct DbfFieldDescriptor {
    std::string name;
    char type = '\0';
    std::uint32_t offset = 0;
    std::uint8_t length = 0;
    std::uint8_t decimal_count = 0;
};

struct DbfRecordValue {
    std::string field_name;
    char field_type = '\0';
    bool is_null = false;
    std::string display_value;
    std::uint32_t memo_block_number = 0;
};

struct DbfRecord {
    std::size_t record_index = 0;
    bool deleted = false;
    std::vector<DbfRecordValue> values;
};

struct DbfTable {
    DbfHeader header{};
    std::vector<DbfFieldDescriptor> fields;
    std::vector<DbfRecord> records;
};

struct DbfTableParseResult {
    bool ok = false;
    DbfTable table{};
    std::string error;
};

struct DbfFieldsOnlyParseResult {
    bool ok = false;
    DbfHeader header{};
    std::vector<DbfFieldDescriptor> fields;
    std::string error;
};

// #5546 review (Codex, Copilot -- duplicate finding): parse_dbf_table_from_file()
// unconditionally reads the whole file and scans every record for memo
// references (collect_referenced_memo_blocks()) even when max_records is
// 0 -- so a caller that only wants field names/types (e.g. relation
// inference) still pays a cost proportional to a potentially multi-
// gigabyte table/memo pair. This reads only the header and field-
// descriptor block, bounding the file read to header.header_length bytes
// -- never the record data or memo sidecar. Shares the same generation-
// specific layout logic (dbf_read_layout()) parse_dbf_table_from_file()
// itself uses, so the two cannot silently disagree on field offsets/
// types for the same file.
[[nodiscard]] DbfFieldsOnlyParseResult parse_dbf_fields_from_file(const std::string& path);

struct DbfWriteResult {
    bool ok = false;
    std::string error;
    std::size_t record_count = 0;
};

// Returns whether a type and width can be admitted by the ordinary DBF writer.
// This excludes the writer's internal raw-byte preservation override.
[[nodiscard]] bool is_dbf_table_field_storage_layout_writable(char type, std::uint8_t length);

DbfTableParseResult parse_dbf_table_from_file(
    const std::string& path,
    std::size_t max_records = 10U,
    const std::string& memo_sidecar_path = {});
DbfWriteResult create_dbf_table_file(
    const std::string& path,
    const std::vector<DbfFieldDescriptor>& fields,
    const std::vector<std::vector<std::string>>& records);
// #5485: writes a dBASE III-compatible table (C/N/L/D fields, no memo/index
// sidecar). Fails closed if `path` already exists rather than overwriting.
DbfWriteResult create_dbase_iii_table_file(
    const std::string& path,
    const std::vector<DbfFieldDescriptor>& fields,
    const std::vector<std::vector<std::string>>& records);
DbfWriteResult add_dbf_table_field(const std::string& path, const DbfFieldDescriptor& field);
DbfWriteResult drop_dbf_table_field(const std::string& path, const std::string& field_name);
DbfWriteResult alter_dbf_table_field(const std::string& path, const DbfFieldDescriptor& field);
DbfWriteResult append_blank_record_to_file(const std::string& path);
DbfWriteResult replace_record_field_value(
    const std::string& path,
    std::size_t record_index,
    const std::string& field_name,
    const std::string& value,
    bool allow_truncation = false);
DbfWriteResult replace_record_field_value_additive(
    const std::string& path,
    std::size_t record_index,
    const std::string& field_name,
    const std::string& value,
    bool allow_truncation = false);
// Whole-file-read/mutate/atomically-rewrite variants of the two entry
// points above, bypassing the targeted-I/O fast paths those use by
// default (see #5509). Exists for callers that specifically need
// write_binary_file()'s staged temp-file-then-rename durability guarantee
// for a non-memo field write -- today, only the verified/staged buffered-
// commit admission path (prg_engine_records.inl), which stages an entire
// snapshot for a TABLEUPDATE() flush and is tested against injected
// partial-write failures at that granularity. Prefer the fast-path-first
// entry points above for ordinary REPLACE/APPEND BLANK; reach for these
// only when that specific whole-file atomicity is the actual requirement.
DbfWriteResult append_blank_record_to_file_full_rewrite(const std::string& path);
DbfWriteResult replace_record_field_value_full_rewrite(
    const std::string& path,
    std::size_t record_index,
    const std::string& field_name,
    const std::string& value,
    bool allow_truncation = false);
DbfWriteResult replace_record_field_value_additive_full_rewrite(
    const std::string& path,
    std::size_t record_index,
    const std::string& field_name,
    const std::string& value,
    bool allow_truncation = false);
DbfWriteResult set_record_deleted_flag(
    const std::string& path,
    std::size_t record_index,
    bool deleted);
DbfWriteResult truncate_dbf_table_file(const std::string& path, std::size_t record_count);
DbfWriteResult pack_dbf_table_file(const std::string& path);
DbfWriteResult pack_dbf_memo_file(const std::string& path);
DbfWriteResult zap_dbf_table_file(const std::string& path);

// #5534 (index-rebuild half): sets the DBF header's table_flags bit
// 0x01 (DbfHeader::has_production_index()) on an existing table file.
// Real VFP9 was found this session to refuse SET ORDER TO TAG ... OF
// <cdx file> against a table lacking this bit, even when the CDX file
// is named explicitly -- see docs/77-cdx-index-write-format-notes.md.
// This codebase's own DBF writers do not currently set this bit at
// table-creation time, so any code that later builds an index for an
// existing table needs to set it via this function.
DbfWriteResult mark_dbf_table_has_production_index(const std::string& path);

// Updates the three DBF header last-update bytes using the local calendar date.
// Returns false when the supplied buffer is too short or local time is unavailable.
bool stamp_dbf_last_update_date(std::vector<std::uint8_t>& bytes);

// Returns the raw bytes of a memo block from a .fpt or .dct sidecar file.
// block_number is the 4-byte LE integer stored in an 'M'-type DBF field.
// Returns an empty vector on failure and also for valid zero-length memo payloads.
[[nodiscard]] std::vector<std::uint8_t> read_memo_block_raw(
    const std::string& sidecar_path,
    std::uint32_t block_number);

// Returns the physical memo-field block size from a table's resolved memo
// sidecar, or no value when the table has no memo field or the sidecar cannot
// be resolved safely.
[[nodiscard]] std::optional<std::uint16_t> read_memo_field_block_size(
    const std::string& table_path);

}  // namespace copperfin::vfp
