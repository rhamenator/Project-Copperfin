// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/security/physical_path_containment.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

namespace copperfin::security {

// Governing requirements: RQ-CF-AGENT-010 and candidate RQ-CF-AGENT-023.

struct WorkspaceAgentProcessTargetPinAuthority;
struct WorkspaceAgentProcessTargetBoundaryAuthority;

struct WorkspaceAgentProcessTargetInspection {
    bool allowed = false;
    std::filesystem::path canonical_executable_path;
    PhysicalPathIdentity executable_identity{};
    std::filesystem::path canonical_working_directory;
    PhysicalPathIdentity working_directory_identity{};
    std::string diagnostic_code;

private:
    std::shared_ptr<WorkspaceAgentProcessTargetPinAuthority> pin_authority_;

    friend class WorkspaceAgentProcessTargetBoundary;
};

// Move-only ownership of the exact workspace root, executable, and working
// directory objects opened by the process-target boundary. It exposes no
// native handle, path, or launch operation. Retention alone does not prove
// executable-content immutability or authorize process creation.
class WorkspaceAgentProcessTargetPins {
public:
    WorkspaceAgentProcessTargetPins() = default;
    ~WorkspaceAgentProcessTargetPins();
    WorkspaceAgentProcessTargetPins(WorkspaceAgentProcessTargetPins&&) noexcept;
    WorkspaceAgentProcessTargetPins& operator=(
        WorkspaceAgentProcessTargetPins&&) noexcept;
    WorkspaceAgentProcessTargetPins(
        const WorkspaceAgentProcessTargetPins&) = delete;
    WorkspaceAgentProcessTargetPins& operator=(
        const WorkspaceAgentProcessTargetPins&) = delete;

    [[nodiscard]] bool valid() const noexcept;

private:
    class Impl;
    explicit WorkspaceAgentProcessTargetPins(std::unique_ptr<Impl> impl) noexcept;

    std::unique_ptr<Impl> impl_;

    friend class WorkspaceAgentProcessTargetBoundary;
};

struct WorkspaceAgentProcessTargetPinResult {
    bool pinned = false;
    std::optional<WorkspaceAgentProcessTargetPins> pins;
    std::string diagnostic_code;
};

// Product code creates this boundary from its trusted absolute workspace root.
// Provider, model, prompt, and workspace content supply only explicit target
// paths. This boundary does not search PATH, interpret a command, or launch.
class WorkspaceAgentProcessTargetBoundary {
public:
    WorkspaceAgentProcessTargetBoundary(
        const WorkspaceAgentProcessTargetBoundary&) = delete;
    WorkspaceAgentProcessTargetBoundary& operator=(
        const WorkspaceAgentProcessTargetBoundary&) = delete;
    WorkspaceAgentProcessTargetBoundary(
        WorkspaceAgentProcessTargetBoundary&&) noexcept = default;
    WorkspaceAgentProcessTargetBoundary& operator=(
        WorkspaceAgentProcessTargetBoundary&&) noexcept = default;

    [[nodiscard]] static std::optional<WorkspaceAgentProcessTargetBoundary> create(
        const std::filesystem::path& trusted_absolute_workspace_root);

    [[nodiscard]] WorkspaceAgentProcessTargetInspection inspect_workspace_process(
        const std::filesystem::path& strict_relative_executable,
        const std::filesystem::path& strict_relative_working_directory) const;
    [[nodiscard]] WorkspaceAgentProcessTargetInspection inspect_local_process(
        const std::filesystem::path& strict_absolute_executable,
        const std::filesystem::path& strict_absolute_working_directory) const;

    // Consumes the one-attempt private authority attached to an inspection
    // issued by this exact boundary and opens all three exact objects. Stale,
    // forged, edited, replayed, or cross-boundary inspections fail closed.
    // The returned pins do not launch and expose no native handles.
    [[nodiscard]] WorkspaceAgentProcessTargetPinResult pin_process_targets(
        const WorkspaceAgentProcessTargetInspection& inspection) const;

private:
    WorkspaceAgentProcessTargetBoundary(
        std::filesystem::path canonical_workspace_root,
        std::uint64_t workspace_storage_id,
        std::uint64_t workspace_file_id);

    [[nodiscard]] bool workspace_root_identity_matches() const;

    std::filesystem::path canonical_workspace_root_;
    std::uint64_t workspace_storage_id_ = 0U;
    std::uint64_t workspace_file_id_ = 0U;
    std::shared_ptr<const WorkspaceAgentProcessTargetBoundaryAuthority>
        pin_boundary_authority_;
};

}  // namespace copperfin::security
