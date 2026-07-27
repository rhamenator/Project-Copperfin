// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/licensing/license_status.h"
#include "copperfin/licensing/license_status_display.h"
#include "canonical_payload_serializer.h"
#include "license_classifier.h"
#include "license_payload_parser.h"
#include "license_payload_value.h"
#include "test_environment_support.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

namespace {

namespace fs = std::filesystem;
using copperfin::test_support::ScopedEnvironmentValue;
using copperfin::test_support::ScopedEnvironmentPath;
using copperfin::test_support::set_env_value;
using copperfin::licensing::LicenseState;
using copperfin::licensing::LicenseStatus;
using copperfin::licensing::PayloadFields;
using copperfin::licensing::PayloadValue;
using copperfin::licensing::ParsedLicenseFile;
using copperfin::licensing::SignerPublicKey;
using copperfin::licensing::canonicalize_payload;
using copperfin::licensing::classify_verified_payload;
using copperfin::licensing::license_state_name;
using copperfin::licensing::load_license_status;
using copperfin::licensing::parse_license_file;
using copperfin::licensing::localized_license_diagnostic;

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

fs::path test_root() {
    return fs::temp_directory_path() / "copperfin_licensing_tests";
}

void write_text_file(const fs::path& path, const std::string& contents) {
    fs::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << contents;
}

// -- Golden fixtures: signed with a throwaway test-only Ed25519 key that
// never existed as a "real" product key, and is not the same key used for
// tools/license-signer/generate_signing_key.sh's "windows-test" output.
// Signatures were produced once via `openssl pkeyutl -sign -rawin` over the
// exact canonical payload bytes and are hardcoded here so this test suite
// never needs a signing implementation of its own -- only the vendored
// verify-only code is exercised.

constexpr char kFixtureSignerKeyId[] = "test-fixture-key";

constexpr std::array<std::uint8_t, 32> kFixturePublicKey = {
    0x63, 0xf6, 0x18, 0x3a, 0xf2, 0x77, 0xb8, 0x44, 0x36, 0x98, 0xf7, 0xc5, 0xe2, 0xd3, 0x6f, 0x22,
    0xc2, 0xe3, 0x72, 0x58, 0xeb, 0x5c, 0xb9, 0x41, 0xc9, 0x88, 0xc2, 0x9f, 0xd2, 0xce, 0x00, 0x4e};

// The real product key table (kKnownSignerPublicKeys) ships empty by
// design, so load_license_status()'s tests inject this throwaway table
// instead -- production call sites never do this and always use the
// function's default parameter.
constexpr std::array<SignerPublicKey, 1> kFixtureSignerKeys{{
    {kFixtureSignerKeyId, kFixturePublicKey}}};

// Perpetual fixture: perpetual_max_major_version=5, so with the product's
// real kCopperfinMajorVersion (1 today), this always classifies as
// perpetual_current end-to-end -- the perpetual_current/out_of_version
// boundary itself is tested directly against classify_verified_payload()
// below, independent of the product's current version constant.
constexpr char kFixture1Json[] =
    R"({"payload":{"issued_date":"2026-01-01","license_id":"test-perpetual","license_type":"perpetual","licensee_email":"buyer@example.com","licensee_name":"Acme Test Co","perpetual_max_major_version":5,"pricing_model":"seat","schema_version":1,"seats":3,"signer_key_id":"test-fixture-key","subscription_expires":""},"signature_algorithm":"ed25519","signature":"wCv6Msko60LUJs668aIP24VDYzRB7tjno44Ek05ozrfCSOywwBWv8s5mOyPQ9w5/ksMs98h5j1VCA1xNIXMvBw=="})";

// Subscription fixture. Payload fields are deliberately written in a
// different order than the canonical (sorted-key) form, to prove that
// on-disk field order does not affect signature verification -- only the
// canonicalized (sorted) byte sequence is ever signed/verified.
constexpr char kFixture2Json[] =
    R"({"payload":{"seats":3,"license_type":"subscription","subscription_expires":"2026-06-01","signer_key_id":"test-fixture-key","schema_version":1,"pricing_model":"seat","licensee_name":"Acme Test Co","licensee_email":"buyer@example.com","license_id":"test-subscription","issued_date":"2026-01-01","perpetual_max_major_version":0},"signature_algorithm":"ed25519","signature":"G69paAXEQMYwTqOr/rpXGhZUB6pgsjZfu9gAbXY8aW1AsSSuhucDFVaDXArG7FyAoAnG90u/ywQe26pxzk6LDQ=="})";

// --- Layer 1: classify_verified_payload direct tests (no file I/O, no
// crypto, fully deterministic regardless of wall-clock time or the
// product's current kCopperfinMajorVersion). ---

PayloadFields make_perpetual_fields(long long max_major_version) {
    PayloadFields fields;
    fields["license_type"] = PayloadValue::make_string("perpetual");
    fields["license_id"] = PayloadValue::make_string("classifier-test-perpetual");
    fields["seats"] = PayloadValue::make_integer(7);
    fields["perpetual_max_major_version"] = PayloadValue::make_integer(max_major_version);
    return fields;
}

PayloadFields make_subscription_fields(const std::string& expires) {
    PayloadFields fields;
    fields["license_type"] = PayloadValue::make_string("subscription");
    fields["license_id"] = PayloadValue::make_string("classifier-test-subscription");
    fields["seats"] = PayloadValue::make_integer(2);
    fields["subscription_expires"] = PayloadValue::make_string(expires);
    return fields;
}

ParsedLicenseFile parse_test_integer(const std::string& number) {
    return parse_license_file(
        R"({"payload":{"test_integer":)" + number +
        R"(},"signature_algorithm":"ed25519","signature":"test"})");
}

ParsedLicenseFile parse_test_string(const std::string& json_string_literal) {
    return parse_license_file(
        R"({"payload":{"test_string":)" + json_string_literal +
        R"(},"signature_algorithm":"ed25519","signature":"test"})");
}

void expect_parsed_string(
    const std::string& json_string_literal,
    const std::string& expected,
    const std::string& description) {
    const auto parsed = parse_test_string(json_string_literal);
    expect(parsed.ok, description + " should parse");
    const auto value = parsed.payload_fields.find("test_string");
    expect(value != parsed.payload_fields.end(), description + " should retain the payload field");
    if (value != parsed.payload_fields.end()) {
        expect(value->second.kind == PayloadValue::Kind::string_value, description + " should remain a string");
        expect(value->second.as_string == expected, description + " should preserve the expected UTF-8 bytes");
    }
}

void test_parser_unicode_escape_boundaries() {
    expect_parsed_string(R"("\u0000")", std::string(1U, '\0'), "escaped NUL");
    expect_parsed_string(R"("\u00e9")", "\xC3\xA9", "representative two-byte BMP escape");
    expect_parsed_string(R"("\u20AC")", "\xE2\x82\xAC", "representative three-byte BMP escape");
    expect_parsed_string(R"("\uD7FF")", "\xED\x9F\xBF", "BMP scalar below the surrogate range");
    expect_parsed_string(R"("\uE000")", "\xEE\x80\x80", "BMP scalar above the surrogate range");
    expect_parsed_string(R"("\uD800\uDC00")", "\xF0\x90\x80\x80", "lowest surrogate pair");
    expect_parsed_string(R"("\uD83D\uDE00")", "\xF0\x9F\x98\x80", "representative surrogate pair");
    expect_parsed_string(R"("\uDBFF\uDFFF")", "\xF4\x8F\xBF\xBF", "highest surrogate pair");
}

void test_parser_preserves_simple_escapes_and_raw_utf8() {
    expect_parsed_string(
        R"("\b\f\n\r\t\"\\\/")",
        std::string("\b\f\n\r\t\"\\/", 8U),
        "simple JSON escapes");

    const std::string raw_utf8 = "\xC3\xA9\xF0\x9F\x98\x80";
    expect_parsed_string("\"" + raw_utf8 + "\"", raw_utf8, "raw valid UTF-8");

    const auto escaped = parse_test_string(R"("\u00e9\uD83D\uDE00")");
    const auto raw = parse_test_string("\"" + raw_utf8 + "\"");
    expect(escaped.ok && raw.ok, "equivalent escaped and raw UTF-8 payloads should parse");
    if (escaped.ok && raw.ok) {
        expect(
            canonicalize_payload(escaped.payload_fields) == canonicalize_payload(raw.payload_fields),
            "equivalent escaped and raw UTF-8 payloads should canonicalize identically");
    }
}

void test_parser_rejects_malformed_surrogates() {
    expect(!parse_test_string(R"("\uD800")").ok, "a lone high surrogate should be rejected");
    expect(!parse_test_string(R"("\uDC00")").ok, "a lone low surrogate should be rejected");
    expect(!parse_test_string(R"("\uD800x")").ok, "a high surrogate followed by text should be rejected");
    expect(!parse_test_string(R"("\uD800\n")").ok, "a high surrogate followed by a simple escape should be rejected");
    expect(!parse_test_string(R"("\uD800\u0041")").ok, "a high surrogate followed by a BMP escape should be rejected");
    expect(!parse_test_string(R"("\uD800\uD800")").ok, "two high surrogates should be rejected");
    expect(!parse_test_string(R"("\uD800\uDC0")").ok, "a truncated low surrogate should be rejected");
    expect(!parse_test_string(R"("\uD800\uZZZZ")").ok, "a malformed low surrogate should be rejected");
    expect(!parse_test_string(R"("\uD80")").ok, "a truncated Unicode escape should be rejected");
}

void expect_parsed_integer(const std::string& number, long long expected, const std::string& description) {
    const auto parsed = parse_test_integer(number);
    expect(parsed.ok, description + " should parse");
    const auto value = parsed.payload_fields.find("test_integer");
    expect(value != parsed.payload_fields.end(), description + " should retain the payload field");
    if (value != parsed.payload_fields.end()) {
        expect(value->second.kind == PayloadValue::Kind::integer_value, description + " should remain an integer");
        expect(value->second.as_integer == expected, description + " should preserve its exact value");
    }
}

void test_parser_checked_integer_boundaries() {
    expect_parsed_integer("9223372036854775807", std::numeric_limits<long long>::max(), "LLONG_MAX");
    expect_parsed_integer("-9223372036854775808", std::numeric_limits<long long>::min(), "LLONG_MIN");
    expect_parsed_integer("42", 42, "ordinary positive integer");
    expect_parsed_integer("-17", -17, "ordinary negative integer");
    expect_parsed_integer("-0", 0, "negative zero");

    expect(!parse_test_integer("9223372036854775808").ok, "one past LLONG_MAX should be rejected");
    expect(!parse_test_integer("-9223372036854775809").ok, "one below LLONG_MIN should be rejected");
    expect(!parse_test_integer(std::string(4096U, '9')).ok, "a very long decimal integer should be rejected");
}

void test_classifier_integer_boundaries() {
    auto fields = make_subscription_fields("2026-06-01");
    fields["seats"] = PayloadValue::make_integer(std::numeric_limits<int>::max());
    auto status = classify_verified_payload(fields, 1, "2026-01-01");
    expect(status.state == LicenseState::subscription_active, "INT_MAX seats should remain representable");
    expect(status.seats == std::numeric_limits<int>::max(), "INT_MAX seats should remain exact");

    fields["seats"] = PayloadValue::make_integer(std::numeric_limits<int>::min());
    status = classify_verified_payload(fields, 1, "2026-01-01");
    expect(status.state == LicenseState::subscription_active, "negative representable seats should preserve existing classification");
    expect(status.seats == std::numeric_limits<int>::min(), "INT_MIN seats should remain exact");

    auto perpetual_fields = make_perpetual_fields(std::numeric_limits<int>::max());
    status = classify_verified_payload(perpetual_fields, 1, "2026-01-01");
    expect(status.state == LicenseState::perpetual_current, "INT_MAX major version should remain representable");
    expect(
        status.perpetual_max_major_version == std::numeric_limits<int>::max(),
        "INT_MAX major version should remain exact");
}

void test_classifier_rejects_out_of_range_integers_without_partial_values() {
    if constexpr (std::numeric_limits<int>::max() < std::numeric_limits<long long>::max()) {
        auto fields = make_subscription_fields("2026-06-01");
        fields["seats"] = PayloadValue::make_integer(
            static_cast<long long>(std::numeric_limits<int>::max()) + 1LL);
        auto status = classify_verified_payload(fields, 1, "2026-01-01");
        expect(status.state == LicenseState::malformed, "one past INT_MAX seats should be malformed");
        expect(status.seats == 0, "out-of-range seats should not expose a wrapped value");
        expect(status.perpetual_max_major_version == 0, "an invalid integer payload should not publish partial limits");

        fields["seats"] = PayloadValue::make_integer(
            static_cast<long long>(std::numeric_limits<int>::min()) - 1LL);
        status = classify_verified_payload(fields, 1, "2026-01-01");
        expect(status.state == LicenseState::malformed, "one below INT_MIN seats should be malformed");
        expect(status.seats == 0, "underflowing seats should not expose a wrapped value");

        auto perpetual_fields = make_perpetual_fields(
            static_cast<long long>(std::numeric_limits<int>::max()) + 1LL);
        status = classify_verified_payload(perpetual_fields, 1, "2026-01-01");
        expect(status.state == LicenseState::malformed, "one past INT_MAX major version should be malformed");
        expect(status.seats == 0, "an invalid major version should not publish otherwise-valid seats");
        expect(status.perpetual_max_major_version == 0, "an out-of-range major version should not be narrowed");
    }
}

void test_classifier_perpetual_current() {
    const auto status = classify_verified_payload(make_perpetual_fields(5), /*current_major_version=*/5, "2026-01-01");
    expect(status.state == LicenseState::perpetual_current, "perpetual_max_major_version == current should be perpetual_current");
    expect(status.seats == 7, "classifier should round-trip the seats field");
    expect(status.license_id == "classifier-test-perpetual", "classifier should round-trip the license_id field");
}

void test_classifier_perpetual_ahead_of_current() {
    const auto status = classify_verified_payload(make_perpetual_fields(5), /*current_major_version=*/3, "2026-01-01");
    expect(status.state == LicenseState::perpetual_current, "perpetual_max_major_version > current should still be perpetual_current");
}

void test_classifier_perpetual_out_of_version() {
    const auto status = classify_verified_payload(make_perpetual_fields(3), /*current_major_version=*/5, "2026-01-01");
    expect(status.state == LicenseState::perpetual_out_of_version, "perpetual_max_major_version < current should be perpetual_out_of_version");
    expect(status.perpetual_max_major_version == 3, "perpetual_out_of_version should still report the full, unrevoked license details");
}

void test_classifier_perpetual_missing_max_version_is_malformed() {
    PayloadFields fields;
    fields["license_type"] = PayloadValue::make_string("perpetual");
    const auto status = classify_verified_payload(fields, 1, "2026-01-01");
    expect(status.state == LicenseState::malformed, "perpetual license without perpetual_max_major_version should be malformed");
}

void test_classifier_subscription_active() {
    const auto status = classify_verified_payload(make_subscription_fields("2026-06-01"), 1, "2026-01-01");
    expect(status.state == LicenseState::subscription_active, "subscription_expires in the future should be subscription_active");
}

void test_classifier_subscription_active_on_expiry_day() {
    const auto status = classify_verified_payload(make_subscription_fields("2026-06-01"), 1, "2026-06-01");
    expect(status.state == LicenseState::subscription_active, "subscription_expires equal to current date should still be active");
}

void test_classifier_subscription_expired() {
    const auto status = classify_verified_payload(make_subscription_fields("2020-01-01"), 1, "2026-07-02");
    expect(status.state == LicenseState::subscription_expired, "subscription_expires in the past should be subscription_expired");
}

void expect_malformed_subscription_expiry(const std::string& expiry, const std::string& description) {
    const auto status = classify_verified_payload(make_subscription_fields(expiry), 1, "2026-01-01");
    expect(status.state == LicenseState::malformed, description + " should be malformed");
    expect(
        status.diagnostic == "subscription license has invalid subscription_expires",
        description + " should report the invalid expiry field");
    expect(status.subscription_expires == expiry, description + " should preserve the signed field value for diagnosis");
}

void test_classifier_subscription_calendar_boundaries() {
    auto status = classify_verified_payload(make_subscription_fields("0000-01-01"), 1, "0000-01-01");
    expect(status.state == LicenseState::subscription_active, "the lowest four-digit year should be active on its day");

    status = classify_verified_payload(make_subscription_fields("0000-02-29"), 1, "0000-02-29");
    expect(status.state == LicenseState::subscription_active, "year zero should follow the divisible-by-400 leap rule");

    status = classify_verified_payload(make_subscription_fields("9999-12-31"), 1, "9999-12-30");
    expect(status.state == LicenseState::subscription_active, "the latest four-digit civil date should be valid");

    status = classify_verified_payload(make_subscription_fields("2000-02-29"), 1, "2000-02-29");
    expect(status.state == LicenseState::subscription_active, "a year divisible by 400 should admit February 29");

    status = classify_verified_payload(make_subscription_fields("2024-02-29"), 1, "2024-03-01");
    expect(status.state == LicenseState::subscription_expired, "a valid leap day should compare chronologically after validation");

    status = classify_verified_payload(make_subscription_fields("2026-04-30"), 1, "2026-04-29");
    expect(status.state == LicenseState::subscription_active, "the last day of a 30-day month should be valid");
}

void test_classifier_subscription_rejects_malformed_dates() {
    expect_malformed_subscription_expiry("1900-02-29", "a non-400 century leap day");
    expect_malformed_subscription_expiry("2100-02-29", "a future non-400 century leap day");
    expect_malformed_subscription_expiry("2023-02-29", "a non-leap-year February 29");
    expect_malformed_subscription_expiry("2026-02-30", "February 30");
    expect_malformed_subscription_expiry("2026-04-31", "the 31st day of a 30-day month");
    expect_malformed_subscription_expiry("2026-00-01", "month zero");
    expect_malformed_subscription_expiry("2026-13-01", "month thirteen");
    expect_malformed_subscription_expiry("2026-01-00", "day zero");
    expect_malformed_subscription_expiry("2026-01-32", "day thirty-two");
    expect_malformed_subscription_expiry("2026/06/01", "alternate separators");
    expect_malformed_subscription_expiry("2026-6-01", "a partial month");
    expect_malformed_subscription_expiry("2026-06-1", "a partial day");
    expect_malformed_subscription_expiry("2026-06", "a partial date");
    expect_malformed_subscription_expiry("2026-06-01T00:00:00Z", "a timestamp");
    expect_malformed_subscription_expiry(" 2026-06-01", "leading whitespace");
    expect_malformed_subscription_expiry("2026-06-01 ", "trailing whitespace");
    expect_malformed_subscription_expiry("202A-06-01", "a non-ASCII-digit year");
    expect_malformed_subscription_expiry("foo", "unrelated text");
}

void test_classifier_subscription_rejects_malformed_current_date() {
    auto status = classify_verified_payload(make_subscription_fields("2026-06-01"), 1, "2026-6-01");
    expect(status.state == LicenseState::malformed, "a partial current-date month should be malformed");
    expect(
        status.diagnostic == "current date is not canonical YYYY-MM-DD",
        "a malformed current date should report the classifier input contract");

    status = classify_verified_payload(make_subscription_fields("2026-06-01"), 1, "2026-02-30");
    expect(status.state == LicenseState::malformed, "an impossible current calendar date should be malformed");

    status = classify_verified_payload(make_subscription_fields("2026-06-01"), 1, "foo");
    expect(status.state == LicenseState::malformed, "an unrelated current-date string should be malformed");
}

void test_classifier_subscription_missing_expiry_is_malformed() {
    PayloadFields fields;
    fields["license_type"] = PayloadValue::make_string("subscription");
    const auto status = classify_verified_payload(fields, 1, "2026-01-01");
    expect(status.state == LicenseState::malformed, "subscription license without subscription_expires should be malformed");
}

void test_classifier_unknown_license_type_is_malformed() {
    PayloadFields fields;
    fields["license_type"] = PayloadValue::make_string("lifetime-vip");
    const auto status = classify_verified_payload(fields, 1, "2026-01-01");
    expect(status.state == LicenseState::malformed, "an unrecognized license_type should be malformed");
}

void test_classifier_missing_license_type_is_malformed() {
    const PayloadFields fields;
    const auto status = classify_verified_payload(fields, 1, "2026-01-01");
    expect(status.state == LicenseState::malformed, "a payload without license_type should be malformed");
}

void test_license_diagnostic_display_preserves_raw_detail_and_localizes_known_key() {
    auto status = classify_verified_payload(make_subscription_fields("not-a-date"), 1, "2026-01-01");
    expect(
        status.diagnostic == "subscription license has invalid subscription_expires" &&
            status.diagnostic_key == "Licensing.Error.SubscriptionExpiryInvalid",
        "license classification should retain raw diagnostic text and add a stable display key");

    copperfin::localization::LocalizedCatalog catalog;
    catalog.requested_locale = "es-419";
    catalog.fallback_chain = {"es-419"};
    catalog.catalogs["es-419"][status.diagnostic_key] = "Fecha de vencimiento no válida.";
    expect(
        localized_license_diagnostic(status, catalog) == "Fecha de vencimiento no válida.",
        "known license diagnostic keys should resolve through the active catalog");

    status.diagnostic_key = "Licensing.Error.UnexpectedTopLevelKey";
    status.diagnostic_argument = "unexpected_field";
    catalog.catalogs["es-419"][status.diagnostic_key] = "Clave inesperada: {argument}";
    expect(
        localized_license_diagnostic(status, catalog) == "Clave inesperada: unexpected_field",
        "license diagnostic placeholders should preserve invariant field names");
}

// --- Layer 2: full load_license_status() end-to-end tests (real file I/O,
// real Ed25519 verification against the golden fixtures above). ---

void test_valid_perpetual_fixture_end_to_end() {
    const fs::path path = test_root() / "valid_perpetual.cflicense";
    write_text_file(path, kFixture1Json);

    const auto status = load_license_status(test_root() / "app.exe", path, kFixtureSignerKeys);
    expect(status.state == LicenseState::perpetual_current, "a validly signed perpetual fixture should verify and classify as perpetual_current");
    expect(status.license_id == "test-perpetual", "license_id should round-trip through the full parse+verify pipeline");
    expect(status.seats == 3, "seats should round-trip through the full parse+verify pipeline");
    expect(status.perpetual_max_major_version == 5, "perpetual_max_major_version should round-trip");
    expect(status.source_path == path.string(), "source_path should record where the license file was loaded from");
}

void test_valid_subscription_fixture_end_to_end() {
    const fs::path path = test_root() / "valid_subscription.cflicense";
    write_text_file(path, kFixture2Json);

    const auto status = load_license_status(test_root() / "app.exe", path, kFixtureSignerKeys);
    // Deliberately not pinning active-vs-expired here (that boundary is
    // covered precisely, and deterministically, by the classifier tests
    // above) -- this test only proves the signature verifies and the file
    // parses correctly despite its shuffled field order.
    expect(
        status.state == LicenseState::subscription_active || status.state == LicenseState::subscription_expired,
        "a validly signed subscription fixture should verify and classify as some subscription state");
    expect(status.license_id == "test-subscription", "license_id should round-trip even with reordered payload fields on disk");
    expect(status.subscription_expires == "2026-06-01", "subscription_expires should round-trip even with reordered payload fields on disk");
}

void test_tampered_payload_fails_verification() {
    std::string tampered = kFixture1Json;
    const std::string needle = R"("seats":3)";
    const std::string replacement = R"("seats":4)";
    const auto position = tampered.find(needle);
    expect(position != std::string::npos, "test setup: fixture 1 JSON should contain the seats field to tamper with");
    tampered.replace(position, needle.size(), replacement);

    const fs::path path = test_root() / "tampered_payload.cflicense";
    write_text_file(path, tampered);

    const auto status = load_license_status(test_root() / "app.exe", path, kFixtureSignerKeys);
    expect(status.state == LicenseState::invalid_signature, "a tampered payload field should fail signature verification");
}

void test_tampered_signature_fails_verification() {
    std::string tampered = kFixture1Json;
    const std::string needle = "wCv6Msko";
    const std::string replacement = "wCv6Mskp";
    const auto position = tampered.find(needle);
    expect(position != std::string::npos, "test setup: fixture 1 JSON should contain the signature prefix to tamper with");
    tampered.replace(position, needle.size(), replacement);

    const fs::path path = test_root() / "tampered_signature.cflicense";
    write_text_file(path, tampered);

    const auto status = load_license_status(test_root() / "app.exe", path, kFixtureSignerKeys);
    expect(status.state == LicenseState::invalid_signature, "a tampered signature should fail verification");
}

void test_malformed_json_is_malformed() {
    const fs::path path = test_root() / "malformed.cflicense";
    write_text_file(path, "{ this is not valid json");

    const auto status = load_license_status(test_root() / "app.exe", path);
    expect(status.state == LicenseState::malformed, "truncated/invalid JSON should be malformed");
    expect(
        status.diagnostic_key == "Licensing.Error.ExpectedJsonKey",
        "malformed license JSON should expose a stable catalog display key");
}

void test_malformed_surrogate_json_is_malformed() {
    const fs::path path = test_root() / "malformed_surrogate.cflicense";
    write_text_file(
        path,
        R"({"payload":{"license_type":"subscription","licensee_name":"\uD800"},"signature_algorithm":"ed25519","signature":"test"})");

    const auto status = load_license_status(test_root() / "app.exe", path);
    expect(status.state == LicenseState::malformed, "an invalid surrogate in a license string should be malformed");
}

void test_missing_file_with_no_explicit_path_is_free() {
    const fs::path isolated_dir = test_root() / "no_license_here";
    std::error_code ignored;
    fs::remove_all(isolated_dir, ignored);
    fs::create_directories(isolated_dir);

    const auto status = load_license_status(isolated_dir / "app.exe", std::nullopt);
    expect(status.state == LicenseState::free, "no license file at the default location, with no override, should be free (not an error)");
}

void test_missing_file_with_explicit_path_is_unreadable() {
    const fs::path missing_path = test_root() / "this_file_does_not_exist.cflicense";
    std::error_code ignored;
    fs::remove(missing_path, ignored);

    const auto status = load_license_status(test_root() / "app.exe", missing_path);
    expect(status.state == LicenseState::file_unreadable, "an explicitly configured but missing license path should be file_unreadable, not free");
}

void test_directory_as_explicit_path_is_unreadable() {
    const fs::path directory_path = test_root() / "a_directory_not_a_license_file";
    fs::create_directories(directory_path);

    const auto status = load_license_status(test_root() / "app.exe", directory_path);
    expect(status.state == LicenseState::file_unreadable, "a directory given as the license path should be file_unreadable");
}

void test_default_location_is_used_when_present() {
    const fs::path default_dir = test_root() / "default_location_app";
    std::error_code ignored;
    fs::remove_all(default_dir, ignored);
    fs::create_directories(default_dir);
    write_text_file(default_dir / "license.cflicense", kFixture1Json);

    const auto status = load_license_status(default_dir / "app.exe", std::nullopt, kFixtureSignerKeys);
    expect(status.state == LicenseState::perpetual_current, "a license.cflicense next to the executable should be picked up with no override or env var");
}

void test_env_var_takes_priority_over_default_location() {
    const fs::path env_path = test_root() / "env_pointed.cflicense";
    write_text_file(env_path, kFixture1Json);

    const fs::path empty_default_dir = test_root() / "env_priority_app";
    std::error_code ignored;
    fs::remove_all(empty_default_dir, ignored);
    fs::create_directories(empty_default_dir);

    ScopedEnvironmentValue env("COPPERFIN_LICENSE_PATH");
    env.set(env_path.string());

    const auto status = load_license_status(empty_default_dir / "app.exe", std::nullopt, kFixtureSignerKeys);
    expect(status.state == LicenseState::perpetual_current, "COPPERFIN_LICENSE_PATH should be consulted when no explicit override is passed");
}

void test_env_var_preserves_non_ascii_license_path() {
    const fs::path env_path =
        test_root() / copperfin::test_support::path_from_utf8_string("licence_caf\xC3\xA9") /
        copperfin::test_support::path_from_utf8_string("licence_\xE7\xA9\xB0.cflicense");
    write_text_file(env_path, kFixture1Json);

    const fs::path empty_default_dir = test_root() / "unicode_env_priority_app";
    std::error_code ignored;
    fs::remove_all(empty_default_dir, ignored);
    fs::create_directories(empty_default_dir);

    ScopedEnvironmentPath env("COPPERFIN_LICENSE_PATH", env_path);

    const auto status = load_license_status(empty_default_dir / "app.exe", std::nullopt, kFixtureSignerKeys);
    expect(
        status.state == LicenseState::perpetual_current,
        "#4302: COPPERFIN_LICENSE_PATH should preserve a non-ASCII Windows path through license loading");
    expect(
        status.source_path == copperfin::test_support::path_to_utf8_string(env_path),
        "#4302: license status should report the configured path as UTF-8 rather than a Windows code-page string");
}

void test_explicit_override_takes_priority_over_env_var() {
    const fs::path env_path = test_root() / "should_be_ignored.cflicense";
    write_text_file(env_path, kFixture1Json);

    ScopedEnvironmentValue env("COPPERFIN_LICENSE_PATH");
    env.set(env_path.string());

    const fs::path explicit_missing_path = test_root() / "explicit_override_missing.cflicense";
    std::error_code ignored;
    fs::remove(explicit_missing_path, ignored);

    const auto status = load_license_status(test_root() / "app.exe", explicit_missing_path, kFixtureSignerKeys);
    expect(
        status.state == LicenseState::file_unreadable,
        "an explicit override pointing at a missing file should take priority over a valid COPPERFIN_LICENSE_PATH, not silently fall back to it");
}

void test_license_state_name_round_trip() {
    expect(license_state_name(LicenseState::free) == "free", "license_state_name(free)");
    expect(license_state_name(LicenseState::perpetual_out_of_version) == "perpetual_out_of_version", "license_state_name(perpetual_out_of_version)");
    expect(license_state_name(LicenseState::invalid_signature) == "invalid_signature", "license_state_name(invalid_signature)");
}

}  // namespace

int main() {
    std::error_code ignored;
    fs::remove_all(test_root(), ignored);
    fs::create_directories(test_root());

    test_classifier_perpetual_current();
    test_classifier_perpetual_ahead_of_current();
    test_classifier_perpetual_out_of_version();
    test_classifier_perpetual_missing_max_version_is_malformed();
    test_classifier_subscription_active();
    test_classifier_subscription_active_on_expiry_day();
    test_classifier_subscription_expired();
    test_classifier_subscription_calendar_boundaries();
    test_classifier_subscription_rejects_malformed_dates();
    test_classifier_subscription_rejects_malformed_current_date();
    test_classifier_subscription_missing_expiry_is_malformed();
    test_classifier_unknown_license_type_is_malformed();
    test_classifier_missing_license_type_is_malformed();
    test_license_diagnostic_display_preserves_raw_detail_and_localizes_known_key();
    test_parser_checked_integer_boundaries();
    test_parser_unicode_escape_boundaries();
    test_parser_preserves_simple_escapes_and_raw_utf8();
    test_parser_rejects_malformed_surrogates();
    test_classifier_integer_boundaries();
    test_classifier_rejects_out_of_range_integers_without_partial_values();

    test_valid_perpetual_fixture_end_to_end();
    test_valid_subscription_fixture_end_to_end();
    test_tampered_payload_fails_verification();
    test_tampered_signature_fails_verification();
    test_malformed_json_is_malformed();
    test_malformed_surrogate_json_is_malformed();
    test_missing_file_with_no_explicit_path_is_free();
    test_missing_file_with_explicit_path_is_unreadable();
    test_directory_as_explicit_path_is_unreadable();
    test_default_location_is_used_when_present();
    test_env_var_takes_priority_over_default_location();
    test_env_var_preserves_non_ascii_license_path();
    test_explicit_override_takes_priority_over_env_var();
    test_license_state_name_round_trip();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
