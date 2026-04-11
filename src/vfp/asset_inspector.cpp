#include "copperfin/vfp/asset_inspector.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
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

    validate_dbf_storage(result, path, result.header, file_size);
    validate_expected_companions(result, path, result.family, result.header);

    for (const auto& companion_index : companion_index_paths_for(std::filesystem::path(path), result.family)) {
        if (!std::filesystem::exists(companion_index)) {
            continue;
        }

        const IndexParseResult index_result = parse_index_probe_from_file(companion_index);
        if (index_result.ok) {
            result.indexes.push_back({.path = companion_index, .probe = index_result.probe});
        } else {
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
