// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/security/physical_path_containment.h"

#include <filesystem>
#include <optional>
#include <string>

namespace copperfin::security {

// Governing requirement: RQ-CF-AGENT-009.

struct WorkspaceAgentFileTargetInspection {
    bool allowed = false;
    std::filesystem::path canonical_path;
    PhysicalPathIdentity identity{};
    std::string diagnostic_code;
};

struct WorkspaceAgentFileTargetSnapshot {
    bool captured = false;
    std::string bytes;
    PhysicalPathIdentity identity{};
    std::string diagnostic_code;
};

// Product code creates this boundary from its trusted absolute workspace root.
// Provider, model, prompt, and workspace content supply only target paths.
class WorkspaceAgentFileTargetBoundary {
public:
    [[nodiscard]] static std::optional<WorkspaceAgentFileTargetBoundary> create(
        const std::filesystem::path& trusted_absolute_workspace_root);

    [[nodiscard]] WorkspaceAgentFileTargetInspection inspect_workspace_file(
        const std::filesystem::path& strict_relative_target) const;
    [[nodiscard]] WorkspaceAgentFileTargetInspection inspect_local_file(
        const std::filesystem::path& strict_absolute_target) const;
    // Captures only a workspace file already admitted by this boundary.  The
    // caller supplies the inspected identity; this boundary rechecks its
    // product-owned root before and after the owned byte snapshot.
    [[nodiscard]] WorkspaceAgentFileTargetSnapshot snapshot_workspace_file(
        const WorkspaceAgentFileTargetInspection& expected,
        std::uint64_t maximum_bytes) const;

private:
    WorkspaceAgentFileTargetBoundary(
        std::filesystem::path canonical_workspace_root,
        std::uint64_t workspace_storage_id,
        std::uint64_t workspace_file_id);

    [[nodiscard]] bool workspace_root_identity_matches() const;

    std::filesystem::path canonical_workspace_root_;
    std::uint64_t workspace_storage_id_ = 0U;
    std::uint64_t workspace_file_id_ = 0U;
};

}  // namespace copperfin::security
