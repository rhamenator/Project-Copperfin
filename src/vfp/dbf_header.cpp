// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/dbf_header.h"
#include "copperfin/platform/path.h"

#include "copperfin/localization/localization.h"

#include <filesystem>
#include <fstream>
#include <locale>
#include <mutex>
#include <optional>
#include <sstream>
#include <string_view>

namespace copperfin::vfp {

namespace {

std::uint16_t read_le_u16(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint16_t>(bytes[offset]) |
           (static_cast<std::uint16_t>(bytes[offset + 1]) << 8U);
}

std::uint32_t read_le_u32(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint32_t>(bytes[offset]) |
           (static_cast<std::uint32_t>(bytes[offset + 1]) << 8U) |
           (static_cast<std::uint32_t>(bytes[offset + 2]) << 16U) |
           (static_cast<std::uint32_t>(bytes[offset + 3]) << 24U);
}

std::string two_digit(std::uint8_t value) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    if (value < 10U) {
        stream << '0';
    }
    stream << static_cast<unsigned int>(value);
    return stream.str();
}

localization::LocalizedCatalog dbf_header_catalog() {
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

std::string dbf_header_text(std::string_view key) {
    return dbf_header_catalog().translate(key);
}

std::string dbf_header_text(const localization::LocalizedCatalog& catalog, std::string_view key) {
    return catalog.translate(key);
}

}  // namespace

bool DbfHeader::looks_like_dbf() const {
    return header_length >= 32U && record_length > 0U;
}

bool DbfHeader::has_database_container() const {
    return (table_flags & 0x04U) != 0U;
}

bool DbfHeader::has_production_index() const {
    return (table_flags & 0x01U) != 0U;
}

bool DbfHeader::has_structural_cdx() const {
    return has_production_index();
}

bool DbfHeader::has_memo_file() const {
    // dBASE Level 7 retains its version in the low three bits. Either bit 3
    // (dBASE IV-style) or bit 7 (dBASE III PLUS-style) records a DBT memo
    // sidecar, independently of the SQL-table flag bits.
    if ((version & 0x07U) == 0x04U) {
        return (version & 0x88U) != 0U;
    }
    return version == 0x83U || version == 0x8BU || version == 0xF5U;
}

DbfFormatFamily DbfHeader::format_family() const {
    // dBASE Level 7 stores version 4 in the low three bits; higher bits
    // describe memo and SQL-table flags and do not change the file family.
    if ((version & 0x07U) == 0x04U) {
        return DbfFormatFamily::dbase;
    }

    switch (version) {
        case 0x02U:
        case 0xFBU:
            return DbfFormatFamily::foxbase;
        case 0x03U:
        case 0x43U:
        case 0x63U:
        case 0x83U:
        case 0x8BU:
        case 0xCBU:
            return DbfFormatFamily::dbase;
        case 0xF5U:
            return DbfFormatFamily::foxpro;
        case 0x30U:
        case 0x31U:
        case 0x32U:
            return DbfFormatFamily::visual_foxpro;
        default:
            return DbfFormatFamily::unknown;
    }
}

const char* dbf_format_family_name(DbfFormatFamily family) {
    switch (family) {
        case DbfFormatFamily::unknown:
            return "unknown";
        case DbfFormatFamily::foxbase:
            return "foxbase";
        case DbfFormatFamily::foxpro:
            return "foxpro";
        case DbfFormatFamily::dbase:
            return "dbase";
        case DbfFormatFamily::visual_foxpro:
            return "visual_foxpro";
    }
    return "unknown";
}

std::string DbfHeader::version_description() const {
    return version_description(dbf_header_catalog());
}

std::string DbfHeader::version_description(const localization::LocalizedCatalog& catalog) const {
    if ((version & 0x07U) == 0x04U) {
        const bool has_memo = (version & 0x88U) != 0U;
        const bool is_sql_table = (version & 0x70U) != 0U;
        if (has_memo && is_sql_table) {
            return dbf_header_text(catalog, "Vfp.DbfHeader.Version.DbaseLevel7MemoSql");
        }
        if (has_memo) {
            return dbf_header_text(catalog, "Vfp.DbfHeader.Version.DbaseLevel7Memo");
        }
        if (is_sql_table) {
            return dbf_header_text(catalog, "Vfp.DbfHeader.Version.DbaseLevel7SqlTable");
        }
        return dbf_header_text(catalog, "Vfp.DbfHeader.Version.DbaseLevel7");
    }
    switch (version) {
        case 0x02U:
        case 0xFBU:
            return dbf_header_text(catalog, "Vfp.DbfHeader.Version.Foxbase");
        case 0x03U:
            return dbf_header_text(catalog, "Vfp.DbfHeader.Version.DbaseIiiCompatible");
        case 0x30U:
            return dbf_header_text(catalog, "Vfp.DbfHeader.Version.VisualFoxPro");
        case 0x31U:
            return dbf_header_text(catalog, "Vfp.DbfHeader.Version.VisualFoxProAutoincrement");
        case 0x32U:
            return dbf_header_text(catalog, "Vfp.DbfHeader.Version.VisualFoxProVarbinaryVarchar");
        case 0x43U:
            return dbf_header_text(catalog, "Vfp.DbfHeader.Version.DbaseIvSqlTable");
        case 0x63U:
            return dbf_header_text(catalog, "Vfp.DbfHeader.Version.DbaseIvSystemFile");
        case 0x83U:
            return dbf_header_text(catalog, "Vfp.DbfHeader.Version.DbaseIiiMemo");
        case 0x8BU:
            return dbf_header_text(catalog, "Vfp.DbfHeader.Version.DbaseIvMemo");
        case 0xCBU:
            return dbf_header_text(catalog, "Vfp.DbfHeader.Version.DbaseIvMemoSql");
        case 0xF5U:
            return dbf_header_text(catalog, "Vfp.DbfHeader.Version.FoxProMemo");
        default:
            return dbf_header_text(catalog, "Vfp.DbfHeader.Version.Unknown");
    }
}

unsigned int DbfHeader::last_update_year_full() const {
    // The on-disk last-update-date byte is nominally documented as
    // "years since 1900" (a theoretical 0-255 range covering 1900-2155),
    // but real dBASE-family products observably write a genuine
    // two-digit calendar year (year % 100) instead -- confirmed against
    // real FoxBASE+ 2.10 output: a table last updated 2026-09-09 has
    // this byte equal to 26, not 126 (issue #5527). That makes any byte
    // in [0, 99] genuinely ambiguous between centuries -- the same
    // ambiguity every two-digit-year scheme has, and not something any
    // rule can perfectly resolve for all time simultaneously.
    //
    // Disambiguated here with a *fixed* century-rollover threshold, not
    // a "now"-relative one like Visual FoxPro's own `SET CENTURY TO ...
    // ROLLOVER` default (current two-digit year + 50): a byte below the
    // threshold is read as 20xx, at or above it as 19xx. A fixed
    // threshold is the better fit specifically for *this* field, even
    // though it departs from VFP's own live-date-string convention --
    // genuine dBASE-family software only ever wrote two-digit years
    // during roughly 1980-1999, a historically fixed window that does
    // not advance with the calendar the way a user typing a 2-digit year
    // today does, so anchoring the threshold to "now" would eventually
    // misclassify *those* real legacy files once enough years pass,
    // rather than staying correct for them indefinitely. 80 keeps every
    // real 1980s/1990s dBASE-era byte (80-99) in the 1900s permanently,
    // while covering the 2000-2079 range for modern/Copperfin-written
    // files -- matching the real FoxBASE+ 2.10 evidence above (26 < 80,
    // so 2000 + 26 = 2026) and remaining stable for decades to come.
    //
    // A byte >= 100 is unambiguous on its own (it cannot be a genuine
    // two-digit year) and always means 1900 + byte -- this covers years
    // Copperfin's own writer produced before #5527, when it wrote the
    // raw years-since-1900 value rather than year % 100, and needs no
    // special-casing: the same "1900 + byte" arithmetic this function
    // always used still applies for exactly those out-of-range bytes.
    //
    // This is the single, shared source of truth for the disambiguation
    // -- every caller that needs `last_update_year` as a real calendar
    // year (not just `last_update_iso8601()`) must call this rather than
    // computing `1900 + last_update_year` directly, or it will silently
    // reintroduce this exact bug for its own callers (found in review:
    // the LUPDATE() runtime function had its own independent, unfixed
    // copy of the old computation).
    constexpr unsigned int century_rollover_threshold = 80U;
    return (last_update_year < century_rollover_threshold)
        ? (2000U + static_cast<unsigned int>(last_update_year))
        : (1900U + static_cast<unsigned int>(last_update_year));
}

std::string DbfHeader::last_update_iso8601() const {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << last_update_year_full() << '-' << two_digit(last_update_month) << '-' << two_digit(last_update_day);
    return stream.str();
}

DbfParseResult parse_dbf_header(const std::vector<std::uint8_t>& bytes) {
    if (bytes.size() < 32U) {
        return {.ok = false, .error = dbf_header_text("Vfp.DbfHeader.Error.ShortHeader")};
    }

    DbfHeader header;
    header.version = bytes[0];
    if (header.version == 0x02U) {
        // FoxBASE (dBASE II-compatible) predates the 32-byte dBASE III
        // header. Its main header is 8 bytes: version, a 2-byte record
        // count, then the last-update month/day/year (one byte each,
        // unlike dBASE III's year/month/day order), then a 2-byte record
        // length -- there is no stored header length, table-flags, or
        // code-page byte. The descriptor table (and therefore the start of
        // record data) is always a fixed 521 bytes regardless of how many
        // fields are actually declared.
        header.record_count = read_le_u16(bytes, 1U);
        header.last_update_month = bytes[3];
        header.last_update_day = bytes[4];
        header.last_update_year = bytes[5];
        header.header_length = 521U;
        header.record_length = read_le_u16(bytes, 6U);
    } else {
        header.last_update_year = bytes[1];
        header.last_update_month = bytes[2];
        header.last_update_day = bytes[3];
        header.record_count = read_le_u32(bytes, 4U);
        header.header_length = read_le_u16(bytes, 8U);
        header.record_length = read_le_u16(bytes, 10U);
        header.table_flags = bytes[28];
        header.code_page_mark = bytes[29];
    }

    if (!header.looks_like_dbf()) {
        return {.ok = false, .header = header, .error = dbf_header_text("Vfp.DbfHeader.Error.InvalidValues")};
    }

    return {.ok = true, .header = header, .error = {}};
}

DbfParseResult parse_dbf_header_from_file(const std::string& path) {
    std::ifstream input(copperfin::platform::path_from_utf8_string(path), std::ios::binary);
    if (!input) {
        return {.ok = false, .error = dbf_header_text("Vfp.DbfHeader.Error.OpenFileFailed")};
    }

    std::vector<std::uint8_t> bytes(32U, 0U);
    input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));

    if (input.gcount() < static_cast<std::streamsize>(bytes.size())) {
        return {.ok = false, .error = dbf_header_text("Vfp.DbfHeader.Error.ReadHeaderFailed")};
    }

    return parse_dbf_header(bytes);
}

std::optional<int> dbf_code_page_from_mark(std::uint8_t mark) {
    switch (mark) {
        case 0x01U:
            return 437;
        case 0x02U:
            return 850;
        case 0x03U:
            return 1252;
        case 0x04U:
            return 10000;
        case 0x64U:
            return 852;
        case 0x65U:
            return 866;
        case 0x66U:
            return 865;
        case 0x67U:
            return 861;
        case 0x68U:
            return 895;
        case 0x69U:
            return 620;
        case 0x6AU:
            return 737;
        case 0x6BU:
            return 857;
        case 0x78U:
            return 950;
        case 0x79U:
            return 949;
        case 0x7AU:
            return 936;
        case 0x7BU:
            return 932;
        case 0x7CU:
            return 874;
        case 0x7DU:
            return 1255;
        case 0x7EU:
            return 1256;
        case 0x96U:
            return 10007;
        case 0x97U:
            return 10029;
        case 0x98U:
            return 10006;
        case 0xC8U:
            return 1250;
        case 0xC9U:
            return 1251;
        case 0xCAU:
            return 1254;
        case 0xCBU:
            return 1253;
        default:
            return std::nullopt;
    }
}

}  // namespace copperfin::vfp
