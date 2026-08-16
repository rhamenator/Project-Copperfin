// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/security/physical_path_containment.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace copperfin::security {

// Governing requirement: candidate RQ-CF-AGENT-018.

inline constexpr std::size_t
    workspace_agent_maximum_windows_process_parser_bindings = 64U;
inline constexpr std::uint64_t
    workspace_agent_maximum_windows_process_parser_image_bytes =
        512U * 1024U * 1024U;

enum class WorkspaceAgentProcessArgumentParserContract : std::uint32_t {
    none = 0U,
    posix_argv_v1 = 1U,
    windows_c_runtime_argv_v1 = 2U
};

enum class WorkspaceAgentProcessParserDependencyContract : std::uint32_t {
    none = 0U,
    // Trusted product evidence for this exact image digest establishes that
    // launch and argument parsing depend only on Windows-trusted system images,
    // never an application-local or working-directory DLL. General adjacent-DLL
    // dependency closures are not supported by this slice.
    self_contained_launch_image_v1 = 1U
};

struct WorkspaceAgentWindowsProcessParserBinding {
    std::filesystem::path trusted_absolute_executable;
    // Must be supplied by the trusted host from its product-owned executable
    // admission record. The boundary never manufactures expected identity from
    // whichever file happens to occupy the configured path.
    PhysicalPathIdentity expected_identity{};
    // Lowercase SHA-256 of the trusted product image. Physical identity alone
    // does not authenticate bytes in a mutable file object.
    std::string expected_sha256;
    WorkspaceAgentProcessParserDependencyContract dependency_contract =
        WorkspaceAgentProcessParserDependencyContract::none;
    WorkspaceAgentProcessArgumentParserContract contract =
        WorkspaceAgentProcessArgumentParserContract::windows_c_runtime_argv_v1;
};

// Trusted product-host configuration only. Provider, model, prompt, workspace,
// and tool-request input cannot create or select parser authority.
struct WorkspaceAgentProcessParserConfiguration {
    std::uint32_t schema_version = 1U;
    std::vector<WorkspaceAgentWindowsProcessParserBinding> windows_bindings;
};

struct WorkspaceAgentProcessParserAuthorization {
    bool allowed = false;
    WorkspaceAgentProcessArgumentParserContract contract =
        WorkspaceAgentProcessArgumentParserContract::none;
    std::string diagnostic_code;
};

// Captures exact canonical executable identities for the one Windows command-
// line parser contract Copperfin currently knows how to serialize. It performs
// no PATH search, command interpretation, executable mutation, or launch.
class WorkspaceAgentProcessParserBoundary {
public:
    [[nodiscard]] static std::optional<WorkspaceAgentProcessParserBoundary>
    create(const WorkspaceAgentProcessParserConfiguration& configuration);

    [[nodiscard]] WorkspaceAgentProcessParserAuthorization authorize_windows(
        const std::filesystem::path& canonical_executable,
        const PhysicalPathIdentity& executable_identity) const;

private:
    struct CapturedBinding {
        std::filesystem::path canonical_executable;
        PhysicalPathIdentity identity{};
        std::string sha256;
        WorkspaceAgentProcessArgumentParserContract contract =
            WorkspaceAgentProcessArgumentParserContract::none;
    };

    explicit WorkspaceAgentProcessParserBoundary(
        std::vector<CapturedBinding> bindings);

    std::vector<CapturedBinding> bindings_;
};

}  // namespace copperfin::security
