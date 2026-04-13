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

}  // namespace

int main() {
    test_basic_translation();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
