// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace copperfin::localization {
struct LocalizedCatalog;
}

namespace copperfin::security {

// Governing requirement: RQ-CF-AGENT-001 in the durable traceability matrix.

inline constexpr const char* workspace_agent_unrestricted_warning_id =
    "workspace-agent.unrestricted-local.v1";

enum class WorkspaceAgentAccessMode {
    advisory,
    workspace_sandbox,
    unrestricted_local
};

[[nodiscard]] std::string_view workspace_agent_access_mode_name(
    WorkspaceAgentAccessMode mode) noexcept;
[[nodiscard]] std::optional<WorkspaceAgentAccessMode> parse_workspace_agent_access_mode(
    std::string_view value) noexcept;

struct WorkspaceAgentCapabilities {
    bool read_workspace_files = false;
    bool write_workspace_files = false;
    bool run_local_processes = false;
    bool access_outside_workspace = false;
    bool use_network = false;
    bool elevate_privileges = false;
};

struct WorkspaceAgentWarningDialog {
    std::string id;
    std::string title;
    std::string body;
    std::string acknowledgement;
};

struct WorkspaceAgentActivationRequest {
    WorkspaceAgentAccessMode requested_mode = WorkspaceAgentAccessMode::advisory;
    bool feature_enabled = false;
    bool permission_granted = false;
    bool trusted_product_ui = false;
    bool audit_sink_available = false;
    bool warning_presented = false;
    std::string warning_id;
    bool user_confirmed = false;
};

struct WorkspaceAgentActivationDecision {
    bool allowed = false;
    WorkspaceAgentAccessMode effective_mode = WorkspaceAgentAccessMode::advisory;
    WorkspaceAgentCapabilities capabilities{};
    std::string diagnostic_code;
    std::string message;
    bool audit_required = true;
    bool warning_required = false;
};

[[nodiscard]] WorkspaceAgentWarningDialog unrestricted_workspace_agent_warning();
[[nodiscard]] WorkspaceAgentWarningDialog unrestricted_workspace_agent_warning(
    const localization::LocalizedCatalog& catalog);
[[nodiscard]] WorkspaceAgentActivationDecision evaluate_workspace_agent_activation(
    const WorkspaceAgentActivationRequest& request);
[[nodiscard]] WorkspaceAgentActivationDecision evaluate_workspace_agent_activation(
    const WorkspaceAgentActivationRequest& request,
    const localization::LocalizedCatalog& catalog);

}  // namespace copperfin::security
