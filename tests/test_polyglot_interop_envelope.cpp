// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/polyglot_interop_envelope.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace {

int failures = 0;

void expect(const bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

std::string read_text(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

copperfin::platform::PolyglotInteropEnvelopeExpectation expectation() {
    return {
        .capability_id = "reports.invoice.render",
        .correlation_id = "example-correlation-001",
        .protocol_version = "1.0.0"};
}

void expect_error(
    const std::string& document,
    const copperfin::platform::PolyglotInteropEnvelopeExpectation& expected,
    const copperfin::platform::PolyglotInteropEnvelopeError error,
    const std::string& error_code,
    const std::string& message) {
    const auto result =
        copperfin::platform::parse_polyglot_interop_envelope(document, expected);
    expect(!result.ok() && result.error == error && result.error_code == error_code, message);
}

void test_versioned_examples_and_field_order() {
    using copperfin::platform::PolyglotInteropEnvelopeKind;
    auto expected = expectation();
    const auto success_document = read_text(COPPERFIN_POLYGLOT_SUCCESS_ENVELOPE_PATH);
    const auto success = copperfin::platform::parse_polyglot_interop_envelope(
        success_document, expected);
    expect(success.ok() && success.envelope.kind == PolyglotInteropEnvelopeKind::success,
           "#91: versioned success example should parse");
    const std::string fixture_newline =
        success_document.find("\r\n") == std::string::npos ? "\n" : "\r\n";
    const std::string expected_fixture_payload =
        "{" + fixture_newline +
        "    \"artifact_id\": \"invoice-preview-001\"," + fixture_newline +
        "    \"content_type\": \"application/pdf\"" + fixture_newline + "  }";
    expect(success.envelope.payload_json == expected_fixture_payload,
           "#91: success payload should remain exact validated JSON bytes");

    expected.correlation_id = "example-correlation-002";
    const auto error = copperfin::platform::parse_polyglot_interop_envelope(
        read_text(COPPERFIN_POLYGLOT_ERROR_ENVELOPE_PATH), expected);
    expect(error.ok() && error.envelope.kind == PolyglotInteropEnvelopeKind::error &&
               error.envelope.candidate_error_code == "bridge.timeout" &&
               error.envelope.candidate_error_retryable,
           "#91: versioned error example should expose structured failure fields");

    const auto reordered = copperfin::platform::parse_polyglot_interop_envelope(
        R"json({"payload":{"nested":[true,null,-1.25e+3]},"protocol_version":"1.0.0","correlation_id":"example-correlation-001","capability_id":"reports.invoice.render","kind":"success","envelope_version":"1.0"})json",
        expectation());
    expect(reordered.ok() &&
               reordered.envelope.payload_json == "{\"nested\":[true,null,-1.25e+3]}",
           "#91: field order should not affect a valid exact response");
}

void test_payload_line_endings_remain_exact() {
    for (const std::string& newline : {std::string("\n"), std::string("\r\n")}) {
        const std::string payload =
            "{" + newline + "  \"value\": true" + newline + "}";
        const std::string document =
            "{\"envelope_version\":\"1.0\",\"kind\":\"success\","
            "\"capability_id\":\"reports.invoice.render\","
            "\"correlation_id\":\"example-correlation-001\","
            "\"protocol_version\":\"1.0.0\",\"payload\":" + payload + "}";
        const auto result = copperfin::platform::parse_polyglot_interop_envelope(
            document, expectation());
        expect(result.ok() && result.envelope.payload_json == payload,
               "#91: LF and CRLF payload bytes should remain exact");
    }
}

void test_identity_and_shape_fail_closed() {
    using copperfin::platform::PolyglotInteropEnvelopeError;
    const std::string success =
        R"json({"envelope_version":"1.0","kind":"success","capability_id":"reports.invoice.render","correlation_id":"example-correlation-001","protocol_version":"1.0.0","payload":{}})json";

    auto expected = expectation();
    expected.capability_id = "reports.other.render";
    expect_error(success, expected, PolyglotInteropEnvelopeError::capability_id_mismatch,
                 "polyglot.envelope.capability_id_mismatch",
                 "#91: capability confusion should fail closed");
    expected = expectation();
    expected.correlation_id = "other-correlation";
    expect_error(success, expected, PolyglotInteropEnvelopeError::correlation_id_mismatch,
                 "polyglot.envelope.correlation_id_mismatch",
                 "#91: replayed correlation identity should fail closed");
    expected = expectation();
    expected.protocol_version = "1.1.0";
    expect_error(success, expected, PolyglotInteropEnvelopeError::protocol_version_mismatch,
                 "polyglot.envelope.protocol_version_mismatch",
                 "#91: protocol mismatch should fail closed");

    expect_error(
        R"json({"envelope_version":"1.0","kind":"success","capability_id":"reports.invoice.render","correlation_id":"example-correlation-001","protocol_version":"1.0.0","error":{"code":"bridge.failed","message":"no","retryable":false}})json",
        expectation(), PolyglotInteropEnvelopeError::payload_required,
        "polyglot.envelope.payload_required",
        "#91: success envelopes should require payload and reject error shape");
    expect_error(
        R"json({"envelope_version":"1.0","kind":"error","capability_id":"reports.invoice.render","correlation_id":"example-correlation-001","protocol_version":"1.0.0","payload":{}})json",
        expectation(), PolyglotInteropEnvelopeError::error_required,
        "polyglot.envelope.error_required",
        "#91: error envelopes should require structured error and reject payload shape");
    expect_error(
        R"json({"envelope_version":"1.0","kind":"error","capability_id":"reports.invoice.render","correlation_id":"example-correlation-001","protocol_version":"1.0.0","error":{"code":"Bridge Failed","message":"no","retryable":false}})json",
        expectation(), PolyglotInteropEnvelopeError::invalid_error_code,
        "polyglot.envelope.invalid_error_code",
        "#91: candidate error code should remain a machine identifier");
}

void test_ambiguous_and_malformed_json_rejected() {
    using copperfin::platform::PolyglotInteropEnvelopeError;
    expect_error("", expectation(), PolyglotInteropEnvelopeError::document_required,
                 "polyglot.envelope.document_required",
                 "#91: empty responses should fail before parsing");
    expect_error(
        R"json({"envelope_version":"2.0","kind":"success","capability_id":"reports.invoice.render","correlation_id":"example-correlation-001","protocol_version":"1.0.0","payload":{}})json",
        expectation(), PolyglotInteropEnvelopeError::invalid_envelope_version,
        "polyglot.envelope.invalid_envelope_version",
        "#91: incompatible envelope versions should be rejected");
    expect_error(
        R"json({"envelope_version":"1.0","kind":"request","capability_id":"reports.invoice.render","correlation_id":"example-correlation-001","protocol_version":"1.0.0","payload":{}})json",
        expectation(), PolyglotInteropEnvelopeError::invalid_kind,
        "polyglot.envelope.invalid_kind",
        "#91: response parser should reject request and unknown kinds");
    expect_error(
        R"json({"envelope_version":"1.0","kind":"success","capability_id":"reports.invoice.render","capability\u005fid":"reports.other.render","correlation_id":"example-correlation-001","protocol_version":"1.0.0","payload":{}})json",
        expectation(), PolyglotInteropEnvelopeError::invalid_document,
        "polyglot.envelope.invalid_document",
        "#91: escape-equivalent duplicate top-level keys should be rejected");
    expect_error(
        R"json({"envelope_version":"1.0","kind":"success","capability_id":"reports.invoice.render","correlation_id":"example-correlation-001","protocol_version":"1.0.0","payload":{"value":1,"value":2}})json",
        expectation(), PolyglotInteropEnvelopeError::invalid_json,
        "polyglot.envelope.invalid_json",
        "#91: duplicate keys inside opaque payloads should still be rejected");
    expect_error(
        R"json({"envelope_version":"1.0","kind":"success","capability_id":"reports.invoice.render","correlation_id":"example-correlation-001","protocol_version":"1.0.0","payload":{},"extra":false})json",
        expectation(), PolyglotInteropEnvelopeError::invalid_document,
        "polyglot.envelope.invalid_document",
        "#91: unknown top-level fields should be rejected");
    expect_error(
        R"json({"envelope_version":"1.0","kind":"error","capability_id":"reports.invoice.render","correlation_id":"example-correlation-001","protocol_version":"1.0.0","error":{"code":"bridge.failed","message":"no","retryable":false,"extra":false}})json",
        expectation(), PolyglotInteropEnvelopeError::invalid_document,
        "polyglot.envelope.invalid_document",
        "#91: unknown candidate-error fields should be rejected");
    expect_error(
        R"json({"envelope_version":"1.0","kind":"error","capability_id":"reports.invoice.render","correlation_id":"example-correlation-001","protocol_version":"1.0.0","error":{"code":"bridge.failed","message":"no","message":"again","retryable":false}})json",
        expectation(), PolyglotInteropEnvelopeError::invalid_document,
        "polyglot.envelope.invalid_document",
        "#91: duplicate candidate-error fields should be rejected");
    expect_error(
        R"json({"envelope_version":"1.0","kind":"success","capability_id":"reports.invoice.render","correlation_id":"example-correlation-001","protocol_version":"1.0.0","payload":[]} )json",
        expectation(), PolyglotInteropEnvelopeError::invalid_json,
        "polyglot.envelope.invalid_json",
        "#91: success payload should be a typed JSON object");
    expect_error(
        R"json({"envelope_version":"1.0","kind":"success","capability_id":"reports.invoice.render","correlation_id":"example-correlation-001","protocol_version":"1.0.0","payload":{"value":01}})json",
        expectation(), PolyglotInteropEnvelopeError::invalid_json,
        "polyglot.envelope.invalid_json",
        "#91: non-JSON number grammar should be rejected");
    expect_error(
        R"json({"envelope_version":"1.0","kind":"success","capability_id":"reports.invoice.render","correlation_id":"example-correlation-001","protocol_version":"1.0.0","payload":{}} trailing)json",
        expectation(), PolyglotInteropEnvelopeError::invalid_json,
        "polyglot.envelope.invalid_json",
        "#91: trailing bytes should be rejected");
}

void test_unicode_and_resource_bounds() {
    using copperfin::platform::PolyglotInteropEnvelopeError;
    const auto escaped_unicode = copperfin::platform::parse_polyglot_interop_envelope(
        R"json({"envelope_version":"1.0","kind":"error","capability_id":"reports.invoice.render","correlation_id":"example-correlation-001","protocol_version":"1.0.0","error":{"code":"bridge.failed","message":"caf\u00e9 \ud83d\ude80","retryable":false}})json",
        expectation());
    expect(escaped_unicode.ok() &&
               escaped_unicode.envelope.candidate_error_message ==
                   std::string{"caf\xC3\xA9 \xF0\x9F\x9A\x80"},
           "#91: valid BMP and surrogate-pair escapes should decode to UTF-8");

    std::string invalid_utf8 =
        R"json({"envelope_version":"1.0","kind":"error","capability_id":"reports.invoice.render","correlation_id":"example-correlation-001","protocol_version":"1.0.0","error":{"code":"bridge.failed","message":")json";
    invalid_utf8.push_back(static_cast<char>(0xC0));
    invalid_utf8 += R"json(","retryable":false}})json";
    expect_error(invalid_utf8, expectation(), PolyglotInteropEnvelopeError::invalid_document,
                 "polyglot.envelope.invalid_document",
                 "#91: invalid raw UTF-8 should be rejected");
    expect_error(
        R"json({"envelope_version":"1.0","kind":"error","capability_id":"reports.invoice.render","correlation_id":"example-correlation-001","protocol_version":"1.0.0","error":{"code":"bridge.failed","message":"\ud800","retryable":false}})json",
        expectation(), PolyglotInteropEnvelopeError::invalid_document,
        "polyglot.envelope.invalid_document",
        "#91: unpaired surrogate escapes should be rejected");

    auto expected = expectation();
    expected.max_document_bytes = 10U;
    expect_error("{\"value\":123}", expected, PolyglotInteropEnvelopeError::document_too_large,
                 "polyglot.envelope.document_too_large",
                 "#91: response byte budget should fail before parsing");
    expected = expectation();
    expected.max_nesting_depth = 2U;
    expect_error(
        R"json({"envelope_version":"1.0","kind":"success","capability_id":"reports.invoice.render","correlation_id":"example-correlation-001","protocol_version":"1.0.0","payload":{"a":{"b":{"c":1}}}})json",
        expected, PolyglotInteropEnvelopeError::invalid_json,
        "polyglot.envelope.invalid_json",
        "#91: response nesting budget should bound recursive parsing");
    expected = expectation();
    expected.max_nesting_depth = 65U;
    expect_error("{}", expected, PolyglotInteropEnvelopeError::invalid_limits,
                 "polyglot.envelope.invalid_limits",
                 "#91: callers should not disable the hard nesting ceiling");
    expected = expectation();
    expected.max_document_bytes = std::size_t{17U} * 1024U * 1024U;
    expect_error("{}", expected, PolyglotInteropEnvelopeError::invalid_limits,
                 "polyglot.envelope.invalid_limits",
                 "#91: callers should not disable the hard byte ceiling");
}

void test_large_bounded_payload() {
    std::string document =
        R"json({"envelope_version":"1.0","kind":"success","capability_id":"reports.invoice.render","correlation_id":"example-correlation-001","protocol_version":"1.0.0","payload":{)json";
    for (unsigned int index = 0U; index < 4096U; ++index) {
        if (index != 0U) {
            document.push_back(',');
        }
        document += "\"field" + std::to_string(index) + "\":" + std::to_string(index);
    }
    document += "}}";
    const auto result =
        copperfin::platform::parse_polyglot_interop_envelope(document, expectation());
    expect(result.ok() && result.envelope.payload_json.size() > 60000U,
           "#91: thousands of unique payload fields should remain bounded and valid");
}

void test_expectation_validation() {
    using copperfin::platform::PolyglotInteropEnvelopeError;
    auto expected = expectation();
    expected.capability_id = "Invalid Capability";
    expect_error("{}", expected, PolyglotInteropEnvelopeError::invalid_capability_id,
                 "polyglot.envelope.invalid_capability_id",
                 "#91: invalid expected capability should fail before response parsing");
    expected = expectation();
    expected.correlation_id.clear();
    expect_error("{}", expected, PolyglotInteropEnvelopeError::correlation_id_required,
                 "polyglot.envelope.correlation_id_required",
                 "#91: correlation expectation should be mandatory");
    expected = expectation();
    expected.protocol_version = "1.0";
    expect_error("{}", expected, PolyglotInteropEnvelopeError::invalid_protocol_version,
                 "polyglot.envelope.invalid_protocol_version",
                 "#91: expected protocol should use semantic version syntax");

    expected = expectation();
    expect_error(
        R"json({"envelope_version":"1.0","kind":"success","capability_id":"Invalid Capability","correlation_id":"example-correlation-001","protocol_version":"1.0.0","payload":{}})json",
        expected, PolyglotInteropEnvelopeError::invalid_capability_id,
        "polyglot.envelope.invalid_capability_id",
        "#91: response capability should use invariant identifier syntax");
    expect_error(
        R"json({"envelope_version":"1.0","kind":"success","capability_id":"reports.invoice.render","correlation_id":"example-correlation-001","protocol_version":"1.0","payload":{}})json",
        expected, PolyglotInteropEnvelopeError::invalid_protocol_version,
        "polyglot.envelope.invalid_protocol_version",
        "#91: response protocol should use semantic version syntax");
}

}  // namespace

int main() {
    test_versioned_examples_and_field_order();
    test_payload_line_endings_remain_exact();
    test_identity_and_shape_fail_closed();
    test_ambiguous_and_malformed_json_rejected();
    test_unicode_and_resource_bounds();
    test_large_bounded_payload();
    test_expectation_validation();
    if (failures != 0) {
        std::cerr << failures << " polyglot interop envelope test(s) failed\n";
        return 1;
    }
    std::cout << "All polyglot interop envelope tests passed\n";
    return 0;
}
