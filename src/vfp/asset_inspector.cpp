// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/vfp/asset_inspector.h"
#include "copperfin/localization/localization.h"
#include "copperfin/platform/path.h"
#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/sidecar_path.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <locale>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <sstream>
#include <string_view>
#include <vector>

namespace copperfin::vfp {

namespace {

std::string asset_inspector_text(
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
        localization::load_catalogs(localization::resolve_catalog_root(), localization::default_locale)
    };

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

std::string lowercase_extension(const std::filesystem::path& path) {
    std::string ext = copperfin::platform::path_to_utf8_string(path.extension());
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return ext;
}

std::string trim_copy(std::string value) {
    const auto first = std::find_if(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    });
    value.erase(value.begin(), first);

    const auto last = std::find_if(value.rbegin(), value.rend(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    });
    value.erase(last.base(), value.end());
    return value;
}

std::string lowercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

const std::string* find_byte_override(
    const AssetByteOverrides* byte_overrides,
    const std::filesystem::path& path) {
    if (byte_overrides == nullptr) {
        return nullptr;
    }

    const std::string normalized = copperfin::platform::path_to_utf8_string(path.lexically_normal());
    const auto exact = byte_overrides->find(normalized);
    if (exact != byte_overrides->end()) {
        return &exact->second;
    }

#if defined(_WIN32)
    const std::string folded = lowercase_copy(normalized);
    const auto folded_match = std::find_if(
        byte_overrides->begin(),
        byte_overrides->end(),
        [&](const auto& candidate) {
            return lowercase_copy(candidate.first) == folded;
        });
    if (folded_match != byte_overrides->end()) {
        return &folded_match->second;
    }
#endif

    return nullptr;
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

    const std::string target_name = lowercase_copy(
        copperfin::platform::path_to_utf8_string(candidate.filename()));
    for (const auto& entry : std::filesystem::directory_iterator(directory, ignored)) {
        if (ignored) {
            break;
        }
        if (lowercase_copy(copperfin::platform::path_to_utf8_string(entry.path().filename())) == target_name) {
            return entry.path();
        }
    }

    return std::nullopt;
}

std::optional<std::filesystem::path> resolve_first_existing_path(
    const std::vector<std::filesystem::path>& candidates) {
    for (const auto& candidate : candidates) {
        if (const auto resolved = resolve_existing_path_casefold(candidate); resolved.has_value()) {
            return resolved;
        }
    }
    return std::nullopt;
}

void append_validation_issue(
    AssetInspectionResult& result,
    AssetValidationSeverity severity,
    std::string code,
    std::string path,
    std::string message);

bool is_dbf_family_asset(AssetFamily family) {
    switch (family) {
        case AssetFamily::project:
        case AssetFamily::form:
        case AssetFamily::class_library:
        case AssetFamily::report:
        case AssetFamily::label:
        case AssetFamily::menu:
        case AssetFamily::table:
        case AssetFamily::database_container:
            return true;
        case AssetFamily::index:
        case AssetFamily::unknown:
        case AssetFamily::program:
        case AssetFamily::header:
            return false;
    }
    return false;
}

bool is_index_extension(const std::string& extension) {
    return extension == ".cdx" || extension == ".dcx" || extension == ".idx" ||
           extension == ".ndx" || extension == ".mdx";
}

std::uint32_t read_le_u32(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint32_t>(bytes[offset]) |
           (static_cast<std::uint32_t>(bytes[offset + 1U]) << 8U) |
           (static_cast<std::uint32_t>(bytes[offset + 2U]) << 16U) |
           (static_cast<std::uint32_t>(bytes[offset + 3U]) << 24U);
}

std::uint16_t read_be_u16(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(bytes[offset]) << 8U) |
                                      static_cast<std::uint16_t>(bytes[offset + 1U]));
}

std::uint32_t read_be_u32(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return (static_cast<std::uint32_t>(bytes[offset]) << 24U) |
           (static_cast<std::uint32_t>(bytes[offset + 1U]) << 16U) |
           (static_cast<std::uint32_t>(bytes[offset + 2U]) << 8U) |
           static_cast<std::uint32_t>(bytes[offset + 3U]);
}

std::vector<std::uint8_t> read_binary_file(const std::string& path) {
    std::ifstream input(copperfin::platform::path_from_utf8_string(path), std::ios::binary);
    if (!input) {
        return {};
    }

    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

std::string read_ascii_name(const std::vector<std::uint8_t>& bytes, std::size_t offset, std::size_t length) {
    std::string value;
    value.reserve(length);
    for (std::size_t index = 0; index < length && (offset + index) < bytes.size(); ++index) {
        const std::uint8_t raw = bytes[offset + index];
        if (raw == 0U) {
            break;
        }
        value.push_back(static_cast<char>(raw));
    }

    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.pop_back();
    }
    return value;
}

struct RawFieldDescriptor {
    std::string name;
    char type = '\0';
    std::uint32_t offset = 0;
    std::uint8_t length = 0;
};

std::vector<RawFieldDescriptor> read_raw_field_descriptors(const std::vector<std::uint8_t>& table_bytes) {
    std::vector<RawFieldDescriptor> fields;
    std::size_t descriptor_offset = 32U;
    while ((descriptor_offset + 32U) <= table_bytes.size() && table_bytes[descriptor_offset] != 0x0DU) {
        fields.push_back({
            .name = read_ascii_name(table_bytes, descriptor_offset, 11U),
            .type = static_cast<char>(table_bytes[descriptor_offset + 11U]),
            .offset = read_le_u32(table_bytes, descriptor_offset + 12U),
            .length = table_bytes[descriptor_offset + 16U]
        });
        descriptor_offset += 32U;
    }
    return fields;
}

std::optional<std::size_t> find_aligned_field_descriptor_terminator(
    const std::vector<std::uint8_t>& table_bytes,
    std::size_t descriptor_region_end) {
    descriptor_region_end = std::min(descriptor_region_end, table_bytes.size());
    if (descriptor_region_end <= 32U) {
        return std::nullopt;
    }

    std::size_t offset = 32U;
    while (offset < descriptor_region_end) {
        if (table_bytes[offset] == 0x0DU) {
            return offset;
        }
        if ((descriptor_region_end - offset) <= 32U) {
            break;
        }
        offset += 32U;
    }
    return std::nullopt;
}

bool is_valid_field_name_char(char ch) {
    const auto raw = static_cast<unsigned char>(ch);
    return std::isalnum(raw) != 0 || ch == '_';
}

std::string uppercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return value;
}

const DbfRecordValue* find_record_value(const DbfRecord& record, std::initializer_list<const char*> names) {
    for (const char* name : names) {
        const std::string target = uppercase_copy(name == nullptr ? std::string{} : std::string(name));
        const auto found = std::find_if(record.values.begin(), record.values.end(), [&](const DbfRecordValue& value) {
            return uppercase_copy(value.field_name) == target;
        });
        if (found != record.values.end()) {
            return &(*found);
        }
    }

    return nullptr;
}

std::string canonical_dbc_object_type(std::string value) {
    value = uppercase_copy(trim_copy(std::move(value)));
    if (value.empty()) {
        return {};
    }

    if (value.find("DATABASE") != std::string::npos) {
        return "database";
    }
    if (value.find("TABLE") != std::string::npos) {
        return "table";
    }
    if (value.find("VIEW") != std::string::npos) {
        return "view";
    }
    if (value.find("RELATION") != std::string::npos) {
        return "relation";
    }
    if (value.find("CONNECTION") != std::string::npos) {
        return "connection";
    }

    return lowercase_copy(value);
}

void extract_database_container_metadata(
    AssetInspectionResult& result,
    const std::string& path,
    const DbfHeader& header) {
    if (header.record_count == 0U) {
        return;
    }

    const DbfTableParseResult table_result = parse_dbf_table_from_file(path, header.record_count);
    if (!table_result.ok) {
        append_validation_issue(
            result,
            AssetValidationSeverity::warning,
            "dbc.catalog_parse_failed",
            path,
            asset_inspector_text(
                "Vfp.AssetInspector.Validation.DbcCatalogParseFailed",
                {{"error", table_result.error}}));
        return;
    }

    DatabaseContainerMetadata metadata;
    constexpr std::size_t preview_limit = 32U;
    for (const DbfRecord& record : table_result.table.records) {
        if (record.deleted) {
            continue;
        }

        const DbfRecordValue* type_value = find_record_value(record, {"OBJECTTYPE", "OBJTYPE", "TYPE"});
        const DbfRecordValue* name_value = find_record_value(record, {"OBJECTNAME", "OBJNAME", "NAME", "OBJECT"});
        const DbfRecordValue* parent_value = find_record_value(record, {"PARENTNAME", "PARENT", "PARENTID"});

        const std::string object_type_hint = type_value == nullptr ? std::string{} : canonical_dbc_object_type(type_value->display_value);
        const std::string object_name_hint = name_value == nullptr ? std::string{} : trim_copy(name_value->display_value);
        const std::string parent_name_hint = parent_value == nullptr ? std::string{} : trim_copy(parent_value->display_value);

        if (object_type_hint.empty() && object_name_hint.empty() && parent_name_hint.empty()) {
            continue;
        }

        ++metadata.total_objects;
        if (object_type_hint == "database") {
            ++metadata.database_objects;
        } else if (object_type_hint == "table") {
            ++metadata.table_objects;
        } else if (object_type_hint == "view") {
            ++metadata.view_objects;
        } else if (object_type_hint == "relation") {
            ++metadata.relation_objects;
        } else if (object_type_hint == "connection") {
            ++metadata.connection_objects;
        }

        if (metadata.objects_preview.size() < preview_limit) {
            metadata.objects_preview.push_back({
                .record_index = record.record_index,
                .deleted = record.deleted,
                .object_type_hint = object_type_hint,
                .object_name_hint = object_name_hint,
                .parent_name_hint = parent_name_hint
            });
        }
    }

    if (metadata.total_objects == 0U) {
        append_validation_issue(
            result,
            AssetValidationSeverity::warning,
            "dbc.catalog_empty",
            path,
            asset_inspector_text("Vfp.AssetInspector.Validation.DbcCatalogEmpty"));
        return;
    }

    metadata.available = true;
    result.database_container_metadata_available = true;
    result.database_container_metadata = std::move(metadata);
}

SidecarPathResolution memo_sidecar_resolution_for(
    const std::filesystem::path& path,
    const std::string& explicit_memo_sidecar_path) {
    return explicit_memo_sidecar_path.empty()
        ? resolve_vfp_memo_sidecar_path(path)
        : resolve_unique_casefold_path(
              copperfin::platform::path_from_utf8_string(explicit_memo_sidecar_path));
}

bool asset_expects_memo_sidecar(AssetFamily family, const DbfHeader& header) {
    switch (family) {
        case AssetFamily::project:
        case AssetFamily::form:
        case AssetFamily::class_library:
        case AssetFamily::report:
        case AssetFamily::label:
        case AssetFamily::menu:
        case AssetFamily::database_container:
            return true;
        case AssetFamily::table:
            return header.has_memo_file();
        case AssetFamily::index:
        case AssetFamily::unknown:
        case AssetFamily::program:
        case AssetFamily::header:
            return false;
    }
    return false;
}

bool table_bytes_declare_memo_field(
    const std::vector<std::uint8_t>& table_bytes,
    const DbfHeader& header) {
    const std::size_t descriptor_limit = std::min<std::size_t>(
        table_bytes.size(),
        header.header_length);
    for (std::size_t offset = 32U; (offset + 32U) <= descriptor_limit; offset += 32U) {
        if (table_bytes[offset] == 0x0DU) {
            return false;
        }
        const char field_type = static_cast<char>(table_bytes[offset + 11U]);
        if (field_type == 'M' || field_type == 'G' || field_type == 'P') {
            return true;
        }
    }
    return false;
}

void append_if_missing(std::vector<std::string>& paths, const std::string& candidate) {
    if (candidate.empty()) {
        return;
    }

    const auto found = std::find(paths.begin(), paths.end(), candidate);
    if (found == paths.end()) {
        paths.push_back(candidate);
    }
}

std::vector<std::string> companion_index_paths_for(const std::filesystem::path& path, AssetFamily family) {
    std::vector<std::string> candidates;
    const std::string path_text = copperfin::platform::path_to_utf8_string(path);
    const auto with_extension = [&](const char* extension) {
        std::filesystem::path candidate = path;
        candidate.replace_extension(extension);
        return copperfin::platform::path_to_utf8_string(candidate);
    };

    switch (family) {
        case AssetFamily::table:
            append_if_missing(candidates, path_text + ".cdx");
            append_if_missing(candidates, path_text + ".idx");
            append_if_missing(candidates, path_text + ".ndx");
            append_if_missing(candidates, path_text + ".mdx");
            append_if_missing(candidates, with_extension(".cdx"));
            append_if_missing(candidates, with_extension(".idx"));
            append_if_missing(candidates, with_extension(".ndx"));
            append_if_missing(candidates, with_extension(".mdx"));
            return candidates;
        case AssetFamily::database_container:
            append_if_missing(candidates, copperfin::platform::path_to_utf8_string(path) + ".dcx");
            append_if_missing(candidates, with_extension(".dcx"));
            return candidates;
        case AssetFamily::project:
        case AssetFamily::form:
        case AssetFamily::class_library:
        case AssetFamily::report:
        case AssetFamily::label:
        case AssetFamily::menu:
        case AssetFamily::index:
        case AssetFamily::program:
        case AssetFamily::header:
        case AssetFamily::unknown:
            return candidates;
    }
    return candidates;
}

std::vector<std::string> expected_structural_companion_paths_for(const std::filesystem::path& path, AssetFamily family) {
    std::vector<std::string> candidates;
    const std::string path_text = copperfin::platform::path_to_utf8_string(path);
    const auto with_extension = [&](const char* extension) {
        std::filesystem::path candidate = path;
        candidate.replace_extension(extension);
        return copperfin::platform::path_to_utf8_string(candidate);
    };

    switch (family) {
        case AssetFamily::table:
            append_if_missing(candidates, path_text + ".cdx");
            append_if_missing(candidates, path_text + ".mdx");
            append_if_missing(candidates, with_extension(".cdx"));
            append_if_missing(candidates, with_extension(".mdx"));
            return candidates;
        case AssetFamily::database_container:
            append_if_missing(candidates, with_extension(".dcx"));
            return candidates;
        case AssetFamily::project:
        case AssetFamily::form:
        case AssetFamily::class_library:
        case AssetFamily::report:
        case AssetFamily::label:
        case AssetFamily::menu:
        case AssetFamily::index:
        case AssetFamily::program:
        case AssetFamily::header:
        case AssetFamily::unknown:
            return candidates;
    }
    return candidates;
}

bool any_existing_path(const std::vector<std::string>& candidates) {
    return std::any_of(candidates.begin(), candidates.end(), [](const std::string& candidate) {
        return resolve_existing_path_casefold(
            copperfin::platform::path_from_utf8_string(candidate)).has_value();
    });
}

void append_validation_issue(
    AssetInspectionResult& result,
    AssetValidationSeverity severity,
    std::string code,
    std::string path,
    std::string message) {
    result.validation_issues.push_back({
        .severity = severity,
        .code = std::move(code),
        .path = std::move(path),
        .message = std::move(message)
    });
}

bool has_validation_issue(
    const AssetInspectionResult& result,
    const std::string& code,
    const std::string& path) {
    return std::any_of(result.validation_issues.begin(), result.validation_issues.end(), [&](const AssetValidationIssue& issue) {
        return issue.code == code && issue.path == path;
    });
}

void validate_dbf_storage(
    AssetInspectionResult& result,
    const std::string& path,
    const DbfHeader& header,
    std::uint64_t file_size) {
    if (file_size < header.header_length) {
        append_validation_issue(
            result,
            AssetValidationSeverity::error,
            "dbf.header_length_exceeds_file_size",
            path,
            asset_inspector_text("Vfp.AssetInspector.Validation.DbfHeaderLengthExceedsFileSize"));
        return;
    }

    const std::uint64_t available_record_bytes = file_size - static_cast<std::uint64_t>(header.header_length);
    const std::uint64_t expected_record_bytes =
        static_cast<std::uint64_t>(header.record_count) * static_cast<std::uint64_t>(header.record_length);

    if (expected_record_bytes > available_record_bytes) {
        append_validation_issue(
            result,
            AssetValidationSeverity::error,
            "dbf.record_storage_truncated",
            path,
            asset_inspector_text("Vfp.AssetInspector.Validation.DbfRecordStorageTruncated"));
        return;
    }

    const std::uint64_t extra_bytes = available_record_bytes - expected_record_bytes;
    if (extra_bytes > 1U) {
        append_validation_issue(
            result,
            AssetValidationSeverity::warning,
            "dbf.record_storage_length_mismatch",
            path,
            asset_inspector_text("Vfp.AssetInspector.Validation.DbfRecordStorageLengthMismatch"));
    }
}

void validate_dbf_field_descriptors(
    AssetInspectionResult& result,
    const std::string& path,
    const DbfHeader& header,
    const std::vector<std::uint8_t>& table_bytes) {
    if (table_bytes.size() < 32U) {
        return;
    }

    const std::size_t descriptor_region_end = static_cast<std::size_t>(std::min<std::uint64_t>(header.header_length, table_bytes.size()));
    if (descriptor_region_end <= 32U) {
        append_validation_issue(
            result,
            AssetValidationSeverity::error,
            "dbf.descriptor_terminator_missing",
            path,
            asset_inspector_text("Vfp.AssetInspector.Validation.DbfDescriptorTerminatorNoRoom"));
        return;
    }

    const auto aligned_terminator =
        find_aligned_field_descriptor_terminator(table_bytes, descriptor_region_end);
    if (!aligned_terminator.has_value()) {
        const auto any_terminator = std::find(
            table_bytes.begin() + static_cast<std::ptrdiff_t>(32U),
            table_bytes.begin() + static_cast<std::ptrdiff_t>(descriptor_region_end),
            static_cast<std::uint8_t>(0x0DU));
        const bool has_misaligned_terminator =
            any_terminator != table_bytes.begin() + static_cast<std::ptrdiff_t>(descriptor_region_end);
        append_validation_issue(
            result,
            AssetValidationSeverity::error,
            has_misaligned_terminator
                ? "dbf.descriptor_span_misaligned"
                : "dbf.descriptor_terminator_missing",
            path,
            asset_inspector_text(
                has_misaligned_terminator
                    ? "Vfp.AssetInspector.Validation.DbfDescriptorSpanMisaligned"
                    : "Vfp.AssetInspector.Validation.DbfDescriptorTerminatorMissing"));
        return;
    }
    const std::size_t terminator_offset = *aligned_terminator;

    if ((terminator_offset + 1U) != static_cast<std::size_t>(header.header_length)) {
        append_validation_issue(
            result,
            AssetValidationSeverity::warning,
            "dbf.header_length_descriptor_mismatch",
            path,
            asset_inspector_text("Vfp.AssetInspector.Validation.DbfHeaderLengthDescriptorMismatch"));
    }

    const auto fields = read_raw_field_descriptors(std::vector<std::uint8_t>(table_bytes.begin(), table_bytes.begin() + static_cast<std::ptrdiff_t>(terminator_offset + 1U)));
    if (fields.empty()) {
        return;
    }

    std::set<std::string> seen_names;
    std::vector<RawFieldDescriptor> sorted_fields = fields;
    std::sort(sorted_fields.begin(), sorted_fields.end(), [](const RawFieldDescriptor& left, const RawFieldDescriptor& right) {
        if (left.offset != right.offset) {
            return left.offset < right.offset;
        }
        return left.name < right.name;
    });

    std::uint32_t computed_record_length = 1U;
    for (const RawFieldDescriptor& field : fields) {
        const std::string trimmed_name = field.name;
        if (trimmed_name.empty()) {
            append_validation_issue(
                result,
                AssetValidationSeverity::error,
                "dbf.field_name_blank",
                path,
                asset_inspector_text("Vfp.AssetInspector.Validation.DbfFieldNameBlank"));
        } else {
            const std::string normalized_name = uppercase_copy(trimmed_name);
            if (!seen_names.insert(normalized_name).second) {
                append_validation_issue(
                    result,
                    AssetValidationSeverity::error,
                    "dbf.field_name_duplicate",
                    path,
                    asset_inspector_text("Vfp.AssetInspector.Validation.DbfFieldNameDuplicate"));
            }

            const bool valid_name =
                trimmed_name.size() <= 10U &&
                (std::isalpha(static_cast<unsigned char>(trimmed_name.front())) != 0 || trimmed_name.front() == '_') &&
                std::all_of(trimmed_name.begin(), trimmed_name.end(), [](char ch) { return is_valid_field_name_char(ch); });
            if (!valid_name) {
                append_validation_issue(
                    result,
                    AssetValidationSeverity::warning,
                    "dbf.field_name_invalid",
                    path,
                    asset_inspector_text("Vfp.AssetInspector.Validation.DbfFieldNameInvalid"));
            }
        }

        if (field.offset < 1U) {
            append_validation_issue(
                result,
                AssetValidationSeverity::error,
                "dbf.field_offset_invalid",
                path,
                asset_inspector_text("Vfp.AssetInspector.Validation.DbfFieldOffsetInvalid"));
        }

        const std::uint32_t field_end = field.offset + static_cast<std::uint32_t>(field.length);
        if (field_end > header.record_length) {
            append_validation_issue(
                result,
                AssetValidationSeverity::error,
                "dbf.field_layout_overflow",
                path,
                asset_inspector_text("Vfp.AssetInspector.Validation.DbfFieldLayoutOverflow"));
        }

        computed_record_length += static_cast<std::uint32_t>(field.length);
    }

    for (std::size_t index = 1; index < sorted_fields.size(); ++index) {
        const std::uint32_t previous_end =
            sorted_fields[index - 1U].offset + static_cast<std::uint32_t>(sorted_fields[index - 1U].length);
        if (sorted_fields[index].offset < previous_end) {
            append_validation_issue(
                result,
                AssetValidationSeverity::error,
                "dbf.field_layout_overlap",
                path,
                asset_inspector_text("Vfp.AssetInspector.Validation.DbfFieldLayoutOverlap"));
            break;
        }
    }

    if (computed_record_length != header.record_length) {
        append_validation_issue(
            result,
            AssetValidationSeverity::warning,
            "dbf.record_length_mismatch",
            path,
            asset_inspector_text("Vfp.AssetInspector.Validation.DbfRecordLengthMismatch"));
    }
}

void validate_expected_companions(
    AssetInspectionResult& result,
    const std::string& path,
    AssetFamily family,
    const DbfHeader& header,
    const std::string& memo_sidecar_path) {
    const std::filesystem::path file_path = copperfin::platform::path_from_utf8_string(path);

    if (asset_expects_memo_sidecar(family, header)) {
        const SidecarPathResolution memo_resolution =
            memo_sidecar_resolution_for(file_path, memo_sidecar_path);
        const auto memo_path = memo_resolution.path.value_or(memo_resolution.requested_path);
        const std::string memo_path_text = copperfin::platform::path_to_utf8_string(memo_path);
        if (!memo_path_text.empty()) {
            if (memo_resolution.ambiguous) {
                append_validation_issue(
                    result,
                    AssetValidationSeverity::error,
                    "memo.sidecar_ambiguous",
                    copperfin::platform::path_to_utf8_string(memo_resolution.requested_path),
                    asset_inspector_text(
                        "Vfp.Sidecar.Error.AmbiguousPath",
                        {{"path", copperfin::platform::path_to_utf8_string(memo_resolution.requested_path)}}));
            } else if (!memo_resolution.path.has_value()) {
                append_validation_issue(
                    result,
                    AssetValidationSeverity::error,
                    "memo.sidecar_missing",
                    memo_path_text,
                    asset_inspector_text("Vfp.AssetInspector.Validation.MemoSidecarMissing"));
            }
        }
    }

    const bool expects_structural_index =
        (family == AssetFamily::database_container) ||
        (family == AssetFamily::table && header.has_production_index());
    if (!expects_structural_index) {
        return;
    }

    const auto structural_candidates = expected_structural_companion_paths_for(file_path, family);
    if (!any_existing_path(structural_candidates)) {
        append_validation_issue(
            result,
            AssetValidationSeverity::error,
            "index.structural_sidecar_missing",
            path,
            asset_inspector_text("Vfp.AssetInspector.Validation.IndexStructuralSidecarMissing"));
    }
}

void validate_memo_sidecar(
    AssetInspectionResult& result,
    const std::string& table_path,
    AssetFamily family,
    const DbfHeader& header,
    const std::vector<std::uint8_t>& table_bytes,
    const std::string& memo_sidecar_path) {
    if (!asset_expects_memo_sidecar(family, header)) {
        return;
    }

    const SidecarPathResolution memo_resolution = memo_sidecar_resolution_for(
        copperfin::platform::path_from_utf8_string(table_path),
        memo_sidecar_path);
    const auto memo_path = memo_resolution.path.value_or(
        memo_resolution.requested_path);
    const std::string memo_path_text = copperfin::platform::path_to_utf8_string(memo_path);
    if (memo_path_text.empty()) {
        return;
    }

    if (memo_resolution.ambiguous) {
        return;
    }
    const auto& resolved_memo_path = memo_resolution.path;
    if (!resolved_memo_path.has_value()) {
        return;
    }

    const std::string resolved_memo_path_text = copperfin::platform::path_to_utf8_string(*resolved_memo_path);
    const std::vector<std::uint8_t> memo_bytes = read_binary_file(resolved_memo_path_text);
    if (memo_bytes.size() < 8U) {
        append_validation_issue(
            result,
            AssetValidationSeverity::error,
            "memo.sidecar_header_truncated",
            resolved_memo_path_text,
            asset_inspector_text("Vfp.AssetInspector.Validation.MemoSidecarHeaderTruncated"));
        return;
    }

    const std::uint16_t block_size = read_be_u16(memo_bytes, 6U);
    if (block_size == 0U) {
        append_validation_issue(
            result,
            AssetValidationSeverity::error,
            "memo.block_size_invalid",
            resolved_memo_path_text,
            asset_inspector_text("Vfp.AssetInspector.Validation.MemoBlockSizeInvalid"));
        return;
    }

    if (memo_bytes.size() < block_size) {
        append_validation_issue(
            result,
            AssetValidationSeverity::error,
            "memo.sidecar_shorter_than_block_size",
            resolved_memo_path_text,
            asset_inspector_text("Vfp.AssetInspector.Validation.MemoSidecarShorterThanBlockSize"));
        return;
    }

    if (table_bytes.size() < header.header_length) {
        return;
    }

    const std::size_t descriptor_region_end = static_cast<std::size_t>(header.header_length);
    if (descriptor_region_end <= 32U) {
        return;
    }
    const auto descriptor_terminator_offset =
        find_aligned_field_descriptor_terminator(table_bytes, descriptor_region_end);
    if (!descriptor_terminator_offset.has_value()) {
        return;
    }
    const auto fields = read_raw_field_descriptors(std::vector<std::uint8_t>(
        table_bytes.begin(),
        table_bytes.begin() + static_cast<std::ptrdiff_t>(*descriptor_terminator_offset + 1U)));
    const bool has_memo_pointer_field =
        std::any_of(fields.begin(), fields.end(), [](const RawFieldDescriptor& field) {
            return field.type == 'M' || field.type == 'G' || field.type == 'P';
        });
    if (!has_memo_pointer_field) {
        return;
    }

    const std::uint64_t available_record_bytes = table_bytes.size() - static_cast<std::uint64_t>(header.header_length);
    const std::size_t readable_records = header.record_length == 0U
        ? 0U
        : static_cast<std::size_t>(std::min<std::uint64_t>(
            header.record_count,
            available_record_bytes / static_cast<std::uint64_t>(header.record_length)));

    std::set<std::uint32_t> checked_blocks;
    for (std::size_t record_index = 0; record_index < readable_records; ++record_index) {
        const std::size_t record_offset =
            static_cast<std::size_t>(header.header_length) + (record_index * static_cast<std::size_t>(header.record_length));
        for (const RawFieldDescriptor& field : fields) {
            if ((field.type != 'M' && field.type != 'G' && field.type != 'P') ||
                field.length < 4U ||
                field.offset < 1U ||
                field.offset > header.record_length ||
                4U > (header.record_length - field.offset)) {
                continue;
            }

            const std::size_t field_offset = record_offset + static_cast<std::size_t>(field.offset);
            if ((field_offset + 4U) > table_bytes.size()) {
                continue;
            }

            const std::uint32_t block_number = read_le_u32(table_bytes, field_offset);
            if (block_number == 0U || !checked_blocks.insert(block_number).second) {
                continue;
            }

            const std::uint64_t block_offset =
                static_cast<std::uint64_t>(block_number) * static_cast<std::uint64_t>(block_size);
            if ((block_offset + 8U) > memo_bytes.size()) {
                append_validation_issue(
                    result,
                    AssetValidationSeverity::error,
                    "memo.pointer_out_of_range",
                    resolved_memo_path_text,
                    asset_inspector_text("Vfp.AssetInspector.Validation.MemoPointerOutOfRange"));
                continue;
            }

            const std::uint32_t payload_length =
                read_be_u32(memo_bytes, static_cast<std::size_t>(block_offset + 4U));
            const std::uint64_t payload_end =
                block_offset + 8U + static_cast<std::uint64_t>(payload_length);
            if (payload_end > memo_bytes.size()) {
                append_validation_issue(
                    result,
                    AssetValidationSeverity::error,
                    "memo.payload_truncated",
                    resolved_memo_path_text,
                    asset_inspector_text("Vfp.AssetInspector.Validation.MemoPayloadTruncated"));
            }
        }
    }
}

}  // namespace

AssetFamily asset_family_from_path(const std::string& path) {
    const std::string ext = lowercase_extension(copperfin::platform::path_from_utf8_string(path));

    if (ext == ".pjx" || ext == ".pjt") {
        return AssetFamily::project;
    }
    if (ext == ".scx" || ext == ".sct") {
        return AssetFamily::form;
    }
    if (ext == ".vcx" || ext == ".vct") {
        return AssetFamily::class_library;
    }
    if (ext == ".frx" || ext == ".frt") {
        return AssetFamily::report;
    }
    if (ext == ".lbx" || ext == ".lbt") {
        return AssetFamily::label;
    }
    if (ext == ".mnx" || ext == ".mnt") {
        return AssetFamily::menu;
    }
    if (is_index_extension(ext)) {
        return AssetFamily::index;
    }
    if (ext == ".dbc") {
        return AssetFamily::database_container;
    }
    if (ext == ".dbf") {
        return AssetFamily::table;
    }
    if (ext == ".prg") {
        return AssetFamily::program;
    }
    if (ext == ".h") {
        return AssetFamily::header;
    }
    return AssetFamily::unknown;
}

const char* asset_family_name(AssetFamily family) {
    switch (family) {
        case AssetFamily::unknown:
            return "unknown";
        case AssetFamily::project:
            return "project";
        case AssetFamily::form:
            return "form";
        case AssetFamily::class_library:
            return "class_library";
        case AssetFamily::report:
            return "report";
        case AssetFamily::label:
            return "label";
        case AssetFamily::menu:
            return "menu";
        case AssetFamily::index:
            return "index";
        case AssetFamily::table:
            return "table";
        case AssetFamily::database_container:
            return "database_container";
        case AssetFamily::program:
            return "program";
        case AssetFamily::header:
            return "header";
    }
    return "unknown";
}

const char* asset_validation_severity_name(AssetValidationSeverity severity) {
    switch (severity) {
        case AssetValidationSeverity::warning:
            return "warning";
        case AssetValidationSeverity::error:
            return "error";
    }
    return "warning";
}

AssetInspectionResult inspect_asset(
    const std::string& path,
    const std::string& memo_sidecar_path,
    const AssetByteOverrides* byte_overrides) {
    AssetInspectionResult result;
    result.path = path;
    result.family = asset_family_from_path(path);
    const std::filesystem::path asset_path = copperfin::platform::path_from_utf8_string(path);

    std::error_code exists_error;
    if (!std::filesystem::exists(asset_path, exists_error) || exists_error) {
        result.ok = false;
        result.error = asset_inspector_text("Vfp.AssetInspector.Error.PathMissing");
        return result;
    }

    if (result.family == AssetFamily::index) {
        const auto* override_bytes = find_byte_override(
            byte_overrides,
            copperfin::platform::path_from_utf8_string(path));
        const IndexParseResult index_result = override_bytes == nullptr
            ? parse_index_probe_from_file(path)
            : parse_index_probe(
                std::vector<std::uint8_t>(override_bytes->begin(), override_bytes->end()),
                static_cast<std::uint64_t>(override_bytes->size()),
                index_kind_from_path(path));
        if (!index_result.ok) {
            result.ok = false;
            result.error = index_result.error;
            return result;
        }

        result.ok = true;
        result.indexes.push_back({.path = path, .probe = index_result.probe});
        return result;
    }

    if (!is_dbf_family_asset(result.family)) {
        result.ok = true;
        return result;
    }

    if (result.family != AssetFamily::table) {
        const SidecarPathResolution memo_resolution = memo_sidecar_resolution_for(
            asset_path,
            memo_sidecar_path);
        if (memo_resolution.ambiguous) {
            result.ok = false;
            result.error = asset_inspector_text(
                "Vfp.Sidecar.Error.AmbiguousPath",
                {{"path", copperfin::platform::path_to_utf8_string(memo_resolution.requested_path)}});
            return result;
        }
    }

    const DbfParseResult header_result = parse_dbf_header_from_file(path);
    if (!header_result.ok) {
        result.ok = false;
        result.error = header_result.error;
        return result;
    }

    if (result.family == AssetFamily::table && header_result.header.has_memo_file()) {
        const SidecarPathResolution memo_resolution = memo_sidecar_resolution_for(
            asset_path,
            memo_sidecar_path);
        if (memo_resolution.ambiguous) {
            result.ok = false;
            result.error = asset_inspector_text(
                "Vfp.Sidecar.Error.AmbiguousPath",
                {{"path", copperfin::platform::path_to_utf8_string(memo_resolution.requested_path)}});
            return result;
        }
    }

    result.ok = true;
    result.header_available = true;
    result.header = header_result.header;
    std::error_code file_size_error;
    const std::uintmax_t file_size_value = std::filesystem::file_size(asset_path, file_size_error);
    if (file_size_error) {
        result.ok = false;
        result.error = asset_inspector_text(
            "Vfp.AssetInspector.Error.ReadFailed",
            {{"path", path}});
        return result;
    }
    const std::uint64_t file_size = static_cast<std::uint64_t>(file_size_value);
    const std::vector<std::uint8_t> table_bytes = read_binary_file(path);

    if (result.family == AssetFamily::table &&
        table_bytes_declare_memo_field(table_bytes, result.header)) {
        const SidecarPathResolution memo_resolution = memo_sidecar_resolution_for(
            asset_path,
            memo_sidecar_path);
        if (memo_resolution.ambiguous) {
            result.ok = false;
            result.error = asset_inspector_text(
                "Vfp.Sidecar.Error.AmbiguousPath",
                {{"path", copperfin::platform::path_to_utf8_string(memo_resolution.requested_path)}});
            return result;
        }
    }

    validate_dbf_storage(result, path, result.header, file_size);
    validate_dbf_field_descriptors(result, path, result.header, table_bytes);
    validate_expected_companions(result, path, result.family, result.header, memo_sidecar_path);
    validate_memo_sidecar(result, path, result.family, result.header, table_bytes, memo_sidecar_path);

    if (result.family == AssetFamily::database_container) {
        extract_database_container_metadata(result, path, result.header);
    }

    for (const auto& companion_index : companion_index_paths_for(asset_path, result.family)) {
        const auto expected_companion_index = copperfin::platform::path_from_utf8_string(companion_index);
        const auto resolved_companion_index = resolve_existing_path_casefold(
            expected_companion_index);
        const std::filesystem::path inspection_index_path = resolved_companion_index.value_or(expected_companion_index);

        const auto* override_bytes = find_byte_override(
            byte_overrides,
            inspection_index_path);
        if (!resolved_companion_index.has_value() && override_bytes == nullptr) {
            continue;
        }

        const std::string resolved_companion_index_text =
            copperfin::platform::path_to_utf8_string(inspection_index_path);
        const IndexParseResult index_result = override_bytes == nullptr
            ? parse_index_probe_from_file(resolved_companion_index_text)
            : parse_index_probe(
                std::vector<std::uint8_t>(override_bytes->begin(), override_bytes->end()),
                static_cast<std::uint64_t>(override_bytes->size()),
                index_kind_from_path(resolved_companion_index_text));
        if (index_result.ok) {
            result.indexes.push_back({.path = resolved_companion_index_text, .probe = index_result.probe});
        } else if (!has_validation_issue(result, "index.companion_parse_failed", resolved_companion_index_text)) {
            append_validation_issue(
                result,
                AssetValidationSeverity::warning,
                "index.companion_parse_failed",
                resolved_companion_index_text,
                asset_inspector_text(
                    "Vfp.AssetInspector.Validation.IndexCompanionParseFailed",
                    {{"error", index_result.error}}));
        }
    }

    return result;
}

// ---- DBC properties binary decoder ----
//
// The VFP DBC PROPERTIES memo stores a binary property bag. Each entry:
//   Byte 0:      type code
//                  0x00 = end / padding — skip and continue
//                  0x01 = Character string  ('C')
//                  0x02 = Numeric           ('N')  — 8-byte IEEE 754 double LE
//                  0x03 = Logical           ('L')  — 1 byte (0=false, else true)
//                  0x04 = Date              ('D')  — 8 ASCII bytes YYYYMMDD
//                  0x05 = DateTime          ('T')  — 8 bytes (VFP internal)
//                  0x06 = Integer           ('I')  — 4-byte LE int32
//   Bytes 1–2:   2-byte LE name length
//   Bytes 3…:    name (ASCII, no null terminator)
//   Value (immediately after name):
//     0x01 (C): 2-byte LE value_length, then value_length ASCII bytes
//     0x02 (N): 8-byte IEEE 754 double LE
//     0x03 (L): 1 byte
//     0x04 (D): 8 ASCII bytes YYYYMMDD
//     0x05 (T): 8 bytes stored verbatim as hex for now
//     0x06 (I): 4-byte LE int32
//
// This format is reverse-engineered from community analysis of real .DBC files.
// Unknown type codes are preserved as hex strings so nothing is silently dropped.

namespace {

char vfp_type_for_code(std::uint8_t code) noexcept {
    switch (code) {
        case 0x01U: return 'C';
        case 0x02U: return 'N';
        case 0x03U: return 'L';
        case 0x04U: return 'D';
        case 0x05U: return 'T';
        case 0x06U: return 'I';
        default:    return '?';
    }
}

std::string hex_bytes(const std::vector<std::uint8_t>& blob, std::size_t offset, std::size_t length) {
    static constexpr std::array<char, 16U> kHex{
        '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    std::string out;
    out.reserve(2U * length);
    for (std::size_t i = 0U; i < length && (offset + i) < blob.size(); ++i) {
        const auto b = static_cast<std::uint8_t>(blob[offset + i]);
        out.push_back(kHex[(b >> 4U) & 0x0FU]);
        out.push_back(kHex[b & 0x0FU]);
    }
    return out;
}

std::vector<DbcProperty> decode_dbc_properties_blob(const std::vector<std::uint8_t>& blob) {
    std::vector<DbcProperty> props;
    std::size_t pos = 0U;

    while (pos < blob.size()) {
        const auto type_code = static_cast<std::uint8_t>(blob[pos]);

        if (type_code == 0x00U) {
            // Padding / end marker — skip this byte and keep scanning so we
            // don't prematurely stop at interior padding.
            ++pos;
            continue;
        }

        // Need type byte + 2-byte name length
        if (pos + 3U > blob.size()) {
            break;
        }

        const auto name_len = static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(blob[pos + 1U]) |
            (static_cast<std::uint16_t>(blob[pos + 2U]) << 8U));
        pos += 3U;

        if (name_len == 0U || pos + name_len > blob.size()) {
            break;
        }

        std::string name(reinterpret_cast<const char*>(blob.data() + pos), name_len);
        pos += name_len;

        DbcProperty prop;
        prop.name       = std::move(name);
        prop.type_hint  = vfp_type_for_code(type_code);

        switch (type_code) {
            case 0x01U: {  // Character — 2-byte LE length prefix
                if (pos + 2U > blob.size()) { goto done; }
                const auto val_len = static_cast<std::uint16_t>(
                    static_cast<std::uint16_t>(blob[pos]) |
                    (static_cast<std::uint16_t>(blob[pos + 1U]) << 8U));
                pos += 2U;
                if (pos + val_len > blob.size()) { goto done; }
                prop.value = std::string(reinterpret_cast<const char*>(blob.data() + pos), val_len);
                pos += val_len;
                break;
            }
            case 0x02U: {  // Numeric — 8-byte IEEE 754 double LE
                if (pos + 8U > blob.size()) { goto done; }
                double d = 0.0;
                std::memcpy(&d, blob.data() + pos, 8U);
                pos += 8U;
                std::ostringstream ss;
                ss.imbue(std::locale::classic());
                ss.precision(15);
                ss << d;
                prop.value = ss.str();
                break;
            }
            case 0x03U: {  // Logical — 1 byte
                if (pos + 1U > blob.size()) { goto done; }
                prop.value = (blob[pos] != 0U) ? "true" : "false";
                ++pos;
                break;
            }
            case 0x04U: {  // Date — 8 ASCII bytes YYYYMMDD
                if (pos + 8U > blob.size()) { goto done; }
                const std::string raw(reinterpret_cast<const char*>(blob.data() + pos), 8U);
                pos += 8U;
                // Format as YYYY-MM-DD if it looks like digits
                if (raw.size() == 8U &&
                    std::all_of(raw.begin(), raw.end(), [](unsigned char c) {
                        return std::isdigit(c) != 0;
                    })) {
                    prop.value = raw.substr(0U, 4U) + "-" + raw.substr(4U, 2U) + "-" + raw.substr(6U, 2U);
                } else {
                    prop.value = raw;
                }
                break;
            }
            case 0x05U: {  // DateTime — 8 bytes, emit as hex pending full decode
                if (pos + 8U > blob.size()) { goto done; }
                prop.value = hex_bytes(blob, pos, 8U);
                pos += 8U;
                break;
            }
            case 0x06U: {  // Integer — 4-byte LE int32
                if (pos + 4U > blob.size()) { goto done; }
                std::int32_t val = 0;
                std::memcpy(&val, blob.data() + pos, 4U);
                pos += 4U;
                prop.value = std::to_string(val);
                break;
            }
            default: {
                // Unknown type: store one byte as hex and advance past it so we
                // do not spin on the same byte forever.
                prop.value = "<type:0x" + hex_bytes(blob, pos > 0U ? pos - 1U : 0U, 1U) + ">";
                ++pos;
                break;
            }
        }

        props.push_back(std::move(prop));
    }

done:
    return props;
}

// Resolve and collect the raw PROPERTIES bytes for every record in a DBC.
// We read the DBF header/field layout ourselves so we can get the raw 4-byte
// memo block pointer from the PROPERTIES 'M' field without going through the
// text-mode decode path (which would strip binary content).
struct RawDbcRow {
    std::size_t record_index = 0;
    bool deleted = false;
    std::string object_type;
    std::string object_name;
    std::string parent_name;
    std::uint32_t properties_block = 0U;  // memo block number, 0 if absent
};

std::vector<RawDbcRow> read_raw_dbc_rows(
    const std::vector<std::uint8_t>& file_bytes,
    const DbfHeader& header) {

    std::vector<RawDbcRow> rows;
    if (file_bytes.size() < header.header_length) {
        return rows;
    }

    // Read field descriptors from the raw bytes
    std::vector<RawFieldDescriptor> raw_fields = read_raw_field_descriptors(file_bytes);

    // Find relevant field offsets
    const RawFieldDescriptor* f_type    = nullptr;
    const RawFieldDescriptor* f_name    = nullptr;
    const RawFieldDescriptor* f_parent  = nullptr;
    const RawFieldDescriptor* f_props   = nullptr;

    for (const auto& f : raw_fields) {
        const std::string upper_name = uppercase_copy(f.name);
        if (upper_name == "OBJECTTYPE" || upper_name == "OBJTYPE" || upper_name == "TYPE") {
            f_type = &f;
        } else if (upper_name == "OBJECTNAME" || upper_name == "OBJNAME" || upper_name == "NAME" || upper_name == "OBJECT") {
            f_name = &f;
        } else if (upper_name == "PARENTNAME" || upper_name == "PARENT" || upper_name == "PARENTID") {
            f_parent = &f;
        } else if (upper_name == "PROPERTIES" || upper_name == "PROPS") {
            f_props = &f;
        }
    }

    const std::size_t record_size = header.record_length;
    if (record_size == 0U) {
        return rows;
    }

    for (std::uint32_t rec_idx = 0U; rec_idx < header.record_count; ++rec_idx) {
        const std::size_t rec_offset =
            static_cast<std::size_t>(header.header_length) +
            static_cast<std::size_t>(rec_idx) * record_size;

        if (rec_offset + record_size > file_bytes.size()) {
            break;
        }

        const bool deleted = (file_bytes[rec_offset] == '*');

        auto read_char_field = [&](const RawFieldDescriptor* fd) -> std::string {
            if (fd == nullptr || fd->offset == 0U) {
                return {};
            }
            const std::size_t abs = rec_offset + fd->offset;
            if (abs + fd->length > file_bytes.size()) {
                return {};
            }
            std::string val(reinterpret_cast<const char*>(file_bytes.data() + abs), fd->length);
            // Trim trailing spaces and nulls
            while (!val.empty() &&
                   (val.back() == ' ' || val.back() == '\0')) {
                val.pop_back();
            }
            return val;
        };

        RawDbcRow row;
        row.record_index  = static_cast<std::size_t>(rec_idx) + 1U;
        row.deleted        = deleted;
        row.object_type    = canonical_dbc_object_type(read_char_field(f_type));
        row.object_name    = read_char_field(f_name);
        row.parent_name    = read_char_field(f_parent);

        // Extract PROPERTIES memo block number (4-byte LE in the 'M' field)
        if (f_props != nullptr && f_props->type == 'M' && f_props->length >= 4U) {
            const std::size_t abs = rec_offset + f_props->offset;
            if (abs + 4U <= file_bytes.size()) {
                row.properties_block = read_le_u32(file_bytes, abs);
            }
        }

        rows.push_back(std::move(row));
    }

    return rows;
}

std::string json_escape_str(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 4U);
    for (const char ch : s) {
        switch (ch) {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (static_cast<unsigned char>(ch) < 0x20U) {
                    // Control character — encode as \uXXXX
                    std::ostringstream esc;
                    esc << "\\u00"
                        << "0123456789ABCDEF"[(static_cast<unsigned char>(ch) >> 4U) & 0xFU]
                        << "0123456789ABCDEF"[static_cast<unsigned char>(ch) & 0xFU];
                    out += esc.str();
                } else {
                    out.push_back(ch);
                }
        }
    }
    return out;
}

}  // namespace (extended)

DatabaseExportResult export_database_as_json(
    const std::string& dbc_path,
    std::size_t max_rows_per_table) {

    namespace fs = std::filesystem;
    const fs::path dbc_fs_path = copperfin::platform::path_from_utf8_string(dbc_path);

    std::error_code dbc_status_error;
    if (!fs::exists(dbc_fs_path, dbc_status_error)) {
        return {
            .ok = false,
            .error = asset_inspector_text("Vfp.AssetInspector.Error.DbcPathMissing", {{"path", dbc_path}}),
            .json = {}
        };
    }

    const SidecarPathResolution dct_resolution = resolve_vfp_memo_sidecar_path(dbc_fs_path);
    if (dct_resolution.ambiguous) {
        return {
            .ok = false,
            .error = asset_inspector_text(
                "Vfp.Sidecar.Error.AmbiguousPath",
                {{"path", copperfin::platform::path_to_utf8_string(dct_resolution.requested_path)}}),
            .json = {}
        };
    }
    const std::optional<fs::path> dct_path = dct_resolution.path;
    const bool has_dct = dct_path.has_value();

    // Load the raw DBC bytes for direct field-pointer extraction
    const std::vector<std::uint8_t> dbc_bytes = read_binary_file(dbc_path);
    if (dbc_bytes.empty()) {
        return {
            .ok = false,
            .error = asset_inspector_text("Vfp.AssetInspector.Error.DbcReadFailed", {{"path", dbc_path}}),
            .json = {}
        };
    }

    const DbfParseResult header_result = parse_dbf_header(dbc_bytes);
    if (!header_result.ok) {
        return {
            .ok = false,
            .error = asset_inspector_text(
                "Vfp.AssetInspector.Error.DbcHeaderParseFailed",
                {{"error", header_result.error}}),
            .json = {}
        };
    }

    // Read all catalog rows with raw memo block numbers
    const std::vector<RawDbcRow> raw_rows = read_raw_dbc_rows(dbc_bytes, header_result.header);

    // Decode properties for each row and build DbcCatalogObject list
    std::vector<DbcCatalogObject> catalog;
    catalog.reserve(raw_rows.size());

    for (const auto& raw : raw_rows) {
        DbcCatalogObject obj;
        obj.record_index = raw.record_index;
        obj.deleted       = raw.deleted;
        obj.object_type   = raw.object_type;
        obj.object_name   = raw.object_name;
        obj.parent_name   = raw.parent_name;

        if (raw.properties_block != 0U && has_dct) {
            const std::vector<std::uint8_t> prop_bytes =
                read_memo_block_raw(
                    copperfin::platform::path_to_utf8_string(*dct_path),
                    raw.properties_block);
            if (!prop_bytes.empty()) {
                obj.properties = decode_dbc_properties_blob(prop_bytes);
            }
        }

        catalog.push_back(std::move(obj));
    }

    // Build JSON ----------------------------------------------------------
    std::ostringstream json;
    json << "{\n";

    // -- database metadata block
    std::string db_name = copperfin::platform::path_to_utf8_string(dbc_fs_path.stem());
    const auto database_object = std::find_if(
        catalog.begin(),
        catalog.end(),
        [](const DbcCatalogObject& object)
        {
            return !object.deleted && object.object_type == "database" && !trim_copy(object.object_name).empty();
        });
    if (database_object != catalog.end()) {
        db_name = database_object->object_name;
    }
    json << "  \"database\": {\n";
    json << "    \"path\": \"" << json_escape_str(dbc_path) << "\",\n";
    json << "    \"name\": \"" << json_escape_str(db_name) << "\"\n";
    json << "  },\n";

    // -- catalog array
    json << "  \"catalog\": [\n";
    std::size_t visible_count = 0U;
    for (const auto& obj : catalog) {
        if (obj.deleted) {
            continue;
        }
        ++visible_count;
    }

    std::size_t emitted = 0U;
    for (const auto& obj : catalog) {
        if (obj.deleted) {
            continue;
        }
        ++emitted;
        const bool last_obj = (emitted == visible_count);

        json << "    {\n";
        json << "      \"record_index\": " << obj.record_index << ",\n";
        json << "      \"object_type\": \"" << json_escape_str(obj.object_type) << "\",\n";
        json << "      \"object_name\": \"" << json_escape_str(obj.object_name) << "\",\n";
        json << "      \"parent_name\": \"" << json_escape_str(obj.parent_name) << "\",\n";
        json << "      \"properties\": {";

        if (!obj.properties.empty()) {
            json << "\n";
            for (std::size_t pi = 0U; pi < obj.properties.size(); ++pi) {
                const auto& prop = obj.properties[pi];
                const bool last_prop = (pi + 1U == obj.properties.size());
                json << "        \""
                     << json_escape_str(prop.name)
                     << "\": \""
                     << json_escape_str(prop.value)
                     << "\""
                     << (last_prop ? "\n" : ",\n");
            }
            json << "      }";
        } else {
            json << "}";
        }

        json << "\n    }" << (last_obj ? "\n" : ",\n");
    }
    json << "  ],\n";

    // -- tables block: row data for each TABLE catalog object
    json << "  \"tables\": {\n";

    // Build a list of (display_name, resolved_path) for tables whose .dbf exists
    const fs::path dbc_dir = dbc_fs_path.parent_path();
    struct ResolvedTable { std::string name; fs::path path; };
    std::vector<ResolvedTable> resolved_tables;

    for (const auto& obj : catalog) {
        if (obj.deleted || obj.object_type != "table" || obj.object_name.empty()) {
            continue;
        }
        const std::string& tname = obj.object_name;
        const auto make_table_path = [&](const std::string& name) {
            return dbc_dir / copperfin::platform::path_from_utf8_string(name);
        };
        const auto resolved_table_path = resolve_first_existing_path({
            make_table_path(tname + ".dbf"),
            make_table_path(lowercase_copy(tname) + ".dbf"),
            make_table_path(uppercase_copy(tname) + ".dbf")
        });
        if (!resolved_table_path.has_value()) {
            continue;
        }
        ResolvedTable resolved_table;
        resolved_table.name = tname;
        resolved_table.path = *resolved_table_path;
        resolved_tables.push_back(std::move(resolved_table));
    }

    for (std::size_t ti = 0U; ti < resolved_tables.size(); ++ti) {
        const auto& rt = resolved_tables[ti];
        const bool last_table = (ti + 1U == resolved_tables.size());

        const std::size_t row_limit = (max_rows_per_table == 0U)
                                          ? std::numeric_limits<std::size_t>::max()
                                          : max_rows_per_table;
        const DbfTableParseResult tbl = parse_dbf_table_from_file(
            copperfin::platform::path_to_utf8_string(rt.path),
            row_limit);
        if (!tbl.ok) {
            // Emit an empty entry rather than skipping to preserve comma correctness
            json << "    \"" << json_escape_str(rt.name) << "\": {\"fields\":[], \"records\":[]}"
                 << (last_table ? "\n" : ",\n");
            continue;
        }

        json << "    \"" << json_escape_str(rt.name) << "\": {\n";

        // fields array
        json << "      \"fields\": [\n";
        for (std::size_t fi = 0U; fi < tbl.table.fields.size(); ++fi) {
            const auto& fld = tbl.table.fields[fi];
            const bool last_field = (fi + 1U == tbl.table.fields.size());
            json << "        {\"name\": \""    << json_escape_str(fld.name)   << "\""
                 << ", \"type\": \""           << fld.type                     << "\""
                 << ", \"length\": "           << static_cast<int>(fld.length)
                 << ", \"decimals\": "         << static_cast<int>(fld.decimal_count)
                 << "}" << (last_field ? "\n" : ",\n");
        }
        json << "      ],\n";

        // Count non-deleted rows
        std::size_t visible_rows = 0U;
        for (const auto& rec : tbl.table.records) {
            if (!rec.deleted) { ++visible_rows; }
        }

        // records array
        json << "      \"records\": [\n";
        std::size_t row_emit = 0U;
        for (const auto& rec : tbl.table.records) {
            if (rec.deleted) { continue; }
            ++row_emit;
            const bool last_rec = (row_emit == visible_rows);

            json << "        {";
            for (std::size_t vi = 0U; vi < rec.values.size(); ++vi) {
                if (vi != 0U) { json << ", "; }
                const auto& rv = rec.values[vi];
                json << "\"" << json_escape_str(rv.field_name) << "\": ";
                bool is_numeric = false;
                bool is_logical = false;
                for (const auto& fd : tbl.table.fields) {
                    if (fd.name == rv.field_name) {
                        const char ft = static_cast<char>(
                            std::toupper(static_cast<unsigned char>(fd.type)));
                        is_numeric = (ft == 'N' || ft == 'F' || ft == 'I' || ft == 'B' || ft == 'Y');
                        is_logical = (ft == 'L');
                        break;
                    }
                }
                if (rv.is_null) {
                    json << "null";
                } else if (is_logical) {
                    const std::string& lv = rv.display_value;
                    json << ((lv == "true" || lv == "T" || lv == "t" ||
                              lv == "Y"    || lv == "y")
                             ? "true" : "false");
                } else if (is_numeric && !rv.display_value.empty()) {
                    json << rv.display_value;
                } else {
                    json << "\"" << json_escape_str(rv.display_value) << "\"";
                }
            }
            json << "}" << (last_rec ? "\n" : ",\n");
        }
        json << "      ]\n";

        json << "    }" << (last_table ? "\n" : ",\n");
    }

    json << "  }\n";
    json << "}\n";

    return {.ok = true, .error = {}, .json = json.str()};
}

}  // namespace copperfin::vfp
