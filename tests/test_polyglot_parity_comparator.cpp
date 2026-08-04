// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/polyglot_parity_comparator.h"

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

using copperfin::platform::PolyglotParityComparisonRequest;
using copperfin::platform::PolyglotParityField;
using copperfin::platform::PolyglotParityMismatchCategory;
using copperfin::platform::PolyglotParityPolicy;

void test_match_and_tolerance() {
    PolyglotParityComparisonRequest request;
    request.capability_id = "reports.invoice.render";
    request.fields = {
        {"total", "integer", "integer", "100", "101"},
        {"caption", "string", "string", "Invoice", "Invoice"}};
    request.native_order = {"total", "caption"};
    request.candidate_order = request.native_order;

    auto result = copperfin::platform::compare_polyglot_outputs({}, request);
    expect(!result.parity_match && result.first_mismatch == PolyglotParityMismatchCategory::value,
           "exact comparison should report a value mismatch");
    expect(result.return_native && result.mismatch_count == 1U && result.telemetry.size() == 2U,
           "shadow comparison should preserve native return and emit mismatch telemetry");
    expect(!result.mismatch_samples.empty() &&
               result.mismatch_samples[0].find("reports.invoice.render|polyglot.parity.value_mismatch") == 0,
           "mismatch sample should include capability and reason");

    PolyglotParityPolicy tolerant;
    tolerant.numeric_tolerance = 1U;
    result = copperfin::platform::compare_polyglot_outputs(tolerant, request);
    expect(result.parity_match && result.mismatch_count == 0U,
           "numeric tolerance should permit an in-range integer difference");
}

void test_shape_type_order_and_sample_limit() {
    PolyglotParityPolicy policy;
    policy.max_mismatch_samples = 2U;
    PolyglotParityComparisonRequest request;
    request.capability_id = "forms.customer.open";
    request.fields = {
        {"missing", "", "string", "", "x"},
        {"extra", "string", "", "x", ""},
        {"kind", "integer", "string", "1", "1"},
        {"value", "string", "string", "a", "b"},
        {"duplicate", "string", "string", "a", "a"},
        {"duplicate", "string", "string", "a", "a"}};
    request.native_order = {"a", "b"};
    request.candidate_order = {"b", "a"};
    const auto result = copperfin::platform::compare_polyglot_outputs(policy, request);
    expect(result.mismatch_count == 6U && result.mismatch_samples.size() == 2U,
           "comparator should count all mismatches while bounding samples");
    expect(result.first_mismatch == PolyglotParityMismatchCategory::missing_field,
           "comparator should preserve the first mismatch category");
    expect(result.telemetry.back().event_name == "polyglot.parity.mismatch" &&
               result.telemetry.back().mismatch_count == 6U,
           "mismatch telemetry should carry the aggregate count");

    policy.ignore_order = true;
    const auto ignored_order = copperfin::platform::compare_polyglot_outputs(policy, request);
    expect(ignored_order.mismatch_count == 5U,
           "ignore-order policy should suppress only the ordering mismatch");
}

void test_failure_parity_and_caller_behavior() {
    PolyglotParityComparisonRequest request;
    request.capability_id = "reports.invoice.render";
    request.native_success = false;
    request.candidate_success = true;
    request.native_error_code = "native.timeout";
    const auto native_failure = copperfin::platform::compare_polyglot_outputs({}, request);
    expect(native_failure.first_mismatch == PolyglotParityMismatchCategory::native_failure &&
               native_failure.return_native,
           "native failure should be visible without changing shadow caller behavior");

    request.native_success = false;
    request.candidate_success = false;
    request.candidate_error_code = "candidate.timeout";
    const auto error_mismatch = copperfin::platform::compare_polyglot_outputs({}, request);
    expect(error_mismatch.first_mismatch == PolyglotParityMismatchCategory::error &&
               error_mismatch.reason_code == "polyglot.parity.error_mismatch",
           "different native/candidate errors should emit an error mismatch");

    request.candidate_error_code = request.native_error_code;
    const auto same_error = copperfin::platform::compare_polyglot_outputs({}, request);
    expect(same_error.parity_match,
           "identical native/candidate error contracts should compare as parity");
}

}  // namespace

int main() {
    test_match_and_tolerance();
    test_shape_type_order_and_sample_limit();
    test_failure_parity_and_caller_behavior();
    if (failures != 0) {
        std::cerr << failures << " polyglot parity comparator test(s) failed\n";
        return 1;
    }
    std::cout << "All polyglot parity comparator tests passed\n";
    return 0;
}
