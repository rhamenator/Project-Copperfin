// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/access_long_value.h"
#include "copperfin/vfp/access_table_definition.h"
#include "copperfin/platform/path.h"
#include "access_bytes_internal.h"

#include "copperfin/localization/localization.h"

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string_view>
#include <system_error>

namespace copperfin::vfp {

namespace {

using access_bytes_internal::read_le_u16;
using access_bytes_internal::read_le_u32;

localization::LocalizedCatalog access_long_value_catalog() {
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

std::string access_long_value_text(std::string_view key) {
    return access_long_value_catalog().translate(key, {});
}

constexpr std::size_t kFieldDescriptorSize = 12U;
constexpr std::uint8_t kBitmaskInline = 0x80U;
constexpr std::uint8_t kBitmaskSinglePage = 0x40U;
constexpr std::uint8_t kBitmaskChainedPages = 0x00U;

// A bounded guard against a corrupt or (maliciously) cyclic LVAL chain --
// the largest real chain verified during this slice's development was 15
// hops; this leaves generous headroom while still guaranteeing
// termination for adversarial input.
constexpr std::uint32_t kMaxChainHops = 100000U;

// One resolved row's byte span within an already-loaded LVAL page, using
// the same page/row-directory format access_msysobjects.cpp's row decoder
// already relies on for regular data pages (this codebase's own real-
// fixture-verified convention, not merely mdbtools' prose): the header
// carries num_rows at a generation-specific offset, followed immediately
// by that many 2-byte directory entries; a directory entry's low 14 bits
// are the row's own start offset, and rows are packed from the END of the
// page backward (row 0 runs to the page's last byte; each subsequent row
// ends where the previous row's directory-declared offset begins).
struct ResolvedLvalRow {
    bool ok = false;
    std::vector<std::uint8_t> bytes;
    std::string error;
};

ResolvedLvalRow resolve_lval_row(
    const std::vector<std::uint8_t>& page_bytes,
    bool is_jet3,
    std::uint32_t row_id) {
    ResolvedLvalRow result;
    if (page_bytes.size() < 8U || page_bytes[0] != 0x01U ||
        page_bytes[4] != 'L' || page_bytes[5] != 'V' || page_bytes[6] != 'A' || page_bytes[7] != 'L') {
        result.error = access_long_value_text("Vfp.AccessLongValue.Error.NotAnLvalPage");
        return result;
    }

    const std::size_t header_size = is_jet3 ? 10U : 14U;
    const std::size_t num_rows_offset = is_jet3 ? 8U : 12U;
    if (page_bytes.size() < header_size) {
        result.error = access_long_value_text("Vfp.AccessLongValue.Error.ReadPageFailed");
        return result;
    }
    const std::uint16_t num_rows = read_le_u16(page_bytes, num_rows_offset);
    if (row_id >= num_rows) {
        result.error = access_long_value_text("Vfp.AccessLongValue.Error.RowIndexOutOfRange");
        return result;
    }

    const std::size_t directory_size = static_cast<std::size_t>(num_rows) * 2U;
    if (header_size + directory_size > page_bytes.size()) {
        result.error = access_long_value_text("Vfp.AccessLongValue.Error.RowStructureInvalid");
        return result;
    }
    const std::size_t directory_end = header_size + directory_size;

    std::vector<std::uint16_t> row_offsets(num_rows);
    for (std::uint16_t index = 0U; index < num_rows; ++index) {
        const std::uint16_t raw_slot = read_le_u16(page_bytes, header_size + static_cast<std::size_t>(index) * 2U);
        row_offsets[index] = static_cast<std::uint16_t>(raw_slot & 0x3FFFU);
        if (row_offsets[index] < directory_end) {
            result.error = access_long_value_text("Vfp.AccessLongValue.Error.RowStructureInvalid");
            return result;
        }
    }

    const std::size_t row_offset = row_offsets[row_id];
    const std::size_t row_end =
        (row_id == 0U) ? (page_bytes.size() - 1U) : (static_cast<std::size_t>(row_offsets[row_id - 1U]) - 1U);
    if (row_offset > row_end || row_end >= page_bytes.size()) {
        result.error = access_long_value_text("Vfp.AccessLongValue.Error.RowStructureInvalid");
        return result;
    }

    result.bytes.assign(
        page_bytes.begin() + static_cast<std::ptrdiff_t>(row_offset),
        page_bytes.begin() + static_cast<std::ptrdiff_t>(row_end + 1U));
    result.ok = true;
    return result;
}

struct LvalPageReader {
    std::ifstream input;
    std::size_t page_size = 0U;
    std::uint64_t page_count = 0U;
    bool is_jet3 = false;

    [[nodiscard]] bool open_ok() const { return static_cast<bool>(input); }
};

std::optional<std::vector<std::uint8_t>> read_lval_page_bytes(LvalPageReader& reader, std::uint32_t page_number) {
    if (page_number >= reader.page_count) {
        return std::nullopt;
    }
    std::vector<std::uint8_t> page_bytes(reader.page_size);
    reader.input.seekg(
        static_cast<std::streamoff>(static_cast<std::uint64_t>(page_number) * reader.page_size), std::ios::beg);
    reader.input.read(reinterpret_cast<char*>(page_bytes.data()), static_cast<std::streamsize>(reader.page_size));
    if (static_cast<std::size_t>(reader.input.gcount()) < reader.page_size) {
        return std::nullopt;
    }
    return page_bytes;
}

}  // namespace

std::optional<AccessLongValueFieldDescriptor> parse_access_long_value_field_descriptor(
    const std::vector<std::uint8_t>& column_bytes) {
    if (column_bytes.size() < kFieldDescriptorSize) {
        return std::nullopt;
    }
    AccessLongValueFieldDescriptor descriptor;
    descriptor.declared_length = static_cast<std::uint32_t>(column_bytes[0]) |
        (static_cast<std::uint32_t>(column_bytes[1]) << 8U) |
        (static_cast<std::uint32_t>(column_bytes[2]) << 16U);
    descriptor.bitmask = column_bytes[3];
    descriptor.lval_dp = read_le_u32(column_bytes, 4U);
    return descriptor;
}

AccessLongValueResult read_access_long_value_column(
    const std::string& path,
    AccessContainerGeneration generation,
    const std::vector<std::uint8_t>& column_bytes) {
    AccessLongValueResult result;

    const std::optional<AccessLongValueFieldDescriptor> descriptor =
        parse_access_long_value_field_descriptor(column_bytes);
    if (!descriptor.has_value()) {
        result.error = access_long_value_text("Vfp.AccessLongValue.Error.DescriptorTooShort");
        return result;
    }

    if (descriptor->bitmask == kBitmaskInline) {
        const std::size_t available = column_bytes.size() - kFieldDescriptorSize;
        if (available != descriptor->declared_length) {
            result.error = access_long_value_text("Vfp.AccessLongValue.Error.InlineLengthMismatch");
            return result;
        }
        result.value.assign(
            column_bytes.begin() + static_cast<std::ptrdiff_t>(kFieldDescriptorSize), column_bytes.end());
        result.ok = true;
        return result;
    }

    if (descriptor->bitmask != kBitmaskSinglePage && descriptor->bitmask != kBitmaskChainedPages) {
        result.error = access_long_value_text("Vfp.AccessLongValue.Error.UnsupportedBitmask");
        return result;
    }

    const std::size_t page_size = access_container_page_size(generation);
    if (page_size == 0U) {
        result.error = access_long_value_text("Vfp.AccessLongValue.Error.UnknownGeneration");
        return result;
    }

    LvalPageReader reader;
    reader.page_size = page_size;
    reader.is_jet3 = (generation == AccessContainerGeneration::jet3);
    reader.input.open(platform::path_from_utf8_string(path), std::ios::binary);
    if (!reader.open_ok()) {
        result.error = access_long_value_text("Vfp.AccessLongValue.Error.OpenFileFailed");
        return result;
    }
    std::error_code file_size_error;
    const auto file_size = std::filesystem::file_size(platform::path_from_utf8_string(path), file_size_error);
    if (file_size_error || file_size < page_size || file_size % page_size != 0U) {
        result.error = access_long_value_text("Vfp.AccessLongValue.Error.ReadPageFailed");
        return result;
    }
    reader.page_count = file_size / page_size;

    std::uint32_t page_number = descriptor->lval_dp >> 8U;
    std::uint32_t row_id = descriptor->lval_dp & 0xFFU;

    if (descriptor->bitmask == kBitmaskSinglePage) {
        const std::optional<std::vector<std::uint8_t>> page_bytes = read_lval_page_bytes(reader, page_number);
        if (!page_bytes.has_value()) {
            result.error = access_long_value_text("Vfp.AccessLongValue.Error.ReadPageFailed");
            return result;
        }
        const ResolvedLvalRow row = resolve_lval_row(*page_bytes, reader.is_jet3, row_id);
        if (!row.ok) {
            result.error = row.error;
            return result;
        }
        if (row.bytes.size() != descriptor->declared_length) {
            result.error = access_long_value_text("Vfp.AccessLongValue.Error.InlineLengthMismatch");
            return result;
        }
        result.value = row.bytes;
        result.ok = true;
        return result;
    }

    // Chained (bitmask 0x00): each hop's row is a 4-byte "next" data
    // pointer followed by that hop's partial value bytes; 0 terminates
    // the chain.
    std::vector<std::uint8_t> collected;
    std::uint32_t hops = 0U;
    while (true) {
        if (++hops > kMaxChainHops) {
            result.error = access_long_value_text("Vfp.AccessLongValue.Error.ChainTooLong");
            return result;
        }
        const std::optional<std::vector<std::uint8_t>> page_bytes = read_lval_page_bytes(reader, page_number);
        if (!page_bytes.has_value()) {
            result.error = access_long_value_text("Vfp.AccessLongValue.Error.ReadPageFailed");
            return result;
        }
        const ResolvedLvalRow row = resolve_lval_row(*page_bytes, reader.is_jet3, row_id);
        if (!row.ok) {
            result.error = row.error;
            return result;
        }
        if (row.bytes.size() < 4U) {
            result.error = access_long_value_text("Vfp.AccessLongValue.Error.RowStructureInvalid");
            return result;
        }
        const std::uint32_t next_dp = read_le_u32(row.bytes, 0U);
        collected.insert(collected.end(), row.bytes.begin() + 4, row.bytes.end());
        if (next_dp == 0U) {
            break;
        }
        page_number = next_dp >> 8U;
        row_id = next_dp & 0xFFU;
    }

    if (collected.size() != descriptor->declared_length) {
        result.error = access_long_value_text("Vfp.AccessLongValue.Error.InlineLengthMismatch");
        return result;
    }
    result.value = std::move(collected);
    result.ok = true;
    return result;
}

}  // namespace copperfin::vfp
