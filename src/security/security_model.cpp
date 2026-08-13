// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/security_model.h"

#include "copperfin/localization/localization.h"

namespace copperfin::security {

namespace {

std::string profile_text(
    const localization::LocalizedCatalog& catalog,
    std::string_view key) {
    return catalog.translate(key);
}

}  // namespace

NativeSecurityProfile default_native_security_profile(const localization::LocalizedCatalog& catalog) {
    NativeSecurityProfile profile;
    profile.available = true;
    profile.optional = true;
    profile.mode = profile_text(catalog, "Security.Profile.Mode");
    profile.package_policy = profile_text(catalog, "Security.Profile.PackagePolicy");
    profile.managed_interop_policy = profile_text(catalog, "Security.Profile.ManagedInteropPolicy");

    profile.permissions = {
        {"project.open", profile_text(catalog, "Security.Profile.Permission.ProjectOpen.Title"), profile_text(catalog, "Security.Profile.Permission.ProjectOpen.Description")},
        {"asset.edit", profile_text(catalog, "Security.Profile.Permission.AssetEdit.Title"), profile_text(catalog, "Security.Profile.Permission.AssetEdit.Description")},
        {"build.execute", profile_text(catalog, "Security.Profile.Permission.BuildExecute.Title"), profile_text(catalog, "Security.Profile.Permission.BuildExecute.Description"), true},
        {"build.release", profile_text(catalog, "Security.Profile.Permission.BuildRelease.Title"), profile_text(catalog, "Security.Profile.Permission.BuildRelease.Description"), true},
        {"runtime.admin", profile_text(catalog, "Security.Profile.Permission.RuntimeAdmin.Title"), profile_text(catalog, "Security.Profile.Permission.RuntimeAdmin.Description"), true},
        {"security.manage", profile_text(catalog, "Security.Profile.Permission.SecurityManage.Title"), profile_text(catalog, "Security.Profile.Permission.SecurityManage.Description"), true},
        {"data.export", profile_text(catalog, "Security.Profile.Permission.DataExport.Title"), profile_text(catalog, "Security.Profile.Permission.DataExport.Description"), true},
        {"interop.dotnet", profile_text(catalog, "Security.Profile.Permission.InteropDotNet.Title"), profile_text(catalog, "Security.Profile.Permission.InteropDotNet.Description")},
        {"interop.python", profile_text(catalog, "Security.Profile.Permission.InteropPython.Title"), profile_text(catalog, "Security.Profile.Permission.InteropPython.Description"), true},
        {"ai.mcp", profile_text(catalog, "Security.Profile.Permission.AiMcp.Title"), profile_text(catalog, "Security.Profile.Permission.AiMcp.Description"), true},
        {"ai.workspace_agent", profile_text(catalog, "Security.Profile.Permission.AiWorkspaceAgent.Title"), profile_text(catalog, "Security.Profile.Permission.AiWorkspaceAgent.Description"), true},
        {"external.process", profile_text(catalog, "Security.Profile.Permission.ExternalProcess.Title"), profile_text(catalog, "Security.Profile.Permission.ExternalProcess.Description"), true}
    };

    profile.roles = {
        {"developer", profile_text(catalog, "Security.Profile.Role.Developer.Title"), profile_text(catalog, "Security.Profile.Role.Developer.Description"), {"project.open", "asset.edit", "interop.dotnet"}, true},
        {"build-engineer", profile_text(catalog, "Security.Profile.Role.BuildEngineer.Title"), profile_text(catalog, "Security.Profile.Role.BuildEngineer.Description"), {"project.open", "build.execute", "build.release", "interop.dotnet"}, false},
        {"security-admin", profile_text(catalog, "Security.Profile.Role.SecurityAdmin.Title"), profile_text(catalog, "Security.Profile.Role.SecurityAdmin.Description"), {"project.open", "security.manage", "runtime.admin"}, false},
        {"auditor", profile_text(catalog, "Security.Profile.Role.Auditor.Title"), profile_text(catalog, "Security.Profile.Role.Auditor.Description"), {"project.open", "data.export"}, false},
        {"runtime-operator", profile_text(catalog, "Security.Profile.Role.RuntimeOperator.Title"), profile_text(catalog, "Security.Profile.Role.RuntimeOperator.Description"), {"project.open", "runtime.admin", "interop.dotnet", "interop.python", "ai.mcp", "ai.workspace_agent"}, false}
    };

    profile.identity_providers = {
        {"local", profile_text(catalog, "Security.Profile.IdentityProvider.Local.Title"), "native", profile_text(catalog, "Security.Profile.IdentityProvider.Local.Description"), true},
        {"windows", profile_text(catalog, "Security.Profile.IdentityProvider.Windows.Title"), "windows", profile_text(catalog, "Security.Profile.IdentityProvider.Windows.Description"), false},
        {"entra-oidc", profile_text(catalog, "Security.Profile.IdentityProvider.EntraOidc.Title"), "federated", profile_text(catalog, "Security.Profile.IdentityProvider.EntraOidc.Description"), false},
        {"external", profile_text(catalog, "Security.Profile.IdentityProvider.External.Title"), "adapter", profile_text(catalog, "Security.Profile.IdentityProvider.External.Description"), false}
    };

    profile.features = {
        {"rbac", profile_text(catalog, "Security.Profile.Feature.Rbac.Title"), profile_text(catalog, "Security.Profile.Feature.Rbac.Description"), true, true},
        {"secrets", profile_text(catalog, "Security.Profile.Feature.Secrets.Title"), profile_text(catalog, "Security.Profile.Feature.Secrets.Description"), true, true},
        {"audit", profile_text(catalog, "Security.Profile.Feature.Audit.Title"), profile_text(catalog, "Security.Profile.Feature.Audit.Description"), true, true},
        {"signing", profile_text(catalog, "Security.Profile.Feature.Signing.Title"), profile_text(catalog, "Security.Profile.Feature.Signing.Description"), true, true},
        {"sandbox", profile_text(catalog, "Security.Profile.Feature.Sandbox.Title"), profile_text(catalog, "Security.Profile.Feature.Sandbox.Description"), true, true}
    };

    profile.audit_events = {
        "login.identity_resolved",
        "role.assigned_or_denied",
        "asset.modified",
        "build.executed",
        "release.signed",
        "runtime.policy_changed",
        "data.exported",
        "interop.dotnet_invoked",
        "interop.python_invoked",
        "ai.mcp_invoked",
        "ai.workspace_agent_activation_allowed",
        "ai.workspace_agent_activation_denied",
        "external.process_launched",
        "policy.denied"
    };

    profile.hardening_profiles = {
        profile_text(catalog, "Security.Profile.Hardening.Bronze"),
        profile_text(catalog, "Security.Profile.Hardening.Silver"),
        profile_text(catalog, "Security.Profile.Hardening.Gold")
    };

    return profile;
}

NativeSecurityProfile default_native_security_profile() {
    const auto catalog = localization::load_catalogs(
        localization::resolve_catalog_root(),
        localization::select_locale());
    return default_native_security_profile(catalog);
}

}  // namespace copperfin::security
