// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/cdx_writer.h"
#include "copperfin/vfp/dbf_table.h"
#include "copperfin/platform/path.h"

#include "copperfin/localization/localization.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string_view>

namespace copperfin::vfp {

namespace {

localization::LocalizedCatalog cdx_writer_catalog() {
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

std::string cdx_writer_text(std::string_view key) {
    return cdx_writer_catalog().translate(key, {});
}

constexpr std::size_t kPageSize = 512U;

void write_le_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
}

void write_le_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

std::string uppercase_ascii_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::toupper(character));
    });
    return value;
}

std::string rtrim_copy(const std::string& value) {
    const auto end = value.find_last_not_of(' ');
    if (end == std::string::npos) {
        return {};
    }
    return value.substr(0, end + 1U);
}

struct ResolvedEntry {
    std::uint8_t record_number = 0;
    std::string key;  // trimmed, significant characters only
};

// The file header (page 0) -- confirmed byte-identical across every
// single-tag fixture this session's investigation generated,
// independent of record count or key content (docs/77).
std::vector<std::uint8_t> build_header_page(std::uint16_t key_length) {
    std::vector<std::uint8_t> page(kPageSize, 0U);
    write_le_u32(page, 0U, 1024U);  // root_node_offset -> page 2.
    write_le_u32(page, 4U, 0U);     // next_free_node_offset.
    write_le_u16(page, 12U, key_length);
    write_le_u16(page, 14U, 480U);  // key_pool_length_hint -- copied verbatim, see docs/77.
    // Trailing 16 bytes -- copied verbatim from every single-tag fixture
    // observed; not independently derived (docs/77).
    const std::vector<std::uint8_t> footer{0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0};
    std::copy(footer.begin(), footer.end(), page.begin() + 496);
    return page;
}

// The tag-table page (page 2) -- confirmed byte-identical across every
// single-tag fixture with the same tag name/key expression, independent
// of record count or key content (docs/77).
std::vector<std::uint8_t> build_tag_table_page(const std::string& tag_name_upper) {
    std::vector<std::uint8_t> page(kPageSize, 0U);
    write_le_u16(page, 0U, 0x0003U);
    write_le_u16(page, 2U, 0x0001U);
    write_le_u32(page, 4U, 0xFFFFFFFFU);
    write_le_u32(page, 8U, 0xFFFFFFFFU);
    write_le_u16(page, 12U, 0x01DEU);  // copied verbatim, see docs/77.
    write_le_u16(page, 14U, 0xFFFFU);
    page[16] = 0x00U;
    page[17] = 0x00U;
    page[18] = 0x0FU;
    page[19] = 0x0FU;
    page[20] = 0x10U;
    page[21] = 0x04U;
    page[22] = 0x04U;
    page[23] = 0x03U;
    write_le_u32(page, 24U, 1536U);  // target page -- always page 3 (tag stats) here.
    write_le_u32(page, 28U, 48U);    // copied verbatim, see docs/77.
    // Tag name, right-aligned to the page end (confirmed via two
    // additional real fixtures with different name lengths -- docs/77).
    const std::size_t name_start = kPageSize - tag_name_upper.size();
    std::copy(tag_name_upper.begin(), tag_name_upper.end(), page.begin() + static_cast<std::ptrdiff_t>(name_start));
    return page;
}

// The tag statistics page (page 3). offset[16:20) and [20:24) (total
// record/key count) are independently derived and confirmed exact;
// offset[24:36) is copied from the smallest-shaped observed fixture
// rather than independently derived (docs/77) -- the write-side
// verification test found this sufficient for real VFP9 to
// open/traverse/SEEK() correctly, though other operations (e.g.
// appending a new key) were not tested.
//
// The [496:512) footer was confirmed IDENTICAL ("05 00 01 00 00 00 05
// 00") across three real fixtures spanning different record counts (5,
// 5, 3) and different key content/compression -- ruling out both an
// earlier "last entry's control byte" theory and a "tracks record_count"
// theory. Treated as a true constant for the single-leaf-page case
// (copied verbatim), not independently explained further.
std::vector<std::uint8_t> build_tag_stats_page(std::uint16_t key_length, std::uint32_t record_count) {
    std::vector<std::uint8_t> page(kPageSize, 0U);
    write_le_u16(page, 0U, key_length);
    write_le_u16(page, 12U, key_length);
    write_le_u16(page, 14U, 0x0160U);  // copied verbatim, see docs/77.
    write_le_u32(page, 16U, record_count);
    write_le_u32(page, 20U, record_count);
    write_le_u32(page, 24U, 11U);  // copied verbatim, see docs/77.
    write_le_u32(page, 28U, 1U);   // copied verbatim, see docs/77.
    write_le_u32(page, 32U, 2U);   // copied verbatim, see docs/77.
    const std::vector<std::uint8_t> footer{0, 0, 5, 0, 1, 0, 0, 0, 5, 0};
    std::copy(footer.begin(), footer.end(), page.begin() + 502);
    return page;
}

std::vector<std::uint8_t> build_key_expression_page(const std::string& key_expression) {
    std::vector<std::uint8_t> page(kPageSize, 0U);
    const std::size_t length = std::min(key_expression.size(), kPageSize);
    std::copy(key_expression.begin(), key_expression.begin() + static_cast<std::ptrdiff_t>(length), page.begin());
    return page;
}

struct LeafBuildResult {
    bool ok = false;
    std::string error;
    std::vector<std::uint8_t> page;
};

// The leaf data page (page 5) -- the core discovery of this slice: the
// front-compression entry format, confirmed byte-exact against two real
// VFP9-generated fixtures and independently re-verified via a real-VFP9
// write-side round trip (docs/77).
LeafBuildResult build_leaf_page(const std::vector<ResolvedEntry>& entries) {
    LeafBuildResult result;
    const auto entry_count = static_cast<std::uint16_t>(entries.size());

    std::vector<std::uint8_t> entry_array;
    entry_array.reserve(entries.size() * 2U);
    std::vector<std::string> trailing_blocks;
    trailing_blocks.reserve(entries.size());
    std::string previous_key;
    for (const ResolvedEntry& entry : entries) {
        std::size_t shared = 0U;
        while (shared < previous_key.size() && shared < entry.key.size() &&
               previous_key[shared] == entry.key[shared]) {
            ++shared;
        }
        const std::string trailing = entry.key.substr(shared);
        const auto control_byte = static_cast<std::uint8_t>(
            ((entry.key.size() & 0x0FU) << 4U) | (shared & 0x0FU));
        entry_array.push_back(entry.record_number);
        entry_array.push_back(control_byte);
        trailing_blocks.push_back(trailing);
        previous_key = entry.key;
    }

    std::string text;
    for (auto it = trailing_blocks.rbegin(); it != trailing_blocks.rend(); ++it) {
        text += *it;
    }

    constexpr std::size_t kLeafHeaderSize = 24U;
    const std::size_t used = kLeafHeaderSize + entry_array.size() + text.size();
    if (used > kPageSize) {
        result.error = cdx_writer_text("Vfp.CdxWriter.Error.LeafPageOverflow");
        return result;
    }

    std::vector<std::uint8_t> page(kPageSize, 0U);
    write_le_u16(page, 0U, 0x0007U);  // copied verbatim, see docs/77.
    write_le_u16(page, 2U, entry_count);
    write_le_u32(page, 4U, 0xFFFFFFFFU);
    write_le_u32(page, 8U, 0xFFFFFFFFU);
    const auto free_space = static_cast<std::uint16_t>(kPageSize - used);
    write_le_u16(page, 12U, free_space);
    write_le_u16(page, 14U, 0x00FFU);  // copied verbatim, see docs/77.
    page[16] = 0x00U;
    page[17] = 0x00U;
    page[18] = 0x0FU;
    page[19] = 0x0FU;
    page[20] = 0x08U;
    page[21] = 0x04U;
    page[22] = 0x04U;
    page[23] = 0x02U;

    std::copy(entry_array.begin(), entry_array.end(), page.begin() + kLeafHeaderSize);
    const std::size_t text_start = kPageSize - text.size();
    std::copy(text.begin(), text.end(), page.begin() + static_cast<std::ptrdiff_t>(text_start));

    result.ok = true;
    result.page = std::move(page);
    return result;
}

}  // namespace

CdxWriteResult create_vfp_cdx_single_tag_index_file(
    const std::string& cdx_path,
    const std::string& dbf_path,
    const std::string& tag_name,
    const std::string& key_expression,
    std::uint16_t key_length,
    const std::vector<CdxIndexEntry>& entries) {
    CdxWriteResult result;

    // The tag name is stored right-aligned at the END of the 512-byte
    // tag-table page (page 2), but that same page's first 32 bytes hold
    // required header fields (docs/77's "Tag-table page" layout). A name
    // longer than kPageSize - 32 would overlap and corrupt those fields.
    constexpr std::size_t kTagTableHeaderSize = 32U;
    if (tag_name.empty() || tag_name.size() > (kPageSize - kTagTableHeaderSize)) {
        result.error = cdx_writer_text("Vfp.CdxWriter.Error.InvalidTagName");
        return result;
    }

    // cdx_header.cpp's own reader treats key_length == 0 or > page_size
    // as unparseable (it can't locate a tag directory at all in that
    // case), so a file built with such a value would be silently
    // unreadable by this codebase's own tooling, not just unusual.
    if (key_length == 0U || key_length > kPageSize) {
        result.error = cdx_writer_text("Vfp.CdxWriter.Error.InvalidKeyLength");
        return result;
    }

    // The key-expression page stores the expression verbatim starting
    // at byte 0 and relies on the page's own zero-initialization to
    // supply a NUL terminator immediately after it; an expression that
    // fills (or exceeds) the whole page would leave no such terminator
    // and silently drop everything past the page boundary.
    if (key_expression.size() >= kPageSize) {
        result.error = cdx_writer_text("Vfp.CdxWriter.Error.KeyExpressionTooLong");
        return result;
    }

    std::vector<ResolvedEntry> resolved;
    resolved.reserve(entries.size());
    for (const CdxIndexEntry& entry : entries) {
        // The leaf entry format's own record-number field is a single
        // byte (docs/77's "Known gaps"): validate the caller's wider
        // value BEFORE narrowing it, rather than narrowing first and
        // losing the out-of-range information.
        if (entry.record_number > 0xFFU) {
            result.error = cdx_writer_text("Vfp.CdxWriter.Error.RecordNumberOutOfRange");
            return result;
        }
        const std::string trimmed = rtrim_copy(entry.key_field_value);
        if (trimmed.size() > key_length) {
            result.error = cdx_writer_text("Vfp.CdxWriter.Error.KeyExceedsDeclaredLength");
            return result;
        }
        // The front-compression control byte packs this key's own
        // trimmed length into a 4-bit nibble (docs/77's "Front-
        // compression algorithm"), so any single key longer than 15
        // bytes cannot be represented and must fail closed rather than
        // silently wrap/truncate into a corrupt leaf page.
        if (trimmed.size() > 15U) {
            result.error = cdx_writer_text("Vfp.CdxWriter.Error.KeyLengthUnsupported");
            return result;
        }
        resolved.push_back({.record_number = static_cast<std::uint8_t>(entry.record_number), .key = trimmed});
    }
    std::sort(resolved.begin(), resolved.end(), [](const ResolvedEntry& left, const ResolvedEntry& right) {
        return left.key < right.key;
    });

    const LeafBuildResult leaf = build_leaf_page(resolved);
    if (!leaf.ok) {
        result.error = leaf.error;
        return result;
    }

    std::vector<std::uint8_t> file_bytes;
    file_bytes.reserve(kPageSize * 6U);
    const auto append_page = [&](const std::vector<std::uint8_t>& page) {
        file_bytes.insert(file_bytes.end(), page.begin(), page.end());
    };
    append_page(build_header_page(key_length));
    append_page(std::vector<std::uint8_t>(kPageSize, 0U));  // page 1: unused.
    append_page(build_tag_table_page(uppercase_ascii_copy(tag_name)));
    append_page(build_tag_stats_page(key_length, static_cast<std::uint32_t>(resolved.size())));
    append_page(build_key_expression_page(key_expression));
    append_page(leaf.page);

    // Staged write: build the complete file under a temporary sibling
    // name and only rename it over the real destination once every byte
    // is confirmed durable, so a mid-write failure (disk full, I/O
    // error) can never leave a truncated/partial CDX at `cdx_path`, and
    // never destroys a pre-existing usable CDX there in the process.
    const std::filesystem::path destination_path = platform::path_from_utf8_string(cdx_path);
    const std::filesystem::path temp_path = platform::path_from_utf8_string(cdx_path + ".cptmp");
    std::error_code remove_ec;
    std::filesystem::remove(temp_path, remove_ec);

    {
        std::ofstream output(temp_path, std::ios::binary | std::ios::trunc);
        if (!output) {
            result.error = cdx_writer_text("Vfp.CdxWriter.Error.OpenFileFailed");
            return result;
        }
        output.write(reinterpret_cast<const char*>(file_bytes.data()), static_cast<std::streamsize>(file_bytes.size()));
        output.flush();
        if (!output.good()) {
            std::filesystem::remove(temp_path, remove_ec);
            result.error = cdx_writer_text("Vfp.CdxWriter.Error.WriteFileFailed");
            return result;
        }
    }

    std::error_code rename_ec;
    std::filesystem::rename(temp_path, destination_path, rename_ec);
    if (rename_ec) {
        std::filesystem::remove(temp_path, remove_ec);
        result.error = cdx_writer_text("Vfp.CdxWriter.Error.WriteFileFailed");
        return result;
    }

    // Only now, with the new CDX durably in place, mark the table as
    // having a production index -- setting this flag before the CDX is
    // known-good would leave the pair inconsistent (flag set, index
    // missing or truncated) on any failure above.
    const DbfWriteResult flag_result = mark_dbf_table_has_production_index(dbf_path);
    if (!flag_result.ok) {
        result.error = flag_result.error;
        return result;
    }

    result.ok = true;
    return result;
}

}  // namespace copperfin::vfp
