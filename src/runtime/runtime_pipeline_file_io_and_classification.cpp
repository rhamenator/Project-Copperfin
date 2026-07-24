// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "runtime_pipeline_support.h"
#include "copperfin/platform/environment.h"

#include <functional>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace copperfin::runtime {

namespace runtime_pipeline_detail {

namespace {

bool path_exists_without_error(const std::filesystem::path& path) {
    std::error_code error;
    return std::filesystem::exists(path, error) && !error;
}

std::string normalize_vfp_separators(std::string value) {
    std::replace(value.begin(), value.end(), '\\', '/');
    return value;
}

std::vector<std::string> split_normalized_path_segments(const std::string& value) {
    std::vector<std::string> segments;
    for (const auto& part : copperfin::platform::path_from_utf8_string(value)) {
        const std::string segment = copperfin::platform::path_to_utf8_string(part);
        if (segment.empty() || segment == "." || segment == "/") {
            continue;
        }
        segments.push_back(segment);
    }
    return segments;
}

bool has_windows_drive_designator(const std::string& value) {
    return value.size() >= 2U &&
        std::isalpha(static_cast<unsigned char>(value[0])) != 0 &&
        value[1] == ':';
}

bool is_windows_drive_absolute_path(const std::string& value) {
    return value.size() >= 3U &&
        has_windows_drive_designator(value) &&
        (value[2] == '\\' || value[2] == '/');
}

bool is_windows_drive_relative_path(const std::string& value) {
    return has_windows_drive_designator(value) &&
        !is_windows_drive_absolute_path(value);
}

bool is_unc_path(const std::string& value) {
    return value.size() >= 2U &&
        ((value[0] == '\\' && value[1] == '\\') || (value[0] == '/' && value[1] == '/'));
}

bool is_vfp_absolute_path(const std::string& value) {
    return is_windows_drive_absolute_path(value) ||
        is_unc_path(value) ||
        copperfin::platform::path_from_utf8_string(value).is_absolute();
}

#if defined(_WIN32)
std::optional<std::filesystem::path> expand_windows_drive_relative_path(
    const std::filesystem::path& candidate) {
    std::wstring buffer(64U, L'\0');
    for (;;) {
        const DWORD length = ::GetFullPathNameW(
            candidate.c_str(),
            static_cast<DWORD>(buffer.size()),
            buffer.data(),
            nullptr);
        if (length == 0U) {
            return std::nullopt;
        }
        if (length < buffer.size()) {
            buffer.resize(length);
            const std::filesystem::path expanded(buffer);
            if (!expanded.is_absolute()) {
                return std::nullopt;
            }
            return expanded.lexically_normal();
        }
        buffer.assign(static_cast<std::size_t>(length) + 1U, L'\0');
    }
}
#endif

std::filesystem::path resolve_vfp_path_from_base(
    const std::filesystem::path& base_dir,
    const std::string& value) {
    if (trim_copy(value).empty()) {
        return {};
    }

    const std::string normalized = normalize_vfp_separators(value);
    const std::filesystem::path candidate = copperfin::platform::path_from_utf8_string(normalized);
    if (is_windows_drive_relative_path(normalized)) {
        return candidate.lexically_normal();
    }
    if (!is_vfp_absolute_path(normalized) && candidate.is_relative()) {
        return (base_dir / candidate).lexically_normal();
    }

    return candidate.lexically_normal();
}

std::string sanitize_package_relative_path(const std::string& value) {
    if (trim_copy(value).empty()) {
        return {};
    }

    std::string normalized = normalize_vfp_separators(value);
    if (is_vfp_absolute_path(normalized)) {
        normalized = copperfin::platform::path_to_utf8_string(
            copperfin::platform::path_from_utf8_string(normalized).filename());
    } else if (is_windows_drive_relative_path(normalized)) {
        normalized.erase(0U, 2U);
        while (!normalized.empty() && normalized.front() == '/') {
            normalized.erase(normalized.begin());
        }
    }

    std::vector<std::string> segments;
    for (const auto& segment : split_normalized_path_segments(normalized)) {
        if (segment == "..") {
            if (!segments.empty()) {
                segments.pop_back();
            }
            continue;
        }

        segments.push_back(segment);
    }

    if (segments.empty()) {
        const std::string file_name = copperfin::platform::path_to_utf8_string(
            copperfin::platform::path_from_utf8_string(normalized).filename());
        if (!file_name.empty() && file_name != "." && file_name != "..") {
            return file_name;
        }
        return {};
    }

    std::ostringstream stream;
    for (std::size_t index = 0; index < segments.size(); ++index) {
        if (index != 0U) {
            stream << '/';
        }
        stream << segments[index];
    }
    return stream.str();
}

bool has_parent_traversal_segment(const std::string& value) {
    const std::string normalized = normalize_vfp_separators(value);
    const auto segments = split_normalized_path_segments(normalized);
    return std::any_of(segments.begin(), segments.end(), [](const std::string& segment) {
        return segment == "..";
    });
}

std::optional<std::filesystem::path> resolve_existing_path_casefold_impl(
    const std::filesystem::path& candidate,
    bool& ambiguous) {
    ambiguous = false;
    if (candidate.empty()) {
        return std::nullopt;
    }

    std::filesystem::path resolved = candidate.root_path();
    if (resolved.empty()) {
        resolved = ".";
    }

    for (const auto& part : candidate.relative_path()) {
        const std::string component = copperfin::platform::path_to_utf8_string(part);
        if (component.empty() || component == ".") {
            continue;
        }
        if (component == "..") {
            resolved = (resolved / part).lexically_normal();
            continue;
        }

        std::error_code error;
        if (!std::filesystem::is_directory(resolved, error) || error) {
            return std::nullopt;
        }

        std::optional<std::filesystem::path> exact_match;
        std::vector<std::filesystem::path> casefold_matches;
        const std::string folded_component = lowercase_copy(component);
        for (std::filesystem::directory_iterator it(resolved, error), end; it != end; it.increment(error)) {
            if (error) {
                return std::nullopt;
            }

            const std::filesystem::path entry_path = it->path();
            const std::string entry_name = copperfin::platform::path_to_utf8_string(entry_path.filename());
            if (entry_name == component) {
                exact_match = entry_path;
                break;
            }
            if (lowercase_copy(entry_name) == folded_component) {
                casefold_matches.push_back(entry_path);
            }
        }

        if (exact_match.has_value()) {
            resolved = exact_match->lexically_normal();
            continue;
        }
        if (casefold_matches.size() > 1U) {
            ambiguous = true;
            return std::nullopt;
        }
        if (casefold_matches.empty()) {
            const std::filesystem::path host_resolved = (resolved / part).lexically_normal();
            std::error_code exists_error;
            if (std::filesystem::exists(host_resolved, exists_error) && !exists_error) {
                resolved = host_resolved;
                continue;
            }
            return std::nullopt;
        }
        resolved = casefold_matches.front().lexically_normal();
    }

    return resolved.lexically_normal();
}

std::optional<std::filesystem::path> find_case_insensitive_tail_match_under_root(
    const std::filesystem::path& search_root,
    const std::string& value,
    bool require_unique,
    bool& ambiguous) {
    ambiguous = false;
    if (trim_copy(value).empty() || !path_exists_without_error(search_root)) {
        return std::nullopt;
    }

    const std::string sanitized_tail = sanitize_package_relative_path(value);
    if (sanitized_tail.empty()) {
        return std::nullopt;
    }

    const auto tail_segments = split_normalized_path_segments(sanitized_tail);
    if (tail_segments.empty()) {
        return std::nullopt;
    }

    std::optional<std::filesystem::path> best_match;
    std::size_t best_extra_segments = static_cast<std::size_t>(-1);
    std::string best_key;
    std::vector<std::filesystem::path> exact_tail_matches;
    std::vector<std::filesystem::path> casefold_tail_matches;
    std::error_code iterator_error;
    for (std::filesystem::recursive_directory_iterator it(search_root, iterator_error), end; it != end; it.increment(iterator_error)) {
        if (iterator_error) {
            iterator_error.clear();
            continue;
        }

        std::error_code status_error;
        if (!it->is_regular_file(status_error) || status_error) {
            continue;
        }

        const auto candidate_relative = it->path().lexically_relative(search_root);
        if (candidate_relative.empty()) {
            continue;
        }

        const auto candidate_segments = split_normalized_path_segments(
            copperfin::platform::path_to_utf8_string(candidate_relative));
        if (candidate_segments.size() < tail_segments.size()) {
            continue;
        }

        bool matches = true;
        bool exact_match = true;
        for (std::size_t index = 0; index < tail_segments.size(); ++index) {
            const auto& candidate_segment = candidate_segments[candidate_segments.size() - tail_segments.size() + index];
            const auto& expected_segment = tail_segments[index];
            if (lowercase_copy(candidate_segment) != lowercase_copy(expected_segment)) {
                matches = false;
                break;
            }
            if (candidate_segment != expected_segment) {
                exact_match = false;
            }
        }

        if (!matches) {
            continue;
        }

        if (require_unique) {
            casefold_tail_matches.push_back(it->path().lexically_normal());
            if (exact_match) {
                exact_tail_matches.push_back(it->path().lexically_normal());
            }
            continue;
        }

        const std::size_t extra_segments = candidate_segments.size() - tail_segments.size();
        const std::string candidate_key = lowercase_copy(
            copperfin::platform::path_to_utf8_string(candidate_relative));
        if (!best_match ||
            extra_segments < best_extra_segments ||
            (extra_segments == best_extra_segments && candidate_key < best_key)) {
            best_match = it->path().lexically_normal();
            best_extra_segments = extra_segments;
            best_key = candidate_key;
        }
    }

    if (require_unique) {
        const auto& preferred_matches =
            exact_tail_matches.empty() ? casefold_tail_matches : exact_tail_matches;
        if (preferred_matches.size() > 1U) {
            ambiguous = true;
            return std::nullopt;
        }
        return preferred_matches.empty()
            ? std::nullopt
            : std::optional<std::filesystem::path>(preferred_matches.front());
    }

    return best_match;
}

}  // namespace

std::optional<std::filesystem::path> resolve_existing_path_casefold(
    const std::filesystem::path& candidate,
    bool& ambiguous) {
    return resolve_existing_path_casefold_impl(candidate, ambiguous);
}

BuildOutputKind parse_build_output_kind(const std::string& value) {
    const std::string normalized = lowercase_copy(trim_copy(value));
    if (normalized == "dll") {
        return BuildOutputKind::dll;
    }
    if (normalized == "app") {
        return BuildOutputKind::app;
    }
    if (normalized == "fll") {
        return BuildOutputKind::fll;
    }
    if (normalized == "fxp") {
        return BuildOutputKind::fxp;
    }
    if (normalized == "ocx") {
        return BuildOutputKind::ocx;
    }
    if (normalized == "executable") {
        return BuildOutputKind::executable;
    }
    return BuildOutputKind::unknown;
}

std::string dotnet_parity_tier_name(copperfin::platform::DotNetParityTier tier) {
    switch (tier) {
        case copperfin::platform::DotNetParityTier::exact:
            return "exact";
        case copperfin::platform::DotNetParityTier::adapted:
            return "adapted";
        case copperfin::platform::DotNetParityTier::intentionally_not_supported:
            return "intentionally_not_supported";
        default:
            return "unknown";
    }
}

bool write_text_file(const std::filesystem::path& path, const std::string& contents, std::string& error) {
#if !defined(_WIN32)
    bool fd_write_handled = false;
    if (!try_write_text_file_fd_backed(
            path,
            contents,
            fd_write_handled,
            error)) {
        return false;
    }
    if (fd_write_handled) {
        return true;
    }
#endif
    std::ofstream output(path, std::ios::binary);
    if (!output) {
        error = runtime_text("Runtime.Package.Error.CreateFileFailed", {{"path", copperfin::platform::path_to_utf8_string(path)}});
        return false;
    }

    output << contents;
    const bool wrote = output.good();
    output.flush();
    const bool flushed = output.good();
    output.close();
    const bool closed = output.good();
    if (!wrote || !flushed || !closed) {
        error = runtime_text("Runtime.Package.Error.WriteFileFailed", {{"path", copperfin::platform::path_to_utf8_string(path)}});
        return false;
    }

    return true;
}

bool write_text_file(
    const std::string& utf8_path,
    const std::string& contents,
    std::string& error) {
    return write_text_file(
        copperfin::platform::path_from_utf8_string(utf8_path),
        contents,
        error);
}

std::string read_text_file(const std::filesystem::path& path) {
#if !defined(_WIN32)
    bool fd_read_handled = false;
    std::string fd_contents;
    if (try_read_file_fd_backed(path, fd_read_handled, fd_contents) && fd_read_handled) {
        return fd_contents;
    }
#endif
    std::ifstream input(path, std::ios::binary);
    std::ostringstream stream;
    stream << input.rdbuf();
    return stream.str();
}

std::string read_binary_file(const std::filesystem::path& path, std::string& error) {
#if !defined(_WIN32)
    bool fd_read_handled = false;
    std::string fd_contents;
    if (try_read_file_fd_backed(path, fd_read_handled, fd_contents) && fd_read_handled) {
        return fd_contents;
    }
    if (fd_read_handled) {
        error = runtime_text("Runtime.Package.Error.OpenFileFailed", {{"path", copperfin::platform::path_to_utf8_string(path)}});
        return {};
    }
#endif
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        error = runtime_text("Runtime.Package.Error.OpenFileFailed", {{"path", copperfin::platform::path_to_utf8_string(path)}});
        return {};
    }

    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

std::string hex_encode_bytes(const std::string& bytes) {
    std::ostringstream stream;
    stream << std::hex << std::setfill('0');
    for (const unsigned char byte : bytes) {
        stream << std::setw(2) << static_cast<unsigned int>(byte);
    }
    return stream.str();
}

bool append_runtime_artifact_digest(
    std::vector<RuntimeArtifactDigest>& digests,
    const std::string& path,
    std::string& error) {
    const std::filesystem::path native_path = copperfin::platform::path_from_utf8_string(path);
    if (trim_copy(path).empty() || !path_exists_without_error(native_path)) {
        return true;
    }

    const auto digest = security::sha256_hex_for_file(path);
    if (!digest.ok) {
        error = digest.error;
        return false;
    }

    const auto existing = std::find_if(digests.begin(), digests.end(), [&](const RuntimeArtifactDigest& entry) {
        return entry.path == path;
    });
    if (existing != digests.end()) {
        existing->sha256 = digest.hex_digest;
        return true;
    }

    digests.push_back({
        .path = path,
        .sha256 = digest.hex_digest
    });
    return true;
}

bool is_library_output_kind(const BuildOutputKind output_kind) {
    return output_kind == BuildOutputKind::dll ||
        output_kind == BuildOutputKind::fll ||
        output_kind == BuildOutputKind::ocx;
}

bool is_native_host_output_kind(const BuildOutputKind output_kind) {
    return output_kind == BuildOutputKind::executable ||
        output_kind == BuildOutputKind::unknown;
}

std::string runtime_host_file_name() {
#if defined(_WIN32)
    return "copperfin_runtime_host.exe";
#else
    return "copperfin_runtime_host";
#endif
}

std::string resolve_output_file_name(const studio::StudioProjectWorkspace& workspace, const std::string& project_title) {
    const std::filesystem::path configured_output = copperfin::platform::path_from_utf8_string(
        workspace.build_plan.output_path);
    const std::string file_name = copperfin::platform::path_to_utf8_string(configured_output.filename());
    if (!trim_copy(file_name).empty()) {
        return file_name;
    }
    return sanitize_file_name(project_title) + ".exe";
}

BuildOutputKind infer_build_output_kind_from_output_path(const std::string& output_path) {
    const std::string extension = lowercase_copy(trim_copy(
        copperfin::platform::path_to_utf8_string(
            copperfin::platform::path_from_utf8_string(output_path).extension())));
    if (extension == ".dll") {
        return BuildOutputKind::dll;
    }
    if (extension == ".app") {
        return BuildOutputKind::app;
    }
    if (extension == ".fll") {
        return BuildOutputKind::fll;
    }
    if (extension == ".fxp") {
        return BuildOutputKind::fxp;
    }
    if (extension == ".ocx") {
        return BuildOutputKind::ocx;
    }
    if (extension == ".exe") {
        return BuildOutputKind::executable;
    }
    return BuildOutputKind::unknown;
}

bool copy_file_if_exists(
    const std::filesystem::path& source,
    const std::filesystem::path& destination,
    std::string& error) {
#if !defined(_WIN32)
    bool fd_source_handled = false;
    std::string fd_source_contents;
    const bool fd_source_readable = try_read_file_fd_backed(
        source,
        fd_source_handled,
        fd_source_contents);
    if ((fd_source_handled && !fd_source_readable) ||
        (!fd_source_handled && !path_exists_without_error(source))) {
#else
    if (!path_exists_without_error(source)) {
#endif
        error = runtime_text("Runtime.Package.Error.SourceFileMissing", {{"path", copperfin::platform::path_to_utf8_string(source)}});
        return false;
    }

#if !defined(_WIN32)
    bool fd_copy_handled = false;
    if (!try_copy_file_if_exists_fd_backed(
            source,
            destination,
            fd_copy_handled,
            error)) {
        return false;
    }
    if (fd_copy_handled) {
        return true;
    }
#endif

    std::error_code directory_error;
    std::filesystem::create_directories(destination.parent_path(), directory_error);
    if (directory_error) {
        error = runtime_text("Runtime.Package.Error.CreateDirectoryFailed", {{"path", copperfin::platform::path_to_utf8_string(destination.parent_path())}});
        return false;
    }

    std::error_code copy_error;
    std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing, copy_error);
    if (copy_error) {
        error = runtime_text("Runtime.Package.Error.CopyFileFailed", {{"path", copperfin::platform::path_to_utf8_string(destination)}});
        return false;
    }

    return true;
}

bool copy_file_if_exists(
    const std::filesystem::path& source,
    const std::string& utf8_destination,
    std::string& error) {
    return copy_file_if_exists(
        source,
        copperfin::platform::path_from_utf8_string(utf8_destination),
        error);
}

bool validate_runtime_host_source_path(
    const RuntimePackagePlan& plan,
    const std::string& runtime_host_source_path,
    std::string& error) {
    if (runtime_host_source_path.empty()) {
        error = runtime_text("Runtime.Package.Error.RuntimeHostSourcePathEmpty");
        return false;
    }

    std::filesystem::path source =
        copperfin::platform::path_from_utf8_string(runtime_host_source_path);
    if (source.empty()) {
        error = runtime_text("Runtime.Package.Error.RuntimeHostSourcePathResolveFailed");
        return false;
    }
    if (plan.security_enabled && !source.is_absolute()) {
        error = runtime_text("Runtime.Package.Error.SecurityRequiresAbsoluteRuntimeHostPath");
        return false;
    }

    std::error_code canonical_error;
    source = std::filesystem::weakly_canonical(source, canonical_error);
    if (canonical_error) {
        error = runtime_text("Runtime.Package.Error.RuntimeHostSourcePathResolveFailed");
        return false;
    }

    std::error_code source_status_error;
    if (!std::filesystem::exists(source, source_status_error) || source_status_error) {
        error = runtime_text("Runtime.Package.Error.RuntimeHostSourcePathNotRegularFile");
        return false;
    }
    source_status_error.clear();
    if (!std::filesystem::is_regular_file(source, source_status_error) || source_status_error) {
        error = runtime_text("Runtime.Package.Error.RuntimeHostSourcePathNotRegularFile");
        return false;
    }

    if (plan.security_enabled) {
        const std::string expected_file_name = runtime_host_file_name();
        if (lowercase_copy(copperfin::platform::path_to_utf8_string(source.filename())) != expected_file_name) {
            error = runtime_text("Runtime.Package.Error.SecurityRequiresCanonicalRuntimeHostName");
            return false;
        }
    }

    return true;
}

std::string resolve_project_item_source(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectEntry& entry,
    bool require_unique_casefold,
    std::string& error) {
    error.clear();
    const std::filesystem::path base_dir = copperfin::platform::path_from_utf8_string(document.path).parent_path();

    const auto resolve_candidate = [&](const std::filesystem::path& candidate)
        -> std::optional<std::filesystem::path> {
        bool ambiguous = false;
        const auto resolved = resolve_existing_path_casefold(candidate, ambiguous);
        if (ambiguous) {
            error = runtime_text(
                "Runtime.Package.Error.AmbiguousProjectAssetPath",
                {{"path", copperfin::platform::path_to_utf8_string(candidate)}});
        }
        return resolved;
    };

    if (!entry.relative_path.empty()) {
        const std::string normalized_relative_path =
            normalize_vfp_separators(entry.relative_path);
        const std::filesystem::path from_relative = resolve_vfp_path_from_base(base_dir, entry.relative_path);
#if !defined(_WIN32)
        if (is_windows_drive_absolute_path(normalized_relative_path) ||
            is_windows_drive_relative_path(normalized_relative_path) ||
            is_unc_path(normalized_relative_path)) {
            return normalized_relative_path;
        }
#endif
#if defined(_WIN32)
        std::filesystem::path lookup_path = from_relative;
        const bool drive_relative = is_windows_drive_relative_path(normalized_relative_path);
        if (drive_relative) {
            const auto expanded = expand_windows_drive_relative_path(from_relative);
            if (expanded.has_value()) {
                lookup_path = *expanded;
            }
        }
#else
        const std::filesystem::path& lookup_path = from_relative;
#endif
        if (const auto resolved = resolve_candidate(lookup_path); resolved.has_value()) {
            return copperfin::platform::path_to_utf8_string(resolved->lexically_normal());
        }
#if defined(_WIN32)
        if (drive_relative && error.empty() && lookup_path.is_absolute()) {
            std::error_code exists_error;
            if (std::filesystem::exists(lookup_path, exists_error) && !exists_error) {
                return copperfin::platform::path_to_utf8_string(lookup_path.lexically_normal());
            }
        }
#endif
        if (!error.empty()) {
            return copperfin::platform::path_to_utf8_string(from_relative.lexically_normal());
        }
        if (is_vfp_absolute_path(normalized_relative_path) ||
            is_windows_drive_relative_path(normalized_relative_path)) {
            return copperfin::platform::path_to_utf8_string(from_relative.lexically_normal());
        }
        if (has_parent_traversal_segment(entry.relative_path)) {
            bool fallback_ambiguous = false;
            if (const auto fallback = find_case_insensitive_tail_match_under_root(
                    base_dir.parent_path(),
                    entry.relative_path,
                    require_unique_casefold,
                    fallback_ambiguous)) {
                return copperfin::platform::path_to_utf8_string(fallback->lexically_normal());
            }
            if (fallback_ambiguous) {
                error = runtime_text(
                    "Runtime.Package.Error.AmbiguousProjectAssetPath",
                    {{"path", copperfin::platform::path_to_utf8_string(from_relative)}});
                return copperfin::platform::path_to_utf8_string(from_relative.lexically_normal());
            }
        }
        if (entry.name.empty()) {
            return copperfin::platform::path_to_utf8_string(from_relative.lexically_normal());
        }
    }

    if (entry.name.empty()) {
        return {};
    }

    const std::filesystem::path from_name = resolve_vfp_path_from_base(base_dir, entry.name);
    if (const auto resolved = resolve_candidate(from_name); resolved.has_value()) {
        return copperfin::platform::path_to_utf8_string(resolved->lexically_normal());
    }
    return copperfin::platform::path_to_utf8_string(from_name.lexically_normal());
}

std::vector<std::filesystem::path> discover_prg_include_source_paths(
    const std::filesystem::path& source) {
    std::vector<std::filesystem::path> discovered;
    std::unordered_set<std::string> visited;

    const auto path_identity = [](const std::filesystem::path& path) {
        return lowercase_copy(copperfin::platform::path_to_utf8_string(path.lexically_normal()));
    };
    const auto is_include_source = [](const std::filesystem::path& path) {
        const std::string extension = lowercase_copy(
            copperfin::platform::path_to_utf8_string(path.extension()));
        return extension == ".prg" || extension == ".mpr" || extension == ".h" ||
            extension == ".inc" || extension == ".ch" || extension == ".txt";
    };
    const auto resolve_include = [](const std::filesystem::path& owner, std::string value) {
        std::replace(value.begin(), value.end(), '\\', '/');
        bool ambiguous = false;
        const auto direct = resolve_existing_path_casefold(
            (owner.parent_path() / copperfin::platform::path_from_utf8_string(value)).lexically_normal(),
            ambiguous);
        if (direct.has_value()) {
            return direct;
        }

        ambiguous = false;
        return resolve_existing_path_casefold(
            (owner.parent_path() / copperfin::platform::path_from_utf8_string(value).filename()).lexically_normal(),
            ambiguous);
    };
    std::function<void(const std::filesystem::path&)> visit;
    visit = [&](const std::filesystem::path& current) {
        const auto normalized = current.lexically_normal();
        if (!is_include_source(normalized) || !visited.insert(path_identity(normalized)).second) {
            return;
        }

        std::ifstream input(normalized);
        if (!input) {
            return;
        }

        std::string line;
        while (std::getline(input, line)) {
            const std::string trimmed = trim_copy(line);
            constexpr char include_directive[] = "#INCLUDE";
            const bool is_include_directive = trimmed.size() >= 8U &&
                std::equal(
                    trimmed.begin(),
                    trimmed.begin() + 8,
                    std::begin(include_directive),
                    [](const char left, const char right) {
                        return std::tolower(static_cast<unsigned char>(left)) ==
                            std::tolower(static_cast<unsigned char>(right));
                    });
            if (!is_include_directive) {
                continue;
            }

            const std::string body = trim_copy(trimmed.substr(8U));
            if (body.size() < 2U ||
                !((body.front() == '"' && body.back() == '"') ||
                  (body.front() == '\'' && body.back() == '\'') ||
                  (body.front() == '<' && body.back() == '>'))) {
                continue;
            }

            const auto resolved = resolve_include(normalized, body.substr(1U, body.size() - 2U));
            if (!resolved.has_value() || !is_include_source(*resolved)) {
                continue;
            }
            if (path_identity(*resolved) != path_identity(normalized)) {
                discovered.push_back(resolved->lexically_normal());
            }
            visit(*resolved);
        }
    };

    visit(source);
    return discovered;
}

bool source_path_exists_on_host(const std::string& value) {
    if (trim_copy(value).empty()) {
        return false;
    }

    const std::string normalized = normalize_vfp_separators(value);
#if !defined(_WIN32)
    if (is_windows_drive_absolute_path(normalized) ||
        is_windows_drive_relative_path(normalized) ||
        is_unc_path(normalized)) {
        return false;
    }
#else
    if (is_windows_drive_relative_path(normalized)) {
        return false;
    }
#endif

    std::error_code error;
        return std::filesystem::exists(copperfin::platform::path_from_utf8_string(normalized), error) && !error;
}

std::string relative_asset_path(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectEntry& entry,
    const std::string& resolved_source_path,
    bool preserve_resolved_spelling) {
    if (preserve_resolved_spelling && !resolved_source_path.empty()) {
        const std::filesystem::path base_dir = copperfin::platform::path_from_utf8_string(document.path).parent_path();
        const std::filesystem::path resolved_relative =
            copperfin::platform::path_from_utf8_string(resolved_source_path).lexically_relative(base_dir);
        if (!resolved_relative.empty() && !resolved_relative.is_absolute()) {
            const std::string sanitized = sanitize_package_relative_path(
                copperfin::platform::path_to_utf8_string(resolved_relative));
            if (!sanitized.empty()) {
                return sanitized;
            }
        }
    }

    const std::string path = !entry.relative_path.empty() ? entry.relative_path : entry.name;
    if (!path.empty()) {
        const std::string sanitized = sanitize_package_relative_path(path);
        if (!sanitized.empty()) {
            return sanitized;
        }
    }
    return "record_" + std::to_string(entry.record_index) + ".asset";
}

std::string resolve_working_directory(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectWorkspace& workspace) {
    const std::filesystem::path document_dir = copperfin::platform::path_from_utf8_string(document.path).parent_path();
    if (!workspace.home_directory.empty()) {
        const std::string normalized_home_directory =
            normalize_vfp_separators(workspace.home_directory);
        const std::filesystem::path normalized_home_path =
            copperfin::platform::path_from_utf8_string(normalized_home_directory);
        const bool is_project_relative =
            !is_vfp_absolute_path(normalized_home_directory) &&
            !has_windows_drive_designator(normalized_home_directory) &&
            !normalized_home_path.has_root_path() &&
            normalized_home_path.is_relative();
        const std::filesystem::path home_directory = is_project_relative
            ? resolve_vfp_path_from_base(document_dir, workspace.home_directory)
            : copperfin::platform::path_from_utf8_string(workspace.home_directory);
        std::error_code directory_error;
        if (std::filesystem::is_directory(home_directory, directory_error) && !directory_error) {
            return copperfin::platform::path_to_utf8_string(home_directory.lexically_normal());
        }
    }
    return copperfin::platform::path_to_utf8_string(document_dir.lexically_normal());
}

std::string resolve_security_role(bool security_enabled) {
    if (!security_enabled) {
        return {};
    }

    std::string role =
        trim_copy(copperfin::platform::read_environment_variable_or_empty("COPPERFIN_SECURITY_ROLE"));
    if (!role.empty()) {
        return role;
    }

    return "developer";
}

bool is_extension_payload_path(const std::filesystem::path& path) {
    const std::string extension = lowercase_copy(trim_copy(
        copperfin::platform::path_to_utf8_string(path.extension())));
    return extension == ".dll" || extension == ".exe" || extension == ".vsix";
}

bool is_recognized_security_role(
    const security::NativeSecurityProfile& profile,
    const std::string& role_id) {
    return std::find_if(profile.roles.begin(), profile.roles.end(), [&](const security::NativeRole& role) {
               return role.id == role_id;
           }) != profile.roles.end();
}

bool is_prg_path(const std::string& value) {
        return lowercase_copy(trim_copy(
            copperfin::platform::path_to_utf8_string(
                copperfin::platform::path_from_utf8_string(value).extension()))) == ".prg";
}

bool is_xasset_path(const std::string& value) {
    const std::string extension = trim_copy(lowercase_copy(
        copperfin::platform::path_to_utf8_string(
            copperfin::platform::path_from_utf8_string(value).extension())));
    return extension == ".scx" ||
        extension == ".vcx" ||
        extension == ".frx" ||
        extension == ".lbx" ||
        extension == ".mnx";
}

bool is_writable_package_data_path(const std::string& value) {
    const std::string extension = trim_copy(lowercase_copy(
        copperfin::platform::path_to_utf8_string(
            copperfin::platform::path_from_utf8_string(value).extension())));
    return extension == ".dbf";
}

bool should_stage_asset(const RuntimePackageAsset& asset) {
    return asset.exists && (!asset.excluded || asset.required_for_runtime);
}

std::vector<std::filesystem::path> infer_companion_source_paths(const std::filesystem::path& source) {
    std::vector<std::filesystem::path> companions;
    const std::string extension = trim_copy(lowercase_copy(
        copperfin::platform::path_to_utf8_string(source.extension())));
    const auto same_stem = [&](const char* companion_extension) {
        auto path = source;
        path.replace_extension(companion_extension);
        return path;
    };

    if (extension == ".pjx") {
        companions.push_back(same_stem(".pjt"));
    } else if (extension == ".scx") {
        companions.push_back(same_stem(".sct"));
    } else if (extension == ".vcx") {
        companions.push_back(same_stem(".vct"));
    } else if (extension == ".frx") {
        companions.push_back(same_stem(".frt"));
    } else if (extension == ".lbx") {
        companions.push_back(same_stem(".lbt"));
    } else if (extension == ".mnx") {
        companions.push_back(same_stem(".mnt"));
    } else if (extension == ".dbf") {
        companions.push_back(same_stem(".fpt"));
        companions.push_back(same_stem(".cdx"));
        companions.push_back(same_stem(".idx"));
        companions.push_back(same_stem(".ndx"));
        companions.push_back(same_stem(".mdx"));
    } else if (extension == ".dbc") {
        companions.push_back(same_stem(".dct"));
        companions.push_back(same_stem(".dcx"));
    }

    return companions;
}

RuntimeCompanionCopyResult copy_companion_files_if_present(
    const RuntimePackageAsset& asset,
    const std::filesystem::path& package_root,
    const std::filesystem::path& content_root,
    std::vector<std::string>& warnings) {
    RuntimeCompanionCopyResult result;
    const std::filesystem::path source = copperfin::platform::path_from_utf8_string(asset.source_path);
    const std::filesystem::path staged_relative = copperfin::platform::path_from_utf8_string(
        asset.relative_path);
    for (const auto& companion_source : infer_companion_source_paths(source)) {
        bool ambiguous = false;
        const auto resolved_companion_source = resolve_existing_path_casefold(companion_source, ambiguous);
        if (ambiguous) {
            result.ok = false;
            result.error = runtime_text(
                "Runtime.Package.Error.AmbiguousCompanionPath",
                {{"path", copperfin::platform::path_to_utf8_string(companion_source)}});
            return result;
        }
        if (!resolved_companion_source.has_value()) {
            if (asset.required_for_runtime && is_xasset_path(asset.source_path)) {
                result.ok = false;
                result.error = runtime_text(
                    "Runtime.Package.Error.SourceFileMissing",
                {{"path", copperfin::platform::path_to_utf8_string(companion_source)}});
                return result;
            }
            continue;
        }

        const auto companion_relative =
            staged_relative.parent_path() / resolved_companion_source->filename();
        std::filesystem::path companion_destination;
        std::string error;
        if (!copy_file_to_package_content(
                *resolved_companion_source,
                package_root,
                content_root,
                companion_relative,
                companion_destination,
                error)) {
            if (asset.required_for_runtime) {
                result.ok = false;
                result.error = error;
                return result;
            }
            warnings.push_back(error);
        } else {
            result.copied_paths.push_back(companion_destination);
        }
    }
    return result;
}

}  // namespace runtime_pipeline_detail

}  // namespace copperfin::runtime
