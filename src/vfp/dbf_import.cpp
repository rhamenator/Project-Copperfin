// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/dbf_import.h"
#include "copperfin/platform/path.h"
#include "copperfin/vfp/dbf_table.h"

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
        case 'F':
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
