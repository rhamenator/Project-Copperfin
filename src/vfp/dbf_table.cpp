// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/dbf_text_encoding.h"

#include "dbf_table_raw_mutation.h"

#include "copperfin/localization/localization.h"
#include "copperfin/platform/environment.h"
#include "copperfin/platform/exclusive_file.h"
#include "copperfin/platform/invariant_numeric.h"
#include "copperfin/platform/path.h"
#include "copperfin/vfp/sidecar_path.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <locale>
#include <mutex>
#include <optional>
#include <sstream>
#include <string_view>
#include <unordered_map>

namespace copperfin::vfp {
namespace {

std::uint32_t read_le_u32(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint32_t>(bytes[offset]) |
           (static_cast<std::uint32_t>(bytes[offset + 1]) << 8U) |
           (static_cast<std::uint32_t>(bytes[offset + 2]) << 16U) |
           (static_cast<std::uint32_t>(bytes[offset + 3]) << 24U);
}

std::uint32_t read_be_u32(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return (static_cast<std::uint32_t>(bytes[offset]) << 24U) |
           (static_cast<std::uint32_t>(bytes[offset + 1]) << 16U) |
           (static_cast<std::uint32_t>(bytes[offset + 2]) << 8U) |
           static_cast<std::uint32_t>(bytes[offset + 3]);
}

void write_le_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
}

std::uint16_t read_le_u16(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint16_t>(bytes[offset]) |
           static_cast<std::uint16_t>(static_cast<std::uint16_t>(bytes[offset + 1U]) << 8U);
}

std::uint16_t read_be_u16(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(bytes[offset]) << 8U) |
                                      static_cast<std::uint16_t>(bytes[offset + 1]));
}

void write_be_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>(value & 0xFFU);
}

void write_le_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

std::int64_t read_le_i64(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    std::uint64_t value = 0U;
    for (std::size_t index = 0; index < 8U; ++index) {
        value |= static_cast<std::uint64_t>(bytes[offset + index]) << (index * 8U);
    }
    return static_cast<std::int64_t>(value);
}

void write_le_i64(std::vector<std::uint8_t>& bytes, std::size_t offset, std::int64_t value) {
    const std::uint64_t raw = static_cast<std::uint64_t>(value);
    for (std::size_t index = 0; index < 8U; ++index) {
        bytes[offset + index] = static_cast<std::uint8_t>((raw >> (index * 8U)) & 0xFFU);
    }
}

void write_be_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>(value & 0xFFU);
}

std::string dbf_table_text(
    std::string_view key,
    const localization::PlaceholderMap& placeholders = {}) {
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
    return cache.catalog.translate(key, placeholders);
}

std::string trim_right(std::string text) {
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0) {
        text.pop_back();
    }
    return text;
}

std::string trim_dbf_text_terminator(std::string text) {
    if (const auto nul = std::find(text.begin(), text.end(), '\0'); nul != text.end()) {
        text.erase(nul, text.end());
    }
    return trim_right(std::move(text));
}

std::string trim_both(std::string text) {
    text = trim_right(std::move(text));
    const auto first = std::find_if(text.begin(), text.end(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    });
    text.erase(text.begin(), first);
    return text;
}

std::string lowercase_copy(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

std::string ascii_lowercase_copy(std::string text) {
    for (char& ch : text) {
        if (ch >= 'A' && ch <= 'Z') {
            ch = static_cast<char>(ch + ('a' - 'A'));
        }
    }
    return text;
}

constexpr std::size_t dbf_descriptor_name_width = 11U;
constexpr std::size_t dbf_free_table_field_name_max_bytes = 10U;

std::optional<std::string> free_table_field_name_length_error(const std::string& field_name) {
    if (field_name.size() <= dbf_free_table_field_name_max_bytes) {
        return std::nullopt;
    }
    return dbf_table_text(
        "Vfp.DbfTable.Error.FreeTableFieldNameTooLong",
        {
            {"fieldName", field_name},
            {"maxBytes", std::to_string(dbf_free_table_field_name_max_bytes)}
        });
}

std::string serialized_dbf_field_name(std::string name) {
    name = trim_both(std::move(name));
    if (name.size() > dbf_descriptor_name_width) {
        name.resize(dbf_descriptor_name_width);
    }
    return name;
}

std::string serialized_dbf_field_name_key(std::string name) {
    return ascii_lowercase_copy(serialized_dbf_field_name(std::move(name)));
}

std::string read_ascii_name(const std::vector<std::uint8_t>& bytes, std::size_t offset, std::size_t length) {
    std::string value;
    value.reserve(length);
    for (std::size_t index = 0; index < length && (offset + index) < bytes.size(); ++index) {
        const auto raw = bytes[offset + index];
        if (raw == 0U) {
            break;
        }
        value.push_back(static_cast<char>(raw));
    }
    return trim_right(std::move(value));
}

std::vector<std::uint8_t> read_binary_file(const std::string& path) {
    std::error_code ignored;
    const std::filesystem::path native_path = platform::path_from_utf8_string(path);
    if (!std::filesystem::is_regular_file(native_path, ignored)) {
        return {};
    }
    std::ifstream input(native_path, std::ios::binary);
    if (!input) {
        return {};
    }

    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

bool write_binary_file(const std::string& path, const std::vector<std::uint8_t>& bytes) {
    const auto should_inject_write_failure = [&path](const char* stage) {
        const auto marker =
            platform::read_environment_variable("COPPERFIN_TEST_FAIL_WRITE_PATH_CONTAINS");
        const auto stage_filter =
            platform::read_environment_variable("COPPERFIN_TEST_FAIL_WRITE_STAGE");
        if (!marker || !stage_filter) {
            return false;
        }
        if (marker->empty() || *stage_filter != stage) {
            return false;
        }
        if (path.find(*marker) == std::string::npos) {
            return false;
        }

        // The existing test-only fault seam normally fails every matching
        // write.  A positive ordinal limits it to one matching write, making
        // partial multi-write mutation paths reproducible without changing
        // any production write behavior.
        const auto match_number =
            platform::read_environment_variable("COPPERFIN_TEST_FAIL_WRITE_MATCH_NUMBER");
        if (!match_number || match_number->empty()) {
            return true;
        }
        char *end = nullptr;
        const unsigned long long requested_match = std::strtoull(match_number->c_str(), &end, 10);
        if (end == match_number->c_str() || *end != '\0' || requested_match == 0U ||
            requested_match > std::numeric_limits<std::size_t>::max()) {
            return false;
        }
        static std::atomic<std::size_t> matching_write_count{0U};
        const std::size_t actual_match = matching_write_count.fetch_add(1U) + 1U;
        return actual_match == static_cast<std::size_t>(requested_match);
    };

    const std::filesystem::path target_path = platform::path_from_utf8_string(path);
    const std::filesystem::path temp_path = platform::path_from_utf8_string(
        platform::path_to_utf8_string(target_path) + ".cptmp");
    const std::filesystem::path backup_path = platform::path_from_utf8_string(
        platform::path_to_utf8_string(target_path) + ".cpbak");

    std::error_code ec;
    std::filesystem::remove(temp_path, ec);
    std::filesystem::remove(backup_path, ec);

    if (should_inject_write_failure("temp-open")) {
        return false;
    }

    std::ofstream output(temp_path, std::ios::binary | std::ios::trunc);
    if (!output) {
        return false;
    }

    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!output) {
        std::filesystem::remove(temp_path, ec);
        return false;
    }

    output.close();
    if (!output) {
        std::filesystem::remove(temp_path, ec);
        return false;
    }

    const bool had_target = std::filesystem::exists(target_path, ec);
    if (should_inject_write_failure("before-backup")) {
        std::filesystem::remove(temp_path, ec);
        return false;
    }
    if (had_target) {
        std::filesystem::rename(target_path, backup_path, ec);
        if (ec) {
            std::filesystem::remove(temp_path, ec);
            return false;
        }
    }

    if (should_inject_write_failure("before-promote")) {
        if (had_target) {
            std::error_code restore_ec;
            std::filesystem::rename(backup_path, target_path, restore_ec);
        }
        std::filesystem::remove(temp_path, ec);
        return false;
    }

    std::filesystem::rename(temp_path, target_path, ec);
    if (ec) {
        if (had_target) {
            std::error_code restore_ec;
            std::filesystem::rename(backup_path, target_path, restore_ec);
        }
        std::filesystem::remove(temp_path, ec);
        return false;
    }

    if (had_target) {
        std::filesystem::remove(backup_path, ec);
    }
    return true;
}

std::string selected_sidecar_path(const SidecarPathResolution& resolution) {
    return platform::path_to_utf8_string(resolution.path.value_or(resolution.requested_path));
}

std::string ambiguous_sidecar_error(const SidecarPathResolution& resolution) {
    return dbf_table_text(
        "Vfp.Sidecar.Error.AmbiguousPath",
        {{"path", platform::path_to_utf8_string(resolution.requested_path)}});
}

bool primary_always_requires_memo_sidecar(const std::string& path) {
    const std::string extension = ascii_lowercase_copy(
        platform::path_to_utf8_string(platform::path_from_utf8_string(path).extension()));
    return extension == ".pjx" || extension == ".scx" || extension == ".vcx" ||
           extension == ".frx" || extension == ".lbx" || extension == ".mnx" ||
           extension == ".dbc";
}

SidecarPathResolution resolve_memo_sidecar_path(const std::string& path) {
    return resolve_vfp_memo_sidecar_path(platform::path_from_utf8_string(path));
}

std::optional<std::string> ambiguous_required_sidecar_error_for_path(const std::string& path) {
    if (!primary_always_requires_memo_sidecar(path)) {
        return std::nullopt;
    }
    const SidecarPathResolution resolution = resolve_memo_sidecar_path(path);
    if (!resolution.ambiguous) {
        return std::nullopt;
    }
    return ambiguous_sidecar_error(resolution);
}

struct RawFieldDescriptor {
    std::string name;
    char type = '\0';
    std::uint32_t offset = 0;
    std::uint8_t length = 0;
    std::uint8_t decimal_count = 0;
};

enum class DbfMemoStorageFormat {
    visual_foxpro,
    dbase_iii,
    dbase_iv,
    // Classic FoxPro (2.x and earlier) stores its FPT header and memo
    // blocks in the same big-endian, length-prefixed layout VFP inherited
    // unchanged, but the in-record memo/General pointer is still the
    // dBASE-style right-justified ASCII decimal text used by FoxBASE/
    // FoxPro, not VFP's 4-byte little-endian binary pointer. Verified
    // against the `dbase_f5.dbf`/`dbase_f5.fpt` fixture (see docs/32):
    // its 'M' field's on-disk bytes read as ASCII digits, and the
    // referenced FPT block begins with the same 4-byte-type/4-byte-length
    // big-endian header VFP uses.
    foxpro
};

struct DbfReadLayout {
    std::size_t descriptor_start = 32U;
    std::size_t descriptor_size = 32U;
    std::size_t descriptor_name_width = 11U;
    std::size_t descriptor_type_offset = 11U;
    std::size_t descriptor_length_offset = 16U;
    std::size_t descriptor_decimal_count_offset = 17U;
    bool uses_physical_field_offsets = true;
    DbfMemoStorageFormat memo_storage_format = DbfMemoStorageFormat::visual_foxpro;
};

bool is_memo_pointer_field(char field_type);

DbfReadLayout dbf_read_layout(const DbfHeader& header) {
    if (header.format_family() == DbfFormatFamily::foxpro) {
        // Classic FoxPro's field descriptors already store the correct
        // on-disk physical field offset the same way VFP does, so only the
        // memo pointer encoding needs to differ from the VFP default.
        return {.memo_storage_format = DbfMemoStorageFormat::foxpro};
    }
    if (header.version == 0x02U) {
        // FoxBASE (dBASE II-compatible) predates the 32-byte dBASE III
        // header entirely: an 8-byte main header is followed by fixed
        // 16-byte descriptors (11-byte name, 1-byte type, 1-byte length,
        // a 2-byte in-memory field-address slot at bytes 13-14 that is
        // meaningless on disk, then a 1-byte decimal count) -- fields are
        // packed sequentially with no on-disk offset.
        return {
            .descriptor_start = 8U,
            .descriptor_size = 16U,
            .descriptor_name_width = 11U,
            .descriptor_type_offset = 11U,
            .descriptor_length_offset = 12U,
            .descriptor_decimal_count_offset = 15U,
            .uses_physical_field_offsets = false
        };
    }
    if (header.version == 0xFBU) {
        // 0xFB's true physical layout was never confirmed by any primary
        // source -- see docs/70-foxbase-0xfb-investigation.md for the
        // full research trail. The community table this value ultimately
        // traces back to (clicketyclick.dk) marks it "FoxPro ???": an
        // acknowledged unknown by its own original compiler, not a
        // documented fact, despite roughly two decades of secondary
        // sources repeating it with increasing (and unwarranted)
        // confidence. Every source that ventured an actual guess at its
        // meaning, rather than just copying the "???", pointed toward
        // the FoxBASE/FoxPro-2.x-without-memo lineage -- the same
        // lineage as the already-verified `0x03` byte below -- so this
        // uses that same sequential-packing, non-physical-offset layout
        // rather than silently falling through to the raw VFP-native
        // default (trusted stored offsets, binary memo pointers) that
        // otherwise applies here and matches none of the converging
        // guesses. This is a documented best-effort choice, not a
        // verified one; revise it if a real specimen ever surfaces.
        return {
            .uses_physical_field_offsets = false,
            .memo_storage_format = DbfMemoStorageFormat::dbase_iii
        };
    }
    if (header.format_family() != DbfFormatFamily::dbase) {
        return {};
    }

    // dBASE III/IV descriptors retain a four-byte in-memory field-address
    // slot, not the on-disk offset used by VFP. Physical fields are packed
    // sequentially after the record deletion marker. dBASE 7 moves its
    // descriptors behind its 68-byte fixed header and expands them to 48
    // bytes; its descriptor contains no on-disk field offset at all.
    if ((header.version & 0x07U) == 0x04U) {
        return {
            .descriptor_start = 68U,
            .descriptor_size = 48U,
            .descriptor_name_width = 32U,
            .descriptor_type_offset = 32U,
            .descriptor_length_offset = 33U,
            .descriptor_decimal_count_offset = 34U,
            .uses_physical_field_offsets = false,
            .memo_storage_format = DbfMemoStorageFormat::dbase_iv
        };
    }

    return {
        .uses_physical_field_offsets = false,
        .memo_storage_format = header.version == 0x83U
            ? DbfMemoStorageFormat::dbase_iii
            : DbfMemoStorageFormat::dbase_iv
    };
}

SidecarPathResolution resolve_memo_sidecar_path(
    const std::string& path,
    const DbfHeader& header) {
    if (header.format_family() != DbfFormatFamily::dbase) {
        return resolve_memo_sidecar_path(path);
    }
    std::filesystem::path candidate = platform::path_from_utf8_string(path);
    candidate.replace_extension(".dbt");
    return resolve_unique_casefold_path(candidate);
}

std::optional<std::uint32_t> parse_memo_block_number(
    const std::vector<std::uint8_t>& raw,
    DbfMemoStorageFormat storage_format) {
    if (storage_format == DbfMemoStorageFormat::visual_foxpro) {
        return raw.size() >= 4U ? std::optional<std::uint32_t>(read_le_u32(raw, 0U)) : std::nullopt;
    }

    const std::string text = trim_both(std::string(raw.begin(), raw.end()));
    if (text.empty()) {
        return 0U;
    }
    if (text.find_first_not_of("0123456789") != std::string::npos) {
        return std::nullopt;
    }
    try {
        const unsigned long value = std::stoul(text);
        if (value > std::numeric_limits<std::uint32_t>::max()) {
            return std::nullopt;
        }
        return static_cast<std::uint32_t>(value);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::vector<std::uint32_t> collect_referenced_memo_blocks(
    const std::vector<std::uint8_t>& table_bytes,
    const DbfHeader& header,
    const std::vector<DbfFieldDescriptor>& fields,
    DbfMemoStorageFormat storage_format) {
    if (storage_format == DbfMemoStorageFormat::visual_foxpro ||
        storage_format == DbfMemoStorageFormat::foxpro ||
        header.record_length == 0U ||
        table_bytes.size() < header.header_length) {
        return {};
    }

    const std::size_t available_record_count =
        (table_bytes.size() - header.header_length) / header.record_length;
    const std::size_t record_count = std::min<std::size_t>(header.record_count, available_record_count);
    std::vector<std::uint32_t> block_numbers;
    for (std::size_t record_index = 0U; record_index < record_count; ++record_index) {
        const std::size_t record_offset = header.header_length + (record_index * header.record_length);
        for (const auto& field : fields) {
            if (field.type != 'B' && !is_memo_pointer_field(field.type)) {
                continue;
            }

            const std::size_t field_start = record_offset + field.offset;
            const std::size_t field_end = field_start + field.length;
            if (field_end > table_bytes.size()) {
                break;
            }

            const std::optional<std::uint32_t> block_number = parse_memo_block_number(
                std::vector<std::uint8_t>{
                    table_bytes.begin() + static_cast<std::ptrdiff_t>(field_start),
                    table_bytes.begin() + static_cast<std::ptrdiff_t>(field_end)
                },
                storage_format);
            if (block_number.has_value() && *block_number != 0U) {
                block_numbers.push_back(*block_number);
            }
        }
    }

    std::sort(block_numbers.begin(), block_numbers.end());
    block_numbers.erase(std::unique(block_numbers.begin(), block_numbers.end()), block_numbers.end());
    return block_numbers;
}

std::vector<RawFieldDescriptor> read_raw_field_descriptors(const std::vector<std::uint8_t>& table_bytes) {
    std::vector<RawFieldDescriptor> fields;
    std::size_t descriptor_offset = 32U;
    while ((descriptor_offset + 32U) <= table_bytes.size() && table_bytes[descriptor_offset] != 0x0DU) {
        fields.push_back({
            .name = read_ascii_name(table_bytes, descriptor_offset, dbf_descriptor_name_width),
            .type = static_cast<char>(table_bytes[descriptor_offset + dbf_descriptor_name_width]),
            .offset = read_le_u32(table_bytes, descriptor_offset + 12U),
            .length = table_bytes[descriptor_offset + 16U],
            .decimal_count = table_bytes[descriptor_offset + 17U]
        });
        descriptor_offset += 32U;
    }
    return fields;
}

std::optional<char> normalize_logical_value(std::string value) {
    value = trim_both(std::move(value));
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });

    if (value.empty() || value == "null" || value == "?") {
        return '?';
    }
    if (value == "t" || value == "true" || value == "y" || value == "yes" || value == ".t.") {
        return 'T';
    }
    if (value == "f" || value == "false" || value == "n" || value == "no" || value == ".f.") {
        return 'F';
    }
    return std::nullopt;
}

std::optional<RawFieldDescriptor> find_raw_field(
    const std::vector<RawFieldDescriptor>& fields,
    const std::string& field_name) {
    const std::string normalized = ascii_lowercase_copy(trim_both(field_name));
    std::optional<RawFieldDescriptor> match;
    for (const auto& field : fields) {
        if (ascii_lowercase_copy(trim_both(field.name)) != normalized) {
            continue;
        }
        if (match.has_value()) {
            return std::nullopt;
        }
        match = field;
    }
    return match;
}

bool supports_direct_field_writes(char field_type) {
    return field_type == 'C' || field_type == 'N' || field_type == 'F' || field_type == 'L' || field_type == 'D' || field_type == 'B' || field_type == 'I' ||
           field_type == 'Y' || field_type == 'T' || field_type == 'V' || field_type == 'Q';
}

bool is_memo_pointer_field(char field_type) {
    return field_type == 'M' || field_type == 'G' || field_type == 'P';
}

template <typename FieldDescriptor>
bool table_uses_memo_sidecar(
    const DbfHeader& header,
    const std::vector<FieldDescriptor>& fields) {
    return header.has_memo_file() ||
        std::any_of(fields.begin(), fields.end(), [](const FieldDescriptor& field) {
            return is_memo_pointer_field(field.type);
        });
}

template <typename FieldDescriptor>
std::optional<std::string> ambiguous_table_sidecar_error(
    const std::string& path,
    const DbfHeader& header,
    const std::vector<FieldDescriptor>& fields) {
    if (!table_uses_memo_sidecar(header, fields)) {
        return std::nullopt;
    }
    const SidecarPathResolution resolution = resolve_memo_sidecar_path(path);
    if (!resolution.ambiguous) {
        return std::nullopt;
    }
    return ambiguous_sidecar_error(resolution);
}

bool supports_table_field_storage(char field_type) {
    return supports_direct_field_writes(field_type) || is_memo_pointer_field(field_type);
}

std::optional<std::vector<std::uint8_t>> parse_opaque_field_bytes(const std::string& value, std::size_t field_length) {
    const std::string trimmed = trim_both(value);
    if (trimmed.empty()) {
        return std::vector<std::uint8_t>(field_length, 0U);
    }

    if (trimmed.size() >= 2U && trimmed[0U] == '0' && (trimmed[1U] == 'x' || trimmed[1U] == 'X')) {
        const std::string hex = trimmed.substr(2U);
        if ((hex.size() % 2U) != 0U || (hex.size() / 2U) > field_length) {
            return std::nullopt;
        }
        std::vector<std::uint8_t> bytes(field_length, 0U);
        for (std::size_t index = 0U; index < hex.size(); index += 2U) {
            const auto high = static_cast<unsigned char>(hex[index]);
            const auto low = static_cast<unsigned char>(hex[index + 1U]);
            if (std::isxdigit(high) == 0 || std::isxdigit(low) == 0) {
                return std::nullopt;
            }
            bytes[index / 2U] = static_cast<std::uint8_t>(std::stoi(hex.substr(index, 2U), nullptr, 16));
        }
        return bytes;
    }

    if (value.size() > field_length) {
        return std::nullopt;
    }

    std::vector<std::uint8_t> bytes(field_length, 0U);
    for (std::size_t index = 0U; index < value.size(); ++index) {
        bytes[index] = static_cast<std::uint8_t>(value[index]);
    }
    return bytes;
}

std::vector<std::uint8_t> create_empty_memo_sidecar(std::uint16_t block_size = 512U) {
    std::vector<std::uint8_t> bytes(block_size, 0U);
    write_be_u32(bytes, 0U, 1U);
    write_be_u16(bytes, 6U, block_size);
    return bytes;
}

std::optional<std::int64_t> parse_scaled_currency_value(const std::string& value) {
    std::string text = trim_both(value);
    if (text.empty()) {
        return static_cast<std::int64_t>(0);
    }

    bool negative = false;
    if (!text.empty() && (text.front() == '+' || text.front() == '-')) {
        negative = text.front() == '-';
        text.erase(text.begin());
    }
    if (text.empty()) {
        return std::nullopt;
    }

    const std::size_t dot = text.find('.');
    if (dot != text.rfind('.')) {
        return std::nullopt;
    }

    std::string whole_text = dot == std::string::npos ? text : text.substr(0U, dot);
    std::string frac_text = dot == std::string::npos ? std::string{} : text.substr(dot + 1U);
    if (whole_text.empty()) {
        whole_text = "0";
    }
    if (whole_text.find_first_not_of("0123456789") != std::string::npos ||
        frac_text.find_first_not_of("0123456789") != std::string::npos ||
        frac_text.size() > 4U) {
        return std::nullopt;
    }

    long long whole = 0;
    try {
        whole = std::stoll(whole_text);
    } catch (const std::exception&) {
        return std::nullopt;
    }

    while (frac_text.size() < 4U) {
        frac_text.push_back('0');
    }

    long long fractional = 0;
    try {
        fractional = frac_text.empty() ? 0LL : std::stoll(frac_text);
    } catch (const std::exception&) {
        return std::nullopt;
    }

    constexpr std::uint64_t scale = 10000U;
    const std::uint64_t magnitude_limit = negative
        ? static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) + 1U
        : static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());
    const std::uint64_t whole_magnitude = static_cast<std::uint64_t>(whole);
    const std::uint64_t fractional_magnitude = static_cast<std::uint64_t>(fractional);
    if (whole_magnitude > magnitude_limit / scale) {
        return std::nullopt;
    }
    const std::uint64_t scaled_whole = whole_magnitude * scale;
    if (fractional_magnitude > magnitude_limit - scaled_whole) {
        return std::nullopt;
    }
    const std::uint64_t scaled_magnitude = scaled_whole + fractional_magnitude;
    if (negative) {
        if (scaled_magnitude == magnitude_limit) {
            return std::numeric_limits<std::int64_t>::min();
        }
        return -static_cast<std::int64_t>(scaled_magnitude);
    }
    return static_cast<std::int64_t>(scaled_magnitude);
}

std::optional<std::pair<std::uint32_t, std::uint32_t>> parse_datetime_storage_value(const std::string& value) {
    const std::string text = trim_both(value);
    if (text.empty()) {
        return std::pair<std::uint32_t, std::uint32_t>{0U, 0U};
    }

    const std::string lowered = lowercase_copy(text);
    constexpr const char* julian_prefix = "julian:";
    constexpr const char* millis_prefix = "millis:";
    if (lowered.rfind(julian_prefix, 0U) != 0U) {
        return std::nullopt;
    }

    const std::size_t millis_pos = lowered.find(millis_prefix);
    if (millis_pos == std::string::npos || millis_pos <= 7U) {
        return std::nullopt;
    }

    const std::string julian_text = trim_both(text.substr(7U, millis_pos - 7U));
    const std::string millis_text = trim_both(text.substr(millis_pos + 7U));
    if (julian_text.empty() || millis_text.empty()) {
        return std::nullopt;
    }

    std::size_t consumed = 0U;
    unsigned long julian = 0;
    unsigned long millis = 0;
    try {
        julian = std::stoul(julian_text, &consumed, 10);
        if (consumed != julian_text.size()) {
            return std::nullopt;
        }
        millis = std::stoul(millis_text, &consumed, 10);
        if (consumed != millis_text.size()) {
            return std::nullopt;
        }
    } catch (const std::exception&) {
        return std::nullopt;
    }

    if (julian > std::numeric_limits<std::uint32_t>::max() || millis > std::numeric_limits<std::uint32_t>::max()) {
        return std::nullopt;
    }
    return std::pair<std::uint32_t, std::uint32_t>{
        static_cast<std::uint32_t>(julian),
        static_cast<std::uint32_t>(millis)
    };
}

std::string format_currency_display_value(std::int64_t scaled) {
    const bool negative = scaled < 0;
    const std::uint64_t magnitude = negative
        ? static_cast<std::uint64_t>(-(scaled + 1)) + 1U
        : static_cast<std::uint64_t>(scaled);

    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    if (negative && magnitude != 0U) {
        stream << '-';
    }
    stream << (magnitude / 10000U) << '.';
    const std::uint64_t fractional = magnitude % 10000U;
    stream.width(4);
    stream.fill('0');
    stream << fractional;
    return stream.str();
}

DbfWriteResult write_memo_field_bytes(
    std::vector<std::uint8_t>& table_bytes,
    std::size_t field_offset,
    std::vector<std::uint8_t>& memo_bytes,
    const std::vector<std::uint8_t>& value,
    std::size_t record_count,
    bool preserve_empty_payload_block) {
    if ((field_offset + 4U) > table_bytes.size()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordDataTruncated"), .record_count = record_count};
    }

    if (value.empty() && !preserve_empty_payload_block) {
        write_le_u32(table_bytes, field_offset, 0U);
        return {.ok = true, .error = {}, .record_count = record_count};
    }

    if (memo_bytes.size() < 8U) {
        memo_bytes = create_empty_memo_sidecar();
    }

    std::uint16_t block_size = read_be_u16(memo_bytes, 6U);
    if (block_size == 0U) {
        block_size = 512U;
        if (memo_bytes.size() < block_size) {
            memo_bytes.resize(block_size, 0U);
        }
        write_be_u16(memo_bytes, 6U, block_size);
    }

    std::uint32_t next_free_block = read_be_u32(memo_bytes, 0U);
    if (next_free_block == 0U) {
        next_free_block = 1U;
        write_be_u32(memo_bytes, 0U, next_free_block);
    }

    const auto required_bytes = static_cast<std::size_t>(8U + value.size());
    const auto required_blocks = static_cast<std::uint32_t>((required_bytes + block_size - 1U) / block_size);
    const std::size_t block_offset = static_cast<std::size_t>(next_free_block) * block_size;
    const std::size_t new_total_size = block_offset + (static_cast<std::size_t>(required_blocks) * block_size);
    if (memo_bytes.size() < new_total_size) {
        memo_bytes.resize(new_total_size, 0U);
    }

    for (std::size_t index = 0; index < 4U; ++index) {
        memo_bytes[block_offset + index] = 0U;
    }
    memo_bytes[block_offset + 3U] = 1U;
    write_be_u32(memo_bytes, block_offset + 4U, static_cast<std::uint32_t>(value.size()));
    std::fill(
        memo_bytes.begin() + static_cast<std::ptrdiff_t>(block_offset + 8U),
        memo_bytes.begin() + static_cast<std::ptrdiff_t>(new_total_size),
        static_cast<std::uint8_t>(0U));
    std::copy(
        value.begin(),
        value.end(),
        memo_bytes.begin() + static_cast<std::ptrdiff_t>(block_offset + 8U));

    write_be_u32(memo_bytes, 0U, next_free_block + required_blocks);
    write_le_u32(table_bytes, field_offset, next_free_block);
    return {.ok = true, .error = {}, .record_count = record_count};
}

DbfWriteResult write_memo_field_text(
    std::vector<std::uint8_t>& table_bytes,
    const DbfHeader& header,
    std::size_t field_offset,
    std::vector<std::uint8_t>& memo_bytes,
    const std::string& value,
    std::size_t record_count) {
    const DbfTextConversionResult encoded = encode_dbf_text(header.code_page_mark, value);
    if (!encoded.ok) {
        return {
            .ok = false,
            .error = dbf_table_text("Vfp.DbfTable.Error.TextEncodingConversionFailed"),
            .record_count = record_count
        };
    }
    return write_memo_field_bytes(
        table_bytes,
        field_offset,
        memo_bytes,
        std::vector<std::uint8_t>(encoded.text.begin(), encoded.text.end()),
        record_count,
        false);
}

namespace {

// A bounded preview of an offending value for an error message: long values
// are cut short (with an explicit marker) so the message stays readable,
// while still giving enough of the actual data for a user to grep the
// source and find the offending record.
std::string summarize_value_for_diagnostic(const std::string& value, std::size_t max_chars = 80U) {
    if (value.size() <= max_chars) {
        return value;
    }
    return value.substr(0U, max_chars) + "...(" + std::to_string(value.size()) + " chars total)";
}

}  // namespace

DbfWriteResult write_field_bytes(
    std::vector<std::uint8_t>& table_bytes,
    const DbfHeader& header,
    std::size_t record_index,
    const RawFieldDescriptor& field,
    const std::string& value,
    const std::string& table_path,
    bool allow_truncation = false,
    // The targeted REPLACE fast path feeds this a single-record buffer
    // (record_index always 0 for that buffer's own offset math) but still
    // needs error messages to name the real record number being replaced;
    // when set, this overrides record_index for diagnostic text only.
    std::optional<std::size_t> diagnostic_record_index = std::nullopt) {
    if (record_index >= header.record_count) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordIndexOutOfRange"), .record_count = header.record_count};
    }
    const std::size_t reported_record_index = diagnostic_record_index.value_or(record_index);

    const std::size_t record_offset = header.header_length + (record_index * header.record_length);
    const std::size_t field_offset = record_offset + field.offset;
    if ((field_offset + field.length) > table_bytes.size()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordDataTruncated"), .record_count = header.record_count};
    }

    std::fill_n(table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset), field.length, static_cast<std::uint8_t>(' '));

    const bool is_string_field = field.type == 'C' || field.type == 'V' || field.type == 'Q';
    const std::string normalized_value = is_string_field ? std::string{} : lowercase_copy(trim_both(value));
    const bool is_null_token = normalized_value == "null";

    switch (field.type) {
        case 'C': {
            const DbfTextConversionResult encoded = encode_dbf_text(
                header.code_page_mark,
                trim_right(value));
            if (!encoded.ok) {
                return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.TextEncodingConversionFailed"), .record_count = header.record_count};
            }
            if (encoded.text.size() > field.length) {
                if (allow_truncation) {
                    std::copy(
                        encoded.text.begin(),
                        encoded.text.begin() + static_cast<std::ptrdiff_t>(field.length),
                        table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset));
                    break;
                }
                return {
                    .ok = false,
                    .error = dbf_table_text(
                        "Vfp.DbfTable.Error.CharacterValueTooLarge",
                        {
                            {"path", table_path},
                            {"recordNumber", std::to_string(reported_record_index + 1U)},
                            {"fieldName", field.name},
                            {"fieldLength", std::to_string(field.length)},
                            {"valueLength", std::to_string(encoded.text.size())},
                            {"valuePreview", summarize_value_for_diagnostic(value)},
                        }),
                    .record_count = header.record_count};
            }
            std::copy(
                encoded.text.begin(),
                encoded.text.end(),
                table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset));
            break;
        }
        case 'N':
        case 'F': {
            if (is_null_token) {
                break;
            }
            const std::string text = trim_both(value);
            if (!text.empty()) {
                if (text.size() > field.length) {
                    if (allow_truncation) {
                        // Legacy dBASE/FoxPro numeric-overflow convention: fill the
                        // field with asterisks rather than silently keeping a
                        // truncated (and therefore wrong) numeric value -- cutting
                        // digits off a number changes its magnitude in a way that
                        // cutting characters off a string does not, so this is
                        // deliberately not the same "just cut it to fit" behavior
                        // as the character-field case above.
                        std::fill_n(
                            table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset),
                            field.length,
                            static_cast<std::uint8_t>('*'));
                        break;
                    }
                    return {
                        .ok = false,
                        .error = dbf_table_text(
                            "Vfp.DbfTable.Error.NumericValueTooLarge",
                            {
                                {"path", table_path},
                                {"recordNumber", std::to_string(reported_record_index + 1U)},
                                {"fieldName", field.name},
                                {"fieldLength", std::to_string(field.length)},
                                {"valueLength", std::to_string(text.size())},
                                {"valuePreview", summarize_value_for_diagnostic(value)},
                            }),
                        .record_count = header.record_count};
                }
                const auto padding = static_cast<std::ptrdiff_t>(field.length - text.size());
                std::copy(text.begin(), text.end(), table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset) + padding);
            }
            break;
        }
        case 'L': {
            if (is_null_token) {
                table_bytes[field_offset] = static_cast<std::uint8_t>('?');
                break;
            }
            const auto logical_value = normalize_logical_value(value);
            if (!logical_value.has_value()) {
                return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.LogicalValueInvalid"), .record_count = header.record_count};
            }
            table_bytes[field_offset] = static_cast<std::uint8_t>(*logical_value);
            break;
        }
        case 'D': {
            if (is_null_token) {
                break;
            }
            std::string text = trim_both(value);
            text.erase(std::remove(text.begin(), text.end(), '-'), text.end());
            if (!text.empty() && text.size() != 8U) {
                return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.DateValueInvalid"), .record_count = header.record_count};
            }
            std::copy(text.begin(), text.end(), table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset));
            break;
        }
        case 'B': {
            if (is_null_token) {
                std::fill_n(
                    table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset),
                    field.length,
                    static_cast<std::uint8_t>(0U));
                break;
            }
            const std::string text = trim_both(value);
            double parsed = 0.0;
            if (!text.empty()) {
                const auto parsed_value = copperfin::platform::try_parse_invariant_double(text, true);
                if (!parsed_value.has_value()) {
                    return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.DoubleValueInvalid"), .record_count = header.record_count};
                }
                parsed = *parsed_value;
            }

            std::array<std::uint8_t, 8U> raw{};
            std::memcpy(raw.data(), &parsed, raw.size());
            std::copy(raw.begin(), raw.end(), table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset));
            break;
        }
        case 'V':
        case 'Q': {
            if (field.length < 2U) {
                return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.VqFieldWidthTooSmall"), .record_count = header.record_count};
            }

            std::fill_n(
                table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset),
                field.length,
                static_cast<std::uint8_t>(0U));
            if (is_null_token) {
                table_bytes[field_offset + field.length - 1U] = 0U;
                break;
            }

            std::string text = value;
            if (field.type == 'V') {
                text = trim_right(std::move(text));
            }

            const std::size_t payload_capacity = static_cast<std::size_t>(field.length - 1U);
            if (text.size() > payload_capacity) {
                if (allow_truncation) {
                    text.resize(payload_capacity);
                } else {
                    return {
                        .ok = false,
                        .error = dbf_table_text(
                            "Vfp.DbfTable.Error.VqValueTooLarge",
                            {
                                {"path", table_path},
                                {"recordNumber", std::to_string(reported_record_index + 1U)},
                                {"fieldName", field.name},
                                {"fieldLength", std::to_string(payload_capacity)},
                                {"valueLength", std::to_string(text.size())},
                                {"valuePreview", summarize_value_for_diagnostic(value)},
                            }),
                        .record_count = header.record_count};
                }
            }

            std::copy(text.begin(), text.end(), table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset));
            table_bytes[field_offset + field.length - 1U] = static_cast<std::uint8_t>(text.size());
            break;
        }
        case 'I': {
            if (is_null_token) {
                write_le_u32(table_bytes, field_offset, 0U);
                break;
            }
            const std::string text = trim_both(value);
            if (text.empty()) {
                write_le_u32(table_bytes, field_offset, 0U);
                break;
            }

            std::size_t consumed = 0U;
            long long parsed = 0;
            try {
                parsed = std::stoll(text, &consumed, 10);
            } catch (const std::exception&) {
                return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.IntegerValueInvalid"), .record_count = header.record_count};
            }
            if (consumed != text.size()) {
                return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.IntegerValueInvalid"), .record_count = header.record_count};
            }
            if (parsed < static_cast<long long>(std::numeric_limits<std::int32_t>::min()) ||
                parsed > static_cast<long long>(std::numeric_limits<std::int32_t>::max())) {
                return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.IntegerValueTooLarge"), .record_count = header.record_count};
            }

            write_le_u32(table_bytes, field_offset, static_cast<std::uint32_t>(static_cast<std::int32_t>(parsed)));
            break;
        }
        case 'Y': {
            if (is_null_token) {
                write_le_i64(table_bytes, field_offset, 0);
                break;
            }
            const auto scaled = parse_scaled_currency_value(value);
            if (!scaled.has_value()) {
                return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.CurrencyValueInvalid"), .record_count = header.record_count};
            }
            write_le_i64(table_bytes, field_offset, *scaled);
            break;
        }
        case 'T': {
            if (is_null_token) {
                write_le_u32(table_bytes, field_offset, 0U);
                write_le_u32(table_bytes, field_offset + 4U, 0U);
                break;
            }
            const auto datetime = parse_datetime_storage_value(value);
            if (!datetime.has_value()) {
                return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.DateTimeValueInvalid"), .record_count = header.record_count};
            }
            write_le_u32(table_bytes, field_offset, datetime->first);
            write_le_u32(table_bytes, field_offset + 4U, datetime->second);
            break;
        }
        default: {
            if (is_null_token) {
                std::fill_n(
                    table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset),
                    field.length,
                    static_cast<std::uint8_t>(0U));
                break;
            }
            const auto opaque_bytes = parse_opaque_field_bytes(value, field.length);
            if (!opaque_bytes.has_value()) {
                return {.ok = false,
                        .error = dbf_table_text("Vfp.DbfTable.Error.OpaqueValueInvalid"),
                        .record_count = header.record_count};
            }
            std::copy(opaque_bytes->begin(), opaque_bytes->end(), table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset));
            break;
        }
    }

    return {.ok = true, .error = {}, .record_count = header.record_count};
}

// Initializes one blank record's default field bytes within table_bytes at
// record_offset (a delete-flag byte of 0x20, then each field's own blank
// representation). Shared between the whole-file append path below and the
// targeted-I/O append fast path further down, so the two can never drift
// out of sync on what "blank" means for a given field type.
DbfWriteResult fill_blank_record_fields(
    std::vector<std::uint8_t>& table_bytes,
    std::size_t record_offset,
    std::size_t record_count_for_error,
    const std::vector<RawFieldDescriptor>& fields) {
    table_bytes[record_offset] = 0x20U;

    for (const auto& field : fields) {
        const std::size_t field_offset = record_offset + field.offset;
        if ((field_offset + field.length) > table_bytes.size()) {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordLayoutExceedsSize"), .record_count = record_count_for_error};
        }

        switch (field.type) {
            case 'C':
            case 'N':
            case 'F':
            case 'D':
                std::fill_n(
                    table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset),
                    field.length,
                    static_cast<std::uint8_t>(' '));
                break;
            case 'L':
                table_bytes[field_offset] = static_cast<std::uint8_t>('?');
                break;
            case 'B':
            case 'I':
            case 'Y':
            case 'T':
            case 'M':
            case 'G':
            case 'P':
            case 'V':
            case 'Q':
            default:
                std::fill_n(
                    table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset),
                    field.length,
                    static_cast<std::uint8_t>(0U));
                break;
        }
    }

    return {.ok = true, .error = {}, .record_count = record_count_for_error};
}

DbfWriteResult append_blank_record_bytes(
    std::vector<std::uint8_t>& table_bytes,
    const DbfHeader& header,
    const std::vector<RawFieldDescriptor>& fields) {
    const std::size_t insert_offset = header.header_length + (static_cast<std::size_t>(header.record_count) * header.record_length);
    if (insert_offset > table_bytes.size()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.TableDataTruncated"), .record_count = header.record_count};
    }

    const bool had_eof_marker = !table_bytes.empty() && table_bytes.back() == 0x1AU;
    if (had_eof_marker) {
        table_bytes.pop_back();
    }

    table_bytes.resize(table_bytes.size() + header.record_length, static_cast<std::uint8_t>(' '));
    const DbfWriteResult fill_result = fill_blank_record_fields(table_bytes, insert_offset, header.record_count, fields);
    if (!fill_result.ok) {
        return fill_result;
    }

    table_bytes.push_back(0x1AU);
    write_le_u32(table_bytes, 4U, header.record_count + 1U);
    return {.ok = true, .error = {}, .record_count = header.record_count + 1U};
}

// Fast targeted-I/O path for append: computes the new blank record's bytes
// directly (needs only the header + field descriptors, bounded by field
// count -- never any existing record data) and appends it via a targeted
// seek, instead of reading the whole file into memory to grow a copy by one
// record and rewriting the whole thing. Writes are ordered new-record
// bytes, then EOF marker, then the last-update stamp, then the
// record-count header field last, so a crash before that last write leaves
// the file's *logical* content (anything reading via the still-unchanged
// record count) unchanged from before the append started -- the orphaned
// new-record bytes are harmless trailing data past the old logical end of
// file, never a torn *existing* record. This is a logical guarantee, not a
// byte-identical one: the last-update stamp (header bytes 1-3) may already
// be updated even if a crash lands before the record-count write. Returns
// std::nullopt for anything unexpected (a structural read failure, an
// unwritable stream), so append_blank_record_to_file falls back to the
// whole-file path rather than leaving the table in a partial state.
std::optional<DbfWriteResult> append_blank_record_to_file_targeted(const std::string& path) {
    if (const auto sidecar_error = ambiguous_required_sidecar_error_for_path(path); sidecar_error.has_value()) {
        return DbfWriteResult{.ok = false, .error = *sidecar_error};
    }

    const DbfParseResult header_result = parse_dbf_header_from_file(path);
    if (!header_result.ok) {
        return std::nullopt;
    }
    const DbfHeader& header = header_result.header;

    std::ifstream descriptor_input(platform::path_from_utf8_string(path), std::ios::binary);
    if (!descriptor_input) {
        return std::nullopt;
    }
    std::vector<std::uint8_t> header_bytes(header.header_length, 0U);
    descriptor_input.read(reinterpret_cast<char*>(header_bytes.data()), static_cast<std::streamsize>(header_bytes.size()));
    if (descriptor_input.gcount() != static_cast<std::streamsize>(header_bytes.size())) {
        return std::nullopt;
    }
    descriptor_input.close();
    const std::vector<RawFieldDescriptor> fields = read_raw_field_descriptors(header_bytes);
    if (const auto error = ambiguous_table_sidecar_error(path, header, fields); error.has_value()) {
        return DbfWriteResult{.ok = false, .error = *error, .record_count = header.record_count};
    }

    std::vector<std::uint8_t> record_bytes(header.record_length, static_cast<std::uint8_t>(' '));
    const DbfWriteResult fill_result = fill_blank_record_fields(record_bytes, 0U, header.record_count, fields);
    if (!fill_result.ok) {
        return fill_result;
    }

    const std::size_t insert_offset =
        static_cast<std::size_t>(header.header_length) + (static_cast<std::size_t>(header.record_count) * header.record_length);

    std::fstream io(platform::path_from_utf8_string(path), std::ios::binary | std::ios::in | std::ios::out);
    if (!io) {
        return std::nullopt;
    }

    // A header declaring more records than the file actually holds (a
    // truncated or corrupt table) would otherwise let insert_offset land
    // past the real end of file: seeking there and writing creates a huge
    // sparse gap instead of failing, unlike the whole-file path this falls
    // back to, which validates this via table_bytes.size() before writing
    // anything. Validate against the real file size the same way here,
    // falling back to that already-correct rejection rather than silently
    // corrupting the file's logical structure.
    io.seekg(0, std::ios::end);
    const std::streamoff actual_size = io.tellg();
    if (actual_size < 0 || insert_offset > static_cast<std::size_t>(actual_size)) {
        return std::nullopt;
    }

    io.seekp(static_cast<std::streamoff>(insert_offset));
    io.write(reinterpret_cast<const char*>(record_bytes.data()), static_cast<std::streamsize>(record_bytes.size()));
    const char eof_marker = static_cast<char>(0x1A);
    io.write(&eof_marker, 1);
    if (!io.good()) {
        return std::nullopt;
    }

    std::vector<std::uint8_t> stamp_bytes(4U, 0U);
    io.clear();
    io.seekg(0);
    io.read(reinterpret_cast<char*>(stamp_bytes.data()), static_cast<std::streamsize>(stamp_bytes.size()));
    if (io.gcount() == static_cast<std::streamsize>(stamp_bytes.size()) && stamp_dbf_last_update_date(stamp_bytes)) {
        io.clear();
        io.seekp(1);
        io.write(reinterpret_cast<const char*>(stamp_bytes.data()) + 1, 3);
    }

    const std::uint32_t new_record_count = header.record_count + 1U;
    std::vector<std::uint8_t> record_count_bytes(4U, 0U);
    write_le_u32(record_count_bytes, 0U, new_record_count);
    io.clear();
    io.seekp(4);
    io.write(reinterpret_cast<const char*>(record_count_bytes.data()), static_cast<std::streamsize>(record_count_bytes.size()));
    io.flush();
    if (!io.good()) {
        return std::nullopt;
    }

    return DbfWriteResult{.ok = true, .error = {}, .record_count = new_record_count};
}

class MemoReader {
public:
    MemoReader() = default;

    explicit MemoReader(
        const std::string& path,
        DbfMemoStorageFormat storage_format = DbfMemoStorageFormat::visual_foxpro,
        std::vector<std::uint32_t> referenced_blocks = {})
        : storage_format_(storage_format),
          referenced_blocks_(std::move(referenced_blocks)) {
        if (path.empty()) {
            return;
        }

        std::ifstream input(platform::path_from_utf8_string(path), std::ios::binary);
        if (!input) {
            return;
        }

        bytes_ = {
            std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>()
        };

        if (bytes_.size() < 512U) {
            bytes_.clear();
            return;
        }

        if (storage_format_ == DbfMemoStorageFormat::visual_foxpro ||
            storage_format_ == DbfMemoStorageFormat::foxpro) {
            block_size_ = read_be_u16(bytes_, 6U);
        } else if (storage_format_ == DbfMemoStorageFormat::dbase_iii) {
            block_size_ = 512U;
        } else {
            block_size_ = read_le_u16(bytes_, 20U);
        }
        if (block_size_ == 0U) {
            bytes_.clear();
            return;
        }
        available_ = true;
    }

    [[nodiscard]] bool available() const {
        return available_;
    }

    [[nodiscard]] std::optional<std::vector<std::uint8_t>> read_block_raw(std::uint32_t block_number) const {
        if (!available_ || block_number == 0U) {
            return std::nullopt;
        }

        const std::uint64_t offset = static_cast<std::uint64_t>(block_number) * block_size_;
        if (offset >= bytes_.size()) {
            return std::nullopt;
        }

        if (storage_format_ == DbfMemoStorageFormat::dbase_iii) {
            const auto begin = bytes_.begin() + static_cast<std::ptrdiff_t>(offset);
            auto search_end = bytes_.end();
            const auto next_block = std::upper_bound(
                referenced_blocks_.begin(),
                referenced_blocks_.end(),
                block_number);
            if (next_block != referenced_blocks_.end()) {
                const std::uint64_t next_offset = static_cast<std::uint64_t>(*next_block) * block_size_;
                if (next_offset < bytes_.size()) {
                    search_end = bytes_.begin() + static_cast<std::ptrdiff_t>(next_offset);
                }
            }
            const auto terminator = std::search(
                begin,
                search_end,
                dbase_iii_memo_terminator.begin(),
                dbase_iii_memo_terminator.end());
            if (terminator == search_end) {
                return std::nullopt;
            }
            return std::vector<std::uint8_t>{begin, terminator};
        }

        if ((offset + 8U) > bytes_.size()) {
            return std::nullopt;
        }

        const std::uint32_t length = storage_format_ == DbfMemoStorageFormat::dbase_iv
            ? read_le_u32(bytes_, static_cast<std::size_t>(offset + 4U))
            : read_be_u32(bytes_, static_cast<std::size_t>(offset + 4U));
        const std::uint64_t payload_offset = offset + 8U;
        const std::uint64_t payload_end = payload_offset + length;
        if (payload_end > bytes_.size()) {
            return std::nullopt;
        }

        return std::vector<std::uint8_t>{
            bytes_.begin() + static_cast<std::ptrdiff_t>(payload_offset),
            bytes_.begin() + static_cast<std::ptrdiff_t>(payload_end)
        };
    }

    [[nodiscard]] std::optional<std::string> read_block(std::uint32_t block_number) const {
        const auto payload = read_block_raw(block_number);
        if (!payload.has_value()) {
            return std::nullopt;
        }

        std::string text;
        text.reserve(payload->size());
        for (const auto byte : *payload) {
            const auto raw = static_cast<unsigned char>(byte);
            if (raw == 0U) {
                text.push_back(' ');
            } else if (std::isprint(raw) != 0 || std::isspace(raw) != 0) {
                text.push_back(static_cast<char>(raw));
            }
        }

        return trim_right(std::move(text));
    }

private:
    static constexpr std::array<std::uint8_t, 2U> dbase_iii_memo_terminator = {0x1AU, 0x1AU};
    std::vector<std::uint8_t> bytes_;
    std::uint32_t block_size_ = 0;
    DbfMemoStorageFormat storage_format_ = DbfMemoStorageFormat::visual_foxpro;
    std::vector<std::uint32_t> referenced_blocks_;
    bool available_ = false;
};

std::string format_binary_bytes(const std::vector<std::uint8_t>& bytes) {
    std::ostringstream stream;
    stream << "0x";
    constexpr std::array<char, 16U> hex = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };
    for (std::uint8_t byte : bytes) {
        stream << hex[(byte >> 4U) & 0x0FU] << hex[byte & 0x0FU];
    }
    return stream.str();
}

struct DecodedDbfValue {
    bool ok = true;
    std::string display_value;

    DecodedDbfValue() = default;
    DecodedDbfValue(const char* value) : display_value(value) {}
    DecodedDbfValue(std::string value) : display_value(std::move(value)) {}
    DecodedDbfValue(bool success, std::string value)
        : ok(success), display_value(std::move(value)) {}
};

DecodedDbfValue decode_value(
    char field_type,
    const std::vector<std::uint8_t>& raw,
    const MemoReader& memo_reader,
    DbfMemoStorageFormat memo_storage_format,
    std::uint8_t code_page_mark,
    bool& is_null,
    std::uint32_t& memo_block_number) {
    is_null = false;
    memo_block_number = 0U;
    if (raw.empty()) {
        return {};
    }

    switch (field_type) {
        case 'C': {
            std::string value(raw.begin(), raw.end());
            const DbfTextConversionResult decoded = decode_dbf_text(
                code_page_mark,
                trim_dbf_text_terminator(std::move(value)));
            return decoded.ok
                ? DecodedDbfValue(std::move(decoded.text))
                : DecodedDbfValue(format_binary_bytes(raw));
        }
        case 'N':
        case 'F': {
            return trim_both(std::string(raw.begin(), raw.end()));
        }
        case 'L': {
            const char value = static_cast<char>(raw[0]);
            if (value == 'Y' || value == 'y' || value == 'T' || value == 't') {
                return "true";
            }
            if (value == 'N' || value == 'n' || value == 'F' || value == 'f') {
                return "false";
            }
            return std::string(1U, value);
        }
        case 'D': {
            const std::string value(raw.begin(), raw.end());
            if (trim_both(value).empty()) {
                return {};
            }
            if (value.size() == 8U) {
                return value.substr(0U, 4U) + "-" + value.substr(4U, 2U) + "-" + value.substr(6U, 2U);
            }
            return trim_both(value);
        }
        case 'I':
        case '+': {
            if (raw.size() < 4U) {
                return {};
            }
            if (memo_storage_format != DbfMemoStorageFormat::visual_foxpro) {
                // dBASE Level 7 stores Long and Autoincrement as a
                // big-endian sign/magnitude value: the high bit is set for
                // non-negative values. VFP's similarly named integer
                // storage is a little-endian two's-complement value.
                const std::uint32_t encoded = read_be_u32(raw, 0U);
                const std::int64_t magnitude = static_cast<std::int64_t>(encoded & 0x7FFFFFFFU);
                return std::to_string((encoded & 0x80000000U) != 0U ? magnitude : -magnitude);
            }
            const std::int32_t value = static_cast<std::int32_t>(read_le_u32(raw, 0U));
            return std::to_string(value);
        }
        case 'B': {
            // dBASE uses B for a ten-character DBT block pointer; VFP uses
            // B for an eight-byte IEEE double. The DBF version family, not
            // the field letter alone, decides which physical representation
            // is safe to decode.
            if (memo_storage_format != DbfMemoStorageFormat::visual_foxpro) {
                const std::optional<std::uint32_t> block_number =
                    parse_memo_block_number(raw, memo_storage_format);
                if (!block_number.has_value() || *block_number == 0U) {
                    return {};
                }
                memo_block_number = *block_number;
                const auto memo_bytes = memo_reader.read_block_raw(*block_number);
                if (memo_bytes.has_value()) {
                    // dBASE B is a binary field. Preserve its bytes as a
                    // deterministic display representation; treating a
                    // printable subset as text would be an unsupported OLE
                    // or binary interpretation claim.
                    return format_binary_bytes(*memo_bytes);
                }
                std::ostringstream stream;
                stream << "<memo block " << *block_number << ">";
                return stream.str();
            }
            if (raw.size() < 8U) {
                return {};
            }
            double value = 0.0;
            std::array<std::uint8_t, 8U> storage{};
            std::copy_n(raw.begin(), 8U, storage.begin());
            std::memcpy(&value, storage.data(), storage.size());
            std::ostringstream stream;
            stream.imbue(std::locale::classic());
            // max_digits10 (17 for IEEE-754 double), not a shorter
            // "display" precision like 15: this text is re-parsed on
            // write (see write_field_bytes()'s 'B' case and #5523's
            // import path), and a shorter precision can silently change
            // the value (e.g. 1.0000000000000002 rounds to 1).
            stream.precision(std::numeric_limits<double>::max_digits10);
            stream << value;
            return trim_both(stream.str());
        }
        case 'V':
        case 'Q': {
            if (raw.size() < 2U) {
                return {};
            }

            const std::size_t payload_capacity = raw.size() - 1U;
            const std::size_t payload_length = std::min<std::size_t>(payload_capacity, raw.back());
            std::string value(raw.begin(), raw.begin() + static_cast<std::ptrdiff_t>(payload_length));
            if (field_type == 'V') {
                const DbfTextConversionResult decoded = decode_dbf_text(code_page_mark, trim_right(std::move(value)));
                return decoded.ok
                    ? DecodedDbfValue(std::move(decoded.text))
                    : DecodedDbfValue(format_binary_bytes(
                        std::vector<std::uint8_t>(value.begin(), value.end())));
            }
            return value;
        }
        case 'M': {
            const std::optional<std::uint32_t> parsed_block_number =
                parse_memo_block_number(raw, memo_storage_format);
            if (!parsed_block_number.has_value()) {
                return {};
            }
            const std::uint32_t block_number = *parsed_block_number;
            memo_block_number = block_number;
            if (block_number == 0U) {
                return {};
            }
            const auto memo_bytes = memo_reader.read_block_raw(block_number);
            if (memo_bytes.has_value()) {
                const DbfTextConversionResult decoded = decode_dbf_text(
                    code_page_mark,
                    std::string_view(
                        reinterpret_cast<const char*>(memo_bytes->data()),
                        memo_bytes->size()));
                if (!decoded.ok) {
                    const auto legacy_display = memo_reader.read_block(block_number);
                    return legacy_display.has_value()
                        ? DecodedDbfValue(trim_dbf_text_terminator(*legacy_display))
                        : DecodedDbfValue{};
                }
                return trim_dbf_text_terminator(std::move(decoded.text));
            }
            std::ostringstream stream;
            stream << "<memo block " << block_number << ">";
            return stream.str();
        }
        case 'G':
        case 'P': {
            const std::optional<std::uint32_t> parsed_block_number =
                parse_memo_block_number(raw, memo_storage_format);
            if (!parsed_block_number.has_value()) {
                return {};
            }
            const std::uint32_t block_number = *parsed_block_number;
            memo_block_number = block_number;
            if (block_number == 0U) {
                return {};
            }
            if (memo_storage_format != DbfMemoStorageFormat::visual_foxpro) {
                const auto memo_bytes = memo_reader.read_block_raw(block_number);
                if (memo_bytes.has_value()) {
                    // dBASE General/Picture payloads are binary assets, not
                    // text memos. Expose bytes without interpreting them.
                    return format_binary_bytes(*memo_bytes);
                }
            } else {
                const auto memo_text = memo_reader.read_block(block_number);
                if (memo_text.has_value()) {
                    return *memo_text;
                }
            }
            std::ostringstream stream;
            stream << "<memo block " << block_number << ">";
            return stream.str();
        }
        case 'Y': {
            if (raw.size() < 8U) {
                return {};
            }
            return format_currency_display_value(read_le_i64(raw, 0U));
        }
        case '@':
        case 'T': {
            if (raw.size() < 8U) {
                return {};
            }
            const std::uint32_t julian_day = read_le_u32(raw, 0U);
            const std::uint32_t millis = read_le_u32(raw, 4U);
            std::ostringstream stream;
            stream.imbue(std::locale::classic());
            stream << "julian:" << julian_day << " millis:" << millis;
            return stream.str();
        }
        case 'O': {
            if (raw.size() < 8U) {
                return {};
            }
            double value = 0.0;
            std::array<std::uint8_t, 8U> storage{};
            std::copy_n(raw.begin(), 8U, storage.begin());
            std::memcpy(&value, storage.data(), storage.size());
            std::ostringstream stream;
            stream.imbue(std::locale::classic());
            // max_digits10 (17 for IEEE-754 double), not a shorter
            // "display" precision like 15: this text is re-parsed on
            // write (see write_field_bytes()'s 'B' case and #5523's
            // import path), and a shorter precision can silently change
            // the value (e.g. 1.0000000000000002 rounds to 1).
            stream.precision(std::numeric_limits<double>::max_digits10);
            stream << value;
            return trim_both(stream.str());
        }
        case '0': {
            is_null = true;
            return "NULL";
        }
        default:
            return format_binary_bytes(raw);
    }
}

struct FieldDescriptorParseResult {
    bool ok = false;
    std::vector<DbfFieldDescriptor> fields;
    std::string error;
};

// Shared by parse_dbf_table_from_file() and parse_dbf_fields_from_file()
// (#5546): the generation-specific field-descriptor-block layout logic
// (dbf_read_layout(), the legacy-family bounds checks, the descriptor-
// terminator requirement), extracted so the two cannot silently disagree
// on a given file's field offsets/types. `bytes` must already contain at
// least `header.header_length` bytes (the caller's own responsibility --
// this function does not itself bound-check that, matching the discipline
// its two callers already established independently before this
// extraction).
FieldDescriptorParseResult parse_dbf_field_descriptor_block(
    const std::vector<std::uint8_t>& bytes,
    const DbfHeader& header,
    const DbfReadLayout& layout) {
    FieldDescriptorParseResult result;
    std::size_t field_offset = layout.descriptor_start;
    const std::size_t header_limit = header.header_length;
    std::uint32_t next_physical_field_offset = 1U;
    bool descriptor_terminator_found = false;
    while ((field_offset + layout.descriptor_size) <= bytes.size() &&
           (field_offset + layout.descriptor_size) <= header_limit &&
           bytes[field_offset] != 0x0DU) {
        DbfFieldDescriptor field;
        field.name = read_ascii_name(bytes, field_offset, layout.descriptor_name_width);
        field.type = static_cast<char>(bytes[field_offset + layout.descriptor_type_offset]);
        field.offset = layout.uses_physical_field_offsets
            ? read_le_u32(bytes, field_offset + 12U)
            : next_physical_field_offset;
        field.length = bytes[field_offset + layout.descriptor_length_offset];
        field.decimal_count = bytes[field_offset + layout.descriptor_decimal_count_offset];
        const DbfFormatFamily strict_layout_family = header.format_family();
        const bool is_strict_legacy_family =
            strict_layout_family == DbfFormatFamily::dbase ||
            strict_layout_family == DbfFormatFamily::foxbase ||
            strict_layout_family == DbfFormatFamily::foxpro;
        if (is_strict_legacy_family &&
            (field.length == 0U || field.offset == 0U ||
             field.offset >= header.record_length ||
             field.length > header.record_length - field.offset)) {
            result.error = dbf_table_text("Vfp.DbfTable.Error.RecordLayoutExceedsSize");
            return result;
        }
        next_physical_field_offset += field.length;
        result.fields.push_back(std::move(field));
        field_offset += layout.descriptor_size;
    }
    if (field_offset < header_limit && field_offset < bytes.size() && bytes[field_offset] == 0x0DU) {
        descriptor_terminator_found = true;
    }
    // The legacy layouts have an independently documented descriptor
    // terminator inside the declared header. Retain the VFP parser's
    // established permissive behavior for its adversarial/recovery fixtures;
    // the read-only dBASE/FoxBASE/FoxPro branches must not change their
    // result.
    {
        const DbfFormatFamily terminator_check_family = header.format_family();
        const bool requires_terminator =
            terminator_check_family == DbfFormatFamily::dbase ||
            terminator_check_family == DbfFormatFamily::foxbase ||
            terminator_check_family == DbfFormatFamily::foxpro;
        if (requires_terminator && !descriptor_terminator_found) {
            result.error = dbf_table_text("Vfp.DbfTable.Error.TableHeaderTruncated");
            return result;
        }
    }
    result.ok = true;
    return result;
}

}  // namespace

bool is_dbf_table_field_storage_layout_writable(char type, std::uint8_t length) {
    if (!supports_table_field_storage(type)) {
        return false;
    }
    if (is_memo_pointer_field(type)) {
        return length >= 4U;
    }
    if (type == 'V' || type == 'Q') {
        return length >= 2U;
    }
    if (type == 'B') {
        return length == 8U;
    }
    if (type == 'I') {
        return length == 4U;
    }
    if (type == 'Y' || type == 'T') {
        return length == 8U;
    }
    return true;
}

std::vector<std::uint8_t> read_memo_block_raw(const std::string& sidecar_path, std::uint32_t block_number) {
    if (sidecar_path.empty() || block_number == 0U) {
        return {};
    }

    std::ifstream input(platform::path_from_utf8_string(sidecar_path), std::ios::binary);
    if (!input) {
        return {};
    }

    const std::vector<std::uint8_t> bytes = {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };

    if (bytes.size() < 512U) {
        return {};
    }

    const std::uint16_t block_size = read_be_u16(bytes, 6U);
    if (block_size == 0U) {
        return {};
    }

    const std::uint64_t offset = static_cast<std::uint64_t>(block_number) * block_size;
    if ((offset + 8U) > bytes.size()) {
        return {};
    }

    const std::uint32_t length = read_be_u32(bytes, static_cast<std::size_t>(offset + 4U));
    const std::uint64_t payload_offset = offset + 8U;
    const std::uint64_t payload_end = payload_offset + length;
    if (payload_end > bytes.size()) {
        return {};
    }

    return {bytes.begin() + static_cast<std::ptrdiff_t>(payload_offset),
            bytes.begin() + static_cast<std::ptrdiff_t>(payload_end)};
}

std::optional<std::uint16_t> read_memo_field_block_size(const std::string& table_path) {
    const std::vector<std::uint8_t> table_bytes = read_binary_file(table_path);
    if (table_bytes.empty()) {
        return std::nullopt;
    }

    const DbfParseResult header_result = parse_dbf_header(table_bytes);
    if (!header_result.ok || table_bytes.size() < header_result.header.header_length) {
        return std::nullopt;
    }
    if (header_result.header.format_family() == DbfFormatFamily::foxbase) {
        // FoxBASE never has a memo file, and its field descriptors (where
        // recognized) do not use the 32-byte-starting-at-offset-32 layout
        // `read_raw_field_descriptors` assumes below; scanning it that way
        // would read descriptor bytes as arbitrary garbage.
        return std::nullopt;
    }

    bool has_memo_field = false;
    for (const RawFieldDescriptor& field : read_raw_field_descriptors(table_bytes)) {
        if (is_memo_pointer_field(field.type)) {
            has_memo_field = true;
            break;
        }
    }
    if (!has_memo_field) {
        return std::nullopt;
    }

    const SidecarPathResolution memo_resolution = resolve_memo_sidecar_path(table_path);
    if (memo_resolution.ambiguous || !memo_resolution.path.has_value()) {
        return std::nullopt;
    }
    const std::vector<std::uint8_t> memo_bytes = read_binary_file(
        platform::path_to_utf8_string(*memo_resolution.path));
    if (memo_bytes.size() < 8U) {
        return std::nullopt;
    }

    const std::uint16_t block_size = read_be_u16(memo_bytes, 6U);
    return block_size == 0U ? std::nullopt : std::optional<std::uint16_t>(block_size);
}

DbfTableParseResult parse_dbf_table_from_file(
    const std::string& path,
    std::size_t max_records,
    const std::string& memo_sidecar_path) {
    SidecarPathResolution memo_resolution;
    std::string resolved_memo_sidecar_path = memo_sidecar_path;
    if (resolved_memo_sidecar_path.empty() && primary_always_requires_memo_sidecar(path)) {
        memo_resolution = resolve_memo_sidecar_path(path);
        if (memo_resolution.ambiguous) {
            return {.ok = false, .error = ambiguous_sidecar_error(memo_resolution)};
        }
        resolved_memo_sidecar_path = selected_sidecar_path(memo_resolution);
    }

    std::ifstream input(platform::path_from_utf8_string(path), std::ios::binary);
    if (!input) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.OpenTableFailed")};
    }

    std::vector<std::uint8_t> bytes = {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };

    const DbfParseResult header_result = parse_dbf_header(bytes);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
    }

    DbfTable table;
    table.header = header_result.header;
    const DbfReadLayout layout = dbf_read_layout(table.header);

    if (bytes.size() < table.header.header_length) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.HeaderLengthExceedsFile")};
    }

    FieldDescriptorParseResult field_result = parse_dbf_field_descriptor_block(bytes, table.header, layout);
    if (!field_result.ok) {
        return {.ok = false, .error = field_result.error};
    }
    table.fields = std::move(field_result.fields);

    if (resolved_memo_sidecar_path.empty() &&
        table_uses_memo_sidecar(table.header, table.fields)) {
        memo_resolution = resolve_memo_sidecar_path(path, table.header);
        if (memo_resolution.ambiguous) {
            return {.ok = false, .error = ambiguous_sidecar_error(memo_resolution)};
        }
        resolved_memo_sidecar_path = selected_sidecar_path(memo_resolution);
    }

    const MemoReader memo_reader(
        resolved_memo_sidecar_path,
        layout.memo_storage_format,
        collect_referenced_memo_blocks(bytes, table.header, table.fields, layout.memo_storage_format));
    const std::size_t record_count = std::min<std::size_t>(table.header.record_count, max_records);
    const std::size_t data_offset = table.header.header_length;
    const std::size_t record_length = table.header.record_length;

    for (std::size_t record_index = 0; record_index < record_count; ++record_index) {
        const std::size_t record_offset = data_offset + (record_index * record_length);
        if ((record_offset + record_length) > bytes.size()) {
            break;
        }

        DbfRecord record;
        record.record_index = record_index;
        record.deleted = bytes[record_offset] == 0x2AU;

        for (const auto& field : table.fields) {
            const std::size_t field_start = record_offset + field.offset;
            const std::size_t field_end = field_start + field.length;
            if (field_end > bytes.size()) {
                break;
            }

            std::vector<std::uint8_t> raw(
                bytes.begin() + static_cast<std::ptrdiff_t>(field_start),
                bytes.begin() + static_cast<std::ptrdiff_t>(field_end));

            bool is_null = false;
            std::uint32_t memo_block_number = 0U;
            const DecodedDbfValue decoded_value = decode_value(
                field.type,
                raw,
                memo_reader,
                layout.memo_storage_format,
                table.header.code_page_mark,
                is_null,
                memo_block_number);
            if (!decoded_value.ok) {
                return {
                    .ok = false,
                    .table = {},
                    .error = dbf_table_text("Vfp.DbfTable.Error.TextEncodingConversionFailed")
                };
            }
            record.values.push_back({
                .field_name = field.name,
                .field_type = field.type,
                .is_null = is_null,
                .display_value = decoded_value.display_value,
                .memo_block_number = memo_block_number
            });
        }

        table.records.push_back(std::move(record));
    }

    return {.ok = true, .table = std::move(table), .error = {}};
}

DbfFieldsOnlyParseResult parse_dbf_fields_from_file(const std::string& path) {
    std::ifstream input(platform::path_from_utf8_string(path), std::ios::binary);
    if (!input) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.OpenTableFailed")};
    }

    // The 32-byte fixed header alone (parse_dbf_header() only needs
    // this much) tells us the real header_length -- only THEN do we
    // know how many bytes the field-descriptor block actually needs,
    // so the real read below is bounded to exactly that, never the
    // whole file.
    std::vector<std::uint8_t> probe_bytes(32U, 0U);
    input.read(reinterpret_cast<char*>(probe_bytes.data()), static_cast<std::streamsize>(probe_bytes.size()));
    if (static_cast<std::size_t>(input.gcount()) < probe_bytes.size()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.OpenTableFailed")};
    }
    const DbfParseResult header_result = parse_dbf_header(probe_bytes);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
    }

    DbfFieldsOnlyParseResult result;
    result.header = header_result.header;
    const DbfReadLayout layout = dbf_read_layout(result.header);

    std::vector<std::uint8_t> bytes(result.header.header_length, 0U);
    input.seekg(0, std::ios::beg);
    input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (static_cast<std::size_t>(input.gcount()) < bytes.size()) {
        result.error = dbf_table_text("Vfp.DbfTable.Error.HeaderLengthExceedsFile");
        return result;
    }

    FieldDescriptorParseResult field_result = parse_dbf_field_descriptor_block(bytes, result.header, layout);
    if (!field_result.ok) {
        result.error = field_result.error;
        return result;
    }
    result.fields = std::move(field_result.fields);
    result.ok = true;
    return result;
}

using DbfCellByteOverrides =
    std::vector<std::vector<std::optional<std::vector<std::uint8_t>>>>;

struct DbfCreateOverrides {
    const DbfCellByteOverrides* memo_payloads = nullptr;
    const DbfCellByteOverrides* raw_field_bytes = nullptr;
    const std::vector<bool>* preserved_raw_fields = nullptr;
    const std::vector<bool>* deleted_flags = nullptr;
};

struct DbfRewriteRowsResult {
    bool ok = false;
    std::string error;
    std::vector<std::vector<std::string>> records;
    DbfCellByteOverrides memo_payloads;
    DbfCellByteOverrides raw_field_bytes;
    std::vector<bool> preserved_raw_fields;
    std::vector<bool> deleted_flags;
};

static DbfRewriteRowsResult collect_dbf_rewrite_rows(
    const std::string& path,
    const DbfTable& table,
    const std::vector<DbfFieldDescriptor>& output_fields,
    const std::vector<std::optional<std::size_t>>& source_field_indices,
    const std::vector<bool>& preserve_raw_source_fields) {
    DbfRewriteRowsResult result;
    if (output_fields.size() != source_field_indices.size() ||
        output_fields.size() != preserve_raw_source_fields.size()) {
        result.error = dbf_table_text("Vfp.DbfTable.Error.RecordFieldCountMismatch");
        return result;
    }

    const bool source_uses_memo_sidecar = primary_always_requires_memo_sidecar(path) ||
        table_uses_memo_sidecar(table.header, table.fields);
    const SidecarPathResolution memo_resolution = source_uses_memo_sidecar
        ? resolve_memo_sidecar_path(path)
        : SidecarPathResolution{};
    if (memo_resolution.ambiguous) {
        result.error = ambiguous_sidecar_error(memo_resolution);
        return result;
    }

    const std::vector<std::uint8_t> table_bytes = read_binary_file(path);
    if (table_bytes.empty()) {
        result.error = dbf_table_text("Vfp.DbfTable.Error.OpenTableFailed");
        return result;
    }

    const MemoReader memo_reader(selected_sidecar_path(memo_resolution));
    result.records.reserve(table.records.size());
    result.memo_payloads.reserve(table.records.size());
    result.raw_field_bytes.reserve(table.records.size());
    result.deleted_flags.reserve(table.records.size());
    result.preserved_raw_fields.assign(output_fields.size(), false);

    for (std::size_t output_index = 0U; output_index < output_fields.size(); ++output_index) {
        if (!source_field_indices[output_index].has_value()) {
            continue;
        }
        const std::size_t source_index = *source_field_indices[output_index];
        if (source_index >= table.fields.size()) {
            result.error = dbf_table_text("Vfp.DbfTable.Error.RecordFieldCountMismatch");
            return result;
        }
        const auto& source_field = table.fields[source_index];
        result.preserved_raw_fields[output_index] =
            preserve_raw_source_fields[output_index] &&
            !is_memo_pointer_field(source_field.type) &&
            source_field.type == output_fields[output_index].type &&
            source_field.length == output_fields[output_index].length;
    }

    for (std::size_t record_index = 0U; record_index < table.records.size(); ++record_index) {
        const auto& record = table.records[record_index];
        if (record.values.size() != table.fields.size()) {
            result.error = dbf_table_text("Vfp.DbfTable.Error.RecordFieldCountMismatch");
            return result;
        }

        std::vector<std::string> output_record;
        DbfCellByteOverrides::value_type output_memo_payloads;
        DbfCellByteOverrides::value_type output_raw_field_bytes;
        output_record.reserve(output_fields.size());
        output_memo_payloads.reserve(output_fields.size());
        output_raw_field_bytes.reserve(output_fields.size());

        for (std::size_t output_index = 0U; output_index < output_fields.size(); ++output_index) {
            const auto source_index = source_field_indices[output_index];
            if (!source_index.has_value()) {
                output_record.push_back({});
                output_memo_payloads.push_back(std::nullopt);
                output_raw_field_bytes.push_back(std::nullopt);
                continue;
            }

            const auto& source_field = table.fields[*source_index];
            const auto& source_value = record.values[*source_index];
            output_record.push_back(source_value.display_value);

            if (is_memo_pointer_field(source_field.type) &&
                is_memo_pointer_field(output_fields[output_index].type) &&
                source_value.memo_block_number != 0U) {
                const auto payload = memo_reader.read_block_raw(source_value.memo_block_number);
                if (!payload.has_value()) {
                    result.error = dbf_table_text("Vfp.DbfTable.Error.ReadMemoPayloadFailed");
                    return result;
                }
                output_memo_payloads.push_back(*payload);
            } else {
                output_memo_payloads.push_back(std::nullopt);
            }

            if (result.preserved_raw_fields[output_index]) {
                const std::size_t record_offset =
                    table.header.header_length + (record_index * table.header.record_length);
                const std::size_t field_offset = record_offset + source_field.offset;
                const std::size_t field_end = field_offset + source_field.length;
                if (field_end > table_bytes.size()) {
                    result.error = dbf_table_text("Vfp.DbfTable.Error.RecordDataTruncated");
                    return result;
                }
                output_raw_field_bytes.emplace_back(std::vector<std::uint8_t>{
                    table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset),
                    table_bytes.begin() + static_cast<std::ptrdiff_t>(field_end)
                });
            } else {
                output_raw_field_bytes.push_back(std::nullopt);
            }
        }

        result.records.push_back(std::move(output_record));
        result.memo_payloads.push_back(std::move(output_memo_payloads));
        result.raw_field_bytes.push_back(std::move(output_raw_field_bytes));
        result.deleted_flags.push_back(record.deleted);
    }

    result.ok = true;
    return result;
}

static DbfWriteResult create_dbf_table_file_with_memo_payloads(
    const std::string& path,
    const std::vector<DbfFieldDescriptor>& fields,
    const std::vector<std::vector<std::string>>& records,
    const DbfCreateOverrides* overrides,
    std::uint8_t code_page_mark = 0U) {
    if (fields.empty()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.CreateFieldRequired")};
    }
    if (overrides != nullptr &&
        ((overrides->memo_payloads != nullptr && overrides->memo_payloads->size() != records.size()) ||
         (overrides->raw_field_bytes != nullptr && overrides->raw_field_bytes->size() != records.size()) ||
         (overrides->preserved_raw_fields != nullptr && overrides->preserved_raw_fields->size() != fields.size()) ||
         (overrides->deleted_flags != nullptr && overrides->deleted_flags->size() != records.size()))) {
        return {
            .ok = false,
            .error = dbf_table_text("Vfp.DbfTable.Error.RecordFieldCountMismatch"),
            .record_count = records.size()
        };
    }

    std::vector<RawFieldDescriptor> raw_fields;
    raw_fields.reserve(fields.size());
    std::uint32_t next_offset = 1U;
    bool has_memo_fields = false;
    for (std::size_t field_index = 0U; field_index < fields.size(); ++field_index) {
        const auto& field = fields[field_index];
        const std::string trimmed_name = trim_both(field.name);
        if (trimmed_name.empty()) {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.FieldNameRequired")};
        }
        if (const auto error = free_table_field_name_length_error(trimmed_name); error.has_value()) {
            return {.ok = false, .error = *error, .record_count = records.size()};
        }
        const std::string normalized_name = serialized_dbf_field_name_key(trimmed_name);
        const auto duplicate = std::find_if(
            raw_fields.begin(),
            raw_fields.end(),
            [&](const RawFieldDescriptor& existing) {
                return serialized_dbf_field_name_key(existing.name) == normalized_name;
            });
        if (duplicate != raw_fields.end()) {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.TargetFieldExists")};
        }
        if (field.length == 0U) {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.FieldLengthRequired")};
        }
        const bool preserves_opaque_field =
            overrides != nullptr && overrides->preserved_raw_fields != nullptr &&
            field_index < overrides->preserved_raw_fields->size() &&
            (*overrides->preserved_raw_fields)[field_index];
        const bool writable_storage_layout =
            is_dbf_table_field_storage_layout_writable(field.type, field.length);
        if (!writable_storage_layout &&
            !(preserves_opaque_field && !supports_table_field_storage(field.type))) {
            if (!supports_table_field_storage(field.type)) {
                return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.CreateUnsupportedFieldType")};
            }
            if (is_memo_pointer_field(field.type)) {
                return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.MemoFieldWidthTooSmall")};
            }
            if (field.type == 'V' || field.type == 'Q') {
                return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.VqFieldWidthTooSmall")};
            }
            if (field.type == 'B') {
                return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.DoubleFieldWidthInvalid")};
            }
            if (field.type == 'I') {
                return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.IntegerFieldWidthInvalid")};
            }
            return {
                .ok = false,
                .error = dbf_table_text(
                    "Vfp.DbfTable.Error.EightByteFieldWidthInvalid",
                    {{"fieldType", std::string(1U, field.type)}})
            };
        }
        if (is_memo_pointer_field(field.type)) {
            has_memo_fields = true;
        }

        raw_fields.push_back({
            .name = trimmed_name,
            .type = field.type,
            .offset = next_offset,
            .length = field.length,
            .decimal_count = field.decimal_count
        });
        next_offset += field.length;
    }

    const std::uint16_t header_length = static_cast<std::uint16_t>(32U + (raw_fields.size() * 32U) + 1U);
    const std::uint16_t record_length = static_cast<std::uint16_t>(next_offset);
    std::vector<std::uint8_t> bytes(
        static_cast<std::size_t>(header_length) + (records.size() * static_cast<std::size_t>(record_length)) + 1U,
        0U);

    bytes[0] = 0x30U;
    write_le_u32(bytes, 4U, static_cast<std::uint32_t>(records.size()));
    write_le_u16(bytes, 8U, header_length);
    write_le_u16(bytes, 10U, record_length);
    bytes[29U] = code_page_mark;

    std::size_t descriptor_offset = 32U;
    for (const auto& field : raw_fields) {
        const std::string field_name = serialized_dbf_field_name(field.name);
        std::copy(field_name.begin(), field_name.end(), bytes.begin() + static_cast<std::ptrdiff_t>(descriptor_offset));
        bytes[descriptor_offset + dbf_descriptor_name_width] = static_cast<std::uint8_t>(field.type);
        write_le_u32(bytes, descriptor_offset + 12U, field.offset);
        bytes[descriptor_offset + 16U] = field.length;
        bytes[descriptor_offset + 17U] = field.decimal_count;
        descriptor_offset += 32U;
    }
    bytes[descriptor_offset] = 0x0DU;
    bytes.back() = 0x1AU;

    const DbfHeader header{
        .version = bytes[0],
        .last_update_year = 0U,
        .last_update_month = 0U,
        .last_update_day = 0U,
        .record_count = static_cast<std::uint32_t>(records.size()),
        .header_length = header_length,
        .record_length = record_length,
        .table_flags = 0U,
        .code_page_mark = code_page_mark
    };

    std::vector<std::uint8_t> memo_bytes = has_memo_fields ? create_empty_memo_sidecar() : std::vector<std::uint8_t>{};

    for (std::size_t record_index = 0; record_index < records.size(); ++record_index) {
        if (records[record_index].size() != raw_fields.size()) {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordFieldCountMismatch"), .record_count = records.size()};
        }
        if (overrides != nullptr && overrides->memo_payloads != nullptr &&
            (record_index >= overrides->memo_payloads->size() ||
             (*overrides->memo_payloads)[record_index].size() != raw_fields.size())) {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordFieldCountMismatch"), .record_count = records.size()};
        }
        if (overrides != nullptr && overrides->raw_field_bytes != nullptr &&
            (record_index >= overrides->raw_field_bytes->size() ||
             (*overrides->raw_field_bytes)[record_index].size() != raw_fields.size())) {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordFieldCountMismatch"), .record_count = records.size()};
        }
        if (overrides != nullptr && overrides->deleted_flags != nullptr &&
            record_index >= overrides->deleted_flags->size()) {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordFieldCountMismatch"), .record_count = records.size()};
        }

        const std::size_t record_offset = header.header_length + (record_index * header.record_length);
        bytes[record_offset] =
            overrides != nullptr && overrides->deleted_flags != nullptr &&
                    (*overrides->deleted_flags)[record_index]
                ? 0x2AU
                : 0x20U;
        for (std::size_t field_index = 0; field_index < raw_fields.size(); ++field_index) {
            DbfWriteResult write_result;
            const bool has_raw_field_override =
                overrides != nullptr && overrides->raw_field_bytes != nullptr &&
                (*overrides->raw_field_bytes)[record_index][field_index].has_value();
            if (has_raw_field_override) {
                const auto& raw_bytes = *(*overrides->raw_field_bytes)[record_index][field_index];
                if (raw_bytes.size() != raw_fields[field_index].length ||
                    is_memo_pointer_field(raw_fields[field_index].type)) {
                    return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordFieldCountMismatch"), .record_count = records.size()};
                }
                std::copy(
                    raw_bytes.begin(),
                    raw_bytes.end(),
                    bytes.begin() + static_cast<std::ptrdiff_t>(record_offset + raw_fields[field_index].offset));
                write_result = {.ok = true, .error = {}, .record_count = records.size()};
            } else if (is_memo_pointer_field(raw_fields[field_index].type)) {
                const bool has_raw_payload_override =
                    overrides != nullptr && overrides->memo_payloads != nullptr &&
                    (*overrides->memo_payloads)[record_index][field_index].has_value();
                if (has_raw_payload_override) {
                    write_result = write_memo_field_bytes(
                        bytes,
                        record_offset + raw_fields[field_index].offset,
                        memo_bytes,
                        *(*overrides->memo_payloads)[record_index][field_index],
                        records.size(),
                        true);
                } else {
                    write_result = write_memo_field_text(
                        bytes,
                        header,
                        record_offset + raw_fields[field_index].offset,
                        memo_bytes,
                        records[record_index][field_index],
                        records.size());
                }
            } else {
                write_result = write_field_bytes(
                    bytes,
                    header,
                    record_index,
                    raw_fields[field_index],
                    records[record_index][field_index],
                    path);
            }
            if (!write_result.ok) {
                return write_result;
            }
        }
    }

    const bool requires_sidecar = primary_always_requires_memo_sidecar(path) || has_memo_fields;
    const SidecarPathResolution memo_resolution = requires_sidecar
        ? resolve_memo_sidecar_path(path)
        : SidecarPathResolution{};
    if (memo_resolution.ambiguous) {
        return {
            .ok = false,
            .error = ambiguous_sidecar_error(memo_resolution),
            .record_count = records.size()
        };
    }

    const std::vector<std::uint8_t> original_table_bytes = read_binary_file(path);
    const bool had_table_file = !original_table_bytes.empty();
    const std::string memo_path = selected_sidecar_path(memo_resolution);
    const std::vector<std::uint8_t> original_memo_bytes = has_memo_fields ? read_binary_file(memo_path) : std::vector<std::uint8_t>{};
    const bool had_memo_file = has_memo_fields && !original_memo_bytes.empty();

    if (!stamp_dbf_last_update_date(bytes) || !write_binary_file(path, bytes)) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.WriteTableFailed"), .record_count = records.size()};
    }
    if (has_memo_fields && !write_binary_file(memo_path, memo_bytes)) {
        if (had_table_file) {
            write_binary_file(path, original_table_bytes);
        } else {
            std::error_code ignored;
            std::filesystem::remove(platform::path_from_utf8_string(path), ignored);
        }

        if (had_memo_file) {
            write_binary_file(memo_path, original_memo_bytes);
        } else {
            std::error_code ignored;
            const std::filesystem::path native_memo_path = platform::path_from_utf8_string(memo_path);
            if (std::filesystem::is_regular_file(native_memo_path, ignored)) {
                std::filesystem::remove(native_memo_path, ignored);
            }
        }
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.WriteMemoSidecarFailed"), .record_count = records.size()};
    }

    return {.ok = true, .error = {}, .record_count = records.size()};
}

static DbfWriteResult rewrite_dbf_table_schema(
    const std::string& path,
    const DbfTable& table,
    const std::vector<DbfFieldDescriptor>& fields,
    const std::vector<std::optional<std::size_t>>& source_field_indices,
    const std::vector<bool>& preserve_raw_source_fields) {
    DbfRewriteRowsResult rewrite_rows = collect_dbf_rewrite_rows(
        path,
        table,
        fields,
        source_field_indices,
        preserve_raw_source_fields);
    if (!rewrite_rows.ok) {
        return {
            .ok = false,
            .error = rewrite_rows.error,
            .record_count = table.records.size()
        };
    }

    const DbfCreateOverrides overrides{
        .memo_payloads = &rewrite_rows.memo_payloads,
        .raw_field_bytes = &rewrite_rows.raw_field_bytes,
        .preserved_raw_fields = &rewrite_rows.preserved_raw_fields,
        .deleted_flags = &rewrite_rows.deleted_flags
    };
    return create_dbf_table_file_with_memo_payloads(
        path,
        fields,
        rewrite_rows.records,
        &overrides,
        table.header.code_page_mark);
}

namespace {

struct RawDbfMutationState {
    DbfHeader header{};
    std::vector<RawFieldDescriptor> fields;
    std::vector<std::uint8_t> table_bytes;
    bool has_memo_sidecar = false;
    std::string memo_path;
    std::vector<std::uint8_t> memo_bytes;
    std::size_t records_end = 0U;
};

DbfRawRecordMutationResult failed_raw_record_mutation(
    std::string error,
    std::size_t record_count = 0U) {
    return {
        .ok = false,
        .error = std::move(error),
        .table_bytes = {},
        .has_memo_sidecar = false,
        .memo_path = {},
        .memo_bytes = {},
        .record_count = record_count
    };
}

bool load_raw_dbf_mutation_state(
    const std::string& path,
    RawDbfMutationState& state,
    DbfRawRecordMutationResult& failure) {
    state.table_bytes = read_binary_file(path);
    if (state.table_bytes.empty()) {
        failure = failed_raw_record_mutation(dbf_table_text("Vfp.DbfTable.Error.OpenTableFailed"));
        return false;
    }

    const DbfParseResult header_result = parse_dbf_header(state.table_bytes);
    if (!header_result.ok) {
        failure = failed_raw_record_mutation(header_result.error);
        return false;
    }
    state.header = header_result.header;

    if (state.header.header_length > state.table_bytes.size()) {
        failure = failed_raw_record_mutation(
            dbf_table_text("Vfp.DbfTable.Error.TableHeaderTruncated"),
            state.header.record_count);
        return false;
    }
    if (state.header.record_count >
        (std::numeric_limits<std::size_t>::max() - state.header.header_length) /
            state.header.record_length) {
        failure = failed_raw_record_mutation(
            dbf_table_text("Vfp.DbfTable.Error.TableDataTruncated"),
            state.header.record_count);
        return false;
    }
    state.records_end = state.header.header_length +
        (static_cast<std::size_t>(state.header.record_count) * state.header.record_length);
    if (state.records_end > state.table_bytes.size()) {
        failure = failed_raw_record_mutation(
            dbf_table_text("Vfp.DbfTable.Error.RecordDataTruncated"),
            state.header.record_count);
        return false;
    }

    std::size_t descriptor_offset = 32U;
    std::size_t descriptor_count = 0U;
    bool descriptor_terminator_found = false;
    while (descriptor_offset < state.header.header_length) {
        if (state.table_bytes[descriptor_offset] == 0x0DU) {
            descriptor_terminator_found = true;
            break;
        }
        if ((descriptor_offset + 32U) > state.header.header_length) {
            break;
        }
        ++descriptor_count;
        descriptor_offset += 32U;
    }
    if (!descriptor_terminator_found) {
        failure = failed_raw_record_mutation(
            dbf_table_text("Vfp.DbfTable.Error.TableHeaderTruncated"),
            state.header.record_count);
        return false;
    }

    state.fields = read_raw_field_descriptors(state.table_bytes);
    if (state.fields.size() != descriptor_count) {
        failure = failed_raw_record_mutation(
            dbf_table_text("Vfp.DbfTable.Error.RecordLayoutExceedsSize"),
            state.header.record_count);
        return false;
    }

    std::vector<bool> occupied(state.header.record_length, false);
    occupied[0] = true;
    for (const auto& field : state.fields) {
        if (field.length == 0U || field.offset == 0U ||
            field.offset >= state.header.record_length ||
            field.length > state.header.record_length - field.offset) {
            failure = failed_raw_record_mutation(
                dbf_table_text("Vfp.DbfTable.Error.RecordLayoutExceedsSize"),
                state.header.record_count);
            return false;
        }
        for (std::size_t offset = field.offset; offset < field.offset + field.length; ++offset) {
            if (occupied[offset]) {
                failure = failed_raw_record_mutation(
                    dbf_table_text("Vfp.DbfTable.Error.RecordLayoutExceedsSize"),
                    state.header.record_count);
                return false;
            }
            occupied[offset] = true;
        }
    }

    const bool requires_sidecar =
        primary_always_requires_memo_sidecar(path) ||
        table_uses_memo_sidecar(state.header, state.fields);
    if (!requires_sidecar) {
        return true;
    }

    const SidecarPathResolution memo_resolution = resolve_memo_sidecar_path(path);
    if (memo_resolution.ambiguous) {
        failure = failed_raw_record_mutation(
            ambiguous_sidecar_error(memo_resolution),
            state.header.record_count);
        return false;
    }
    state.memo_path = selected_sidecar_path(memo_resolution);
    if (state.memo_path.empty()) {
        failure = failed_raw_record_mutation(
            dbf_table_text("Vfp.DbfTable.Error.MemoSidecarPathMissing"),
            state.header.record_count);
        return false;
    }
    state.memo_bytes = read_binary_file(state.memo_path);
    if (state.memo_bytes.size() < 8U) {
        failure = failed_raw_record_mutation(
            dbf_table_text("Vfp.DbfTable.Error.ReadMemoPayloadFailed"),
            state.header.record_count);
        return false;
    }

    const std::uint16_t block_size = read_be_u16(state.memo_bytes, 6U);
    const std::uint32_t next_free_block = read_be_u32(state.memo_bytes, 0U);
    if (block_size < 8U || next_free_block == 0U ||
        next_free_block > std::numeric_limits<std::size_t>::max() / block_size ||
        (static_cast<std::size_t>(next_free_block) * block_size) > state.memo_bytes.size()) {
        failure = failed_raw_record_mutation(
            dbf_table_text("Vfp.DbfTable.Error.ReadMemoPayloadFailed"),
            state.header.record_count);
        return false;
    }

    for (std::size_t record_index = 0U; record_index < state.header.record_count; ++record_index) {
        const std::size_t record_offset = state.header.header_length +
            (record_index * state.header.record_length);
        for (const auto& field : state.fields) {
            if (!is_memo_pointer_field(field.type)) {
                continue;
            }
            if (field.length < 4U) {
                failure = failed_raw_record_mutation(
                    dbf_table_text("Vfp.DbfTable.Error.MemoFieldWidthTooSmall"),
                    state.header.record_count);
                return false;
            }
            const std::uint32_t block_number = read_le_u32(
                state.table_bytes,
                record_offset + field.offset);
            if (block_number == 0U) {
                continue;
            }
            if (block_number >= next_free_block ||
                block_number > std::numeric_limits<std::size_t>::max() / block_size) {
                failure = failed_raw_record_mutation(
                    dbf_table_text("Vfp.DbfTable.Error.ReadMemoPayloadFailed"),
                    state.header.record_count);
                return false;
            }
            const std::size_t block_offset = static_cast<std::size_t>(block_number) * block_size;
            if (block_offset > state.memo_bytes.size() ||
                (state.memo_bytes.size() - block_offset) < 8U) {
                failure = failed_raw_record_mutation(
                    dbf_table_text("Vfp.DbfTable.Error.ReadMemoPayloadFailed"),
                    state.header.record_count);
                return false;
            }
            const std::uint32_t payload_length = read_be_u32(state.memo_bytes, block_offset + 4U);
            const std::size_t declared_memo_end =
                static_cast<std::size_t>(next_free_block) * block_size;
            if ((declared_memo_end - block_offset) < 8U ||
                payload_length > declared_memo_end - block_offset - 8U) {
                failure = failed_raw_record_mutation(
                    dbf_table_text("Vfp.DbfTable.Error.ReadMemoPayloadFailed"),
                    state.header.record_count);
                return false;
            }
        }
    }

    state.has_memo_sidecar = true;
    return true;
}

std::vector<std::uint8_t> make_blank_raw_record(
    const DbfHeader& header,
    const std::vector<RawFieldDescriptor>& fields) {
    std::vector<std::uint8_t> record(header.record_length, static_cast<std::uint8_t>(' '));
    record[0] = 0x20U;
    for (const auto& field : fields) {
        auto first = record.begin() + static_cast<std::ptrdiff_t>(field.offset);
        switch (field.type) {
            case 'C':
            case 'N':
            case 'F':
            case 'D':
                std::fill_n(first, field.length, static_cast<std::uint8_t>(' '));
                break;
            case 'L':
                std::fill_n(first, field.length, static_cast<std::uint8_t>(0U));
                *first = static_cast<std::uint8_t>('?');
                break;
            default:
                std::fill_n(first, field.length, static_cast<std::uint8_t>(0U));
                break;
        }
    }
    return record;
}

DbfWriteResult write_memo_field_bytes_preserving_prefix(
    std::vector<std::uint8_t>& table_bytes,
    std::size_t field_offset,
    std::vector<std::uint8_t>& memo_bytes,
    const std::string& value,
    std::size_t record_count) {
    if ((field_offset + 4U) > table_bytes.size()) {
        return {
            .ok = false,
            .error = dbf_table_text("Vfp.DbfTable.Error.RecordDataTruncated"),
            .record_count = record_count
        };
    }
    if (value.empty()) {
        write_le_u32(table_bytes, field_offset, 0U);
        return {.ok = true, .error = {}, .record_count = record_count};
    }
    if (memo_bytes.size() < 8U) {
        return {
            .ok = false,
            .error = dbf_table_text("Vfp.DbfTable.Error.ReadMemoPayloadFailed"),
            .record_count = record_count
        };
    }

    const std::uint16_t block_size = read_be_u16(memo_bytes, 6U);
    const std::uint32_t declared_next_free_block = read_be_u32(memo_bytes, 0U);
    if (block_size == 0U || declared_next_free_block == 0U) {
        return {
            .ok = false,
            .error = dbf_table_text("Vfp.DbfTable.Error.ReadMemoPayloadFailed"),
            .record_count = record_count
        };
    }

    if (value.size() > std::numeric_limits<std::uint32_t>::max() ||
        value.size() > std::numeric_limits<std::size_t>::max() - 8U) {
        return {
            .ok = false,
            .error = dbf_table_text("Vfp.DbfTable.Error.ReadMemoPayloadFailed"),
            .record_count = record_count
        };
    }
    const std::size_t physical_next_block =
        (memo_bytes.size() / block_size) +
        (memo_bytes.size() % block_size == 0U ? 0U : 1U);
    const std::size_t next_block = std::max<std::size_t>(declared_next_free_block, physical_next_block);
    const std::size_t required_bytes = 8U + value.size();
    const std::size_t required_blocks =
        (required_bytes / block_size) +
        (required_bytes % block_size == 0U ? 0U : 1U);
    if (next_block > std::numeric_limits<std::uint32_t>::max() ||
        required_blocks > std::numeric_limits<std::uint32_t>::max() - next_block ||
        next_block > std::numeric_limits<std::size_t>::max() / block_size ||
        required_blocks >
            (std::numeric_limits<std::size_t>::max() / block_size) - next_block) {
        return {
            .ok = false,
            .error = dbf_table_text("Vfp.DbfTable.Error.ReadMemoPayloadFailed"),
            .record_count = record_count
        };
    }

    const std::size_t block_offset = next_block * block_size;
    const std::size_t new_size = (next_block + required_blocks) * block_size;
    memo_bytes.resize(new_size, 0U);
    memo_bytes[block_offset + 3U] = 1U;
    write_be_u32(memo_bytes, block_offset + 4U, static_cast<std::uint32_t>(value.size()));
    std::copy(
        value.begin(),
        value.end(),
        memo_bytes.begin() + static_cast<std::ptrdiff_t>(block_offset + 8U));
    write_be_u32(
        memo_bytes,
        0U,
        static_cast<std::uint32_t>(next_block + required_blocks));
    write_le_u32(table_bytes, field_offset, static_cast<std::uint32_t>(next_block));
    return {.ok = true, .error = {}, .record_count = record_count};
}

}  // namespace

bool stamp_dbf_last_update_date(std::vector<std::uint8_t>& bytes) {
    if (bytes.size() < 4U) {
        return false;
    }

    const std::time_t now = std::time(nullptr);
    std::tm local_time{};
#if defined(_WIN32)
    if (localtime_s(&local_time, &now) != 0) {
        return false;
    }
#else
    if (localtime_r(&now, &local_time) == nullptr) {
        return false;
    }
#endif
    if (local_time.tm_year < 0 ||
        local_time.tm_mon < 0 || local_time.tm_mon > 11 ||
        local_time.tm_mday < 1 || local_time.tm_mday > 31) {
        return false;
    }

    // #5527: write the genuine two-digit calendar year (year % 100), the
    // convention real dBASE-family products actually use on disk --
    // confirmed against real FoxBASE+ 2.10 output, which writes 26 (not
    // 126) for a table last updated in 2026. Writing the raw
    // years-since-1900 value instead (this function's behavior before
    // #5527) produced a byte no real xBase-family product would ever
    // write for a post-1999 date, an interop gap for a project whose
    // whole purpose is xBase-family compatibility.
    // DbfHeader::last_update_iso8601() disambiguates the resulting
    // two-digit byte back into a real year using a fixed century-
    // rollover threshold (see that function's own comment for why a
    // fixed threshold, not a "now"-relative one, is the right choice
    // for this specific field).
    bytes[1U] = static_cast<std::uint8_t>(local_time.tm_year % 100);
    bytes[2U] = static_cast<std::uint8_t>(local_time.tm_mon + 1);
    bytes[3U] = static_cast<std::uint8_t>(local_time.tm_mday);
    return true;
}

DbfRawRecordMutationResult stage_dbf_raw_record_appends(
    const std::string& path,
    const std::vector<DbfRawRecordAppend>& appends) {
    RawDbfMutationState state;
    DbfRawRecordMutationResult failure;
    if (!load_raw_dbf_mutation_state(path, state, failure)) {
        return failure;
    }
    const std::vector<std::uint8_t> original_table_bytes = state.table_bytes;

    if (appends.size() >
        std::numeric_limits<std::uint32_t>::max() - state.header.record_count) {
        return failed_raw_record_mutation(
            dbf_table_text("Vfp.DbfTable.Error.RecordFieldCountMismatch"),
            state.header.record_count);
    }
    if (appends.size() >
        (std::numeric_limits<std::size_t>::max() - state.table_bytes.size()) /
            state.header.record_length) {
        return failed_raw_record_mutation(
            dbf_table_text("Vfp.DbfTable.Error.TableDataTruncated"),
            state.header.record_count);
    }

    std::vector<std::uint8_t> appended_record_bytes;
    appended_record_bytes.reserve(appends.size() * state.header.record_length);
    for (const auto& append : appends) {
        if (append.source_record_index.has_value()) {
            if (*append.source_record_index >= state.header.record_count) {
                return failed_raw_record_mutation(
                    dbf_table_text("Vfp.DbfTable.Error.RecordIndexOutOfRange"),
                    state.header.record_count);
            }
            const std::size_t source_offset = state.header.header_length +
                (*append.source_record_index * state.header.record_length);
            appended_record_bytes.insert(
                appended_record_bytes.end(),
                state.table_bytes.begin() + static_cast<std::ptrdiff_t>(source_offset),
                state.table_bytes.begin() + static_cast<std::ptrdiff_t>(
                    source_offset + state.header.record_length));
        } else {
            const auto blank_record = make_blank_raw_record(state.header, state.fields);
            appended_record_bytes.insert(
                appended_record_bytes.end(),
                blank_record.begin(),
                blank_record.end());
        }
    }

    state.table_bytes.insert(
        state.table_bytes.begin() + static_cast<std::ptrdiff_t>(state.records_end),
        appended_record_bytes.begin(),
        appended_record_bytes.end());
    const std::uint32_t new_record_count =
        state.header.record_count + static_cast<std::uint32_t>(appends.size());
    write_le_u32(state.table_bytes, 4U, new_record_count);
    DbfHeader staged_header = state.header;
    staged_header.record_count = new_record_count;

    for (std::size_t append_index = 0U; append_index < appends.size(); ++append_index) {
        const std::size_t record_index = state.header.record_count + append_index;
        for (const auto& [field_name, value] : appends[append_index].field_values) {
            const auto field = find_raw_field(state.fields, field_name);
            if (!field.has_value()) {
                return failed_raw_record_mutation(
                    dbf_table_text("Vfp.DbfTable.Error.TargetFieldNotFoundInTable"),
                    state.header.record_count);
            }

            DbfWriteResult write_result;
            if (is_memo_pointer_field(field->type)) {
                if (!state.has_memo_sidecar || field->length < 4U) {
                    return failed_raw_record_mutation(
                        state.has_memo_sidecar
                            ? dbf_table_text("Vfp.DbfTable.Error.MemoFieldWidthTooSmall")
                            : dbf_table_text("Vfp.DbfTable.Error.MemoSidecarPathMissing"),
                        state.header.record_count);
                }
                const std::size_t field_offset = staged_header.header_length +
                    (record_index * staged_header.record_length) + field->offset;
                write_result = write_memo_field_bytes_preserving_prefix(
                    state.table_bytes,
                    field_offset,
                    state.memo_bytes,
                    value,
                    staged_header.record_count);
            } else {
                write_result = write_field_bytes(
                    state.table_bytes,
                    staged_header,
                    record_index,
                    *field,
                    value,
                    path);
            }
            if (!write_result.ok) {
                return failed_raw_record_mutation(write_result.error, state.header.record_count);
            }
        }
    }

    if (state.table_bytes != original_table_bytes &&
        !stamp_dbf_last_update_date(state.table_bytes)) {
        return failed_raw_record_mutation(
            dbf_table_text("Vfp.DbfTable.Error.WriteTableFailed"),
            state.header.record_count);
    }

    return {
        .ok = true,
        .error = {},
        .table_bytes = std::move(state.table_bytes),
        .has_memo_sidecar = state.has_memo_sidecar,
        .memo_path = std::move(state.memo_path),
        .memo_bytes = std::move(state.memo_bytes),
        .record_count = staged_header.record_count
    };
}

DbfRawRecordMutationResult stage_dbf_raw_record_reorder(
    const std::string& path,
    const std::vector<std::size_t>& record_order) {
    RawDbfMutationState state;
    DbfRawRecordMutationResult failure;
    if (!load_raw_dbf_mutation_state(path, state, failure)) {
        return failure;
    }
    const std::vector<std::uint8_t> original_table_bytes = state.table_bytes;
    if (record_order.size() != state.header.record_count) {
        return failed_raw_record_mutation(
            dbf_table_text("Vfp.DbfTable.Error.RecordFieldCountMismatch"),
            state.header.record_count);
    }

    std::vector<bool> seen(record_order.size(), false);
    for (const auto record_index : record_order) {
        if (record_index >= state.header.record_count || seen[record_index]) {
            return failed_raw_record_mutation(
                dbf_table_text("Vfp.DbfTable.Error.RecordIndexOutOfRange"),
                state.header.record_count);
        }
        seen[record_index] = true;
    }

    const std::vector<std::uint8_t> original_record_bytes{
        state.table_bytes.begin() + static_cast<std::ptrdiff_t>(state.header.header_length),
        state.table_bytes.begin() + static_cast<std::ptrdiff_t>(state.records_end)
    };
    for (std::size_t destination_index = 0U;
         destination_index < record_order.size();
         ++destination_index) {
        const std::size_t source_offset = record_order[destination_index] * state.header.record_length;
        const std::size_t destination_offset = state.header.header_length +
            (destination_index * state.header.record_length);
        std::copy_n(
            original_record_bytes.begin() + static_cast<std::ptrdiff_t>(source_offset),
            state.header.record_length,
            state.table_bytes.begin() + static_cast<std::ptrdiff_t>(destination_offset));
    }

    if (state.table_bytes != original_table_bytes &&
        !stamp_dbf_last_update_date(state.table_bytes)) {
        return failed_raw_record_mutation(
            dbf_table_text("Vfp.DbfTable.Error.WriteTableFailed"),
            state.header.record_count);
    }

    return {
        .ok = true,
        .error = {},
        .table_bytes = std::move(state.table_bytes),
        .has_memo_sidecar = state.has_memo_sidecar,
        .memo_path = std::move(state.memo_path),
        .memo_bytes = std::move(state.memo_bytes),
        .record_count = state.header.record_count
    };
}

DbfWriteResult create_dbf_table_file(
    const std::string& path,
    const std::vector<DbfFieldDescriptor>& fields,
    const std::vector<std::vector<std::string>>& records) {
    return create_dbf_table_file_with_memo_payloads(path, fields, records, nullptr);
}

// #5485: the first legacy binary-output target. Writes a dBASE III-
// compatible table (structure + data only, no memo/index sidecar) using
// #5482's read-side ground truth for the format: a 32-byte main header with
// version 0x03, 32-byte field descriptors whose offset slot (bytes 12-15)
// is left zero -- meaningless on disk in real dBASE III files, per the
// dBASE-family reader's own sequential-offset handling -- and physically
// sequential C/N/L/D field packing after the deletion marker. Field value
// encoding is otherwise identical to VFP's, so this reuses write_field_bytes
// unchanged rather than re-deriving C/N/L/D formatting rules.
DbfWriteResult create_dbase_iii_table_file(
    const std::string& path,
    const std::vector<DbfFieldDescriptor>& fields,
    const std::vector<std::vector<std::string>>& records) {
    if (fields.empty()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.CreateFieldRequired")};
    }

    std::vector<RawFieldDescriptor> raw_fields;
    raw_fields.reserve(fields.size());
    std::uint32_t next_offset = 1U;
    for (const auto& field : fields) {
        const std::string trimmed_name = trim_both(field.name);
        if (trimmed_name.empty()) {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.FieldNameRequired")};
        }
        if (const auto error = free_table_field_name_length_error(trimmed_name); error.has_value()) {
            return {.ok = false, .error = *error, .record_count = records.size()};
        }
        const std::string normalized_name = serialized_dbf_field_name_key(trimmed_name);
        const auto duplicate = std::find_if(
            raw_fields.begin(),
            raw_fields.end(),
            [&](const RawFieldDescriptor& existing) {
                return serialized_dbf_field_name_key(existing.name) == normalized_name;
            });
        if (duplicate != raw_fields.end()) {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.TargetFieldExists")};
        }
        if (field.length == 0U) {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.FieldLengthRequired")};
        }
        // dBASE III's real field-type set: no memo/General/Picture (no DBT
        // sidecar in this first slice), no VFP-only Double/Integer/Varchar
        // types.
        if (field.type != 'C' && field.type != 'N' && field.type != 'L' && field.type != 'D') {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.CreateUnsupportedFieldType")};
        }
        // write_field_bytes()'s 'D' case always writes an 8-character
        // YYYYMMDD payload regardless of the descriptor's declared length;
        // a shorter declared length would let that write run past the
        // field (or, for the last field, past the record buffer).
        if (field.type == 'D' && field.length != 8U) {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.DateFieldWidthInvalid")};
        }

        raw_fields.push_back({
            .name = trimmed_name,
            .type = field.type,
            .offset = next_offset,
            .length = field.length,
            .decimal_count = field.decimal_count
        });
        // The on-disk header/record-length fields are 16-bit; reject a
        // schema that would silently wrap them rather than build a
        // malformed file (or, for the field-count case, write descriptor
        // bytes past a too-small buffer allocated from the wrapped size).
        next_offset += field.length;
        if (next_offset > std::numeric_limits<std::uint16_t>::max()) {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordWidthTooLarge")};
        }
    }
    if ((32U + (raw_fields.size() * 32U) + 1U) > std::numeric_limits<std::uint16_t>::max()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.TooManyFieldsForHeader")};
    }

    const std::uint16_t header_length = static_cast<std::uint16_t>(32U + (raw_fields.size() * 32U) + 1U);
    const std::uint16_t record_length = static_cast<std::uint16_t>(next_offset);
    std::vector<std::uint8_t> bytes(
        static_cast<std::size_t>(header_length) + (records.size() * static_cast<std::size_t>(record_length)) + 1U,
        0U);

    bytes[0] = 0x03U;
    write_le_u32(bytes, 4U, static_cast<std::uint32_t>(records.size()));
    write_le_u16(bytes, 8U, header_length);
    write_le_u16(bytes, 10U, record_length);

    std::size_t descriptor_offset = 32U;
    for (const auto& field : raw_fields) {
        const std::string field_name = serialized_dbf_field_name(field.name);
        std::copy(field_name.begin(), field_name.end(), bytes.begin() + static_cast<std::ptrdiff_t>(descriptor_offset));
        bytes[descriptor_offset + dbf_descriptor_name_width] = static_cast<std::uint8_t>(field.type);
        // Bytes 12-15 deliberately left zero: real dBASE III files store an
        // in-memory field address there, not an on-disk offset.
        bytes[descriptor_offset + 16U] = field.length;
        bytes[descriptor_offset + 17U] = field.decimal_count;
        descriptor_offset += 32U;
    }
    bytes[descriptor_offset] = 0x0DU;
    bytes.back() = 0x1AU;

    const DbfHeader header{
        .version = bytes[0],
        .last_update_year = 0U,
        .last_update_month = 0U,
        .last_update_day = 0U,
        .record_count = static_cast<std::uint32_t>(records.size()),
        .header_length = header_length,
        .record_length = record_length,
        .table_flags = 0U,
        .code_page_mark = 0U
    };

    for (std::size_t record_index = 0U; record_index < records.size(); ++record_index) {
        if (records[record_index].size() != raw_fields.size()) {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordFieldCountMismatch"), .record_count = records.size()};
        }
        const std::size_t record_offset = header.header_length + (record_index * header.record_length);
        bytes[record_offset] = 0x20U;
        for (std::size_t field_index = 0U; field_index < raw_fields.size(); ++field_index) {
            const DbfWriteResult write_result = write_field_bytes(
                bytes, header, record_index, raw_fields[field_index], records[record_index][field_index], path);
            if (!write_result.ok) {
                return write_result;
            }
        }
    }

    // A clean, confident "it exists" stat result gives a precise early
    // error; anything else (doesn't exist, or the stat itself failed) falls
    // through to the exclusive-create attempt below, which is the actual
    // no-overwrite guarantee -- it refuses to replace an existing entry
    // atomically, so a stat error here can never be mistaken for "safe to
    // write."
    std::error_code exists_error;
    if (std::filesystem::exists(platform::path_from_utf8_string(path), exists_error) && !exists_error) {
        return {
            .ok = false,
            .error = dbf_table_text("Vfp.DbfTable.Error.CreateDestinationExists", {{"path", path}}),
            .record_count = records.size()
        };
    }

    if (!stamp_dbf_last_update_date(bytes) ||
        !platform::write_new_durable_file(
            platform::path_from_utf8_string(path),
            std::string_view(reinterpret_cast<const char*>(bytes.data()), bytes.size()))) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.WriteTableFailed"), .record_count = records.size()};
    }

    return {.ok = true, .error = {}, .record_count = records.size()};
}

// The legacy dBASE/FoxBASE/FoxPro readers deliberately have no write-format
// implementation. Rejecting a mutation here, before a VFP-oriented writer
// can reinterpret the older descriptor or memo layout, is safer than
// relying on documentation alone to preserve the input table byte-for-byte.
static std::optional<DbfWriteResult> dbase_read_only_mutation_error(
    const std::string& path) {
    const DbfParseResult header_result = parse_dbf_header_from_file(path);
    if (!header_result.ok) {
        return std::nullopt;
    }
    switch (header_result.header.format_family()) {
        case DbfFormatFamily::dbase:
            return DbfWriteResult{
                .ok = false,
                .error = dbf_table_text("Vfp.DbfTable.Error.LegacyDbaseReadOnly"),
                .record_count = header_result.header.record_count
            };
        case DbfFormatFamily::foxbase:
        case DbfFormatFamily::foxpro:
            return DbfWriteResult{
                .ok = false,
                .error = dbf_table_text("Vfp.DbfTable.Error.LegacyFoxproReadOnly"),
                .record_count = header_result.header.record_count
            };
        default:
            return std::nullopt;
    }
}

DbfWriteResult add_dbf_table_field(const std::string& path, const DbfFieldDescriptor& field) {
    if (const auto error = ambiguous_required_sidecar_error_for_path(path); error.has_value()) {
        return {.ok = false, .error = *error};
    }
    if (const auto error = dbase_read_only_mutation_error(path); error.has_value()) {
        return *error;
    }
    const DbfParseResult header_result = parse_dbf_header_from_file(path);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
    }

    DbfTableParseResult table_result = parse_dbf_table_from_file(path, header_result.header.record_count);
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }

    const std::string trimmed_new_name = trim_both(field.name);
    if (trimmed_new_name.empty()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.FieldNameRequired"), .record_count = table_result.table.records.size()};
    }
    if (const auto error = free_table_field_name_length_error(trimmed_new_name); error.has_value()) {
        return {.ok = false, .error = *error, .record_count = table_result.table.records.size()};
    }
    const std::string normalized_new_name = serialized_dbf_field_name_key(trimmed_new_name);
    const auto duplicate = std::find_if(
        table_result.table.fields.begin(),
        table_result.table.fields.end(),
        [&](const DbfFieldDescriptor& existing) {
            return serialized_dbf_field_name_key(existing.name) == normalized_new_name;
        });
    if (duplicate != table_result.table.fields.end()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.TargetFieldExists"), .record_count = table_result.table.records.size()};
    }

    std::vector<DbfFieldDescriptor> fields = table_result.table.fields;
    fields.push_back(field);

    std::vector<std::optional<std::size_t>> source_field_indices;
    source_field_indices.reserve(fields.size());
    std::vector<bool> preserve_raw_source_fields;
    preserve_raw_source_fields.reserve(fields.size());
    for (std::size_t index = 0U; index < table_result.table.fields.size(); ++index) {
        source_field_indices.push_back(index);
        preserve_raw_source_fields.push_back(true);
    }
    source_field_indices.push_back(std::nullopt);
    preserve_raw_source_fields.push_back(false);

    return rewrite_dbf_table_schema(
        path,
        table_result.table,
        fields,
        source_field_indices,
        preserve_raw_source_fields);
}

DbfWriteResult drop_dbf_table_field(const std::string& path, const std::string& field_name) {
    if (const auto error = ambiguous_required_sidecar_error_for_path(path); error.has_value()) {
        return {.ok = false, .error = *error};
    }
    if (const auto error = dbase_read_only_mutation_error(path); error.has_value()) {
        return *error;
    }
    const DbfParseResult header_result = parse_dbf_header_from_file(path);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
    }

    DbfTableParseResult table_result = parse_dbf_table_from_file(path, header_result.header.record_count);
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }

    const std::string normalized_field_name = lowercase_copy(trim_both(field_name));
    const auto field = std::find_if(
        table_result.table.fields.begin(),
        table_result.table.fields.end(),
        [&](const DbfFieldDescriptor& candidate) {
            return lowercase_copy(trim_both(candidate.name)) == normalized_field_name;
    });
    if (field == table_result.table.fields.end()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.TargetFieldNotFound"), .record_count = table_result.table.records.size()};
    }
    if (table_result.table.fields.size() <= 1U) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.DropLastField"), .record_count = table_result.table.records.size()};
    }

    const std::size_t field_index = static_cast<std::size_t>(std::distance(table_result.table.fields.begin(), field));
    std::vector<DbfFieldDescriptor> fields;
    fields.reserve(table_result.table.fields.size() - 1U);
    std::vector<std::optional<std::size_t>> source_field_indices;
    source_field_indices.reserve(table_result.table.fields.size() - 1U);
    std::vector<bool> preserve_raw_source_fields;
    preserve_raw_source_fields.reserve(table_result.table.fields.size() - 1U);
    for (std::size_t index = 0U; index < table_result.table.fields.size(); ++index) {
        if (index != field_index) {
            fields.push_back(table_result.table.fields[index]);
            source_field_indices.push_back(index);
            preserve_raw_source_fields.push_back(true);
        }
    }

    return rewrite_dbf_table_schema(
        path,
        table_result.table,
        fields,
        source_field_indices,
        preserve_raw_source_fields);
}

DbfWriteResult alter_dbf_table_field(const std::string& path, const DbfFieldDescriptor& field) {
    if (const auto error = ambiguous_required_sidecar_error_for_path(path); error.has_value()) {
        return {.ok = false, .error = *error};
    }
    if (const auto error = dbase_read_only_mutation_error(path); error.has_value()) {
        return *error;
    }
    const DbfParseResult header_result = parse_dbf_header_from_file(path);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
    }

    DbfTableParseResult table_result = parse_dbf_table_from_file(path, header_result.header.record_count);
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }

    const std::string normalized_field_name = lowercase_copy(trim_both(field.name));
    const auto existing = std::find_if(
        table_result.table.fields.begin(),
        table_result.table.fields.end(),
        [&](const DbfFieldDescriptor& candidate) {
            return lowercase_copy(trim_both(candidate.name)) == normalized_field_name;
    });
    if (existing == table_result.table.fields.end()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.TargetFieldNotFound"), .record_count = table_result.table.records.size()};
    }

    std::vector<DbfFieldDescriptor> fields = table_result.table.fields;
    const std::size_t field_index = static_cast<std::size_t>(std::distance(table_result.table.fields.begin(), existing));
    fields[field_index] = field;

    std::vector<std::optional<std::size_t>> source_field_indices;
    source_field_indices.reserve(fields.size());
    std::vector<bool> preserve_raw_source_fields;
    preserve_raw_source_fields.reserve(fields.size());
    for (std::size_t index = 0U; index < fields.size(); ++index) {
        source_field_indices.push_back(index);
        preserve_raw_source_fields.push_back(index != field_index);
    }

    return rewrite_dbf_table_schema(
        path,
        table_result.table,
        fields,
        source_field_indices,
        preserve_raw_source_fields);
}

DbfWriteResult append_blank_record_to_file_full_rewrite(const std::string& path) {
    if (const auto error = ambiguous_required_sidecar_error_for_path(path); error.has_value()) {
        return {.ok = false, .error = *error};
    }
    if (const auto error = dbase_read_only_mutation_error(path); error.has_value()) {
        return *error;
    }
    std::ifstream input(platform::path_from_utf8_string(path), std::ios::binary);
    if (!input) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.OpenTableFailed")};
    }

    std::vector<std::uint8_t> bytes = {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
    input.close();

    const DbfParseResult header_result = parse_dbf_header(bytes);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
    }
    const std::vector<RawFieldDescriptor> fields = read_raw_field_descriptors(bytes);
    if (const auto error = ambiguous_table_sidecar_error(
            path, header_result.header, fields); error.has_value()) {
        return {.ok = false, .error = *error, .record_count = header_result.header.record_count};
    }
    DbfWriteResult result = append_blank_record_bytes(bytes, header_result.header, fields);
    if (!result.ok) {
        return result;
    }
    if (!stamp_dbf_last_update_date(bytes) || !write_binary_file(path, bytes)) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.WriteTableFailed"), .record_count = header_result.header.record_count};
    }
    return result;
}

DbfWriteResult append_blank_record_to_file(const std::string& path) {
    if (const auto error = dbase_read_only_mutation_error(path); error.has_value()) {
        return *error;
    }
    if (const auto fast_result = append_blank_record_to_file_targeted(path); fast_result.has_value()) {
        return *fast_result;
    }
    return append_blank_record_to_file_full_rewrite(path);
}

// Fast targeted-I/O path for the common case: a non-memo field write to an
// existing record. Reads only the fixed header plus field-descriptor region
// (bounded by field count, not record count) and the single target record's
// bytes -- not the whole file -- then writes back only those same
// record-length bytes plus the header's 3-byte last-update stamp. This
// avoids reading and rewriting the entire table on every write, which made
// writing N records one at a time (REPLACE inside a SCAN loop, or the
// DIF/SYLK/spreadsheet import loops' per-row/per-field replace calls) O(n^2)
// in total I/O instead of O(n).
//
// A crash mid-write can therefore only ever tear the single record being
// written -- never the header, never any other record, never the file's
// structure -- matching real xBase engines' traditional per-record write
// durability rather than this codebase's previous (and still-used, for the
// memo case below) whole-file-atomic-rewrite-per-write guarantee.
//
// Returns std::nullopt for anything this fast path doesn't handle -- a memo
// field (which needs the sidecar/rollback logic in
// replace_record_field_value_impl), an out-of-range record, or any
// structural read failure -- so the full-rewrite implementation remains the
// single source of truth for every case this fast path doesn't explicitly
// claim.
std::optional<DbfWriteResult> replace_record_field_value_targeted(
    const std::string& path,
    std::size_t record_index,
    const std::string& field_name,
    const std::string& value,
    bool allow_truncation) {
    if (const auto sidecar_error = ambiguous_required_sidecar_error_for_path(path); sidecar_error.has_value()) {
        return DbfWriteResult{.ok = false, .error = *sidecar_error};
    }

    const DbfParseResult header_result = parse_dbf_header_from_file(path);
    if (!header_result.ok) {
        return std::nullopt;
    }
    const DbfHeader& header = header_result.header;
    if (record_index >= header.record_count) {
        return DbfWriteResult{.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordIndexOutOfRange"), .record_count = header.record_count};
    }

    std::ifstream descriptor_input(platform::path_from_utf8_string(path), std::ios::binary);
    if (!descriptor_input) {
        return std::nullopt;
    }
    std::vector<std::uint8_t> header_bytes(header.header_length, 0U);
    descriptor_input.read(reinterpret_cast<char*>(header_bytes.data()), static_cast<std::streamsize>(header_bytes.size()));
    if (descriptor_input.gcount() != static_cast<std::streamsize>(header_bytes.size())) {
        return std::nullopt;
    }
    descriptor_input.close();

    const std::vector<RawFieldDescriptor> fields = read_raw_field_descriptors(header_bytes);
    if (const auto error = ambiguous_table_sidecar_error(path, header, fields); error.has_value()) {
        return DbfWriteResult{.ok = false, .error = *error, .record_count = header.record_count};
    }
    const auto field = find_raw_field(fields, field_name);
    if (!field.has_value()) {
        return DbfWriteResult{.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.TargetFieldNotFoundInTable"), .record_count = header.record_count};
    }
    if (is_memo_pointer_field(field->type)) {
        return std::nullopt;
    }

    const std::size_t record_offset = static_cast<std::size_t>(header.header_length) + (record_index * header.record_length);

    std::fstream io(platform::path_from_utf8_string(path), std::ios::binary | std::ios::in | std::ios::out);
    if (!io) {
        return std::nullopt;
    }

    std::vector<std::uint8_t> record_bytes(header.record_length, 0U);
    io.seekg(static_cast<std::streamoff>(record_offset));
    io.read(reinterpret_cast<char*>(record_bytes.data()), static_cast<std::streamsize>(record_bytes.size()));
    if (io.gcount() != static_cast<std::streamsize>(record_bytes.size())) {
        return std::nullopt;
    }

    // write_field_bytes() derives its write offset from
    // header_length + record_index * record_length + field.offset. Feed it a
    // header describing a single-record buffer (header_length 0, one
    // record) so that math lands correctly within record_bytes -- this
    // reuses its exact tested field-encoding logic unchanged rather than
    // duplicating it for a targeted write. record_index is forced to 0 for
    // that buffer-offset math, so pass the real record_index separately as
    // diagnostic_record_index so an overflow error still names the actual
    // record being replaced instead of always reporting record 1.
    DbfHeader single_record_header = header;
    single_record_header.header_length = 0U;
    single_record_header.record_count = 1U;
    const DbfWriteResult write_result = write_field_bytes(
        record_bytes, single_record_header, 0U, *field, value, path, allow_truncation, record_index);
    if (!write_result.ok) {
        return DbfWriteResult{.ok = false, .error = write_result.error, .record_count = header.record_count};
    }

    io.clear();
    io.seekp(static_cast<std::streamoff>(record_offset));
    io.write(reinterpret_cast<const char*>(record_bytes.data()), static_cast<std::streamsize>(record_bytes.size()));
    if (!io.good()) {
        return std::nullopt;
    }

    std::vector<std::uint8_t> stamp_bytes(4U, 0U);
    io.clear();
    io.seekg(0);
    io.read(reinterpret_cast<char*>(stamp_bytes.data()), static_cast<std::streamsize>(stamp_bytes.size()));
    if (io.gcount() == static_cast<std::streamsize>(stamp_bytes.size()) && stamp_dbf_last_update_date(stamp_bytes)) {
        io.clear();
        io.seekp(1);
        io.write(reinterpret_cast<const char*>(stamp_bytes.data()) + 1, 3);
    }
    io.flush();
    if (!io.good()) {
        return std::nullopt;
    }

    return DbfWriteResult{.ok = true, .error = {}, .record_count = header.record_count};
}

static DbfWriteResult replace_record_field_value_impl(
    const std::string& path,
    std::size_t record_index,
    const std::string& field_name,
    const std::string& value,
    bool additive,
    bool allow_truncation) {
    SidecarPathResolution memo_resolution;
    if (primary_always_requires_memo_sidecar(path)) {
        memo_resolution = resolve_memo_sidecar_path(path);
        if (memo_resolution.ambiguous) {
            return {.ok = false, .error = ambiguous_sidecar_error(memo_resolution)};
        }
    }

    std::ifstream input(platform::path_from_utf8_string(path), std::ios::binary);
    if (!input) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.OpenTableFailed")};
    }

    std::vector<std::uint8_t> bytes = {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
    input.close();
    const std::vector<std::uint8_t> original_table_bytes = bytes;

    const DbfParseResult header_result = parse_dbf_header(bytes);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
    }
    const std::vector<RawFieldDescriptor> fields = read_raw_field_descriptors(bytes);
    if (const auto error = ambiguous_table_sidecar_error(
            path, header_result.header, fields); error.has_value()) {
        return {.ok = false, .error = *error, .record_count = header_result.header.record_count};
    }
    if (memo_resolution.requested_path.empty() &&
        table_uses_memo_sidecar(header_result.header, fields)) {
        memo_resolution = resolve_memo_sidecar_path(path);
    }
    const auto field = find_raw_field(fields, field_name);
    if (!field.has_value()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.TargetFieldNotFoundInTable"), .record_count = header_result.header.record_count};
    }
    if (record_index >= header_result.header.record_count) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordIndexOutOfRange"), .record_count = header_result.header.record_count};
    }

    DbfWriteResult result;
    std::vector<std::uint8_t> memo_bytes;
    std::vector<std::uint8_t> original_memo_bytes;
    std::string memo_path;
    bool had_memo_file = false;
    if (is_memo_pointer_field(field->type)) {
        if (memo_resolution.requested_path.empty()) {
            memo_resolution = resolve_memo_sidecar_path(path);
            if (memo_resolution.ambiguous) {
                return {
                    .ok = false,
                    .error = ambiguous_sidecar_error(memo_resolution),
                    .record_count = header_result.header.record_count
                };
            }
        }
        memo_path = selected_sidecar_path(memo_resolution);
        if (memo_path.empty()) {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.MemoSidecarPathMissing"), .record_count = header_result.header.record_count};
        }
        original_memo_bytes = read_binary_file(memo_path);
        had_memo_file = !original_memo_bytes.empty();
        memo_bytes = original_memo_bytes;
        const std::size_t field_offset =
            header_result.header.header_length +
            (record_index * header_result.header.record_length) +
            field->offset;
        if (additive && field->type == 'M') {
            if (field->length < 4U || field_offset + 4U > bytes.size()) {
                return {
                    .ok = false,
                    .error = dbf_table_text("Vfp.DbfTable.Error.MemoFieldWidthTooSmall"),
                    .record_count = header_result.header.record_count
                };
            }
            const std::uint32_t block_number = read_le_u32(bytes, field_offset);
            std::vector<std::uint8_t> appended_payload;
            if (block_number != 0U) {
                const MemoReader memo_reader(memo_path);
                const auto payload = memo_reader.read_block_raw(block_number);
                if (!payload.has_value()) {
                    return {
                        .ok = false,
                        .error = dbf_table_text("Vfp.DbfTable.Error.ReadMemoPayloadFailed"),
                        .record_count = header_result.header.record_count
                    };
                }
                appended_payload = *payload;
            }
            const DbfTextConversionResult encoded = encode_dbf_text(header_result.header.code_page_mark, value);
            if (!encoded.ok) {
                return {
                    .ok = false,
                    .error = dbf_table_text("Vfp.DbfTable.Error.TextEncodingConversionFailed"),
                    .record_count = header_result.header.record_count
                };
            }
            appended_payload.insert(appended_payload.end(), encoded.text.begin(), encoded.text.end());
            result = write_memo_field_bytes(
                bytes,
                field_offset,
                memo_bytes,
                appended_payload,
                header_result.header.record_count,
                block_number != 0U);
        } else {
            result = write_memo_field_text(
                bytes,
                header_result.header,
                field_offset,
                memo_bytes,
                value,
                header_result.header.record_count);
        }
    } else {
        result = write_field_bytes(bytes, header_result.header, record_index, *field, value, path, allow_truncation);
    }
    if (!result.ok) {
        return result;
    }
    if (!stamp_dbf_last_update_date(bytes) || !write_binary_file(path, bytes)) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.WriteTableFailed"), .record_count = header_result.header.record_count};
    }
    if (is_memo_pointer_field(field->type) && !write_binary_file(memo_path, memo_bytes)) {
        write_binary_file(path, original_table_bytes);
        if (had_memo_file) {
            write_binary_file(memo_path, original_memo_bytes);
        } else {
            std::error_code ignored;
            const std::filesystem::path native_memo_path = platform::path_from_utf8_string(memo_path);
            if (std::filesystem::is_regular_file(native_memo_path, ignored)) {
                std::filesystem::remove(native_memo_path, ignored);
            }
        }
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.WriteMemoSidecarFailed"), .record_count = header_result.header.record_count};
    }
    return result;
}

DbfWriteResult replace_record_field_value(
    const std::string& path,
    std::size_t record_index,
    const std::string& field_name,
    const std::string& value,
    bool allow_truncation) {
    if (const auto error = dbase_read_only_mutation_error(path); error.has_value()) {
        return *error;
    }
    if (const auto fast_result = replace_record_field_value_targeted(path, record_index, field_name, value, allow_truncation);
        fast_result.has_value()) {
        return *fast_result;
    }
    return replace_record_field_value_impl(path, record_index, field_name, value, false, allow_truncation);
}

DbfWriteResult replace_record_field_value_additive(
    const std::string& path,
    std::size_t record_index,
    const std::string& field_name,
    const std::string& value,
    bool allow_truncation) {
    if (const auto error = dbase_read_only_mutation_error(path); error.has_value()) {
        return *error;
    }
    // Additive only changes behavior for memo fields (byte-append rather
    // than replace); the fast path above never handles memo fields, so an
    // additive request always goes straight to the full-rewrite path, which
    // already implements the memo-append semantics.
    return replace_record_field_value_impl(path, record_index, field_name, value, true, allow_truncation);
}

DbfWriteResult replace_record_field_value_full_rewrite(
    const std::string& path,
    std::size_t record_index,
    const std::string& field_name,
    const std::string& value,
    bool allow_truncation) {
    if (const auto error = dbase_read_only_mutation_error(path); error.has_value()) {
        return *error;
    }
    return replace_record_field_value_impl(path, record_index, field_name, value, false, allow_truncation);
}

DbfWriteResult replace_record_field_value_additive_full_rewrite(
    const std::string& path,
    std::size_t record_index,
    const std::string& field_name,
    const std::string& value,
    bool allow_truncation) {
    if (const auto error = dbase_read_only_mutation_error(path); error.has_value()) {
        return *error;
    }
    return replace_record_field_value_impl(path, record_index, field_name, value, true, allow_truncation);
}

DbfWriteResult set_record_deleted_flag(
    const std::string& path,
    std::size_t record_index,
    bool deleted) {
    if (const auto error = ambiguous_required_sidecar_error_for_path(path); error.has_value()) {
        return {.ok = false, .error = *error};
    }
    if (const auto error = dbase_read_only_mutation_error(path); error.has_value()) {
        return *error;
    }
    std::ifstream input(platform::path_from_utf8_string(path), std::ios::binary);
    if (!input) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.OpenTableFailed")};
    }

    std::vector<std::uint8_t> bytes = {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
    input.close();

    const DbfParseResult header_result = parse_dbf_header(bytes);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
    }
    const std::vector<RawFieldDescriptor> fields = read_raw_field_descriptors(bytes);
    if (const auto error = ambiguous_table_sidecar_error(
            path, header_result.header, fields); error.has_value()) {
        return {.ok = false, .error = *error, .record_count = header_result.header.record_count};
    }
    if (record_index >= header_result.header.record_count) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordIndexOutOfRange"), .record_count = header_result.header.record_count};
    }

    const std::size_t record_offset = header_result.header.header_length + (record_index * header_result.header.record_length);
    if (record_offset >= bytes.size()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordDataTruncated"), .record_count = header_result.header.record_count};
    }

    bytes[record_offset] = deleted ? 0x2AU : 0x20U;
    if (!stamp_dbf_last_update_date(bytes) || !write_binary_file(path, bytes)) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.WriteTableFailed"), .record_count = header_result.header.record_count};
    }

    return {.ok = true, .error = {}, .record_count = header_result.header.record_count};
}

DbfWriteResult truncate_dbf_table_file(const std::string& path, std::size_t record_count) {
    if (const auto error = ambiguous_required_sidecar_error_for_path(path); error.has_value()) {
        return {.ok = false, .error = *error};
    }
    if (const auto error = dbase_read_only_mutation_error(path); error.has_value()) {
        return *error;
    }
    std::vector<std::uint8_t> bytes = read_binary_file(path);
    if (bytes.empty()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.OpenTableFailed")};
    }

    const DbfParseResult header_result = parse_dbf_header(bytes);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
    }
    const std::vector<RawFieldDescriptor> fields = read_raw_field_descriptors(bytes);
    if (const auto error = ambiguous_table_sidecar_error(
            path, header_result.header, fields); error.has_value()) {
        return {.ok = false, .error = *error, .record_count = header_result.header.record_count};
    }

    const auto& header = header_result.header;
    if (record_count > header.record_count) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RequestedRecordCountTooLarge"), .record_count = header.record_count};
    }
    if (header.header_length > bytes.size()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.TableHeaderTruncated"), .record_count = header.record_count};
    }

    const std::size_t new_size = header.header_length + (record_count * static_cast<std::size_t>(header.record_length)) + 1U;
    if (new_size > bytes.size()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordDataTruncated"), .record_count = header.record_count};
    }

    bytes.resize(new_size);
    bytes.back() = 0x1AU;
    write_le_u32(bytes, 4U, static_cast<std::uint32_t>(record_count));
    if (!stamp_dbf_last_update_date(bytes) || !write_binary_file(path, bytes)) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.WriteTableFailed"), .record_count = header.record_count};
    }

    return {.ok = true, .error = {}, .record_count = record_count};
}

DbfWriteResult pack_dbf_table_file(const std::string& path) {
    if (const auto error = ambiguous_required_sidecar_error_for_path(path); error.has_value()) {
        return {.ok = false, .error = *error};
    }
    if (const auto error = dbase_read_only_mutation_error(path); error.has_value()) {
        return *error;
    }
    std::vector<std::uint8_t> bytes = read_binary_file(path);
    if (bytes.empty()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.OpenTableFailed")};
    }

    const DbfParseResult header_result = parse_dbf_header(bytes);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
    }
    const std::vector<RawFieldDescriptor> fields = read_raw_field_descriptors(bytes);
    if (const auto error = ambiguous_table_sidecar_error(
            path, header_result.header, fields); error.has_value()) {
        return {.ok = false, .error = *error, .record_count = header_result.header.record_count};
    }

    const auto& header = header_result.header;
    const std::size_t data_start = header.header_length;
    const std::size_t original_record_count = header.record_count;
    const std::size_t required_size = data_start + (original_record_count * static_cast<std::size_t>(header.record_length));
    if (required_size > bytes.size()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordDataTruncated"), .record_count = original_record_count};
    }

    std::vector<std::uint8_t> packed;
    packed.reserve(bytes.size());
    packed.insert(packed.end(), bytes.begin(), bytes.begin() + static_cast<std::ptrdiff_t>(data_start));

    std::uint32_t kept_count = 0U;
    for (std::size_t record_index = 0U; record_index < original_record_count; ++record_index) {
        const std::size_t record_offset = data_start + (record_index * static_cast<std::size_t>(header.record_length));
        if (bytes[record_offset] == 0x2AU) {
            continue;
        }
        packed.insert(
            packed.end(),
            bytes.begin() + static_cast<std::ptrdiff_t>(record_offset),
            bytes.begin() + static_cast<std::ptrdiff_t>(record_offset + header.record_length));
        ++kept_count;
    }

    packed.push_back(0x1AU);
    write_le_u32(packed, 4U, kept_count);
    if (!stamp_dbf_last_update_date(packed) || !write_binary_file(path, packed)) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.WriteTableFailed"), .record_count = original_record_count};
    }

    return {.ok = true, .error = {}, .record_count = kept_count};
}

DbfWriteResult pack_dbf_memo_file(const std::string& path) {
    if (const auto error = ambiguous_required_sidecar_error_for_path(path); error.has_value()) {
        return {.ok = false, .error = *error};
    }
    if (const auto error = dbase_read_only_mutation_error(path); error.has_value()) {
        return *error;
    }
    const DbfParseResult header_result = parse_dbf_header_from_file(path);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
    }

    DbfTableParseResult table_result = parse_dbf_table_from_file(path, header_result.header.record_count);
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }

    bool has_memo_fields = false;
    for (const auto& field : table_result.table.fields) {
        if (is_memo_pointer_field(field.type)) {
            has_memo_fields = true;
            break;
        }
    }
    if (!has_memo_fields) {
        return {.ok = true, .error = {}, .record_count = table_result.table.records.size()};
    }

    std::vector<std::optional<std::size_t>> source_field_indices;
    source_field_indices.reserve(table_result.table.fields.size());
    std::vector<bool> preserve_raw_source_fields;
    preserve_raw_source_fields.reserve(table_result.table.fields.size());
    for (std::size_t index = 0U; index < table_result.table.fields.size(); ++index) {
        source_field_indices.push_back(index);
        preserve_raw_source_fields.push_back(true);
    }

    return rewrite_dbf_table_schema(
        path,
        table_result.table,
        table_result.table.fields,
        source_field_indices,
        preserve_raw_source_fields);
}

DbfWriteResult zap_dbf_table_file(const std::string& path) {
    if (const auto error = ambiguous_required_sidecar_error_for_path(path); error.has_value()) {
        return {.ok = false, .error = *error};
    }
    if (const auto error = dbase_read_only_mutation_error(path); error.has_value()) {
        return *error;
    }
    std::vector<std::uint8_t> bytes = read_binary_file(path);
    if (bytes.empty()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.OpenTableFailed")};
    }

    const DbfParseResult header_result = parse_dbf_header(bytes);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
    }
    const std::vector<RawFieldDescriptor> fields = read_raw_field_descriptors(bytes);
    if (const auto error = ambiguous_table_sidecar_error(
            path, header_result.header, fields); error.has_value()) {
        return {.ok = false, .error = *error, .record_count = header_result.header.record_count};
    }

    if (header_result.header.header_length > bytes.size()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.TableHeaderTruncated"), .record_count = header_result.header.record_count};
    }

    std::vector<std::uint8_t> truncated(
        bytes.begin(),
        bytes.begin() + static_cast<std::ptrdiff_t>(header_result.header.header_length));
    truncated.push_back(0x1AU);
    write_le_u32(truncated, 4U, 0U);
    if (!stamp_dbf_last_update_date(truncated) || !write_binary_file(path, truncated)) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.WriteTableFailed"), .record_count = header_result.header.record_count};
    }

    return {.ok = true, .error = {}, .record_count = 0U};
}

DbfWriteResult mark_dbf_table_has_production_index(const std::string& path) {
    if (const auto error = ambiguous_required_sidecar_error_for_path(path); error.has_value()) {
        return {.ok = false, .error = *error};
    }
    if (const auto error = dbase_read_only_mutation_error(path); error.has_value()) {
        return *error;
    }
    std::vector<std::uint8_t> bytes = read_binary_file(path);
    if (bytes.empty()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.OpenTableFailed")};
    }

    const DbfParseResult header_result = parse_dbf_header(bytes);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
    }
    if (bytes.size() <= 28U) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.TableHeaderTruncated"), .record_count = header_result.header.record_count};
    }

    bytes[28] = static_cast<std::uint8_t>(bytes[28] | 0x01U);
    if (!write_binary_file(path, bytes)) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.WriteTableFailed"), .record_count = header_result.header.record_count};
    }

    return {.ok = true, .error = {}, .record_count = header_result.header.record_count};
}

}  // namespace copperfin::vfp
