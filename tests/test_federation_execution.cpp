// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/localization/localization.h"
#include "copperfin/platform/federation_execution.h"
#include "test_environment_support.h"
#include "test_locale_catalog_environment_support.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int failures = 0;
using copperfin::test_support::ScopedDefaultLocaleCatalogEnvironment;
using copperfin::test_support::ScopedEnvironmentValue;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void test_backend_parsing() {
    const auto postgres = copperfin::platform::federation_backend_from_string("postgres");
    expect(postgres.has_value(), "postgres alias should parse to a backend");
    if (postgres.has_value()) {
        expect(*postgres == copperfin::platform::FederationBackend::postgresql,
               "postgres alias should map to postgresql backend");
    }

    const auto invalid = copperfin::platform::federation_backend_from_string("mongo");
    expect(!invalid.has_value(), "unknown backend names should be rejected");
}

void test_plan_generation() {
    const auto plan = copperfin::platform::build_federation_execution_plan({
        .backend = copperfin::platform::FederationBackend::sqlserver,
        .fox_sql = "SELECT SUBSTR(name, 1, 2) FROM customer WHERE active = .T.",
        .target = "erp-prod"
    });

    expect(plan.ok, "execution plan should succeed for supported select SQL");
    if (plan.ok) {
        expect(plan.connector == "sqlserver", "sqlserver backend should use sqlserver connector mapping");
        expect(plan.target == "erp-prod", "explicit federation target should be preserved");
        expect(plan.translated_sql.find("SUBSTRING(") != std::string::npos,
               "sqlserver plan should include translated SQL");
        expect(plan.execution_command.find("connector.execute_query(") == 0,
               "execution command should use deterministic connector.execute_query shape");
    }
}

void test_plan_rejection() {
    const auto plan = copperfin::platform::build_federation_execution_plan({
        .backend = copperfin::platform::FederationBackend::oracle,
        .fox_sql = "DELETE FROM customer",
        .target = ""
    });

    expect(!plan.ok, "non-select SQL should be rejected by first-pass execution planning");
    expect(!plan.error.empty(), "failed execution planning should report an error message");
}

void test_plan_rejects_with_ai_policy_disabled() {
    const auto plan = copperfin::platform::build_federation_execution_plan({
        .backend = copperfin::platform::FederationBackend::oracle,
        .fox_sql = "DELETE FROM customer",
        .target = "",
        .planning_policy = {.enable_ai_assistance = false, .require_ai_assistance = false}
    });

    expect(!plan.ok, "AI-disabled policy should reject non-deterministic plan paths");
    expect(plan.planning_mode == "deterministic_rejected", "planning mode should be deterministic_rejected when AI is disabled");
    expect(!plan.ai_assisted, "AI-assistance should remain disabled");
    expect(!plan.planning_policy_allows_ai, "policy should report AI disallowed");
    expect(!plan.deterministic_translation_succeeded, "deterministic translation should be marked failed");
}

void test_plan_allows_ai_fallback_metadata() {
    const auto plan = copperfin::platform::build_federation_execution_plan({
        .backend = copperfin::platform::FederationBackend::oracle,
        .fox_sql = "DELETE FROM customer",
        .target = "",
        .planning_policy = {.enable_ai_assistance = true, .require_ai_assistance = false}
    });

    expect(!plan.ok, "AI-enabled policy should still fail while planner is not implemented");
    expect(plan.planning_mode == "ai_optional_fallback", "planning mode should describe optional AI fallback request");
    expect(plan.ai_assisted, "plan should mark AI assistance as requested when policy enables it");
    expect(plan.planning_policy_allows_ai, "policy should report AI as allowed");
    expect(!plan.deterministic_translation_succeeded, "deterministic translation should be marked failed");
    expect(plan.execution_command.find("connector.plan_query(") == 0, "fallback plan should expose connector plan_query intent");
    expect(
        plan.error ==
            "Planner is not yet implemented for optional AI policy. Deterministic translation failed: "
            "Only first-pass SELECT...FROM SQL translation is supported.",
        "#2389: optional AI fallback should preserve the default localized planner diagnostic");
}

void test_plan_requires_ai_fallback_metadata() {
    const auto plan = copperfin::platform::build_federation_execution_plan({
        .backend = copperfin::platform::FederationBackend::oracle,
        .fox_sql = "DELETE FROM customer",
        .target = "",
        .planning_policy = {.enable_ai_assistance = true, .require_ai_assistance = true}
    });

    expect(!plan.ok, "required AI policy should also route to AI fallback metadata");
    expect(plan.planning_mode == "ai_required_fallback", "planning mode should describe required AI fallback request");
    expect(plan.ai_assisted, "required AI policy should mark AI assistance as requested");
    expect(plan.planning_policy_allows_ai, "required AI policy should report AI allowed");
    expect(!plan.deterministic_translation_succeeded, "deterministic translation should still be marked failed");
    expect(plan.planning_policy_audit_enabled, "required AI policy should preserve default audit setting");
}

void test_platform_federation_diagnostics_resolve_through_localization_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    const copperfin::localization::PlaceholderMap placeholders{
        {"planMode", "required"},
        {"translationError", "Only first-pass SELECT...FROM SQL translation is supported."}
    };
    expect(
        english_catalog.translate("Platform.FederationExecution.Error.AiPlannerNotImplemented", placeholders) ==
            "Planner is not yet implemented for required AI policy. Deterministic translation failed: "
            "Only first-pass SELECT...FROM SQL translation is supported.",
        "#2389: federation diagnostics should preserve named placeholders");
    expect(
        pseudo_catalog.translate("Platform.FederationExecution.Error.AiPlannerNotImplemented", placeholders) !=
            english_catalog.translate("Platform.FederationExecution.Error.AiPlannerNotImplemented", placeholders),
        "#2389: federation diagnostics should be pseudo-localizable");
}

void test_platform_federation_diagnostics_refresh_when_locale_changes() {
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;
    ScopedEnvironmentValue locale_override("COPPERFIN_LOCALE", false);

    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const copperfin::localization::PlaceholderMap english_placeholders{
        {"planMode", "required"},
        {"translationError", english_catalog.translate("Platform.QueryTranslator.Error.SelectFromOnly")}
    };
    const copperfin::localization::PlaceholderMap spanish_placeholders{
        {"planMode", "required"},
        {"translationError", spanish_catalog.translate("Platform.QueryTranslator.Error.SelectFromOnly")}
    };

    locale_override.set("en-US");
    const auto english_plan = copperfin::platform::build_federation_execution_plan({
        .backend = copperfin::platform::FederationBackend::oracle,
        .fox_sql = "DELETE FROM customer",
        .target = "",
        .planning_policy = {.enable_ai_assistance = true, .require_ai_assistance = true}
    });
    expect(!english_plan.ok, "#3712: English federation fallback should fail while planner is unimplemented");
    expect(
        english_plan.error ==
            english_catalog.translate("Platform.FederationExecution.Error.AiPlannerNotImplemented", english_placeholders),
        "#3712: platform federation diagnostics should honor the initial locale");

    locale_override.set("es-419");
    const auto spanish_plan = copperfin::platform::build_federation_execution_plan({
        .backend = copperfin::platform::FederationBackend::oracle,
        .fox_sql = "DELETE FROM customer",
        .target = "",
        .planning_policy = {.enable_ai_assistance = true, .require_ai_assistance = true}
    });
    expect(!spanish_plan.ok, "#3712: Spanish federation fallback should fail while planner is unimplemented");
    expect(
        spanish_plan.error ==
            spanish_catalog.translate("Platform.FederationExecution.Error.AiPlannerNotImplemented", spanish_placeholders),
        "#3712: platform federation diagnostics should refresh after a locale change");
}

void test_plan_tracks_policy_audit_toggle() {
    const auto plan = copperfin::platform::build_federation_execution_plan({
        .backend = copperfin::platform::FederationBackend::sqlite,
        .fox_sql = "SELECT * FROM customer",
        .target = "",
        .planning_policy = {.enable_ai_assistance = false, .require_ai_assistance = false, .policy_audit_enabled = false}
    });

    expect(plan.planning_policy_audit_enabled == false, "planning policy audit flag should be preserved in the plan");
}

void test_plan_exposes_projection_fields() {
    const auto plan = copperfin::platform::build_federation_execution_plan({
        .backend = copperfin::platform::FederationBackend::postgresql,
        .fox_sql = "SELECT id, name AS customer_name, score + 1 AS projected_score FROM customer",
        .target = "analytics"
    });

    expect(plan.ok, "execution plan should succeed and expose projection metadata");
    if (plan.ok) {
        expect(plan.projection_fields.size() == 3U, "execution plan should expose three projection entries");
        expect(plan.projection_fields[1].alias == "customer_name", "execution plan should preserve AS alias");
        expect(plan.projection_fields[2].alias == "projected_score", "execution plan should preserve computed alias");
        expect(!plan.projection_fields[0].wildcard, "plain projected field should not be wildcard");
    }
}

}  // namespace

int main() {
    test_backend_parsing();
    test_plan_generation();
    test_plan_rejection();
    test_plan_rejects_with_ai_policy_disabled();
    test_plan_allows_ai_fallback_metadata();
    test_plan_requires_ai_fallback_metadata();
    test_platform_federation_diagnostics_resolve_through_localization_catalog();
    test_platform_federation_diagnostics_refresh_when_locale_changes();
    test_plan_tracks_policy_audit_toggle();
    test_plan_exposes_projection_fields();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
