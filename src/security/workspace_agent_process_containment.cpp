// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/workspace_agent_process_containment.h"

#include <system_error>
#include <utility>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace copperfin::security {
namespace {

WorkspaceAgentProcessTargetInspection denied(std::string diagnostic_code) {
    return {
        .allowed = false,
        .canonical_executable_path = {},
        .executable_identity = {},
        .canonical_working_directory = {},
        .working_directory_identity = {},
        .diagnostic_code = std::move(diagnostic_code)};
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
bool path_has_windows_device_or_stream_syntax(const std::filesystem::path& path) {
    const auto& native = path.native();
    if (native.rfind(L"\\\\?\\", 0U) == 0U ||
        native.rfind(L"\\\\.\\", 0U) == 0U ||
        native.rfind(L"\\??\\", 0U) == 0U) {
        return true;
    }
    const auto root_name = path.root_name();
    for (const auto& component : path.relative_path()) {
        if (component.native().find(L':') != std::wstring::npos) {
            return true;
        }
    }
    return root_name.empty() && native.find(L':') != std::wstring::npos;
}
#else
bool path_has_windows_device_or_stream_syntax(const std::filesystem::path&) {
    return false;
}
#endif

bool strict_relative_executable_path(const std::filesystem::path& path) {
    return !path.empty() && !path_has_embedded_nul(path) &&
        !path.is_absolute() && !path.has_root_name() &&
        !path.has_root_directory() && !path_has_dot_component(path) &&
        !path.filename().empty() &&
        !path_has_windows_device_or_stream_syntax(path);
}

bool strict_relative_working_directory_path(const std::filesystem::path& path) {
    if (path == ".") {
        return true;
    }
    return !path.empty() && !path_has_embedded_nul(path) &&
        !path.is_absolute() && !path.has_root_name() &&
        !path.has_root_directory() && !path_has_dot_component(path) &&
        !path.filename().empty() &&
        !path_has_windows_device_or_stream_syntax(path);
}

bool strict_absolute_executable_path(const std::filesystem::path& path) {
    return !path.empty() && !path_has_embedded_nul(path) && path.is_absolute() &&
        !path_has_dot_component(path) && !path.filename().empty() &&
        !path_has_windows_device_or_stream_syntax(path);
}

bool strict_absolute_working_directory_path(const std::filesystem::path& path) {
    return !path.empty() && !path_has_embedded_nul(path) && path.is_absolute() &&
        !path_has_dot_component(path) &&
        !path_has_windows_device_or_stream_syntax(path);
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

bool executable_is_eligible(const std::filesystem::path& path) {
    std::error_code filesystem_error;
    if (!std::filesystem::is_regular_file(path, filesystem_error) ||
        filesystem_error) {
        return false;
    }
#if defined(_WIN32)
    return true;
#else
    return ::access(path.c_str(), X_OK) == 0;
#endif
}

std::string executable_diagnostic_for_failure(
    PhysicalPathContainmentFailure failure) {
    switch (failure) {
        case PhysicalPathContainmentFailure::outside_root:
            return "workspace_agent.process_executable_outside_workspace";
        case PhysicalPathContainmentFailure::indirect_component:
            return "workspace_agent.process_executable_indirect_component";
        case PhysicalPathContainmentFailure::cross_device_component:
            return "workspace_agent.process_executable_cross_device_component";
        case PhysicalPathContainmentFailure::root_unavailable:
            return "workspace_agent.process_workspace_root_unavailable";
        case PhysicalPathContainmentFailure::none:
        case PhysicalPathContainmentFailure::path_unavailable:
        case PhysicalPathContainmentFailure::identity_changed:
        case PhysicalPathContainmentFailure::not_regular_file:
        case PhysicalPathContainmentFailure::size_limit_exceeded:
        case PhysicalPathContainmentFailure::read_failed:
            return "workspace_agent.process_executable_unavailable";
    }
    return "workspace_agent.process_executable_unavailable";
}

std::string working_directory_diagnostic_for_failure(
    PhysicalPathContainmentFailure failure) {
    switch (failure) {
        case PhysicalPathContainmentFailure::outside_root:
            return "workspace_agent.process_working_directory_outside_workspace";
        case PhysicalPathContainmentFailure::indirect_component:
            return "workspace_agent.process_working_directory_indirect_component";
        case PhysicalPathContainmentFailure::cross_device_component:
            return "workspace_agent.process_working_directory_cross_device_component";
        case PhysicalPathContainmentFailure::root_unavailable:
            return "workspace_agent.process_workspace_root_unavailable";
        case PhysicalPathContainmentFailure::none:
        case PhysicalPathContainmentFailure::path_unavailable:
        case PhysicalPathContainmentFailure::identity_changed:
        case PhysicalPathContainmentFailure::not_regular_file:
        case PhysicalPathContainmentFailure::size_limit_exceeded:
        case PhysicalPathContainmentFailure::read_failed:
            return "workspace_agent.process_working_directory_unavailable";
    }
    return "workspace_agent.process_working_directory_unavailable";
}

std::optional<PhysicalPathContainmentResult> inspect_executable(
    const std::filesystem::path& path,
    const std::filesystem::path& containment_root,
    WorkspaceAgentProcessTargetInspection& failure_result) {
    const auto containment = inspect_physical_path_containment(path, containment_root);
    if (!containment.allowed) {
        failure_result = denied(executable_diagnostic_for_failure(containment.failure));
        return std::nullopt;
    }
    if (!executable_is_eligible(containment.canonical_path)) {
        failure_result = denied("workspace_agent.process_executable_not_eligible");
        return std::nullopt;
    }
    if (containment.identity.link_count != 1U) {
        failure_result = denied("workspace_agent.process_executable_multiply_linked");
        return std::nullopt;
    }
    return containment;
}

std::optional<PhysicalPathContainmentResult> inspect_working_directory(
    const std::filesystem::path& path,
    const std::filesystem::path& containment_root,
    WorkspaceAgentProcessTargetInspection& failure_result) {
    const auto containment = inspect_physical_path_containment(path, containment_root);
    if (!containment.allowed) {
        failure_result = denied(
            working_directory_diagnostic_for_failure(containment.failure));
        return std::nullopt;
    }
    std::error_code filesystem_error;
    if (!std::filesystem::is_directory(
            containment.canonical_path, filesystem_error) ||
        filesystem_error) {
        failure_result = denied(
            "workspace_agent.process_working_directory_not_directory");
        return std::nullopt;
    }
    return containment;
}

WorkspaceAgentProcessTargetInspection allowed(
    const PhysicalPathContainmentResult& executable,
    const PhysicalPathContainmentResult& working_directory) {
    return {
        .allowed = true,
        .canonical_executable_path = executable.canonical_path,
        .executable_identity = executable.identity,
        .canonical_working_directory = working_directory.canonical_path,
        .working_directory_identity = working_directory.identity,
        .diagnostic_code = "workspace_agent.process_target_allowed"};
}

}  // namespace

WorkspaceAgentProcessTargetBoundary::WorkspaceAgentProcessTargetBoundary(
    std::filesystem::path canonical_workspace_root,
    std::uint64_t workspace_storage_id,
    std::uint64_t workspace_file_id)
    : canonical_workspace_root_(std::move(canonical_workspace_root)),
      workspace_storage_id_(workspace_storage_id),
      workspace_file_id_(workspace_file_id) {}

std::optional<WorkspaceAgentProcessTargetBoundary>
WorkspaceAgentProcessTargetBoundary::create(
    const std::filesystem::path& trusted_absolute_workspace_root) {
    if (!strict_absolute_working_directory_path(trusted_absolute_workspace_root) ||
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
    return WorkspaceAgentProcessTargetBoundary(
        containment.canonical_path,
        containment.identity.storage_id,
        containment.identity.file_id);
}

bool WorkspaceAgentProcessTargetBoundary::workspace_root_identity_matches() const {
    if (!path_is_direct_directory(canonical_workspace_root_)) {
        return false;
    }
    const auto current = inspect_physical_path_containment(
        canonical_workspace_root_, canonical_workspace_root_);
    return current.allowed &&
        current.identity.storage_id == workspace_storage_id_ &&
        current.identity.file_id == workspace_file_id_;
}

WorkspaceAgentProcessTargetInspection
WorkspaceAgentProcessTargetBoundary::inspect_workspace_process(
    const std::filesystem::path& strict_relative_executable,
    const std::filesystem::path& strict_relative_working_directory) const {
    if (!strict_relative_executable_path(strict_relative_executable)) {
        return denied("workspace_agent.process_invalid_relative_executable");
    }
    if (!strict_relative_working_directory_path(
            strict_relative_working_directory)) {
        return denied("workspace_agent.process_invalid_relative_working_directory");
    }
    if (!workspace_root_identity_matches()) {
        return denied("workspace_agent.process_workspace_root_identity_changed");
    }

    WorkspaceAgentProcessTargetInspection failure_result;
    const auto executable = inspect_executable(
        canonical_workspace_root_ / strict_relative_executable,
        canonical_workspace_root_,
        failure_result);
    if (!executable.has_value()) {
        return failure_result;
    }
    const auto working_directory = inspect_working_directory(
        strict_relative_working_directory == "."
            ? canonical_workspace_root_
            : canonical_workspace_root_ / strict_relative_working_directory,
        canonical_workspace_root_,
        failure_result);
    if (!working_directory.has_value()) {
        return failure_result;
    }
    if (!workspace_root_identity_matches()) {
        return denied("workspace_agent.process_workspace_root_identity_changed");
    }
    return allowed(*executable, *working_directory);
}

WorkspaceAgentProcessTargetInspection
WorkspaceAgentProcessTargetBoundary::inspect_local_process(
    const std::filesystem::path& strict_absolute_executable,
    const std::filesystem::path& strict_absolute_working_directory) const {
    if (!strict_absolute_executable_path(strict_absolute_executable)) {
        return denied("workspace_agent.process_invalid_absolute_executable");
    }
    if (!strict_absolute_working_directory_path(
            strict_absolute_working_directory)) {
        return denied("workspace_agent.process_invalid_absolute_working_directory");
    }
    const std::filesystem::path executable_parent =
        strict_absolute_executable.parent_path();
    if (executable_parent.empty()) {
        return denied("workspace_agent.process_invalid_absolute_executable");
    }
    if (!path_is_direct_directory(strict_absolute_working_directory)) {
        return denied("workspace_agent.process_working_directory_indirect_component");
    }

    WorkspaceAgentProcessTargetInspection failure_result;
    const auto executable = inspect_executable(
        strict_absolute_executable, executable_parent, failure_result);
    if (!executable.has_value()) {
        return failure_result;
    }
    const auto working_directory = inspect_working_directory(
        strict_absolute_working_directory,
        strict_absolute_working_directory,
        failure_result);
    return working_directory.has_value()
        ? allowed(*executable, *working_directory)
        : failure_result;
}

}  // namespace copperfin::security
