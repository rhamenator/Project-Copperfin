// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/sidecar_path.h"

#include "copperfin/platform/path.h"

#include <string>

namespace copperfin::vfp {
namespace {

std::string lowercase_ascii(std::string value) {
    for (char& ch : value) {
        if (ch >= 'A' && ch <= 'Z') {
            ch = static_cast<char>(ch + ('a' - 'A'));
        }
    }
    return value;
}

}  // namespace

SidecarPathResolution resolve_unique_casefold_path(
    const std::filesystem::path& candidate,
    bool require_regular_file) {
    SidecarPathResolution result{.requested_path = candidate};
    std::error_code ignored;
    const auto admissible = [&](const std::filesystem::path& path) {
        ignored.clear();
        return !require_regular_file || std::filesystem::is_regular_file(path, ignored);
    };

    const bool candidate_exists =
        std::filesystem::exists(candidate, ignored) && admissible(candidate);
    ignored.clear();

    const std::filesystem::path directory = candidate.has_parent_path()
        ? candidate.parent_path()
        : std::filesystem::current_path(ignored);
    if (ignored || directory.empty() || !std::filesystem::is_directory(directory, ignored)) {
        if (candidate_exists) {
            result.path = candidate;
        }
        return result;
    }

    const std::string requested_name = copperfin::platform::path_to_utf8_string(candidate.filename());
    const std::string folded_name = lowercase_ascii(requested_name);
    for (const auto& entry : std::filesystem::directory_iterator(directory, ignored)) {
        if (ignored) {
            break;
        }
        if (!admissible(entry.path())) {
            continue;
        }

        const std::string entry_name = copperfin::platform::path_to_utf8_string(entry.path().filename());
        if (entry_name == requested_name) {
            result.path = entry.path();
            result.ambiguous = false;
            return result;
        }
        if (lowercase_ascii(entry_name) != folded_name) {
            continue;
        }
        if (result.path.has_value()) {
            result.path.reset();
            result.ambiguous = true;
        } else if (!result.ambiguous) {
            result.path = entry.path();
        }
    }

    if (!result.path.has_value() && !result.ambiguous && candidate_exists) {
        result.path = candidate;
    }
    return result;
}

SidecarPathResolution resolve_vfp_sidecar_path(
    const std::filesystem::path& primary_path,
    std::string_view sidecar_extension) {
    std::filesystem::path candidate = primary_path;
    candidate.replace_extension(sidecar_extension);
    return resolve_unique_casefold_path(candidate);
}

SidecarPathResolution resolve_vfp_memo_sidecar_path(
    const std::filesystem::path& primary_path) {
    const std::string extension = lowercase_ascii(
        copperfin::platform::path_to_utf8_string(primary_path.extension()));
    if (extension == ".pjx") {
        return resolve_vfp_sidecar_path(primary_path, ".pjt");
    }
    if (extension == ".scx") {
        return resolve_vfp_sidecar_path(primary_path, ".sct");
    }
    if (extension == ".vcx") {
        return resolve_vfp_sidecar_path(primary_path, ".vct");
    }
    if (extension == ".frx") {
        return resolve_vfp_sidecar_path(primary_path, ".frt");
    }
    if (extension == ".lbx") {
        return resolve_vfp_sidecar_path(primary_path, ".lbt");
    }
    if (extension == ".mnx") {
        return resolve_vfp_sidecar_path(primary_path, ".mnt");
    }
    if (extension == ".dbc") {
        return resolve_vfp_sidecar_path(primary_path, ".dct");
    }
    if (extension == ".dbf") {
        return resolve_vfp_sidecar_path(primary_path, ".fpt");
    }
    return {};
}

}  // namespace copperfin::vfp
