// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/localization/localization.h"
#include "copperfin/security/authorization.h"
#include "copperfin/security/security_model.h"
#include "copperfin/security/workspace_agent_policy.h"

// Verification of RQ-CF-AGENT-001.

#include <algorithm>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

copperfin::security::WorkspaceAgentActivationRequest base_request(
    copperfin::security::WorkspaceAgentAccessMode mode) {
    return {
        .requested_mode = mode,
        .feature_enabled = true,
        .permission_granted = true,
        .trusted_product_ui = true,
        .audit_sink_available = true,
        .warning_presented = false,
        .warning_id = {},
        .user_confirmed = false};
}

void test_default_modes_are_bounded() {
    using namespace copperfin::security;

    auto advisory = evaluate_workspace_agent_activation(
        base_request(WorkspaceAgentAccessMode::advisory));
    expect(advisory.allowed, "trusted opt-in advisory mode should be allowed");
    expect(!advisory.capabilities.read_workspace_files &&
               !advisory.capabilities.write_workspace_files &&
               !advisory.capabilities.run_local_processes &&
               !advisory.capabilities.access_outside_workspace &&
               !advisory.capabilities.use_network,
           "advisory mode should carry no local execution capability");

    auto sandbox = evaluate_workspace_agent_activation(
        base_request(WorkspaceAgentAccessMode::workspace_sandbox));
    expect(sandbox.allowed, "trusted opt-in workspace sandbox should be allowed");
    expect(sandbox.capabilities.read_workspace_files &&
               sandbox.capabilities.write_workspace_files &&
               sandbox.capabilities.run_local_processes,
           "workspace sandbox should support useful local coding work");
    expect(!sandbox.capabilities.access_outside_workspace &&
               !sandbox.capabilities.use_network &&
               !sandbox.capabilities.elevate_privileges,
           "workspace sandbox should retain its filesystem, network, and privilege boundaries");
}

void test_mode_names_are_stable_and_parse_exactly() {
    using namespace copperfin::security;

    for (const auto mode : {
             WorkspaceAgentAccessMode::advisory,
             WorkspaceAgentAccessMode::workspace_sandbox,
             WorkspaceAgentAccessMode::unrestricted_local}) {
        const auto name = workspace_agent_access_mode_name(mode);
        const auto parsed = parse_workspace_agent_access_mode(name);
        expect(parsed.has_value() && *parsed == mode,
               "every stable workspace-agent mode name should round-trip");
    }
    expect(!parse_workspace_agent_access_mode("unrestricted").has_value() &&
               !parse_workspace_agent_access_mode("UNRESTRICTED_LOCAL").has_value() &&
               !parse_workspace_agent_access_mode(" unrestricted_local ").has_value(),
           "mode parsing should reject aliases, case changes, and whitespace variants");
    expect(workspace_agent_access_mode_name(
               static_cast<WorkspaceAgentAccessMode>(99)) == "invalid",
           "unknown enum values should serialize to a non-admissible sentinel");
}

void test_unrestricted_mode_requires_current_warning_and_consent() {
    using namespace copperfin::security;

    auto request = base_request(WorkspaceAgentAccessMode::unrestricted_local);
    auto decision = evaluate_workspace_agent_activation(request);
    expect(!decision.allowed && decision.warning_required &&
               decision.diagnostic_code == "workspace_agent.warning_required",
           "unrestricted access should fail closed until the warning dialog is presented");

    request.warning_presented = true;
    request.warning_id = "workspace-agent.unrestricted-local.stale";
    decision = evaluate_workspace_agent_activation(request);
    expect(!decision.allowed && decision.warning_required &&
               decision.diagnostic_code == "workspace_agent.warning_version_mismatch",
           "a stale or substituted warning should not authorize unrestricted access");

    request.warning_id = workspace_agent_unrestricted_warning_id;
    decision = evaluate_workspace_agent_activation(request);
    expect(!decision.allowed && decision.warning_required &&
               decision.diagnostic_code == "workspace_agent.confirmation_required",
           "displaying the warning without affirmative consent should not authorize access");

    request.user_confirmed = true;
    decision = evaluate_workspace_agent_activation(request);
    expect(decision.allowed &&
               decision.effective_mode == WorkspaceAgentAccessMode::unrestricted_local,
           "the exact warning plus affirmative consent should admit unrestricted mode");
    expect(decision.capabilities.read_workspace_files &&
               decision.capabilities.write_workspace_files &&
               decision.capabilities.run_local_processes &&
               decision.capabilities.access_outside_workspace &&
               decision.capabilities.use_network,
           "unrestricted mode should expose the explicitly requested local capabilities");
    expect(!decision.capabilities.elevate_privileges,
           "unrestricted mode should never imply administrator or root elevation");
}

void test_provider_identity_cannot_bypass_local_admission() {
    using namespace copperfin::security;

    auto request = base_request(WorkspaceAgentAccessMode::unrestricted_local);
    request.permission_granted = false;
    request.warning_presented = true;
    request.warning_id = workspace_agent_unrestricted_warning_id;
    request.user_confirmed = true;
    auto decision = evaluate_workspace_agent_activation(request);
    expect(!decision.allowed &&
               decision.diagnostic_code == "workspace_agent.permission_denied",
           "provider authentication should not substitute for native workspace-agent permission");

    request.permission_granted = true;
    request.trusted_product_ui = false;
    decision = evaluate_workspace_agent_activation(request);
    expect(!decision.allowed &&
               decision.diagnostic_code == "workspace_agent.trusted_ui_required",
           "credentials or direct adapter calls should not substitute for the trusted product UI");

    request.trusted_product_ui = true;
    request.audit_sink_available = false;
    decision = evaluate_workspace_agent_activation(request);
    expect(!decision.allowed &&
               decision.diagnostic_code == "workspace_agent.audit_unavailable",
           "unrestricted activation should fail closed without an audit sink");

    request.audit_sink_available = true;
    request.feature_enabled = false;
    decision = evaluate_workspace_agent_activation(request);
    expect(!decision.allowed &&
               decision.diagnostic_code == "workspace_agent.feature_disabled",
           "provider authentication should not override the user's disabled feature state");
    expect(decision.audit_required,
           "feature-disabled activation attempts should still require a denial audit event");
}

void test_unknown_mode_fails_closed() {
    using namespace copperfin::security;

    auto request = base_request(static_cast<WorkspaceAgentAccessMode>(99));
    request.warning_presented = true;
    request.warning_id = workspace_agent_unrestricted_warning_id;
    request.user_confirmed = true;
    const auto decision = evaluate_workspace_agent_activation(request);
    expect(!decision.allowed &&
               decision.diagnostic_code == "workspace_agent.invalid_mode",
           "an unknown serialized mode should not fall through to unrestricted admission");
    expect(!decision.capabilities.read_workspace_files &&
               !decision.capabilities.run_local_processes &&
               !decision.capabilities.access_outside_workspace,
           "an unknown mode should receive no local capabilities");
}

void test_native_profile_keeps_workspace_agent_permission_nondefault() {
    const auto profile = copperfin::security::default_native_security_profile();
    const auto permission = std::find_if(
        profile.permissions.begin(),
        profile.permissions.end(),
        [](const auto& candidate) { return candidate.id == "ai.workspace_agent"; });
    expect(permission != profile.permissions.end() && permission->high_risk,
           "the native profile should expose workspace-agent authority as high risk");
    expect(!copperfin::security::role_has_permission(
               profile,
               "developer",
               "ai.workspace_agent"),
           "the default developer assignment should not receive workspace-agent authority");
    expect(copperfin::security::role_has_permission(
               profile,
               "runtime-operator",
               "ai.workspace_agent"),
           "the nondefault runtime-operator role should be able to opt into the policy");
}

void test_warning_dialog_is_versioned_and_localized() {
    using namespace copperfin::security;

    const auto root = copperfin::localization::resolve_catalog_root();
    const auto english = unrestricted_workspace_agent_warning(
        copperfin::localization::load_catalogs(root, "en-US"));
    const auto spanish = unrestricted_workspace_agent_warning(
        copperfin::localization::load_catalogs(root, "es-419"));
    const auto pseudo = unrestricted_workspace_agent_warning(
        copperfin::localization::load_catalogs(root, "qps-ploc"));

    expect(english.id == workspace_agent_unrestricted_warning_id &&
               spanish.id == workspace_agent_unrestricted_warning_id &&
               pseudo.id == workspace_agent_unrestricted_warning_id,
           "localization should not change the warning contract identity");
    expect(english.title == "Enable unrestricted local agent access?",
           "the default warning title should preserve its reviewed English text");
    expect(spanish.title == "Habilitar acceso local sin restricciones para el agente?",
           "the warning dialog should use the selected Spanish catalog");
    expect(pseudo.title.find("[!! ") == 0U,
           "pseudo-localization should cover the warning dialog");
    expect(english.body.find("read, modify, or delete") != std::string::npos &&
               english.body.find("Provider sign-in does not grant this access") != std::string::npos,
           "the warning should state concrete risks and separate provider identity from local authority");
}

}  // namespace

int main() {
    test_mode_names_are_stable_and_parse_exactly();
    test_default_modes_are_bounded();
    test_unrestricted_mode_requires_current_warning_and_consent();
    test_provider_identity_cannot_bypass_local_admission();
    test_unknown_mode_fails_closed();
    test_native_profile_keeps_workspace_agent_permission_nondefault();
    test_warning_dialog_is_versioned_and_localized();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }
    std::cout << "All workspace-agent policy tests passed.\n";
    return 0;
}
