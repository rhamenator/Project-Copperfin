// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "studio_host_main_support.h"

#include <array>

namespace cf_studio_host_main_detail {
namespace {

using copperfin::security::WorkspaceAgentAccessMode;
using copperfin::security::WorkspaceAgentActivationDecision;
using copperfin::security::WorkspaceAgentActivationRequest;
using copperfin::security::WorkspaceAgentCapabilities;

constexpr std::array<WorkspaceAgentAccessMode, 3> access_modes{
    WorkspaceAgentAccessMode::advisory,
    WorkspaceAgentAccessMode::workspace_sandbox,
    WorkspaceAgentAccessMode::unrestricted_local};

WorkspaceAgentActivationDecision describe_mode(
    WorkspaceAgentAccessMode mode,
    const copperfin::localization::LocalizedCatalog& catalog) {
    return copperfin::security::evaluate_workspace_agent_activation(
        WorkspaceAgentActivationRequest{
            .requested_mode = mode,
            .feature_enabled = true,
            .permission_granted = true,
            .trusted_product_ui = true,
            .audit_sink_available = true,
            .warning_presented = mode == WorkspaceAgentAccessMode::unrestricted_local,
            .warning_id = mode == WorkspaceAgentAccessMode::unrestricted_local
                ? copperfin::security::workspace_agent_unrestricted_warning_id
                : "",
            .user_confirmed = mode == WorkspaceAgentAccessMode::unrestricted_local},
        catalog);
}

void print_json_bool(bool value) {
    std::cout << (value ? "true" : "false");
}

void print_json_capabilities(const WorkspaceAgentCapabilities& capabilities) {
    std::cout << "{\n";
    std::cout << "        \"readWorkspaceFiles\": ";
    print_json_bool(capabilities.read_workspace_files);
    std::cout << ",\n        \"writeWorkspaceFiles\": ";
    print_json_bool(capabilities.write_workspace_files);
    std::cout << ",\n        \"runLocalProcesses\": ";
    print_json_bool(capabilities.run_local_processes);
    std::cout << ",\n        \"accessOutsideWorkspace\": ";
    print_json_bool(capabilities.access_outside_workspace);
    std::cout << ",\n        \"useNetwork\": ";
    print_json_bool(capabilities.use_network);
    std::cout << ",\n        \"elevatePrivileges\": ";
    print_json_bool(capabilities.elevate_privileges);
    std::cout << "\n      }";
}

void print_workspace_agent_policy_json(
    const copperfin::localization::LocalizedCatalog& catalog) {
    const auto warning = copperfin::security::unrestricted_workspace_agent_warning(catalog);
    std::cout << "{\n  \"schemaVersion\": 1,\n  \"status\": \"ok\",\n";
    std::cout << "  \"descriptorOnly\": true,\n";
    std::cout << "  \"activationAvailable\": false,\n";
    std::cout << "  \"defaultMode\": \"advisory\",\n";
    std::cout << "  \"featureEnabledByDefault\": false,\n";
    std::cout << "  \"providerAuthenticationGrantsLocalAuthority\": false,\n";
    std::cout << "  \"activation\": {\n";
    std::cout << "    \"permissionId\": \"ai.workspace_agent\",\n";
    std::cout << "    \"trustedProductUiRequired\": true,\n";
    std::cout << "    \"auditSinkRequired\": true,\n";
    std::cout << "    \"unrestrictedWarningRequired\": true,\n";
    std::cout << "    \"privilegeElevationAllowed\": false\n  },\n";
    std::cout << "  \"unrestrictedWarning\": {\n    \"id\": ";
    print_json_string(warning.id);
    std::cout << ",\n    \"title\": ";
    print_json_string(warning.title);
    std::cout << ",\n    \"body\": ";
    print_json_string(warning.body);
    std::cout << ",\n    \"acknowledgement\": ";
    print_json_string(warning.acknowledgement);
    std::cout << "\n  },\n  \"modes\": [\n";
    for (std::size_t index = 0; index < access_modes.size(); ++index) {
        const auto mode = access_modes[index];
        const auto decision = describe_mode(mode, catalog);
        std::cout << "    {\n      \"name\": ";
        print_json_string_view(copperfin::security::workspace_agent_access_mode_name(mode));
        std::cout << ",\n      \"capabilities\": ";
        print_json_capabilities(decision.capabilities);
        std::cout << "\n    }" << (index + 1U == access_modes.size() ? "" : ",") << "\n";
    }
    std::cout << "  ]\n}\n";
}

void print_text_capabilities(const WorkspaceAgentCapabilities& capabilities) {
    std::cout
        << " read_workspace_files=" << capabilities.read_workspace_files
        << " write_workspace_files=" << capabilities.write_workspace_files
        << " run_local_processes=" << capabilities.run_local_processes
        << " access_outside_workspace=" << capabilities.access_outside_workspace
        << " use_network=" << capabilities.use_network
        << " elevate_privileges=" << capabilities.elevate_privileges << "\n";
}

void print_workspace_agent_policy_text(
    const copperfin::localization::LocalizedCatalog& catalog) {
    const auto warning = copperfin::security::unrestricted_workspace_agent_warning(catalog);
    std::cout << "schema_version: 1\nstatus: ok\ndefault_mode: advisory\n";
    std::cout << "descriptor_only: true\nactivation_available: false\n";
    std::cout << "feature_enabled_by_default: false\n";
    std::cout << "provider_authentication_grants_local_authority: false\n";
    std::cout << "activation.permission_id: ai.workspace_agent\n";
    std::cout << "activation.trusted_product_ui_required: true\n";
    std::cout << "activation.audit_sink_required: true\n";
    std::cout << "activation.unrestricted_warning_required: true\n";
    std::cout << "activation.privilege_elevation_allowed: false\n";
    std::cout << "unrestricted_warning.id: " << warning.id << "\n";
    std::cout << "unrestricted_warning.title: " << warning.title << "\n";
    std::cout << "unrestricted_warning.body: " << warning.body << "\n";
    std::cout << "unrestricted_warning.acknowledgement: " << warning.acknowledgement << "\n";
    for (const auto mode : access_modes) {
        const auto decision = describe_mode(mode, catalog);
        std::cout << "mode." << copperfin::security::workspace_agent_access_mode_name(mode) << ":";
        print_text_capabilities(decision.capabilities);
    }
}

bool valid_policy_arguments(const std::vector<std::string>& args) {
    return (args.size() == 1U && args.front() == "--workspace-agent-policy") ||
        (args.size() == 2U && args.front() == "--workspace-agent-policy" && args[1] == "--json");
}

}  // namespace

std::optional<int> try_handle_workspace_agent_policy(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    if (std::find(args.begin(), args.end(), "--workspace-agent-policy") == args.end()) {
        return std::nullopt;
    }
    if (!valid_policy_arguments(args)) {
        std::cerr << "workspace-agent-policy accepts only an optional trailing --json switch\n";
        return 2;
    }
    if (args.size() == 2U) {
        print_workspace_agent_policy_json(catalog);
    } else {
        print_workspace_agent_policy_text(catalog);
    }
    return 0;
}

}  // namespace cf_studio_host_main_detail
