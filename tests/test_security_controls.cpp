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

// #247 [gap-06a]
void test_authorization_unknown_role_returns_false() {
    const auto profile = copperfin::security::default_native_security_profile();
    expect(
        !copperfin::security::role_has_permission(profile, "completely-unknown-role", "build.execute"),
        "unknown role should return false from role_has_permission");
    expect(
        !copperfin::security::role_has_permission(profile, "", "build.execute"),
        "empty role id should return false from role_has_permission");
}

// #255 [gap-06b]
void test_authorization_empty_permission_returns_false() {
    const auto profile = copperfin::security::default_native_security_profile();
    expect(
        !copperfin::security::role_has_permission(profile, "build-engineer", ""),
        "empty permission id should return false even for a known role");
    expect(
        !copperfin::security::role_has_permission(profile, "build-engineer", "nonexistent.permission"),
        "nonexistent permission id should return false for a known role");
}

// #245 [gap-06c]
void test_secret_provider_missing_env_var_returns_not_ok() {
    // Ensure the variable is absent before testing.
    const std::string missing_var = "COPPERFIN_MISSING_SECRET_XYZ";
#ifdef _WIN32
    _putenv_s(missing_var.c_str(), "");
#else
    unsetenv(missing_var.c_str());
#endif

    const auto result = copperfin::security::resolve_secret_reference("env:" + missing_var);
    expect(!result.ok, "resolve_secret_reference should return not-ok for a missing environment variable");
    expect(!result.error.empty(), "resolve_secret_reference should provide an error message for a missing variable");
}

// #248 [gap-06d]
void test_audit_stream_tamper_detection() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_security_tamper_tests";
    const fs::path clean_log = temp_root / "clean" / "events.log";
    const fs::path tampered_log = temp_root / "tampered" / "events.log";

    std::error_code ignored;
    fs::remove_all(temp_root, ignored);

    // Build a clean two-entry chain and record the second hash.
    copperfin::security::append_immutable_audit_event(clean_log.string(), "op.one", "clean-detail-one");
    const auto clean_second = copperfin::security::append_immutable_audit_event(
        clean_log.string(), "op.two", "clean-detail-two");
    expect(clean_second.ok, "clean audit chain second append should succeed");

    // Build the same chain in the tampered log.
    copperfin::security::append_immutable_audit_event(tampered_log.string(), "op.one", "clean-detail-one");
    copperfin::security::append_immutable_audit_event(tampered_log.string(), "op.two", "clean-detail-two");

    // Corrupt the tampered log: replace one character in the first line.
    {
        std::ifstream src(tampered_log, std::ios::binary);
        std::string contents((std::istreambuf_iterator<char>(src)), std::istreambuf_iterator<char>());
        if (!contents.empty()) {
                // Corrupt the LAST line's hash field so that the next append picks up a different prev_hash.
                // The last line ends with '|<64-hex-chars>\n'; flip the first hex digit of that hash.
                const auto last_pipe = contents.rfind('|');
                if (last_pipe != std::string::npos && last_pipe + 1 < contents.size()) {
                    const std::size_t hash_pos = last_pipe + 1;
                    contents[hash_pos] = (contents[hash_pos] == 'f') ? '0' : 'f';
                }
        }
        std::ofstream dst(tampered_log, std::ios::trunc | std::ios::binary);
        dst << contents;
    }

    // Append a third entry to each log.
    const auto clean_third = copperfin::security::append_immutable_audit_event(
        clean_log.string(), "op.three", "detail-three");
    const auto tampered_third = copperfin::security::append_immutable_audit_event(
        tampered_log.string(), "op.three", "detail-three");

    expect(clean_third.ok, "clean third append should succeed");
    expect(tampered_third.ok, "tampered third append should still succeed (append does not verify)");

    // The two third-entry hashes must differ because the chain root was mutated.
    expect(
        clean_third.entry_hash != tampered_third.entry_hash,
        "audit chain hash should differ when an earlier entry was tampered");

    fs::remove_all(temp_root, ignored);
}

void test_audit_stream_chain_verification() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_security_audit_verify_tests";
    const fs::path good_log = temp_root / "good" / "events.log";
    const fs::path tampered_log = temp_root / "tampered" / "events.log";
    const fs::path deleted_line_log = temp_root / "deleted" / "events.log";

    std::error_code ignored;
    fs::remove_all(temp_root, ignored);

    const auto one = copperfin::security::append_immutable_audit_event(good_log.string(), "runtime.start", "good-one");
    const auto two = copperfin::security::append_immutable_audit_event(good_log.string(), "runtime.middle", "good-two");
    const auto three = copperfin::security::append_immutable_audit_event(good_log.string(), "runtime.finish", "good-three");
    expect(one.ok && two.ok && three.ok, "good audit chain append operations should succeed");
    if (one.ok && two.ok && three.ok) {
        const auto verify_good = copperfin::security::verify_immutable_audit_chain(good_log.string());
        expect(verify_good.ok, "verify_immutable_audit_chain should validate a clean log");
        expect(verify_good.entries == 3U, "verify_immutable_audit_chain should report all three entries");
    }

    const auto t1 = copperfin::security::append_immutable_audit_event(tampered_log.string(), "runtime.start", "tamper-one");
    expect(t1.ok, "tampered log first append should succeed");
    if (t1.ok) {
        const auto t2 = copperfin::security::append_immutable_audit_event(tampered_log.string(), "runtime.middle", "tamper-two");
        expect(t2.ok, "tampered log second append should succeed");
        if (t2.ok) {
            std::ifstream source(tampered_log, std::ios::binary);
            std::string text((std::istreambuf_iterator<char>(source)), std::istreambuf_iterator<char>());
            const auto hash_pos = text.rfind("tamper-two");
            if (hash_pos != std::string::npos) {
                // Corrupt the visible detail text; hash validation should fail.
                text[hash_pos] = 'x';
            }
            std::ofstream target(tampered_log, std::ios::trunc | std::ios::binary);
            target << text;
        }

        const auto verify_tampered = copperfin::security::verify_immutable_audit_chain(tampered_log.string());
        expect(!verify_tampered.ok, "verify_immutable_audit_chain should fail on tampered detail/hash input");
        expect(!verify_tampered.error.empty(), "audit verification error should be reported for tampered log");
    }

    const auto d1 = copperfin::security::append_immutable_audit_event(deleted_line_log.string(), "runtime.start", "delete-one");
    expect(d1.ok, "deleted-line test first append should succeed");
    if (d1.ok) {
        const auto d2 = copperfin::security::append_immutable_audit_event(deleted_line_log.string(), "runtime.middle", "delete-two");
        expect(d2.ok, "deleted-line test second append should succeed");
    }
    {
        std::ifstream source(deleted_line_log, std::ios::binary);
        std::string contents((std::istreambuf_iterator<char>(source)), std::istreambuf_iterator<char>());
        const auto line_break = contents.find('\n');
        if (line_break != std::string::npos) {
            contents.erase(0, line_break + 1U);
        }
        std::ofstream target(deleted_line_log, std::ios::trunc | std::ios::binary);
        target << contents;
    }
    const auto verify_deleted = copperfin::security::verify_immutable_audit_chain(deleted_line_log.string());
    expect(!verify_deleted.ok, "verify should fail when a middle audit line is removed");
    expect(!verify_deleted.error.empty(), "audit verification should report chain break for deleted line");

    fs::remove_all(temp_root, ignored);
}

// #252 [gap-06e]
void test_audit_stream_append_to_readonly_path_fails_gracefully() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_security_readonly_path_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    // Place a regular file where the audit log's parent directory must be created.
    // create_directories will fail because "not_a_dir" is a file, not a directory.
    const fs::path blocker = temp_root / "not_a_dir";
    {
        std::ofstream f(blocker);
        f << "blocker\n";
    }
    const fs::path blocked_log = blocker / "events.log";

    const auto result = copperfin::security::append_immutable_audit_event(
        blocked_log.string(), "op.blocked", "should-not-appear");

    expect(!result.ok, "append_immutable_audit_event should return not-ok when directory creation fails");
    expect(!result.error.empty(), "append_immutable_audit_event should report an error for an unwritable path");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_authorization();
    test_secret_provider();
    test_audit_stream_chain();
    test_sha256_helpers();
    test_authorization_unknown_role_returns_false();
    test_authorization_empty_permission_returns_false();
    test_secret_provider_missing_env_var_returns_not_ok();
    test_audit_stream_tamper_detection();
    test_audit_stream_chain_verification();
    test_audit_stream_append_to_readonly_path_fails_gracefully();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
