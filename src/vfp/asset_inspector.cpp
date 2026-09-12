// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/asset_inspector.h"
#include "copperfin/localization/localization.h"
#include "copperfin/platform/json.h"
#include "copperfin/platform/path.h"
#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/dbf_text_encoding.h"
#include "copperfin/vfp/index_probe.h"
#include "copperfin/vfp/sidecar_path.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <map>
#include <mutex>
#include <optional>
#include <random>
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
           extension == ".ndx" || extension == ".mdx" || extension == ".ntx";
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
            // Single-entry .idx candidates precede .cdx candidates, across both naming
            // styles, to match the documented KEY()/TAG()/ORDER()/TAGCOUNT() ordinal
            // enumeration order (single-entry index files first, then compound-index
            // tags); .ndx/.mdx keep their prior relative position pending real VFP9
            // evidence for where they belong in that ordinal sequence.
            append_if_missing(candidates, path_text + ".idx");
            append_if_missing(candidates, with_extension(".idx"));
            append_if_missing(candidates, path_text + ".cdx");
            append_if_missing(candidates, with_extension(".cdx"));
            append_if_missing(candidates, path_text + ".ndx");
            append_if_missing(candidates, with_extension(".ndx"));
            append_if_missing(candidates, path_text + ".mdx");
            append_if_missing(candidates, with_extension(".mdx"));
            append_if_missing(candidates, path_text + ".ntx");
            append_if_missing(candidates, with_extension(".ntx"));
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
        // #5697: an aligned terminator immediately at the descriptor start
        // parses "successfully" with zero fields -- a schema with no
        // usable columns, not a well-formed table. Previously this
        // returned silently with no diagnostic at all; the export/import
        // boundaries have their own independent fail-closed checks
        // (#5697), but this is the one place that inspects an ordinary
        // DBF/DBC member table's own structure and should surface the
        // defect directly rather than reporting a clean inspection.
        append_validation_issue(
            result,
            AssetValidationSeverity::error,
            "dbf.field_count_zero",
            path,
            asset_inspector_text("Vfp.AssetInspector.Validation.DbfFieldCountZero"));
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
    std::uint32_t code_block = 0U;  // CODE memo field's block number, 0 if absent -- see #5538
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
    const RawFieldDescriptor* f_code    = nullptr;

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
        } else if (upper_name == "CODE") {
            // #5538: real Visual FoxPro's DBC catalog stores Stored
            // Procedures source text in a memo field literally named
            // CODE, on a dedicated "StoredProceduresSource" row -- see
            // DbcStoredProceduresResult's own documented evidence.
            f_code = &f;
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
        // Extract CODE memo block number (4-byte LE in the 'M' field) -- see #5538.
        if (f_code != nullptr && f_code->type == 'M' && f_code->length >= 4U) {
            const std::size_t abs = rec_offset + f_code->offset;
            if (abs + 4U <= file_bytes.size()) {
                row.code_block = read_le_u32(file_bytes, abs);
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

std::string json_pointer_token(std::string_view token) {
    std::string encoded;
    encoded.reserve(token.size());
    for (const char character : token) {
        if (character == '~') {
            encoded += "~0";
        } else if (character == '/') {
            encoded += "~1";
        } else {
            encoded.push_back(character);
        }
    }
    return encoded;
}

bool parse_decimal_in_range(
    const std::string_view text,
    const std::size_t minimum,
    const std::size_t maximum,
    std::size_t& value) {
    if (text.empty()) {
        return false;
    }
    std::size_t parsed = 0U;
    for (const unsigned char character : text) {
        if (character < static_cast<unsigned char>('0') ||
            character > static_cast<unsigned char>('9')) {
            return false;
        }
        const std::size_t digit = static_cast<std::size_t>(character - static_cast<unsigned char>('0'));
        if (digit > maximum || parsed > (maximum - digit) / 10U) {
            return false;
        }
        parsed = parsed * 10U + digit;
    }
    if (parsed < minimum || parsed > maximum) {
        return false;
    }
    value = parsed;
    return true;
}

bool is_single_printable_ascii_character(const std::string& value) {
    return value.size() == 1U &&
           static_cast<unsigned char>(value.front()) >= 0x20U &&
           static_cast<unsigned char>(value.front()) <= 0x7EU;
}

// Defined later in this file (reused by the JSON import materializer); a
// forward declaration lets load_database_catalog_snapshot() below share
// the identical validation rather than duplicating or drifting from it.
bool table_name_is_safe_filesystem_component(const std::string& name);

// Shared by export_database_as_json() and export_database_as_sql(): loads
// and decodes the DBC catalog, derives the database display name, and
// resolves which catalog TABLE objects have an existing .dbf on disk --
// the exact same catalog/table-resolution path both exporters must use so
// neither format silently drifts from what the other considers "the
// database." Only the per-format serialization differs between callers.
struct DatabaseCatalogSnapshot {
    bool ok = false;
    std::string error;
    std::filesystem::path dbc_fs_path;
    std::string db_name;
    std::vector<DbcCatalogObject> catalog;

    struct ResolvedTable { std::string name; std::filesystem::path path; };
    std::vector<ResolvedTable> resolved_tables;
};

DatabaseCatalogSnapshot load_database_catalog_snapshot(const std::string& dbc_path) {
    namespace fs = std::filesystem;
    DatabaseCatalogSnapshot snapshot;
    snapshot.dbc_fs_path = copperfin::platform::path_from_utf8_string(dbc_path);

    std::error_code dbc_status_error;
    if (!fs::exists(snapshot.dbc_fs_path, dbc_status_error)) {
        snapshot.error = asset_inspector_text("Vfp.AssetInspector.Error.DbcPathMissing", {{"path", dbc_path}});
        return snapshot;
    }

    const SidecarPathResolution dct_resolution = resolve_vfp_memo_sidecar_path(snapshot.dbc_fs_path);
    if (dct_resolution.ambiguous) {
        snapshot.error = asset_inspector_text(
            "Vfp.Sidecar.Error.AmbiguousPath",
            {{"path", copperfin::platform::path_to_utf8_string(dct_resolution.requested_path)}});
        return snapshot;
    }
    const std::optional<fs::path> dct_path = dct_resolution.path;
    const bool has_dct = dct_path.has_value();

    // Load the raw DBC bytes for direct field-pointer extraction
    const std::vector<std::uint8_t> dbc_bytes = read_binary_file(dbc_path);
    if (dbc_bytes.empty()) {
        snapshot.error = asset_inspector_text("Vfp.AssetInspector.Error.DbcReadFailed", {{"path", dbc_path}});
        return snapshot;
    }

    const DbfParseResult header_result = parse_dbf_header(dbc_bytes);
    if (!header_result.ok) {
        snapshot.error = asset_inspector_text(
            "Vfp.AssetInspector.Error.DbcHeaderParseFailed",
            {{"error", header_result.error}});
        return snapshot;
    }

    // Read all catalog rows with raw memo block numbers
    const std::vector<RawDbcRow> raw_rows = read_raw_dbc_rows(dbc_bytes, header_result.header);

    // Decode properties for each row and build DbcCatalogObject list
    snapshot.catalog.reserve(raw_rows.size());
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
        // #5544 review (Codex, P2): load_database_catalog_snapshot() is
        // shared by the JSON/SQL/Access-SQL exporters, none of which
        // need CODE content -- decoding it unconditionally for every row
        // (including a StoredProceduresObject row's compiled p-code,
        // which isn't text at all) would add avoidable memo-sidecar I/O
        // and allocation to those unrelated operations for a real
        // database with large stored-procedure content. Only the
        // StoredProceduresSource row itself -- the one
        // extract_dbc_stored_procedures_source() actually looks for --
        // is decoded here.
        if (raw.code_block != 0U && has_dct &&
            lowercase_copy(raw.object_name) == "storedproceduressource") {
            const std::vector<std::uint8_t> code_bytes =
                read_memo_block_raw(
                    copperfin::platform::path_to_utf8_string(*dct_path),
                    raw.code_block);
            if (!code_bytes.empty()) {
                const DbfTextConversionResult decoded = decode_dbf_text(
                    header_result.header.code_page_mark,
                    std::string_view(reinterpret_cast<const char*>(code_bytes.data()), code_bytes.size()));
                if (decoded.ok) {
                    obj.code_source = decoded.text;
                }
            }
        }

        snapshot.catalog.push_back(std::move(obj));
    }

    snapshot.db_name = copperfin::platform::path_to_utf8_string(snapshot.dbc_fs_path.stem());
    const auto database_object = std::find_if(
        snapshot.catalog.begin(),
        snapshot.catalog.end(),
        [](const DbcCatalogObject& object)
        {
            return !object.deleted && object.object_type == "database" && !trim_copy(object.object_name).empty();
        });
    if (database_object != snapshot.catalog.end()) {
        snapshot.db_name = database_object->object_name;
    }

    // #5636: a catalog TABLE object's own name is untrusted data from the
    // DBC a crafted or foreign-tool-written bundle controls, not a name
    // this codebase's own writer is bound to keep free of path syntax.
    // std::filesystem::path's own operator/ silently *replaces* the whole
    // left-hand path when the appended component is absolute, and a
    // relative "../secret" component resolves outside dbc_dir at the OS
    // level even though the path string still nominally starts with it --
    // directly reproduced during triage: a table object named "../secret"
    // made every exporter (they all share this one loader) read and
    // disclose a sibling directory's own .dbf verbatim. Rejecting via
    // table_name_is_safe_filesystem_component() -- the identical check
    // build_database_json_import_plan()'s own table-name validation
    // already applies on the import side -- happens before any path is
    // ever constructed from the name, matching that function's own stated
    // reasoning, and fails the whole snapshot load closed rather than
    // silently skipping just the one unsafe table (a crafted catalog is
    // not the kind of "missing/unreadable table" #5537's own established
    // skipped-table comment convention exists for).
    const fs::path dbc_dir = snapshot.dbc_fs_path.parent_path();
    std::error_code canonical_dbc_dir_error;
    const fs::path canonical_dbc_dir = fs::weakly_canonical(
        dbc_dir.empty() ? fs::path(".") : dbc_dir, canonical_dbc_dir_error);
    // #5685 PR review (chatgpt-codex-connector, P1): a string-safe table
    // name (no separators, no "..") can still name a symlink/junction
    // planted directly inside dbc_dir that itself points outside it -- the
    // string check alone cannot see through that, so a *resolved* path's
    // own canonical form must be verified still contained beneath
    // dbc_dir's own canonical form before it is trusted. Shared as one
    // lambda (rather than only checking the primary .dbf path, the
    // original narrower fix's own scope) because the same disclosure
    // applies identically to a table's own memo (.fpt) sidecar: an
    // in-directory foo.dbf with memo fields alongside a foo.fpt that is
    // itself a symlink to a memo file outside dbc_dir lets
    // parse_dbf_table_from_file() independently resolve and read that
    // unchecked sidecar later, disclosing its content the same way the
    // primary-path check alone was meant to prevent.
    const auto path_escapes_database_directory = [&](const fs::path& resolved) {
        std::error_code canonical_error;
        const fs::path canonical_resolved = fs::weakly_canonical(resolved, canonical_error);
        const auto containment_mismatch = std::mismatch(
            canonical_dbc_dir.begin(), canonical_dbc_dir.end(),
            canonical_resolved.begin(), canonical_resolved.end());
        return canonical_error || canonical_dbc_dir_error ||
            containment_mismatch.first != canonical_dbc_dir.end();
    };
    for (const auto& obj : snapshot.catalog) {
        if (obj.deleted || obj.object_type != "table" || obj.object_name.empty()) {
            continue;
        }
        const std::string& tname = obj.object_name;
        if (!table_name_is_safe_filesystem_component(tname)) {
            snapshot = {};
            snapshot.dbc_fs_path = copperfin::platform::path_from_utf8_string(dbc_path);
            snapshot.error = asset_inspector_text(
                "Vfp.AssetInspector.Error.DbcTableNameUnsafe", {{"table", tname}});
            return snapshot;
        }
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
        if (path_escapes_database_directory(*resolved_table_path)) {
            snapshot = {};
            snapshot.dbc_fs_path = copperfin::platform::path_from_utf8_string(dbc_path);
            snapshot.error = asset_inspector_text(
                "Vfp.AssetInspector.Error.DbcTableEscapesDirectory", {{"table", tname}});
            return snapshot;
        }
        // The table's own memo sidecar (if one exists on disk at all) gets
        // the identical containment check -- see this loop's own comment
        // above for why a per-table skip on the primary path alone is not
        // sufficient.
        const SidecarPathResolution table_memo_resolution =
            resolve_vfp_memo_sidecar_path(*resolved_table_path);
        if (table_memo_resolution.path.has_value() &&
            path_escapes_database_directory(*table_memo_resolution.path)) {
            snapshot = {};
            snapshot.dbc_fs_path = copperfin::platform::path_from_utf8_string(dbc_path);
            snapshot.error = asset_inspector_text(
                "Vfp.AssetInspector.Error.DbcTableEscapesDirectory", {{"table", tname}});
            return snapshot;
        }
        snapshot.resolved_tables.push_back({tname, *resolved_table_path});
    }

    snapshot.ok = true;
    return snapshot;
}

// Portable/ANSI-ish identifier quoting for export_database_as_sql(): wraps
// in double quotes, doubling any embedded quote character. Deliberately not
// dialect-specific (no backtick/bracket variants) -- see that function's
// own non-goal note about dialect targeting.
std::string sql_quote_identifier(const std::string& name) {
    std::string quoted = "\"";
    for (const char character : name) {
        if (character == '"') {
            quoted += "\"\"";
        } else {
            quoted += character;
        }
    }
    quoted += "\"";
    return quoted;
}

// Standard SQL string-literal quoting: single quotes, doubling any embedded
// single quote.
std::string sql_quote_string_literal(const std::string& value) {
    std::string quoted = "'";
    for (const char character : value) {
        if (character == '\'') {
            quoted += "''";
        } else {
            quoted += character;
        }
    }
    quoted += "'";
    return quoted;
}

// Both export_database_as_sql() and export_database_as_access_sql() emit
// a numeric (N/F/I/B/Y) field's decoded display_value token *unquoted*,
// since a real numeric literal needs no string quoting -- but that means,
// unlike a string value (which sql_quote_string_literal() always escapes
// into a safely delimited literal), there is no quoting layer standing
// between this text and the generated SQL. A table genuinely written by
// this codebase's own writer
// always decodes N/F to a plain optionally-signed decimal string, but a
// crafted or corrupted source is not bound by that: a numeric-overflow
// marker ("*****", dBASE-family's own convention for a value too wide for
// its field) or arbitrary injected text would otherwise be emitted
// unquoted and unescaped, silently producing invalid DDL/DML at best and
// letting untrusted legacy data inject additional SQL statements at
// worst. Validates that `text` is a plain optionally-signed decimal
// number (at most one leading '+' or '-', digits, at most one '.')
// before it is trusted to appear unquoted. '+' is accepted alongside
// '-' -- not just for symmetry, but because this codebase's own value
// parsing (e.g. parse_scaled_currency_value(), dbf_table.cpp) already
// treats a leading '+' as valid numeric input elsewhere, so a genuinely
// real (not crafted) "+123.45" reaching here must not be misclassified
// as unsafe and silently turned into NULL.
//
// #5545 review (Codex, P1): a VFP 'B' (double) field's decoded
// display_value comes from an unflagged std::ostringstream
// (src/vfp/dbf_table.cpp's decode_value(), case 'B') at
// max_digits10 precision -- the default iostream formatting this uses
// (no std::fixed) switches to scientific notation ("1e+20", "1.5e-10")
// for sufficiently large or small magnitudes, exactly the way printf's
// unflagged %g would. Before this fix, any such value failed this
// validator (which only recognized a plain optionally-signed decimal)
// and was silently replaced with NULL by every caller -- a genuine,
// real-world (not merely crafted-input) data-loss bug, not a security
// concern like the original digit-only check was guarding against.
// Recognizes an optional well-formed 'e'/'E' exponent suffix
// (optional sign, at least one digit) after the mantissa, in addition
// to the plain decimal form already accepted.
bool looks_like_safe_unquoted_sql_numeric_literal(const std::string& text) {
    if (text.empty()) {
        return false;
    }
    std::size_t index = (text.front() == '-' || text.front() == '+') ? 1U : 0U;
    if (index >= text.size()) {
        return false;
    }
    bool seen_digit = false;
    bool seen_decimal_point = false;
    for (; index < text.size(); ++index) {
        const char character = text[index];
        if (character == '.') {
            if (seen_decimal_point) {
                return false;
            }
            seen_decimal_point = true;
            continue;
        }
        if (character == 'e' || character == 'E') {
            if (!seen_digit) {
                return false;
            }
            ++index;
            if (index < text.size() && (text[index] == '-' || text[index] == '+')) {
                ++index;
            }
            if (index >= text.size()) {
                return false;
            }
            bool seen_exponent_digit = false;
            for (; index < text.size(); ++index) {
                if (std::isdigit(static_cast<unsigned char>(text[index])) == 0) {
                    return false;
                }
                seen_exponent_digit = true;
            }
            return seen_exponent_digit;
        }
        if (std::isdigit(static_cast<unsigned char>(character)) == 0) {
            return false;
        }
        seen_digit = true;
    }
    return seen_digit;
}

// export_database_as_access_sql() embeds a 'D' field's decoded
// display_value directly inside Access SQL's #...# date-literal
// delimiters, the same "no quoting layer to escape it with" situation
// looks_like_safe_unquoted_sql_numeric_literal() documents for numeric
// tokens. decode_value()'s 'D' case (dbf_table.cpp) only formats the
// value as "YYYY-MM-DD" when the raw field is exactly 8 bytes wide;
// anything else -- a differently-sized or genuinely corrupt/crafted
// field -- falls back to returning the trimmed raw bytes verbatim,
// which could contain a literal '#' and break out of the delimiter.
// Validates the exact "YYYY-MM-DD" shape (4 digits, '-', 2 digits, '-',
// 2 digits) before trusting it to appear inside #...#.
bool looks_like_safe_access_sql_date_literal(const std::string& text) {
    if (text.size() != 10U) {
        return false;
    }
    for (const std::size_t digit_index : {0U, 1U, 2U, 3U, 5U, 6U, 8U, 9U}) {
        if (std::isdigit(static_cast<unsigned char>(text[digit_index])) == 0) {
            return false;
        }
    }
    return text[4U] == '-' && text[7U] == '-';
}

// Maps a DBF field descriptor to a portable/ANSI-ish SQL column type.
// Memo/general/picture pointer fields (M/G/P) map to TEXT: their content is
// not resolved by the shared catalog/table-reading path this exporter
// shares with export_database_as_json(), which likewise only ever emits
// whatever raw display_value that path already produces for those types --
// this is an existing, not newly introduced, limitation.
std::string sql_column_type(char field_type, std::uint8_t length, std::uint8_t decimal_count) {
    const char normalized = static_cast<char>(std::toupper(static_cast<unsigned char>(field_type)));
    switch (normalized) {
        case 'N':
        case 'F':
            return "DECIMAL(" + std::to_string(length > 0U ? length : 1U) + ", " +
                std::to_string(decimal_count) + ")";
        case 'Y':
            // VFP currency is a fixed 8-byte scaled integer with exactly
            // four fractional digits, independent of the DBF descriptor's
            // own length/decimal_count (which this codebase's writer
            // leaves at length 8 / decimals 0 for Y fields) -- deriving
            // precision from those would silently round stored cents and
            // cannot represent the type's actual range.
            return "DECIMAL(19, 4)";
        case 'I':
            return "INTEGER";
        case 'B':
            return "DOUBLE PRECISION";
        case 'L':
            return "BOOLEAN";
        case 'D':
            return "DATE";
        case 'T':
            return "TIMESTAMP";
        case 'C':
        case 'V':
            return "VARCHAR(" + std::to_string(length > 0U ? length : 255U) + ")";
        default:
            // M, G, P, and any other/unrecognized storage type.
            return "TEXT";
    }
}

// Access/Jet SQL identifier quoting for export_database_as_access_sql():
// square brackets, per the publicly documented Access SQL/DDL dialect (see
// docs/66-access-container-format-notes.md's "logical surface is citable"
// finding). A real Access UI forbids "]" in object names, but this
// exporter's input is DBC/DBF catalog metadata, which a crafted or
// corrupt source file does not have to obey that rule for -- so an
// embedded "]" is escaped by doubling it (the Jet/ACE bracket-escape
// convention), matching sql_quote_identifier()'s own embedded-quote
// handling for the ANSI dialect, rather than assumed to be impossible.
std::string access_quote_identifier(const std::string& name) {
    std::string quoted = "[";
    for (const char character : name) {
        if (character == ']') {
            quoted += "]]";
        } else {
            quoted += character;
        }
    }
    quoted += "]";
    return quoted;
}

// Maps a DBF field descriptor to an Access/Jet SQL native column type,
// rather than reusing sql_column_type()'s portable/ANSI-ish vocabulary --
// per docs/66, the Access SQL/DDL dialect (unlike the physical MDB/ACCDB
// byte layout) is legitimately citable public documentation, so this
// dialect-specific mapping is grounded rather than guessed. Two notable
// departures from sql_column_type(): 'Y' (VFP currency) maps to Access's
// own CURRENCY type -- an exact fixed-point match, not the DECIMAL(19, 4)
// approximation the ANSI exporter uses -- and Access's Short Text (TEXT)
// type caps at 255 characters, so a wider Character field must fall back
// to MEMO (Access's unbounded text type) rather than an invalid TEXT(n)
// declaration.
std::string access_column_type(char field_type, std::uint8_t length, std::uint8_t decimal_count) {
    const char normalized = static_cast<char>(std::toupper(static_cast<unsigned char>(field_type)));
    switch (normalized) {
        case 'N':
        case 'F': {
            // A crafted/corrupt DBF header does not have to keep
            // decimal_count <= length the way a table genuinely written
            // by this codebase's own writer always does, and Access/Jet
            // SQL's own DECIMAL type caps precision at 28 -- emitting an
            // unclamped value here would produce DDL Access itself
            // rejects (invalid precision, or scale > precision). Clamp
            // into Access-valid ranges rather than trust the header.
            constexpr std::uint8_t max_access_decimal_precision = 28U;
            const std::uint8_t precision = std::min(
                std::max(length, static_cast<std::uint8_t>(1U)),
                max_access_decimal_precision);
            const std::uint8_t scale = std::min(decimal_count, precision);
            return "DECIMAL(" + std::to_string(precision) + ", " + std::to_string(scale) + ")";
        }
        case 'Y':
            return "CURRENCY";
        case 'I':
            return "LONG";
        case 'B':
            return "DOUBLE";
        case 'L':
            return "YESNO";
        case 'D':
            // Access has no date-only column type distinct from DATETIME;
            // a date-only value is simply a DATETIME with a zero time
            // component, which Access's UI formats per the column's
            // display format rather than a separate storage type.
            return "DATETIME";
        case 'T':
            return "DATETIME";
        case 'C':
        case 'V':
            // length is a uint8_t (max 255), which already fits Access
            // Short Text's own 255-character ceiling -- so any nonzero
            // length is safely representable as TEXT(length); only a
            // (theoretically impossible here, but checked for honesty)
            // zero length falls back to MEMO.
            return (length > 0U) ? ("TEXT(" + std::to_string(length) + ")") : "MEMO";
        default:
            // M, G, P, and any other/unrecognized storage type.
            return "MEMO";
    }
}

// #5554: T-SQL identifier quoting for export_database_as_sqlserver_sql() --
// square brackets, per Microsoft's own public "Delimited Identifiers"
// T-SQL reference. Kept as its own dedicated function rather than an
// alias for access_quote_identifier() (even though the escaping rule is
// byte-identical), matching this file's own established per-vendor
// precedent, so a future SQL-Server-specific divergence has somewhere to
// go without touching Access's own code path. Directly confirmed against
// a real local SQL Server 2022 engine: an identifier containing an
// embedded "]", doubled per this function's own escaping, round-tripped
// through CREATE TABLE and back out of sys.tables unchanged.
std::string sqlserver_quote_identifier(const std::string& name) {
    std::string quoted = "[";
    for (const char character : name) {
        if (character == ']') {
            quoted += "]]";
        } else {
            quoted += character;
        }
    }
    quoted += "]";
    return quoted;
}

// Maps a DBF field descriptor to a T-SQL native column type, grounded in
// Microsoft's own public T-SQL data-type reference (not the ANSI-ish
// vocabulary sql_column_type() emits, nor Access/Jet's -- T-SQL has its
// own distinct type names and quirks). 'Y' (VFP currency) maps to
// MONEY, an exact match for VFP currency's own fixed 4-decimal-digit
// scaled-integer semantics (mirroring why export_database_as_access_sql()
// picks Access's own CURRENCY type over a DECIMAL approximation). 'L'
// (logical) maps to BIT -- T-SQL has no BOOLEAN type. 'T' (VFP
// datetime) maps to DATETIME2, Microsoft's own documented modern
// replacement for the legacy DATETIME type. The memo/general/picture
// and any-other-unrecognized fallback maps to VARCHAR(MAX) rather than
// the legacy TEXT type, which Microsoft's own documentation marks
// deprecated in favor of VARCHAR(MAX)/NVARCHAR(MAX)/VARBINARY(MAX).
// Directly confirmed against a real local SQL Server 2022 engine: a
// CREATE TABLE using every one of these exact type names, followed by
// INSERT statements exercising each, loaded and queried back correctly.
std::string sqlserver_column_type(char field_type, std::uint8_t length, std::uint8_t decimal_count) {
    const char normalized = static_cast<char>(std::toupper(static_cast<unsigned char>(field_type)));
    switch (normalized) {
        case 'N':
        case 'F': {
            // #5554 PR review (chatgpt-codex-connector and
            // copilot-pull-request-reviewer, independently): a crafted or
            // corrupt DBF header does not have to keep decimal_count <=
            // length the way a table genuinely written by this codebase's
            // own writer always does, and real SQL Server rejects any
            // DECIMAL declaration whose precision exceeds 38 (directly
            // confirmed against a real local SQL Server 2022 engine:
            // DECIMAL(39, 0) fails with "Specified column precision 39 is
            // greater than the maximum precision of 38", DECIMAL(38, 38)
            // succeeds) or whose scale exceeds its own precision.
            // access_column_type() already applies the identical clamp
            // pattern for Access's own 28-precision ceiling.
            constexpr std::uint8_t max_sqlserver_decimal_precision = 38U;
            const std::uint8_t precision = std::min(
                std::max(length, static_cast<std::uint8_t>(1U)),
                max_sqlserver_decimal_precision);
            const std::uint8_t scale = std::min(decimal_count, precision);
            return "DECIMAL(" + std::to_string(precision) + ", " + std::to_string(scale) + ")";
        }
        case 'Y':
            return "MONEY";
        case 'I':
            return "INT";
        case 'B':
            // T-SQL's bare FLOAT defaults to FLOAT(53) -- IEEE double
            // precision, matching VFP's own 'B' (double) field width.
            return "FLOAT";
        case 'L':
            return "BIT";
        case 'D':
            return "DATE";
        case 'T':
            return "DATETIME2";
        case 'C':
        case 'V':
            return "VARCHAR(" + std::to_string(length > 0U ? length : 255U) + ")";
        default:
            // M, G, P, and any other/unrecognized storage type.
            return "VARCHAR(MAX)";
    }
}

// #5554: Oracle identifier quoting for export_database_as_oracle_sql() --
// double quotes, per Oracle's own public SQL Language Reference
// "Database Object Naming Rules". Unlike every other dialect this file
// emits (all of which escape an embedded quote character by doubling
// it), Oracle provides *no* escape mechanism for a double quote inside
// a quoted identifier at all -- directly confirmed against a real local
// Oracle 23ai engine: `CREATE TABLE "weird""name" (...)` fails outright
// with ORA-25716 ("The identifier contains a double quotation mark (")
// character"), not the doubled-quote-survives-as-literal-quote behavior
// every other dialect's own quoting convention relies on. A crafted or
// corrupt DBC/DBF is not bound to avoid embedding one, so this strips
// any embedded `"` outright (a deliberately lossy transformation,
// unlike the other three dialects' lossless doubling) rather than
// emit identifier text a real Oracle engine would reject wholesale.
std::string oracle_quote_identifier(const std::string& name) {
    std::string quoted = "\"";
    for (const char character : name) {
        if (character != '"') {
            quoted += character;
        }
    }
    quoted += "\"";
    return quoted;
}

// Maps a DBF field descriptor to a native Oracle SQL column type,
// grounded in Oracle's own public SQL Language Reference data types
// documentation (not the ANSI-ish vocabulary sql_column_type() emits,
// nor any other dialect's own vocabulary this file already maps to --
// Oracle has its own distinct type names and numeric-precision/scale
// rules). 'Y' (VFP currency) maps to NUMBER(19, 4), an exact match for
// VFP currency's own fixed 4-decimal-digit scaled-integer semantics
// (the same reasoning export_database_as_sqlserver_sql() applies to its
// own MONEY choice). 'I' maps to INTEGER, Oracle's own documented ANSI-
// compatible subtype of NUMBER(38). 'B' (VFP double) maps to
// BINARY_DOUBLE, Oracle's true IEEE 754 double-precision type -- an
// exact width match, unlike NUMBER's arbitrary-precision decimal
// semantics. 'L' (logical) maps to NUMBER(1) with 1/0 literals (see
// write_oracle_tables_and_data()'s own comment): Oracle has no
// dedicated BOOLEAN table-column type prior to Oracle 23c, and this
// exporter targets the traditional, universally-supported convention
// rather than a feature only the newest Oracle major version has --
// directly confirmed against a real local Oracle 23ai engine that even
// there, `INSERT ... VALUES (TRUE)` into a NUMBER(1) column silently
// converts to 1 rather than erroring, so 1/0 is not merely the safe
// choice but the one every Oracle version -- old or new -- accepts
// identically. The memo/general/picture and any-other-unrecognized
// fallback maps to CLOB, Oracle's own unbounded text type (VARCHAR2
// caps at 4000 bytes by default, too narrow to assume large enough).
std::string oracle_column_type(char field_type, std::uint8_t length, std::uint8_t decimal_count) {
    const char normalized = static_cast<char>(std::toupper(static_cast<unsigned char>(field_type)));
    switch (normalized) {
        case 'N':
        case 'F': {
            // #5554: a crafted or corrupt DBF header does not have to
            // keep length/decimal_count within Oracle's own valid
            // ranges the way a table genuinely written by this
            // codebase's own writer always does. Directly confirmed
            // against a real local Oracle 23ai engine: NUMBER(38, 0)
            // succeeds while NUMBER(39, 0) fails ("numeric precision
            // specifier is out of range (1 to 38)"), and -- a real,
            // material difference from SQL Server's own DECIMAL, whose
            // scale must not exceed its own precision -- Oracle's own
            // scale is independent of precision entirely, valid from
            // -84 to 127 regardless of precision (NUMBER(10, 127)
            // succeeds; NUMBER(10, 128) fails, "numeric scale specifier
            // is out of range (-84 to 127)"). Clamping scale to
            // precision the way access_column_type()/sqlserver_column_type()
            // do would therefore silently narrow a scale Oracle itself
            // has no problem with.
            constexpr std::uint8_t max_oracle_number_precision = 38U;
            constexpr std::uint8_t max_oracle_number_scale = 127U;
            const std::uint8_t precision = std::min(
                std::max(length, static_cast<std::uint8_t>(1U)),
                max_oracle_number_precision);
            const std::uint8_t scale = std::min(decimal_count, max_oracle_number_scale);
            return "NUMBER(" + std::to_string(precision) + ", " + std::to_string(scale) + ")";
        }
        case 'Y':
            return "NUMBER(19, 4)";
        case 'I':
            return "INTEGER";
        case 'B':
            return "BINARY_DOUBLE";
        case 'L':
            return "NUMBER(1)";
        case 'D':
            return "DATE";
        case 'T':
            return "TIMESTAMP";
        case 'C':
        case 'V':
            // #5564 PR review (chatgpt-codex-connector, P1): a bare
            // `VARCHAR2(n)` is measured in *bytes* under Oracle's own
            // default `NLS_LENGTH_SEMANTICS=BYTE` -- directly confirmed
            // against a real local Oracle 23ai engine that a 10-byte
            // (BYTE-semantics) `VARCHAR2(10)` rejects 10 non-ASCII
            // characters that decode to 20 UTF-8 bytes (ORA-12899),
            // while explicit `VARCHAR2(10 CHAR)` accepts the identical
            // 10 characters regardless of session settings. This
            // codebase decodes legacy single-byte DBF text into UTF-8
            // for every dialect's own output, so a VFP `C(n)` field's
            // own length (always a *character* count) must be declared
            // with `CHAR` semantics here to guarantee `n` characters of
            // capacity independent of the target session's own NLS
            // configuration.
            return "VARCHAR2(" + std::to_string(length > 0U ? length : 255U) + " CHAR)";
        default:
            // M, G, P, and any other/unrecognized storage type.
            return "CLOB";
    }
}

// Strips C0 control characters (CR/LF in particular) from text destined for
// a single-line "-- ..." SQL comment. export_database_as_sql()'s database
// name and source path both come from data an untrusted/crafted DBC could
// influence (the catalog's own DATABASE object name), and an embedded
// newline would let injected text escape the comment and execute as SQL
// when the generated script is run.
std::string sql_sanitize_comment_text(const std::string& text) {
    std::string sanitized;
    sanitized.reserve(text.size());
    for (const char character : text) {
        sanitized += (static_cast<unsigned char>(character) < 0x20U) ? ' ' : character;
    }
    return sanitized;
}

// Mirrors src/runtime/prg_engine_helpers.cpp's julian_to_date() (Fliegel-Van
// Flandern astronomical Julian day, minus 702 to match this codebase's
// existing epoch convention) rather than depending on cf_xbase_runtime from
// this lower-level library -- cf_xbase_runtime already depends on
// cf_vfp_assets, so the reverse dependency isn't available. Verified to
// reproduce the same year/month/day as the runtime for representative
// fixture values (see the paired regression test).
void sql_julian_day_to_date(int julian, int& year, int& month, int& day) {
    int l = (julian + 702) + 68569;
    const int n = (4 * l) / 146097;
    l = l - (146097 * n + 3) / 4;
    const int i = (4000 * (l + 1)) / 1461001;
    l = l - (1461 * i) / 4 + 31;
    const int j = (80 * l) / 2447;
    day = l - (2447 * j) / 80;
    l = j / 11;
    month = j + 2 - (12 * l);
    year = 100 * (n - 49) + i + l;
}

// Converts the "julian:<day> millis:<milliseconds-since-midnight>" internal
// storage representation parse_dbf_table_from_file() returns for T-type
// fields into a "YYYY-MM-DD HH:MM:SS" SQL timestamp literal body (caller
// still quotes it as a string literal). Returns std::nullopt for anything
// this parser doesn't recognize as that exact contract, rather than
// guessing -- the caller falls back to NULL in that case.
std::optional<std::string> sql_datetime_literal_from_storage(const std::string& raw) {
    const std::string trimmed = trim_copy(raw);
    if (trimmed.empty()) {
        return std::nullopt;
    }
    constexpr std::string_view julian_prefix = "julian:";
    constexpr std::string_view millis_prefix = "millis:";
    const std::string lowered = lowercase_copy(trimmed);
    if (lowered.rfind(julian_prefix, 0U) != 0U) {
        return std::nullopt;
    }
    const std::size_t millis_pos = lowered.find(millis_prefix);
    if (millis_pos == std::string::npos || millis_pos <= julian_prefix.size()) {
        return std::nullopt;
    }
    const std::string julian_text = trim_copy(
        trimmed.substr(julian_prefix.size(), millis_pos - julian_prefix.size()));
    const std::string millis_text = trim_copy(trimmed.substr(millis_pos + millis_prefix.size()));
    if (julian_text.empty() || millis_text.empty()) {
        return std::nullopt;
    }

    int julian_day = 0;
    int millis = 0;
    try {
        std::size_t consumed = 0U;
        julian_day = std::stoi(julian_text, &consumed, 10);
        if (consumed != julian_text.size()) {
            return std::nullopt;
        }
        consumed = 0U;
        millis = std::stoi(millis_text, &consumed, 10);
        if (consumed != millis_text.size()) {
            return std::nullopt;
        }
    } catch (...) {
        return std::nullopt;
    }
    if (julian_day <= 0 || millis < 0 || millis >= 24 * 60 * 60 * 1000) {
        return std::nullopt;
    }

    int year = 0;
    int month = 0;
    int day = 0;
    sql_julian_day_to_date(julian_day, year, month, day);
    if (year < 1 || year > 9999 || month < 1 || month > 12 || day < 1 || day > 31) {
        return std::nullopt;
    }

    const int total_seconds = millis / 1000;
    const int hours = total_seconds / 3600;
    const int minutes = (total_seconds % 3600) / 60;
    const int seconds = total_seconds % 60;

    std::ostringstream literal;
    literal.imbue(std::locale::classic());
    literal << std::setfill('0')
            << std::setw(4) << year << "-" << std::setw(2) << month << "-" << std::setw(2) << day
            << " " << std::setw(2) << hours << ":" << std::setw(2) << minutes << ":" << std::setw(2) << seconds;
    return literal.str();
}

}  // namespace (extended)

DbcStoredProceduresResult extract_dbc_stored_procedures_source(const std::string& dbc_path) {
    DbcStoredProceduresResult result;
    const DatabaseCatalogSnapshot snapshot = load_database_catalog_snapshot(dbc_path);
    if (!snapshot.ok) {
        result.error = snapshot.error;
        return result;
    }

    // #5538: real Visual FoxPro names this catalog row "StoredProceduresSource"
    // (a sibling "StoredProceduresObject" row holds compiled p-code, which
    // this function does not look for -- see DbcStoredProceduresResult's
    // own comment). Matched case-insensitively, consistent with this
    // codebase's existing catalog-field-name tolerance elsewhere.
    for (const DbcCatalogObject& object : snapshot.catalog) {
        if (object.deleted) {
            continue;
        }
        if (lowercase_copy(trim_copy(object.object_name)) == "storedproceduressource") {
            if (object.code_source.has_value()) {
                result.available = true;
                result.source_code = *object.code_source;
            }
            break;
        }
    }

    result.ok = true;
    return result;
}

DatabaseExportResult export_database_as_json(
    const std::string& dbc_path,
    std::size_t max_rows_per_table) {

    namespace fs = std::filesystem;
    const DatabaseCatalogSnapshot snapshot = load_database_catalog_snapshot(dbc_path);
    if (!snapshot.ok) {
        return {.ok = false, .error = snapshot.error, .json = {}};
    }
    const auto& catalog = snapshot.catalog;
    const std::string& db_name = snapshot.db_name;

    // Build JSON ----------------------------------------------------------
    std::ostringstream json;
    json.imbue(std::locale::classic());
    json << "{\n";
    // Version the owner-policy interchange envelope before it is ever used
    // as a reconstruction input. Consumers must select their parser by this
    // value rather than infer a contract from optional catalog/table content.
    json << "  \"schema_version\": 1,\n";

    // -- database metadata block
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

    const auto& resolved_tables = snapshot.resolved_tables;
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
        if (tbl.table.fields.empty()) {
            // #5697: a member DBF whose field-descriptor block parses
            // "successfully" with zero fields (an immediately-encountered
            // terminator) previously fell through to the ordinary success
            // path here, emitting `"fields": []` and reporting ok=true --
            // but build_database_json_import_plan() (this exporter's own
            // JSON round-trip counterpart) rejects that exact shape, so
            // Copperfin's own migration round-trip cannot actually load
            // its own output. Fail the whole export closed rather than
            // return a partial/unusable artifact -- matching this
            // codebase's own established fail-closed precedent for a case
            // with no safe corrective action.
            return {.ok = false, .error = asset_inspector_text(
                "Vfp.AssetInspector.Validation.ExportTableHasNoFields",
                {{"table", rt.name}}), .json = {}};
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

namespace {

// Shared by export_database_as_sql() and export_database_as_postgresql_sql():
// both dialects accept the exact same CREATE TABLE/INSERT shape (double-
// quoted identifiers, DECIMAL/INTEGER/DOUBLE PRECISION/BOOLEAN/DATE/
// TIMESTAMP/VARCHAR/TEXT column types, single-quoted string literals) --
// see export_database_as_postgresql_sql()'s own comment for why real
// PostgreSQL already accepts this portable/ANSI-ish dialect verbatim.
// Returns every successfully-parsed table (a failed one already got its
// own skip comment written here and is omitted), so a caller needing the
// parsed field list for something else -- export_database_as_postgresql_sql()'s
// CREATE INDEX generation -- does not have to re-open and re-parse the
// same .dbf a second time.
struct ParsedSqlExportTable {
    DatabaseCatalogSnapshot::ResolvedTable resolved;
    DbfTable table;
};

std::vector<ParsedSqlExportTable> write_sql_tables_and_data(
    std::ostringstream& sql,
    const DatabaseCatalogSnapshot& snapshot,
    std::size_t row_limit,
    std::string& hard_failure_error) {
    std::vector<ParsedSqlExportTable> parsed_tables;
    for (const auto& rt : snapshot.resolved_tables) {
        // #5545 review (Copilot): not const, so tbl.table (fields +
        // every parsed record) can be moved into parsed_tables below
        // rather than deep-copied.
        DbfTableParseResult tbl = parse_dbf_table_from_file(
            copperfin::platform::path_to_utf8_string(rt.path), row_limit);
        if (!tbl.ok) {
            // Keep the same table (not skip silently) via a comment, so a
            // reader of the script can see every catalog table was
            // considered, matching how the JSON exporter emits an empty
            // fields/records entry rather than omitting the key entirely.
            // rt.name and tbl.error both come from data a crafted DBC/DBF
            // could influence (a catalog table name; a parse-error message
            // that can itself echo untrusted content) -- sanitized the
            // same way the database-name/source-path header lines already
            // are, so neither can embed a newline and escape this
            // single-line "-- ..." comment into executable SQL.
            sql << "-- skipped table " << sql_sanitize_comment_text(rt.name) << ": "
                << sql_sanitize_comment_text(tbl.error) << "\n\n";
            continue;
        }
        if (tbl.table.fields.empty()) {
            // #5697: a member DBF that parses "successfully" with zero
            // fields would otherwise emit `CREATE TABLE "name" (\n);`,
            // invalid DDL on every supported engine. Fail the whole
            // export closed rather than return a script that cannot
            // load, matching this exporter's own established fail-closed
            // precedent (see write_oracle_tables_and_data()'s own
            // hard_failure_error uses) for a case with no safe
            // corrective action.
            hard_failure_error = asset_inspector_text(
                "Vfp.AssetInspector.Validation.ExportTableHasNoFields",
                {{"table", rt.name}});
            return {};
        }

        const std::string quoted_table = sql_quote_identifier(rt.name);
        sql << "CREATE TABLE " << quoted_table << " (\n";
        for (std::size_t fi = 0U; fi < tbl.table.fields.size(); ++fi) {
            const auto& fld = tbl.table.fields[fi];
            const bool last_field = (fi + 1U == tbl.table.fields.size());
            sql << "    " << sql_quote_identifier(fld.name) << " "
                << sql_column_type(fld.type, fld.length, fld.decimal_count)
                << (last_field ? "\n" : ",\n");
        }
        sql << ");\n\n";

        std::size_t row_number = 0U;
        for (const auto& rec : tbl.table.records) {
            if (rec.deleted) {
                continue;
            }
            ++row_number;
            sql << "INSERT INTO " << quoted_table << " (";
            for (std::size_t vi = 0U; vi < rec.values.size(); ++vi) {
                sql << sql_quote_identifier(rec.values[vi].field_name)
                    << (vi + 1U == rec.values.size() ? "" : ", ");
            }
            sql << ") VALUES (";
            for (std::size_t vi = 0U; vi < rec.values.size(); ++vi) {
                const auto& rv = rec.values[vi];
                const char ft = static_cast<char>(
                    std::toupper(static_cast<unsigned char>(rv.field_type)));
                const bool is_numeric = (ft == 'N' || ft == 'F' || ft == 'I' || ft == 'B' || ft == 'Y');
                const bool is_logical = (ft == 'L');
                const bool is_date = (ft == 'D');
                const bool is_datetime = (ft == 'T');
                if (rv.is_null) {
                    sql << "NULL";
                } else if (is_logical) {
                    // decode_value() (src/vfp/dbf_table.cpp) only ever
                    // produces exactly "true" or "false" for a recognized
                    // logical byte, or a raw single character (e.g. "?" for
                    // an uninitialized/blank field) otherwise -- map only
                    // the two recognized values, NULL for anything else,
                    // rather than defaulting an unrecognized value to FALSE.
                    const std::string& lv = rv.display_value;
                    if (lv == "true") {
                        sql << "TRUE";
                    } else if (lv == "false") {
                        sql << "FALSE";
                    } else {
                        sql << "NULL";
                    }
                } else if (is_numeric) {
                    // #5698: a blank numeric cell decodes to an empty
                    // display_value (not is_null) -- emit NULL rather than
                    // an empty string literal, which is not valid syntax
                    // inside a DECIMAL/INTEGER/DOUBLE PRECISION column on
                    // real SQL engines. A *non-blank* value that isn't a
                    // safe plain decimal literal (a numeric-overflow
                    // marker, malformed fixed-width numeric text, a
                    // nonfinite binary Double, or crafted/corrupted
                    // content) previously also became NULL -- silently
                    // changing the source data and erasing the reason the
                    // cell couldn't be represented, indistinguishable from
                    // a genuinely blank cell in the generated script. Fail
                    // the whole export closed instead, naming the table,
                    // row, and column, matching this exporter's own
                    // established fail-closed precedent (see
                    // write_oracle_tables_and_data()'s own
                    // hard_failure_error uses) for a case with no safe
                    // corrective action -- see
                    // looks_like_safe_unquoted_sql_numeric_literal()'s own
                    // comment for why this can't be validated by quoting
                    // instead.
                    if (rv.display_value.empty()) {
                        sql << "NULL";
                    } else if (looks_like_safe_unquoted_sql_numeric_literal(rv.display_value)) {
                        sql << rv.display_value;
                    } else {
                        hard_failure_error = asset_inspector_text(
                            "Vfp.AssetInspector.Validation.UnsafeNumericValue",
                            {{"table", rt.name}, {"row", std::to_string(row_number)},
                             {"column", rv.field_name}});
                        return {};
                    }
                } else if (is_date) {
                    // #5696: a blank VFP date field decodes to an empty
                    // display_value (not is_null), same as a blank numeric
                    // cell above -- emitting it as an empty string literal
                    // into a column declared DATE is invalid input on
                    // PostgreSQL (rejected outright) and silently stores the
                    // wrong SQL storage class on SQLite (a TEXT value in a
                    // NUMERIC-affinity column) rather than preserving the
                    // blank. Mirrors the fix already applied to the SQL
                    // Server/Oracle/MySQL writers (see
                    // write_sqlserver_tables_and_data()'s own comment for
                    // the real-engine-confirmed behavior). A non-blank
                    // value already decodes to a plain "YYYY-MM-DD" string
                    // (dbf_table.cpp's decode_value() 'D' case), which loads
                    // correctly as a DATE literal with no further handling
                    // needed here.
                    sql << (rv.display_value.empty() ? "NULL" : sql_quote_string_literal(rv.display_value));
                } else if (is_datetime) {
                    // The DBF decoder's T-type display_value is this
                    // codebase's internal "julian:<day> millis:<ms>" storage
                    // contract, not a SQL-loadable timestamp string -- convert
                    // it, or emit NULL if conversion isn't possible, rather
                    // than quoting the raw internal representation into a
                    // column declared TIMESTAMP.
                    const auto converted = sql_datetime_literal_from_storage(rv.display_value);
                    sql << (converted.has_value() ? sql_quote_string_literal(*converted) : "NULL");
                } else {
                    sql << sql_quote_string_literal(rv.display_value);
                }
                sql << (vi + 1U == rec.values.size() ? "" : ", ");
            }
            sql << ");\n";
        }
        sql << "\n";
        parsed_tables.push_back({.resolved = rt, .table = std::move(tbl.table)});
    }
    return parsed_tables;
}

// #5537: a tag's key_expression_hint maps to a plain column reference
// only when it is (trimmed, case-insensitively) an exact match for one of
// the table's own column names -- a composite expression (concatenation,
// function call, multiple columns) does not map cleanly to a single-
// column CREATE INDEX and is deliberately not guessed at here.
std::optional<std::string> plain_column_name_for_index_tag(
    const std::string& key_expression_hint,
    const std::vector<DbfFieldDescriptor>& fields) {
    const std::string trimmed = trim_copy(key_expression_hint);
    if (trimmed.empty()) {
        return std::nullopt;
    }
    const std::string upper_trimmed = uppercase_copy(trimmed);
    for (const auto& field : fields) {
        if (uppercase_copy(field.name) == upper_trimmed) {
            return field.name;
        }
    }
    return std::nullopt;
}

// #5537: emits a CREATE INDEX statement for each of `table`'s production
// CDX tags whose key expression is a plain column reference (see
// plain_column_name_for_index_tag()'s own comment), or a "-- skipped
// index" comment for one that is not -- never silently dropped. Only a
// same-base-name ".cdx" sidecar is consulted (VFP's conventional
// "production" compound index, auto-opened alongside the table, resolved
// case-insensitively via resolve_vfp_sidecar_path() -- #5545 review,
// Codex: a literal ".cdx" lookup would silently miss a real
// "TABLE.CDX" companion on a case-sensitive filesystem); a single-file
// .idx/.ndx/.mdx/.ntx companion is not scanned in this first slice. No
// per-tag uniqueness is captured by index_probe.cpp's CDX tag reader, so
// every emitted index is a plain (non-unique) CREATE INDEX. The
// generated index name incorporates the CDX tag's own name (unique
// within one CDX by construction, unlike the column it resolves to --
// #5545 review, Copilot and Codex both independently flagged that two
// distinct tags over the same plain column would otherwise generate the
// identical `<table>_<column>_idx` name, and PostgreSQL rejects a
// second CREATE INDEX for an already-existing relation name).
//
// #5559: PostgreSQL index names are schema-wide, not scoped to their own
// table (same fact independently confirmed for SQLite in the #5558
// review) -- a table named `a_b` with a tag named `c` and a table named
// `a` with a tag named `b_c` both concatenate to the identical
// `a_b_c_idx`, and real PostgreSQL rejects the second `CREATE INDEX`
// with "relation ... already exists" rather than silently accepting it.
// `used_index_names` tracks every index name already emitted across the
// *entire* export (not just the current table) via the same
// disambiguate_index_name() helper write_sqlite_create_indexes() uses.
//
// #5554 PR review (chatgpt-codex-connector): PostgreSQL's `NAMEDATALEN`
// limit is fundamentally a *byte* count (a fixed-size C struct), while
// SQL Server's `sysname` (`NVARCHAR(128)`) limit is a *character* count
// -- the two dialects genuinely need different truncation semantics, not
// just a shared byte count with a different constant.
enum class IdentifierLengthUnit { bytes, unicode_code_points };

// Returns the byte length of the UTF-8 sequence starting at `text[index]`
// (1 for ASCII/a stray continuation or invalid lead byte, up to 4 for a
// valid multi-byte lead byte), clamped so it never reads past
// `text.size()`.
std::size_t utf8_sequence_length_at(const std::string& text, std::size_t index) {
    const auto lead_byte = static_cast<unsigned char>(text[index]);
    std::size_t length = 1U;
    if ((lead_byte & 0xE0U) == 0xC0U) {
        length = 2U;
    } else if ((lead_byte & 0xF0U) == 0xE0U) {
        length = 3U;
    } else if ((lead_byte & 0xF8U) == 0xF0U) {
        length = 4U;
    }
    return std::min(length, text.size() - index);
}

std::size_t count_utf8_code_points(const std::string& text) {
    std::size_t count = 0U;
    std::size_t index = 0U;
    while (index < text.size()) {
        index += utf8_sequence_length_at(text, index);
        ++count;
    }
    return count;
}

std::size_t identifier_length_in_unit(const std::string& text, IdentifierLengthUnit unit) {
    return (unit == IdentifierLengthUnit::unicode_code_points)
        ? count_utf8_code_points(text)
        : text.size();
}

// #5554 PR review (chatgpt-codex-connector): a plain
// `text.substr(0, max_units)` byte cut can land inside a multi-byte
// UTF-8 sequence, emitting malformed UTF-8 into the generated script --
// a real risk for a non-ASCII VFP table name, which this codebase does
// not otherwise restrict to ASCII. `unit == bytes` truncates to at most
// `max_units` bytes, backing off to the last complete character boundary
// if the raw cut would split one (dropping the partial trailing
// sequence entirely, standard safe UTF-8 truncation practice) --
// PostgreSQL's own `NAMEDATALEN` truncation behaves the same way.
// `unit == unicode_code_points` instead counts whole Unicode code
// points (SQL Server's `sysname` is `NVARCHAR(128)`, a *character*
// limit, not a byte limit), which can never split a multi-byte sequence
// by construction. For an all-ASCII candidate (the common case: a CDX
// tag name is always ASCII by cdx_header.cpp's own
// looks_like_tag_name_candidate(); only a table name can carry non-ASCII
// bytes) both units agree exactly with a plain byte count, so this
// changes nothing for the existing PostgreSQL/SQLite regression fixtures.
std::string utf8_safe_truncate(
    const std::string& text, std::size_t max_units, IdentifierLengthUnit unit) {
    if (unit == IdentifierLengthUnit::unicode_code_points) {
        std::size_t code_points = 0U;
        std::size_t byte_index = 0U;
        while (byte_index < text.size() && code_points < max_units) {
            byte_index += utf8_sequence_length_at(text, byte_index);
            ++code_points;
        }
        return text.substr(0U, byte_index);
    }
    if (text.size() <= max_units) {
        return text;
    }
    std::size_t cut = max_units;
    while (cut > 0U && (static_cast<unsigned char>(text[cut]) & 0xC0U) == 0x80U) {
        --cut;
    }
    return text.substr(0U, cut);
}

// #5559: PostgreSQL silently truncates any identifier over
// `NAMEDATALEN - 1` (63) bytes to that length, so two distinct
// candidates that differ only after byte 63 -- or a single candidate
// at/over that length that needs a numeric suffix -- would still
// collide, or would have that suffix itself truncated away, inside the
// *real* engine even though this function's own untruncated bookkeeping
// saw them as unique. `max_identifier_length` (0 = no limit, used by
// write_sqlite_create_indexes() below, since SQLite imposes no such
// practical limit) truncates the base candidate to that length (in
// `unit`, see utf8_safe_truncate()'s own comment for why SQL Server
// needs a genuinely different unit from PostgreSQL) before the first
// uniqueness check, and reserves room for the numeric suffix by
// truncating further when one is needed, so every name this function
// actually records and emits is already what the target engine itself
// would see.
std::string disambiguate_index_name(
    const std::string& candidate,
    std::set<std::string>& used_index_names,
    std::size_t max_identifier_length = 0U,
    IdentifierLengthUnit unit = IdentifierLengthUnit::bytes) {
    const std::string base = (max_identifier_length > 0U)
        ? utf8_safe_truncate(candidate, max_identifier_length, unit)
        : candidate;
    if (used_index_names.insert(base).second) {
        return base;
    }
    std::size_t suffix = 2U;
    while (true) {
        const std::string suffix_text = "_" + std::to_string(suffix);
        std::string attempt = base;
        if (max_identifier_length > 0U &&
            identifier_length_in_unit(attempt, unit) + suffix_text.size() > max_identifier_length) {
            attempt = utf8_safe_truncate(attempt, max_identifier_length - suffix_text.size(), unit);
        }
        attempt += suffix_text;
        if (used_index_names.insert(attempt).second) {
            return attempt;
        }
        ++suffix;
    }
}

// PostgreSQL's default identifier limit (`NAMEDATALEN` is 64, so 63
// usable bytes after the implicit terminator) -- see
// disambiguate_index_name()'s own comment above.
constexpr std::size_t kPostgresqlMaxIdentifierBytes = 63U;

void write_postgresql_create_indexes(
    std::ostringstream& sql,
    const DatabaseCatalogSnapshot::ResolvedTable& rt,
    const std::vector<DbfFieldDescriptor>& fields,
    std::set<std::string>& used_index_names) {
    const SidecarPathResolution cdx_resolution = resolve_vfp_sidecar_path(rt.path, ".cdx");
    if (!cdx_resolution.path.has_value()) {
        // No production CDX at all (or an unresolvable case-fold
        // ambiguity, itself surfaced elsewhere by inspect_asset()'s own
        // diagnostics) -- a table with no compound index is a normal,
        // silent case, not a failure.
        return;
    }
    const std::string quoted_table = sql_quote_identifier(rt.name);
    const IndexParseResult index_result = parse_index_probe_from_file(
        copperfin::platform::path_to_utf8_string(*cdx_resolution.path));
    if (!index_result.ok || index_result.probe.kind != IndexKind::cdx) {
        // Unlike "no CDX at all" above, a companion that exists but
        // fails to parse or isn't recognized as CDX is a real, visible
        // loss of index information for this table -- #5545 review,
        // Copilot: report it rather than silently omitting every index
        // with no indication anything was skipped.
        sql << "-- skipped indexes on " << sql_sanitize_comment_text(rt.name)
            << ": companion .cdx exists but could not be read as a compound index\n\n";
        return;
    }

    bool wrote_anything = false;
    for (std::size_t tag_index = 0U; tag_index < index_result.probe.tags.size(); ++tag_index) {
        const IndexTagProbe& tag = index_result.probe.tags[tag_index];
        const auto column = plain_column_name_for_index_tag(tag.key_expression_hint, fields);
        const std::string tag_label = tag.name_hint.empty() ? std::string("(unnamed tag)") : tag.name_hint;
        if (!column.has_value()) {
            sql << "-- skipped index " << sql_sanitize_comment_text(tag_label)
                << " on " << sql_sanitize_comment_text(rt.name)
                << ": key expression is not a plain column reference\n";
            wrote_anything = true;
            continue;
        }
        // A tag with no captured name (name_hint empty, e.g. an
        // unresolved/inferred entry) still needs a unique identifier --
        // fall back to its ordinal position within this CDX rather than
        // a shared placeholder string every such tag would collide on.
        const std::string tag_identity =
            tag.name_hint.empty() ? ("tag" + std::to_string(tag_index)) : tag.name_hint;
        const std::string index_name = disambiguate_index_name(
            rt.name + "_" + tag_identity + "_idx", used_index_names, kPostgresqlMaxIdentifierBytes);
        sql << "CREATE INDEX " << sql_quote_identifier(index_name)
            << " ON " << quoted_table << " (" << sql_quote_identifier(*column) << ");\n";
        wrote_anything = true;
    }
    if (wrote_anything) {
        sql << "\n";
    }
}

// #5554: SQLite's own CREATE INDEX syntax is identical to PostgreSQL's
// for this exporter's plain-column-reference case (see
// write_postgresql_create_indexes()'s own comment for the full scope
// and documented non-goals -- composite/expression keys skipped as a
// comment, no per-tag uniqueness captured). Kept as its own dedicated
// function rather than a shared/renamed one, matching this file's own
// established per-vendor-dialect precedent (a future vendor-specific
// divergence has somewhere to go without touching another vendor's
// code path).
void write_sqlite_create_indexes(
    std::ostringstream& sql,
    const DatabaseCatalogSnapshot::ResolvedTable& rt,
    const std::vector<DbfFieldDescriptor>& fields,
    std::set<std::string>& used_index_names) {
    const SidecarPathResolution cdx_resolution = resolve_vfp_sidecar_path(rt.path, ".cdx");
    if (!cdx_resolution.path.has_value()) {
        return;
    }
    const std::string quoted_table = sql_quote_identifier(rt.name);
    const IndexParseResult index_result = parse_index_probe_from_file(
        copperfin::platform::path_to_utf8_string(*cdx_resolution.path));
    if (!index_result.ok || index_result.probe.kind != IndexKind::cdx) {
        sql << "-- skipped indexes on " << sql_sanitize_comment_text(rt.name)
            << ": companion .cdx exists but could not be read as a compound index\n\n";
        return;
    }

    bool wrote_anything = false;
    for (std::size_t tag_index = 0U; tag_index < index_result.probe.tags.size(); ++tag_index) {
        const IndexTagProbe& tag = index_result.probe.tags[tag_index];
        const auto column = plain_column_name_for_index_tag(tag.key_expression_hint, fields);
        const std::string tag_label = tag.name_hint.empty() ? std::string("(unnamed tag)") : tag.name_hint;
        if (!column.has_value()) {
            sql << "-- skipped index " << sql_sanitize_comment_text(tag_label)
                << " on " << sql_sanitize_comment_text(rt.name)
                << ": key expression is not a plain column reference\n";
            wrote_anything = true;
            continue;
        }
        const std::string tag_identity =
            tag.name_hint.empty() ? ("tag" + std::to_string(tag_index)) : tag.name_hint;
        const std::string index_name =
            disambiguate_index_name(rt.name + "_" + tag_identity + "_idx", used_index_names);
        sql << "CREATE INDEX " << sql_quote_identifier(index_name)
            << " ON " << quoted_table << " (" << sql_quote_identifier(*column) << ");\n";
        wrote_anything = true;
    }
    if (wrote_anything) {
        sql << "\n";
    }
}

// #5554: T-SQL's dialect diverges from the portable/ANSI-ish baseline
// write_sql_tables_and_data() emits enough that this exporter does not
// reuse it at all (the same reason export_database_as_access_sql() has
// its own inline loop) -- bracket identifiers and T-SQL-native column
// types via sqlserver_quote_identifier()/sqlserver_column_type(), and
// (the one INSERT-value-encoding difference from the shared writer) a
// BIT column's logical literal must be 1/0, not the TRUE/FALSE keyword
// the other three dialects all accept -- directly confirmed against a
// real local SQL Server 2022 engine: `INSERT ... VALUES (TRUE)` fails
// with "Invalid column name 'TRUE'" (T-SQL has no boolean-literal syntax
// outside a predicate context), while 1/0 loads and round-trips
// correctly. Kept as its own dedicated function per this file's
// established per-vendor precedent. Returns every successfully-parsed
// table the same way write_sql_tables_and_data() does, so
// write_sqlserver_create_indexes() doesn't have to re-open and re-parse
// the same .dbf a second time.
std::vector<ParsedSqlExportTable> write_sqlserver_tables_and_data(
    std::ostringstream& sql,
    const DatabaseCatalogSnapshot& snapshot,
    std::size_t row_limit,
    std::string& hard_failure_error) {
    std::vector<ParsedSqlExportTable> parsed_tables;
    for (const auto& rt : snapshot.resolved_tables) {
        DbfTableParseResult tbl = parse_dbf_table_from_file(
            copperfin::platform::path_to_utf8_string(rt.path), row_limit);
        if (!tbl.ok) {
            sql << "-- skipped table " << sql_sanitize_comment_text(rt.name) << ": "
                << sql_sanitize_comment_text(tbl.error) << "\n\n";
            continue;
        }
        if (tbl.table.fields.empty()) {
            // #5697: see write_sql_tables_and_data()'s own comment -- a
            // zero-field member table would otherwise emit invalid
            // `CREATE TABLE [name] (\n);` DDL.
            hard_failure_error = asset_inspector_text(
                "Vfp.AssetInspector.Validation.ExportTableHasNoFields",
                {{"table", rt.name}});
            return {};
        }

        const std::string quoted_table = sqlserver_quote_identifier(rt.name);
        sql << "CREATE TABLE " << quoted_table << " (\n";
        for (std::size_t fi = 0U; fi < tbl.table.fields.size(); ++fi) {
            const auto& fld = tbl.table.fields[fi];
            const bool last_field = (fi + 1U == tbl.table.fields.size());
            sql << "    " << sqlserver_quote_identifier(fld.name) << " "
                << sqlserver_column_type(fld.type, fld.length, fld.decimal_count)
                << (last_field ? "\n" : ",\n");
        }
        sql << ");\n\n";

        std::size_t row_number = 0U;
        for (const auto& rec : tbl.table.records) {
            if (rec.deleted) {
                continue;
            }
            ++row_number;
            sql << "INSERT INTO " << quoted_table << " (";
            for (std::size_t vi = 0U; vi < rec.values.size(); ++vi) {
                sql << sqlserver_quote_identifier(rec.values[vi].field_name)
                    << (vi + 1U == rec.values.size() ? "" : ", ");
            }
            sql << ") VALUES (";
            for (std::size_t vi = 0U; vi < rec.values.size(); ++vi) {
                const auto& rv = rec.values[vi];
                const char ft = static_cast<char>(
                    std::toupper(static_cast<unsigned char>(rv.field_type)));
                const bool is_numeric = (ft == 'N' || ft == 'F' || ft == 'I' || ft == 'B' || ft == 'Y');
                const bool is_logical = (ft == 'L');
                const bool is_date = (ft == 'D');
                const bool is_datetime = (ft == 'T');
                if (rv.is_null) {
                    sql << "NULL";
                } else if (is_logical) {
                    // T-SQL BIT literals are 1/0, not TRUE/FALSE -- see
                    // this function's own comment above.
                    const std::string& lv = rv.display_value;
                    if (lv == "true") {
                        sql << "1";
                    } else if (lv == "false") {
                        sql << "0";
                    } else {
                        sql << "NULL";
                    }
                } else if (is_numeric) {
                    // #5698: see write_sql_tables_and_data()'s own comment
                    // -- a non-blank value that isn't a safe plain decimal
                    // literal fails the whole export closed instead of
                    // silently becoming NULL.
                    if (rv.display_value.empty()) {
                        sql << "NULL";
                    } else if (looks_like_safe_unquoted_sql_numeric_literal(rv.display_value)) {
                        sql << rv.display_value;
                    } else {
                        hard_failure_error = asset_inspector_text(
                            "Vfp.AssetInspector.Validation.UnsafeNumericValue",
                            {{"table", rt.name}, {"row", std::to_string(row_number)},
                             {"column", rv.field_name}});
                        return {};
                    }
                } else if (is_date) {
                    // #5554 PR review (chatgpt-codex-connector, P1): a
                    // blank VFP date field decodes to an empty
                    // display_value (not is_null) -- emitting it as an
                    // empty string literal is not merely wrong syntax
                    // (unlike PostgreSQL/SQLite, which would reject it),
                    // it is silent data corruption: directly confirmed
                    // against a real local SQL Server 2022 engine that
                    // `CAST('' AS DATE)` succeeds and silently produces
                    // `1900-01-01`, inventing a date that was never
                    // there. Emit NULL instead, preserving the blank. A
                    // non-blank value already decodes to a plain
                    // "YYYY-MM-DD" string (dbf_table.cpp's decode_value()
                    // 'D' case), which loads correctly as a DATE literal
                    // with no further handling needed here.
                    sql << (rv.display_value.empty() ? "NULL" : sql_quote_string_literal(rv.display_value));
                } else if (is_datetime) {
                    const auto converted = sql_datetime_literal_from_storage(rv.display_value);
                    sql << (converted.has_value() ? sql_quote_string_literal(*converted) : "NULL");
                } else {
                    sql << sql_quote_string_literal(rv.display_value);
                }
                sql << (vi + 1U == rec.values.size() ? "" : ", ");
            }
            sql << ");\n";
        }
        sql << "\n";
        parsed_tables.push_back({.resolved = rt, .table = std::move(tbl.table)});
    }
    return parsed_tables;
}

// SQL Server's own identifier limit (`sysname` is `NVARCHAR(128)`, a
// *character* limit -- #5554 PR review, chatgpt-codex-connector, see
// utf8_safe_truncate()'s own comment for why this uses
// IdentifierLengthUnit::unicode_code_points rather than
// PostgreSQL's byte-based unit) -- see disambiguate_index_name()'s own
// comment above. Unlike PostgreSQL, which silently truncates an
// over-length identifier, a real SQL Server directly confirmed during
// this issue's own development *rejects* (error 103, "identifier ... is
// too long") any identifier over 128 characters outright rather than
// truncating it -- so this exporter truncates its own generated
// candidate to this limit *before* emission (the same
// disambiguate_index_name() mechanism #5559 already uses for
// PostgreSQL's 63-byte limit) to guarantee it never constructs a name
// the real engine would reject, even though a realistic VFP table+tag
// concatenation is very unlikely to reach 128 characters in practice.
constexpr std::size_t kSqlServerMaxIdentifierCodePoints = 128U;

// #5554: T-SQL's own CREATE INDEX syntax is identical to PostgreSQL's
// for this exporter's plain-column-reference case (see
// write_postgresql_create_indexes()'s own comment for the full scope
// and documented non-goals -- composite/expression keys skipped as a
// comment, no per-tag uniqueness captured). Kept as its own dedicated
// function rather than a shared/renamed one, matching this file's own
// established per-vendor-dialect precedent.
//
// Unlike PostgreSQL and SQLite (#5559, #5558), T-SQL index names are
// *not* schema-wide -- directly confirmed against a real local SQL
// Server 2022 engine: two different tables can each carry an index of
// the identical name with no error at all, and a table can be named
// identically to an unrelated table's own index without collision
// either. An index name only has to be unique *within the table it
// belongs to* (and, by construction, a CDX tag's own name is already
// unique within its own compound index, so two tags on the *same* table
// can never generate the same `<table>_<tag>_idx` candidate before
// truncation). The one place a same-table collision can still arise is
// truncation itself: if `rt.name` alone is at or beyond the 128-byte
// limit, two different tags' full candidates could truncate to the
// identical first-128-byte prefix (the tag-derived suffix never
// survives truncation at all). `used_index_names` is therefore a fresh,
// per-table set (not threaded in from the caller across the whole
// export, unlike write_postgresql_create_indexes()/
// write_sqlite_create_indexes()) -- exactly scoped to the collision that
// can actually occur on this engine.
void write_sqlserver_create_indexes(
    std::ostringstream& sql,
    const DatabaseCatalogSnapshot::ResolvedTable& rt,
    const std::vector<DbfFieldDescriptor>& fields) {
    const SidecarPathResolution cdx_resolution = resolve_vfp_sidecar_path(rt.path, ".cdx");
    if (!cdx_resolution.path.has_value()) {
        return;
    }
    const std::string quoted_table = sqlserver_quote_identifier(rt.name);
    const IndexParseResult index_result = parse_index_probe_from_file(
        copperfin::platform::path_to_utf8_string(*cdx_resolution.path));
    if (!index_result.ok || index_result.probe.kind != IndexKind::cdx) {
        sql << "-- skipped indexes on " << sql_sanitize_comment_text(rt.name)
            << ": companion .cdx exists but could not be read as a compound index\n\n";
        return;
    }

    std::set<std::string> used_index_names;
    bool wrote_anything = false;
    for (std::size_t tag_index = 0U; tag_index < index_result.probe.tags.size(); ++tag_index) {
        const IndexTagProbe& tag = index_result.probe.tags[tag_index];
        const auto column = plain_column_name_for_index_tag(tag.key_expression_hint, fields);
        const std::string tag_label = tag.name_hint.empty() ? std::string("(unnamed tag)") : tag.name_hint;
        if (!column.has_value()) {
            sql << "-- skipped index " << sql_sanitize_comment_text(tag_label)
                << " on " << sql_sanitize_comment_text(rt.name)
                << ": key expression is not a plain column reference\n";
            wrote_anything = true;
            continue;
        }
        const std::string tag_identity =
            tag.name_hint.empty() ? ("tag" + std::to_string(tag_index)) : tag.name_hint;
        const std::string index_name = disambiguate_index_name(
            rt.name + "_" + tag_identity + "_idx", used_index_names,
            kSqlServerMaxIdentifierCodePoints, IdentifierLengthUnit::unicode_code_points);
        sql << "CREATE INDEX " << sqlserver_quote_identifier(index_name)
            << " ON " << quoted_table << " (" << sqlserver_quote_identifier(*column) << ");\n";
        wrote_anything = true;
    }
    if (wrote_anything) {
        sql << "\n";
    }
}

// #5564 PR review (chatgpt-codex-connector, P1): a CLOB column's own
// literal cannot be a plain quoted string the way every other
// character-ish dialect's own memo/text fallback emits one. An empty
// string literal silently becomes NULL for a CLOB column -- directly
// confirmed against a real local Oracle 23ai engine (`INSERT INTO ...
// VALUES ('')` leaves the column NULL, not an actual empty CLOB) --
// corrupting a genuinely blank memo into a NULL one. And Oracle's own
// SQL text-literal limit is 4000 *bytes*, not characters -- directly
// confirmed: a 4000-byte literal succeeds, a 4001-byte one fails with
// ORA-01704 ("string literal too long"), and 2000 two-byte UTF-8
// characters (4000 bytes) succeed while 2001 of them (4002 bytes) fail
// the identical way -- a real VFP memo can easily exceed that, which
// would otherwise silently truncate the entire generated script
// partway through a single `INSERT`. `EMPTY_CLOB()` and chunked
// `TO_CLOB('...') || TO_CLOB('...')` concatenation (each chunk within
// the 4000-byte limit, split on a UTF-8 character boundary via the
// same `utf8_safe_truncate()` byte-mode this file's own identifier
// truncation already uses, so a chunk boundary can never split a
// multi-byte character) are both directly confirmed against the real
// engine to produce a correct, full-length, non-NULL CLOB value.
std::string oracle_clob_literal(const std::string& text) {
    if (text.empty()) {
        return "EMPTY_CLOB()";
    }
    constexpr std::size_t max_chunk_bytes = 4000U;
    std::string result;
    std::string remaining = text;
    while (!remaining.empty()) {
        const std::string chunk =
            utf8_safe_truncate(remaining, max_chunk_bytes, IdentifierLengthUnit::bytes);
        if (chunk.empty()) {
            // utf8_safe_truncate() only returns empty here if even the
            // first character's own byte sequence exceeds
            // max_chunk_bytes, which cannot happen for any valid UTF-8
            // character (at most 4 bytes) against a 4000-byte budget --
            // guarded defensively rather than looping forever on
            // malformed input.
            break;
        }
        if (!result.empty()) {
            result += " || ";
        }
        result += "TO_CLOB(" + sql_quote_string_literal(chunk) + ")";
        remaining.erase(0U, chunk.size());
    }
    return result;
}

// #5564 PR review (chatgpt-codex-connector, P2): oracle_quote_identifier()'s
// own embedded-quote stripping is not collision-safe -- distinct source
// names (e.g. "ab" and "a\"b") can sanitize to the identical quoted
// identifier. Silently emitting a colliding identifier a second time
// would produce a script that either fails outright (Oracle rejects a
// duplicate column name in one `CREATE TABLE`) or, worse, succeeds
// while silently targeting the wrong column/table. Returns true (and
// records `quoted_identifier`) the first time a given sanitized
// identifier is seen in `used_identifiers`; returns false on a genuine
// collision, letting the caller fail the export closed rather than
// report success for an unusable script -- this codebase's own
// established practice for a case with no safe corrective action
// (matching, e.g., an unbalanced `Begin`/`End` block failing closed in
// `parse_access_saveastext_design()` rather than guessing a repair).
bool oracle_record_identifier_or_detect_collision(
    const std::string& quoted_identifier, std::set<std::string>& used_identifiers) {
    return used_identifiers.insert(quoted_identifier).second;
}

// #5554: Oracle's dialect diverges from the portable/ANSI-ish baseline
// enough that this exporter does not reuse write_sql_tables_and_data()
// at all (the same reason export_database_as_sqlserver_sql()/
// export_database_as_access_sql() each have their own inline loop):
// oracle_quote_identifier()/oracle_column_type() for identifiers/types,
// and two INSERT-value-encoding differences from the shared writer.
// First, like SQL Server's own BIT, a NUMBER(1) logical column's
// literal is 1/0 -- directly confirmed against a real local Oracle 23ai
// engine that, while `INSERT ... VALUES (TRUE)` does succeed there
// (Oracle 23c's own new implicit boolean-to-number conversion), this
// exporter targets 1/0 instead since it is the one literal form every
// Oracle version -- not just the newest major release -- accepts
// identically. Second, and unlike every other dialect this file emits
// (all of which quote a 'D'/'T' value as a plain string that the target
// engine implicitly converts), a bare `'2026-01-15'` string is *not*
// safe for an Oracle DATE/TIMESTAMP column: implicit string-to-date
// conversion depends on the session's own NLS_DATE_FORMAT, which
// defaults to `DD-MON-RR` (directly confirmed against the real engine:
// `INSERT INTO ... VALUES ('2026-01-15')` into a DATE column fails with
// ORA-01861, "literal does not match format string"), not the ISO
// `YYYY-MM-DD` shape this codebase's own decode_value() produces.
// Oracle's own ANSI-style `DATE 'YYYY-MM-DD'`/`TIMESTAMP 'YYYY-MM-DD
// HH:MM:SS'` literal syntax is documented to always parse in that exact
// ISO shape regardless of NLS settings, directly confirmed against the
// real engine. A blank VFP date's own empty display_value therefore
// cannot merely fall back to a quoted empty string the way SQL Server's
// own (data-corrupting, but at least *not erroring*) blank-date case
// does: `DATE ''` is not valid syntax at all (directly confirmed:
// ORA-01841, "(full) year must be between -4713 and +9999, and not be
// 0") -- so a blank date must resolve to NULL for this exporter to
// produce a script that loads at all, not merely one that avoids
// silently inventing data the way SQL Server's own #5562-era fix cared
// about.
std::vector<ParsedSqlExportTable> write_oracle_tables_and_data(
    std::ostringstream& sql,
    const DatabaseCatalogSnapshot& snapshot,
    std::size_t row_limit,
    std::string& hard_failure_error) {
    std::vector<ParsedSqlExportTable> parsed_tables;
    // #5564 PR review (chatgpt-codex-connector, P2): tracks every
    // sanitized-and-quoted table name across the *entire* export (mirroring
    // the schema-wide scope oracle_quote_identifier()'s own output
    // occupies) so two distinct source table names that collide after
    // quote-stripping are caught here rather than producing a script with
    // a duplicate/wrong-target CREATE TABLE.
    std::set<std::string> used_table_names;
    for (const auto& rt : snapshot.resolved_tables) {
        DbfTableParseResult tbl = parse_dbf_table_from_file(
            copperfin::platform::path_to_utf8_string(rt.path), row_limit);
        if (!tbl.ok) {
            sql << "-- skipped table " << sql_sanitize_comment_text(rt.name) << ": "
                << sql_sanitize_comment_text(tbl.error) << "\n\n";
            continue;
        }
        if (tbl.table.fields.empty()) {
            // #5697: see write_sql_tables_and_data()'s own comment -- a
            // zero-field member table would otherwise emit invalid
            // `CREATE TABLE "name" (\n);` DDL.
            hard_failure_error = asset_inspector_text(
                "Vfp.AssetInspector.Validation.ExportTableHasNoFields",
                {{"table", rt.name}});
            return {};
        }

        const std::string quoted_table = oracle_quote_identifier(rt.name);
        if (!oracle_record_identifier_or_detect_collision(quoted_table, used_table_names)) {
            hard_failure_error = asset_inspector_text(
                "Vfp.AssetInspector.Validation.OracleIdentifierCollision",
                {{"identifier", quoted_table}});
            return {};
        }

        sql << "CREATE TABLE " << quoted_table << " (\n";
        // Scoped to this one table -- two distinct column names
        // colliding after sanitization is a per-table concern (Oracle's
        // own column namespace is per-table, unlike its schema-wide
        // table/index namespaces).
        std::set<std::string> used_column_names;
        for (std::size_t fi = 0U; fi < tbl.table.fields.size(); ++fi) {
            const auto& fld = tbl.table.fields[fi];
            const std::string quoted_column = oracle_quote_identifier(fld.name);
            if (!oracle_record_identifier_or_detect_collision(quoted_column, used_column_names)) {
                hard_failure_error = asset_inspector_text(
                    "Vfp.AssetInspector.Validation.OracleIdentifierCollision",
                    {{"identifier", quoted_column}});
                return {};
            }
            const bool last_field = (fi + 1U == tbl.table.fields.size());
            sql << "    " << quoted_column << " "
                << oracle_column_type(fld.type, fld.length, fld.decimal_count)
                << (last_field ? "\n" : ",\n");
        }
        sql << ");\n\n";

        // #5717 PR review (chatgpt-codex-connector, P1): this codebase does
        // not currently decode a VFP nullable field's own `_NullFlags`
        // record bitmap at all -- `DbfFieldDescriptor` drops the
        // descriptor's own nullable flag entirely, and `decode_value()`
        // only ever sets `is_null` when decoding the special type-`0`
        // pseudo-field itself, never applying its bits back to the real
        // field the bitmap actually describes. That means, right now, a
        // genuinely non-null empty value and a genuinely null value in a
        // *nullable* field are indistinguishable in this codebase's own
        // in-memory representation (both are `is_null=false,
        // display_value=""`) -- the empty-character check below cannot
        // safely apply to a table that declares any nullable field at
        // all, since it would then reject exports of a genuinely null
        // field the same way it (correctly) rejects a genuinely non-null
        // empty one, a real regression from this fix's own first version,
        // caught by this review round rather than shipped. A table with
        // *no* `_NullFlags` pseudo-field, however, has no nullable fields
        // at all -- VFP only ever adds that hidden bookkeeping field when
        // at least one real field is marked nullable -- so a blank `C`/`V`
        // value there can only ever mean "genuinely non-null empty," never
        // "null," with no ambiguity. Scoping the check to that unambiguous
        // case specifically (rather than dropping it entirely) still
        // closes #5693's own demonstrated repro, which used an ordinary
        // table with no nullable fields declared. Proper `_NullFlags`
        // bitmap decoding -- needed to close the ambiguous case too -- is
        // tracked separately as a foundational, cross-cutting gap
        // affecting every dialect and format this codebase reads, not
        // something specific to Oracle's own export.
        const bool table_declares_nullable_fields = std::any_of(
            tbl.table.fields.begin(), tbl.table.fields.end(),
            [](const DbfFieldDescriptor& field) { return field.type == '0'; });

        std::size_t row_number = 0U;
        for (const auto& rec : tbl.table.records) {
            if (rec.deleted) {
                continue;
            }
            ++row_number;
            sql << "INSERT INTO " << quoted_table << " (";
            for (std::size_t vi = 0U; vi < rec.values.size(); ++vi) {
                sql << oracle_quote_identifier(rec.values[vi].field_name)
                    << (vi + 1U == rec.values.size() ? "" : ", ");
            }
            sql << ") VALUES (";
            for (std::size_t vi = 0U; vi < rec.values.size(); ++vi) {
                const auto& rv = rec.values[vi];
                const char ft = static_cast<char>(
                    std::toupper(static_cast<unsigned char>(rv.field_type)));
                const bool is_numeric = (ft == 'N' || ft == 'F' || ft == 'I' || ft == 'B' || ft == 'Y');
                const bool is_logical = (ft == 'L');
                const bool is_date = (ft == 'D');
                const bool is_datetime = (ft == 'T');
                const bool is_character = (ft == 'C' || ft == 'V');
                if (rv.is_null) {
                    sql << "NULL";
                } else if (is_logical) {
                    // Oracle NUMBER(1) literals are 1/0 -- see this
                    // function's own comment above.
                    const std::string& lv = rv.display_value;
                    if (lv == "true") {
                        sql << "1";
                    } else if (lv == "false") {
                        sql << "0";
                    } else {
                        sql << "NULL";
                    }
                } else if (is_numeric) {
                    // #5698: see write_sql_tables_and_data()'s own comment
                    // -- a non-blank value that isn't a safe plain decimal
                    // literal fails the whole export closed instead of
                    // silently becoming NULL.
                    if (rv.display_value.empty()) {
                        sql << "NULL";
                    } else if (looks_like_safe_unquoted_sql_numeric_literal(rv.display_value)) {
                        sql << rv.display_value;
                    } else {
                        hard_failure_error = asset_inspector_text(
                            "Vfp.AssetInspector.Validation.UnsafeNumericValue",
                            {{"table", rt.name}, {"row", std::to_string(row_number)},
                             {"column", rv.field_name}});
                        return {};
                    }
                } else if (is_date) {
                    // A blank date must become NULL, not `DATE ''` --
                    // see this function's own comment above for why
                    // that specific literal form is invalid Oracle
                    // syntax outright, not merely lossy. A non-blank
                    // value already decodes to a plain "YYYY-MM-DD"
                    // string (dbf_table.cpp's decode_value() 'D' case),
                    // which the ANSI DATE literal wrapper accepts
                    // exactly, independent of session NLS_DATE_FORMAT.
                    sql << (rv.display_value.empty()
                        ? "NULL"
                        : ("DATE " + sql_quote_string_literal(rv.display_value)));
                } else if (is_datetime) {
                    const auto converted = sql_datetime_literal_from_storage(rv.display_value);
                    sql << (converted.has_value()
                        ? ("TIMESTAMP " + sql_quote_string_literal(*converted))
                        : "NULL");
                } else if (is_character) {
                    // #5693: Oracle treats a zero-length VARCHAR2 literal
                    // as NULL -- directly confirmed against a real local
                    // Oracle 23ai engine (`INSERT INTO ... VALUES ('')`
                    // into a VARCHAR2(n CHAR) column leaves it NULL, not
                    // an actual empty string: `IS NULL` reports TRUE).
                    // Unlike a blank Date (which has a real NULL
                    // representation to fall back to with no distinction
                    // lost) or a blank Memo (which CLOB's own EMPTY_CLOB()
                    // already represents distinctly from NULL, see
                    // oracle_clob_literal()'s own comment), there is no
                    // VARCHAR2 literal at all that preserves a genuinely
                    // non-null empty Character/Varchar value's own
                    // non-null-ness -- any literal this exporter could
                    // emit either isn't empty (wrong value) or is empty
                    // (silently becomes NULL, corrupting the distinction).
                    // Rather than report a successful export whose
                    // ordinary load silently turns a non-null empty value
                    // into NULL, this fails the whole export closed with a
                    // diagnostic naming the table and column, matching
                    // this exporter's own established fail-closed
                    // precedent for a case with no safe corrective action
                    // -- but see this loop's own comment above for why
                    // this only applies unambiguously to a table with no
                    // nullable fields at all.
                    if (!table_declares_nullable_fields && rv.display_value.empty()) {
                        hard_failure_error = asset_inspector_text(
                            "Vfp.AssetInspector.Validation.OracleEmptyCharacterValueUnrepresentable",
                            {{"table", rt.name}, {"column", rv.field_name}});
                        return {};
                    }
                    // VARCHAR2's own max length (255-CHAR-derived, well
                    // under Oracle's 4000-byte literal ceiling even at
                    // 4 UTF-8 bytes per character) never needs the CLOB
                    // chunking below -- a plain literal is always safe.
                    sql << sql_quote_string_literal(rv.display_value);
                } else {
                    // M, G, P, and any other/unrecognized storage type
                    // -- see oracle_clob_literal()'s own comment for
                    // why a CLOB column cannot share the plain-literal
                    // handling above.
                    sql << oracle_clob_literal(rv.display_value);
                }
                sql << (vi + 1U == rec.values.size() ? "" : ", ");
            }
            sql << ");\n";
        }
        sql << "\n";
        parsed_tables.push_back({.resolved = rt, .table = std::move(tbl.table)});
    }
    return parsed_tables;
}

// Oracle's own identifier limit (128 *bytes* -- a real, material
// difference from SQL Server's own character-counted `sysname`, see
// utf8_safe_truncate()'s own comment on why the two need genuinely
// different IdentifierLengthUnit values): directly confirmed against a
// real local Oracle 23ai engine that a 128-byte identifier succeeds
// while a 129-byte one fails outright with ORA-00972 ("identifier ...
// exceeds the maximum length of 128 bytes") -- the same hard-rejecting
// (not silently-truncating) failure mode #5554 already established for
// SQL Server, just byte-counted like PostgreSQL's own 63-byte limit
// rather than character-counted.
constexpr std::size_t kOracleMaxIdentifierBytes = 128U;

// #5554: Oracle's own CREATE INDEX syntax is identical to PostgreSQL's
// for this exporter's plain-column-reference case (see
// write_postgresql_create_indexes()'s own comment for the full scope
// and documented non-goals -- composite/expression keys skipped as a
// comment, no per-tag uniqueness captured). Kept as its own dedicated
// function rather than a shared/renamed one, matching this file's own
// established per-vendor-dialect precedent.
//
// Oracle's own namespace rules are a genuine hybrid of the other two
// patterns this file already handles, directly confirmed against a
// real local Oracle 23ai engine rather than assumed from either
// precedent: like PostgreSQL/SQLite (#5559, #5558), and *unlike* SQL
// Server, index names must be unique *schema-wide* -- two different
// tables cannot each carry an index of the identical name (ORA-00955,
// "name is already used by an existing object"). But like SQL Server,
// and *unlike* PostgreSQL, tables and indexes live in *separate*
// namespaces -- a table can share its own name with an unrelated
// table's index with no collision at all (Oracle's own public "Schema
// Object Namespaces" reference documents this: TABLES/VIEWS/SEQUENCES/
// private synonyms share one namespace, while INDEXES/CLUSTERS have
// their own). `used_index_names` is therefore threaded across the
// *whole* export the way PostgreSQL's/SQLite's own sets are, but --
// unlike PostgreSQL's own #5559 fix -- deliberately *not* seeded with
// already-emitted table names, since that specific collision cannot
// occur on this engine.
void write_oracle_create_indexes(
    std::ostringstream& sql,
    const DatabaseCatalogSnapshot::ResolvedTable& rt,
    const std::vector<DbfFieldDescriptor>& fields,
    std::set<std::string>& used_index_names) {
    const SidecarPathResolution cdx_resolution = resolve_vfp_sidecar_path(rt.path, ".cdx");
    if (!cdx_resolution.path.has_value()) {
        return;
    }
    const std::string quoted_table = oracle_quote_identifier(rt.name);
    const IndexParseResult index_result = parse_index_probe_from_file(
        copperfin::platform::path_to_utf8_string(*cdx_resolution.path));
    if (!index_result.ok || index_result.probe.kind != IndexKind::cdx) {
        sql << "-- skipped indexes on " << sql_sanitize_comment_text(rt.name)
            << ": companion .cdx exists but could not be read as a compound index\n\n";
        return;
    }

    bool wrote_anything = false;
    for (std::size_t tag_index = 0U; tag_index < index_result.probe.tags.size(); ++tag_index) {
        const IndexTagProbe& tag = index_result.probe.tags[tag_index];
        const auto column = plain_column_name_for_index_tag(tag.key_expression_hint, fields);
        const std::string tag_label = tag.name_hint.empty() ? std::string("(unnamed tag)") : tag.name_hint;
        if (!column.has_value()) {
            sql << "-- skipped index " << sql_sanitize_comment_text(tag_label)
                << " on " << sql_sanitize_comment_text(rt.name)
                << ": key expression is not a plain column reference\n";
            wrote_anything = true;
            continue;
        }
        const std::string tag_identity =
            tag.name_hint.empty() ? ("tag" + std::to_string(tag_index)) : tag.name_hint;
        const std::string index_name = disambiguate_index_name(
            rt.name + "_" + tag_identity + "_idx", used_index_names, kOracleMaxIdentifierBytes);
        sql << "CREATE INDEX " << oracle_quote_identifier(index_name)
            << " ON " << quoted_table << " (" << oracle_quote_identifier(*column) << ");\n";
        wrote_anything = true;
    }
    if (wrote_anything) {
        sql << "\n";
    }
}

}  // namespace

DatabaseSqlExportResult export_database_as_sql(
    const std::string& dbc_path,
    std::size_t max_rows_per_table) {

    const DatabaseCatalogSnapshot snapshot = load_database_catalog_snapshot(dbc_path);
    if (!snapshot.ok) {
        return {.ok = false, .error = snapshot.error, .sql = {}};
    }

    std::ostringstream sql;
    sql.imbue(std::locale::classic());
    sql << "-- Copperfin EXPORT DATABASE ... TYPE SQL\n";
    sql << "-- database: " << sql_sanitize_comment_text(snapshot.db_name) << "\n";
    sql << "-- source: " << sql_sanitize_comment_text(dbc_path) << "\n\n";

    const std::size_t row_limit = (max_rows_per_table == 0U)
        ? std::numeric_limits<std::size_t>::max()
        : max_rows_per_table;

    std::string hard_failure_error;
    write_sql_tables_and_data(sql, snapshot, row_limit, hard_failure_error);
    // #5697: a member table with zero fields fails the whole export
    // closed rather than emit invalid `CREATE TABLE "name" ( );` DDL.
    // #5698: a non-blank numeric value that cannot be safely represented
    // shares this same hard_failure_error path rather than silently
    // substituting NULL. See write_sql_tables_and_data()'s own comment
    // for both.
    if (!hard_failure_error.empty()) {
        return {.ok = false, .error = hard_failure_error, .sql = {}};
    }

    return {.ok = true, .error = {}, .sql = sql.str()};
}

DatabaseSqlExportResult export_database_as_postgresql_sql(
    const std::string& dbc_path,
    std::size_t max_rows_per_table) {

    const DatabaseCatalogSnapshot snapshot = load_database_catalog_snapshot(dbc_path);
    if (!snapshot.ok) {
        return {.ok = false, .error = snapshot.error, .sql = {}};
    }

    std::ostringstream sql;
    sql.imbue(std::locale::classic());
    sql << "-- Copperfin EXPORT DATABASE ... TYPE POSTGRESQL\n";
    sql << "-- database: " << sql_sanitize_comment_text(snapshot.db_name) << "\n";
    sql << "-- source: " << sql_sanitize_comment_text(dbc_path) << "\n\n";

    const std::size_t row_limit = (max_rows_per_table == 0U)
        ? std::numeric_limits<std::size_t>::max()
        : max_rows_per_table;

    std::string hard_failure_error;
    const std::vector<ParsedSqlExportTable> parsed_tables =
        write_sql_tables_and_data(sql, snapshot, row_limit, hard_failure_error);
    // #5697: a member table with zero fields fails the whole export
    // closed rather than emit invalid `CREATE TABLE "name" ( );` DDL.
    // #5698: a non-blank numeric value that cannot be safely represented
    // shares this same hard_failure_error path rather than silently
    // substituting NULL. See write_sql_tables_and_data()'s own comment
    // for both.
    if (!hard_failure_error.empty()) {
        return {.ok = false, .error = hard_failure_error, .sql = {}};
    }

    // #5559 review (chatgpt-codex-connector): PostgreSQL puts tables and
    // indexes in the same schema-wide relation namespace -- a valid
    // catalog table literally named e.g. "A_B_CDEF_idx" (already emitted
    // above by write_sql_tables_and_data()'s own CREATE TABLE) would
    // collide with an index this function is about to generate for an
    // unrelated table+tag pair that happens to produce the identical
    // name. Seeding `used_index_names` with every table name already
    // emitted lets disambiguate_index_name() catch and suffix that case
    // exactly like a same-named index from a different table.
    std::set<std::string> used_index_names;
    for (const auto& parsed : parsed_tables) {
        used_index_names.insert(parsed.resolved.name);
    }
    for (const auto& parsed : parsed_tables) {
        write_postgresql_create_indexes(sql, parsed.resolved, parsed.table.fields, used_index_names);
    }

    return {.ok = true, .error = {}, .sql = sql.str()};
}

DatabaseSqlExportResult export_database_as_sqlite_sql(
    const std::string& dbc_path,
    std::size_t max_rows_per_table) {

    const DatabaseCatalogSnapshot snapshot = load_database_catalog_snapshot(dbc_path);
    if (!snapshot.ok) {
        return {.ok = false, .error = snapshot.error, .sql = {}};
    }

    std::ostringstream sql;
    sql.imbue(std::locale::classic());
    sql << "-- Copperfin EXPORT DATABASE ... TYPE SQLITE\n";
    sql << "-- database: " << sql_sanitize_comment_text(snapshot.db_name) << "\n";
    sql << "-- source: " << sql_sanitize_comment_text(dbc_path) << "\n\n";

    const std::size_t row_limit = (max_rows_per_table == 0U)
        ? std::numeric_limits<std::size_t>::max()
        : max_rows_per_table;

    std::string hard_failure_error;
    const std::vector<ParsedSqlExportTable> parsed_tables =
        write_sql_tables_and_data(sql, snapshot, row_limit, hard_failure_error);
    // #5697: a member table with zero fields fails the whole export
    // closed rather than emit invalid `CREATE TABLE "name" ( );` DDL.
    // #5698: a non-blank numeric value that cannot be safely represented
    // shares this same hard_failure_error path rather than silently
    // substituting NULL. See write_sql_tables_and_data()'s own comment
    // for both.
    if (!hard_failure_error.empty()) {
        return {.ok = false, .error = hard_failure_error, .sql = {}};
    }

    std::set<std::string> used_index_names;
    for (const auto& parsed : parsed_tables) {
        write_sqlite_create_indexes(sql, parsed.resolved, parsed.table.fields, used_index_names);
    }

    return {.ok = true, .error = {}, .sql = sql.str()};
}

DatabaseSqlExportResult export_database_as_access_sql(
    const std::string& dbc_path,
    std::size_t max_rows_per_table) {

    const DatabaseCatalogSnapshot snapshot = load_database_catalog_snapshot(dbc_path);
    if (!snapshot.ok) {
        return {.ok = false, .error = snapshot.error, .sql = {}};
    }

    std::ostringstream sql;
    sql.imbue(std::locale::classic());
    // Unlike export_database_as_sql()'s ANSI dialect, this script carries
    // no "-- ..." header/provenance or skipped-table comment lines at
    // all: independently verified research confirms native Jet/ACE SQL
    // (the engine actually executing this text, whether through Access's
    // interactive SQL View or a DAO/ADO Execute() call) has no supported
    // in-band comment syntax -- neither "--" nor "/* */" -- so embedding
    // one here would make the generated script fail exactly where this
    // function's whole purpose is to succeed. A skipped table (one whose
    // underlying .dbf failed to parse) therefore simply contributes
    // nothing to the output, the same as it would for a table with zero
    // rows, rather than a diagnostic comment a real engine cannot run.

    const std::size_t row_limit = (max_rows_per_table == 0U)
        ? std::numeric_limits<std::size_t>::max()
        : max_rows_per_table;

    for (const auto& rt : snapshot.resolved_tables) {
        const DbfTableParseResult tbl = parse_dbf_table_from_file(
            copperfin::platform::path_to_utf8_string(rt.path), row_limit);
        if (!tbl.ok) {
            continue;
        }
        if (tbl.table.fields.empty()) {
            // #5697: see write_sql_tables_and_data()'s own comment -- a
            // zero-field member table would otherwise emit invalid
            // `CREATE TABLE [name] (\n);` DDL.
            return {.ok = false, .error = asset_inspector_text(
                "Vfp.AssetInspector.Validation.ExportTableHasNoFields",
                {{"table", rt.name}}), .sql = {}};
        }

        const std::string quoted_table = access_quote_identifier(rt.name);
        sql << "CREATE TABLE " << quoted_table << " (\n";
        for (std::size_t fi = 0U; fi < tbl.table.fields.size(); ++fi) {
            const auto& fld = tbl.table.fields[fi];
            const bool last_field = (fi + 1U == tbl.table.fields.size());
            sql << "    " << access_quote_identifier(fld.name) << " "
                << access_column_type(fld.type, fld.length, fld.decimal_count)
                << (last_field ? "\n" : ",\n");
        }
        sql << ");\n\n";

        std::size_t row_number = 0U;
        for (const auto& rec : tbl.table.records) {
            if (rec.deleted) {
                continue;
            }
            ++row_number;
            sql << "INSERT INTO " << quoted_table << " (";
            for (std::size_t vi = 0U; vi < rec.values.size(); ++vi) {
                sql << access_quote_identifier(rec.values[vi].field_name)
                    << (vi + 1U == rec.values.size() ? "" : ", ");
            }
            sql << ") VALUES (";
            for (std::size_t vi = 0U; vi < rec.values.size(); ++vi) {
                const auto& rv = rec.values[vi];
                const char ft = static_cast<char>(
                    std::toupper(static_cast<unsigned char>(rv.field_type)));
                const bool is_numeric = (ft == 'N' || ft == 'F' || ft == 'I' || ft == 'B' || ft == 'Y');
                const bool is_logical = (ft == 'L');
                const bool is_date = (ft == 'D');
                const bool is_datetime = (ft == 'T');
                if (rv.is_null) {
                    sql << "NULL";
                } else if (is_logical) {
                    const std::string& lv = rv.display_value;
                    if (lv == "true") {
                        sql << "TRUE";
                    } else if (lv == "false") {
                        sql << "FALSE";
                    } else {
                        sql << "NULL";
                    }
                } else if (is_numeric) {
                    // #5698: see write_sql_tables_and_data()'s own comment
                    // -- a non-blank value that isn't a safe plain decimal
                    // literal fails the whole export closed instead of
                    // silently becoming NULL.
                    if (rv.display_value.empty()) {
                        sql << "NULL";
                    } else if (looks_like_safe_unquoted_sql_numeric_literal(rv.display_value)) {
                        sql << rv.display_value;
                    } else {
                        return {.ok = false, .error = asset_inspector_text(
                            "Vfp.AssetInspector.Validation.UnsafeNumericValue",
                            {{"table", rt.name}, {"row", std::to_string(row_number)},
                             {"column", rv.field_name}}), .sql = {}};
                    }
                } else if (is_date) {
                    // decode_value()'s 'D' case (dbf_table.cpp) formats a
                    // well-formed value as "YYYY-MM-DD" (empty string for
                    // a blank date), which embeds directly inside Access
                    // SQL's #...# date-literal delimiters -- ISO form
                    // specifically, since Access accepts it unambiguously
                    // regardless of the connection's regional date format,
                    // unlike locale-dependent MM/DD/YYYY. A value that
                    // isn't blank and doesn't match that exact shape (a
                    // corrupt/crafted source whose 'D' field isn't the
                    // expected 8 raw bytes) becomes NULL instead of being
                    // trusted to embed safely -- see
                    // looks_like_safe_access_sql_date_literal()'s comment.
                    if (rv.display_value.empty()) {
                        sql << "NULL";
                    } else if (looks_like_safe_access_sql_date_literal(rv.display_value)) {
                        sql << "#" << rv.display_value << "#";
                    } else {
                        sql << "NULL";
                    }
                } else if (is_datetime) {
                    const auto converted = sql_datetime_literal_from_storage(rv.display_value);
                    sql << (converted.has_value() ? ("#" + *converted + "#") : "NULL");
                } else {
                    sql << sql_quote_string_literal(rv.display_value);
                }
                sql << (vi + 1U == rec.values.size() ? "" : ", ");
            }
            sql << ");\n";
        }
        sql << "\n";
    }

    return {.ok = true, .error = {}, .sql = sql.str()};
}

DatabaseSqlExportResult export_database_as_sqlserver_sql(
    const std::string& dbc_path,
    std::size_t max_rows_per_table) {

    const DatabaseCatalogSnapshot snapshot = load_database_catalog_snapshot(dbc_path);
    if (!snapshot.ok) {
        return {.ok = false, .error = snapshot.error, .sql = {}};
    }

    std::ostringstream sql;
    sql.imbue(std::locale::classic());
    // Unlike export_database_as_access_sql()'s Jet/ACE dialect, T-SQL
    // does support "-- ..." line comments (directly confirmed against a
    // real local SQL Server 2022 engine alongside this exporter's whole
    // dialect), so this script carries the same header/provenance and
    // skipped-table comment lines export_database_as_sql()/
    // export_database_as_postgresql_sql()/export_database_as_sqlite_sql()
    // already do.
    sql << "-- Copperfin EXPORT DATABASE ... TYPE SQLSERVER\n";
    sql << "-- database: " << sql_sanitize_comment_text(snapshot.db_name) << "\n";
    sql << "-- source: " << sql_sanitize_comment_text(dbc_path) << "\n\n";

    const std::size_t row_limit = (max_rows_per_table == 0U)
        ? std::numeric_limits<std::size_t>::max()
        : max_rows_per_table;

    std::string hard_failure_error;
    const std::vector<ParsedSqlExportTable> parsed_tables =
        write_sqlserver_tables_and_data(sql, snapshot, row_limit, hard_failure_error);
    // #5697: a member table with zero fields fails the whole export
    // closed. #5698: a non-blank numeric value that cannot be safely
    // represented shares this same hard_failure_error path. See
    // write_sqlserver_tables_and_data()'s own comment for both.
    if (!hard_failure_error.empty()) {
        return {.ok = false, .error = hard_failure_error, .sql = {}};
    }

    // Unlike export_database_as_postgresql_sql()/export_database_as_sqlite_sql(),
    // no whole-export used_index_names set is threaded through here --
    // write_sqlserver_create_indexes() keeps its own fresh per-table set
    // internally, matching SQL Server's real per-table (not schema-wide)
    // index-name scoping. See that function's own comment for the
    // real-engine verification this is grounded in.
    for (const auto& parsed : parsed_tables) {
        write_sqlserver_create_indexes(sql, parsed.resolved, parsed.table.fields);
    }

    return {.ok = true, .error = {}, .sql = sql.str()};
}

DatabaseSqlExportResult export_database_as_oracle_sql(
    const std::string& dbc_path,
    std::size_t max_rows_per_table) {

    const DatabaseCatalogSnapshot snapshot = load_database_catalog_snapshot(dbc_path);
    if (!snapshot.ok) {
        return {.ok = false, .error = snapshot.error, .sql = {}};
    }

    std::ostringstream sql;
    sql.imbue(std::locale::classic());
    // Oracle's own SQL dialect does support "-- ..." line comments
    // (directly confirmed against a real local Oracle 23ai engine
    // alongside this exporter's whole dialect), so this script carries
    // the same header/provenance and skipped-table comment lines
    // export_database_as_sql()/export_database_as_postgresql_sql()/
    // export_database_as_sqlite_sql()/export_database_as_sqlserver_sql()
    // already do.
    sql << "-- Copperfin EXPORT DATABASE ... TYPE ORACLE\n";
    sql << "-- database: " << sql_sanitize_comment_text(snapshot.db_name) << "\n";
    sql << "-- source: " << sql_sanitize_comment_text(dbc_path) << "\n\n";

    const std::size_t row_limit = (max_rows_per_table == 0U)
        ? std::numeric_limits<std::size_t>::max()
        : max_rows_per_table;

    std::string hard_failure_error;
    const std::vector<ParsedSqlExportTable> parsed_tables =
        write_oracle_tables_and_data(sql, snapshot, row_limit, hard_failure_error);
    // #5564 PR review (chatgpt-codex-connector, P2): a table/column name
    // colliding with another after oracle_quote_identifier()'s own
    // quote-stripping sanitization fails the whole export closed --
    // see write_oracle_tables_and_data()'s own comment for why silently
    // reusing the colliding identifier is not a safe alternative.
    if (!hard_failure_error.empty()) {
        return {.ok = false, .error = hard_failure_error, .sql = {}};
    }

    // Unlike SQL Server (a fresh per-table set, its own real per-table
    // index scoping) but like PostgreSQL/SQLite (schema-wide scoping,
    // #5559/#5558), Oracle index names are unique across the whole
    // export -- but, unlike PostgreSQL specifically, this set is *not*
    // seeded with already-emitted table names, since Oracle keeps
    // tables and indexes in separate namespaces. See
    // write_oracle_create_indexes()'s own comment for the real-engine
    // verification this is grounded in.
    std::set<std::string> used_index_names;
    for (const auto& parsed : parsed_tables) {
        write_oracle_create_indexes(sql, parsed.resolved, parsed.table.fields, used_index_names);
    }

    return {.ok = true, .error = {}, .sql = sql.str()};
}

// #5554: MySQL identifier quoting for export_database_as_mysql_sql() --
// backticks, per MySQL's own public "Schema Object Names" reference. An
// embedded backtick is escaped by doubling it, like every other dialect
// this file emits except Oracle -- directly confirmed against a real
// local MySQL 8.0 engine: `CREATE TABLE `` weird``name`` `` round-tripped
// correctly through CREATE TABLE and back out of information_schema.
std::string mysql_quote_identifier(const std::string& name) {
    std::string quoted = "`";
    for (const char character : name) {
        if (character == '`') {
            quoted += "``";
        } else {
            quoted += character;
        }
    }
    quoted += "`";
    return quoted;
}

// #5554 (fifth and final vendor-dialect slice of #141's real-target-engine
// `EXPORT DATABASE` family): unlike every other dialect this file emits --
// all of which reuse the shared sql_quote_string_literal()'s plain
// doubled-single-quote escaping -- MySQL cannot, because backslash is a
// live escape character *inside* a string literal under MySQL's own
// default `sql_mode` (specifically, whenever `NO_BACKSLASH_ESCAPES` is
// *not* set, which is the out-of-the-box default this exporter targets,
// matching how this file already targets each other engine's own default
// configuration rather than a non-default hardening mode). Directly
// confirmed against a real local MySQL 8.0 engine: inserting the literal
// `'C:\temp''s file'` (correct ANSI-style doubled-quote escaping, the
// same convention sql_quote_string_literal() already applies) stores only
// 13 characters, not 14 -- the `\t` was silently interpreted as a TAB
// character, not a literal backslash followed by `t`, corrupting a
// genuinely common case for this codebase (a Windows path inside a VFP
// character or memo field). Doubling the backslash too (`'C:\\temp''s
// file'`) stores the correct, literal 14-character string. So this
// function escapes *both* a single quote and a backslash by doubling,
// unlike every sibling dialect's shared helper.
std::string mysql_quote_string_literal(const std::string& value) {
    std::string quoted = "'";
    for (const char character : value) {
        if (character == '\'' || character == '\\') {
            quoted += character;
        }
        quoted += character;
    }
    quoted += "'";
    return quoted;
}

// Maps a DBF field descriptor to a native MySQL column type, grounded in
// MySQL's own public "Data Types" reference (not the ANSI-ish vocabulary
// sql_column_type() emits, nor any other dialect's own vocabulary this
// file already maps to). 'Y' (VFP currency) maps to DECIMAL(19, 4), an
// exact match for VFP currency's own fixed 4-decimal-digit scaled-integer
// semantics (the same reasoning every other dialect's own currency choice
// in this file applies). 'I' maps to INT, MySQL's own 4-byte integer,
// matching VFP's own 4-byte 'I' field width. 'B' maps to DOUBLE, MySQL's
// true IEEE 754 double-precision type -- an exact width match, unlike
// DECIMAL's arbitrary-precision semantics. 'L' maps to TINYINT(1) with
// 1/0 literals: MySQL's own BOOLEAN keyword is documented as merely a
// synonym for TINYINT(1) (directly confirmed against a real local MySQL
// 8.0 engine: `SHOW CREATE TABLE` on a `BOOLEAN` column reports
// `tinyint(1)` verbatim), so this exporter declares the underlying type
// directly rather than the alias, matching this file's own convention of
// targeting each engine's own real physical type. 'D' maps to DATE and
// 'T' to DATETIME -- unlike Oracle, a plain ISO `'YYYY-MM-DD'`/
// `'YYYY-MM-DD HH:MM:SS'` string loads directly with no special literal
// wrapper needed (directly confirmed against the real engine). The
// memo/general/picture and any-other-unrecognized fallback maps to
// LONGTEXT (up to ~4 GiB) rather than the 64 KiB-capped TEXT, mirroring
// why export_database_as_sqlserver_sql() picks VARCHAR(MAX) over the
// legacy TEXT type -- a real VFP memo is not bound by TEXT's own ceiling.
std::string mysql_column_type(char field_type, std::uint8_t length, std::uint8_t decimal_count) {
    const char normalized = static_cast<char>(std::toupper(static_cast<unsigned char>(field_type)));
    switch (normalized) {
        case 'N':
        case 'F': {
            // #5554: a crafted or corrupt DBF header does not have to keep
            // length/decimal_count within MySQL's own valid ranges the way
            // a table genuinely written by this codebase's own writer
            // always does. Directly confirmed against a real local MySQL
            // 8.0 engine: DECIMAL(65, 0) succeeds while DECIMAL(66, 0)
            // fails ("Too-big precision 66 specified... Maximum is 65."),
            // DECIMAL(65, 30) succeeds while DECIMAL(10, 31) fails
            // ("Too big scale 31... Maximum is 30."), and -- like SQL
            // Server's own DECIMAL, a genuine difference from Oracle's own
            // independent scale range -- scale must not exceed precision
            // (DECIMAL(10, 20) fails: "M must be >= D"). sqlserver_column_type()
            // already applies the identical clamp-scale-to-precision
            // pattern for SQL Server's own 38-digit ceiling.
            constexpr std::uint8_t max_mysql_decimal_precision = 65U;
            constexpr std::uint8_t max_mysql_decimal_scale = 30U;
            const std::uint8_t precision = std::min(
                std::max(length, static_cast<std::uint8_t>(1U)),
                max_mysql_decimal_precision);
            const std::uint8_t scale = std::min(
                {decimal_count, max_mysql_decimal_scale, precision});
            return "DECIMAL(" + std::to_string(precision) + ", " + std::to_string(scale) + ")";
        }
        case 'Y':
            return "DECIMAL(19, 4)";
        case 'I':
            return "INT";
        case 'B':
            return "DOUBLE";
        case 'L':
            return "TINYINT(1)";
        case 'D':
            return "DATE";
        case 'T':
            return "DATETIME";
        case 'C':
        case 'V':
            return "VARCHAR(" + std::to_string(length > 0U ? length : 255U) + ")";
        default:
            // M, G, P, and any other/unrecognized storage type.
            return "LONGTEXT";
    }
}

// #5554: MySQL's dialect is close enough to the SQL Server precedent
// (export_database_as_sqlserver_sql()) to share its overall shape --
// mysql_quote_identifier()/mysql_column_type() for identifiers/types, and
// the same 1/0 boolean-literal, blank-date-to-NULL, and plain-ISO-string
// date/datetime handling -- but keeps its own dedicated function per this
// file's established per-vendor precedent, and (the one genuinely
// MySQL-specific value-encoding difference) uses mysql_quote_string_literal()
// rather than the shared sql_quote_string_literal() for every character-ish
// value, since MySQL's own default sql_mode treats backslash as a live
// escape character inside a string literal (see that function's own
// comment for the real-engine-confirmed data-corruption this avoids).
// Unlike Oracle, no identifier-collision tracking is needed here: an
// embedded backtick is losslessly escaped by doubling (like SQL Server's
// own `]]`), not stripped, so two distinct source names can never
// sanitize to the same quoted identifier. Directly confirmed against a
// real local MySQL 8.0 engine that a blank VFP date/datetime field's own
// empty display_value must resolve to NULL, not an empty string literal:
// under MySQL 8.0's own default `sql_mode` (which includes
// `STRICT_TRANS_TABLES`), `INSERT INTO ... VALUES ('')` into a DATE or
// DATETIME column fails outright with error 1292 ("Incorrect date value:
// ''"/"Incorrect datetime value: ''") rather than SQL Server's own
// silent-1900-01-01 data corruption or Oracle's own invalid-syntax
// rejection -- a third, distinct failure mode from the two dialects this
// file already handles, but the same NULL-instead-of-empty-string fix
// applies. And, unlike Oracle's own CLOB, a plain (correctly escaped)
// string literal is safe for a MySQL LONGTEXT value with no special
// chunking: directly confirmed a 10,000-byte literal loads correctly in
// one statement (MySQL's own `max_allowed_packet` -- 64 MiB by default in
// 8.0 -- is the only real ceiling, nothing like Oracle's tight 4000-byte
// SQL text-literal limit), and an empty string literal correctly stores
// as an empty (non-NULL) string, not Oracle's own silently-NULLed CLOB.
// Returns every successfully-parsed table the same way
// write_sqlserver_tables_and_data() does, so write_mysql_create_indexes()
// doesn't have to re-open and re-parse the same .dbf a second time.
//
// #5582 PR review (chatgpt-codex-connector, P2): a DBC catalog's own
// OBJECTNAME column (a DBF field this codebase lets a caller size
// arbitrarily wide, e.g. the fixture in this file's own regression test)
// permits a table name longer than MySQL's real 64-character identifier
// limit -- this codebase's own writer never produces one that long, but a
// crafted/corrupt or foreign-tool-written DBC is not bound by that.
// Without this check, such a table name would be quoted and emitted
// unchanged, letting this function return a "successful" export whose
// very first `CREATE TABLE` a real MySQL engine rejects outright with
// error 1059 -- the same class of gap `write_mysql_create_indexes()`
// already guards against for its own generated index names via
// `disambiguate_index_name()`, just never applied to a table's own
// pre-existing name. The identical check on each column name is
// currently unreachable through this codebase's own classic-DBF field
// descriptor (a hard-structural 10-byte name slot, `dbf_descriptor_name_width`
// in dbf_table.cpp, well under 64 characters), but is included anyway as
// defense-in-depth against a future long-field-name storage mechanism or
// a hand-crafted descriptor bypassing that structural width. Failing the
// whole export closed with a diagnostic naming the identifier (rather
// than silently truncating, which risks a same-table column-name
// collision with no established disambiguation story for that case)
// matches this codebase's own established fail-closed precedent for a
// case with no safe corrective action (see
// `oracle_record_identifier_or_detect_collision()`'s own comment).
//
// MySQL's own identifier limit (64 *characters*, not bytes -- directly
// confirmed against a real local MySQL 8.0 engine that a 64-character
// identifier succeeds while a 65-character one fails outright with error
// 1059, "Identifier name ... is too long", and that this holds true by
// character count rather than byte count: 64 two-byte UTF-8 characters
// (128 bytes) succeeds identically to 64 ASCII characters, while 65 of
// either fails the same way) -- see utf8_safe_truncate()'s own comment
// for why this uses IdentifierLengthUnit::unicode_code_points, matching
// SQL Server's own character-counted `sysname` rather than PostgreSQL's/
// Oracle's own byte-counted limits. Index names share this identical
// 64-character ceiling (directly confirmed the same way).
constexpr std::size_t kMysqlMaxIdentifierCodePoints = 64U;

std::vector<ParsedSqlExportTable> write_mysql_tables_and_data(
    std::ostringstream& sql,
    const DatabaseCatalogSnapshot& snapshot,
    std::size_t row_limit,
    std::string& hard_failure_error) {
    std::vector<ParsedSqlExportTable> parsed_tables;
    for (const auto& rt : snapshot.resolved_tables) {
        DbfTableParseResult tbl = parse_dbf_table_from_file(
            copperfin::platform::path_to_utf8_string(rt.path), row_limit);
        if (!tbl.ok) {
            sql << "-- skipped table " << sql_sanitize_comment_text(rt.name) << ": "
                << sql_sanitize_comment_text(tbl.error) << "\n\n";
            continue;
        }
        if (tbl.table.fields.empty()) {
            // #5697: see write_sql_tables_and_data()'s own comment -- a
            // zero-field member table would otherwise emit invalid
            // `CREATE TABLE \`name\` (\n);` DDL.
            hard_failure_error = asset_inspector_text(
                "Vfp.AssetInspector.Validation.ExportTableHasNoFields",
                {{"table", rt.name}});
            return {};
        }

        const std::string quoted_table = mysql_quote_identifier(rt.name);
        if (identifier_length_in_unit(rt.name, IdentifierLengthUnit::unicode_code_points) >
            kMysqlMaxIdentifierCodePoints) {
            hard_failure_error = asset_inspector_text(
                "Vfp.AssetInspector.Validation.MysqlIdentifierTooLong",
                {{"identifier", quoted_table}});
            return {};
        }
        sql << "CREATE TABLE " << quoted_table << " (\n";
        for (std::size_t fi = 0U; fi < tbl.table.fields.size(); ++fi) {
            const auto& fld = tbl.table.fields[fi];
            const std::string quoted_column = mysql_quote_identifier(fld.name);
            if (identifier_length_in_unit(fld.name, IdentifierLengthUnit::unicode_code_points) >
                kMysqlMaxIdentifierCodePoints) {
                hard_failure_error = asset_inspector_text(
                    "Vfp.AssetInspector.Validation.MysqlIdentifierTooLong",
                    {{"identifier", quoted_column}});
                return {};
            }
            const bool last_field = (fi + 1U == tbl.table.fields.size());
            sql << "    " << quoted_column << " "
                << mysql_column_type(fld.type, fld.length, fld.decimal_count)
                << (last_field ? "\n" : ",\n");
        }
        sql << ");\n\n";

        std::size_t row_number = 0U;
        for (const auto& rec : tbl.table.records) {
            if (rec.deleted) {
                continue;
            }
            ++row_number;
            sql << "INSERT INTO " << quoted_table << " (";
            for (std::size_t vi = 0U; vi < rec.values.size(); ++vi) {
                sql << mysql_quote_identifier(rec.values[vi].field_name)
                    << (vi + 1U == rec.values.size() ? "" : ", ");
            }
            sql << ") VALUES (";
            for (std::size_t vi = 0U; vi < rec.values.size(); ++vi) {
                const auto& rv = rec.values[vi];
                const char ft = static_cast<char>(
                    std::toupper(static_cast<unsigned char>(rv.field_type)));
                const bool is_numeric = (ft == 'N' || ft == 'F' || ft == 'I' || ft == 'B' || ft == 'Y');
                const bool is_logical = (ft == 'L');
                const bool is_date = (ft == 'D');
                const bool is_datetime = (ft == 'T');
                if (rv.is_null) {
                    sql << "NULL";
                } else if (is_logical) {
                    const std::string& lv = rv.display_value;
                    if (lv == "true") {
                        sql << "1";
                    } else if (lv == "false") {
                        sql << "0";
                    } else {
                        sql << "NULL";
                    }
                } else if (is_numeric) {
                    // #5698: see write_sql_tables_and_data()'s own comment
                    // -- a non-blank value that isn't a safe plain decimal
                    // literal fails the whole export closed instead of
                    // silently becoming NULL.
                    if (rv.display_value.empty()) {
                        sql << "NULL";
                    } else if (looks_like_safe_unquoted_sql_numeric_literal(rv.display_value)) {
                        sql << rv.display_value;
                    } else {
                        hard_failure_error = asset_inspector_text(
                            "Vfp.AssetInspector.Validation.UnsafeNumericValue",
                            {{"table", rt.name}, {"row", std::to_string(row_number)},
                             {"column", rv.field_name}});
                        return {};
                    }
                } else if (is_date) {
                    // A blank VFP date's own empty display_value must
                    // become NULL, not an empty string literal -- see
                    // this function's own comment above for the real
                    // engine's own strict-mode rejection (error 1292).
                    sql << (rv.display_value.empty() ? "NULL" : mysql_quote_string_literal(rv.display_value));
                } else if (is_datetime) {
                    const auto converted = sql_datetime_literal_from_storage(rv.display_value);
                    sql << (converted.has_value() ? mysql_quote_string_literal(*converted) : "NULL");
                } else {
                    sql << mysql_quote_string_literal(rv.display_value);
                }
                sql << (vi + 1U == rec.values.size() ? "" : ", ");
            }
            sql << ");\n";
        }
        sql << "\n";
        parsed_tables.push_back({.resolved = rt, .table = std::move(tbl.table)});
    }
    return parsed_tables;
}

// #5554: MySQL's own CREATE INDEX syntax is identical to PostgreSQL's for
// this exporter's plain-column-reference case (see
// write_postgresql_create_indexes()'s own comment for the full scope and
// documented non-goals -- composite/expression keys skipped as a
// comment, no per-tag uniqueness captured). Kept as its own dedicated
// function rather than a shared/renamed one, matching this file's own
// established per-vendor-dialect precedent.
//
// Like SQL Server (#5561) and *unlike* PostgreSQL/SQLite/Oracle
// (#5559/#5558/#5564), MySQL index names are *not* schema-wide -- directly
// confirmed against a real local MySQL 8.0 engine: two different tables
// can each carry an index of the identical name with no error at all, and
// a table can be named identically to an unrelated table's own index
// without collision either (an index's own namespace is scoped to the
// table it belongs to). `used_index_names` is therefore a fresh, per-table
// set (not threaded in from the caller across the whole export), exactly
// mirroring write_sqlserver_create_indexes()'s own scoping and its own
// stated reasoning for why a same-table collision can still arise purely
// from truncation once a table name alone is at or beyond the 64-character
// limit.
void write_mysql_create_indexes(
    std::ostringstream& sql,
    const DatabaseCatalogSnapshot::ResolvedTable& rt,
    const std::vector<DbfFieldDescriptor>& fields) {
    const SidecarPathResolution cdx_resolution = resolve_vfp_sidecar_path(rt.path, ".cdx");
    if (!cdx_resolution.path.has_value()) {
        return;
    }
    const std::string quoted_table = mysql_quote_identifier(rt.name);
    const IndexParseResult index_result = parse_index_probe_from_file(
        copperfin::platform::path_to_utf8_string(*cdx_resolution.path));
    if (!index_result.ok || index_result.probe.kind != IndexKind::cdx) {
        sql << "-- skipped indexes on " << sql_sanitize_comment_text(rt.name)
            << ": companion .cdx exists but could not be read as a compound index\n\n";
        return;
    }

    std::set<std::string> used_index_names;
    bool wrote_anything = false;
    for (std::size_t tag_index = 0U; tag_index < index_result.probe.tags.size(); ++tag_index) {
        const IndexTagProbe& tag = index_result.probe.tags[tag_index];
        const auto column = plain_column_name_for_index_tag(tag.key_expression_hint, fields);
        const std::string tag_label = tag.name_hint.empty() ? std::string("(unnamed tag)") : tag.name_hint;
        if (!column.has_value()) {
            sql << "-- skipped index " << sql_sanitize_comment_text(tag_label)
                << " on " << sql_sanitize_comment_text(rt.name)
                << ": key expression is not a plain column reference\n";
            wrote_anything = true;
            continue;
        }
        const std::string tag_identity =
            tag.name_hint.empty() ? ("tag" + std::to_string(tag_index)) : tag.name_hint;
        const std::string index_name = disambiguate_index_name(
            rt.name + "_" + tag_identity + "_idx", used_index_names,
            kMysqlMaxIdentifierCodePoints, IdentifierLengthUnit::unicode_code_points);
        sql << "CREATE INDEX " << mysql_quote_identifier(index_name)
            << " ON " << quoted_table << " (" << mysql_quote_identifier(*column) << ");\n";
        wrote_anything = true;
    }
    if (wrote_anything) {
        sql << "\n";
    }
}

DatabaseSqlExportResult export_database_as_mysql_sql(
    const std::string& dbc_path,
    std::size_t max_rows_per_table) {

    const DatabaseCatalogSnapshot snapshot = load_database_catalog_snapshot(dbc_path);
    if (!snapshot.ok) {
        return {.ok = false, .error = snapshot.error, .sql = {}};
    }

    std::ostringstream sql;
    sql.imbue(std::locale::classic());
    // MySQL's own SQL dialect does support "-- ..." line comments
    // (directly confirmed against a real local MySQL 8.0 engine alongside
    // this exporter's whole dialect), so this script carries the same
    // header/provenance and skipped-table comment lines this file's other
    // TYPE variants already do.
    sql << "-- Copperfin EXPORT DATABASE ... TYPE MYSQL\n";
    sql << "-- database: " << sql_sanitize_comment_text(snapshot.db_name) << "\n";
    sql << "-- source: " << sql_sanitize_comment_text(dbc_path) << "\n\n";

    const std::size_t row_limit = (max_rows_per_table == 0U)
        ? std::numeric_limits<std::size_t>::max()
        : max_rows_per_table;

    std::string hard_failure_error;
    const std::vector<ParsedSqlExportTable> parsed_tables =
        write_mysql_tables_and_data(sql, snapshot, row_limit, hard_failure_error);
    // #5582 PR review (chatgpt-codex-connector, P2): a table or column
    // name longer than MySQL's real 64-character identifier limit fails
    // the whole export closed -- see write_mysql_tables_and_data()'s own
    // comment for why silently emitting it (or truncating it, with no
    // established collision-safe disambiguation story for a table's own
    // pre-existing column names) is not a safe alternative. #5697: a
    // zero-field member table shares this same hard_failure_error path.
    // #5698: a non-blank numeric value that cannot be safely represented
    // shares it too.
    if (!hard_failure_error.empty()) {
        return {.ok = false, .error = hard_failure_error, .sql = {}};
    }

    // Unlike export_database_as_postgresql_sql()/export_database_as_sqlite_sql()/
    // export_database_as_oracle_sql(), no whole-export used_index_names set
    // is threaded through here -- write_mysql_create_indexes() keeps its own
    // fresh per-table set internally, matching MySQL's real per-table (not
    // schema-wide) index-name scoping, the same as SQL Server's own #5561
    // precedent.
    for (const auto& parsed : parsed_tables) {
        write_mysql_create_indexes(sql, parsed.resolved, parsed.table.fields);
    }

    return {.ok = true, .error = {}, .sql = sql.str()};
}

DatabaseJsonImportPlanResult build_database_json_import_plan(const std::string_view document) {
    using copperfin::platform::JsonSelectionError;
    using copperfin::platform::JsonValueKind;
    using copperfin::platform::parse_json_document;

    constexpr std::size_t maximum_fields_per_table = 255U;
    const auto failure = [](std::string code) {
        return DatabaseJsonImportPlanResult{
            .ok = false,
            .error_code = std::move(code),
            .plan = {}};
    };
    const auto parsed_document = parse_json_document(document);
    if (!parsed_document.ok()) {
        return failure("database_json_import.invalid_document");
    }
    const auto required_value = [&](const std::string_view pointer) {
        return parsed_document.document.select(pointer);
    };

    const auto root = required_value({});
    if (!root.ok() || root.kind != JsonValueKind::object) {
        return failure("database_json_import.invalid_document");
    }
    const auto root_members = parsed_document.document.object_member_names();
    if (!root_members.ok()) {
        return failure("database_json_import.invalid_document");
    }
    const std::set<std::string> required_root_members{
        "schema_version", "database", "catalog", "tables"};
    if (root_members.names.size() != required_root_members.size() ||
        !std::all_of(root_members.names.begin(), root_members.names.end(),
            [&](const std::string& name) { return required_root_members.contains(name); })) {
        return failure("database_json_import.invalid_document");
    }

    const auto schema_version = required_value("/schema_version");
    if (!schema_version.ok() || schema_version.kind != JsonValueKind::number ||
        schema_version.raw_json != "1") {
        return failure("database_json_import.unsupported_schema_version");
    }

    const auto database = required_value("/database");
    const auto database_name = required_value("/database/name");
    const auto database_path = required_value("/database/path");
    if (!database.ok() || database.kind != JsonValueKind::object ||
        !database_name.ok() || database_name.kind != JsonValueKind::string ||
        database_name.decoded_string.empty() ||
        !database_path.ok() || database_path.kind != JsonValueKind::string) {
        return failure("database_json_import.invalid_database");
    }

    const auto catalog = required_value("/catalog");
    if (!catalog.ok() || catalog.kind != JsonValueKind::array) {
        return failure("database_json_import.invalid_catalog");
    }
    const auto tables = required_value("/tables");
    if (!tables.ok() || tables.kind != JsonValueKind::object) {
        return failure("database_json_import.invalid_tables");
    }
    const auto table_members = parsed_document.document.object_member_names("/tables");
    if (!table_members.ok()) {
        return failure("database_json_import.invalid_tables");
    }

    DatabaseJsonImportPlan plan;
    plan.database_name = database_name.decoded_string;
    plan.catalog_json = catalog.raw_json;
    std::vector<std::string> table_names = table_members.names;
    std::sort(table_names.begin(), table_names.end());
    std::set<std::string> casefolded_table_names;
    for (const std::string& table_name : table_names) {
        if (table_name.empty() || !casefolded_table_names.insert(lowercase_copy(table_name)).second) {
            return failure("database_json_import.duplicate_table_name");
        }
    }
    for (const std::string& table_name : table_names) {
        const std::string table_pointer = "/tables/" + json_pointer_token(table_name);
        const auto table = required_value(table_pointer);
        const auto fields = required_value(table_pointer + "/fields");
        const auto records = required_value(table_pointer + "/records");
        if (!table.ok() || table.kind != JsonValueKind::object ||
            !fields.ok() || fields.kind != JsonValueKind::array ||
            !records.ok() || records.kind != JsonValueKind::array) {
            return failure("database_json_import.invalid_table");
        }

        DatabaseJsonImportTablePlan table_plan;
        table_plan.name = table_name;
        table_plan.records_json = records.raw_json;
        std::set<std::string> casefolded_field_names;
        for (std::size_t index = 0U; index <= maximum_fields_per_table; ++index) {
            const std::string field_pointer = table_pointer + "/fields/" + std::to_string(index);
            const auto field = required_value(field_pointer);
            if (field.error == JsonSelectionError::value_not_found) {
                break;
            }
            if (!field.ok() || field.kind != JsonValueKind::object || index == maximum_fields_per_table) {
                return failure("database_json_import.invalid_field");
            }
            const auto field_name = required_value(field_pointer + "/name");
            const auto field_type = required_value(field_pointer + "/type");
            const auto field_length = required_value(field_pointer + "/length");
            const auto field_decimals = required_value(field_pointer + "/decimals");
            std::size_t length = 0U;
            std::size_t decimals = 0U;
            if (!field_name.ok() || field_name.kind != JsonValueKind::string ||
                field_name.decoded_string.empty() ||
                !casefolded_field_names.insert(lowercase_copy(field_name.decoded_string)).second ||
                !field_type.ok() || field_type.kind != JsonValueKind::string ||
                !is_single_printable_ascii_character(field_type.decoded_string) ||
                !field_length.ok() || field_length.kind != JsonValueKind::number ||
                !parse_decimal_in_range(field_length.raw_json, 1U, 255U, length) ||
                !field_decimals.ok() || field_decimals.kind != JsonValueKind::number ||
                !parse_decimal_in_range(field_decimals.raw_json, 0U, length, decimals)) {
                return failure("database_json_import.invalid_field");
            }
            const DbfFieldDescriptor descriptor{
                .name = field_name.decoded_string,
                .type = field_type.decoded_string.front(),
                .offset = 0U,
                .length = static_cast<std::uint8_t>(length),
                .decimal_count = static_cast<std::uint8_t>(decimals)};
            if (!is_dbf_table_field_storage_layout_writable(descriptor.type, descriptor.length)) {
                return failure("database_json_import.invalid_field");
            }
            table_plan.fields.push_back(descriptor);
        }
        // #5697: an empty "fields" array parses this loop's own
        // value_not_found break immediately, leaving table_plan with zero
        // fields -- deliberately NOT rejected here. This is the exact
        // shape export_database_as_json() itself emits for a cataloged
        // table whose underlying .dbf could not be parsed at all (its own
        // documented "-- skipped table" / empty-marker precedent, proven
        // by test_database_json_import_plan_admits_exporter_unreadable_
        // table_marker), and the planner has no way to distinguish that
        // case from a genuinely field-less table using the JSON alone.
        // Since export_database_as_json() itself now fails the whole
        // export closed for a genuinely zero-field *parseable* table
        // (#5697's own real gap, fixed at the export boundary instead),
        // this exact marker shape can only mean "source table was
        // unreadable" in any JSON this codebase's own exporter produces.
        // materialize_database_json_import_plan() still fails the import
        // closed if such a table plan is materialized directly
        // (create_dbf_table_file()'s own required-field invariant, with
        // no partial writes thanks to its staging/abort-staging design),
        // so rejecting it here too would only duplicate that safety net
        // while breaking the documented unreadable-table round-trip.
        plan.tables.push_back(std::move(table_plan));
    }

    return {.ok = true, .error_code = {}, .plan = std::move(plan)};
}

namespace {

// ---- build_database_sql_import_plan() tokenizer/parser ----
//
// A hand-rolled tokenizer/parser for the exact, narrow SQL dialect
// export_database_as_sql() itself emits -- not a general-purpose SQL
// parser. Every token shape below mirrors that function's own emission
// exactly: sql_quote_identifier()'s doubled-double-quote escaping,
// sql_quote_string_literal()'s doubled-single-quote escaping, and
// sql_column_type()'s fixed vocabulary. Anything else yields an `invalid`
// token or a parse failure with a distinct error_code, rather than a guess.

enum class SqlTokenKind { identifier, quoted_identifier, string_literal, number, punct, end_of_input, invalid };

struct SqlToken {
    SqlTokenKind kind = SqlTokenKind::end_of_input;
    std::string text;
};

class SqlImportTokenizer {
public:
    explicit SqlImportTokenizer(std::string_view text) : text_(text) {}

    SqlToken next() {
        skip_ignorable();
        if (position_ >= text_.size()) {
            return {.kind = SqlTokenKind::end_of_input, .text = {}};
        }
        const char ch = text_[position_];
        if (ch == '"') {
            return read_quoted('"', SqlTokenKind::quoted_identifier);
        }
        if (ch == '\'') {
            return read_quoted('\'', SqlTokenKind::string_literal);
        }
        if (ch == '(' || ch == ')' || ch == ',' || ch == ';') {
            ++position_;
            return {.kind = SqlTokenKind::punct, .text = std::string(1U, ch)};
        }
        if (ch == '-' || (ch >= '0' && ch <= '9')) {
            return read_number();
        }
        if (std::isalpha(static_cast<unsigned char>(ch)) != 0 || ch == '_') {
            return read_bare_word();
        }
        ++position_;
        return {.kind = SqlTokenKind::invalid, .text = std::string(1U, ch)};
    }

private:
    void skip_ignorable() {
        for (;;) {
            while (position_ < text_.size() &&
                   std::isspace(static_cast<unsigned char>(text_[position_])) != 0) {
                ++position_;
            }
            if (position_ + 1U < text_.size() && text_[position_] == '-' && text_[position_ + 1U] == '-') {
                while (position_ < text_.size() && text_[position_] != '\n') {
                    ++position_;
                }
                continue;
            }
            break;
        }
    }

    SqlToken read_quoted(char quote, SqlTokenKind kind) {
        ++position_;
        std::string decoded;
        for (;;) {
            if (position_ >= text_.size()) {
                return {.kind = SqlTokenKind::invalid, .text = decoded};
            }
            const char ch = text_[position_];
            if (ch == quote) {
                if (position_ + 1U < text_.size() && text_[position_ + 1U] == quote) {
                    decoded.push_back(quote);
                    position_ += 2U;
                    continue;
                }
                ++position_;
                return {.kind = kind, .text = decoded};
            }
            decoded.push_back(ch);
            ++position_;
        }
    }

    // JSON-number-compatible: requires at least one digit before a decimal
    // point (rejecting a bare ".5") and at least one after it if present
    // (rejecting a bare "1."), and accepts the scientific-notation exponent
    // suffix ostringstream's default double formatting can emit for very
    // large or small DOUBLE PRECISION values (e.g. "1e+20") -- a token this
    // parser must accept even though this dialect's grammar has no other
    // use for 'e'/'E', since export_database_as_sql() can produce it.
    SqlToken read_number() {
        const std::size_t start = position_;
        if (position_ < text_.size() && text_[position_] == '-') {
            ++position_;
        }
        const std::size_t integer_start = position_;
        while (position_ < text_.size() && text_[position_] >= '0' && text_[position_] <= '9') {
            ++position_;
        }
        if (position_ == integer_start) {
            return invalid_number(start);
        }
        if (position_ < text_.size() && text_[position_] == '.') {
            const std::size_t dot_position = position_;
            ++position_;
            const std::size_t fraction_start = position_;
            while (position_ < text_.size() && text_[position_] >= '0' && text_[position_] <= '9') {
                ++position_;
            }
            if (position_ == fraction_start) {
                position_ = dot_position;
                return invalid_number(start);
            }
        }
        if (position_ < text_.size() && (text_[position_] == 'e' || text_[position_] == 'E')) {
            const std::size_t exponent_marker = position_;
            ++position_;
            if (position_ < text_.size() && (text_[position_] == '+' || text_[position_] == '-')) {
                ++position_;
            }
            const std::size_t exponent_digits_start = position_;
            while (position_ < text_.size() && text_[position_] >= '0' && text_[position_] <= '9') {
                ++position_;
            }
            if (position_ == exponent_digits_start) {
                position_ = exponent_marker;
                return invalid_number(start);
            }
        }
        return {.kind = SqlTokenKind::number, .text = std::string(text_.substr(start, position_ - start))};
    }

    SqlToken invalid_number(std::size_t start) {
        return {.kind = SqlTokenKind::invalid, .text = std::string(text_.substr(start, position_ - start))};
    }

    SqlToken read_bare_word() {
        const std::size_t start = position_;
        while (position_ < text_.size() &&
               (std::isalnum(static_cast<unsigned char>(text_[position_])) != 0 || text_[position_] == '_')) {
            ++position_;
        }
        return {.kind = SqlTokenKind::identifier, .text = std::string(text_.substr(start, position_ - start))};
    }

    std::string_view text_;
    std::size_t position_ = 0U;
};

bool sql_token_is_keyword(const SqlToken& token, const std::string_view keyword) {
    if (token.kind != SqlTokenKind::identifier || token.text.size() != keyword.size()) {
        return false;
    }
    for (std::size_t index = 0U; index < keyword.size(); ++index) {
        if (std::tolower(static_cast<unsigned char>(token.text[index])) !=
            std::tolower(static_cast<unsigned char>(keyword[index]))) {
            return false;
        }
    }
    return true;
}

bool sql_token_is_punct(const SqlToken& token, char punct) {
    return token.kind == SqlTokenKind::punct && token.text.size() == 1U && token.text.front() == punct;
}

struct SqlImportHeader {
    bool ok = false;
    std::string database_name;
    std::size_t body_offset = 0U;
};

// The exact three-line header export_database_as_sql() always emits, used
// as the up-front narrow-subset gate: anything else is rejected before any
// token is even read, rather than attempting to parse it as SQL.
SqlImportHeader parse_sql_import_header(const std::string_view document) {
    SqlImportHeader header;
    constexpr std::string_view magic_line = "-- Copperfin EXPORT DATABASE ... TYPE SQL";
    constexpr std::string_view database_prefix = "-- database: ";
    constexpr std::string_view source_prefix = "-- source: ";
    std::size_t position = 0U;
    const auto read_line = [&]() {
        const std::size_t newline = document.find('\n', position);
        const std::string_view line = (newline == std::string_view::npos)
            ? document.substr(position)
            : document.substr(position, newline - position);
        position = (newline == std::string_view::npos) ? document.size() : newline + 1U;
        return (!line.empty() && line.back() == '\r') ? line.substr(0U, line.size() - 1U) : line;
    };
    if (read_line() != magic_line) {
        return header;
    }
    const std::string_view line2 = read_line();
    if (line2.size() <= database_prefix.size() || line2.substr(0U, database_prefix.size()) != database_prefix) {
        return header;
    }
    const std::string database_name(line2.substr(database_prefix.size()));
    if (database_name.empty()) {
        return header;
    }
    const std::string_view line3 = read_line();
    if (line3.size() < source_prefix.size() || line3.substr(0U, source_prefix.size()) != source_prefix) {
        return header;
    }
    header.ok = true;
    header.database_name = database_name;
    header.body_offset = position;
    return header;
}

// Inverse of sql_datetime_literal_from_storage(): parses the exact
// "YYYY-MM-DD HH:MM:SS" shape that function (and therefore
// export_database_as_sql()) always emits for a non-null T-type value, back
// into this codebase's "julian:<day> millis:<ms>" internal storage contract
// parse_datetime_storage_value() (src/vfp/dbf_table.cpp) requires --
// without this conversion, a table with any populated timestamp value
// could never be materialized. Returns std::nullopt for anything not
// exactly that shape, rather than guessing.
std::optional<std::string> sql_datetime_storage_from_literal(const std::string& literal) {
    if (literal.size() != 19U ||
        literal[4] != '-' || literal[7] != '-' || literal[10] != ' ' ||
        literal[13] != ':' || literal[16] != ':') {
        return std::nullopt;
    }
    const auto parse_digits = [&](std::size_t offset, std::size_t count) -> std::optional<int> {
        int value = 0;
        for (std::size_t index = 0U; index < count; ++index) {
            const char character = literal[offset + index];
            if (character < '0' || character > '9') {
                return std::nullopt;
            }
            value = value * 10 + (character - '0');
        }
        return value;
    };
    const auto year = parse_digits(0U, 4U);
    const auto month = parse_digits(5U, 2U);
    const auto day = parse_digits(8U, 2U);
    const auto hour = parse_digits(11U, 2U);
    const auto minute = parse_digits(14U, 2U);
    const auto second = parse_digits(17U, 2U);
    if (!year.has_value() || !month.has_value() || !day.has_value() ||
        !hour.has_value() || !minute.has_value() || !second.has_value() ||
        *month < 1 || *month > 12 || *day < 1 || *day > 31 ||
        *hour > 23 || *minute > 59 || *second > 59) {
        return std::nullopt;
    }
    // Mirrors sql_julian_day_to_date()'s own inverse (Fliegel-Van Flandern
    // astronomical Julian day, minus 702 to match this codebase's existing
    // epoch convention) rather than depending on cf_xbase_runtime's
    // date_to_julian(), for the same reason that function's own comment
    // gives: cf_xbase_runtime already depends on cf_vfp_assets, so the
    // reverse dependency isn't available.
    const int julian_day =
        ((1461 * (*year + 4800 + (*month - 14) / 12)) / 4 +
         (367 * (*month - 2 - 12 * ((*month - 14) / 12))) / 12 -
         (3 * ((*year + 4900 + (*month - 14) / 12) / 100)) / 4 +
         *day - 32075) - 702;
    const int millis = ((*hour * 3600) + (*minute * 60) + *second) * 1000;
    return "julian:" + std::to_string(julian_day) + " millis:" + std::to_string(millis);
}

struct SqlImportValueResult {
    bool ok = false;
    std::string json_fragment;
};

// Converts one already-typed VALUES literal into the JSON fragment
// extract_import_table_rows() (shared with the JSON import path) already
// knows how to read for that field's type category -- number literal for
// numeric fields, true/false for logical fields, a converted timestamp
// string for T-type fields, a quoted JSON string otherwise. A literal of
// the wrong shape for its column's type is rejected rather than coerced.
SqlImportValueResult sql_import_value_to_json(const SqlToken& token, char field_type) {
    if (sql_token_is_keyword(token, "NULL")) {
        return {.ok = true, .json_fragment = "null"};
    }
    const char upper_type = static_cast<char>(std::toupper(static_cast<unsigned char>(field_type)));
    const bool is_numeric = (upper_type == 'N' || upper_type == 'F' || upper_type == 'I' ||
        upper_type == 'B' || upper_type == 'Y');
    const bool is_logical = (upper_type == 'L');
    const bool is_datetime = (upper_type == 'T');
    if (is_logical) {
        if (sql_token_is_keyword(token, "TRUE")) {
            return {.ok = true, .json_fragment = "true"};
        }
        if (sql_token_is_keyword(token, "FALSE")) {
            return {.ok = true, .json_fragment = "false"};
        }
        return {};
    }
    if (is_numeric) {
        if (token.kind != SqlTokenKind::number) {
            return {};
        }
        return {.ok = true, .json_fragment = token.text};
    }
    if (token.kind != SqlTokenKind::string_literal) {
        return {};
    }
    if (is_datetime) {
        const auto storage = sql_datetime_storage_from_literal(token.text);
        if (!storage.has_value()) {
            return {};
        }
        return {.ok = true, .json_fragment = "\"" + json_escape_str(*storage) + "\""};
    }
    return {.ok = true, .json_fragment = "\"" + json_escape_str(token.text) + "\""};
}

}  // namespace

DatabaseJsonImportPlanResult build_database_sql_import_plan(const std::string_view document) {
    const auto failure = [](std::string code) {
        return DatabaseJsonImportPlanResult{.ok = false, .error_code = std::move(code), .plan = {}};
    };

    const SqlImportHeader header = parse_sql_import_header(document);
    if (!header.ok) {
        return failure("database_sql_import.invalid_header");
    }

    SqlImportTokenizer tokenizer(document.substr(header.body_offset));
    SqlToken current = tokenizer.next();
    const auto advance = [&]() { current = tokenizer.next(); };

    DatabaseJsonImportPlan plan;
    plan.database_name = header.database_name;
    std::map<std::string, std::size_t> table_index_by_name;
    std::set<std::string> casefolded_table_names;

    while (current.kind != SqlTokenKind::end_of_input) {
        if (current.kind == SqlTokenKind::invalid) {
            return failure("database_sql_import.invalid_token");
        }
        if (sql_token_is_keyword(current, "CREATE")) {
            advance();
            if (!sql_token_is_keyword(current, "TABLE")) {
                return failure("database_sql_import.invalid_create_table");
            }
            advance();
            if (current.kind != SqlTokenKind::quoted_identifier || current.text.empty()) {
                return failure("database_sql_import.invalid_create_table");
            }
            const std::string table_name = current.text;
            if (!casefolded_table_names.insert(lowercase_copy(table_name)).second) {
                return failure("database_sql_import.duplicate_table_name");
            }
            advance();
            if (!sql_token_is_punct(current, '(')) {
                return failure("database_sql_import.invalid_create_table");
            }
            advance();

            DatabaseJsonImportTablePlan table_plan;
            table_plan.name = table_name;
            std::set<std::string> casefolded_field_names;
            for (;;) {
                if (current.kind != SqlTokenKind::quoted_identifier || current.text.empty()) {
                    return failure("database_sql_import.invalid_field");
                }
                const std::string field_name = current.text;
                if (!casefolded_field_names.insert(lowercase_copy(field_name)).second) {
                    return failure("database_sql_import.duplicate_field_name");
                }
                advance();
                if (current.kind != SqlTokenKind::identifier) {
                    return failure("database_sql_import.unknown_column_type");
                }
                char field_type = '\0';
                std::size_t length = 0U;
                std::size_t decimals = 0U;
                if (sql_token_is_keyword(current, "VARCHAR")) {
                    advance();
                    if (!sql_token_is_punct(current, '(')) return failure("database_sql_import.unknown_column_type");
                    advance();
                    if (current.kind != SqlTokenKind::number || !parse_decimal_in_range(current.text, 1U, 255U, length))
                        return failure("database_sql_import.unknown_column_type");
                    advance();
                    if (!sql_token_is_punct(current, ')')) return failure("database_sql_import.unknown_column_type");
                    advance();
                    field_type = 'C';
                } else if (sql_token_is_keyword(current, "DECIMAL")) {
                    advance();
                    if (!sql_token_is_punct(current, '(')) return failure("database_sql_import.unknown_column_type");
                    advance();
                    if (current.kind != SqlTokenKind::number || !parse_decimal_in_range(current.text, 1U, 255U, length))
                        return failure("database_sql_import.unknown_column_type");
                    advance();
                    if (!sql_token_is_punct(current, ',')) return failure("database_sql_import.unknown_column_type");
                    advance();
                    if (current.kind != SqlTokenKind::number || !parse_decimal_in_range(current.text, 0U, length, decimals))
                        return failure("database_sql_import.unknown_column_type");
                    advance();
                    if (!sql_token_is_punct(current, ')')) return failure("database_sql_import.unknown_column_type");
                    advance();
                    field_type = 'N';
                } else if (sql_token_is_keyword(current, "INTEGER")) {
                    advance(); field_type = 'I'; length = 4U; decimals = 0U;
                } else if (sql_token_is_keyword(current, "DOUBLE")) {
                    advance();
                    if (!sql_token_is_keyword(current, "PRECISION")) return failure("database_sql_import.unknown_column_type");
                    advance(); field_type = 'B'; length = 8U; decimals = 0U;
                } else if (sql_token_is_keyword(current, "BOOLEAN")) {
                    advance(); field_type = 'L'; length = 1U; decimals = 0U;
                } else if (sql_token_is_keyword(current, "DATE")) {
                    advance(); field_type = 'D'; length = 8U; decimals = 0U;
                } else if (sql_token_is_keyword(current, "TIMESTAMP")) {
                    advance(); field_type = 'T'; length = 8U; decimals = 0U;
                } else if (sql_token_is_keyword(current, "TEXT")) {
                    // A memo-pointer field's on-disk value is always a
                    // 4-byte block number, matching the length this
                    // codebase's own JSON import path and other schema
                    // construction already use for M/G/P fields -- not an
                    // arbitrary width, even though is_dbf_table_field_
                    // storage_layout_writable() itself only requires >= 4.
                    advance(); field_type = 'M'; length = 4U; decimals = 0U;
                } else {
                    return failure("database_sql_import.unknown_column_type");
                }
                const DbfFieldDescriptor descriptor{
                    .name = field_name,
                    .type = field_type,
                    .offset = 0U,
                    .length = static_cast<std::uint8_t>(length),
                    .decimal_count = static_cast<std::uint8_t>(decimals)};
                if (!is_dbf_table_field_storage_layout_writable(descriptor.type, descriptor.length)) {
                    return failure("database_sql_import.invalid_field");
                }
                table_plan.fields.push_back(descriptor);

                if (sql_token_is_punct(current, ',')) { advance(); continue; }
                if (sql_token_is_punct(current, ')')) { advance(); break; }
                return failure("database_sql_import.invalid_create_table");
            }
            if (!sql_token_is_punct(current, ';')) {
                return failure("database_sql_import.invalid_create_table");
            }
            advance();
            if (table_plan.fields.empty()) {
                return failure("database_sql_import.invalid_create_table");
            }
            table_index_by_name.emplace(table_name, plan.tables.size());
            plan.tables.push_back(std::move(table_plan));
            continue;
        }

        if (sql_token_is_keyword(current, "INSERT")) {
            advance();
            if (!sql_token_is_keyword(current, "INTO")) {
                return failure("database_sql_import.invalid_insert");
            }
            advance();
            if (current.kind != SqlTokenKind::quoted_identifier) {
                return failure("database_sql_import.invalid_insert");
            }
            const auto table_lookup = table_index_by_name.find(current.text);
            if (table_lookup == table_index_by_name.end()) {
                return failure("database_sql_import.insert_unknown_table");
            }
            DatabaseJsonImportTablePlan& table_plan = plan.tables[table_lookup->second];
            advance();
            if (!sql_token_is_punct(current, '(')) {
                return failure("database_sql_import.invalid_insert");
            }
            advance();

            std::vector<std::string> column_names;
            for (;;) {
                if (current.kind != SqlTokenKind::quoted_identifier) {
                    return failure("database_sql_import.invalid_insert");
                }
                column_names.push_back(current.text);
                advance();
                if (sql_token_is_punct(current, ',')) { advance(); continue; }
                if (sql_token_is_punct(current, ')')) { advance(); break; }
                return failure("database_sql_import.invalid_insert");
            }
            if (column_names.empty()) {
                return failure("database_sql_import.invalid_insert");
            }

            std::vector<const DbfFieldDescriptor*> column_fields;
            column_fields.reserve(column_names.size());
            for (const std::string& column_name : column_names) {
                const auto field_iterator = std::find_if(
                    table_plan.fields.begin(), table_plan.fields.end(),
                    [&](const DbfFieldDescriptor& field) { return field.name == column_name; });
                if (field_iterator == table_plan.fields.end()) {
                    return failure("database_sql_import.insert_unknown_column");
                }
                column_fields.push_back(&*field_iterator);
            }

            if (!sql_token_is_keyword(current, "VALUES")) {
                return failure("database_sql_import.invalid_insert");
            }
            advance();
            if (!sql_token_is_punct(current, '(')) {
                return failure("database_sql_import.invalid_insert");
            }
            advance();

            std::vector<std::string> value_fragments;
            for (;;) {
                if (value_fragments.size() >= column_fields.size()) {
                    return failure("database_sql_import.insert_value_count_mismatch");
                }
                const SqlImportValueResult converted =
                    sql_import_value_to_json(current, column_fields[value_fragments.size()]->type);
                if (!converted.ok) {
                    return failure("database_sql_import.invalid_insert_value");
                }
                value_fragments.push_back(converted.json_fragment);
                advance();
                if (sql_token_is_punct(current, ',')) { advance(); continue; }
                if (sql_token_is_punct(current, ')')) { advance(); break; }
                return failure("database_sql_import.invalid_insert");
            }
            if (value_fragments.size() != column_fields.size()) {
                return failure("database_sql_import.insert_value_count_mismatch");
            }
            if (!sql_token_is_punct(current, ';')) {
                return failure("database_sql_import.invalid_insert");
            }
            advance();

            std::string row_json = "{";
            for (std::size_t index = 0U; index < column_names.size(); ++index) {
                if (index > 0U) row_json += ",";
                row_json += "\"" + json_escape_str(column_names[index]) + "\":" + value_fragments[index];
            }
            row_json += "}";
            table_plan.records_json += (table_plan.records_json.empty() ? "[" : ",") + row_json;
            continue;
        }

        return failure("database_sql_import.invalid_document");
    }

    if (plan.tables.empty()) {
        return failure("database_sql_import.invalid_document");
    }
    for (auto& table_plan : plan.tables) {
        table_plan.records_json = table_plan.records_json.empty() ? "[]" : table_plan.records_json + "]";
    }

    return {.ok = true, .error_code = {}, .plan = std::move(plan)};
}

namespace {

// A table name from untrusted JSON must never be usable to escape the
// destination DBC's own directory. std::filesystem::path's operator/
// silently replaces the whole path when the appended component is
// absolute, and a relative "../x" component resolves outside dbc_dir at
// the OS level even though the path string still nominally starts with
// it -- so this is rejected before any path is ever constructed from the
// name, not detected afterward.
bool table_name_is_safe_filesystem_component(const std::string& name) {
    if (name.empty() || name == "." || name == "..") {
        return false;
    }
    return name.find_first_of("/\\:") == std::string::npos;
}

std::string generate_import_staging_suffix() {
    static thread_local std::mt19937_64 engine{std::random_device{}()};
    std::uniform_int_distribution<std::uint64_t> distribution;
    std::ostringstream stream;
    stream << std::hex << std::setfill('0') << std::setw(16) << distribution(engine);
    return stream.str();
}

struct StagedImportFile {
    std::filesystem::path staged_path;
    std::filesystem::path final_path;
};

void remove_staged_import_files_and_directory(
    const std::vector<StagedImportFile>& staged,
    const std::filesystem::path& staging_dir) {
    std::error_code ignored;
    for (const auto& file : staged) {
        std::filesystem::remove(file.staged_path, ignored);
    }
    std::filesystem::remove_all(staging_dir, ignored);
}

struct TableRowExtractionResult {
    bool ok = false;
    std::string error;
    std::vector<std::vector<std::string>> rows;
};

// Re-parses one table's already-bounded records_json fragment (a substring
// of the overall envelope build_database_json_import_plan() already size-
// limited) and converts each row into the plain-string-per-field shape
// create_dbf_table_file() expects, using each field's own validated type to
// decide how a JSON value must be shaped: boolean for logical fields,
// number for numeric fields, string otherwise. A present-but-wrongly-typed
// value fails closed rather than silently coercing; a null or absent value
// becomes an empty field value.
TableRowExtractionResult extract_import_table_rows(
    const DatabaseJsonImportTablePlan& table_plan) {
    using copperfin::platform::JsonSelectionError;
    using copperfin::platform::JsonValueKind;
    using copperfin::platform::parse_json_document;

    const auto parsed = parse_json_document(table_plan.records_json);
    if (!parsed.ok()) {
        return {.ok = false, .error = asset_inspector_text(
            "Vfp.AssetInspector.Error.DatabaseImportInvalidRecords",
            {{"table", table_plan.name}})};
    }

    TableRowExtractionResult result;
    for (std::size_t row_index = 0U;; ++row_index) {
        const std::string row_pointer = "/" + std::to_string(row_index);
        const auto row = parsed.document.select(row_pointer);
        if (row.error == JsonSelectionError::value_not_found) {
            break;
        }
        if (!row.ok() || row.kind != JsonValueKind::object) {
            return {.ok = false, .error = asset_inspector_text(
                "Vfp.AssetInspector.Error.DatabaseImportInvalidRecords",
                {{"table", table_plan.name}})};
        }

        std::vector<std::string> row_values;
        row_values.reserve(table_plan.fields.size());
        for (const auto& field : table_plan.fields) {
            const auto value = parsed.document.select(
                row_pointer + "/" + json_pointer_token(field.name));
            const char upper_type = static_cast<char>(
                std::toupper(static_cast<unsigned char>(field.type)));
            const bool is_numeric = (upper_type == 'N' || upper_type == 'F' ||
                upper_type == 'I' || upper_type == 'B' || upper_type == 'Y');
            const bool is_logical = (upper_type == 'L');

            if (!value.ok() || value.kind == JsonValueKind::null_value) {
                row_values.emplace_back();
                continue;
            }
            if (is_logical) {
                if (value.kind != JsonValueKind::boolean) {
                    return {.ok = false, .error = asset_inspector_text(
                        "Vfp.AssetInspector.Error.DatabaseImportInvalidRecords",
                        {{"table", table_plan.name}})};
                }
                row_values.push_back(value.raw_json == "true" ? "T" : "F");
            } else if (is_numeric) {
                if (value.kind != JsonValueKind::number) {
                    return {.ok = false, .error = asset_inspector_text(
                        "Vfp.AssetInspector.Error.DatabaseImportInvalidRecords",
                        {{"table", table_plan.name}})};
                }
                row_values.push_back(value.raw_json);
            } else {
                if (value.kind != JsonValueKind::string) {
                    return {.ok = false, .error = asset_inspector_text(
                        "Vfp.AssetInspector.Error.DatabaseImportInvalidRecords",
                        {{"table", table_plan.name}})};
                }
                row_values.push_back(value.decoded_string);
            }
        }
        result.rows.push_back(std::move(row_values));
    }
    result.ok = true;
    return result;
}

}  // namespace

DatabaseJsonImportResult materialize_database_json_import_plan(
    const DatabaseJsonImportPlan& plan,
    const std::string& dbc_path) {
    namespace fs = std::filesystem;
    const auto failure = [](std::string message) {
        return DatabaseJsonImportResult{.ok = false, .error = std::move(message), .table_count = 0U};
    };

    if (plan.tables.empty()) {
        return failure(asset_inspector_text("Vfp.AssetInspector.Error.DatabaseImportNoTables"));
    }

    const fs::path dbc_fs_path = copperfin::platform::path_from_utf8_string(dbc_path);
    const fs::path dbc_dir = dbc_fs_path.parent_path();

    std::error_code exists_error;
    if (fs::exists(dbc_fs_path, exists_error)) {
        return failure(asset_inspector_text(
            "Vfp.AssetInspector.Error.DatabaseImportDestinationExists", {{"path", dbc_path}}));
    }

    // Resolve and pre-check every table's destination path up front -- one
    // already-existing table file must fail the whole import closed before
    // anything is written, not partially materialize around it.
    struct TableDestination {
        const DatabaseJsonImportTablePlan* plan = nullptr;
        fs::path path;
    };
    std::vector<TableDestination> table_destinations;
    std::set<std::string> casefolded_names;
    for (const auto& table_plan : plan.tables) {
        if (!table_name_is_safe_filesystem_component(table_plan.name)) {
            return failure(asset_inspector_text(
                "Vfp.AssetInspector.Error.DatabaseImportUnsafeTableName",
                {{"table", table_plan.name}}));
        }
        if (!casefolded_names.insert(lowercase_copy(table_plan.name)).second) {
            return failure(asset_inspector_text(
                "Vfp.AssetInspector.Error.DatabaseImportDuplicateTableName",
                {{"table", table_plan.name}}));
        }
        const fs::path table_path = dbc_dir /
            copperfin::platform::path_from_utf8_string(table_plan.name + ".dbf");
        if (fs::exists(table_path, exists_error)) {
            return failure(asset_inspector_text(
                "Vfp.AssetInspector.Error.DatabaseImportDestinationExists",
                {{"path", copperfin::platform::path_to_utf8_string(table_path)}}));
        }
        const bool table_has_memo_field = std::any_of(
            table_plan.fields.begin(), table_plan.fields.end(),
            [](const DbfFieldDescriptor& field) {
                return field.type == 'M' || field.type == 'G' || field.type == 'P';
            });
        if (table_has_memo_field) {
            fs::path memo_path = table_path;
            memo_path.replace_extension(".fpt");
            if (fs::exists(memo_path, exists_error)) {
                return failure(asset_inspector_text(
                    "Vfp.AssetInspector.Error.DatabaseImportDestinationExists",
                    {{"path", copperfin::platform::path_to_utf8_string(memo_path)}}));
            }
        }
        table_destinations.push_back({&table_plan, table_path});
    }

    // Stage every file in a temporary directory beside the destination DBC
    // (same volume, so the final commit renames are atomic on POSIX and
    // Windows), verify each one, and only then commit them into place.
    const fs::path staging_dir = dbc_dir /
        (".copperfin-import-" + generate_import_staging_suffix());
    std::error_code mkdir_error;
    fs::create_directories(staging_dir, mkdir_error);
    if (mkdir_error) {
        return failure(asset_inspector_text("Vfp.AssetInspector.Error.DatabaseImportStagingFailed"));
    }

    std::vector<StagedImportFile> staged;
    const auto abort_staging = [&](std::string message) {
        remove_staged_import_files_and_directory(staged, staging_dir);
        return failure(std::move(message));
    };

    for (const auto& destination : table_destinations) {
        const TableRowExtractionResult rows = extract_import_table_rows(*destination.plan);
        if (!rows.ok) {
            return abort_staging(rows.error);
        }
        const fs::path staged_path = staging_dir / destination.path.filename();
        const DbfWriteResult write_result = create_dbf_table_file(
            copperfin::platform::path_to_utf8_string(staged_path),
            destination.plan->fields,
            rows.rows);
        if (!write_result.ok) {
            return abort_staging(write_result.error);
        }
        staged.push_back({staged_path, destination.path});

        // A table with an M/G/P field gets a .fpt memo sidecar written
        // alongside the .dbf by create_dbf_table_file() -- it must be
        // staged and committed too, or a successful-looking import either
        // loses the memo payload (removed with the rest of staging_dir) or
        // resolves the DBF's memo pointers against an unrelated,
        // previously-existing sidecar at the final destination.
        const bool has_memo_field = std::any_of(
            destination.plan->fields.begin(), destination.plan->fields.end(),
            [](const DbfFieldDescriptor& field) {
                return field.type == 'M' || field.type == 'G' || field.type == 'P';
            });
        if (has_memo_field) {
            fs::path staged_memo_path = staged_path;
            staged_memo_path.replace_extension(".fpt");
            std::error_code memo_exists_error;
            if (!fs::exists(staged_memo_path, memo_exists_error)) {
                return abort_staging(
                    asset_inspector_text("Vfp.AssetInspector.Error.DatabaseImportStagingFailed"));
            }
            fs::path final_memo_path = destination.path;
            final_memo_path.replace_extension(".fpt");
            staged.push_back({staged_memo_path, final_memo_path});
        }
    }

    // The catalog is a minimal DBF: one row per table registering it as a
    // "table" object so load_database_catalog_snapshot() resolves it back.
    // No PROPERTIES memo, database-level row, field-level catalog rows, or
    // relation metadata is reconstructed -- table structure and data only,
    // matching this slice's scope.
    const std::vector<DbfFieldDescriptor> catalog_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 0U, .length = 10U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 0U, .length = 128U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 0U, .length = 128U, .decimal_count = 0U},
    };
    std::vector<std::vector<std::string>> catalog_rows;
    catalog_rows.reserve(table_destinations.size());
    for (const auto& destination : table_destinations) {
        catalog_rows.push_back({"Table", destination.plan->name, std::string{}});
    }
    const fs::path staged_dbc_path = staging_dir / dbc_fs_path.filename();
    const DbfWriteResult catalog_result = create_dbf_table_file(
        copperfin::platform::path_to_utf8_string(staged_dbc_path),
        catalog_fields,
        catalog_rows);
    if (!catalog_result.ok) {
        return abort_staging(catalog_result.error);
    }
    staged.push_back({staged_dbc_path, dbc_fs_path});

    // Commit: tables before the catalog (already the staged order above),
    // one file at a time via create_hard_link() rather than rename().
    // std::filesystem::rename() replaces an existing destination on POSIX,
    // which would silently defeat the fail-closed preflight checks above
    // against anything created during the staging window; create_hard_link()
    // fails instead of replacing when the destination already exists, so a
    // race during that window is caught here too, not just at preflight.
    // If any commit fails partway, every already-committed file is removed
    // so the destination is left exactly as it was found -- nothing
    // partial. The staging copies themselves are cleaned up afterward by
    // removing staging_dir; each committed file is now an independent hard
    // link to the same data, unaffected by that removal.
    std::vector<StagedImportFile> committed;
    committed.reserve(staged.size());
    for (const auto& file : staged) {
        std::error_code link_error;
        fs::create_hard_link(file.staged_path, file.final_path, link_error);
        if (link_error) {
            std::error_code ignored;
            for (const auto& done : committed) {
                fs::remove(done.final_path, ignored);
            }
            remove_staged_import_files_and_directory(staged, staging_dir);
            return failure(asset_inspector_text("Vfp.AssetInspector.Error.DatabaseImportCommitFailed"));
        }
        committed.push_back(file);
    }

    std::error_code cleanup_error;
    fs::remove_all(staging_dir, cleanup_error);

    return {.ok = true, .error = {}, .table_count = table_destinations.size()};
}

}  // namespace copperfin::vfp
