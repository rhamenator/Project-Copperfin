#include "copperfin/vfp/asset_inspector.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <set>
#include <vector>

namespace copperfin::vfp {

namespace {

std::string lowercase_extension(const std::filesystem::path& path) {
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return ext;
}

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

std::uint16_t read_le_u16(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint16_t>(bytes[offset]) |
           (static_cast<std::uint16_t>(bytes[offset + 1U]) << 8U);
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
    std::ifstream input(path, std::ios::binary);
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

std::string memo_sidecar_path_for(const std::filesystem::path& path, AssetFamily family) {
    std::filesystem::path candidate = path;
    switch (family) {
        case AssetFamily::project:
            candidate.replace_extension(".pjt");
            return candidate.string();
        case AssetFamily::form:
            candidate.replace_extension(".sct");
            return candidate.string();
        case AssetFamily::class_library:
            candidate.replace_extension(".vct");
            return candidate.string();
        case AssetFamily::report:
            candidate.replace_extension(".frt");
            return candidate.string();
        case AssetFamily::label:
            candidate.replace_extension(".lbt");
            return candidate.string();
        case AssetFamily::menu:
            candidate.replace_extension(".mnt");
            return candidate.string();
        case AssetFamily::database_container:
            candidate.replace_extension(".dct");
            return candidate.string();
        case AssetFamily::table:
            candidate.replace_extension(".fpt");
            return candidate.string();
        case AssetFamily::index:
        case AssetFamily::unknown:
        case AssetFamily::program:
        case AssetFamily::header:
            return {};
    }
    return {};
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
    const auto with_extension = [&](const char* extension) {
        std::filesystem::path candidate = path;
        candidate.replace_extension(extension);
        return candidate.string();
    };

    switch (family) {
        case AssetFamily::table:
            append_if_missing(candidates, path.string() + ".cdx");
            append_if_missing(candidates, path.string() + ".idx");
            append_if_missing(candidates, path.string() + ".ndx");
            append_if_missing(candidates, path.string() + ".mdx");
            append_if_missing(candidates, with_extension(".cdx"));
            append_if_missing(candidates, with_extension(".idx"));
            append_if_missing(candidates, with_extension(".ndx"));
            append_if_missing(candidates, with_extension(".mdx"));
            return candidates;
        case AssetFamily::database_container:
            append_if_missing(candidates, with_extension(".dcx"));
            return candidates;
        case AssetFamily::project:
        case AssetFamily::form:
        case AssetFamily::class_library:
        case AssetFamily::report:
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
    const auto with_extension = [&](const char* extension) {
        std::filesystem::path candidate = path;
        candidate.replace_extension(extension);
        return candidate.string();
    };

    switch (family) {
        case AssetFamily::table:
            append_if_missing(candidates, path.string() + ".cdx");
            append_if_missing(candidates, path.string() + ".mdx");
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
        std::error_code ignored;
        return std::filesystem::exists(candidate, ignored);
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
            "The DBF header length exceeds the file size.");
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
            "The DBF record storage is shorter than the header record-count and record-length values require.");
        return;
    }

    const std::uint64_t extra_bytes = available_record_bytes - expected_record_bytes;
    if (extra_bytes > 1U) {
        append_validation_issue(
            result,
            AssetValidationSeverity::warning,
            "dbf.record_storage_length_mismatch",
            path,
            "The DBF file contains extra bytes beyond the header-declared record storage.");
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
            "The DBF header does not leave room for a field-descriptor terminator.");
        return;
    }

    const auto terminator = std::find(
        table_bytes.begin() + static_cast<std::ptrdiff_t>(32U),
        table_bytes.begin() + static_cast<std::ptrdiff_t>(descriptor_region_end),
        static_cast<std::uint8_t>(0x0DU));
    if (terminator == table_bytes.begin() + static_cast<std::ptrdiff_t>(descriptor_region_end)) {
        append_validation_issue(
            result,
            AssetValidationSeverity::error,
            "dbf.descriptor_terminator_missing",
            path,
            "The DBF header does not contain a field-descriptor terminator within the declared header length.");
        return;
    }

    const std::size_t terminator_offset = static_cast<std::size_t>(std::distance(table_bytes.begin(), terminator));
    const std::size_t descriptor_span = terminator_offset - 32U;
    if ((descriptor_span % 32U) != 0U) {
        append_validation_issue(
            result,
            AssetValidationSeverity::error,
            "dbf.descriptor_span_misaligned",
            path,
            "The DBF field-descriptor span is not aligned to whole 32-byte descriptors.");
        return;
    }

    if ((terminator_offset + 1U) != static_cast<std::size_t>(header.header_length)) {
        append_validation_issue(
            result,
            AssetValidationSeverity::warning,
            "dbf.header_length_descriptor_mismatch",
            path,
            "The DBF header length does not match the field-descriptor terminator position.");
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
                "The DBF contains a blank field name.");
        } else {
            const std::string normalized_name = uppercase_copy(trimmed_name);
            if (!seen_names.insert(normalized_name).second) {
                append_validation_issue(
                    result,
                    AssetValidationSeverity::error,
                    "dbf.field_name_duplicate",
                    path,
                    "The DBF contains duplicate field names.");
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
                    "The DBF contains a field name that is blank, too long, or uses invalid identifier characters.");
            }
        }

        if (field.offset < 1U) {
            append_validation_issue(
                result,
                AssetValidationSeverity::error,
                "dbf.field_offset_invalid",
                path,
                "The DBF contains a field descriptor with an invalid record offset.");
        }

        const std::uint32_t field_end = field.offset + static_cast<std::uint32_t>(field.length);
        if (field_end > header.record_length) {
            append_validation_issue(
                result,
                AssetValidationSeverity::error,
                "dbf.field_layout_overflow",
                path,
                "The DBF contains a field descriptor that extends past the declared record length.");
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
                "The DBF contains overlapping field descriptors.");
            break;
        }
    }

    if (computed_record_length != header.record_length) {
        append_validation_issue(
            result,
            AssetValidationSeverity::warning,
            "dbf.record_length_mismatch",
            path,
            "The DBF header record length does not match the descriptor-derived field layout.");
    }
}

void validate_expected_companions(
    AssetInspectionResult& result,
    const std::string& path,
    AssetFamily family,
    const DbfHeader& header) {
    const std::filesystem::path file_path(path);

    if (asset_expects_memo_sidecar(family, header)) {
        const std::string memo_path = memo_sidecar_path_for(file_path, family);
        if (!memo_path.empty()) {
            std::error_code ignored;
            if (!std::filesystem::exists(memo_path, ignored)) {
                append_validation_issue(
                    result,
                    AssetValidationSeverity::error,
                    "memo.sidecar_missing",
                    memo_path,
                    "The DBF-family asset expects a memo sidecar file, but the sidecar is missing.");
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
            "The DBF-family asset expects a structural/production index companion, but no matching companion file was found.");
    }
}

void validate_memo_sidecar(
    AssetInspectionResult& result,
    const std::string& table_path,
    AssetFamily family,
    const DbfHeader& header,
    const std::vector<std::uint8_t>& table_bytes) {
    if (!asset_expects_memo_sidecar(family, header)) {
        return;
    }

    const std::string memo_path = memo_sidecar_path_for(std::filesystem::path(table_path), family);
    if (memo_path.empty()) {
        return;
    }

    std::error_code ignored;
    if (!std::filesystem::exists(memo_path, ignored)) {
        return;
    }

    const std::vector<std::uint8_t> memo_bytes = read_binary_file(memo_path);
    if (memo_bytes.size() < 8U) {
        append_validation_issue(
            result,
            AssetValidationSeverity::error,
            "memo.sidecar_header_truncated",
            memo_path,
            "The memo sidecar is too short to contain a valid header.");
        return;
    }

    const std::uint16_t block_size = read_be_u16(memo_bytes, 6U);
    if (block_size == 0U) {
        append_validation_issue(
            result,
            AssetValidationSeverity::error,
            "memo.block_size_invalid",
            memo_path,
            "The memo sidecar declares an invalid zero block size.");
        return;
    }

    if (memo_bytes.size() < block_size) {
        append_validation_issue(
            result,
            AssetValidationSeverity::error,
            "memo.sidecar_shorter_than_block_size",
            memo_path,
            "The memo sidecar is shorter than its declared block size.");
        return;
    }

    if (table_bytes.size() < header.header_length) {
        return;
    }

    const auto fields = read_raw_field_descriptors(table_bytes);
    const auto memo_field = std::find_if(fields.begin(), fields.end(), [](const RawFieldDescriptor& field) {
        return field.type == 'M';
    });
    if (memo_field == fields.end()) {
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
        const std::size_t field_offset = record_offset + memo_field->offset;
        if ((field_offset + 4U) > table_bytes.size()) {
            break;
        }

        const std::uint32_t block_number = read_le_u32(table_bytes, field_offset);
        if (block_number == 0U || !checked_blocks.insert(block_number).second) {
            continue;
        }

        const std::uint64_t block_offset = static_cast<std::uint64_t>(block_number) * static_cast<std::uint64_t>(block_size);
        if ((block_offset + 8U) > memo_bytes.size()) {
            append_validation_issue(
                result,
                AssetValidationSeverity::error,
                "memo.pointer_out_of_range",
                memo_path,
                "A memo field points to a block outside the available sidecar range.");
            continue;
        }

        const std::uint32_t payload_length = read_be_u32(memo_bytes, static_cast<std::size_t>(block_offset + 4U));
        const std::uint64_t payload_end = block_offset + 8U + static_cast<std::uint64_t>(payload_length);
        if (payload_end > memo_bytes.size()) {
            append_validation_issue(
                result,
                AssetValidationSeverity::error,
                "memo.payload_truncated",
                memo_path,
                "A referenced memo payload extends beyond the available sidecar bytes.");
        }
    }
}

}  // namespace

AssetFamily asset_family_from_path(const std::string& path) {
    const std::string ext = lowercase_extension(std::filesystem::path(path));

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

AssetInspectionResult inspect_asset(const std::string& path) {
    AssetInspectionResult result;
    result.path = path;
    result.family = asset_family_from_path(path);

    if (!std::filesystem::exists(path)) {
        result.ok = false;
        result.error = "Path does not exist.";
        return result;
    }

    if (result.family == AssetFamily::index) {
        const IndexParseResult index_result = parse_index_probe_from_file(path);
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

    const DbfParseResult header_result = parse_dbf_header_from_file(path);
    if (!header_result.ok) {
        result.ok = false;
        result.error = header_result.error;
        return result;
    }

    result.ok = true;
    result.header_available = true;
    result.header = header_result.header;
    const std::uint64_t file_size = static_cast<std::uint64_t>(std::filesystem::file_size(path));
    const std::vector<std::uint8_t> table_bytes = read_binary_file(path);

    validate_dbf_storage(result, path, result.header, file_size);
    validate_dbf_field_descriptors(result, path, result.header, table_bytes);
    validate_expected_companions(result, path, result.family, result.header);
    validate_memo_sidecar(result, path, result.family, result.header, table_bytes);

    for (const auto& companion_index : companion_index_paths_for(std::filesystem::path(path), result.family)) {
        if (!std::filesystem::exists(companion_index)) {
            continue;
        }

        const IndexParseResult index_result = parse_index_probe_from_file(companion_index);
        if (index_result.ok) {
            result.indexes.push_back({.path = companion_index, .probe = index_result.probe});
        } else if (!has_validation_issue(result, "index.companion_parse_failed", companion_index)) {
            append_validation_issue(
                result,
                AssetValidationSeverity::warning,
                "index.companion_parse_failed",
                companion_index,
                "A companion index file exists but could not be parsed: " + index_result.error);
        }
    }

    return result;
}

}  // namespace copperfin::vfp
