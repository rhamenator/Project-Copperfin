// Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/platform/polyglot_parity_comparator.h"

#include <algorithm>
#include <charconv>
#include <system_error>

namespace copperfin::platform {

namespace {

const char* mismatch_reason(PolyglotParityMismatchCategory category) noexcept {
    switch (category) {
    case PolyglotParityMismatchCategory::none:
        return "polyglot.parity.match";
    case PolyglotParityMismatchCategory::native_failure:
        return "polyglot.parity.native_failure";
    case PolyglotParityMismatchCategory::candidate_failure:
        return "polyglot.parity.candidate_failure";
    case PolyglotParityMismatchCategory::error:
        return "polyglot.parity.error_mismatch";
    case PolyglotParityMismatchCategory::missing_field:
        return "polyglot.parity.missing_field";
    case PolyglotParityMismatchCategory::extra_field:
        return "polyglot.parity.extra_field";
    case PolyglotParityMismatchCategory::type:
        return "polyglot.parity.type_mismatch";
    case PolyglotParityMismatchCategory::value:
        return "polyglot.parity.value_mismatch";
    case PolyglotParityMismatchCategory::ordering:
        return "polyglot.parity.ordering_mismatch";
    case PolyglotParityMismatchCategory::shape:
        return "polyglot.parity.shape_mismatch";
    }
    return "polyglot.parity.shape_mismatch";
}

bool is_numeric_type(const std::string& type) noexcept {
    return type == "integer" || type == "number";
}

bool parse_integer(const std::string& text, std::int64_t& value) noexcept {
    if (text.empty()) {
        return false;
    }
    const auto parsed = std::from_chars(
        text.data(), text.data() + text.size(), value);
    return parsed.ec == std::errc{} && parsed.ptr == text.data() + text.size();
}

std::uint64_t magnitude(std::int64_t value) noexcept {
    if (value >= 0) {
        return static_cast<std::uint64_t>(value);
    }
    return static_cast<std::uint64_t>(-(value + 1)) + 1U;
}

bool within_numeric_tolerance(
    const std::string& native_value,
    const std::string& candidate_value,
    std::uint64_t tolerance) noexcept {
    std::int64_t native = 0;
    std::int64_t candidate = 0;
    if (!parse_integer(native_value, native) || !parse_integer(candidate_value, candidate)) {
        return false;
    }
    if ((native < 0) != (candidate < 0)) {
        return magnitude(native) + magnitude(candidate) <= tolerance;
    }
    const std::uint64_t native_magnitude = magnitude(native);
    const std::uint64_t candidate_magnitude = magnitude(candidate);
    const std::uint64_t difference = native_magnitude >= candidate_magnitude
        ? native_magnitude - candidate_magnitude
        : candidate_magnitude - native_magnitude;
    return difference <= tolerance;
}

void record_mismatch(
    const PolyglotParityPolicy& policy,
    const PolyglotParityComparisonRequest& request,
    PolyglotParityComparisonResult& result,
    PolyglotParityMismatchCategory category,
    const std::string& path) {
    result.parity_match = false;
    ++result.mismatch_count;
    if (result.first_mismatch == PolyglotParityMismatchCategory::none) {
        result.first_mismatch = category;
        result.reason_code = mismatch_reason(category);
    }
    if (result.mismatch_samples.size() < policy.max_mismatch_samples) {
        result.mismatch_samples.push_back(
            request.capability_id + "|" + mismatch_reason(category) + "|" + path);
    }
}

}  // namespace

PolyglotParityComparisonResult compare_polyglot_outputs(
    const PolyglotParityPolicy& policy,
    const PolyglotParityComparisonRequest& request) {
    PolyglotParityComparisonResult result;
    if (!request.native_success && request.candidate_success) {
        record_mismatch(policy, request, result,
                        PolyglotParityMismatchCategory::native_failure, "<native>");
    } else if (request.native_success && !request.candidate_success) {
        record_mismatch(policy, request, result,
                        PolyglotParityMismatchCategory::candidate_failure, "<candidate>");
    } else if (!request.native_success && !request.candidate_success &&
               request.native_error_code != request.candidate_error_code) {
        record_mismatch(policy, request, result,
                        PolyglotParityMismatchCategory::error, "<error>");
    }

    std::vector<std::string> seen_paths;
    seen_paths.reserve(request.fields.size());
    for (const PolyglotParityField& field : request.fields) {
        if (std::find(seen_paths.begin(), seen_paths.end(), field.path) != seen_paths.end()) {
            record_mismatch(policy, request, result,
                            PolyglotParityMismatchCategory::shape, field.path);
            continue;
        }
        seen_paths.push_back(field.path);

        const bool native_present = !field.native_type.empty();
        const bool candidate_present = !field.candidate_type.empty();
        if (!native_present && candidate_present) {
            record_mismatch(policy, request, result,
                            PolyglotParityMismatchCategory::missing_field, field.path);
            continue;
        }
        if (native_present && !candidate_present) {
            record_mismatch(policy, request, result,
                            PolyglotParityMismatchCategory::extra_field, field.path);
            continue;
        }
        if (!native_present) {
            record_mismatch(policy, request, result,
                            PolyglotParityMismatchCategory::shape, field.path);
            continue;
        }
        if (field.native_type != field.candidate_type) {
            record_mismatch(policy, request, result,
                            PolyglotParityMismatchCategory::type, field.path);
            continue;
        }
        if (field.native_value == field.candidate_value) {
            continue;
        }
        if (is_numeric_type(field.native_type) &&
            within_numeric_tolerance(
                field.native_value,
                field.candidate_value,
                policy.numeric_tolerance)) {
            continue;
        }
        record_mismatch(policy, request, result,
                        PolyglotParityMismatchCategory::value, field.path);
    }

    if (!policy.ignore_order && request.native_order != request.candidate_order) {
        record_mismatch(policy, request, result,
                        PolyglotParityMismatchCategory::ordering, "<order>");
    }

    result.telemetry.push_back(
        PolyglotParityTelemetryEvent{
            "polyglot.parity.checked",
            request.capability_id,
            result.reason_code.empty() ? "polyglot.parity.match" : result.reason_code,
            result.mismatch_count,
            result.parity_match});
    if (!result.parity_match) {
        result.telemetry.push_back(
            PolyglotParityTelemetryEvent{
                "polyglot.parity.mismatch",
                request.capability_id,
                result.reason_code,
                result.mismatch_count,
                false});
    }
    return result;
}

const char* polyglot_parity_mismatch_name(
    PolyglotParityMismatchCategory category) noexcept {
    switch (category) {
    case PolyglotParityMismatchCategory::none:
        return "none";
    case PolyglotParityMismatchCategory::native_failure:
        return "native-failure";
    case PolyglotParityMismatchCategory::candidate_failure:
        return "candidate-failure";
    case PolyglotParityMismatchCategory::error:
        return "error";
    case PolyglotParityMismatchCategory::missing_field:
        return "missing-field";
    case PolyglotParityMismatchCategory::extra_field:
        return "extra-field";
    case PolyglotParityMismatchCategory::type:
        return "type";
    case PolyglotParityMismatchCategory::value:
        return "value";
    case PolyglotParityMismatchCategory::ordering:
        return "ordering";
    case PolyglotParityMismatchCategory::shape:
        return "shape";
    }
    return "shape";
}

}  // namespace copperfin::platform
