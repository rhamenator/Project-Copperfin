// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/dbf_import.h"
#include "copperfin/platform/path.h"
#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/sidecar_path.h"

#include "copperfin/localization/localization.h"

#include <filesystem>
#include <limits>
#include <mutex>
#include <optional>
#include <string_view>

namespace copperfin::vfp {

namespace {

localization::LocalizedCatalog dbf_import_catalog() {
    struct CatalogCache {
        std::filesystem::path locale_root;
        std::string locale;
        localization::LocalizedCatalog catalog;
    };

    static std::mutex cache_mutex;
    static CatalogCache cache{
        {},
        {},
        localization::load_catalogs(
            localization::resolve_catalog_root(),
            localization::default_locale)};
    const std::filesystem::path locale_root = localization::resolve_catalog_root();
    const std::string locale = localization::select_locale();
    std::lock_guard<std::mutex> lock(cache_mutex);
    if (cache.locale_root != locale_root || cache.locale != locale) {
        cache.locale_root = locale_root;
        cache.locale = locale;
        cache.catalog = localization::load_catalogs(locale_root, locale);
    }
    return cache.catalog;
}

std::string dbf_import_text(std::string_view key) {
    return dbf_import_catalog().translate(key);
}

std::string dbf_import_text(
    std::string_view key,
    const localization::PlaceholderMap& placeholders) {
    return dbf_import_catalog().translate(key, placeholders);
}

// Maps one dBASE-family source field to its VFP-native target type and
// width, per RQ-CF-MIGRATION-003's written mapping table. Returns no value
// for a source type this slice does not support.
std::optional<DbfFieldDescriptor> map_dbase_field_to_vfp_native(const DbfFieldDescriptor& source) {
    switch (source.type) {
        case 'C':
        case 'N':
        case 'L':
        case 'D':
            // Direct: identical on-disk text encoding and semantics on
            // both sides (VFP's DBF reader/writer already treats these
            // dBASE letters the same way it treats its own).
            return DbfFieldDescriptor{
                .name = source.name,
                .type = source.type,
                .offset = 0U,
                .length = source.length,
                .decimal_count = source.decimal_count
            };
        case 'F':
            // dBASE Float -> VFP Numeric: both sides already decode/encode
            // 'F' and 'N' identically (see write_field_bytes()'s shared
            // 'N'/'F' case), and VFP itself treats Float as a legacy
            // synonym for Numeric, so this maps to the canonical 'N'
            // rather than perpetuating the separate letter.
            return DbfFieldDescriptor{
                .name = source.name,
                .type = 'N',
                .offset = 0U,
                .length = source.length,
                .decimal_count = source.decimal_count
            };
        case 'M':
            // dBASE III/IV/Level 7 text memo -> VFP memo. Real content is
            // filled in by a second pass (see import_dbase_table_to_vfp_native)
            // since the base writer only accepts a blank memo pointer at
            // table-creation time.
            return DbfFieldDescriptor{
                .name = source.name,
                .type = 'M',
                .offset = 0U,
                .length = 4U,
                .decimal_count = 0U
            };
        case 'I':
        case '+':
            // dBASE Level 7 Long/Autoincrement -> VFP Integer. The
            // autoincrement *behavior* is lost (the target is a plain,
            // static integer field) -- the value itself is preserved.
            return DbfFieldDescriptor{
                .name = source.name,
                .type = 'I',
                .offset = 0U,
                .length = 4U,
                .decimal_count = 0U
            };
        case 'O':
            // dBASE Level 7 IEEE double -> VFP Double. Both sides already
            // decode/encode the same little-endian 8-byte IEEE-754 layout
            // through a plain decimal string, so the value round-trips
            // exactly.
            return DbfFieldDescriptor{
                .name = source.name,
                .type = 'B',
                .offset = 0U,
                .length = 8U,
                .decimal_count = 0U
            };
        default:
            // dBASE B (binary DBT payload), @ (Julian-day/millisecond
            // timestamp -- the reader does not yet convert this to a real
            // calendar value, so there is nothing meaningful to write),
            // and any other letter: not supported by this first slice.
            return std::nullopt;
    }
}

// The dBASE-family reader (dbf_table.cpp) exposes an unresolved memo
// payload (missing/truncated/unreadable DBT block) as the diagnostic text
// "<memo block N>" rather than failing the whole table parse -- see the
// 'M'/'G'/'P' cases in that file's field-decoding switch. Copying that
// diagnostic string into the destination as if it were real memo content
// would silently corrupt the imported data while still reporting success,
// so it must be detected and rejected here instead.
bool looks_like_unresolved_memo_placeholder(const DbfRecordValue& value) {
    if (value.memo_block_number == 0U) {
        return false;
    }
    return value.display_value == ("<memo block " + std::to_string(value.memo_block_number) + ">");
}

void remove_destination_artifacts(const std::string& destination_path) {
    std::error_code ignored;
    std::filesystem::remove(platform::path_from_utf8_string(destination_path), ignored);
    const SidecarPathResolution memo_resolution =
        resolve_vfp_memo_sidecar_path(platform::path_from_utf8_string(destination_path));
    if (memo_resolution.path.has_value()) {
        std::filesystem::remove(*memo_resolution.path, ignored);
    }
}

}  // namespace

DbfImportResult import_dbase_table_to_vfp_native(
    const std::string& source_path,
    const std::string& destination_path,
    const std::string& source_memo_sidecar_path) {
    const DbfTableParseResult source = parse_dbf_table_from_file(
        source_path,
        std::numeric_limits<std::size_t>::max(),
        source_memo_sidecar_path);
    if (!source.ok) {
        return {.ok = false, .error = source.error};
    }
    if (source.table.header.format_family() != DbfFormatFamily::dbase) {
        return {.ok = false, .error = dbf_import_text("Vfp.DbfImport.Error.UnsupportedSourceFamily")};
    }
    if (source.table.header.code_page_mark != 0U) {
        // code_page_mark 0 decodes/encodes as UTF-8 on both the read and
        // write side (see dbf_text_encoding.cpp), so a code-page-0 source
        // round-trips through this import with identical byte widths. A
        // real single-byte code page (e.g. CP1252) does not: the reader
        // widens it to (possibly multi-byte) UTF-8, but this slice copies
        // the source field's declared byte width unchanged and the
        // destination table is always created as code-page-0, so a
        // non-ASCII character in a tightly-sized field can silently fail
        // to fit. Rather than guess a safe worst-case width expansion,
        // this first slice only supports code-page-0 sources.
        return {.ok = false, .error = dbf_import_text("Vfp.DbfImport.Error.UnsupportedCodePage")};
    }

    std::error_code exists_error;
    const bool destination_exists = std::filesystem::exists(
        platform::path_from_utf8_string(destination_path),
        exists_error);
    if (!exists_error && destination_exists) {
        return {.ok = false, .error = dbf_import_text("Vfp.DbfImport.Error.DestinationExists")};
    }

    std::vector<DbfFieldDescriptor> target_fields;
    std::vector<DbfImportFieldMapping> field_mappings;
    std::vector<std::size_t> memo_field_indexes;
    target_fields.reserve(source.table.fields.size());
    field_mappings.reserve(source.table.fields.size());
    for (const DbfFieldDescriptor& source_field : source.table.fields) {
        const auto mapped = map_dbase_field_to_vfp_native(source_field);
        if (!mapped.has_value()) {
            return {
                .ok = false,
                .error = dbf_import_text(
                    "Vfp.DbfImport.Error.UnsupportedFieldType",
                    {
                        {"fieldName", source_field.name},
                        {"fieldType", std::string(1U, source_field.type)},
                    })
            };
        }
        if (mapped->type == 'M') {
            memo_field_indexes.push_back(target_fields.size());
        }
        field_mappings.push_back({
            .field_name = source_field.name,
            .source_type = source_field.type,
            .target_type = mapped->type
        });
        target_fields.push_back(*mapped);
    }

    if (!memo_field_indexes.empty()) {
        // create_dbf_table_file() resolves and writes to whatever memo
        // sidecar path already exists alongside destination_path (a
        // case-insensitive companion-discovery lookup, matching how the
        // rest of this table-creation family behaves), not only a path it
        // creates itself. If destination_path itself is absent but a
        // same-base .fpt companion already exists from something else,
        // creating a table with memo fields would overwrite that
        // unrelated file. Only check when the target schema actually has
        // a memo field, matching the writer's own conditional behavior.
        const SidecarPathResolution memo_conflict =
            resolve_vfp_memo_sidecar_path(platform::path_from_utf8_string(destination_path));
        if (memo_conflict.path.has_value()) {
            return {.ok = false, .error = dbf_import_text("Vfp.DbfImport.Error.DestinationExists")};
        }
    }

    for (const DbfRecord& record : source.table.records) {
        for (const std::size_t field_index : memo_field_indexes) {
            if (field_index >= record.values.size()) {
                continue;
            }
            if (looks_like_unresolved_memo_placeholder(record.values[field_index])) {
                return {
                    .ok = false,
                    .error = dbf_import_text(
                        "Vfp.DbfImport.Error.UnresolvedMemoPayload",
                        {
                            {"fieldName", target_fields[field_index].name},
                            {"recordNumber", std::to_string(record.record_index + 1U)},
                        })
                };
            }
        }
    }

    // First pass: create the table structure with every non-memo value
    // copied directly and every memo field left blank. The public table
    // writer only accepts a blank memo pointer at creation time; real memo
    // content is filled in below via the existing, already-tested REPLACE
    // path, which knows how to append memo blocks and resolve pointers.
    std::vector<std::vector<std::string>> target_records;
    target_records.reserve(source.table.records.size());
    for (const DbfRecord& record : source.table.records) {
        std::vector<std::string> row;
        row.reserve(record.values.size());
        for (std::size_t field_index = 0U; field_index < record.values.size(); ++field_index) {
            const DbfRecordValue& value = record.values[field_index];
            if (target_fields[field_index].type == 'M') {
                row.emplace_back();
                continue;
            }
            row.push_back(value.is_null ? std::string("NULL") : value.display_value);
        }
        target_records.push_back(std::move(row));
    }

    const DbfWriteResult create_result = create_dbf_table_file(destination_path, target_fields, target_records);
    if (!create_result.ok) {
        return {.ok = false, .error = create_result.error};
    }

    // Second pass: fill in real memo content record by record.
    for (std::size_t record_index = 0U; record_index < source.table.records.size(); ++record_index) {
        const DbfRecord& record = source.table.records[record_index];
        for (const std::size_t field_index : memo_field_indexes) {
            if (field_index >= record.values.size()) {
                continue;
            }
            const DbfRecordValue& value = record.values[field_index];
            if (value.is_null || value.display_value.empty()) {
                continue;
            }
            const DbfWriteResult memo_result = replace_record_field_value(
                destination_path,
                record_index,
                target_fields[field_index].name,
                value.display_value);
            if (!memo_result.ok) {
                // Roll back rather than leaving a partially-imported
                // destination behind: a retry would otherwise immediately
                // fail on the destination-exists check above despite the
                // import having failed.
                remove_destination_artifacts(destination_path);
                return {.ok = false, .error = memo_result.error};
            }
        }
    }

    return {
        .ok = true,
        .error = {},
        .record_count = source.table.records.size(),
        .field_mappings = std::move(field_mappings)
    };
}

}  // namespace copperfin::vfp
