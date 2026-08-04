// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/localization/localization.h"
#include "copperfin/platform/path.h"
#include "copperfin/security/audit_stream.h"
#include "copperfin/security/authorization.h"
#include "copperfin/security/external_process_policy.h"
#include "copperfin/security/physical_path_containment.h"
#include "copperfin/security/process_hardening.h"
#include "copperfin/security/secret_provider.h"
#include "copperfin/security/security_model.h"
#include "copperfin/security/sha256.h"
#include "test_environment_support.h"

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <cwctype>
#include <filesystem>
#include <functional>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

#if defined(_WIN32)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace {

using copperfin::test_support::ScopedEnvironmentValue;
using copperfin::test_support::set_env_value;

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

std::string read_file_bytes(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return std::string(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>());
}

void write_file_bytes(const std::filesystem::path& path, const std::string& bytes) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
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
    ScopedEnvironmentValue secret_value("COPPERFIN_TEST_SECRET", "alpha-secret");

    const auto secret = copperfin::security::resolve_secret_reference("env:COPPERFIN_TEST_SECRET");
    expect(secret.ok, "secret provider should resolve env references");
    if (secret.ok) {
        expect(secret.value == "alpha-secret", "resolved secret value should match environment value");
    }

    const auto invalid = copperfin::security::resolve_secret_reference("plain-text-secret");
    expect(!invalid.ok, "secret provider should reject non-provider references");
    expect(
        invalid.error == "Secret reference must use env:<NAME> format.",
        "#2388: secret provider should preserve the default localized invalid-reference diagnostic");
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

void test_audit_stream_preserves_unicode_paths() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() /
        copperfin::platform::path_from_utf8_string(
            "copperfin_security_unicode_audit_tests-\xC3\xA9-\xF0\x9F\x9A\x80");
    const fs::path package_root = temp_root / "package";
    const fs::path contained_log = package_root / "logs" /
        copperfin::platform::path_from_utf8_string(
            "security-\xC3\xA9-\xF0\x9F\x9A\x80.log");
    const fs::path direct_log = temp_root / "direct" / "events.log";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(package_root);

    const auto contained = copperfin::security::append_immutable_audit_event_to_contained_file(
        copperfin::platform::path_to_utf8_string(contained_log),
        copperfin::platform::path_to_utf8_string(package_root),
        "runtime.unicode",
        "contained");
    expect(contained.ok, "contained audit append should preserve Unicode package and log paths");
    const auto contained_chain = copperfin::security::verify_immutable_audit_chain(
        copperfin::platform::path_to_utf8_string(contained_log));
    expect(contained_chain.ok && contained_chain.entries == 1U,
           "contained Unicode audit log should retain a valid chain");
    expect(read_file_bytes(contained_log).find("|runtime.unicode|contained|") != std::string::npos,
           "contained Unicode audit log should persist its invariant event fields");

    const auto direct = copperfin::security::append_immutable_audit_event(
        copperfin::platform::path_to_utf8_string(direct_log),
        "runtime.direct",
        "direct");
    expect(direct.ok, "direct audit append should preserve a Unicode parent path");
    const auto direct_chain = copperfin::security::verify_immutable_audit_chain(
        copperfin::platform::path_to_utf8_string(direct_log));
    expect(direct_chain.ok && direct_chain.entries == 1U,
           "direct Unicode audit log should retain a valid chain");

    fs::remove_all(temp_root, ignored);
}

void test_sha256_helpers() {
    const auto digest = copperfin::security::sha256_hex_for_text("copperfin-security");
    expect(digest.ok, "sha256 text digest should succeed");
    if (digest.ok) {
        expect(!digest.hex_digest.empty(), "sha256 digest should not be empty");
        expect(digest.hex_digest.size() == 64U, "sha256 hex digest should be 64 characters");
    }

    const auto missing_file = copperfin::security::sha256_hex_for_file("missing-security-hash-input.bin");
    expect(!missing_file.ok, "sha256 file digest should fail for missing input");
    expect(
        missing_file.error == "Unable to open file for SHA-256: missing-security-hash-input.bin",
        "#2388: SHA-256 file diagnostics should preserve the default localized missing-file message");
}

void test_security_diagnostics_resolve_through_localization_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english_catalog.translate(
            "Security.Secret.Error.EnvironmentVariableNotFound",
            {{"variableName", "COPPERFIN_TEST_MISSING_SECRET"}}) ==
            "Secret environment variable was not found: COPPERFIN_TEST_MISSING_SECRET",
        "#2388: secret diagnostics should preserve named placeholders");
    expect(
        english_catalog.translate("Security.Audit.Error.MalformedLine", {{"lineNumber", "7"}}) ==
            "Malformed audit line 7",
        "#2388: audit diagnostics should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Security.Audit.Error.InvalidExistingLogTail") ==
            "Existing audit log has an invalid or truncated tail.",
        "#3970: invalid existing audit-tail diagnostics should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Security.Audit.Error.ReadExistingLogFailed") ==
            "Unable to read existing audit log.",
        "#3970: existing audit-log read diagnostics should resolve through the en-US catalog");
    expect(
        english_catalog.translate(
            "Security.ExternalProcessPolicy.Error.ResolveExecutableOnPathFailed",
            {{"executableName", "dotnet"}}) ==
            "Unable to resolve executable on PATH: dotnet",
        "#2388: external process diagnostics should preserve executable placeholders");
    expect(
        spanish_catalog.translate("Security.ProcessHardening.Status.NoopOutsideWindows") ==
            "El hardening de procesos actualmente es un no-op fuera de Windows.",
        "#2601: es-419 process-hardening status should localize the prose");
    expect(
        spanish_catalog.translate("Security.Secret.Error.InvalidReferenceFormat") ==
            "La referencia del secreto debe usar el formato env:<NAME>.",
        "#2601: es-419 secret reference format error should localize the prose");
    expect(
        portuguese_catalog.translate("Security.Audit.Error.MalformedLine", {{"lineNumber", "7"}}) ==
            "Linha de auditoria malformada 7",
        "#2601: pt-BR audit malformed-line error should localize the prose");
    expect(
        portuguese_catalog.translate("Security.Sha256.Error.OpenFileFailed", {{"path", "missing.bin"}}) ==
            "Nao foi possivel abrir o arquivo para SHA-256: missing.bin",
        "#2601: pt-BR SHA-256 open-file error should localize the prose while preserving the path");
    expect(
        pseudo_catalog.translate("Security.ProcessHardening.Status.NoopOutsideWindows") !=
            english_catalog.translate("Security.ProcessHardening.Status.NoopOutsideWindows"),
        "#2388: security diagnostics should be pseudo-localizable");
    expect(
        pseudo_catalog.translate("Security.Audit.Error.InvalidExistingLogTail") !=
            english_catalog.translate("Security.Audit.Error.InvalidExistingLogTail"),
        "#3970: invalid existing audit-tail diagnostics should be pseudo-localizable");
}

void test_security_diagnostics_follow_selected_locale() {
    {
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
        set_env_value("COPPERFIN_LOCALE", "es-419", true);
        const auto invalid = copperfin::security::resolve_secret_reference("plain-text-secret");
        expect(!invalid.ok, "#2601: es-419 invalid secret reference should still fail");
        expect(
            invalid.error == "La referencia del secreto debe usar el formato env:<NAME>.",
            "#2601: es-419 invalid secret reference should route through the selected locale");
    }

    {
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
        set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
        const auto missing_file = copperfin::security::sha256_hex_for_file("missing-security-hash-input.bin");
        expect(!missing_file.ok, "#2601: pt-BR SHA-256 missing-file lookup should still fail");
        expect(
            missing_file.error == "Nao foi possivel abrir o arquivo para SHA-256: missing-security-hash-input.bin",
            "#2601: pt-BR SHA-256 missing-file error should route through the selected locale");
    }

    {
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
        set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
        const auto invalid = copperfin::security::resolve_secret_reference("plain-text-secret");
        expect(!invalid.ok, "#2601: qps-ploc invalid secret reference should still fail");
        expect(
            invalid.error.find("[!! ") == 0U &&
                invalid.error.find("Secret reference must use env:<NAME> format.") == std::string::npos,
            "#2601: qps-ploc invalid secret reference should pseudo-localize the prose");
    }
}

void test_security_profile_follows_selected_locale() {
    ScopedEnvironmentValue locale("COPPERFIN_LOCALE");

    set_env_value("COPPERFIN_LOCALE", "en-US", true);
    const auto english_profile = copperfin::security::default_native_security_profile();

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    const auto spanish_profile = copperfin::security::default_native_security_profile();

    const auto english_project_open = std::find_if(
        english_profile.permissions.begin(),
        english_profile.permissions.end(),
        [](const auto& permission) { return permission.id == "project.open"; });
    const auto spanish_project_open = std::find_if(
        spanish_profile.permissions.begin(),
        spanish_profile.permissions.end(),
        [](const auto& permission) { return permission.id == "project.open"; });
    expect(english_project_open != english_profile.permissions.end(),
           "en-US security profile should expose project.open");
    expect(spanish_project_open != spanish_profile.permissions.end(),
           "es-419 security profile should expose project.open");
    if (english_project_open != english_profile.permissions.end() &&
        spanish_project_open != spanish_profile.permissions.end()) {
        expect(english_project_open->title == "Open Project",
               "initial security profile should use the selected en-US catalog");
        expect(spanish_project_open->title == "Abrir proyecto",
               "subsequent security profile should follow an in-process locale change");
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
    ScopedEnvironmentValue missing_secret(missing_var);

    const auto result = copperfin::security::resolve_secret_reference("env:" + missing_var);
    expect(!result.ok, "resolve_secret_reference should return not-ok for a missing environment variable");
    expect(!result.error.empty(), "resolve_secret_reference should provide an error message for a missing variable");
    expect(
        result.error == "Secret environment variable was not found: " + missing_var,
        "#2388: missing secret diagnostics should preserve variable-name placeholders");
}

void test_secret_provider_rejects_malformed_env_var_names() {
    const auto whitespace = copperfin::security::resolve_secret_reference("env:COPPERFIN BAD SECRET");
    expect(!whitespace.ok, "resolve_secret_reference should reject whitespace in environment variable names");
    expect(whitespace.error.find("invalid characters") != std::string::npos,
           "malformed secret reference should report invalid-character diagnostics");

    const auto equals = copperfin::security::resolve_secret_reference("env:COPPERFIN=BAD");
    expect(!equals.ok, "resolve_secret_reference should reject '=' in environment variable names");
    expect(equals.error.find("invalid characters") != std::string::npos,
           "equals-sign secret reference should report invalid-character diagnostics");

    const auto control = copperfin::security::resolve_secret_reference(std::string("env:COPPERFIN\tBAD"));
    expect(!control.ok, "resolve_secret_reference should reject control characters in environment variable names");
    expect(control.error.find("invalid characters") != std::string::npos,
           "control-character secret reference should report invalid-character diagnostics");
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

void test_audit_stream_append_rejects_invalid_existing_tail() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_security_audit_tail_tests";
    const fs::path empty_log = temp_root / "empty" / "events.log";
    const fs::path blank_log = temp_root / "blank" / "events.log";
    const fs::path whitespace_log = temp_root / "whitespace" / "events.log";
    const fs::path blank_crlf_log = temp_root / "blank-crlf" / "events.log";
    const fs::path malformed_log = temp_root / "malformed" / "events.log";
    const fs::path truncated_log = temp_root / "truncated" / "events.log";
    const fs::path invalid_hash_log = temp_root / "invalid-hash" / "events.log";
    const fs::path invalid_previous_hash_log = temp_root / "invalid-previous" / "events.log";
    const fs::path localized_malformed_log = temp_root / "localized-malformed" / "events.log";
    const fs::path whitespace_injected_log = temp_root / "whitespace-injected" / "events.log";
    const fs::path unreadable_log = temp_root / "unreadable" / "events.log";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);

    write_file_bytes(empty_log, {});
    const auto empty_append = copperfin::security::append_immutable_audit_event(
        empty_log.string(), "runtime.empty", "empty-log");
    expect(empty_append.ok, "#3970: an existing zero-byte audit log should start at GENESIS");
    const auto empty_verify = copperfin::security::verify_immutable_audit_chain(empty_log.string());
    expect(empty_verify.ok && empty_verify.entries == 1U,
           "#3970: a first append to a zero-byte audit log should produce one valid entry");

    write_file_bytes(blank_log, "\n\n");
    const auto blank_append = copperfin::security::append_immutable_audit_event(
        blank_log.string(), "runtime.blank", "blank-log");
    expect(blank_append.ok, "#3970: a blank-only audit log should start at GENESIS");
    const auto blank_verify = copperfin::security::verify_immutable_audit_chain(blank_log.string());
    expect(blank_verify.ok && blank_verify.entries == 1U,
           "#3970: a first append after blank lines should produce one valid entry");

    const auto reject_unchanged = [&](
        const fs::path& path,
        const std::string& fixture,
        const std::string& label) {
        write_file_bytes(path, fixture);
        const std::string before = read_file_bytes(path);
        const auto result = copperfin::security::append_immutable_audit_event(
            path.string(), "runtime.rejected", label);
        expect(!result.ok, "#3970: " + label + " should reject audit append");
        expect(
            result.error == "Existing audit log has an invalid or truncated tail.",
            "#3970: " + label + " should use the localized invalid-tail diagnostic");
        expect(result.entry_hash.empty(), "#3970: " + label + " should not return a new entry hash");
        expect(read_file_bytes(path) == before, "#3970: " + label + " should leave the audit bytes unchanged");
    };

    reject_unchanged(malformed_log, "not|enough|fields\n", "malformed tail");
    reject_unchanged(whitespace_log, "  \t  ", "unterminated whitespace tail");
    reject_unchanged(blank_crlf_log, "\r\n\r\n", "CRLF whitespace tail");
    reject_unchanged(
        invalid_hash_log,
        "1|runtime.start|detail|GENESIS|not-a-sha256-hash\n",
        "invalid observed hash");
    reject_unchanged(
        invalid_previous_hash_log,
        "1|runtime.start|detail|not-a-previous-hash|" + std::string(64U, '0') + "\n",
        "invalid previous hash");

    write_file_bytes(localized_malformed_log, "not|enough|fields\n");
    const std::string localized_before = read_file_bytes(localized_malformed_log);
    {
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
        set_env_value("COPPERFIN_LOCALE", "es-419", true);
        const auto localized = copperfin::security::append_immutable_audit_event(
            localized_malformed_log.string(), "runtime.rejected", "localized-malformed-tail");
        expect(!localized.ok, "#3970: localized malformed-tail append should reject corruption");
        expect(
            localized.error == "El log de auditoria existente tiene una cola no valida o truncada.",
            "#3970: malformed-tail rejection should route through the selected locale");
        expect(localized.entry_hash.empty(),
               "#3970: localized malformed-tail rejection should not return an entry hash");
    }
    expect(read_file_bytes(localized_malformed_log) == localized_before,
           "#3970: localized malformed-tail rejection should leave audit bytes unchanged");

    const auto initial = copperfin::security::append_immutable_audit_event(
        truncated_log.string(), "runtime.start", "valid-before-truncation");
    expect(initial.ok, "#3970: truncated-tail fixture should begin with a valid event");
    if (initial.ok) {
        std::string truncated = read_file_bytes(truncated_log);
        if (!truncated.empty() && truncated.back() == '\n') {
            truncated.pop_back();
        }
        reject_unchanged(truncated_log, truncated, "non-newline-terminated tail");
    }

    const auto whitespace_fixture = copperfin::security::append_immutable_audit_event(
        whitespace_injected_log.string(), "runtime.start", "before-whitespace-injection");
    expect(whitespace_fixture.ok, "#3970: whitespace-injection fixture should begin with a valid event");
    if (whitespace_fixture.ok) {
        write_file_bytes(
            whitespace_injected_log,
            " \t\n" + read_file_bytes(whitespace_injected_log));
        const auto whitespace_verify = copperfin::security::verify_immutable_audit_chain(
            whitespace_injected_log.string());
        expect(!whitespace_verify.ok && whitespace_verify.entries == 0U,
               "#3970: verification should not ignore injected whitespace-only audit lines");
    }

    fs::create_directories(unreadable_log);
    const auto unreadable = copperfin::security::append_immutable_audit_event(
        unreadable_log.string(), "runtime.unreadable", "directory-at-log-path");
    expect(!unreadable.ok, "#3970: an unreadable existing audit path should reject append");
    expect(
        unreadable.error == "Unable to read existing audit log.",
        "#3970: an unreadable existing audit path should use the localized read diagnostic");
    expect(unreadable.entry_hash.empty(),
           "#3970: an unreadable existing audit path should not return a new entry hash");
    expect(fs::is_directory(unreadable_log) && fs::is_empty(unreadable_log),
           "#3970: unreadable existing audit rejection should leave the directory path unchanged");

    fs::remove_all(temp_root, ignored);
}

void test_audit_stream_localized_malformed_line_diagnostic() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_security_malformed_audit_tests";
    const fs::path malformed_log = temp_root / "audit" / "events.log";

    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(malformed_log.parent_path());

    {
        std::ofstream output(malformed_log, std::ios::binary);
        output << "not|enough|fields\n";
    }

    const auto result = copperfin::security::verify_immutable_audit_chain(malformed_log.string());
    expect(!result.ok, "verify_immutable_audit_chain should reject malformed audit lines");
    expect(
        result.error == "Malformed audit line 1",
        "#2388: audit verification should preserve the default localized malformed-line diagnostic");

    fs::remove_all(temp_root, ignored);
}

void test_contained_audit_append_rechecks_open_file_identity() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_contained_audit_append_tests";
    const fs::path package_root = temp_root / "package";
    const fs::path outside_root = temp_root / "outside";
    const fs::path outside_log = outside_root / "events.log";
    const fs::path hard_link_log = package_root / "hard-linked.log";
    const fs::path valid_log = package_root / "nested" / "events.log";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(package_root);
    fs::create_directories(outside_root);
    write_file_bytes(outside_log, {});

    std::error_code hard_link_error;
    fs::create_hard_link(outside_log, hard_link_log, hard_link_error);
    if (!hard_link_error) {
        const auto rejected = copperfin::security::append_immutable_audit_event_to_contained_file(
            hard_link_log.string(),
            package_root.string(),
            "runtime.rejected",
            "hard-link");
        expect(!rejected.ok,
               "#4015: contained audit append should reject a multiply linked file at open time");
        expect(read_file_bytes(outside_log).empty(),
               "#4015: append-time hard-link rejection should leave the external identity unchanged");
        fs::remove(hard_link_log, ignored);
    }

    const auto valid = copperfin::security::append_immutable_audit_event_to_contained_file(
        valid_log.string(),
        package_root.string(),
        "runtime.valid",
        "contained");
    expect(valid.ok,
           "#4015: contained audit append should create missing direct package directories and the audit leaf");
    const auto valid_chain = copperfin::security::verify_immutable_audit_chain(valid_log.string());
    expect(valid_chain.ok && valid_chain.entries == 1U,
           "#4015: contained audit append should preserve the immutable chain contract");

    fs::remove_all(temp_root, ignored);
}

void test_direct_audit_append_rejects_linked_leaf() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_direct_audit_link_tests";
    const fs::path outside_root = temp_root / "outside";
    const fs::path outside_log = outside_root / "events.log";
    const fs::path audit_root = temp_root / "audit";
    const fs::path hard_link_log = audit_root / "hard-linked.log";
    const fs::path symlink_log = audit_root / "symlinked.log";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(outside_root);
    fs::create_directories(audit_root);
    write_file_bytes(outside_log, {});

    std::error_code hard_link_error;
    fs::create_hard_link(outside_log, hard_link_log, hard_link_error);
    if (!hard_link_error) {
        const auto rejected = copperfin::security::append_immutable_audit_event(
            hard_link_log.string(),
            "build.rejected",
            "hard-link");
        expect(!rejected.ok,
               "#4375: direct audit append should reject a multiply linked leaf");
        expect(read_file_bytes(outside_log).empty(),
               "#4375: direct hard-link rejection should preserve the external file");
        fs::remove(hard_link_log, ignored);
    }

    std::error_code symlink_error;
    fs::create_symlink(outside_log, symlink_log, symlink_error);
    if (!symlink_error) {
        const auto rejected = copperfin::security::append_immutable_audit_event(
            symlink_log.string(),
            "build.rejected",
            "symlink");
        expect(!rejected.ok,
               "#4375: direct audit append should reject a symlinked leaf");
        expect(read_file_bytes(outside_log).empty(),
               "#4375: direct symlink rejection should preserve the external file");
    }

    fs::remove_all(temp_root, ignored);
}

void test_contained_audit_append_serializes_concurrent_writers() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_contained_audit_concurrency_tests";
    const fs::path package_root = temp_root / "package";
    const fs::path log_path = package_root / "audit" / "events.log";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(package_root);

    constexpr int writer_count = 4;
    constexpr int events_per_writer = 12;
    std::atomic<int> append_failures{0};
    std::vector<std::thread> writers;
    for (int writer = 0; writer < writer_count; ++writer) {
        writers.emplace_back([&, writer] {
            for (int event = 0; event < events_per_writer; ++event) {
                const auto result = copperfin::security::append_immutable_audit_event_to_contained_file(
                    log_path.string(),
                    package_root.string(),
                    "runtime.concurrent",
                    "writer=" + std::to_string(writer) + ",event=" + std::to_string(event));
                if (!result.ok) {
                    append_failures.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }
    for (auto& writer : writers) {
        writer.join();
    }

    expect(append_failures.load(std::memory_order_relaxed) == 0,
           "#4015: contained audit writers should serialize without append failures");
    const auto chain = copperfin::security::verify_immutable_audit_chain(log_path.string());
    expect(chain.ok && chain.entries == static_cast<std::size_t>(writer_count * events_per_writer),
           "#4015: concurrent contained audit writers should preserve every immutable chain entry");

    fs::remove_all(temp_root, ignored);
}

void test_external_process_and_process_hardening_diagnostics() {
#ifndef _WIN32
    const copperfin::security::ExternalProcessPolicy policy{
        .executable_name = "dotnet",
        .allowed_path_roots = {},
        .allowed_publishers = {},
        .require_trusted_signature = true
    };
    const auto authorization = copperfin::security::authorize_external_process(policy);
    expect(!authorization.allowed, "external process authorization should report unsupported platforms outside Windows");
    expect(
        authorization.error == "External process policy authorization is implemented for Windows only.",
        "#2388: external process policy should preserve the default localized non-Windows diagnostic");

    {
        ScopedEnvironmentValue unavailable_path("PATH");
        const copperfin::security::ExternalProcessPolicy default_path_policy{
            .executable_name = "sh",
            .allowed_path_roots = {"/bin", "/usr/bin"},
            .allowed_publishers = {},
            .require_trusted_signature = false
        };
        const auto default_path_authorization =
            copperfin::security::authorize_external_process(default_path_policy);
        expect(default_path_authorization.allowed,
               "#4372: unset POSIX PATH should retain default authorized executable discovery");
    }

    const auto hardening = copperfin::security::apply_default_process_hardening();
    expect(hardening.applied, "process hardening should be a no-op success outside Windows");
    expect(
        hardening.message == "Process hardening is currently a no-op outside Windows.",
        "#2388: process hardening should preserve the default localized non-Windows status");
#endif
}

#ifndef _WIN32
void test_external_process_authorization_rejects_replacement() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin_external_process_revalidation_tests";
    const fs::path fixture_path = temp_root / "authorized-tool";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    fs::copy_file("/bin/sh", fixture_path, fs::copy_options::overwrite_existing, ignored);
    if (ignored) {
        expect(false, "#4376: POSIX revalidation fixture should be created");
        fs::remove_all(temp_root, ignored);
        return;
    }
    fs::permissions(
        fixture_path,
        fs::perms::owner_exec | fs::perms::owner_read,
        fs::perm_options::add,
        ignored);

    const copperfin::security::ExternalProcessPolicy policy{
        .executable_name = fixture_path.string(),
        .allowed_path_roots = {temp_root.string()},
        .allowed_publishers = {},
        .require_trusted_signature = false
    };
    auto authorization = copperfin::security::authorize_external_process(policy);
    expect(authorization.allowed,
           "#4376: POSIX external-process authorization should accept the fixture");
    expect(
        copperfin::security::revalidate_external_process_authorization(authorization),
        "#4376: unchanged authorized executable should pass revalidation");

    const fs::path original_path = temp_root / "authorized-tool.original";
    const fs::path replacement_path = temp_root / "authorized-tool.replacement";
    fs::copy_file(fixture_path, replacement_path, fs::copy_options::overwrite_existing, ignored);
    if (!ignored) {
        fs::rename(fixture_path, original_path, ignored);
    }
    if (!ignored) {
        fs::rename(replacement_path, fixture_path, ignored);
    }
    expect(!ignored, "#4376: POSIX revalidation fixture should be replaceable");
    if (!ignored) {
        expect(
            !copperfin::security::revalidate_external_process_authorization(authorization),
            "#4376: replacing an authorized executable should fail revalidation");
        expect(
            authorization.error ==
                "The authorized executable changed before launch.",
            "#4376: replacement denial should use the localized diagnostic");
    }

    fs::remove_all(temp_root, ignored);
}
#endif

#ifdef _WIN32
void test_external_process_policy_preserves_unicode_paths() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() /
        copperfin::platform::path_from_utf8_string("copperfin_security_policy_\xC3\xA9");
    const fs::path fixture_directory = temp_root / "bin";
    const fs::path fixture_path = fixture_directory / "copperfin-policy-fixture.exe";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(fixture_directory);

    wchar_t system_directory[MAX_PATH]{};
    const UINT system_directory_length =
        GetSystemDirectoryW(system_directory, MAX_PATH);
    const fs::path system_command = system_directory_length == 0U
        ? fs::path{}
        : fs::path(std::wstring(system_directory, system_directory_length)) / L"cmd.exe";
    if (system_command.empty() || !fs::exists(system_command, ignored)) {
        expect(false, "#4277: Windows Unicode external-process fixture should find cmd.exe");
        fs::remove_all(temp_root, ignored);
        return;
    }
    fs::copy_file(system_command, fixture_path, fs::copy_options::overwrite_existing, ignored);
    if (ignored) {
        expect(false, "#4277: Windows Unicode external-process fixture should copy cmd.exe");
        fs::remove_all(temp_root, ignored);
        return;
    }

    const DWORD path_length = GetEnvironmentVariableW(L"PATH", nullptr, 0U);
    std::wstring original_path;
    if (path_length > 0U) {
        original_path.resize(static_cast<std::size_t>(path_length));
        const DWORD copied_length = GetEnvironmentVariableW(
            L"PATH",
            original_path.data(),
            path_length);
        original_path.resize(static_cast<std::size_t>(copied_length));
    }
    const std::wstring fixture_path_value = fixture_directory.wstring() + L";" + original_path;
    const bool path_set = SetEnvironmentVariableW(L"PATH", fixture_path_value.c_str()) != FALSE;
    if (!path_set) {
        expect(false, "#4277: Windows Unicode external-process fixture should update PATH");
        fs::remove_all(temp_root, ignored);
        return;
    }

    const copperfin::security::ExternalProcessPolicy policy{
        .executable_name = "copperfin-policy-fixture.exe",
        .allowed_path_roots = {copperfin::platform::path_to_utf8_string(temp_root)},
        .allowed_publishers = {},
        .require_trusted_signature = false
    };
    auto authorization = copperfin::security::authorize_external_process(policy);
    const auto resolved_path = copperfin::platform::path_from_utf8_string(authorization.resolved_path);
    const auto canonical_fixture_path = fs::weakly_canonical(fixture_path, ignored);
    expect(authorization.allowed,
           "#4277: Windows external-process policy should authorize a Unicode executable inside its allowed root");
    expect(
        copperfin::security::revalidate_external_process_authorization(authorization),
        "#4376: Windows external-process authorization should remain current before launch");
    expect(authorization.resolved_path.find("\xC3\xA9") != std::string::npos,
           "#4277: Windows external-process policy should preserve the non-ASCII path component in its result");
    expect(resolved_path == canonical_fixture_path,
           "#4277: Windows external-process policy should preserve canonical filesystem identity in its result");
    expect(fs::equivalent(resolved_path, fixture_path, ignored),
           "#4277: Windows external-process policy should resolve the Unicode fixture to the copied executable");

    std::string case_variant_root = copperfin::platform::path_to_utf8_string(temp_root);
    const std::string root_marker = "copperfin_security_policy_";
    const std::size_t root_marker_start = case_variant_root.find(root_marker);
    expect(root_marker_start != std::string::npos,
           "#4329: Windows external-process fixture should retain its ASCII root marker");
    if (root_marker_start != std::string::npos) {
        std::string uppercase_marker = root_marker;
        std::transform(
            uppercase_marker.begin(),
            uppercase_marker.end(),
            uppercase_marker.begin(),
            [](const char ch) {
                return ch >= 'a' && ch <= 'z'
                    ? static_cast<char>(ch - 'a' + 'A')
                    : ch;
            });
        case_variant_root.replace(root_marker_start, root_marker.size(), uppercase_marker);
        const copperfin::security::ExternalProcessPolicy case_variant_policy{
            .executable_name = "copperfin-policy-fixture.exe",
            .allowed_path_roots = {case_variant_root},
            .allowed_publishers = {},
            .require_trusted_signature = false
        };
        const auto case_variant_authorization =
            copperfin::security::authorize_external_process(case_variant_policy);
        expect(case_variant_authorization.allowed,
               "#4329: Windows external-process policy should accept case-equivalent allowed roots");
    }

    const fs::path sibling_root = temp_root.parent_path() /
        (temp_root.filename().wstring() + L"-sibling");
    const copperfin::security::ExternalProcessPolicy sibling_policy{
        .executable_name = "copperfin-policy-fixture.exe",
        .allowed_path_roots = {copperfin::platform::path_to_utf8_string(sibling_root)},
        .allowed_publishers = {},
        .require_trusted_signature = false
    };
    const auto sibling_authorization =
        copperfin::security::authorize_external_process(sibling_policy);
    expect(!sibling_authorization.allowed,
           "#4329: Windows external-process policy should reject sibling-prefix roots");

    if (original_path.empty()) {
        SetEnvironmentVariableW(L"PATH", nullptr);
    } else {
        SetEnvironmentVariableW(L"PATH", original_path.c_str());
    }
    fs::remove_all(temp_root, ignored);
}

void test_external_process_policy_handles_long_paths() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / L"copperfin_security_policy_long_path";
    fs::path fixture_directory = temp_root;
    while (fixture_directory.wstring().size() <= static_cast<std::size_t>(MAX_PATH) + 32U) {
        fixture_directory /= L"long-process-policy-component-0123456789";
    }
    const fs::path fixture_path = fixture_directory / L"copperfin-policy-fixture.exe";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(fixture_directory, ignored);
    if (ignored) {
        std::cerr << "SKIP: #4324 Windows long-path fixture directory is unavailable.\n";
        fs::remove_all(temp_root, ignored);
        return;
    }

    wchar_t system_directory[MAX_PATH]{};
    const UINT system_directory_length = GetSystemDirectoryW(system_directory, MAX_PATH);
    const fs::path system_command = system_directory_length == 0U
        ? fs::path{}
        : fs::path(std::wstring(system_directory, system_directory_length)) / L"cmd.exe";
    if (system_command.empty() || !fs::exists(system_command, ignored)) {
        expect(false, "#4324: Windows long-path fixture should find cmd.exe");
        fs::remove_all(temp_root, ignored);
        return;
    }
    fs::copy_file(system_command, fixture_path, fs::copy_options::overwrite_existing, ignored);
    if (ignored) {
        std::cerr << "SKIP: #4324 Windows long-path file creation is unavailable.\n";
        fs::remove_all(temp_root, ignored);
        return;
    }

    std::wstring original_path;
    const DWORD path_length = GetEnvironmentVariableW(L"PATH", nullptr, 0U);
    if (path_length != 0U) {
        original_path.resize(static_cast<std::size_t>(path_length));
        DWORD copied_length = GetEnvironmentVariableW(
            L"PATH",
            original_path.data(),
            path_length);
        if (copied_length >= path_length) {
            original_path.resize(static_cast<std::size_t>(copied_length) + 1U);
            copied_length = GetEnvironmentVariableW(
                L"PATH",
                original_path.data(),
                static_cast<DWORD>(original_path.size()));
        }
        original_path.resize(static_cast<std::size_t>(copied_length));
    }
    const std::wstring fixture_path_value = fixture_directory.wstring() + L";" + original_path;
    const bool path_set = SetEnvironmentVariableW(L"PATH", fixture_path_value.c_str()) != FALSE;
    if (!path_set) {
        expect(false, "#4324: Windows long-path fixture should update PATH");
        fs::remove_all(temp_root, ignored);
        return;
    }

    const copperfin::security::ExternalProcessPolicy policy{
        .executable_name = "copperfin-policy-fixture.exe",
        .allowed_path_roots = {copperfin::platform::path_to_utf8_string(temp_root)},
        .allowed_publishers = {},
        .require_trusted_signature = false
    };
    const auto authorization = copperfin::security::authorize_external_process(policy);
    const auto resolved_path = copperfin::platform::path_from_utf8_string(authorization.resolved_path);
    expect(authorization.allowed,
           "#4324: Windows external-process policy should authorize an executable beyond MAX_PATH");
    expect(resolved_path == fs::weakly_canonical(fixture_path, ignored),
           "#4324: Windows external-process policy should preserve the canonical long executable path");
    expect(fs::equivalent(resolved_path, fixture_path, ignored),
           "#4324: Windows external-process policy should resolve the long fixture to the copied executable");

    if (original_path.empty()) {
        SetEnvironmentVariableW(L"PATH", nullptr);
    } else {
        SetEnvironmentVariableW(L"PATH", original_path.c_str());
    }
    fs::remove_all(temp_root, ignored);
}
#endif

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

void test_physical_path_containment_rejects_indirection() {
    namespace fs = std::filesystem;
    using copperfin::security::PhysicalPathContainmentFailure;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_physical_path_containment_tests";
    const fs::path package_root = temp_root / "package";
    const fs::path content_root = package_root / "content";
    const fs::path outside_root = temp_root / "outside";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(content_root);
    fs::create_directories(outside_root);

    const fs::path packaged_file = content_root / "main.prg";
    const fs::path outside_file = outside_root / "outside.prg";
    {
        std::ofstream output(packaged_file, std::ios::binary);
        output << "RETURN\n";
    }
    {
        std::ofstream output(outside_file, std::ios::binary);
        output << "RETURN\n";
    }

    const auto ordinary = copperfin::security::inspect_physical_path_containment(
        packaged_file,
        package_root);
    expect(ordinary.allowed && fs::equivalent(ordinary.canonical_path, packaged_file),
           "ordinary package files should pass physical containment inspection");
    const auto root_itself = copperfin::security::inspect_physical_path_containment(
        package_root,
        package_root);
    expect(root_itself.allowed && fs::equivalent(root_itself.canonical_path, package_root),
           "the exact package root should pass physical containment inspection");
    const auto ordinary_snapshot = copperfin::security::read_physically_contained_file_snapshot(
        ordinary,
        package_root);
    expect(ordinary_snapshot.ok && ordinary_snapshot.bytes == "RETURN\n",
           "physical file snapshots should read the same admitted package file object");

#if defined(_WIN32)
    std::wstring differently_cased_root = package_root.native();
    std::wstring differently_cased_file = packaged_file.native();
    std::transform(
        differently_cased_root.begin(),
        differently_cased_root.end(),
        differently_cased_root.begin(),
        [](const wchar_t ch) { return static_cast<wchar_t>(std::towupper(ch)); });
    std::transform(
        differently_cased_file.begin(),
        differently_cased_file.end(),
        differently_cased_file.begin(),
        [](const wchar_t ch) { return static_cast<wchar_t>(std::towupper(ch)); });
    const auto differently_cased = copperfin::security::inspect_physical_path_containment(
        fs::path(differently_cased_file),
        fs::path(differently_cased_root));
    expect(differently_cased.allowed && fs::equivalent(differently_cased.canonical_path, packaged_file),
           "Windows package containment should compare existing path components case-insensitively");
#endif

    {
        std::ofstream output(packaged_file, std::ios::binary | std::ios::trunc);
        output << "RETURN\n? 1\n";
    }
    const auto replaced = copperfin::security::inspect_physical_path_containment(
        packaged_file,
        package_root);
    expect(replaced.allowed && replaced.identity != ordinary.identity,
           "physical containment identities should detect an ordinary file replacement or mutation");
    const auto stale_snapshot = copperfin::security::read_physically_contained_file_snapshot(
        ordinary,
        package_root);
    expect(!stale_snapshot.ok &&
               stale_snapshot.failure == PhysicalPathContainmentFailure::identity_changed,
           "physical file snapshots should reject a path changed after admission");

    const auto outside = copperfin::security::inspect_physical_path_containment(
        outside_file,
        package_root);
    expect(!outside.allowed && outside.failure == PhysicalPathContainmentFailure::outside_root,
           "lexically external package files should fail physical containment inspection");

    const fs::path file_link = content_root / "linked.prg";
    fs::create_symlink(outside_file, file_link, ignored);
    if (!ignored) {
        const auto linked = copperfin::security::inspect_physical_path_containment(
            file_link,
            package_root);
        expect(!linked.allowed && linked.failure == PhysicalPathContainmentFailure::indirect_component,
               "package file symlinks should fail physical containment inspection");
    }

    ignored.clear();
    const fs::path directory_link = content_root / "linked-dir";
    fs::create_directory_symlink(outside_root, directory_link, ignored);
    const bool directory_link_created = !ignored;
    if (!ignored) {
        const auto linked_child = copperfin::security::inspect_physical_path_containment(
            directory_link / "outside.prg",
            package_root);
        expect(!linked_child.allowed &&
                   linked_child.failure == PhysicalPathContainmentFailure::indirect_component,
               "package directory symlinks or reparse points should fail containment inspection");
    }

    ignored.clear();
    const fs::path package_alias = temp_root / "package-alias";
    fs::create_directory_symlink(package_root, package_alias, ignored);
    if (!ignored) {
        const auto aliased_path = copperfin::security::inspect_physical_path_containment(
            package_alias / "content" / "main.prg",
            package_root);
        expect(aliased_path.allowed && fs::equivalent(aliased_path.canonical_path, packaged_file),
               "a package-root path alias should preserve containment against the physical package root");
        if (directory_link_created) {
            const auto aliased_linked_child =
                copperfin::security::inspect_physical_path_containment(
                    package_alias / "content" / "linked-dir" / "outside.prg",
                    package_root);
            expect(!aliased_linked_child.allowed &&
                       aliased_linked_child.failure ==
                           PhysicalPathContainmentFailure::indirect_component,
                   "a package-root alias must not hide indirection beneath the package root");
        }

        const auto relocated = copperfin::security::inspect_physical_path_containment(
            package_alias / "content" / "main.prg",
            package_alias);
        expect(relocated.allowed && fs::equivalent(relocated.canonical_path, packaged_file),
               "a package-root deployment link should preserve physical containment beneath its target");

        fs::remove(package_alias, ignored);
        ignored.clear();
        fs::create_directory_symlink(outside_root, package_alias, ignored);
        if (!ignored) {
            const auto swapped_root_snapshot =
                copperfin::security::read_physically_contained_file_snapshot(
                    relocated,
                    package_alias);
            expect(!swapped_root_snapshot.ok &&
                       swapped_root_snapshot.failure == PhysicalPathContainmentFailure::identity_changed,
                   "physical snapshots should fail deterministically when an admitted package-root link changes target");
            fs::remove(package_alias, ignored);
        }
    }

#if defined(_WIN32)
    const DWORD short_root_size = ::GetShortPathNameW(package_root.c_str(), nullptr, 0U);
    if (short_root_size > 0U) {
        std::vector<wchar_t> short_root_buffer(short_root_size, L'\0');
        const DWORD short_root_length = ::GetShortPathNameW(
            package_root.c_str(),
            short_root_buffer.data(),
            static_cast<DWORD>(short_root_buffer.size()));
        if (short_root_length > 0U && short_root_length < short_root_buffer.size()) {
            const fs::path short_root(
                std::wstring(short_root_buffer.data(), short_root_length));
            if (short_root != package_root) {
                const auto short_candidate =
                    copperfin::security::inspect_physical_path_containment(
                        short_root / "content" / "main.prg",
                        package_root);
                expect(short_candidate.allowed &&
                           fs::equivalent(short_candidate.canonical_path, packaged_file),
                       "Windows short-path package aliases should remain physically contained");

                const auto short_root_containment =
                    copperfin::security::inspect_physical_path_containment(
                        packaged_file,
                        short_root);
                expect(short_root_containment.allowed &&
                           fs::equivalent(short_root_containment.canonical_path, packaged_file),
                       "Windows long package paths should remain contained beneath a short-path root alias");
            }
        }
    }
#endif

#if !defined(_WIN32)
    const fs::path secondary_root = "/dev/shm";
    std::error_code secondary_status_error;
    const fs::file_status secondary_status =
        fs::symlink_status(secondary_root, secondary_status_error);
    if (!secondary_status_error && fs::is_directory(secondary_status)) {
        const fs::path cross_device_file =
            secondary_root /
            ("copperfin_physical_path_cross_device_" +
             std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id())) +
             ".prg");
        write_file_bytes(cross_device_file, "RETURN\n");
        const auto filesystem_root =
            copperfin::security::inspect_physical_path_containment("/", "/");
        const auto secondary_file =
            copperfin::security::inspect_physical_path_containment(
                cross_device_file,
                secondary_root);
        if (filesystem_root.allowed && secondary_file.allowed &&
            filesystem_root.identity.storage_id != secondary_file.identity.storage_id) {
            const auto crossed_device =
                copperfin::security::inspect_physical_path_containment(
                    cross_device_file,
                    "/");
            expect(!crossed_device.allowed &&
                       crossed_device.failure ==
                           PhysicalPathContainmentFailure::cross_device_component,
                   "physical containment should reject a mounted filesystem below its root");
        }
        fs::remove(cross_device_file, ignored);
    }
#endif

    ignored.clear();
    const fs::path dangling_link = content_root / "dangling.prg";
    fs::create_symlink(outside_root / "missing.prg", dangling_link, ignored);
    if (!ignored) {
        const auto dangling = copperfin::security::inspect_physical_path_containment(
            dangling_link,
            package_root);
        expect(!dangling.allowed && dangling.failure == PhysicalPathContainmentFailure::indirect_component,
               "dangling package symlinks should fail as indirection instead of being followed");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_authorization();
    test_secret_provider();
    test_audit_stream_chain();
    test_audit_stream_preserves_unicode_paths();
    test_sha256_helpers();
    test_security_diagnostics_resolve_through_localization_catalog();
    test_security_diagnostics_follow_selected_locale();
    test_security_profile_follows_selected_locale();
    test_authorization_unknown_role_returns_false();
    test_authorization_empty_permission_returns_false();
    test_secret_provider_missing_env_var_returns_not_ok();
    test_secret_provider_rejects_malformed_env_var_names();
    test_audit_stream_tamper_detection();
    test_audit_stream_chain_verification();
    test_audit_stream_append_rejects_invalid_existing_tail();
    test_audit_stream_localized_malformed_line_diagnostic();
    test_contained_audit_append_rechecks_open_file_identity();
    test_direct_audit_append_rejects_linked_leaf();
    test_contained_audit_append_serializes_concurrent_writers();
    test_audit_stream_append_to_readonly_path_fails_gracefully();
    test_external_process_and_process_hardening_diagnostics();
#ifndef _WIN32
    test_external_process_authorization_rejects_replacement();
#endif
#ifdef _WIN32
    test_external_process_policy_preserves_unicode_paths();
    test_external_process_policy_handles_long_paths();
#endif
    test_physical_path_containment_rejects_indirection();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
