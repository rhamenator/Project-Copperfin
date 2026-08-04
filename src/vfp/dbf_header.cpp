// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

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
    return version == 0x83U || version == 0x8BU || version == 0xF5U;
}

std::string DbfHeader::version_description() const {
    return version_description(dbf_header_catalog());
}

std::string DbfHeader::version_description(const localization::LocalizedCatalog& catalog) const {
    switch (version) {
        case 0x02U:
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

std::string DbfHeader::last_update_iso8601() const {
    const unsigned int year = 1900U + static_cast<unsigned int>(last_update_year);
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << year << '-' << two_digit(last_update_month) << '-' << two_digit(last_update_day);
    return stream.str();
}

DbfParseResult parse_dbf_header(const std::vector<std::uint8_t>& bytes) {
    if (bytes.size() < 32U) {
        return {.ok = false, .error = dbf_header_text("Vfp.DbfHeader.Error.ShortHeader")};
    }

    DbfHeader header;
    header.version = bytes[0];
    header.last_update_year = bytes[1];
    header.last_update_month = bytes[2];
    header.last_update_day = bytes[3];
    header.record_count = read_le_u32(bytes, 4U);
    header.header_length = read_le_u16(bytes, 8U);
    header.record_length = read_le_u16(bytes, 10U);
    header.table_flags = bytes[28];
    header.code_page_mark = bytes[29];

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
