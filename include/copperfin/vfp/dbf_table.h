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
    // #6047: mirrors the on-disk field-descriptor flags byte's 0x02 bit,
    // recovered from real VFP9 behavior (docs/80-dbf-nullflags-field-format-notes.md).
    // A nullable field's per-record NULL state lives in the hidden
    // "_NullFlags" bitmap field, not in this struct or in ordinary field
    // storage; this flag only records that the field *can* hold NULL.
    bool nullable = false;
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
    // Runtime-only identity for an in-memory row while user code runs against
    // it (e.g. a provisional APPEND FROM row, #6551). Survives REPLACE, PACK,
    // and record_index renumbering; zero when unused. Never persisted.
    std::uint64_t transient_row_token = 0;
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

// Optional import-time proof of the bytes assembled by the DBF writer before
// they enter the staging filesystem. The importer compares these digests
// against its retained read handles, closing the write-close/reopen gap.
struct DbfGeneratedDigests {
    std::string table_sha256;
    std::string memo_sha256;
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
    const std::vector<std::vector<std::string>>& records,
    DbfGeneratedDigests* generated_digests = nullptr);
// #5567 review (chatgpt-codex-connector/Copilot, P1): a caller that already
// knows which of its records are deleted at creation time (e.g. a DBF-to-
// DBF import preserving the source's own deletion flags) should be able to
// set every deletion marker in this same single-pass write, rather than
// calling set_record_deleted_flag() once per deleted record afterward --
// that API re-reads and rewrites the *entire* file on every call, making a
// per-record loop over d deleted rows in an S-byte destination cost
// Theta(d*S) I/O instead of O(S). `deleted_flags`, when non-null, must have
// exactly `records.size()` entries.
DbfWriteResult create_dbf_table_file_with_deleted_flags(
    const std::string& path,
    const std::vector<DbfFieldDescriptor>& fields,
    const std::vector<std::vector<std::string>>& records,
    const std::vector<bool>& deleted_flags,
    DbfGeneratedDigests* generated_digests = nullptr);
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
// #6047: `is_null`, when true, blanks the field's storage exactly like an
// empty value regardless of type and sets its bit in the table's
// _NullFlags bitmap (a no-op if the table has none, or if `field_name`
// isn't declared nullable -- see apply_null_flag_bit(), dbf_table.cpp).
// When false, clears that bit if present and writes `value` normally.
DbfWriteResult replace_record_field_value(
    const std::string& path,
    std::size_t record_index,
    const std::string& field_name,
    const std::string& value,
    bool allow_truncation = false,
    bool is_null = false);
DbfWriteResult replace_record_field_value_additive(
    const std::string& path,
    std::size_t record_index,
    const std::string& field_name,
    const std::string& value,
    bool allow_truncation = false,
    bool is_null = false);
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
    bool allow_truncation = false,
    bool is_null = false);
DbfWriteResult replace_record_field_value_additive_full_rewrite(
    const std::string& path,
    std::size_t record_index,
    const std::string& field_name,
    const std::string& value,
    bool allow_truncation = false,
    bool is_null = false);
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
