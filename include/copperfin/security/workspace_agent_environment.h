// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/security/physical_path_containment.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace copperfin::security {

// Governing requirements: RQ-CF-AGENT-011 through RQ-CF-AGENT-014.

inline constexpr std::size_t workspace_agent_environment_max_path_directories = 16U;
inline constexpr std::size_t workspace_agent_environment_max_entry_bytes = 4096U;
inline constexpr std::size_t workspace_agent_environment_max_total_bytes = 32768U;

enum class WorkspaceAgentProcessEnvironmentPolicy : std::uint32_t {
    isolated_session_v1 = 1U
};

enum class WorkspaceAgentProcessEnvironmentPlatform : std::uint32_t {
    windows_v1 = 1U,
    posix_v1 = 2U
};

// Converts the logical UTF-8 profile limit into the serializer's storage-unit
// cap. Every entry needs one terminator; a Windows environment block needs one
// additional final terminator. Zero denotes an invalid platform or overflow.
[[nodiscard]] constexpr std::size_t
workspace_agent_serialized_environment_maximum_units(
    const WorkspaceAgentProcessEnvironmentPlatform platform,
    const std::size_t entry_count) noexcept {
    std::size_t final_block_terminators = 0U;
    switch (platform) {
    case WorkspaceAgentProcessEnvironmentPlatform::windows_v1:
        final_block_terminators = 1U;
        break;
    case WorkspaceAgentProcessEnvironmentPlatform::posix_v1:
        break;
    default:
        return 0U;
    }
    if (entry_count > std::numeric_limits<std::size_t>::max() -
            workspace_agent_environment_max_total_bytes - final_block_terminators) {
        return 0U;
    }
    return workspace_agent_environment_max_total_bytes + entry_count +
        final_block_terminators;
}

[[nodiscard]] constexpr bool workspace_agent_process_environment_inherits_parent(
    WorkspaceAgentProcessEnvironmentPolicy) noexcept {
    return false;
}

// Platform selection is implemented behind the public portability seam. Public
// consumers do not select native implementations with compiler macros.
[[nodiscard]] WorkspaceAgentProcessEnvironmentPlatform
workspace_agent_process_environment_host_platform() noexcept;

struct WorkspaceAgentEnvironmentEntry {
    std::string name;
    std::string value;

    bool operator==(const WorkspaceAgentEnvironmentEntry&) const = default;
};

// This is trusted product-host configuration, not provider, model, prompt,
// workspace, or tool-request input. Environment names and arbitrary values are
// deliberately absent. The host must create and access-control the session
// storage layout before constructing an environment.
struct WorkspaceAgentIsolatedEnvironmentConfiguration {
    std::uint32_t schema_version = 1U;
    std::filesystem::path trusted_session_storage_root;
    std::vector<std::filesystem::path> trusted_executable_directories;
    // Required only on Windows. It supplies fixed SystemRoot and WINDIR values;
    // it is not implicitly added to PATH.
    std::filesystem::path trusted_windows_system_root;
};

struct WorkspaceAgentIsolatedEnvironmentConstruction {
    bool allowed = false;
    std::uint64_t session_generation = 0U;
    WorkspaceAgentProcessEnvironmentPolicy policy =
        WorkspaceAgentProcessEnvironmentPolicy::isolated_session_v1;
    WorkspaceAgentProcessEnvironmentPlatform platform =
        workspace_agent_process_environment_host_platform();
    std::vector<WorkspaceAgentEnvironmentEntry> entries;
    std::string diagnostic_code;
};

struct WorkspaceAgentSessionLayoutPreparationResult {
    bool prepared = false;
    std::uint64_t session_generation = 0U;
    std::string diagnostic_code;
};

// The boundary derives only session-<generation>/{home,temp,config,cache,data}
// and product-configured executable directories. Its explicit preparation
// method creates a new generation layout with the platform owner-only security
// contract; construction then requires that contract. It never reads or
// modifies the parent process environment and performs no cleanup, command
// serialization, sandboxing, or launch.
class WorkspaceAgentIsolatedEnvironmentBoundary {
public:
    [[nodiscard]] static std::optional<WorkspaceAgentIsolatedEnvironmentBoundary>
    create(const WorkspaceAgentIsolatedEnvironmentConfiguration& configuration);

    [[nodiscard]] WorkspaceAgentIsolatedEnvironmentConstruction construct(
        std::uint64_t session_generation,
        WorkspaceAgentProcessEnvironmentPolicy policy) const;

    // Creates exactly one previously absent generation layout. Existing or
    // partial layouts are never adopted, repaired, overwritten, or deleted.
    [[nodiscard]] WorkspaceAgentSessionLayoutPreparationResult
    prepare_session_layout(std::uint64_t session_generation) const;

private:
    WorkspaceAgentIsolatedEnvironmentBoundary(
        PhysicalPathContainmentResult session_storage_root,
        std::vector<PhysicalPathContainmentResult> executable_directories,
        std::optional<PhysicalPathContainmentResult> windows_system_root);

    PhysicalPathContainmentResult session_storage_root_;
    std::vector<PhysicalPathContainmentResult> executable_directories_;
    std::optional<PhysicalPathContainmentResult> windows_system_root_;
};

}  // namespace copperfin::security
