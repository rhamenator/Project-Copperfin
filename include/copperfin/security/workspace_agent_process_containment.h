// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/security/physical_path_containment.h"

#include <filesystem>
#include <optional>
#include <string>

namespace copperfin::security {

// Governing requirement: RQ-CF-AGENT-010.

struct WorkspaceAgentProcessTargetInspection {
    bool allowed = false;
    std::filesystem::path canonical_executable_path;
    PhysicalPathIdentity executable_identity{};
    std::filesystem::path canonical_working_directory;
    PhysicalPathIdentity working_directory_identity{};
    std::string diagnostic_code;
};

// Product code creates this boundary from its trusted absolute workspace root.
// Provider, model, prompt, and workspace content supply only explicit target
// paths. This boundary does not search PATH, interpret a command, or launch.
class WorkspaceAgentProcessTargetBoundary {
public:
    [[nodiscard]] static std::optional<WorkspaceAgentProcessTargetBoundary> create(
        const std::filesystem::path& trusted_absolute_workspace_root);

    [[nodiscard]] WorkspaceAgentProcessTargetInspection inspect_workspace_process(
        const std::filesystem::path& strict_relative_executable,
        const std::filesystem::path& strict_relative_working_directory) const;
    [[nodiscard]] WorkspaceAgentProcessTargetInspection inspect_local_process(
        const std::filesystem::path& strict_absolute_executable,
        const std::filesystem::path& strict_absolute_working_directory) const;

private:
    WorkspaceAgentProcessTargetBoundary(
        std::filesystem::path canonical_workspace_root,
        std::uint64_t workspace_storage_id,
        std::uint64_t workspace_file_id);

    [[nodiscard]] bool workspace_root_identity_matches() const;

    std::filesystem::path canonical_workspace_root_;
    std::uint64_t workspace_storage_id_ = 0U;
    std::uint64_t workspace_file_id_ = 0U;
};

}  // namespace copperfin::security
