#include "copperfin/security/audit_stream.h"
#include "copperfin/security/authorization.h"
#include "copperfin/security/secret_provider.h"
#include "copperfin/security/security_model.h"
#include "copperfin/security/sha256.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void test_authorization() {
    const auto profile = copperfin::security::default_native_security_profile();
    expect(
        copperfin::security::role_has_permission(profile, "build-engineer", "build.execute"),
        "build-engineer should have build.execute permission");
    expect(
        !copperfin::security::role_has_permission(profile, "developer", "build.release"),
        "developer should not have build.release permission");
}

void test_secret_provider() {
#ifdef _WIN32
    _putenv_s("COPPERFIN_TEST_SECRET", "alpha-secret");
#else
    setenv("COPPERFIN_TEST_SECRET", "alpha-secret", 1);
#endif

    const auto secret = copperfin::security::resolve_secret_reference("env:COPPERFIN_TEST_SECRET");
    expect(secret.ok, "secret provider should resolve env references");
    if (secret.ok) {
        expect(secret.value == "alpha-secret", "resolved secret value should match environment value");
    }

    const auto invalid = copperfin::security::resolve_secret_reference("plain-text-secret");
    expect(!invalid.ok, "secret provider should reject non-provider references");
}

void test_audit_stream_chain() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_security_control_tests";
    const fs::path audit_path = temp_root / "audit" / "events.log";

    std::error_code ignored;
    fs::remove_all(temp_root, ignored);

    const auto first = copperfin::security::append_immutable_audit_event(
        audit_path.string(),
        "runtime.start",
        "unit-test-entry-one");
    expect(first.ok, "first audit event append should succeed");

    const auto second = copperfin::security::append_immutable_audit_event(
        audit_path.string(),
        "runtime.complete",
        "unit-test-entry-two");
    expect(second.ok, "second audit event append should succeed");

    std::ifstream input(audit_path, std::ios::binary);
    std::string line;
    int line_count = 0;
    while (std::getline(input, line)) {
        if (!line.empty()) {
            ++line_count;
        }
    }
    expect(line_count == 2, "audit stream should persist appended events");

    fs::remove_all(temp_root, ignored);
}

void test_sha256_helpers() {
    const auto digest = copperfin::security::sha256_hex_for_text("copperfin-security");
    expect(digest.ok, "sha256 text digest should succeed");
    if (digest.ok) {
        expect(!digest.hex_digest.empty(), "sha256 digest should not be empty");
        expect(digest.hex_digest.size() == 64U, "sha256 hex digest should be 64 characters");
    }
}

}  // namespace

int main() {
    test_authorization();
    test_secret_provider();
    test_audit_stream_chain();
    test_sha256_helpers();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
