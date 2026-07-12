// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/vfp/dbf_table.h"

#include "copperfin/localization/localization.h"
#include "copperfin/platform/environment.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
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

const localization::LocalizedCatalog& dbf_table_catalog() {
    static const localization::LocalizedCatalog catalog =
        localization::load_catalogs(localization::resolve_catalog_root(), localization::select_locale());
    return catalog;
}

std::string dbf_table_text(
    std::string_view key,
    const localization::PlaceholderMap& placeholders = {}) {
    return dbf_table_catalog().translate(key, placeholders);
}

std::string trim_right(std::string text) {
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0) {
        text.pop_back();
    }
    return text;
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

std::optional<std::filesystem::path> resolve_existing_path_casefold(const std::filesystem::path& candidate) {
    std::error_code ignored;
    if (std::filesystem::exists(candidate, ignored)) {
        return candidate;
    }

    const std::filesystem::path directory =
        candidate.has_parent_path() ? candidate.parent_path() : std::filesystem::current_path(ignored);
    if (directory.empty() || !std::filesystem::exists(directory, ignored)) {
        return std::nullopt;
    }

    const std::string target_name = lowercase_copy(candidate.filename().string());
    for (const auto& entry : std::filesystem::directory_iterator(directory, ignored)) {
        if (ignored) {
            break;
        }

        if (lowercase_copy(entry.path().filename().string()) == target_name) {
            return entry.path();
        }
    }

    return std::nullopt;
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
    if (!std::filesystem::is_regular_file(path, ignored)) {
        return {};
    }
    std::ifstream input(path, std::ios::binary);
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
        return path.find(*marker) != std::string::npos;
    };

    const std::filesystem::path target_path(path);
    const std::filesystem::path temp_path = target_path.string() + ".cptmp";
    const std::filesystem::path backup_path = target_path.string() + ".cpbak";

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

std::string infer_memo_sidecar_path(const std::string& path) {
    std::filesystem::path file_path(path);
    const std::string ext = lowercase_copy(file_path.extension().string());

    auto resolve_sidecar = [&](std::string_view extension) {
        const auto candidate = file_path.replace_extension(extension).string();
        if (const auto resolved = resolve_existing_path_casefold(candidate); resolved.has_value()) {
            return resolved->string();
        }

        return candidate;
    };

    if (ext == ".scx") {
        return resolve_sidecar(".sct");
    }
    if (ext == ".vcx") {
        return resolve_sidecar(".vct");
    }
    if (ext == ".frx") {
        return resolve_sidecar(".frt");
    }
    if (ext == ".lbx") {
        return resolve_sidecar(".lbt");
    }
    if (ext == ".mnx") {
        return resolve_sidecar(".mnt");
    }
    if (ext == ".pjx") {
        return resolve_sidecar(".pjt");
    }
    if (ext == ".dbc") {
        return resolve_sidecar(".dct");
    }
    if (ext == ".dbf") {
        return resolve_sidecar(".fpt");
    }
    return {};
}

struct RawFieldDescriptor {
    std::string name;
    char type = '\0';
    std::uint32_t offset = 0;
    std::uint8_t length = 0;
    std::uint8_t decimal_count = 0;
};

std::vector<RawFieldDescriptor> read_raw_field_descriptors(const std::vector<std::uint8_t>& table_bytes) {
    std::vector<RawFieldDescriptor> fields;
    std::size_t descriptor_offset = 32U;
    while ((descriptor_offset + 32U) <= table_bytes.size() && table_bytes[descriptor_offset] != 0x0DU) {
        fields.push_back({
            .name = read_ascii_name(table_bytes, descriptor_offset, 11U),
            .type = static_cast<char>(table_bytes[descriptor_offset + 11U]),
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

    constexpr long long scale = 10000LL;
    if (whole > (std::numeric_limits<long long>::max() / scale)) {
        return std::nullopt;
    }

    long long scaled = (whole * scale) + fractional;
    if (negative) {
        scaled = -scaled;
    }
    return static_cast<std::int64_t>(scaled);
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

DbfWriteResult write_memo_field_bytes(
    std::vector<std::uint8_t>& table_bytes,
    std::size_t field_offset,
    std::vector<std::uint8_t>& memo_bytes,
    const std::string& value,
    std::size_t record_count) {
    return write_memo_field_bytes(
        table_bytes,
        field_offset,
        memo_bytes,
        std::vector<std::uint8_t>(value.begin(), value.end()),
        record_count,
        false);
}

DbfWriteResult write_field_bytes(
    std::vector<std::uint8_t>& table_bytes,
    const DbfHeader& header,
    std::size_t record_index,
    const RawFieldDescriptor& field,
    const std::string& value) {
    if (record_index >= header.record_count) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordIndexOutOfRange"), .record_count = header.record_count};
    }

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
            const std::string text = trim_right(value);
            if (text.size() > field.length) {
                return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.CharacterValueTooLarge"), .record_count = header.record_count};
            }
            std::copy(text.begin(), text.end(), table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset));
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
                    return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.NumericValueTooLarge"), .record_count = header.record_count};
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
                std::size_t consumed = 0U;
                try {
                    parsed = std::stod(text, &consumed);
                } catch (const std::exception&) {
                    return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.DoubleValueInvalid"), .record_count = header.record_count};
                }
                if (consumed != text.size()) {
                    return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.DoubleValueInvalid"), .record_count = header.record_count};
                }
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
                return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.VqValueTooLarge"), .record_count = header.record_count};
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
    const std::size_t record_offset = insert_offset;
    table_bytes[record_offset] = 0x20U;

    for (const auto& field : fields) {
        const std::size_t field_offset = record_offset + field.offset;
        if ((field_offset + field.length) > table_bytes.size()) {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordLayoutExceedsSize"), .record_count = header.record_count};
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
                std::fill_n(
                    table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset),
                    field.length,
                    static_cast<std::uint8_t>(0U));
                break;
            case 'I':
                std::fill_n(
                    table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset),
                    field.length,
                    static_cast<std::uint8_t>(0U));
                break;
            case 'Y':
            case 'T':
                std::fill_n(
                    table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset),
                    field.length,
                    static_cast<std::uint8_t>(0U));
                break;
            case 'M':
            case 'G':
            case 'P':
                std::fill_n(
                    table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset),
                    field.length,
                    static_cast<std::uint8_t>(0U));
                break;
            case 'V':
            case 'Q':
                std::fill_n(
                    table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset),
                    field.length,
                    static_cast<std::uint8_t>(0U));
                break;
            default:
                std::fill_n(
                    table_bytes.begin() + static_cast<std::ptrdiff_t>(field_offset),
                    field.length,
                    static_cast<std::uint8_t>(0U));
                break;
        }
    }

    table_bytes.push_back(0x1AU);
    write_le_u32(table_bytes, 4U, header.record_count + 1U);
    return {.ok = true, .error = {}, .record_count = header.record_count + 1U};
}

class MemoReader {
public:
    MemoReader() = default;

    explicit MemoReader(const std::string& path) {
        if (path.empty()) {
            return;
        }

        std::ifstream input(path, std::ios::binary);
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

        block_size_ = read_be_u16(bytes_, 6U);
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
        if ((offset + 8U) > bytes_.size()) {
            return std::nullopt;
        }

        const std::uint32_t length = read_be_u32(bytes_, static_cast<std::size_t>(offset + 4U));
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
    std::vector<std::uint8_t> bytes_;
    std::uint32_t block_size_ = 0;
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

std::string decode_value(
    char field_type,
    const std::vector<std::uint8_t>& raw,
    const MemoReader& memo_reader,
    bool& is_null,
    std::uint32_t& memo_block_number) {
    is_null = false;
    memo_block_number = 0U;
    if (raw.empty()) {
        return {};
    }

    switch (field_type) {
        case 'C': {
            return trim_right(std::string(raw.begin(), raw.end()));
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
        case 'I': {
            if (raw.size() < 4U) {
                return {};
            }
            const std::int32_t value = static_cast<std::int32_t>(read_le_u32(raw, 0U));
            return std::to_string(value);
        }
        case 'B': {
            if (raw.size() < 8U) {
                return {};
            }
            double value = 0.0;
            std::array<std::uint8_t, 8U> storage{};
            std::copy_n(raw.begin(), 8U, storage.begin());
            std::memcpy(&value, storage.data(), storage.size());
            std::ostringstream stream;
            stream.precision(15);
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
                return trim_right(std::move(value));
            }
            return value;
        }
        case 'T': {
            if (raw.size() < 8U) {
                return {};
            }
            const std::uint32_t julian_day = read_le_u32(raw, 0U);
            const std::uint32_t millis = read_le_u32(raw, 4U);
            std::ostringstream stream;
            stream << "julian:" << julian_day << " millis:" << millis;
            return stream.str();
        }
        case 'M':
        case 'G':
        case 'P': {
            if (raw.size() < 4U) {
                return {};
            }
            const std::uint32_t block_number = read_le_u32(raw, 0U);
            memo_block_number = block_number;
            if (block_number == 0U) {
                return {};
            }
            const auto memo_text = memo_reader.read_block(block_number);
            if (memo_text.has_value()) {
                return *memo_text;
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
        case '0': {
            is_null = true;
            return "NULL";
        }
        default:
            return format_binary_bytes(raw);
    }
}

}  // namespace

std::vector<std::uint8_t> read_memo_block_raw(const std::string& sidecar_path, std::uint32_t block_number) {
    if (sidecar_path.empty() || block_number == 0U) {
        return {};
    }

    std::ifstream input(sidecar_path, std::ios::binary);
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

DbfTableParseResult parse_dbf_table_from_file(
    const std::string& path,
    std::size_t max_records,
    const std::string& memo_sidecar_path) {
    std::ifstream input(path, std::ios::binary);
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

    if (bytes.size() < table.header.header_length) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.HeaderLengthExceedsFile")};
    }

    std::size_t field_offset = 32U;
    const std::size_t header_limit = table.header.header_length;
    while ((field_offset + 32U) <= bytes.size() &&
           (field_offset + 32U) <= header_limit &&
           bytes[field_offset] != 0x0DU) {
        DbfFieldDescriptor field;
        field.name = read_ascii_name(bytes, field_offset, 11U);
        field.type = static_cast<char>(bytes[field_offset + 11U]);
        field.offset = read_le_u32(bytes, field_offset + 12U);
        field.length = bytes[field_offset + 16U];
        field.decimal_count = bytes[field_offset + 17U];
        table.fields.push_back(std::move(field));
        field_offset += 32U;
    }

    const MemoReader memo_reader(
        memo_sidecar_path.empty() ? infer_memo_sidecar_path(path) : memo_sidecar_path);
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
            const std::string display_value = decode_value(field.type, raw, memo_reader, is_null, memo_block_number);
            record.values.push_back({
                .field_name = field.name,
                .field_type = field.type,
                .is_null = is_null,
                .display_value = display_value,
                .memo_block_number = memo_block_number
            });
        }

        table.records.push_back(std::move(record));
    }

    return {.ok = true, .table = std::move(table), .error = {}};
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

    const std::vector<std::uint8_t> table_bytes = read_binary_file(path);
    if (table_bytes.empty()) {
        result.error = dbf_table_text("Vfp.DbfTable.Error.OpenTableFailed");
        return result;
    }

    const MemoReader memo_reader(infer_memo_sidecar_path(path));
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
    const DbfCreateOverrides* overrides) {
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
        const std::string normalized_name = lowercase_copy(trimmed_name);
        const auto duplicate = std::find_if(
            raw_fields.begin(),
            raw_fields.end(),
            [&](const RawFieldDescriptor& existing) {
                return lowercase_copy(trim_both(existing.name)) == normalized_name;
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
        if (!supports_table_field_storage(field.type) && !preserves_opaque_field) {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.CreateUnsupportedFieldType")};
        }
        if (is_memo_pointer_field(field.type)) {
            if (field.length < 4U) {
                return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.MemoFieldWidthTooSmall")};
            }
            has_memo_fields = true;
        } else if ((field.type == 'V' || field.type == 'Q') && field.length < 2U) {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.VqFieldWidthTooSmall")};
        } else if (field.type == 'B' && field.length != 8U) {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.DoubleFieldWidthInvalid")};
        } else if (field.type == 'I' && field.length != 4U) {
            return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.IntegerFieldWidthInvalid")};
        } else if ((field.type == 'Y' || field.type == 'T') && field.length != 8U) {
            return {
                .ok = false,
                .error = dbf_table_text(
                    "Vfp.DbfTable.Error.EightByteFieldWidthInvalid",
                    {{"fieldType", std::string(1U, field.type)}})
            };
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

    std::size_t descriptor_offset = 32U;
    for (const auto& field : raw_fields) {
        const std::string field_name = field.name.substr(0U, 11U);
        std::copy(field_name.begin(), field_name.end(), bytes.begin() + static_cast<std::ptrdiff_t>(descriptor_offset));
        bytes[descriptor_offset + 11U] = static_cast<std::uint8_t>(field.type);
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
        .code_page_mark = 0U
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
                    write_result = write_memo_field_bytes(
                        bytes,
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
                    records[record_index][field_index]);
            }
            if (!write_result.ok) {
                return write_result;
            }
        }
    }

    const std::vector<std::uint8_t> original_table_bytes = read_binary_file(path);
    const bool had_table_file = !original_table_bytes.empty();
    const std::string memo_path = infer_memo_sidecar_path(path);
    const std::vector<std::uint8_t> original_memo_bytes = has_memo_fields ? read_binary_file(memo_path) : std::vector<std::uint8_t>{};
    const bool had_memo_file = has_memo_fields && !original_memo_bytes.empty();

    if (!write_binary_file(path, bytes)) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.WriteTableFailed"), .record_count = records.size()};
    }
    if (has_memo_fields && !write_binary_file(memo_path, memo_bytes)) {
        if (had_table_file) {
            write_binary_file(path, original_table_bytes);
        } else {
            std::error_code ignored;
            std::filesystem::remove(path, ignored);
        }

        if (had_memo_file) {
            write_binary_file(memo_path, original_memo_bytes);
        } else {
            std::error_code ignored;
            if (std::filesystem::is_regular_file(memo_path, ignored)) {
                std::filesystem::remove(memo_path, ignored);
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
    return create_dbf_table_file_with_memo_payloads(path, fields, rewrite_rows.records, &overrides);
}

DbfWriteResult create_dbf_table_file(
    const std::string& path,
    const std::vector<DbfFieldDescriptor>& fields,
    const std::vector<std::vector<std::string>>& records) {
    return create_dbf_table_file_with_memo_payloads(path, fields, records, nullptr);
}

DbfWriteResult add_dbf_table_field(const std::string& path, const DbfFieldDescriptor& field) {
    const DbfParseResult header_result = parse_dbf_header_from_file(path);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
    }

    DbfTableParseResult table_result = parse_dbf_table_from_file(path, header_result.header.record_count);
    if (!table_result.ok) {
        return {.ok = false, .error = table_result.error};
    }

    const std::string normalized_new_name = lowercase_copy(trim_both(field.name));
    if (normalized_new_name.empty()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.FieldNameRequired"), .record_count = table_result.table.records.size()};
    }
    const auto duplicate = std::find_if(
        table_result.table.fields.begin(),
        table_result.table.fields.end(),
        [&](const DbfFieldDescriptor& existing) {
            return lowercase_copy(trim_both(existing.name)) == normalized_new_name;
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

DbfWriteResult append_blank_record_to_file(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
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
    DbfWriteResult result = append_blank_record_bytes(bytes, header_result.header, fields);
    if (!result.ok) {
        return result;
    }
    if (!write_binary_file(path, bytes)) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.WriteTableFailed"), .record_count = header_result.header.record_count};
    }
    return result;
}

static DbfWriteResult replace_record_field_value_impl(
    const std::string& path,
    std::size_t record_index,
    const std::string& field_name,
    const std::string& value,
    bool additive) {
    std::ifstream input(path, std::ios::binary);
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
        memo_path = infer_memo_sidecar_path(path);
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
            appended_payload.insert(appended_payload.end(), value.begin(), value.end());
            result = write_memo_field_bytes(
                bytes,
                field_offset,
                memo_bytes,
                appended_payload,
                header_result.header.record_count,
                block_number != 0U);
        } else {
            result = write_memo_field_bytes(
                bytes,
                field_offset,
                memo_bytes,
                value,
                header_result.header.record_count);
        }
    } else {
        result = write_field_bytes(bytes, header_result.header, record_index, *field, value);
    }
    if (!result.ok) {
        return result;
    }
    if (!write_binary_file(path, bytes)) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.WriteTableFailed"), .record_count = header_result.header.record_count};
    }
    if (is_memo_pointer_field(field->type) && !write_binary_file(memo_path, memo_bytes)) {
        write_binary_file(path, original_table_bytes);
        if (had_memo_file) {
            write_binary_file(memo_path, original_memo_bytes);
        } else {
            std::error_code ignored;
            if (std::filesystem::is_regular_file(memo_path, ignored)) {
                std::filesystem::remove(memo_path, ignored);
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
    const std::string& value) {
    return replace_record_field_value_impl(path, record_index, field_name, value, false);
}

DbfWriteResult replace_record_field_value_additive(
    const std::string& path,
    std::size_t record_index,
    const std::string& field_name,
    const std::string& value) {
    return replace_record_field_value_impl(path, record_index, field_name, value, true);
}

DbfWriteResult set_record_deleted_flag(
    const std::string& path,
    std::size_t record_index,
    bool deleted) {
    std::ifstream input(path, std::ios::binary);
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
    if (record_index >= header_result.header.record_count) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordIndexOutOfRange"), .record_count = header_result.header.record_count};
    }

    const std::size_t record_offset = header_result.header.header_length + (record_index * header_result.header.record_length);
    if (record_offset >= bytes.size()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.RecordDataTruncated"), .record_count = header_result.header.record_count};
    }

    bytes[record_offset] = deleted ? 0x2AU : 0x20U;
    if (!write_binary_file(path, bytes)) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.WriteTableFailed"), .record_count = header_result.header.record_count};
    }

    return {.ok = true, .error = {}, .record_count = header_result.header.record_count};
}

DbfWriteResult truncate_dbf_table_file(const std::string& path, std::size_t record_count) {
    std::vector<std::uint8_t> bytes = read_binary_file(path);
    if (bytes.empty()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.OpenTableFailed")};
    }

    const DbfParseResult header_result = parse_dbf_header(bytes);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
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
    if (!write_binary_file(path, bytes)) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.WriteTableFailed"), .record_count = header.record_count};
    }

    return {.ok = true, .error = {}, .record_count = record_count};
}

DbfWriteResult pack_dbf_table_file(const std::string& path) {
    std::vector<std::uint8_t> bytes = read_binary_file(path);
    if (bytes.empty()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.OpenTableFailed")};
    }

    const DbfParseResult header_result = parse_dbf_header(bytes);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
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
    if (!write_binary_file(path, packed)) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.WriteTableFailed"), .record_count = original_record_count};
    }

    return {.ok = true, .error = {}, .record_count = kept_count};
}

DbfWriteResult pack_dbf_memo_file(const std::string& path) {
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
    std::vector<std::uint8_t> bytes = read_binary_file(path);
    if (bytes.empty()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.OpenTableFailed")};
    }

    const DbfParseResult header_result = parse_dbf_header(bytes);
    if (!header_result.ok) {
        return {.ok = false, .error = header_result.error};
    }

    if (header_result.header.header_length > bytes.size()) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.TableHeaderTruncated"), .record_count = header_result.header.record_count};
    }

    std::vector<std::uint8_t> truncated(
        bytes.begin(),
        bytes.begin() + static_cast<std::ptrdiff_t>(header_result.header.header_length));
    truncated.push_back(0x1AU);
    write_le_u32(truncated, 4U, 0U);
    if (!write_binary_file(path, truncated)) {
        return {.ok = false, .error = dbf_table_text("Vfp.DbfTable.Error.WriteTableFailed"), .record_count = header_result.header.record_count};
    }

    return {.ok = true, .error = {}, .record_count = 0U};
}

}  // namespace copperfin::vfp
