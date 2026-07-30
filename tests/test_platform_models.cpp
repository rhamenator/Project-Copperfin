// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/platform/database_model.h"
#include "copperfin/platform/environment.h"
#include "copperfin/platform/extensibility_model.h"
#include "copperfin/platform/json.h"
#include "copperfin/localization/localization.h"
#include "copperfin/security/security_model.h"
#include "test_environment_support.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <locale>
#include <string>

namespace {

int failures = 0;

class every_digit_numpunct final : public std::numpunct<char> {
protected:
    char do_thousands_sep() const override { return '.'; }
    std::string do_grouping() const override { return "\1"; }
};

class global_locale_guard final {
public:
    explicit global_locale_guard(const std::locale& replacement)
        : previous_(std::locale::global(replacement)) {}
    ~global_locale_guard() { std::locale::global(previous_); }
    global_locale_guard(const global_locale_guard&) = delete;
    global_locale_guard& operator=(const global_locale_guard&) = delete;
private:
    std::locale previous_;
};

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void test_json_escape_string_is_locale_invariant() {
    const std::locale grouping_locale(std::locale::classic(), new every_digit_numpunct());
    const global_locale_guard locale_guard(grouping_locale);
    std::string value{"\"\\\b\f\n\r\t", 7U};
    value.push_back('\0');
    value.push_back(static_cast<char>(0x1f));
    value += "caf\xC3\xA9";
    expect(
        copperfin::platform::json_escape_string(value) ==
            "\\\"\\\\\\b\\f\\n\\r\\t\\u0000\\u001fcaf\xC3\xA9",
        "JSON escaping should preserve canonical control escapes and UTF-8 under every-digit grouping");
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

    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog =
        copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const auto spanish_profile = copperfin::security::default_native_security_profile(spanish_catalog);
    const auto portuguese_profile = copperfin::security::default_native_security_profile(portuguese_catalog);
    const auto pseudo_profile = copperfin::security::default_native_security_profile(pseudo_catalog);

    const auto spanish_security_admin =
        std::find_if(spanish_profile.roles.begin(), spanish_profile.roles.end(), [](const auto& role) {
            return role.id == "security-admin";
        });
    expect(spanish_security_admin != spanish_profile.roles.end(),
           "#2600: es-419 security profile should still expose the security-admin role");
    if (spanish_security_admin != spanish_profile.roles.end()) {
        expect(spanish_security_admin->title == "Administrador de seguridad",
               "#2600: es-419 security-admin title should localize the prose");
        expect(spanish_security_admin->description ==
                   "Es responsable del mapeo de identidades, la politica de roles y la configuracion de confianza.",
               "#2600: es-419 security-admin description should localize the prose");
    }

    const auto spanish_project_open =
        std::find_if(spanish_profile.permissions.begin(), spanish_profile.permissions.end(), [](const auto& permission) {
            return permission.id == "project.open";
        });
    expect(spanish_project_open != spanish_profile.permissions.end(),
           "#2600: es-419 security profile should still expose the project.open permission");
    if (spanish_project_open != spanish_profile.permissions.end()) {
        expect(spanish_project_open->title == "Abrir proyecto",
               "#2600: es-419 project.open title should localize the prose");
        expect(spanish_project_open->description == "Abrir e inspeccionar assets del proyecto.",
               "#2600: es-419 project.open description should localize the prose");
    }

    expect(spanish_profile.mode == "seguridad nativa opcional con RBAC de plataforma",
           "#2600: es-419 security profile mode should localize the prose");
    expect(!spanish_profile.features.empty() &&
               spanish_profile.features[0].title == "Control de acceso basado en roles",
           "#2600: es-419 security features should localize without falling back to English");
    expect(spanish_profile.identity_providers.size() > 1U &&
               spanish_profile.identity_providers[1].kind == "windows" &&
               spanish_profile.identity_providers[1].title == "Identidad Windows/AD",
           "#2600: es-419 identity provider prose should localize while preserving kind values");

    const auto portuguese_ai_permission =
        std::find_if(portuguese_profile.permissions.begin(), portuguese_profile.permissions.end(), [](const auto& permission) {
            return permission.id == "ai.mcp";
        });
    expect(portuguese_ai_permission != portuguese_profile.permissions.end(),
           "#2600: pt-BR security profile should still expose the ai.mcp permission");
    if (portuguese_ai_permission != portuguese_profile.permissions.end()) {
        expect(portuguese_ai_permission->title == "Usar ferramentas MCP e IA",
               "#2600: pt-BR ai.mcp title should localize the prose");
        expect(portuguese_ai_permission->description ==
                   "Invocar ferramentas MCP ou fluxos assistidos por IA para desenvolvedores sob auditoria.",
               "#2600: pt-BR ai.mcp description should localize the prose");
    }

    expect(portuguese_profile.package_policy ==
               "pacotes assinados, extensoes assinadas, manifestos explicitos de confianca",
           "#2600: pt-BR security package policy should localize the prose");
    expect(!portuguese_profile.hardening_profiles.empty() &&
               portuguese_profile.hardening_profiles[2] ==
                   "ouro: identidade empresarial, auditoria completa, provedores de segredos, restricoes de interop, assinatura release",
           "#2600: pt-BR hardening profiles should localize without falling back to English");
    expect(!portuguese_profile.features.empty() &&
               portuguese_profile.features[0].id == "rbac",
           "#2600: pt-BR security features should preserve feature ids");

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
    if (python != profile.languages.end()) {
        expect(
            python->title == "Python Sidecar And Analytics Jobs",
            "#2492: default Python language title should preserve en-US prose");
        expect(
            python->trust_boundary == "restricted external process boundary",
            "#2492: default Python trust boundary should preserve en-US prose");
    }

    const auto mcp = std::find_if(profile.ai_features.begin(), profile.ai_features.end(), [](const auto& feature) {
        return feature.id == "mcp-host";
    });
    expect(mcp != profile.ai_features.end(), "extensibility profile should include MCP hosting");
    if (mcp != profile.ai_features.end()) {
        expect(
            mcp->title == "MCP Host Facility",
            "#2492: default MCP feature title should preserve en-US prose");
    }
    expect(
        profile.dotnet_output.primary_story.find("native executables") != std::string::npos,
        "#2492: default .NET output story should preserve en-US prose");

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
    if (deny_capability != profile.dotnet_output.parity_matrix.end()) {
        expect(
            deny_capability->title == "Arbitrary reflection-based assembly loading",
            "#2492: default .NET parity capability title should preserve en-US prose");
        expect(
            !deny_capability->reason_tags.empty() && deny_capability->reason_tags[0] == "security",
            "#2492: default .NET parity reason tags should remain invariant");
    }

    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog =
        copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const auto spanish_profile = copperfin::platform::default_extensibility_profile(spanish_catalog);
    const auto portuguese_profile = copperfin::platform::default_extensibility_profile(portuguese_catalog);
    const auto pseudo_profile = copperfin::platform::default_extensibility_profile(pseudo_catalog);

    const auto spanish_xbase =
        std::find_if(spanish_profile.languages.begin(), spanish_profile.languages.end(), [](const auto& language) {
            return language.id == "xbase";
        });
    expect(spanish_xbase != spanish_profile.languages.end(),
           "#2599: es-419 extensibility profile should still expose the xbase language entry");
    if (spanish_xbase != spanish_profile.languages.end()) {
        expect(spanish_xbase->title == "Runtime nativo Copperfin/xBase",
               "#2599: es-419 xbase language title should localize the prose");
        expect(spanish_xbase->trust_boundary == "nucleo confiable",
               "#2599: es-419 xbase trust boundary should localize the prose");
    }

    const auto spanish_mcp =
        std::find_if(spanish_profile.ai_features.begin(), spanish_profile.ai_features.end(), [](const auto& feature) {
            return feature.id == "mcp-host";
        });
    expect(spanish_mcp != spanish_profile.ai_features.end(),
           "#2599: es-419 extensibility profile should still expose the MCP feature");
    if (spanish_mcp != spanish_profile.ai_features.end()) {
        expect(spanish_mcp->title == "Capacidad de host MCP",
               "#2599: es-419 MCP feature title should localize the prose");
        expect(spanish_mcp->trust_boundary == "limite de herramienta externa administrado por politica",
               "#2599: es-419 MCP feature trust boundary should localize the prose");
    }

    expect(!spanish_profile.guardrails.empty() &&
               spanish_profile.guardrails[0] ==
                   "El nucleo de ejecucion confiable sigue siendo nativo primero y orientado a la seguridad.",
           "#2599: es-419 extensibility guardrails should localize without falling back to English");

    const auto portuguese_python =
        std::find_if(portuguese_profile.languages.begin(), portuguese_profile.languages.end(), [](const auto& language) {
            return language.id == "python";
        });
    expect(portuguese_python != portuguese_profile.languages.end(),
           "#2599: pt-BR extensibility profile should still expose the python language entry");
    if (portuguese_python != portuguese_profile.languages.end()) {
        expect(portuguese_python->title == "Sidecar de Python e jobs de analitica",
               "#2599: pt-BR python language title should localize the prose");
        expect(portuguese_python->output_story ==
                   "O suporte a Python e posicionado como sidecar ou servico de jobs para ciencia de dados e automacao, nao como o nucleo confiavel.",
               "#2599: pt-BR python output story should localize the prose");
    }

    const auto portuguese_task_primitives = std::find_if(
        portuguese_profile.dotnet_output.parity_matrix.begin(),
        portuguese_profile.dotnet_output.parity_matrix.end(),
        [](const auto& capability) { return capability.id == "task-primitives"; });
    expect(portuguese_task_primitives != portuguese_profile.dotnet_output.parity_matrix.end(),
           "#2599: pt-BR extensibility profile should still expose task-primitives parity");
    if (portuguese_task_primitives != portuguese_profile.dotnet_output.parity_matrix.end()) {
        expect(portuguese_task_primitives->title == "Primitivas Task/Async",
               "#2599: pt-BR task-primitives title should localize the prose");
        expect(portuguese_task_primitives->rationale ==
                   "Expor comportamento no estilo async/await por meio de fachadas de comandos e funcoes amigaveis para FP/VFP.",
               "#2599: pt-BR task-primitives rationale should localize the prose");
        expect(!portuguese_task_primitives->reason_tags.empty() &&
                   portuguese_task_primitives->reason_tags[0] == "ergonomics",
               "#2599: pt-BR parity entries should preserve reason tags");
    }

    expect(
        pseudo_profile.languages.size() == profile.languages.size(),
        "#2492: pseudo-localized extensibility profile should preserve language counts");
    expect(
        !pseudo_profile.languages.empty() && pseudo_profile.languages[0].id == "xbase",
        "#2492: pseudo-localized extensibility profile should preserve language ids");
    expect(
        !pseudo_profile.languages.empty() && pseudo_profile.languages[0].title.find("[!! ") != std::string::npos,
        "#2492: pseudo-localized language titles should route through the catalog");
    expect(
        !pseudo_profile.languages.empty() &&
            pseudo_profile.languages[0].title.find("Native Copperfin/xBase Runtime") == std::string::npos,
        "#2492: pseudo-localized language titles should not fall back to raw English prose");
    expect(
        !pseudo_profile.ai_features.empty() && pseudo_profile.ai_features[0].id == "mcp-host",
        "#2492: pseudo-localized extensibility profile should preserve AI feature ids");
    expect(
        !pseudo_profile.ai_features.empty() && pseudo_profile.ai_features[0].title.find("[!! ") != std::string::npos,
        "#2492: pseudo-localized AI feature titles should route through the catalog");
    expect(
        !pseudo_profile.dotnet_output.parity_matrix.empty() &&
            pseudo_profile.dotnet_output.parity_matrix[0].id == "task-primitives",
        "#2492: pseudo-localized extensibility profile should preserve .NET parity ids");
    expect(
        !pseudo_profile.dotnet_output.parity_matrix.empty() &&
            pseudo_profile.dotnet_output.parity_matrix[0].title.find("[!! ") != std::string::npos,
        "#2492: pseudo-localized .NET parity titles should route through the catalog");
    expect(
        !pseudo_profile.guardrails.empty() && pseudo_profile.guardrails[0].find("[!! ") != std::string::npos,
        "#2492: pseudo-localized extensibility guardrails should route through the catalog");

    copperfin::test_support::ScopedEnvironmentValue locale_override("COPPERFIN_LOCALE");
    locale_override.set("en-US");
    const auto live_english_profile =
        copperfin::platform::default_extensibility_profile();
    locale_override.set("es-419");
    const auto live_spanish_profile =
        copperfin::platform::default_extensibility_profile();
    locale_override.set("qps-ploc");
    const auto live_pseudo_profile =
        copperfin::platform::default_extensibility_profile();
    const auto find_xbase = [](const auto& profile) {
        return std::find_if(
            profile.languages.begin(),
            profile.languages.end(),
            [](const auto& language) { return language.id == "xbase"; });
    };
    const auto live_english_xbase = find_xbase(live_english_profile);
    const auto live_spanish_xbase = find_xbase(live_spanish_profile);
    const auto live_pseudo_xbase = find_xbase(live_pseudo_profile);
    expect(live_english_xbase != live_english_profile.languages.end() &&
               live_spanish_xbase != live_spanish_profile.languages.end() &&
               live_pseudo_xbase != live_pseudo_profile.languages.end(),
           "#4359: default extensibility profile refresh should preserve language identity");
    if (live_english_xbase != live_english_profile.languages.end() &&
        live_spanish_xbase != live_spanish_profile.languages.end() &&
        live_pseudo_xbase != live_pseudo_profile.languages.end()) {
        expect(live_english_xbase->id == "xbase" &&
                   live_spanish_xbase->id == "xbase" &&
                   live_pseudo_xbase->id == "xbase",
               "#4359: extensibility language ids must remain invariant across locale refresh");
        expect(live_english_xbase->title == "Native Copperfin/xBase Runtime",
               "#4359: default extensibility profile should begin in en-US");
        expect(live_spanish_xbase->title == "Runtime nativo Copperfin/xBase",
               "#4359: default extensibility profile should refresh to es-419");
        expect(live_pseudo_xbase->title.find("[!! ") != std::string::npos,
               "#4359: default extensibility profile should refresh to qps-ploc");
    }
    expect(!live_pseudo_profile.ai_features.empty() &&
               live_pseudo_profile.ai_features[0].id == "mcp-host" &&
               live_pseudo_profile.ai_features[0].title.find("[!! ") != std::string::npos,
           "#4359: refreshed pseudo-locale should preserve AI ids and localize display text");
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
    expect(
        allowed.execution_path == "dotnet",
        "#2493: allowed .NET interop execution path should remain invariant");
    expect(
        allowed.reason == "allowed by policy with audit-required path",
        "#2493: allowed .NET interop reason should preserve default en-US prose");

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
    expect(
        denied.execution_path == "none",
        "#2493: denylisted .NET interop execution path should remain invariant");
    expect(
        denied.reason == "capability denied by policy allowlist/denylist",
        "#2493: denylisted .NET interop reason should preserve default en-US prose");

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
    expect(
        fallback.execution_path == "native",
        "#2493: latency fallback execution path should remain invariant");
    expect(
        fallback.reason == "estimated latency exceeds in-process policy budget",
        "#2493: latency fallback reason should preserve default en-US prose");

    auto unavailable_profile = profile;
    unavailable_profile.dotnet_output.available = false;
    const auto unavailable = copperfin::platform::evaluate_dotnet_interop_call(
        unavailable_profile,
        copperfin::platform::DotNetInteropCallRequest{.capability_id = "task-primitives"});
    expect(
        unavailable.reason == "dotnet output profile unavailable",
        "#2493: unavailable .NET output reason should preserve default en-US prose");

    const auto empty_capability = copperfin::platform::evaluate_dotnet_interop_call(
        profile,
        copperfin::platform::DotNetInteropCallRequest{});
    expect(
        empty_capability.reason == "capability id is required",
        "#2493: empty capability reason should preserve default en-US prose");

    const auto not_allowlisted = copperfin::platform::evaluate_dotnet_interop_call(
        profile,
        copperfin::platform::DotNetInteropCallRequest{.capability_id = "legacy-helper"});
    expect(
        not_allowlisted.reason == "capability not in allowlist",
        "#2493: non-allowlisted capability reason should preserve default en-US prose");

    const auto reflection_denied = copperfin::platform::evaluate_dotnet_interop_call(
        profile,
        copperfin::platform::DotNetInteropCallRequest{
            .capability_id = "task-primitives",
            .estimated_latency_ms = 5U,
            .requires_reflection = true,
            .untrusted_input = true,
            .security_sensitive = true});
    expect(
        reflection_denied.reason == "reflection on untrusted input is blocked",
        "#2493: untrusted reflection reason should preserve default en-US prose");

    auto unaudited_profile = profile;
    unaudited_profile.dotnet_output.policy.require_policy_audit = false;
    const auto unaudited_allowed = copperfin::platform::evaluate_dotnet_interop_call(
        unaudited_profile,
        copperfin::platform::DotNetInteropCallRequest{
            .capability_id = "task-primitives",
            .estimated_latency_ms = 10U});
    expect(
        unaudited_allowed.reason == "allowed by policy",
        "#2493: unaudited allowed reason should preserve default en-US prose");

    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog =
        copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const auto spanish_allowed = copperfin::platform::evaluate_dotnet_interop_call(
        profile,
        copperfin::platform::DotNetInteropCallRequest{
            .capability_id = "task-primitives",
            .estimated_latency_ms = 10U},
        spanish_catalog);
    expect(spanish_allowed.reason == "permitida por politica con ruta que requiere auditoria",
           "#2599: es-419 allowed .NET interop reason should localize the prose");
    expect(spanish_allowed.execution_path == "dotnet",
           "#2599: es-419 allowed .NET interop should preserve execution-path values");

    const auto portuguese_denied = copperfin::platform::evaluate_dotnet_interop_call(
        profile,
        copperfin::platform::DotNetInteropCallRequest{
            .capability_id = "unsafe-reflection-load",
            .estimated_latency_ms = 5U,
            .requires_reflection = true,
            .untrusted_input = true,
            .security_sensitive = true},
        portuguese_catalog);
    expect(portuguese_denied.reason == "capacidade negada pela politica de allowlist/denylist",
           "#2599: pt-BR denylisted .NET interop reason should localize the prose");
    expect(portuguese_denied.execution_path == "none",
           "#2599: pt-BR denylisted .NET interop should preserve execution-path values");

    const auto pseudo_allowed = copperfin::platform::evaluate_dotnet_interop_call(
        profile,
        copperfin::platform::DotNetInteropCallRequest{
            .capability_id = "task-primitives",
            .estimated_latency_ms = 10U},
        pseudo_catalog);
    expect(
        pseudo_allowed.decision == copperfin::platform::DotNetInteropDecision::allow,
        "#2493: pseudo-localized allowed decision enum should remain invariant");
    expect(
        pseudo_allowed.execution_path == "dotnet",
        "#2493: pseudo-localized allowed execution path should remain invariant");
    expect(
        pseudo_allowed.reason.find("[!! ") != std::string::npos,
        "#2493: pseudo-localized allowed reason should route through the catalog");
    expect(
        pseudo_allowed.reason.find("allowed by policy with audit-required path") == std::string::npos,
        "#2493: pseudo-localized allowed reason should not fall back to raw English prose");

    const auto pseudo_denied = copperfin::platform::evaluate_dotnet_interop_call(
        profile,
        copperfin::platform::DotNetInteropCallRequest{
            .capability_id = "unsafe-reflection-load",
            .estimated_latency_ms = 5U,
            .requires_reflection = true,
            .untrusted_input = true,
            .security_sensitive = true},
        pseudo_catalog);
    expect(
        pseudo_denied.decision == copperfin::platform::DotNetInteropDecision::reject,
        "#2493: pseudo-localized denied decision enum should remain invariant");
    expect(
        pseudo_denied.execution_path == "none",
        "#2493: pseudo-localized denied execution path should remain invariant");
    expect(
        pseudo_denied.reason.find("[!! ") != std::string::npos,
        "#2493: pseudo-localized denied reason should route through the catalog");

    const auto pseudo_fallback = copperfin::platform::evaluate_dotnet_interop_call(
        profile,
        copperfin::platform::DotNetInteropCallRequest{
            .capability_id = "json-helpers",
            .estimated_latency_ms = 250U},
        pseudo_catalog);
    expect(
        pseudo_fallback.decision == copperfin::platform::DotNetInteropDecision::fallback_native,
        "#2493: pseudo-localized fallback decision enum should remain invariant");
    expect(
        pseudo_fallback.execution_path == "native",
        "#2493: pseudo-localized fallback execution path should remain invariant");
    expect(
        pseudo_fallback.reason.find("[!! ") != std::string::npos,
        "#2493: pseudo-localized fallback reason should route through the catalog");
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
        expect(sqlite->title == "SQLite", "#2491: default SQLite connector title should preserve en-US prose");
        expect(
            sqlite->access_mode == "embedded SQL engine",
            "#2491: default SQLite connector access mode should preserve en-US prose");
    }

    const auto mongodb = std::find_if(profile.connectors.begin(), profile.connectors.end(), [](const auto& connector) {
        return connector.id == "mongodb";
    });
    expect(mongodb != profile.connectors.end(), "database profile should include document database guidance");
    if (mongodb != profile.connectors.end()) {
        expect(mongodb->ai_query_planning_optional, "document database planning should allow optional AI assistance");
        expect(
            mongodb->translation_story ==
                "Document stores need projection and predicate translation from FoxPro-style queries into dynamic document pipelines.",
            "#2491: default MongoDB connector story should preserve en-US prose");
    }

    const auto relational_path = copperfin::platform::query_translation_path_by_id(profile, "foxsql-relational");
    expect(relational_path != nullptr, "database profile should include Fox SQL relational translation");
    if (relational_path != nullptr) {
        expect(
            relational_path->title == "Fox SQL To Relational SQL",
            "#2491: default query path title should preserve en-US prose");
        expect(
            relational_path->complexity == "low-to-medium",
            "#2491: default query path complexity should preserve en-US prose");
    }

    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog =
        copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    const auto spanish_profile = copperfin::platform::default_database_federation_profile(spanish_catalog);
    const auto portuguese_profile = copperfin::platform::default_database_federation_profile(portuguese_catalog);
    const auto pseudo_profile = copperfin::platform::default_database_federation_profile(pseudo_catalog);

    const auto spanish_dbf = copperfin::platform::database_connector_by_id(spanish_profile, "dbf");
    expect(spanish_dbf != nullptr, "#2598: es-419 database profile should still expose the DBF connector");
    if (spanish_dbf != nullptr) {
        expect(
            spanish_dbf->title == "Almacenamiento nativo DBF/CDX/FPT",
            "#2598: es-419 DBF connector title should localize the prose");
        expect(
            spanish_dbf->access_mode == "motor de archivos embebido",
            "#2598: es-419 DBF connector access mode should localize the prose");
        expect(
            spanish_dbf->family == "xbase",
            "#2598: es-419 database profile should preserve connector family values");
    }

    const auto spanish_relational_path =
        copperfin::platform::query_translation_path_by_id(spanish_profile, "foxsql-relational");
    expect(spanish_relational_path != nullptr, "#2598: es-419 database profile should still expose foxsql-relational");
    if (spanish_relational_path != nullptr) {
        expect(
            spanish_relational_path->title == "Fox SQL a SQL relacional",
            "#2598: es-419 foxsql-relational title should localize the prose");
        expect(
            spanish_relational_path->complexity == "baja a media",
            "#2598: es-419 foxsql-relational complexity should localize the prose");
        expect(
            spanish_relational_path->id == "foxsql-relational",
            "#2598: es-419 database profile should preserve query-path ids");
    }

    expect(
        !spanish_profile.guardrails.empty() &&
            spanish_profile.guardrails[0] ==
                "Los traductores deterministas van primero para los backends relacionales y otros mapeos directos.",
        "#2598: es-419 database guardrail prose should localize without falling back to English");

    const auto portuguese_mongodb = copperfin::platform::database_connector_by_id(portuguese_profile, "mongodb");
    expect(portuguese_mongodb != nullptr, "#2598: pt-BR database profile should still expose the MongoDB connector");
    if (portuguese_mongodb != nullptr) {
        expect(
            portuguese_mongodb->translation_story ==
                "Armazenamentos de documentos exigem traducao de projecoes e predicados de consultas no estilo FoxPro para pipelines dinamicos de documentos.",
            "#2598: pt-BR MongoDB connector story should localize the prose");
        expect(
            portuguese_mongodb->family == "document",
            "#2598: pt-BR database profile should preserve connector family values");
    }

    const auto portuguese_vector = copperfin::platform::database_connector_by_id(portuguese_profile, "vector");
    expect(portuguese_vector != nullptr, "#2598: pt-BR database profile should still expose the vector connector");
    if (portuguese_vector != nullptr) {
        expect(
            portuguese_vector->title == "Armazenamentos de vetores/embeddings",
            "#2598: pt-BR vector connector title should localize the prose");
        expect(
            portuguese_vector->schema_shape == "vetor",
            "#2598: pt-BR vector connector schema shape should localize the prose");
    }

    const auto portuguese_polyglot_path =
        copperfin::platform::query_translation_path_by_id(portuguese_profile, "xbase-polyglot");
    expect(portuguese_polyglot_path != nullptr, "#2598: pt-BR database profile should still expose xbase-polyglot");
    if (portuguese_polyglot_path != nullptr) {
        expect(
            portuguese_polyglot_path->source_shape == "intencao SEEK/BROWSE/SCAN/REPORT",
            "#2598: pt-BR xbase-polyglot source shape should localize prose while preserving command tokens");
        expect(
            portuguese_polyglot_path->id == "xbase-polyglot",
            "#2598: pt-BR database profile should preserve query-path ids");
    }

    expect(
        pseudo_profile.connectors.size() == profile.connectors.size(),
        "#2491: pseudo-localized database profile should preserve connector counts");
    expect(
        !pseudo_profile.connectors.empty() && pseudo_profile.connectors[0].id == "dbf",
        "#2491: pseudo-localized database profile should preserve connector ids");
    expect(
        !pseudo_profile.connectors.empty() && pseudo_profile.connectors[0].family == "xbase",
        "#2491: pseudo-localized database profile should preserve connector family values");
    expect(
        !pseudo_profile.connectors.empty() && pseudo_profile.connectors[0].title.find("[!! ") != std::string::npos,
        "#2491: pseudo-localized connector titles should route through the catalog");
    expect(
        !pseudo_profile.connectors.empty() &&
            pseudo_profile.connectors[0].title.find("DBF/CDX/FPT Native Storage") == std::string::npos,
        "#2491: pseudo-localized connector titles should not fall back to raw English prose");
    expect(
        !pseudo_profile.query_paths.empty() && pseudo_profile.query_paths[0].id == "foxsql-relational",
        "#2491: pseudo-localized database profile should preserve query path ids");
    expect(
        !pseudo_profile.query_paths.empty() && pseudo_profile.query_paths[0].title.find("[!! ") != std::string::npos,
        "#2491: pseudo-localized query path titles should route through the catalog");
    expect(
        !pseudo_profile.guardrails.empty() && pseudo_profile.guardrails[0].find("[!! ") != std::string::npos,
        "#2491: pseudo-localized guardrails should route through the catalog");

    copperfin::test_support::ScopedEnvironmentValue locale_override("COPPERFIN_LOCALE");
    locale_override.set("en-US");
    const auto live_english_profile =
        copperfin::platform::default_database_federation_profile();
    locale_override.set("es-419");
    const auto live_spanish_profile =
        copperfin::platform::default_database_federation_profile();
    locale_override.set("qps-ploc");
    const auto live_pseudo_profile =
        copperfin::platform::default_database_federation_profile();
    const auto live_english_dbf =
        copperfin::platform::database_connector_by_id(live_english_profile, "dbf");
    const auto live_spanish_dbf =
        copperfin::platform::database_connector_by_id(live_spanish_profile, "dbf");
    const auto live_pseudo_dbf =
        copperfin::platform::database_connector_by_id(live_pseudo_profile, "dbf");
    expect(live_english_dbf != nullptr && live_spanish_dbf != nullptr && live_pseudo_dbf != nullptr,
           "#2348: default database profile locale refresh should preserve connector identity");
    if (live_english_dbf != nullptr && live_spanish_dbf != nullptr && live_pseudo_dbf != nullptr) {
        expect(live_english_dbf->id == "dbf" && live_spanish_dbf->id == "dbf" && live_pseudo_dbf->id == "dbf",
               "#2348: database connector ids must remain invariant across locale refresh");
        expect(live_english_dbf->title == "DBF/CDX/FPT Native Storage",
               "#2348: default database profile should begin in en-US");
        expect(live_spanish_dbf->title == "Almacenamiento nativo DBF/CDX/FPT",
               "#2348: default database profile should refresh to es-419");
        expect(live_pseudo_dbf->title.find("[!! ") != std::string::npos,
               "#2348: default database profile should refresh to qps-ploc");
    }
    const auto live_pseudo_path = copperfin::platform::query_translation_path_by_id(
        live_pseudo_profile,
        "foxsql-relational");
    expect(live_pseudo_path != nullptr && live_pseudo_path->id == "foxsql-relational",
           "#2348: default database query-path identity must remain invariant across locale refresh");
    const auto live_spanish_path = copperfin::platform::query_translation_path_by_id(
        live_spanish_profile,
        "foxsql-relational");
    expect(live_spanish_path != nullptr &&
               live_spanish_path->title == "Fox SQL a SQL relacional",
           "#2348: default database query-path display should refresh to es-419");
    expect(live_pseudo_path != nullptr &&
               live_pseudo_path->title.find("[!! ") != std::string::npos,
           "#2348: default database query-path display should refresh to qps-ploc");
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

void test_platform_environment_helper_reads_and_clears_variables() {
    using copperfin::test_support::ScopedEnvironmentValue;
    using copperfin::test_support::set_env_value;

    ScopedEnvironmentValue scoped_value("COPPERFIN_TEST_PLATFORM_ENV");
    expect(!copperfin::platform::read_environment_variable("COPPERFIN_TEST_PLATFORM_ENV").has_value(),
           "platform env helper should report missing variables as nullopt");
    expect(copperfin::platform::read_environment_variable_or_empty("COPPERFIN_TEST_PLATFORM_ENV").empty(),
           "platform env helper should return an empty string for missing variables");

    set_env_value("COPPERFIN_TEST_PLATFORM_ENV", "alpha-env", true);
    const auto value = copperfin::platform::read_environment_variable("COPPERFIN_TEST_PLATFORM_ENV");
    expect(value.has_value(), "platform env helper should read present variables");
    if (value.has_value()) {
        expect(*value == "alpha-env", "platform env helper should preserve the environment value");
    }
    expect(copperfin::platform::read_environment_variable_or_empty("COPPERFIN_TEST_PLATFORM_ENV") == "alpha-env",
           "platform env helper should return present values through the string helper");
}

}  // namespace

int main() {
    test_json_escape_string_is_locale_invariant();
    test_default_security_profile();
    test_default_extensibility_profile();
    test_dotnet_interop_policy_gateway();
    test_default_database_profile();
    test_document_and_vector_mapping_paths();
    test_platform_environment_helper_reads_and_clears_variables();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
