// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace copperfin::localization {
struct LocalizedCatalog;
}

namespace copperfin::vfp {

// Stable inspection classification derived only from the DBF version byte.
// It identifies a file-format family; it does not assert runtime compatibility.
enum class DbfFormatFamily {
    unknown,
    foxbase,
    foxpro,
    dbase,
    visual_foxpro
};

struct DbfHeader {
    std::uint8_t version = 0;
    std::uint8_t last_update_year = 0;
    std::uint8_t last_update_month = 0;
    std::uint8_t last_update_day = 0;
    std::uint32_t record_count = 0;
    std::uint16_t header_length = 0;
    std::uint16_t record_length = 0;
    std::uint8_t table_flags = 0;
    std::uint8_t code_page_mark = 0;

    [[nodiscard]] bool looks_like_dbf() const;
    [[nodiscard]] bool has_database_container() const;
    [[nodiscard]] bool has_production_index() const;
    [[nodiscard]] bool has_structural_cdx() const;
    [[nodiscard]] bool has_memo_file() const;
    [[nodiscard]] DbfFormatFamily format_family() const;
    [[nodiscard]] std::string version_description() const;
    [[nodiscard]] std::string version_description(const localization::LocalizedCatalog& catalog) const;
    [[nodiscard]] std::string last_update_iso8601() const;
};

[[nodiscard]] const char* dbf_format_family_name(DbfFormatFamily family);

struct DbfParseResult {
    bool ok = false;
    DbfHeader header{};
    std::string error;
};

DbfParseResult parse_dbf_header(const std::vector<std::uint8_t>& bytes);
DbfParseResult parse_dbf_header_from_file(const std::string& path);
std::optional<int> dbf_code_page_from_mark(std::uint8_t mark);

}  // namespace copperfin::vfp
