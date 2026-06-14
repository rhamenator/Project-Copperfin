#include "copperfin/platform/query_translator.h"

#include <cstdlib>
#include <iostream>

namespace {

int failures = 0;

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

}  // namespace

int main() {
    test_basic_translation();
    test_case_variants_and_whitespace_variants();
    test_iif_is_translated_to_case_when();
    test_nested_iif_is_translated_recursively();
    test_literals_and_functions_are_not_rewritten_in_string_literals();
    test_legacy_function_compatibility_across_backends();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
