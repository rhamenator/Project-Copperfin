// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/security/physical_path_containment.h"

#include <filesystem>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

namespace copperfin::security {

// Governing requirements: RQ-CF-AGENT-010 and candidate RQ-CF-AGENT-023 and
// RQ-CF-AGENT-024.

struct WorkspaceAgentProcessTargetPinAuthority;
struct WorkspaceAgentProcessTargetBoundaryAuthority;

inline constexpr std::uint64_t
    workspace_agent_process_max_executable_bytes = 256ULL * 1024ULL * 1024ULL;

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

struct WorkspaceAgentProcessTargetAuthenticationResult {
    bool authenticated = false;
    std::string diagnostic_code;
};

// Move-only ownership of the exact workspace root, executable, and working
// directory objects opened by the process-target boundary plus an immutable
// private executable-byte snapshot. It exposes no bytes, native handle, path,
// or launch operation. Snapshot authentication does not authorize process
// creation.
class WorkspaceAgentProcessTargetPins {
public:
    WorkspaceAgentProcessTargetPins();
    ~WorkspaceAgentProcessTargetPins();
    WorkspaceAgentProcessTargetPins(WorkspaceAgentProcessTargetPins&&) noexcept;
    WorkspaceAgentProcessTargetPins& operator=(
        WorkspaceAgentProcessTargetPins&&) noexcept;
    WorkspaceAgentProcessTargetPins(
        const WorkspaceAgentProcessTargetPins&) = delete;
    WorkspaceAgentProcessTargetPins& operator=(
        const WorkspaceAgentProcessTargetPins&) = delete;

    [[nodiscard]] bool valid() const noexcept;

    // Rehashes the immutable private byte snapshot captured during pin
    // acquisition. No path is reopened and no bytes, digest, native handle, or
    // launch authority are exposed. A future executor must consume exactly this
    // snapshot while holding the separate revocation lease.
    [[nodiscard]] WorkspaceAgentProcessTargetAuthenticationResult
    verify_executable_bytes();

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

    // These point-in-time-only variants avoid executable hashing and cannot
    // later authorize pin acquisition. Session preflight uses them so repeated
    // plan construction does not repeatedly stream a large executable.
    [[nodiscard]] WorkspaceAgentProcessTargetInspection
    preflight_workspace_process(
        const std::filesystem::path& strict_relative_executable,
        const std::filesystem::path& strict_relative_working_directory) const;
    [[nodiscard]] WorkspaceAgentProcessTargetInspection
    preflight_local_process(
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
    [[nodiscard]] WorkspaceAgentProcessTargetInspection
    inspect_workspace_process_impl(
        const std::filesystem::path& strict_relative_executable,
        const std::filesystem::path& strict_relative_working_directory,
        bool authorize_pinning) const;
    [[nodiscard]] WorkspaceAgentProcessTargetInspection
    inspect_local_process_impl(
        const std::filesystem::path& strict_absolute_executable,
        const std::filesystem::path& strict_absolute_working_directory,
        bool authorize_pinning) const;

    std::filesystem::path canonical_workspace_root_;
    std::uint64_t workspace_storage_id_ = 0U;
    std::uint64_t workspace_file_id_ = 0U;
    std::shared_ptr<const WorkspaceAgentProcessTargetBoundaryAuthority>
        pin_boundary_authority_;
};

}  // namespace copperfin::security
