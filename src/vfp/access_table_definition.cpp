// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/access_table_definition.h"
#include "copperfin/vfp/access_msysobjects.h"
#include "copperfin/platform/path.h"
#include "access_bytes_internal.h"

#include "copperfin/localization/localization.h"

#include <filesystem>
#include <fstream>
#include <mutex>
#include <set>
#include <string_view>
#include <system_error>

namespace copperfin::vfp {

namespace {

using copperfin::vfp::access_bytes_internal::read_le_u16;
using copperfin::vfp::access_bytes_internal::read_le_u32;
using copperfin::vfp::access_bytes_internal::sanitize_as_utf8;

localization::LocalizedCatalog access_table_definition_catalog() {
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

std::string access_table_definition_text(
    std::string_view key,
    const localization::PlaceholderMap& placeholders = {}) {
    return access_table_definition_catalog().translate(key, placeholders);
}

// Bounds-checked cursor over a single page's bytes. Every read advances the
// cursor; a read that would run past the page end sets `overrun` and
// returns 0 rather than reading out of bounds. Callers check `overrun`
// once after a whole structure is read, rather than after every field --
// this mirrors the fail-closed-on-crafted-input discipline used elsewhere
// in this codebase (e.g. dbf_table.cpp's bounds checks) for a page that
// declares more columns/indexes than it actually has room for.
struct PageCursor {
    const std::vector<std::uint8_t>& bytes;
    std::size_t position = 0U;
    bool overrun = false;

    explicit PageCursor(const std::vector<std::uint8_t>& page_bytes) : bytes(page_bytes) {}

    // `position <= bytes.size()` is an invariant this cursor always
    // maintains (every method that would advance past the end sets
    // `overrun` and clamps `position` to `bytes.size()` instead), so
    // `bytes.size() - position` can never underflow here -- which is what
    // lets this comparison avoid `position + count`, an addition that
    // could otherwise overflow for a large untrusted `count` (e.g. one
    // derived from a crafted page's own multi-byte field) and wrap around
    // to a small value, incorrectly appearing in-bounds and letting a
    // caller's bounds check be bypassed.
    [[nodiscard]] bool would_overrun(std::size_t count) const {
        return overrun || count > bytes.size() - position;
    }

    std::uint8_t u8() {
        if (would_overrun(1U)) {
            overrun = true;
            return 0U;
        }
        return bytes[position++];
    }

    std::uint16_t u16() {
        if (would_overrun(2U)) {
            overrun = true;
            return 0U;
        }
        const std::uint16_t value = read_le_u16(bytes, position);
        position += 2U;
        return value;
    }

    std::uint32_t u32() {
        if (would_overrun(4U)) {
            overrun = true;
            return 0U;
        }
        const std::uint32_t value = read_le_u32(bytes, position);
        position += 4U;
        return value;
    }

    void skip(std::size_t count) {
        if (would_overrun(count)) {
            overrun = true;
            position = bytes.size();
            return;
        }
        position += count;
    }

    // Safely skips `count` repeated `stride`-byte records (used for the
    // TDEF page's index-info block, whose repeat count -- num_real_idx --
    // is untrusted page data) without ever computing `count * stride`
    // directly: that multiplication is exactly the kind of untrusted-data
    // arithmetic that can overflow and wrap around to a small value,
    // silently bypassing bounds checking the way the plain multiply-then-
    // skip pattern this replaces once did. Dividing the remaining capacity
    // by `stride` instead can only shrink the effective count, never
    // overflow.
    void skip_repeated(std::size_t count, std::size_t stride) {
        if (overrun || stride == 0U) {
            return;
        }
        const std::size_t remaining = bytes.size() - position;
        if (count > remaining / stride) {
            overrun = true;
            position = bytes.size();
            return;
        }
        position += count * stride;
    }

    std::vector<std::uint8_t> take(std::size_t count) {
        if (would_overrun(count)) {
            overrun = true;
            return {};
        }
        std::vector<std::uint8_t> span(
            bytes.begin() + static_cast<std::ptrdiff_t>(position),
            bytes.begin() + static_cast<std::ptrdiff_t>(position + count));
        position += count;
        return span;
    }
};

AccessColumnType classify_column_type(std::uint8_t raw_type) {
    switch (raw_type) {
        case 0x01U: return AccessColumnType::boolean;
        case 0x02U: return AccessColumnType::byte_value;
        case 0x03U: return AccessColumnType::integer;
        case 0x04U: return AccessColumnType::long_integer;
        case 0x05U: return AccessColumnType::currency;
        case 0x06U: return AccessColumnType::single_float;
        case 0x07U: return AccessColumnType::double_float;
        case 0x08U: return AccessColumnType::date_time;
        case 0x09U: return AccessColumnType::binary;
        case 0x0AU: return AccessColumnType::text;
        case 0x0BU: return AccessColumnType::ole;
        case 0x0CU: return AccessColumnType::memo;
        case 0x0FU: return AccessColumnType::guid;
        case 0x10U: return AccessColumnType::numeric;
        default: return AccessColumnType::unknown;
    }
}

struct RawColumnDescriptor {
    std::uint8_t col_type = 0U;
    std::uint16_t col_num = 0U;
    std::uint8_t bitmask = 0U;
    std::uint16_t col_len = 0U;
    // Documented (and, for fixed-length columns, cross-validated against
    // real fixtures) to hold uninitialized garbage for non-fixed-length
    // columns -- see AccessColumnDefinition::offset_f's own comment.
    // Captured (not discarded) so #5539's row decoder
    // (access_msysobjects.cpp) can locate a fixed column's value within
    // a row without re-parsing the TDEF page a second time.
    std::uint16_t offset_f = 0U;
};

// Jet3 column descriptor, 18 bytes: col_type(1), col_num(2), offset_V(2),
// col_num repeat(2), sort_order(2), misc(2), unknown(2), bitmask(1),
// offset_F(2), col_len(2). offset_V/sort_order/misc are read and
// discarded here -- they are not needed for this slice's name/type/length/
// fixed-length schema output.
RawColumnDescriptor read_jet3_column_descriptor(PageCursor& cursor) {
    RawColumnDescriptor descriptor;
    descriptor.col_type = cursor.u8();
    descriptor.col_num = cursor.u16();
    cursor.u16();  // offset_V
    cursor.u16();  // col_num (repeat)
    cursor.u16();  // sort_order
    cursor.u16();  // misc
    cursor.u16();  // unknown
    descriptor.bitmask = cursor.u8();
    descriptor.offset_f = cursor.u16();
    descriptor.col_len = cursor.u16();
    return descriptor;
}

// Jet4 column descriptor, 25 bytes: col_type(1), unknown(4), col_num(2),
// offset_V(2), col_num repeat(2), misc(2), misc_ext(2), bitmask(1),
// misc_flags(1), unknown(4), offset_F(2), col_len(2).
RawColumnDescriptor read_jet4_column_descriptor(PageCursor& cursor) {
    RawColumnDescriptor descriptor;
    descriptor.col_type = cursor.u8();
    cursor.u32();  // unknown
    descriptor.col_num = cursor.u16();
    cursor.u16();  // offset_V
    cursor.u16();  // col_num (repeat)
    cursor.u16();  // misc
    cursor.u16();  // misc_ext
    descriptor.bitmask = cursor.u8();
    cursor.u8();   // misc_flags
    cursor.u32();  // unknown
    descriptor.offset_f = cursor.u16();
    descriptor.col_len = cursor.u16();
    return descriptor;
}

// Jet3 column names are stored in the database's legacy single-byte code
// page, not UTF-8. This codebase does not yet read the database's code-
// page byte -- it lives on the Database Definition page (page 0), which
// is itself "encrypted" with a simple RC4 key per mdbtools' notes, and
// decrypting that page is out of scope for this slice (see docs/71's own
// documented gaps). Correct transcoding therefore isn't possible without
// that follow-up work. sanitize_as_utf8() (access_bytes_internal.h, shared
// with access_msysobjects.cpp) guarantees the returned string is always
// valid UTF-8 regardless -- see that header's own comment.
std::string read_jet3_column_name(PageCursor& cursor) {
    const std::uint8_t length = cursor.u8();
    const std::vector<std::uint8_t> raw = cursor.take(length);
    return sanitize_as_utf8(raw);
}

// The Jet4 column-name length prefix is a byte count (not a character
// count) of UCS-2LE-encoded text, confirmed against real fixtures during
// this slice's development (the byte-count interpretation was the one
// that landed exactly on every subsequent column name's own length
// prefix, for every column in both a Jet3 and Jet4 real MSysObjects TDEF).
// Only the Basic Multilingual Plane is handled -- a surrogate pair encodes
// as two mojibake code points (U+FFFD-adjacent) rather than being
// reassembled, since real Access column names in practice are ASCII/Latin
// identifiers and this codebase has no evidence of needing astral-plane
// support here.
std::string read_jet4_column_name(PageCursor& cursor) {
    const std::uint16_t byte_length = cursor.u16();
    const std::vector<std::uint8_t> raw = cursor.take(byte_length);
    std::string text;
    text.reserve(raw.size());
    for (std::size_t index = 0U; index + 1U < raw.size(); index += 2U) {
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
    return text;
}

}  // namespace

std::size_t access_container_page_size(AccessContainerGeneration generation) {
    switch (generation) {
        case AccessContainerGeneration::jet3:
            return 2048U;
        case AccessContainerGeneration::jet4:
            return 4096U;
        case AccessContainerGeneration::later:
            // Extrapolated from Jet4, not independently verified against a
            // real .accdb fixture -- see this header's own documentation.
            return 4096U;
        case AccessContainerGeneration::unknown:
            return 0U;
    }
    return 0U;
}

const char* access_column_type_name(AccessColumnType type) {
    switch (type) {
        case AccessColumnType::unknown: return "unknown";
        case AccessColumnType::boolean: return "boolean";
        case AccessColumnType::byte_value: return "byte";
        case AccessColumnType::integer: return "integer";
        case AccessColumnType::long_integer: return "long_integer";
        case AccessColumnType::currency: return "currency";
        case AccessColumnType::single_float: return "single_float";
        case AccessColumnType::double_float: return "double_float";
        case AccessColumnType::date_time: return "date_time";
        case AccessColumnType::binary: return "binary";
        case AccessColumnType::text: return "text";
        case AccessColumnType::ole: return "ole";
        case AccessColumnType::memo: return "memo";
        case AccessColumnType::guid: return "guid";
        case AccessColumnType::numeric: return "numeric";
    }
    return "unknown";
}

AccessTableDefinition parse_access_table_definition_page(
    const std::vector<std::uint8_t>& page_bytes,
    AccessContainerGeneration generation,
    std::uint32_t page_number) {
    const std::size_t expected_size = access_container_page_size(generation);
    if (expected_size == 0U) {
        return {.ok = false, .error = access_table_definition_text("Vfp.AccessTableDefinition.Error.UnknownGeneration")};
    }
    if (page_bytes.size() != expected_size) {
        return {
            .ok = false,
            .error = access_table_definition_text(
                "Vfp.AccessTableDefinition.Error.WrongPageSize",
                {
                    {"expected", std::to_string(expected_size)},
                    {"actual", std::to_string(page_bytes.size())},
                })};
    }

    PageCursor cursor(page_bytes);
    const std::uint8_t page_type = cursor.u8();
    if (page_type != 0x02U) {
        return {
            .ok = false,
            .error = access_table_definition_text(
                "Vfp.AccessTableDefinition.Error.NotATdefPage",
                {{"pageType", std::to_string(page_type)}})};
    }
    cursor.u8();   // "unknown" byte
    cursor.u16();  // tdef_id (Jet3 "VC") / free_space (Jet4) -- unused here
    const std::uint32_t next_pg = cursor.u32();
    if (next_pg != 0U) {
        return {.ok = false, .error = access_table_definition_text("Vfp.AccessTableDefinition.Error.MultiPageTdefUnsupported")};
    }

    const bool is_jet3 = (generation == AccessContainerGeneration::jet3);

    std::uint32_t num_rows = 0U;
    std::uint8_t table_type = 0U;
    std::uint16_t num_cols = 0U;
    std::uint32_t num_real_idx = 0U;

    if (is_jet3) {
        cursor.u32();  // tdef_len
        num_rows = cursor.u32();
        cursor.u32();  // autonumber
        table_type = cursor.u8();
        cursor.u16();  // max_cols
        cursor.u16();  // num_var_cols
        num_cols = cursor.u16();
        cursor.u32();  // num_idx
        num_real_idx = cursor.u32();
        cursor.u32();  // used_pages
        cursor.u32();  // free_pages
        cursor.skip_repeated(num_real_idx, 8U);
    } else {
        cursor.u32();  // tdef_len
        cursor.u32();  // unknown
        num_rows = cursor.u32();
        cursor.u32();  // autonumber
        cursor.u8();   // autonum_flag
        cursor.skip(3U);
        cursor.u32();  // ct_autonum
        cursor.skip(8U);
        table_type = cursor.u8();
        cursor.u16();  // max_cols
        cursor.u16();  // num_var_cols
        num_cols = cursor.u16();
        cursor.u32();  // num_idx
        num_real_idx = cursor.u32();
        cursor.u32();  // used_pages
        cursor.u32();  // free_pages
        cursor.skip_repeated(num_real_idx, 12U);
    }

    if (cursor.overrun) {
        return {.ok = false, .error = access_table_definition_text("Vfp.AccessTableDefinition.Error.StructureOutOfBounds")};
    }
    if (table_type != 0x4EU && table_type != 0x53U) {
        // table_type is only documented to be 0x4E ('N', user table) or
        // 0x53 ('S', system table) -- treating anything else as "not a
        // system table" (the permissive default this codebase's first
        // version used) would let a corrupt page, or a page that merely
        // happens to start with the TDEF page-type byte by coincidence,
        // report ok=true with fabricated schema data instead of failing
        // closed the way an unrecognized value should.
        return {
            .ok = false,
            .error = access_table_definition_text(
                "Vfp.AccessTableDefinition.Error.UnrecognizedTableType",
                {{"tableType", std::to_string(table_type)}})};
    }

    std::vector<RawColumnDescriptor> raw_columns;
    raw_columns.reserve(num_cols);
    for (std::uint16_t index = 0U; index < num_cols; ++index) {
        raw_columns.push_back(is_jet3 ? read_jet3_column_descriptor(cursor) : read_jet4_column_descriptor(cursor));
    }
    if (cursor.overrun) {
        return {.ok = false, .error = access_table_definition_text("Vfp.AccessTableDefinition.Error.StructureOutOfBounds")};
    }

    std::vector<std::string> names;
    names.reserve(num_cols);
    for (std::uint16_t index = 0U; index < num_cols; ++index) {
        names.push_back(is_jet3 ? read_jet3_column_name(cursor) : read_jet4_column_name(cursor));
    }
    if (cursor.overrun) {
        return {.ok = false, .error = access_table_definition_text("Vfp.AccessTableDefinition.Error.StructureOutOfBounds")};
    }

    AccessTableDefinition result;
    result.ok = true;
    result.page_number = page_number;
    result.is_system_table = (table_type == 0x53U);
    result.row_count = num_rows;
    result.columns.reserve(num_cols);
    for (std::uint16_t index = 0U; index < num_cols; ++index) {
        const RawColumnDescriptor& raw = raw_columns[index];
        result.columns.push_back({
            .name = names[index],
            .type = classify_column_type(raw.col_type),
            .raw_type = raw.col_type,
            .column_number = raw.col_num,
            .length = raw.col_len,
            .fixed_length = (raw.bitmask & 0x01U) != 0U,
            .raw_bitmask = raw.bitmask,
            .offset_f = raw.offset_f
        });
    }
    return result;
}

AccessContainerSchemaResult scan_access_container_schema(const std::string& path) {
    AccessContainerSchemaResult result;

    const AccessContainerParseResult header_result = parse_access_container_header_from_file(path);
    if (!header_result.ok) {
        result.error = header_result.error;
        return result;
    }
    if (!header_result.header.looks_like_access_container()) {
        result.error = access_table_definition_text("Vfp.AccessTableDefinition.Error.NotAnAccessContainer");
        return result;
    }

    const std::size_t page_size = access_container_page_size(header_result.header.generation);
    if (page_size == 0U) {
        result.error = access_table_definition_text("Vfp.AccessTableDefinition.Error.UnknownGeneration");
        return result;
    }

    // A real Access database can be up to ~2 GB (the Jet/ACE file-size
    // ceiling), so this scan deliberately never buffers the whole file --
    // only individual pages, one at a time. std::filesystem::file_size()
    // gets the size without reading any file content.
    std::error_code file_size_error;
    const auto file_size = std::filesystem::file_size(
        copperfin::platform::path_from_utf8_string(path), file_size_error);
    if (file_size_error) {
        result.error = access_table_definition_text("Vfp.AccessTableDefinition.Error.OpenFileFailed");
        return result;
    }
    if (file_size < page_size || file_size % page_size != 0U) {
        // A file that isn't an exact multiple of the generation's page
        // size is truncated or corrupt -- fail closed rather than
        // silently ignoring the trailing partial page via integer
        // division, which could leave a real (but incomplete) page
        // unexamined without any indication anything was skipped.
        result.error = access_table_definition_text("Vfp.AccessTableDefinition.Error.ReadPageFailed");
        return result;
    }
    const auto page_count = static_cast<std::uint32_t>(file_size / page_size);

    std::ifstream input(copperfin::platform::path_from_utf8_string(path), std::ios::binary);
    if (!input) {
        result.error = access_table_definition_text("Vfp.AccessTableDefinition.Error.OpenFileFailed");
        return result;
    }

    // First pass: a lightweight 8-byte read per page (just enough for the
    // page-type byte and the next_pg field) to discover every TDEF-typed
    // page and its continuation chain, without ever holding more than one
    // page's worth of bytes in memory. page_headers itself is O(page
    // count), not O(file size) -- 5 bytes per page even for the largest
    // real Access file (~2 GB / 2048-byte Jet3 pages is roughly 1M pages,
    // ~5 MB here).
    struct PageHeaderInfo {
        std::uint8_t page_type = 0U;
        std::uint32_t next_pg = 0U;
    };
    std::vector<PageHeaderInfo> page_headers(page_count);
    std::vector<std::uint8_t> header_probe(8U);
    for (std::uint32_t page_index = 0U; page_index < page_count; ++page_index) {
        input.seekg(
            static_cast<std::streamoff>(static_cast<std::uint64_t>(page_index) * page_size),
            std::ios::beg);
        input.read(reinterpret_cast<char*>(header_probe.data()), static_cast<std::streamsize>(header_probe.size()));
        if (static_cast<std::size_t>(input.gcount()) < header_probe.size()) {
            result.error = access_table_definition_text("Vfp.AccessTableDefinition.Error.ReadPageFailed");
            return result;
        }
        page_headers[page_index].page_type = header_probe[0];
        page_headers[page_index].next_pg = read_le_u32(header_probe, 4U);
    }

    // A page pointed to by another TDEF's next_pg is a continuation page,
    // not an independent table -- excluded from the discovered-table list
    // even though its own leading byte is also 0x02. See
    // AccessTableDefinition::page_number's header comment for why this
    // scan cannot yet recover table *names*, only their TDEF page numbers.
    std::set<std::uint32_t> continuation_pages;
    std::vector<std::uint32_t> tdef_candidate_pages;
    for (std::uint32_t page_index = 0U; page_index < page_count; ++page_index) {
        if (page_headers[page_index].page_type == 0x02U) {
            tdef_candidate_pages.push_back(page_index);
        }
    }

    // Walk each TDEF candidate's own next_pg chain to mark every page in
    // it (beyond the chain's own start) as a continuation page, not just
    // the first hop -- a chain spanning more than two pages would
    // otherwise leave its later pages unmarked, letting them be
    // misreported as independent tables (and, if a later page's own
    // next_pg happens to be 0, even "successfully" parsed as one). Cycle
    // protection: a well-formed chain cannot legitimately revisit a page,
    // so a repeated or out-of-range next_pg target ends the walk rather
    // than looping forever on crafted/corrupt data. A "continuation" that
    // does not itself begin with the TDEF page-type byte also ends the
    // walk without erroring the whole scan, rather than reading an
    // unrelated page's bytes as if they were another next_pg pointer. All
    // of this uses the already-collected page_headers cache, not another
    // physical read.
    for (const std::uint32_t start_page : tdef_candidate_pages) {
        std::set<std::uint32_t> visited{start_page};
        std::uint32_t next_pg = page_headers[start_page].next_pg;
        while (next_pg != 0U && next_pg < page_count && !visited.contains(next_pg)) {
            continuation_pages.insert(next_pg);
            visited.insert(next_pg);
            if (page_headers[next_pg].page_type != 0x02U) {
                break;
            }
            next_pg = page_headers[next_pg].next_pg;
        }
    }

    // Second pass: read only the individual pages actually being decoded,
    // one page_size buffer reused across iterations -- never the whole
    // file.
    std::vector<std::uint8_t> page_bytes(page_size);
    for (const std::uint32_t page_index : tdef_candidate_pages) {
        if (continuation_pages.contains(page_index)) {
            continue;
        }
        input.seekg(
            static_cast<std::streamoff>(static_cast<std::uint64_t>(page_index) * page_size),
            std::ios::beg);
        input.read(reinterpret_cast<char*>(page_bytes.data()), static_cast<std::streamsize>(page_size));
        if (static_cast<std::size_t>(input.gcount()) < page_size) {
            result.skipped.push_back({
                .page_number = page_index,
                .reason = access_table_definition_text("Vfp.AccessTableDefinition.Error.ReadPageFailed")});
            continue;
        }
        AccessTableDefinition table = parse_access_table_definition_page(
            page_bytes, header_result.header.generation, page_index);
        if (table.ok) {
            result.tables.push_back(std::move(table));
        } else {
            result.skipped.push_back({.page_number = page_index, .reason = table.error});
        }
    }

    // #5539: attach a real name to each discovered table by decoding
    // MSysObjects's own catalog rows and matching each Type == 1 row's
    // candidate TDEF page number (its Id, masked) against the tables
    // actually discovered above. Only a match against an
    // ACTUALLY-DISCOVERED table is trusted -- a candidate page number
    // with no corresponding discovered table (e.g. #5541's real-fixture
    // finding that Id is not unconditionally reliable, before that was
    // resolved via independent-reader cross-verification -- see
    // docs/72) is silently left unattached rather than fabricating a
    // table entry from the catalog row alone. Failure to decode
    // MSysObjects at all (a container this slice's row decoder cannot
    // handle -- see access_msysobjects.h's own documented non-goals) is
    // not itself a scan failure: page-number-only results, this slice's
    // pre-#5539 behavior, remain the fallback rather than blocking the
    // whole schema scan on a best-effort enrichment step.
    const AccessMSysObjectsScanResult catalog = scan_access_msysobjects_catalog(path);
    if (catalog.ok) {
        for (AccessTableDefinition& table : result.tables) {
            for (const AccessCatalogEntry& entry : catalog.entries) {
                if (entry.type == 1 && entry.candidate_page_number == table.page_number) {
                    table.name = entry.name;
                    break;
                }
            }
        }
    }

    result.ok = true;
    return result;
}

}  // namespace copperfin::vfp
