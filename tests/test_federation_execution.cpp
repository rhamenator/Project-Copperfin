#include "copperfin/platform/federation_execution.h"

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

}  // namespace

int main() {
    test_backend_parsing();
    test_plan_generation();
    test_plan_rejection();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}