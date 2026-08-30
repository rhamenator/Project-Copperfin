// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/workspace_agent_target_containment.h"

#include <string_view>
#include <system_error>
#include <utility>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/stat.h>
#endif

namespace copperfin::security {
namespace {

WorkspaceAgentFileTargetInspection denied(std::string diagnostic_code) {
    return {
        .allowed = false,
        .canonical_path = {},
        .identity = {},
        .diagnostic_code = std::move(diagnostic_code)
    };
}

bool path_has_embedded_nul(const std::filesystem::path& path) {
    const auto& native = path.native();
    return native.find(typename std::filesystem::path::value_type{}) !=
        std::filesystem::path::string_type::npos;
}

bool path_has_dot_component(const std::filesystem::path& path) {
    for (const auto& component : path) {
        if (component == "." || component == "..") {
            return true;
        }
    }
    return false;
}

#if defined(_WIN32)
// Win32's CreateFileW/GetFullPathNameW silently strip a trailing '.' or ' '
// from each path component before resolving it, so "tool." and "tool " name
// the same object as "tool". A component-boundary check alone cannot reject
// this: it must inspect each component's own last character.
bool path_has_windows_alias_prone_component(const std::filesystem::path& path) {
    for (const auto& component : path) {
        const auto& native = component.native();
        if (!native.empty() &&
            (native.back() == L'.' || native.back() == L' ')) {
            return true;
        }
    }
    return false;
}

bool path_component_is_reserved_windows_device_name(
    const std::filesystem::path::string_type& component) {
    const auto dot = component.find(L'.');
    const std::filesystem::path::string_type stem =
        (dot == std::filesystem::path::string_type::npos)
            ? component
            : component.substr(0U, dot);
    if (stem.empty() || stem.size() > 4U) {
        return false;
    }
    std::filesystem::path::string_type upper;
    upper.reserve(stem.size());
    for (const wchar_t ch : stem) {
        upper.push_back(
            (ch >= L'a' && ch <= L'z') ? static_cast<wchar_t>(ch - (L'a' - L'A')) : ch);
    }
    static constexpr std::wstring_view reserved_names[] = {
        L"CON", L"PRN", L"AUX", L"NUL",
        L"COM1", L"COM2", L"COM3", L"COM4", L"COM5", L"COM6", L"COM7", L"COM8", L"COM9",
        L"LPT1", L"LPT2", L"LPT3", L"LPT4", L"LPT5", L"LPT6", L"LPT7", L"LPT8", L"LPT9"
    };
    for (const std::wstring_view name : reserved_names) {
        if (upper == name) {
            return true;
        }
    }
    return false;
}

// Windows reserves these names as device objects at every path component,
// not just the final one, and regardless of any extension: CreateFileW on
// "NUL.txt" opens the NUL device rather than creating a file with that
// name. Matching is case-insensitive against the portion of the component
// before its first '.', mirroring how Windows itself resolves the name.
bool path_has_reserved_windows_device_name_component(
    const std::filesystem::path& path) {
    for (const auto& component : path) {
        if (path_component_is_reserved_windows_device_name(component.native())) {
            return true;
        }
    }
    return false;
}
#else
bool path_has_windows_alias_prone_component(const std::filesystem::path&) {
    return false;
}

bool path_has_reserved_windows_device_name_component(const std::filesystem::path&) {
    return false;
}
#endif

bool strict_relative_file_path(const std::filesystem::path& path) {
    return !path.empty() && !path_has_embedded_nul(path) &&
        !path.is_absolute() && !path.has_root_name() &&
        !path.has_root_directory() && !path_has_dot_component(path) &&
        !path.filename().empty() &&
        !path_has_windows_alias_prone_component(path) &&
        !path_has_reserved_windows_device_name_component(path);
}

bool strict_absolute_file_path(const std::filesystem::path& path) {
    return !path.empty() && !path_has_embedded_nul(path) &&
        path.is_absolute() && !path_has_dot_component(path) &&
        !path.filename().empty() &&
        !path_has_windows_alias_prone_component(path) &&
        !path_has_reserved_windows_device_name_component(path);
}

bool path_is_direct_directory(const std::filesystem::path& path) {
#if defined(_WIN32)
    const DWORD attributes = ::GetFileAttributesW(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES &&
        (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0U &&
        (attributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0U;
#else
    struct stat status{};
    return ::lstat(path.c_str(), &status) == 0 && S_ISDIR(status.st_mode) &&
        !S_ISLNK(status.st_mode);
#endif
}

std::string diagnostic_for_failure(PhysicalPathContainmentFailure failure) {
    switch (failure) {
        case PhysicalPathContainmentFailure::outside_root:
            return "workspace_agent.target_outside_workspace";
        case PhysicalPathContainmentFailure::indirect_component:
            return "workspace_agent.target_indirect_component";
        case PhysicalPathContainmentFailure::cross_device_component:
            return "workspace_agent.target_cross_device_component";
        case PhysicalPathContainmentFailure::not_regular_file:
            return "workspace_agent.target_not_regular_file";
        case PhysicalPathContainmentFailure::identity_changed:
            return "workspace_agent.target_identity_changed";
        case PhysicalPathContainmentFailure::root_unavailable:
            return "workspace_agent.target_root_unavailable";
        case PhysicalPathContainmentFailure::none:
        case PhysicalPathContainmentFailure::path_unavailable:
        case PhysicalPathContainmentFailure::size_limit_exceeded:
        case PhysicalPathContainmentFailure::read_failed:
            return "workspace_agent.target_unavailable";
    }
    return "workspace_agent.target_unavailable";
}

WorkspaceAgentFileTargetInspection inspect_existing_regular_file(
    const std::filesystem::path& path,
    const std::filesystem::path& containment_root) {
    const auto containment = inspect_physical_path_containment(path, containment_root);
    if (!containment.allowed) {
        return denied(diagnostic_for_failure(containment.failure));
    }

    std::error_code filesystem_error;
    const bool regular = std::filesystem::is_regular_file(
        containment.canonical_path, filesystem_error);
    if (filesystem_error || !regular) {
        return denied("workspace_agent.target_not_regular_file");
    }
    if (containment.identity.link_count != 1U) {
        return denied("workspace_agent.target_multiply_linked");
    }
    return {
        .allowed = true,
        .canonical_path = containment.canonical_path,
        .identity = containment.identity,
        .diagnostic_code = "workspace_agent.target_allowed"
    };
}

}  // namespace

WorkspaceAgentFileTargetBoundary::WorkspaceAgentFileTargetBoundary(
    std::filesystem::path canonical_workspace_root,
    std::uint64_t workspace_storage_id,
    std::uint64_t workspace_file_id)
    : canonical_workspace_root_(std::move(canonical_workspace_root)),
      workspace_storage_id_(workspace_storage_id),
      workspace_file_id_(workspace_file_id) {}

std::optional<WorkspaceAgentFileTargetBoundary>
WorkspaceAgentFileTargetBoundary::create(
    const std::filesystem::path& trusted_absolute_workspace_root) {
    if (trusted_absolute_workspace_root.empty() ||
        path_has_embedded_nul(trusted_absolute_workspace_root) ||
        !trusted_absolute_workspace_root.is_absolute() ||
        path_has_dot_component(trusted_absolute_workspace_root) ||
        path_has_windows_alias_prone_component(trusted_absolute_workspace_root) ||
        path_has_reserved_windows_device_name_component(trusted_absolute_workspace_root) ||
        !path_is_direct_directory(trusted_absolute_workspace_root)) {
        return std::nullopt;
    }
    const auto containment = inspect_physical_path_containment(
        trusted_absolute_workspace_root, trusted_absolute_workspace_root);
    if (!containment.allowed) {
        return std::nullopt;
    }
    std::error_code filesystem_error;
    if (!std::filesystem::is_directory(
            containment.canonical_path, filesystem_error) ||
        filesystem_error) {
        return std::nullopt;
    }
    return WorkspaceAgentFileTargetBoundary(
        containment.canonical_path,
        containment.identity.storage_id,
        containment.identity.file_id);
}

bool WorkspaceAgentFileTargetBoundary::workspace_root_identity_matches() const {
    if (!path_is_direct_directory(canonical_workspace_root_)) {
        return false;
    }
    const auto current = inspect_physical_path_containment(
        canonical_workspace_root_, canonical_workspace_root_);
    return current.allowed &&
        current.identity.storage_id == workspace_storage_id_ &&
        current.identity.file_id == workspace_file_id_;
}

WorkspaceAgentFileTargetInspection
WorkspaceAgentFileTargetBoundary::inspect_workspace_file(
    const std::filesystem::path& strict_relative_target) const {
    if (!strict_relative_file_path(strict_relative_target)) {
        return denied("workspace_agent.target_invalid_relative_path");
    }
    if (!workspace_root_identity_matches()) {
        return denied("workspace_agent.target_root_identity_changed");
    }
    auto result = inspect_existing_regular_file(
        canonical_workspace_root_ / strict_relative_target,
        canonical_workspace_root_);
    if (result.allowed && !workspace_root_identity_matches()) {
        return denied("workspace_agent.target_root_identity_changed");
    }
    return result;
}

WorkspaceAgentFileTargetInspection
WorkspaceAgentFileTargetBoundary::inspect_local_file(
    const std::filesystem::path& strict_absolute_target) const {
    if (!strict_absolute_file_path(strict_absolute_target)) {
        return denied("workspace_agent.target_invalid_absolute_path");
    }
    const std::filesystem::path parent = strict_absolute_target.parent_path();
    if (parent.empty()) {
        return denied("workspace_agent.target_invalid_absolute_path");
    }
    return inspect_existing_regular_file(strict_absolute_target, parent);
}

WorkspaceAgentFileTargetSnapshot
WorkspaceAgentFileTargetBoundary::snapshot_workspace_file(
    const WorkspaceAgentFileTargetInspection& expected,
    const std::uint64_t maximum_bytes) const {
    if (!expected.allowed || expected.canonical_path.empty() ||
        !workspace_root_identity_matches()) {
        return {false, {}, {}, "workspace_agent.target_root_identity_changed"};
    }
    const PhysicalPathContainmentResult expected_containment{
        .allowed = true,
        .canonical_path = expected.canonical_path,
        .identity = expected.identity,
        .failure = PhysicalPathContainmentFailure::none};
    const auto snapshot = read_physically_contained_file_snapshot(
        expected_containment, canonical_workspace_root_, maximum_bytes);
    if (!snapshot.ok) {
        switch (snapshot.failure) {
            case PhysicalPathContainmentFailure::size_limit_exceeded:
                return {false, {}, {}, "workspace_agent.file_read_size_limit_exceeded"};
            case PhysicalPathContainmentFailure::identity_changed:
                return {false, {}, {}, "workspace_agent.file_read_identity_changed"};
            case PhysicalPathContainmentFailure::read_failed:
                return {false, {}, {}, "workspace_agent.file_read_failed"};
            default:
                return {false, {}, {}, "workspace_agent.file_read_target_unavailable"};
        }
    }
    if (!workspace_root_identity_matches()) {
        return {false, {}, {}, "workspace_agent.target_root_identity_changed"};
    }
    return {
        .captured = true,
        .bytes = snapshot.bytes,
        .identity = snapshot.containment.identity,
        .diagnostic_code = "workspace_agent.file_read_captured"};
}

}  // namespace copperfin::security
