#include "copperfin/platform/database_model.h"
#include "copperfin/platform/extensibility_model.h"
#include "copperfin/localization/localization.h"
#include "copperfin/security/security_model.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void test_default_security_profile() {
    const auto profile = copperfin::security::default_native_security_profile();
    expect(profile.available, "security profile should be available");
    expect(profile.optional, "security profile should be optional to enable");
    expect(!profile.roles.empty(), "security profile should define roles");
    expect(!profile.permissions.empty(), "security profile should define permissions");
    expect(!profile.identity_providers.empty(), "security profile should define identity providers");

    const auto security_admin = std::find_if(profile.roles.begin(), profile.roles.end(), [](const auto& role) {
        return role.id == "security-admin";
    });
    expect(security_admin != profile.roles.end(), "security profile should include a security administrator role");
    if (security_admin != profile.roles.end()) {
        expect(
            security_admin->title == "Security Administrator",
            "#2490: default security role title should preserve en-US prose");
        expect(
            security_admin->description == "Owns identity mapping, role policy, and trust settings.",
            "#2490: default security role description should preserve en-US prose");
    }

    const auto ai_permission = std::find_if(profile.permissions.begin(), profile.permissions.end(), [](const auto& permission) {
        return permission.id == "ai.mcp";
    });
    expect(ai_permission != profile.permissions.end(), "security profile should include MCP/AI permissions");
    if (ai_permission != profile.permissions.end()) {
        expect(
            ai_permission->title == "Use MCP And AI Tools",
            "#2490: default security permission title should preserve en-US prose");
    }

    expect(
        profile.mode == "optional native security with platform RBAC",
        "#2490: default security profile mode should preserve en-US prose");
    expect(
        profile.package_policy == "signed packages, signed extensions, explicit trust manifests",
        "#2490: default security package policy should preserve en-US prose");

    const auto pseudo_catalog =
        copperfin::localization::load_catalogs(copperfin::localization::resolve_catalog_root(), "qps-ploc");
    const auto pseudo_profile = copperfin::security::default_native_security_profile(pseudo_catalog);
    expect(
        pseudo_profile.mode.find("[!! ") != std::string::npos,
        "#2490: pseudo-localized security profile mode should route through the catalog");
    expect(
        pseudo_profile.permissions.size() == profile.permissions.size(),
        "#2490: pseudo-localized security profile should preserve permission counts");
    expect(
        !pseudo_profile.permissions.empty() && pseudo_profile.permissions[0].id == "project.open",
        "#2490: pseudo-localized security profile should preserve permission ids");
    expect(
        !pseudo_profile.permissions.empty() && pseudo_profile.permissions[0].title.find("[!! ") != std::string::npos,
        "#2490: pseudo-localized permission titles should route through the catalog");
    expect(
        !pseudo_profile.permissions.empty() && pseudo_profile.permissions[0].title.find("Open Project") == std::string::npos,
        "#2490: pseudo-localized permission titles should not fall back to raw English prose");
    expect(
        pseudo_profile.identity_providers.size() > 1U && pseudo_profile.identity_providers[1].kind == "windows",
        "#2490: pseudo-localized security profile should preserve identity provider kind values");
    expect(
        !pseudo_profile.audit_events.empty() && pseudo_profile.audit_events[0] == "login.identity_resolved",
        "#2490: pseudo-localized security profile should preserve audit event ids");
}

void test_default_extensibility_profile() {
    const auto profile = copperfin::platform::default_extensibility_profile();
    expect(profile.available, "extensibility profile should be available");
    expect(profile.dotnet_output.available, "extensibility profile should include a .NET output story");
    expect(profile.dotnet_output.managed_wrappers, "extensibility profile should support managed wrappers");
    expect(!profile.dotnet_output.parity_matrix.empty(), "extensibility profile should define a .NET parity matrix");
    expect(!profile.dotnet_output.policy.allowlist.empty(), "extensibility profile should include a .NET interop allowlist");
    expect(!profile.dotnet_output.policy.denylist.empty(), "extensibility profile should include a .NET interop denylist");

    const auto python = std::find_if(profile.languages.begin(), profile.languages.end(), [](const auto& language) {
        return language.id == "python";
    });
    expect(python != profile.languages.end(), "extensibility profile should include Python as a sidecar story");

    const auto mcp = std::find_if(profile.ai_features.begin(), profile.ai_features.end(), [](const auto& feature) {
        return feature.id == "mcp-host";
    });
    expect(mcp != profile.ai_features.end(), "extensibility profile should include MCP hosting");

    const auto deny_capability = std::find_if(
        profile.dotnet_output.parity_matrix.begin(),
        profile.dotnet_output.parity_matrix.end(),
        [](const auto& capability) {
            return capability.id == "unsafe-reflection-load" &&
                capability.tier == copperfin::platform::DotNetParityTier::intentionally_not_supported;
        });
    expect(
        deny_capability != profile.dotnet_output.parity_matrix.end(),
        "extensibility parity matrix should include explicit intentionally-not-supported capabilities");
}

void test_dotnet_interop_policy_gateway() {
    const auto profile = copperfin::platform::default_extensibility_profile();

    const auto allowed = copperfin::platform::evaluate_dotnet_interop_call(
        profile,
        copperfin::platform::DotNetInteropCallRequest{
            .capability_id = "task-primitives",
            .estimated_latency_ms = 10U,
            .requires_reflection = false,
            .untrusted_input = false,
            .security_sensitive = false});
    expect(
        allowed.decision == copperfin::platform::DotNetInteropDecision::allow,
        "policy gateway should allow listed .NET parity capabilities within budget");

    const auto denied = copperfin::platform::evaluate_dotnet_interop_call(
        profile,
        copperfin::platform::DotNetInteropCallRequest{
            .capability_id = "unsafe-reflection-load",
            .estimated_latency_ms = 5U,
            .requires_reflection = true,
            .untrusted_input = true,
            .security_sensitive = true});
    expect(
        denied.decision == copperfin::platform::DotNetInteropDecision::reject,
        "policy gateway should reject denylisted capabilities");

    const auto fallback = copperfin::platform::evaluate_dotnet_interop_call(
        profile,
        copperfin::platform::DotNetInteropCallRequest{
            .capability_id = "json-helpers",
            .estimated_latency_ms = 250U,
            .requires_reflection = false,
            .untrusted_input = false,
            .security_sensitive = false});
    expect(
        fallback.decision == copperfin::platform::DotNetInteropDecision::fallback_native,
        "policy gateway should fall back to native when estimated latency exceeds budget");
}

void test_default_database_profile() {
    const auto profile = copperfin::platform::default_database_federation_profile();
    expect(profile.available, "database profile should be available");
    expect(!profile.connectors.empty(), "database profile should define connectors");
    expect(!profile.query_paths.empty(), "database profile should define query translation paths");

    const auto sqlite = std::find_if(profile.connectors.begin(), profile.connectors.end(), [](const auto& connector) {
        return connector.id == "sqlite";
    });
    expect(sqlite != profile.connectors.end(), "database profile should include SQLite");
    if (sqlite != profile.connectors.end()) {
        expect(sqlite->fox_sql_translation_direct, "SQLite should support direct Fox SQL translation");
    }

    const auto mongodb = std::find_if(profile.connectors.begin(), profile.connectors.end(), [](const auto& connector) {
        return connector.id == "mongodb";
    });
    expect(mongodb != profile.connectors.end(), "database profile should include document database guidance");
    if (mongodb != profile.connectors.end()) {
        expect(mongodb->ai_query_planning_optional, "document database planning should allow optional AI assistance");
    }
}

void test_document_and_vector_mapping_paths() {
    const auto profile = copperfin::platform::default_database_federation_profile();

    const auto mongo = copperfin::platform::database_connector_by_id(profile, "mongodb");
    expect(mongo != nullptr, "connector lookup by id should resolve mongodb");
    if (mongo != nullptr) {
        expect(mongo->family == "document", "mongodb should remain in the document connector family");
        expect(!mongo->translation_story.empty(), "document connectors should define a translation story");
    }

    const auto json_api = copperfin::platform::database_connector_by_id(profile, "json-api");
    expect(json_api != nullptr, "connector lookup by id should resolve json-api");

    const auto vector = copperfin::platform::database_connector_by_id(profile, "vector");
    expect(vector != nullptr, "connector lookup by id should resolve vector");
    if (vector != nullptr) {
        expect(vector->family == "vector", "vector connector should remain in the vector family");
        expect(vector->ai_query_planning_optional, "vector connectors should keep optional AI planning enabled");
    }

    const auto foxsql_doc = copperfin::platform::query_translation_path_by_id(profile, "foxsql-document");
    expect(foxsql_doc != nullptr, "query path lookup should resolve foxsql-document");
    if (foxsql_doc != nullptr) {
        expect(foxsql_doc->deterministic_first, "foxsql-document translation should stay deterministic-first");
        expect(foxsql_doc->ai_optional, "foxsql-document translation should keep optional AI planning");
        expect(foxsql_doc->source_shape == "FoxPro-style SQL", "foxsql-document source-shape should remain FoxPro SQL");
    }

    const auto foxsql_vector = copperfin::platform::query_translation_path_by_id(profile, "foxsql-vector");
    expect(foxsql_vector != nullptr, "query path lookup should resolve foxsql-vector");
    if (foxsql_vector != nullptr) {
        expect(foxsql_vector->deterministic_first, "foxsql-vector translation should stay deterministic-first");
        expect(foxsql_vector->ai_optional, "foxsql-vector translation should keep optional AI planning");
        expect(foxsql_vector->source_shape == "FoxPro-style SQL plus semantic operators",
               "foxsql-vector source-shape should remain semantic-aware");
    }

    const auto browse_doc = copperfin::platform::query_translation_path_by_id(profile, "xbase-browse-document");
    expect(browse_doc != nullptr, "query path lookup should resolve xbase-browse-document");
    if (browse_doc != nullptr) {
        expect(browse_doc->deterministic_first, "document browse intent mapping should stay deterministic-first");
        expect(!browse_doc->target_shape.empty(), "document browse intent mapping should expose a target shape");
    }
}

}  // namespace

int main() {
    test_default_security_profile();
    test_default_extensibility_profile();
    test_dotnet_interop_policy_gateway();
    test_default_database_profile();
    test_document_and_vector_mapping_paths();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
