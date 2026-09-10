// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/access_msysobjects.h"
#include "copperfin/vfp/access_table_definition.h"
#include "copperfin/platform/path.h"
#include "access_bytes_internal.h"

#include "copperfin/localization/localization.h"

#include <filesystem>
#include <fstream>
#include <mutex>
#include <optional>
#include <string_view>
#include <system_error>

namespace copperfin::vfp {

namespace {

using access_bytes_internal::read_le_u16;
using access_bytes_internal::read_le_u32;
using access_bytes_internal::sanitize_as_utf8;

localization::LocalizedCatalog access_msysobjects_catalog() {
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

std::string access_msysobjects_text(
    std::string_view key,
    const localization::PlaceholderMap& placeholders = {}) {
    return access_msysobjects_catalog().translate(key, placeholders);
}

// MSysObjects is always rooted at TDEF page 2 -- a well-corroborated
// constant every source on this format agrees on (it is the very first
// system table page in every real fixture cross-checked during this
// slice's development, and mdbtools itself hardcodes it the same way).
constexpr std::uint32_t msysobjects_tdef_page = 2U;

// A single declared column from MSysObjects's own TDEF, reduced to only
// what row-decoding needs (byte width, fixed-vs-variable, and which
// physical slot a variable column occupies among only the variable
// columns, in TDEF declaration order).
struct RowColumnLayout {
    AccessColumnType type = AccessColumnType::unknown;
    bool fixed_length = false;
    std::uint16_t length = 0;
    std::uint16_t offset_f = 0;  // meaningful only when fixed_length
};

struct DecodedValue {
    bool is_null = true;
    std::vector<std::uint8_t> bytes;
};

// One physical row's worth of already-boundary-resolved column spans,
// built once per row and then read from by field-specific accessors
// (read_fixed_integer, read_variable_text, ...) below.
struct DecodedRow {
    bool ok = false;
    std::string error;
    std::vector<DecodedValue> columns;  // parallel to the TDEF's own column order
};

// Bounds-checked, backward-reading cursor over a single row's bytes.
// Mirrors access_table_definition.cpp's PageCursor discipline (fail
// closed rather than read out of bounds), but this decoder reads several
// of a row's trailing sections (null mask, var_len, var_table, eod) from
// the END of the row backward, since their sizes are only known once
// preceding fields (num_cols, var_len) have themselves been read -- see
// docs/72's worked byte-level walkthrough for why this ordering is
// unavoidable rather than a stylistic choice.
[[nodiscard]] bool row_would_overrun(std::size_t row_size, std::size_t start, std::size_t count) {
    return count > row_size || start > row_size - count;
}

DecodedRow decode_row(
    const std::vector<std::uint8_t>& row_bytes,
    const std::vector<RowColumnLayout>& tdef_columns,
    bool is_jet3) {
    DecodedRow result;
    const std::size_t row_size = row_bytes.size();
    const std::size_t num_cols_field_width = is_jet3 ? 1U : 2U;
    if (row_would_overrun(row_size, 0U, num_cols_field_width)) {
        result.error = access_msysobjects_text("Vfp.AccessMSysObjects.Error.RowTooShort");
        return result;
    }
    const std::uint32_t num_cols_in_row =
        is_jet3 ? row_bytes[0] : read_le_u16(row_bytes, 0U);
    if (num_cols_in_row != tdef_columns.size()) {
        // A row whose own column count does not match the TDEF's current
        // column count would mean the table was ALTERed after this row
        // was written -- MSysObjects is an Access-internal system table
        // never subject to user ALTER TABLE, so real fixtures never
        // exercise this path. Failing closed here rather than attempting
        // partial reconstruction matches this codebase's established
        // discipline for unverified shapes (e.g. multi-page TDEF
        // rejection in access_table_definition.cpp).
        result.error = access_msysobjects_text(
            "Vfp.AccessMSysObjects.Error.ColumnCountMismatch",
            {{"rowColumns", std::to_string(num_cols_in_row)},
             {"tdefColumns", std::to_string(tdef_columns.size())}});
        return result;
    }

    const std::size_t null_mask_size = (num_cols_in_row + 7U) / 8U;
    if (row_would_overrun(row_size, row_size - null_mask_size, null_mask_size)) {
        result.error = access_msysobjects_text("Vfp.AccessMSysObjects.Error.RowTooShort");
        return result;
    }
    const std::size_t null_mask_start = row_size - null_mask_size;

    const std::size_t var_len_field_width = is_jet3 ? 1U : 2U;
    if (row_would_overrun(row_size, 0U, var_len_field_width) ||
        null_mask_start < var_len_field_width) {
        result.error = access_msysobjects_text("Vfp.AccessMSysObjects.Error.RowTooShort");
        return result;
    }
    const std::size_t var_len_start = null_mask_start - var_len_field_width;
    const std::uint32_t var_len =
        is_jet3 ? row_bytes[var_len_start] : read_le_u16(row_bytes, var_len_start);

    std::size_t declared_var_cols = 0U;
    for (const RowColumnLayout& column : tdef_columns) {
        if (!column.fixed_length) {
            ++declared_var_cols;
        }
    }
    if (var_len != declared_var_cols) {
        result.error = access_msysobjects_text(
            "Vfp.AccessMSysObjects.Error.ColumnCountMismatch",
            {{"rowColumns", std::to_string(var_len)},
             {"tdefColumns", std::to_string(declared_var_cols)}});
        return result;
    }

    // Jet3 offsets are 1-byte fields (max 256), requiring a jump table
    // for rows whose total size reaches that limit -- a sub-path this
    // slice deliberately does not implement (see access_msysobjects.h's
    // own documented non-goal; no real fixture available during this
    // slice's development ever produced a row this large). Jet4 offsets
    // are always 2 bytes and never need a jump table.
    if (is_jet3 && row_size >= 256U) {
        result.error = access_msysobjects_text("Vfp.AccessMSysObjects.Error.JumpTableUnsupported");
        return result;
    }
    const std::size_t entry_width = is_jet3 ? 1U : 2U;

    // The eod field plus var_len variable-column-offset entries sit,
    // contiguously and in that order, immediately before var_len_start.
    // eod occupies the lowest address in this block; var_table entries
    // follow in REVERSE declaration order (the last declared variable
    // column's offset is stored first / at the lowest address, the
    // first declared variable column's offset last / immediately
    // adjacent to var_len) -- see docs/72's worked byte-level example
    // for how this was derived and cross-checked against real fixtures.
    const std::size_t eod_and_table_size = entry_width * (var_len + 1U);
    if (var_len_start < eod_and_table_size) {
        result.error = access_msysobjects_text("Vfp.AccessMSysObjects.Error.RowTooShort");
        return result;
    }
    const std::size_t block_start = var_len_start - eod_and_table_size;
    const std::uint32_t eod =
        is_jet3 ? row_bytes[block_start] : read_le_u16(row_bytes, block_start);
    if (eod > row_size) {
        result.error = access_msysobjects_text("Vfp.AccessMSysObjects.Error.RowStructureInvalid");
        return result;
    }

    // Reverse-order table entries -> ascending column-declaration-order
    // boundaries, with eod appended as the final upper-bound sentinel.
    std::vector<std::uint32_t> boundaries(var_len + 1U);
    for (std::size_t slot = 0U; slot < var_len; ++slot) {
        const std::size_t entry_position = block_start + entry_width * (1U + slot);
        const std::uint32_t raw_value =
            is_jet3 ? row_bytes[entry_position] : read_le_u16(row_bytes, entry_position);
        // slot 0 (lowest address) is the LAST declared variable column;
        // slot (var_len - 1) (highest address) is the FIRST declared
        // variable column.
        boundaries[var_len - 1U - slot] = raw_value;
    }
    boundaries[var_len] = eod;
    for (std::size_t index = 0U; index < var_len; ++index) {
        if (boundaries[index] > boundaries[index + 1U] || boundaries[index + 1U] > row_size) {
            result.error = access_msysobjects_text("Vfp.AccessMSysObjects.Error.RowStructureInvalid");
            return result;
        }
    }

    result.columns.resize(tdef_columns.size());
    std::size_t var_slot = 0U;
    for (std::size_t index = 0U; index < tdef_columns.size(); ++index) {
        const RowColumnLayout& column = tdef_columns[index];
        const bool not_null = ((row_bytes[null_mask_start + index / 8U] >> (index % 8U)) & 0x01U) != 0U;
        DecodedValue& value = result.columns[index];
        if (column.fixed_length) {
            const std::size_t start = num_cols_field_width + column.offset_f;
            if (row_would_overrun(row_size, start, column.length)) {
                result.error = access_msysobjects_text("Vfp.AccessMSysObjects.Error.RowStructureInvalid");
                return result;
            }
            value.is_null = !not_null;
            value.bytes.assign(row_bytes.begin() + static_cast<std::ptrdiff_t>(start),
                                row_bytes.begin() + static_cast<std::ptrdiff_t>(start + column.length));
        } else {
            const std::uint32_t span_start = boundaries[var_slot];
            const std::uint32_t span_end = boundaries[var_slot + 1U];
            ++var_slot;
            value.is_null = !not_null || span_start == span_end;
            if (!value.is_null) {
                value.bytes.assign(
                    row_bytes.begin() + static_cast<std::ptrdiff_t>(span_start),
                    row_bytes.begin() + static_cast<std::ptrdiff_t>(span_end));
            }
        }
    }

    result.ok = true;
    return result;
}

// Plain (uncompressed) UCS-2LE decode, matching every real Jet4 Name
// value observed during this slice's development. A value beginning
// with the 0xFF 0xFE "compressed unicode" marker mdbtools' HACKING.md
// documents is deliberately not decoded (see this file's own header
// comment) -- callers get a clear failure rather than a guess.
struct Jet4TextDecodeResult {
    bool ok = false;
    std::string text;
};

Jet4TextDecodeResult decode_jet4_text(const std::vector<std::uint8_t>& raw) {
    Jet4TextDecodeResult result;
    if (raw.size() >= 2U && raw[0] == 0xFFU && raw[1] == 0xFEU) {
        return result;  // ok=false: compressed unicode, not implemented.
    }
    if (raw.size() % 2U != 0U) {
        return result;
    }
    std::string text;
    text.reserve(raw.size());
    for (std::size_t index = 0U; index + 1U < raw.size() + 1U && index < raw.size(); index += 2U) {
        const auto code_unit = static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(raw[index]) |
            (static_cast<std::uint16_t>(raw[index + 1U]) << 8U));
        if (code_unit < 0x80U) {
            text += static_cast<char>(code_unit);
        } else if (code_unit < 0x800U) {
            text += static_cast<char>(0xC0U | (code_unit >> 6U));
            text += static_cast<char>(0x80U | (code_unit & 0x3FU));
        } else {
            text += static_cast<char>(0xE0U | (code_unit >> 12U));
            text += static_cast<char>(0x80U | ((code_unit >> 6U) & 0x3FU));
            text += static_cast<char>(0x80U | (code_unit & 0x3FU));
        }
    }
    result.ok = true;
    result.text = text;
    return result;
}

std::int32_t decode_signed_le(const std::vector<std::uint8_t>& bytes) {
    std::uint32_t accumulator = 0U;
    for (std::size_t index = 0U; index < bytes.size() && index < 4U; ++index) {
        accumulator |= static_cast<std::uint32_t>(bytes[index]) << (8U * index);
    }
    if (bytes.size() == 2U) {
        return static_cast<std::int16_t>(accumulator);
    }
    return static_cast<std::int32_t>(accumulator);
}

}  // namespace

AccessMSysObjectsScanResult scan_access_msysobjects_catalog(const std::string& path) {
    AccessMSysObjectsScanResult result;

    const AccessContainerParseResult header_result = parse_access_container_header_from_file(path);
    if (!header_result.ok) {
        result.error = header_result.error;
        return result;
    }
    if (!header_result.header.looks_like_access_container()) {
        result.error = access_msysobjects_text("Vfp.AccessMSysObjects.Error.NotAnAccessContainer");
        return result;
    }
    const std::size_t page_size = access_container_page_size(header_result.header.generation);
    if (page_size == 0U) {
        result.error = access_msysobjects_text("Vfp.AccessMSysObjects.Error.UnknownGeneration");
        return result;
    }
    const bool is_jet3 = (header_result.header.generation == AccessContainerGeneration::jet3);

    std::error_code file_size_error;
    const auto file_size = std::filesystem::file_size(
        copperfin::platform::path_from_utf8_string(path), file_size_error);
    if (file_size_error || file_size < page_size || file_size % page_size != 0U) {
        result.error = access_msysobjects_text("Vfp.AccessMSysObjects.Error.ReadPageFailed");
        return result;
    }
    const auto page_count = static_cast<std::uint32_t>(file_size / page_size);
    if (msysobjects_tdef_page >= page_count) {
        result.error = access_msysobjects_text("Vfp.AccessMSysObjects.Error.NotAnAccessContainer");
        return result;
    }

    std::ifstream input(copperfin::platform::path_from_utf8_string(path), std::ios::binary);
    if (!input) {
        result.error = access_msysobjects_text("Vfp.AccessMSysObjects.Error.OpenFileFailed");
        return result;
    }

    std::vector<std::uint8_t> tdef_page_bytes(page_size);
    input.seekg(static_cast<std::streamoff>(static_cast<std::uint64_t>(msysobjects_tdef_page) * page_size),
                std::ios::beg);
    input.read(reinterpret_cast<char*>(tdef_page_bytes.data()), static_cast<std::streamsize>(page_size));
    if (static_cast<std::size_t>(input.gcount()) < page_size) {
        result.error = access_msysobjects_text("Vfp.AccessMSysObjects.Error.ReadPageFailed");
        return result;
    }
    const AccessTableDefinition tdef = parse_access_table_definition_page(
        tdef_page_bytes, header_result.header.generation, msysobjects_tdef_page);
    if (!tdef.ok) {
        result.error = tdef.error;
        return result;
    }

    // The TDEF page's own column-descriptor order is physical STORAGE
    // order, not logical declaration order -- real-fixture testing
    // during this slice's development found a genuine Jet4 database
    // whose MSysObjects TDEF stores its column descriptors alphabetized
    // by name rather than in column_number order (docs/71 already noted
    // this same phenomenon for a different real fixture during #5476's
    // development: "the same 17 columns, alphabetically ordered in that
    // particular file"). Row decoding's null-mask bit position and
    // variable-column physical ordering are both driven by each column's
    // own column_number field (its true declaration order), not by its
    // position in tdef.columns -- so columns must be reindexed by
    // column_number here before anything downstream can safely use
    // vector position as that index. A column_number sequence that is
    // not a dense 0..N-1 permutation (e.g. after a real ALTER TABLE left
    // gaps -- never expected for Access's own MSysObjects, but checked
    // rather than assumed) fails closed instead of silently
    // mis-mapping a null-mask bit or a variable column's physical slot.
    std::vector<RowColumnLayout> columns(tdef.columns.size());
    std::vector<bool> column_number_seen(tdef.columns.size(), false);
    std::optional<std::size_t> id_index;
    std::optional<std::size_t> parent_id_index;
    std::optional<std::size_t> name_index;
    std::optional<std::size_t> type_index;
    for (const AccessColumnDefinition& column : tdef.columns) {
        if (column.column_number >= tdef.columns.size() || column_number_seen[column.column_number]) {
            result.error = access_msysobjects_text("Vfp.AccessMSysObjects.Error.UnexpectedSchema");
            return result;
        }
        column_number_seen[column.column_number] = true;
        columns[column.column_number] = {
            .type = column.type,
            .fixed_length = column.fixed_length,
            .length = column.length,
            .offset_f = column.offset_f};
        if (column.name == "Id") {
            id_index = column.column_number;
        } else if (column.name == "ParentId") {
            parent_id_index = column.column_number;
        } else if (column.name == "Name") {
            name_index = column.column_number;
        } else if (column.name == "Type") {
            type_index = column.column_number;
        }
    }
    if (!id_index.has_value() || !parent_id_index.has_value() || !name_index.has_value() ||
        !type_index.has_value()) {
        result.error = access_msysobjects_text("Vfp.AccessMSysObjects.Error.UnexpectedSchema");
        return result;
    }

    // Discover every data page belonging to MSysObjects (tdef_pg == 2),
    // via the same lightweight 8-byte-per-page header probe
    // scan_access_container_schema() uses -- never buffering the whole
    // file, only one page at a time for pages actually decoded.
    std::vector<std::uint32_t> data_pages;
    std::vector<std::uint8_t> header_probe(8U);
    for (std::uint32_t page_index = 0U; page_index < page_count; ++page_index) {
        input.seekg(static_cast<std::streamoff>(static_cast<std::uint64_t>(page_index) * page_size),
                    std::ios::beg);
        input.read(reinterpret_cast<char*>(header_probe.data()), static_cast<std::streamsize>(header_probe.size()));
        if (static_cast<std::size_t>(input.gcount()) < header_probe.size()) {
            result.error = access_msysobjects_text("Vfp.AccessMSysObjects.Error.ReadPageFailed");
            return result;
        }
        if (header_probe[0] == 0x01U && read_le_u32(header_probe, 4U) == msysobjects_tdef_page) {
            data_pages.push_back(page_index);
        }
    }

    std::vector<std::uint8_t> page_bytes(page_size);
    for (const std::uint32_t page_index : data_pages) {
        input.seekg(static_cast<std::streamoff>(static_cast<std::uint64_t>(page_index) * page_size),
                    std::ios::beg);
        input.read(reinterpret_cast<char*>(page_bytes.data()), static_cast<std::streamsize>(page_size));
        if (static_cast<std::size_t>(input.gcount()) < page_size) {
            result.skipped.push_back({
                .page_number = page_index,
                .row_index = 0U,
                .reason = access_msysobjects_text("Vfp.AccessMSysObjects.Error.ReadPageFailed")});
            continue;
        }

        const std::size_t header_size = is_jet3 ? 10U : 14U;
        const std::size_t num_rows_offset = is_jet3 ? 8U : 12U;
        const std::uint16_t num_rows = read_le_u16(page_bytes, num_rows_offset);
        std::vector<std::pair<std::uint16_t, std::uint16_t>> row_slots;  // (offset, flags)
        row_slots.reserve(num_rows);
        for (std::uint16_t row_index = 0U; row_index < num_rows; ++row_index) {
            const std::size_t slot_position = header_size + static_cast<std::size_t>(row_index) * 2U;
            if (slot_position + 2U > page_bytes.size()) {
                break;
            }
            const std::uint16_t raw_slot = read_le_u16(page_bytes, slot_position);
            row_slots.push_back({static_cast<std::uint16_t>(raw_slot & 0x3FFFU),
                                  static_cast<std::uint16_t>(raw_slot & 0xC000U)});
        }

        for (std::size_t row_index = 0U; row_index < row_slots.size(); ++row_index) {
            const auto [row_offset, flags] = row_slots[row_index];
            if ((flags & 0x8000U) != 0U) {
                continue;  // deleted row -- silently excluded, not an error.
            }
            if ((flags & 0x4000U) != 0U) {
                result.skipped.push_back({
                    .page_number = page_index,
                    .row_index = static_cast<std::uint32_t>(row_index),
                    .reason = access_msysobjects_text("Vfp.AccessMSysObjects.Error.LookupOverflowUnsupported")});
                continue;
            }
            const std::size_t row_end =
                (row_index == 0U) ? (page_size - 1U) : (static_cast<std::size_t>(row_slots[row_index - 1U].first) - 1U);
            if (row_offset > row_end || row_end >= page_bytes.size()) {
                result.skipped.push_back({
                    .page_number = page_index,
                    .row_index = static_cast<std::uint32_t>(row_index),
                    .reason = access_msysobjects_text("Vfp.AccessMSysObjects.Error.RowStructureInvalid")});
                continue;
            }
            const std::vector<std::uint8_t> row_bytes(
                page_bytes.begin() + static_cast<std::ptrdiff_t>(row_offset),
                page_bytes.begin() + static_cast<std::ptrdiff_t>(row_end + 1U));

            const DecodedRow decoded = decode_row(row_bytes, columns, is_jet3);
            if (!decoded.ok) {
                result.skipped.push_back({
                    .page_number = page_index,
                    .row_index = static_cast<std::uint32_t>(row_index),
                    .reason = decoded.error});
                continue;
            }

            const std::size_t id_column = *id_index;
            const std::size_t parent_id_column = *parent_id_index;
            const std::size_t name_column = *name_index;
            const std::size_t type_column = *type_index;

            AccessCatalogEntry entry;
            bool entry_ok = true;
            if (!decoded.columns[id_column].is_null) {
                entry.id = static_cast<std::uint32_t>(decode_signed_le(decoded.columns[id_column].bytes));
            }
            if (!decoded.columns[parent_id_column].is_null) {
                entry.parent_id = static_cast<std::uint32_t>(decode_signed_le(decoded.columns[parent_id_column].bytes));
            }
            if (!decoded.columns[type_column].is_null) {
                entry.type = static_cast<std::int16_t>(decode_signed_le(decoded.columns[type_column].bytes));
            }
            if (decoded.columns[name_column].is_null) {
                entry.name.clear();
            } else if (is_jet3) {
                entry.name = sanitize_as_utf8(decoded.columns[name_column].bytes);
            } else {
                const Jet4TextDecodeResult name_decode = decode_jet4_text(decoded.columns[name_column].bytes);
                if (!name_decode.ok) {
                    result.skipped.push_back({
                        .page_number = page_index,
                        .row_index = static_cast<std::uint32_t>(row_index),
                        .reason = access_msysobjects_text("Vfp.AccessMSysObjects.Error.CompressedUnicodeUnsupported")});
                    entry_ok = false;
                } else {
                    entry.name = name_decode.text;
                }
            }
            if (!entry_ok) {
                continue;
            }
            entry.candidate_page_number = entry.id & 0x00FFFFFFU;
            result.entries.push_back(std::move(entry));
        }
    }

    result.ok = true;
    return result;
}

}  // namespace copperfin::vfp
