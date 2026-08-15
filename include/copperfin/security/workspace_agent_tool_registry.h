// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/security/workspace_agent_policy.h"

#include <span>
#include <string_view>

namespace copperfin::security {

// Governing requirements: RQ-CF-AGENT-007, RQ-CF-AGENT-008, and
// RQ-CF-AGENT-009.

// Reuse the policy capability shape so registered requirements cannot acquire
// a parallel vocabulary that drifts from the admitted session snapshot.
using WorkspaceAgentToolRequirements = WorkspaceAgentCapabilities;

enum class WorkspaceAgentToolTargetKind {
    workspace_file,
    workspace_process,
    local_file,
    local_process,
    network_endpoint
};

struct WorkspaceAgentToolDefinition {
    std::string_view id;
    WorkspaceAgentToolRequirements requirements{};
    WorkspaceAgentToolTargetKind target_kind =
        WorkspaceAgentToolTargetKind::workspace_file;
};

inline constexpr std::string_view workspace_agent_tool_workspace_inspect =
    "workspace.inspect.v1";
inline constexpr std::string_view workspace_agent_tool_workspace_apply_edit =
    "workspace.apply_edit.v1";
inline constexpr std::string_view workspace_agent_tool_workspace_run_process =
    "workspace.run_process.v1";
inline constexpr std::string_view workspace_agent_tool_local_inspect =
    "local.inspect.v1";
inline constexpr std::string_view workspace_agent_tool_local_apply_edit =
    "local.apply_edit.v1";
inline constexpr std::string_view workspace_agent_tool_local_run_process =
    "local.run_process.v1";
inline constexpr std::string_view workspace_agent_tool_network_request =
    "network.request.v1";

// This is the immutable product registry. Provider configuration, workspace
// content, prompts, and model output cannot add or alter definitions.
[[nodiscard]] std::span<const WorkspaceAgentToolDefinition>
workspace_agent_product_tool_definitions() noexcept;

[[nodiscard]] const WorkspaceAgentToolDefinition* find_workspace_agent_product_tool(
    std::string_view id) noexcept;

}  // namespace copperfin::security
