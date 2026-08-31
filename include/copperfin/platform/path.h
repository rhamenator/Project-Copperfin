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

// Reports whether any component of path contains an embedded NUL character.
// Used by strict path-spelling checks that must reject an untrusted path
// before it ever reaches a filesystem call, since a NUL truncates most
// native APIs' view of a string.
bool path_has_embedded_nul(const std::filesystem::path& path);

// Reports whether any component of path is exactly "." or "..". Used by
// strict relative-path-spelling checks that must reject traversal or
// self-reference components before a path is joined onto a trusted root.
bool path_has_dot_component(const std::filesystem::path& path);

// Reports whether any component of path ends in a trailing '.' or ' '.
// Win32's CreateFileW/GetFullPathNameW silently strip a trailing '.' or ' '
// from each path component before resolving it, so e.g. "tool." and "tool "
// name the same object as "tool" -- a component-boundary check alone cannot
// reject this, it must inspect each component's own last character. Always
// returns false on non-Windows platforms, where this aliasing does not
// occur.
bool path_has_windows_alias_prone_component(const std::filesystem::path& path);

// Reports whether any component of path is a reserved Windows device name
// (CON, PRN, AUX, NUL, COM1-COM9, LPT1-LPT9), matched case-insensitively
// against the portion of the component before its first '.' or ':' --
// Windows recognizes these names as device objects even when followed by an
// extension ("NUL.txt") or by legacy MS-DOS colon-terminated device syntax
// ("NUL:", "NUL:hidden.txt"), both still honored by Win32 path resolution
// for backward compatibility. CreateFileW on such a component opens the
// device object rather than a regular file or directory with that name.
// Always returns false on non-Windows platforms, where these names have no
// special meaning.
bool path_has_reserved_windows_device_name_component(
    const std::filesystem::path& path);

}  // namespace copperfin::platform
