// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/workspace_agent_policy.h"

#include "copperfin/localization/localization.h"

#include <utility>

namespace copperfin::security {

namespace {

std::string policy_text(
    const localization::LocalizedCatalog& catalog,
    std::string_view key) {
    return catalog.translate(key);
}

WorkspaceAgentActivationDecision deny(
    std::string diagnostic_code,
    std::string message,
    bool warning_required = false) {
    WorkspaceAgentActivationDecision decision;
    decision.effective_mode = WorkspaceAgentAccessMode::advisory;
    decision.diagnostic_code = std::move(diagnostic_code);
    decision.message = std::move(message);
    decision.warning_required = warning_required;
    decision.audit_required = true;
    return decision;
}

WorkspaceAgentActivationDecision allow(
    WorkspaceAgentAccessMode mode,
    WorkspaceAgentCapabilities capabilities,
    std::string diagnostic_code,
    std::string message) {
    WorkspaceAgentActivationDecision decision;
    decision.allowed = true;
    decision.effective_mode = mode;
    decision.capabilities = capabilities;
    decision.diagnostic_code = std::move(diagnostic_code);
    decision.message = std::move(message);
    decision.audit_required = true;
    return decision;
}

localization::LocalizedCatalog selected_catalog() {
    return localization::load_catalogs(
        localization::resolve_catalog_root(),
        localization::select_locale());
}

}  // namespace

std::string_view workspace_agent_access_mode_name(
    WorkspaceAgentAccessMode mode) noexcept {
    switch (mode) {
        case WorkspaceAgentAccessMode::advisory:
            return "advisory";
        case WorkspaceAgentAccessMode::workspace_sandbox:
            return "workspace_sandbox";
        case WorkspaceAgentAccessMode::unrestricted_local:
            return "unrestricted_local";
    }
    return "invalid";
}

std::optional<WorkspaceAgentAccessMode> parse_workspace_agent_access_mode(
    std::string_view value) noexcept {
    if (value == "advisory") {
        return WorkspaceAgentAccessMode::advisory;
    }
    if (value == "workspace_sandbox") {
        return WorkspaceAgentAccessMode::workspace_sandbox;
    }
    if (value == "unrestricted_local") {
        return WorkspaceAgentAccessMode::unrestricted_local;
    }
    return std::nullopt;
}

WorkspaceAgentWarningDialog unrestricted_workspace_agent_warning(
    const localization::LocalizedCatalog& catalog) {
    return WorkspaceAgentWarningDialog{
        .id = workspace_agent_unrestricted_warning_id,
        .title = policy_text(catalog, "Security.WorkspaceAgent.Warning.Unrestricted.Title"),
        .body = policy_text(catalog, "Security.WorkspaceAgent.Warning.Unrestricted.Body"),
        .acknowledgement = policy_text(
            catalog,
            "Security.WorkspaceAgent.Warning.Unrestricted.Acknowledgement")};
}

WorkspaceAgentWarningDialog unrestricted_workspace_agent_warning() {
    return unrestricted_workspace_agent_warning(selected_catalog());
}

WorkspaceAgentActivationDecision evaluate_workspace_agent_activation(
    const WorkspaceAgentActivationRequest& request,
    const localization::LocalizedCatalog& catalog) {
    if (!request.feature_enabled) {
        return deny(
            "workspace_agent.feature_disabled",
            policy_text(catalog, "Security.WorkspaceAgent.Decision.FeatureDisabled"));
    }
    if (!request.permission_granted) {
        return deny(
            "workspace_agent.permission_denied",
            policy_text(catalog, "Security.WorkspaceAgent.Decision.PermissionDenied"));
    }
    if (!request.trusted_product_ui) {
        return deny(
            "workspace_agent.trusted_ui_required",
            policy_text(catalog, "Security.WorkspaceAgent.Decision.TrustedUiRequired"));
    }
    if (!request.audit_sink_available) {
        return deny(
            "workspace_agent.audit_unavailable",
            policy_text(catalog, "Security.WorkspaceAgent.Decision.AuditUnavailable"));
    }

    switch (request.requested_mode) {
        case WorkspaceAgentAccessMode::advisory:
            return allow(
                request.requested_mode,
                {},
                "workspace_agent.advisory_allowed",
                policy_text(catalog, "Security.WorkspaceAgent.Decision.AdvisoryAllowed"));
        case WorkspaceAgentAccessMode::workspace_sandbox:
            return allow(
                request.requested_mode,
                WorkspaceAgentCapabilities{
                    .read_workspace_files = true,
                    .write_workspace_files = true,
                    .run_local_processes = true},
                "workspace_agent.sandbox_allowed",
                policy_text(catalog, "Security.WorkspaceAgent.Decision.SandboxAllowed"));
        case WorkspaceAgentAccessMode::unrestricted_local:
            break;
        default:
            return deny(
                "workspace_agent.invalid_mode",
                policy_text(catalog, "Security.WorkspaceAgent.Decision.InvalidMode"));
    }

    if (!request.warning_presented) {
        return deny(
            "workspace_agent.warning_required",
            policy_text(catalog, "Security.WorkspaceAgent.Decision.WarningRequired"),
            true);
    }
    if (request.warning_id != workspace_agent_unrestricted_warning_id) {
        return deny(
            "workspace_agent.warning_version_mismatch",
            policy_text(catalog, "Security.WorkspaceAgent.Decision.WarningVersionMismatch"),
            true);
    }
    if (!request.user_confirmed) {
        return deny(
            "workspace_agent.confirmation_required",
            policy_text(catalog, "Security.WorkspaceAgent.Decision.ConfirmationRequired"),
            true);
    }

    return allow(
        request.requested_mode,
        WorkspaceAgentCapabilities{
            .read_workspace_files = true,
            .write_workspace_files = true,
            .run_local_processes = true,
            .access_outside_workspace = true,
            .use_network = true,
            .elevate_privileges = false},
        "workspace_agent.unrestricted_allowed",
        policy_text(catalog, "Security.WorkspaceAgent.Decision.UnrestrictedAllowed"));
}

WorkspaceAgentActivationDecision evaluate_workspace_agent_activation(
    const WorkspaceAgentActivationRequest& request) {
    return evaluate_workspace_agent_activation(request, selected_catalog());
}

}  // namespace copperfin::security
