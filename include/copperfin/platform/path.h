// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace copperfin::platform {

std::string path_to_utf8_string(const std::filesystem::path& value);

std::filesystem::path path_from_utf8_string(std::string_view value);

bool path_component_equal_for_platform(
    const std::filesystem::path& left,
    const std::filesystem::path& right);

bool path_equal_case_insensitive(
    const std::filesystem::path& left,
    const std::filesystem::path& right);

// Reports whether an existing filesystem entry carries the Windows Hidden
// attribute. POSIX has no equivalent attribute, so this approximates VFP9's
// Hidden semantics with the traditional leading-dot filename convention.
bool path_is_hidden(const std::filesystem::path& value);

// Reports whether an existing filesystem entry carries the Windows System
// attribute. POSIX has no equivalent and always reports false.
bool path_is_system(const std::filesystem::path& value);

// Convenience predicate for callers that intentionally group Hidden and
// System visibility, such as FILE() and DIRECTORY().
bool path_is_hidden_or_system(const std::filesystem::path& value);

// Returns the filename spelling supplied by Windows' DOS 8.3 short-name
// facility for an existing entry. Hosts without that Windows facility retain
// the entry's original filename spelling.
std::string path_dos_8dot3_filename(const std::filesystem::path& value);

// Returns the volume label for the volume containing an existing path. VFP9
// ADIR(..., "V") is a Windows drive-volume operation; hosts without that
// facility report no label rather than manufacturing one.
std::optional<std::string> path_volume_label(const std::filesystem::path& value);

}  // namespace copperfin::platform
