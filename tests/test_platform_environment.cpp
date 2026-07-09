// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/platform/environment.h"
#include "test_environment_support.h"

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

void test_platform_environment_round_trips_values() {
    const std::string key = "COPPERFIN_TEST_PLATFORM_ENVIRONMENT";
    copperfin::test_support::ScopedEnvironmentValue scoped(key);

    expect(!copperfin::platform::read_environment_variable(key).has_value(),
           "#3214: clearing a test environment variable should leave no readable value");
    expect(copperfin::platform::write_environment_variable(key, "alpha beta"),
           "#3214: shared platform environment helper should set variables");

    const auto assigned = copperfin::platform::read_environment_variable(key);
    expect(assigned.has_value() && *assigned == "alpha beta",
           "#3214: shared platform environment helper should read back assigned values");

    expect(copperfin::platform::clear_environment_variable(key),
           "#3214: shared platform environment helper should clear variables");
    expect(!copperfin::platform::read_environment_variable(key).has_value(),
           "#3214: cleared environment variables should not report stale values");
}

void test_platform_environment_rejects_empty_names() {
    expect(!copperfin::platform::read_environment_variable("").has_value(),
           "#3214: shared platform environment helper should reject empty read keys");
    expect(!copperfin::platform::write_environment_variable("", "value"),
           "#3214: shared platform environment helper should reject empty write keys");
    expect(!copperfin::platform::clear_environment_variable(""),
           "#3214: shared platform environment helper should reject empty clear keys");
}

void test_scoped_environment_support_uses_shared_platform_helpers() {
    const std::string key = "COPPERFIN_TEST_PLATFORM_ENVIRONMENT_SCOPED";
    copperfin::test_support::ScopedEnvironmentValue original(key);
    expect(copperfin::platform::write_environment_variable(key, "original"),
           "#3214: shared platform environment helper should seed scoped test fixtures");

    {
        copperfin::test_support::ScopedEnvironmentValue override_value(key, "override");
        const auto during_scope = copperfin::platform::read_environment_variable(key);
        expect(during_scope.has_value() && *during_scope == "override",
               "#3214: scoped test environment helper should expose the temporary override");
    }

    const auto restored = copperfin::platform::read_environment_variable(key);
    expect(restored.has_value() && *restored == "original",
           "#3214: scoped test environment helper should restore the original environment value");
}

}  // namespace

int main() {
    test_platform_environment_round_trips_values();
    test_platform_environment_rejects_empty_names();
    test_scoped_environment_support_uses_shared_platform_helpers();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
