// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/localization/localization.h"
#include "copperfin/platform/query_translator.h"
#include "test_environment_support.h"
#include "test_locale_catalog_environment_support.h"

#include <cstdlib>
#include <iostream>

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

void test_basic_translation() {
    const auto sqlite = copperfin::platform::translate_fox_sql_to_backend(
        copperfin::platform::FederationBackend::sqlite,
        "SELECT * FROM customer WHERE active = .T.");

    expect(sqlite.ok, "sqlite translation should succeed for first-pass select SQL");
    if (sqlite.ok) {
        expect(sqlite.translated_sql.find("TRUE") != std::string::npos, "sqlite translation should normalize boolean literals");
    }

    const auto sqlserver = copperfin::platform::translate_fox_sql_to_backend(
        copperfin::platform::FederationBackend::sqlserver,
        "SELECT SUBSTR(name, 1, 2) FROM customer");
    expect(sqlserver.ok, "sqlserver translation should succeed");
    if (sqlserver.ok) {
        expect(sqlserver.translated_sql.find("SUBSTRING(") != std::string::npos, "sqlserver translation should map SUBSTR to SUBSTRING");
    }

    const auto rejected = copperfin::platform::translate_fox_sql_to_backend(
        copperfin::platform::FederationBackend::postgresql,
        "DELETE FROM customer");
    expect(!rejected.ok, "non-select SQL should be rejected in first-pass deterministic translator");
    expect(
        rejected.error == "Only first-pass SELECT...FROM SQL translation is supported.",
        "#2389: query translator rejection should preserve the default localized diagnostic");
}

void test_platform_query_diagnostics_resolve_through_localization_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english_catalog.translate("Platform.QueryTranslator.Error.SelectFromOnly") ==
            "Only first-pass SELECT...FROM SQL translation is supported.",
        "#2389: query translator diagnostics should resolve through the en-US catalog");
    expect(
        spanish_catalog.translate("Platform.QueryTranslator.Error.SelectFromOnly") ==
            "Solo se admite la traduccion SQL deterministica de primera pasada de SELECT...FROM.",
        "#2594: query translator diagnostics should resolve through the es-419 catalog");
    expect(
        portuguese_catalog.translate("Platform.QueryTranslator.Error.SelectFromOnly") ==
            "Somente a traducao SQL deterministica de primeira passagem de SELECT...FROM e suportada.",
        "#2594: query translator diagnostics should resolve through the pt-BR catalog");
    expect(
        pseudo_catalog.translate("Platform.QueryTranslator.Error.SelectFromOnly") !=
            english_catalog.translate("Platform.QueryTranslator.Error.SelectFromOnly"),
        "#2389: platform query diagnostics should be pseudo-localizable");
    expect(
        pseudo_catalog.translate("Platform.QueryTranslator.Error.SelectFromOnly") ==
            copperfin::localization::pseudo_localize(
                "Only first-pass SELECT...FROM SQL translation is supported."),
        "#2594: pseudo-localized query translator diagnostics should resolve through the pseudo-localization transform");
}

void test_platform_query_diagnostics_refresh_when_locale_changes() {
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;
    ScopedEnvironmentValue locale_override("COPPERFIN_LOCALE", false);

    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");

    locale_override.set("en-US");
    const auto english_rejected = copperfin::platform::translate_fox_sql_to_backend(
        copperfin::platform::FederationBackend::postgresql,
        "DELETE FROM customer");
    expect(!english_rejected.ok, "#3712: English query rejection should fail deterministically");
    expect(
        english_rejected.error ==
            english_catalog.translate("Platform.QueryTranslator.Error.SelectFromOnly"),
        "#3712: platform query diagnostics should honor the initial locale");

    locale_override.set("es-419");
    const auto spanish_rejected = copperfin::platform::translate_fox_sql_to_backend(
        copperfin::platform::FederationBackend::postgresql,
        "DELETE FROM customer");
    expect(!spanish_rejected.ok, "#3712: Spanish query rejection should fail deterministically");
    expect(
        spanish_rejected.error ==
            spanish_catalog.translate("Platform.QueryTranslator.Error.SelectFromOnly"),
        "#3712: platform query diagnostics should refresh after a locale change");
}

void test_case_variants_and_whitespace_variants() {
    const auto sqlite = copperfin::platform::translate_fox_sql_to_backend(
        copperfin::platform::FederationBackend::sqlite,
        "select alltrim ( name ) from customer where active = .t. and inactive = .F.");

    expect(sqlite.ok, "lower-case select and spaced alltrim should still translate");
    if (sqlite.ok) {
        expect(sqlite.translated_sql.find("TRIM(") != std::string::npos,
               "case-insensitive and whitespace-tolerant ALLTRIM should map to TRIM");
        expect(sqlite.translated_sql.find("TRUE") != std::string::npos,
               "lowercase .t. literal should map to TRUE");
        expect(sqlite.translated_sql.find("FALSE") != std::string::npos,
               "uppercase .F. literal should map to FALSE");
    }

    const auto sqlserver = copperfin::platform::translate_fox_sql_to_backend(
        copperfin::platform::FederationBackend::sqlserver,
        "SeLeCt sUbStR(name, 1, 2) FROM customer");
    expect(sqlserver.ok, "case-mixed backend function names should still translate");
    if (sqlserver.ok) {
        expect(sqlserver.translated_sql.find("SUBSTRING(") != std::string::npos,
               "mixed-case SUBSTR should map to SUBSTRING for sqlserver");
    }
}

void test_ascii_case_matching_preserves_utf8_query_bytes() {
    const std::string utf8_identifier = "caf\xC3\xA9";
    const std::string sql =
        "select alltrim(" + utf8_identifier + ") AS display_name FROM customer "
        "WHERE note = '" + utf8_identifier + "'";
    const auto translated = copperfin::platform::translate_fox_sql_to_backend(
        copperfin::platform::FederationBackend::sqlite,
        sql);

    expect(translated.ok, "#3973: UTF-8 query bytes should survive ASCII keyword matching");
    if (!translated.ok) {
        return;
    }
    expect(
        translated.translated_sql.find("TRIM(" + utf8_identifier + ")") != std::string::npos,
        "#3973: ASCII function translation should preserve UTF-8 identifier bytes");
    expect(
        translated.translated_sql.find("'" + utf8_identifier + "'") != std::string::npos,
        "#3973: ASCII case scanning should preserve UTF-8 literal bytes");

    const auto suffixed_select = copperfin::platform::translate_fox_sql_to_backend(
        copperfin::platform::FederationBackend::sqlite,
        "SELECT" + utf8_identifier + " * FROM customer");
    expect(!suffixed_select.ok,
           "#3973: SELECT should not match inside a UTF-8-suffixed identifier token");
    const auto suffixed_from = copperfin::platform::translate_fox_sql_to_backend(
        copperfin::platform::FederationBackend::sqlite,
        "SELECT * FROM" + utf8_identifier + " customer");
    expect(!suffixed_from.ok,
           "#3973: FROM should not match inside a UTF-8-suffixed identifier token");
}

void test_iif_is_translated_to_case_when() {
    const auto translated = copperfin::platform::translate_fox_sql_to_backend(
        copperfin::platform::FederationBackend::sqlite,
        "SELECT IIF(active = .T., 'ENABLED', 'DISABLED') AS status FROM customer");

    expect(translated.ok, "IIF call should translate in sqlite query");
    if (translated.ok) {
        expect(translated.translated_sql.find("CASE WHEN active = TRUE THEN 'ENABLED' ELSE 'DISABLED' END") != std::string::npos,
               "IIF should emit CASE WHEN cond THEN true_expr ELSE false_expr END");
    }
}

void test_nested_iif_is_translated_recursively() {
    const auto translated = copperfin::platform::translate_fox_sql_to_backend(
        copperfin::platform::FederationBackend::postgresql,
        "SELECT IIF(active = .T., IIF(score > 10, 'HIGH', 'LOW'), 'UNKNOWN') AS level FROM customer");

    expect(translated.ok, "nested IIF should translate in first-pass SQL");
    if (translated.ok) {
        expect(translated.translated_sql.find("CASE WHEN active = TRUE THEN CASE WHEN score > 10 THEN 'HIGH' ELSE 'LOW' END ELSE 'UNKNOWN' END") != std::string::npos,
               "nested IIF should remain a full CASE expression after translation");
    }
}

void test_literals_and_functions_are_not_rewritten_in_string_literals() {
    const auto translated = copperfin::platform::translate_fox_sql_to_backend(
        copperfin::platform::FederationBackend::postgresql,
        "SELECT IIF(active = .T., ALLTRIM(name), \"quoted .T. text\") AS status, 'ALLTRIM(' || notes || ')' AS note FROM customer WHERE note = '.F.' AND COALESCE(name, '') = ''");

    expect(translated.ok, "quoted-token regression SQL should translate");
    if (!translated.ok)
    {
        return;
    }

    expect(translated.translated_sql.find("CASE WHEN active = TRUE THEN TRIM(name) ELSE \"quoted .T. text\" END AS status") != std::string::npos,
           "outside quotes should still rewrite IIF and ALLTRIM");
    expect(translated.translated_sql.find("note = '.F.'") != std::string::npos,
           "quoted .F. inside single-quoted literal should be preserved");
    expect(translated.translated_sql.find("\"quoted .T. text\"") != std::string::npos,
           "quoted .T. inside double-quoted literal should be preserved");
    expect(translated.translated_sql.find("'ALLTRIM(' || notes || ')'") != std::string::npos,
           "quoted ALLTRIM token inside single quotes should be preserved");
}

void test_legacy_function_compatibility_across_backends() {
    const auto sqlite = copperfin::platform::translate_fox_sql_to_backend(
        copperfin::platform::FederationBackend::sqlite,
        "SELECT LEN(name), IFNULL(alias, 'n/a') FROM customer WHERE alias <> ''");

    expect(sqlite.ok, "sqlite compatibility translation should support LEN + IFNULL");
    if (sqlite.ok) {
        expect(sqlite.translated_sql.find("LENGTH(") != std::string::npos,
               "LEN() should map to LENGTH() for sqlite");
        expect(sqlite.translated_sql.find("COALESCE(") != std::string::npos,
               "IFNULL() should map to COALESCE() for sqlite");
    }

    const auto postgres = copperfin::platform::translate_fox_sql_to_backend(
        copperfin::platform::FederationBackend::postgresql,
        "SELECT LEN(name), ISNULL(alias, 'n/a') FROM customer");

    expect(postgres.ok, "postgresql compatibility translation should support LEN + ISNULL");
    if (postgres.ok) {
        expect(postgres.translated_sql.find("LENGTH(") != std::string::npos,
               "LEN() should map to LENGTH() for postgres");
        expect(postgres.translated_sql.find("COALESCE(") != std::string::npos,
               "ISNULL() should map to COALESCE() for postgres");
    }

    const auto sqlserver = copperfin::platform::translate_fox_sql_to_backend(
        copperfin::platform::FederationBackend::sqlserver,
        "SELECT NVL(name, 'none'), IFNULL(alias, 'n/a') FROM customer");

    expect(sqlserver.ok, "sqlserver compatibility translation should support NVL + IFNULL");
    if (sqlserver.ok) {
        expect(sqlserver.translated_sql.find("ISNULL(") != std::string::npos,
               "NVL() and IFNULL() should map to ISNULL() for sqlserver");
    }

    const auto oracle = copperfin::platform::translate_fox_sql_to_backend(
        copperfin::platform::FederationBackend::oracle,
        "SELECT LEN(name), IFNULL(alias, 'n/a') FROM customer");

    expect(oracle.ok, "oracle compatibility translation should support LEN + IFNULL");
    if (oracle.ok) {
        expect(oracle.translated_sql.find("LENGTH(") != std::string::npos,
               "LEN() should map to LENGTH() for oracle");
        expect(oracle.translated_sql.find("NVL(") != std::string::npos,
               "IFNULL() should map to NVL() for oracle");
    }
}

void test_boolean_literals_are_dialect_sensitive() {
    const auto sqlserver = copperfin::platform::translate_fox_sql_to_backend(
        copperfin::platform::FederationBackend::sqlserver,
        "SELECT * FROM customer WHERE active = .T. AND archived = .F.");

    expect(sqlserver.ok, "sqlserver translation should preserve booleans in deterministic form");
    if (sqlserver.ok) {
        expect(sqlserver.translated_sql.find("active = 1") != std::string::npos,
               "sqlserver translator should emit numeric true literal");
        expect(sqlserver.translated_sql.find("archived = 0") != std::string::npos,
               "sqlserver translator should emit numeric false literal");
        expect(sqlserver.translated_sql.find("TRUE") == std::string::npos,
               "sqlserver output should avoid generic TRUE literal");
        expect(sqlserver.translated_sql.find("FALSE") == std::string::npos,
               "sqlserver output should avoid generic FALSE literal");
    }
}

void test_projection_fields_are_extracted() {
    const auto translated = copperfin::platform::translate_fox_sql_to_backend(
        copperfin::platform::FederationBackend::sqlite,
        "SELECT id, name AS customer_name, score + 1 AS projected_score FROM customer");

    expect(translated.ok, "projection extraction should preserve supported SELECT syntax");
    if (!translated.ok) {
        return;
    }

    expect(translated.projection_fields.size() == 3U, "projection extraction should return three fields");
    if (translated.projection_fields.size() == 3U) {
        expect(translated.projection_fields[0].expression == "id",
               "first projection should preserve raw expression for plain field");
        expect(translated.projection_fields[0].alias.empty(), "plain projected field should have no alias");
        expect(!translated.projection_fields[0].wildcard, "plain field projection should not be wildcard");

        expect(translated.projection_fields[1].expression == "name",
               "explicit AS projection expression should remain expression only");
        expect(translated.projection_fields[1].alias == "customer_name",
               "AS projection alias should be preserved");

        expect(translated.projection_fields[2].expression == "score + 1",
               "inline expression should remain the projection expression");
        expect(translated.projection_fields[2].alias == "projected_score",
               "inline alias should be preserved");
        expect(!translated.projection_fields[2].wildcard, "computed projection should not be wildcard");
    }
}

void test_projection_metadata_handles_wildcard() {
    const auto translated = copperfin::platform::translate_fox_sql_to_backend(
        copperfin::platform::FederationBackend::sqlite,
        "SELECT * FROM customer");

    expect(translated.ok, "wildcard projection should still translate");
    if (!translated.ok) {
        return;
    }

    expect(translated.projection_fields.size() == 1U, "wildcard query should emit one projection field");
    if (translated.projection_fields.size() == 1U) {
        expect(translated.projection_fields[0].wildcard, "wildcard query should set wildcard flag");
        expect(translated.projection_fields[0].alias.empty(), "wildcard projection should have empty alias");
    }
}

}  // namespace

int main() {
    test_basic_translation();
    test_platform_query_diagnostics_resolve_through_localization_catalog();
    test_platform_query_diagnostics_refresh_when_locale_changes();
    test_case_variants_and_whitespace_variants();
    test_ascii_case_matching_preserves_utf8_query_bytes();
    test_iif_is_translated_to_case_when();
    test_nested_iif_is_translated_recursively();
    test_literals_and_functions_are_not_rewritten_in_string_literals();
    test_legacy_function_compatibility_across_backends();
    test_boolean_literals_are_dialect_sensitive();
    test_projection_fields_are_extracted();
    test_projection_metadata_handles_wildcard();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
