// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/access_saved_queries.h"
#include "copperfin/vfp/access_table_definition.h"
#include "copperfin/vfp/access_msysobjects.h"
#include "copperfin/vfp/access_long_value.h"
#include "copperfin/platform/path.h"
#include "access_bytes_internal.h"

#include "copperfin/localization/localization.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <string_view>
#include <system_error>

namespace copperfin::vfp {

namespace {

using access_bytes_internal::read_le_u16;
using access_bytes_internal::read_le_u32;
using access_bytes_internal::sanitize_as_utf8;

localization::LocalizedCatalog access_saved_queries_catalog() {
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

std::string access_saved_queries_text(std::string_view key) {
    return access_saved_queries_catalog().translate(key, {});
}

// mdbtools' object-type enum (include/mdbtools.h.in), derived from
// MSysObjects.Type's low 7 bits (`type & 0x7F`) -- this codebase's own
// already-verified `Type == 1` == table convention (docs/72) matches
// this scheme's MDB_TABLE exactly.
constexpr std::int16_t kMdbQueryObjectType = 5;

// Plain (uncompressed) UCS-2LE decode, matching access_msysobjects.cpp's
// own local copy (#5539) -- duplicated here rather than shared, per this
// codebase's established precedent (access_long_value.cpp's own design
// notes) of keeping small, single-purpose decode helpers local rather
// than risking a cross-file refactor of already-verified code.
struct Jet4TextDecodeResult {
    bool ok = false;
    std::string text;
};

Jet4TextDecodeResult decode_jet4_text(const std::vector<std::uint8_t>& raw) {
    Jet4TextDecodeResult result;
    if (raw.size() >= 2U && raw[0] == 0xFFU && raw[1] == 0xFEU) {
        // Jet4 "compressed unicode" (mdbtools' HACKING.md, Text Data Type
        // section): after the 0xFF 0xFE marker, each byte is one
        // compressed character (an implicit 0x00 high byte) unless a
        // literal 0x00 byte appears, which HACKING.md documents as a
        // mode-switch to/from uncompressed 2-byte-per-character encoding
        // -- semantics this slice does not attempt to interpret (see
        // this file's own header comment). Real-fixture testing during
        // this slice's development found every observed
        // MSysQueries.Expression compressed value (mostly bare
        // table/column identifiers, e.g. "Employers.LeadMedia") to be
        // pure single-byte-per-character content with no embedded 0x00
        // byte, cross-checked against `mdb-queries`' own reconstructed
        // SQL as ground truth. Decoding that common, real, verified case
        // -- while failing closed rather than guessing at the
        // mode-switch case, which no real fixture available during this
        // slice's development ever exercised -- matches this codebase's
        // established discipline for #5539's own compressed-unicode gap
        // on MSysObjects.Name.
        std::string text;
        text.reserve(raw.size() - 2U);
        for (std::size_t index = 2U; index < raw.size(); ++index) {
            if (raw[index] == 0x00U) {
                return {};  // ok=false: mode-switch byte, not interpreted.
            }
            const auto code_unit = static_cast<std::uint16_t>(raw[index]);
            if (code_unit < 0x80U) {
                text += static_cast<char>(code_unit);
            } else {
                text += static_cast<char>(0xC0U | (code_unit >> 6U));
                text += static_cast<char>(0x80U | (code_unit & 0x3FU));
            }
        }
        result.ok = true;
        result.text = text;
        return result;
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

// A single MSysQueries column's already-boundary-resolved layout,
// mirroring access_msysobjects.cpp's own RowColumnLayout -- duplicated
// (not shared) per this codebase's established row-decoder precedent.
struct RowColumnLayout {
    AccessColumnType type = AccessColumnType::unknown;
    bool fixed_length = false;
    std::uint16_t length = 0;
    std::uint16_t offset_f = 0;
};

struct DecodedValue {
    bool is_null = true;
    std::vector<std::uint8_t> bytes;
};

struct DecodedRow {
    bool ok = false;
    std::string error;
    std::vector<DecodedValue> columns;
};

[[nodiscard]] bool row_would_overrun(std::size_t row_size, std::size_t start, std::size_t count) {
    return count > row_size || start > row_size - count;
}

// The general Jet3/Jet4 data-row decoding algorithm -- identical in
// structure to access_msysobjects.cpp's own decode_row() (#5539), which
// established and real-fixture-verified this exact boundary-computation
// approach. Duplicated here (not shared) rather than risking a
// cross-file refactor of that already-verified code, matching
// access_long_value.cpp's own documented precedent for this codebase.
DecodedRow decode_row(
    const std::vector<std::uint8_t>& row_bytes,
    const std::vector<RowColumnLayout>& tdef_columns,
    bool is_jet3) {
    DecodedRow result;
    const std::size_t row_size = row_bytes.size();
    const std::size_t num_cols_field_width = is_jet3 ? 1U : 2U;
    if (row_would_overrun(row_size, 0U, num_cols_field_width)) {
        result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.RowTooShort");
        return result;
    }
    const std::uint32_t num_cols_in_row =
        is_jet3 ? row_bytes[0] : read_le_u16(row_bytes, 0U);
    if (num_cols_in_row != tdef_columns.size()) {
        result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.ColumnCountMismatch");
        return result;
    }

    const std::size_t null_mask_size = (num_cols_in_row + 7U) / 8U;
    if (row_would_overrun(row_size, row_size - null_mask_size, null_mask_size)) {
        result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.RowTooShort");
        return result;
    }
    const std::size_t null_mask_start = row_size - null_mask_size;

    const std::size_t var_len_field_width = is_jet3 ? 1U : 2U;
    if (row_would_overrun(row_size, 0U, var_len_field_width) ||
        null_mask_start < var_len_field_width) {
        result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.RowTooShort");
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
        result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.ColumnCountMismatch");
        return result;
    }

    if (is_jet3 && row_size >= 256U) {
        result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.JumpTableUnsupported");
        return result;
    }
    const std::size_t entry_width = is_jet3 ? 1U : 2U;

    const std::size_t eod_and_table_size = entry_width * (var_len + 1U);
    if (var_len_start < eod_and_table_size) {
        result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.RowTooShort");
        return result;
    }
    const std::size_t block_start = var_len_start - eod_and_table_size;
    const std::uint32_t eod =
        is_jet3 ? row_bytes[block_start] : read_le_u16(row_bytes, block_start);
    if (eod > row_size) {
        result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.RowStructureInvalid");
        return result;
    }

    std::vector<std::uint32_t> boundaries(var_len + 1U);
    for (std::size_t slot = 0U; slot < var_len; ++slot) {
        const std::size_t entry_position = block_start + entry_width * (1U + slot);
        const std::uint32_t raw_value =
            is_jet3 ? row_bytes[entry_position] : read_le_u16(row_bytes, entry_position);
        boundaries[var_len - 1U - slot] = raw_value;
    }
    boundaries[var_len] = eod;
    for (std::size_t index = 0U; index < var_len; ++index) {
        if (boundaries[index] > boundaries[index + 1U] || boundaries[index + 1U] > row_size) {
            result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.RowStructureInvalid");
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
                result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.RowStructureInvalid");
                return result;
            }
            value.is_null = !not_null;
            value.bytes.assign(row_bytes.begin() + static_cast<std::ptrdiff_t>(start),
                                row_bytes.begin() + static_cast<std::ptrdiff_t>(start + column.length));
        } else {
            const std::uint32_t span_start = boundaries[var_slot];
            const std::uint32_t span_end = boundaries[var_slot + 1U];
            ++var_slot;
            value.is_null = !not_null;
            if (!value.is_null && span_start != span_end) {
                value.bytes.assign(
                    row_bytes.begin() + static_cast<std::ptrdiff_t>(span_start),
                    row_bytes.begin() + static_cast<std::ptrdiff_t>(span_end));
            }
        }
    }

    result.ok = true;
    return result;
}

struct QueryClauseRow {
    std::int32_t object_id = 0;
    std::uint8_t attribute = 0;
    std::uint16_t flag = 0;
    std::string name1;
    std::string name2;
    std::string expression;
};

// Decodes every non-deleted MSysQueries row, resolving Expression's
// long-value bytes (#5549) and Name1/Name2/Expression's Jet3/Jet4 text
// encoding the same way access_msysobjects.cpp already does for its own
// columns.
//
// A row whose fixed columns (ObjectId/Attribute/Flag) decode successfully
// but whose Name1/Name2/Expression text cannot be decoded (e.g. the
// Jet4 compressed-unicode mode-switch case this slice does not
// interpret) still has a known ObjectId -- its id is recorded in
// `incomplete_object_ids` so the caller can mark that WHOLE query
// skipped rather than silently reconstructing incomplete SQL from only
// its other, successfully-decoded clause rows.
//
// A row that fails to decode structurally (corrupt directory, jump-table
// Jet3 row, column-count mismatch) has no known ObjectId to attribute
// the failure to -- recorded only in `any_unattributed_row_skipped`,
// which the caller treats as a signal that a page-level anomaly exists,
// without being able to say which specific query it affected.
struct ClauseRowScanResult {
    bool ok = false;
    std::string error;
    std::vector<QueryClauseRow> rows;
    std::set<std::int32_t> incomplete_object_ids;
    bool any_unattributed_row_skipped = false;
};

ClauseRowScanResult scan_msysqueries_rows(
    const std::string& path,
    AccessContainerGeneration generation,
    std::uint32_t page_size,
    bool is_jet3,
    std::uint32_t msysqueries_page_number,
    std::uint32_t msysqueries_declared_row_count,
    const std::vector<AccessColumnDefinition>& raw_columns) {
    ClauseRowScanResult result;

    std::vector<const AccessColumnDefinition*> ordered(raw_columns.size(), nullptr);
    for (const AccessColumnDefinition& column : raw_columns) {
        if (column.column_number >= ordered.size() || ordered[column.column_number] != nullptr) {
            result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.UnexpectedSchema");
            return result;
        }
        ordered[column.column_number] = &column;
    }

    std::optional<std::size_t> object_id_index;
    std::optional<std::size_t> attribute_index;
    std::optional<std::size_t> flag_index;
    std::optional<std::size_t> name1_index;
    std::optional<std::size_t> name2_index;
    std::optional<std::size_t> expression_index;
    std::vector<RowColumnLayout> columns(ordered.size());
    for (std::size_t index = 0U; index < ordered.size(); ++index) {
        if (ordered[index] == nullptr) {
            // #5551 review (Copilot): a non-dense column_number sequence
            // (a gap) would otherwise leave this slot null and crash on
            // dereference below -- fail closed instead, matching
            // access_msysobjects.cpp's own column_number_seen precedent.
            result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.UnexpectedSchema");
            return result;
        }
        columns[index] = {
            .type = ordered[index]->type,
            .fixed_length = ordered[index]->fixed_length,
            .length = ordered[index]->length,
            .offset_f = ordered[index]->offset_f};
        if (ordered[index]->name == "ObjectId") {
            object_id_index = index;
        } else if (ordered[index]->name == "Attribute") {
            attribute_index = index;
        } else if (ordered[index]->name == "Flag") {
            flag_index = index;
        } else if (ordered[index]->name == "Name1") {
            name1_index = index;
        } else if (ordered[index]->name == "Name2") {
            name2_index = index;
        } else if (ordered[index]->name == "Expression") {
            expression_index = index;
        }
    }
    if (!object_id_index.has_value() || !attribute_index.has_value() || !flag_index.has_value() ||
        !name1_index.has_value() || !name2_index.has_value() || !expression_index.has_value()) {
        result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.UnexpectedSchema");
        return result;
    }

    std::ifstream input(platform::path_from_utf8_string(path), std::ios::binary);
    if (!input) {
        result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.OpenFileFailed");
        return result;
    }
    std::error_code file_size_error;
    const auto file_size = std::filesystem::file_size(platform::path_from_utf8_string(path), file_size_error);
    if (file_size_error || file_size < page_size || file_size % page_size != 0U) {
        result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.ReadPageFailed");
        return result;
    }
    const auto page_count = static_cast<std::uint32_t>(file_size / page_size);

    std::vector<std::uint32_t> data_pages;
    std::vector<std::uint8_t> header_probe(8U);
    for (std::uint32_t page_index = 0U; page_index < page_count; ++page_index) {
        input.seekg(static_cast<std::streamoff>(static_cast<std::uint64_t>(page_index) * page_size), std::ios::beg);
        input.read(reinterpret_cast<char*>(header_probe.data()), static_cast<std::streamsize>(header_probe.size()));
        if (static_cast<std::size_t>(input.gcount()) < header_probe.size()) {
            result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.ReadPageFailed");
            return result;
        }
        if (header_probe[0] == 0x01U && read_le_u32(header_probe, 4U) == msysqueries_page_number) {
            data_pages.push_back(page_index);
        }
    }

    // #5551 review (Codex, P2): a stale/freed page that still carries
    // MSysQueries's own tdef_pg header would otherwise be silently
    // incorporated -- matching access_msysobjects.cpp's own
    // RowCountExceedsDeclared precedent, this is checked against the
    // TDEF's own declared row_count after the scan below.
    std::size_t total_non_deleted_row_slots = 0U;

    std::vector<std::uint8_t> page_bytes(page_size);
    for (const std::uint32_t page_index : data_pages) {
        input.seekg(static_cast<std::streamoff>(static_cast<std::uint64_t>(page_index) * page_size), std::ios::beg);
        input.read(reinterpret_cast<char*>(page_bytes.data()), static_cast<std::streamsize>(page_size));
        if (static_cast<std::size_t>(input.gcount()) < page_size) {
            result.any_unattributed_row_skipped = true;
            continue;
        }

        const std::size_t header_size = is_jet3 ? 10U : 14U;
        const std::size_t num_rows_offset = is_jet3 ? 8U : 12U;
        const std::uint16_t num_rows = read_le_u16(page_bytes, num_rows_offset);
        const std::size_t directory_size = static_cast<std::size_t>(num_rows) * 2U;
        if (directory_size > page_bytes.size() - header_size) {
            result.any_unattributed_row_skipped = true;
            continue;
        }
        const std::size_t directory_end = header_size + directory_size;

        std::vector<std::pair<std::uint16_t, std::uint16_t>> row_slots;
        row_slots.reserve(num_rows);
        bool directory_valid = true;
        for (std::uint16_t row_index = 0U; row_index < num_rows; ++row_index) {
            const std::size_t slot_position = header_size + static_cast<std::size_t>(row_index) * 2U;
            const std::uint16_t raw_slot = read_le_u16(page_bytes, slot_position);
            const auto row_offset = static_cast<std::uint16_t>(raw_slot & 0x3FFFU);
            if (row_offset < directory_end) {
                directory_valid = false;
                break;
            }
            row_slots.push_back({row_offset, static_cast<std::uint16_t>(raw_slot & 0xC000U)});
        }
        if (!directory_valid) {
            result.any_unattributed_row_skipped = true;
            continue;
        }

        for (std::size_t row_index = 0U; row_index < row_slots.size(); ++row_index) {
            const auto [row_offset, flags] = row_slots[row_index];
            if ((flags & 0x8000U) != 0U) {
                continue;  // deleted row.
            }
            ++total_non_deleted_row_slots;
            if ((flags & 0x4000U) != 0U) {
                result.any_unattributed_row_skipped = true;
                continue;  // lookup-overflow row, not followed.
            }
            const std::size_t row_end =
                (row_index == 0U) ? (page_size - 1U) : (static_cast<std::size_t>(row_slots[row_index - 1U].first) - 1U);
            if (row_offset > row_end || row_end >= page_bytes.size()) {
                result.any_unattributed_row_skipped = true;
                continue;
            }
            const std::vector<std::uint8_t> row_bytes(
                page_bytes.begin() + static_cast<std::ptrdiff_t>(row_offset),
                page_bytes.begin() + static_cast<std::ptrdiff_t>(row_end + 1U));

            const DecodedRow decoded = decode_row(row_bytes, columns, is_jet3);
            if (!decoded.ok) {
                result.any_unattributed_row_skipped = true;
                continue;
            }

            if (decoded.columns[*object_id_index].is_null) {
                // #5551 review (Copilot): a NULL ObjectId cannot be
                // attributed to a specific query -- treating it as 0
                // would risk misattributing this row to whatever query
                // happens to have Id 0 (or silently to nothing). Matches
                // this function's own unattributed-failure precedent for
                // other structurally-anomalous rows.
                result.any_unattributed_row_skipped = true;
                continue;
            }
            QueryClauseRow clause_row;
            clause_row.object_id = decode_signed_le(decoded.columns[*object_id_index].bytes);
            if (!decoded.columns[*attribute_index].is_null) {
                clause_row.attribute = static_cast<std::uint8_t>(
                    decode_signed_le(decoded.columns[*attribute_index].bytes));
            }
            if (!decoded.columns[*flag_index].is_null) {
                clause_row.flag = static_cast<std::uint16_t>(decode_signed_le(decoded.columns[*flag_index].bytes));
            }

            bool text_ok = true;
            const auto decode_text_column = [&](std::size_t column_index, std::string& out) {
                const DecodedValue& value = decoded.columns[column_index];
                if (value.is_null) {
                    return;
                }
                if (is_jet3) {
                    out = sanitize_as_utf8(value.bytes);
                } else {
                    const Jet4TextDecodeResult decoded_text = decode_jet4_text(value.bytes);
                    if (!decoded_text.ok) {
                        text_ok = false;
                        return;
                    }
                    out = decoded_text.text;
                }
            };
            decode_text_column(*name1_index, clause_row.name1);
            decode_text_column(*name2_index, clause_row.name2);

            const DecodedValue& expression_value = decoded.columns[*expression_index];
            if (!expression_value.is_null && !expression_value.bytes.empty()) {
                const AccessLongValueResult long_value_result =
                    read_access_long_value_column(path, generation, expression_value.bytes);
                if (!long_value_result.ok) {
                    text_ok = false;
                } else if (is_jet3) {
                    clause_row.expression = sanitize_as_utf8(long_value_result.value);
                } else {
                    const Jet4TextDecodeResult decoded_text = decode_jet4_text(long_value_result.value);
                    if (!decoded_text.ok) {
                        text_ok = false;
                    } else {
                        clause_row.expression = decoded_text.text;
                    }
                }
            }
            if (!text_ok) {
                // ObjectId is already known at this point (decode_row()
                // succeeded and the fixed columns were extracted above),
                // so this failure can be attributed to a specific query
                // -- the caller marks that whole query skipped rather
                // than reconstructing incomplete SQL from only its other
                // clause rows.
                result.incomplete_object_ids.insert(clause_row.object_id);
                continue;
            }

            result.rows.push_back(std::move(clause_row));
        }
    }

    if (total_non_deleted_row_slots > msysqueries_declared_row_count) {
        result.rows.clear();
        result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.RowCountExceedsDeclared");
        return result;
    }

    result.ok = true;
    return result;
}

// Reconstructs one query's SQL from its own clause rows, matching
// mdb-queries.c's own Attribute-value switch exactly (see this file's
// header comment for the full mapping and its documented scope limits).
std::string reconstruct_sql(const std::vector<const QueryClauseRow*>& clause_rows) {
    std::string predicate;
    std::string tables;
    std::string columns_list;
    std::string where_clause;
    std::string sorting;

    for (const QueryClauseRow* row : clause_rows) {
        switch (row->attribute) {
            case 3U:  // predicate
                if ((row->flag & 0x30U) != 0U) {
                    predicate = " TOP " + row->name1;
                    if ((row->flag & 0x20U) != 0U) {
                        predicate += " PERCENT";
                    }
                } else if ((row->flag & 0x8U) != 0U) {
                    predicate = " DISTINCTROW";
                } else if ((row->flag & 0x2U) != 0U) {
                    predicate = " DISTINCT";
                }
                break;
            case 5U:  // table name
                if (!tables.empty()) {
                    tables += ",";
                }
                tables += "[" + row->name1 + "]";
                break;
            case 6U:  // column expression
                if (!columns_list.empty()) {
                    columns_list += ",";
                }
                columns_list += row->expression;
                break;
            case 7U:  // join/relationship clause -- read but not incorporated, matching mdb-queries.c.
                break;
            case 8U:  // where clause
                where_clause = row->expression;
                break;
            case 11U:  // sorting
                // #5551 review (Codex P1): a query sorted by two or more
                // fields has one MSysQueries row per sort expression --
                // append every one, comma-separated, rather than
                // discarding all but the first.
                if (!sorting.empty()) {
                    sorting += ",";
                }
                sorting += row->expression;
                if (row->name1 == "D") {
                    sorting += " DESCENDING";
                }
                break;
            default:
                break;
        }
    }

    std::string sql = "SELECT" + predicate + " " + columns_list + " FROM " + tables;
    if (!where_clause.empty()) {
        sql += " WHERE " + where_clause;
    }
    if (!sorting.empty()) {
        sql += " ORDER BY " + sorting;
    }
    return sql;
}

}  // namespace

AccessSavedQueriesScanResult scan_access_saved_queries(const std::string& path) {
    AccessSavedQueriesScanResult result;

    const AccessContainerParseResult header_result = parse_access_container_header_from_file(path);
    if (!header_result.ok) {
        result.error = header_result.error;
        return result;
    }
    if (!header_result.header.looks_like_access_container()) {
        result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.NotAnAccessContainer");
        return result;
    }
    const std::size_t page_size = access_container_page_size(header_result.header.generation);
    if (page_size == 0U) {
        result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.UnknownGeneration");
        return result;
    }
    const bool is_jet3 = (header_result.header.generation == AccessContainerGeneration::jet3);

    const AccessMSysObjectsScanResult catalog = scan_access_msysobjects_catalog(path);
    if (!catalog.ok) {
        result.error = catalog.error;
        return result;
    }

    std::map<std::int32_t, std::string> query_names_by_id;
    for (const AccessCatalogEntry& entry : catalog.entries) {
        if ((entry.type & 0x7FU) == kMdbQueryObjectType) {
            query_names_by_id[static_cast<std::int32_t>(entry.id)] = entry.name;
        }
    }
    if (query_names_by_id.empty()) {
        result.ok = true;
        return result;
    }

    const AccessContainerSchemaResult schema = scan_access_container_schema(path);
    if (!schema.ok) {
        result.error = schema.error;
        return result;
    }
    const AccessTableDefinition* msysqueries = nullptr;
    for (const AccessTableDefinition& table : schema.tables) {
        if (table.name.has_value() && *table.name == "MSysQueries") {
            msysqueries = &table;
            break;
        }
    }
    if (msysqueries == nullptr) {
        result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.MSysQueriesNotFound");
        return result;
    }

    const ClauseRowScanResult clause_scan = scan_msysqueries_rows(
        path, header_result.header.generation, static_cast<std::uint32_t>(page_size), is_jet3,
        msysqueries->page_number, msysqueries->row_count, msysqueries->columns);
    if (!clause_scan.ok) {
        result.error = clause_scan.error;
        return result;
    }
    if (clause_scan.any_unattributed_row_skipped) {
        // A structurally-undecodable MSysQueries row (corrupt directory,
        // Jet3 jump-table row, column-count mismatch) has no ObjectId to
        // attribute the failure to a specific query -- unlike an
        // incomplete-text failure (handled per-query below), this could
        // silently affect ANY query's reconstruction. Matching
        // access_msysobjects.cpp's own RowCountExceedsDeclared precedent
        // for provenance uncertainty, fail the whole scan closed rather
        // than returning a query set of unknown completeness.
        result.error = access_saved_queries_text("Vfp.AccessSavedQueries.Error.UnattributedRowFailure");
        return result;
    }

    std::map<std::int32_t, std::vector<const QueryClauseRow*>> rows_by_object_id;
    for (const QueryClauseRow& row : clause_scan.rows) {
        rows_by_object_id[row.object_id].push_back(&row);
    }

    for (const auto& [id, name] : query_names_by_id) {
        if (clause_scan.incomplete_object_ids.count(id) > 0U) {
            // At least one of this query's own clause rows had text this
            // slice could not decode (e.g. the Jet4 compressed-unicode
            // mode-switch case) -- skip the whole query rather than
            // reconstructing SQL from only its other, successfully
            // decoded clause rows, which would silently look complete
            // but be missing pieces (a column, a WHERE clause, ...).
            result.skipped.push_back({.name = name, .reason = access_saved_queries_text(
                "Vfp.AccessSavedQueries.Error.IncompleteClauseRow")});
            continue;
        }
        const auto it = rows_by_object_id.find(id);
        if (it == rows_by_object_id.end() || it->second.empty()) {
            result.skipped.push_back({.name = name, .reason = access_saved_queries_text(
                "Vfp.AccessSavedQueries.Error.NoClauseRows")});
            continue;
        }
        // #5551 review (Codex P1, Copilot, duplicate finding): neither
        // mdb-queries.c nor any currently-allowed evidence source
        // documents how to positively identify a non-SELECT query type
        // (action/crosstab/union/pass-through/data-definition) from
        // MSysQueries's own row structure -- guessing at undocumented
        // Attribute values is exactly what this codebase's discipline
        // avoids. What IS a defensible, evidence-grounded minimum sanity
        // bar: every real SELECT-shaped query this slice's development
        // observed had at least one Attribute==5 (table) row. A query
        // with clause rows but no table reference at all cannot be a
        // sensible SELECT -- skip it rather than returning a fabricated
        // "SELECT ... FROM " with an empty FROM clause. This is a
        // partial mitigation, not full query-type detection: a non-
        // SELECT query that DOES reference tables the same way a SELECT
        // would (plausible for UPDATE/DELETE) is not caught by this
        // check and may still produce misleading SQL text -- see this
        // file's own header comment and docs/76 for the documented scope
        // boundary this leaves.
        const bool has_table_reference = std::any_of(
            it->second.begin(), it->second.end(),
            [](const QueryClauseRow* row) { return row->attribute == 5U; });
        if (!has_table_reference) {
            result.skipped.push_back({.name = name, .reason = access_saved_queries_text(
                "Vfp.AccessSavedQueries.Error.NoTableReference")});
            continue;
        }
        result.queries.push_back({.name = name, .sql = reconstruct_sql(it->second)});
    }

    result.ok = true;
    return result;
}

}  // namespace copperfin::vfp
