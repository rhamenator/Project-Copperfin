// Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::platform {

enum class PolyglotParityMismatchCategory {
    none,
    native_failure,
    candidate_failure,
    error,
    missing_field,
    extra_field,
    type,
    value,
    ordering,
    shape
};

struct PolyglotParityField {
    std::string path;
    std::string native_type;
    std::string candidate_type;
    std::string native_value;
    std::string candidate_value;
};

struct PolyglotParityPolicy {
    std::uint64_t numeric_tolerance = 0U;
    bool ignore_order = false;
    std::uint32_t max_mismatch_samples = 8U;
};

struct PolyglotParityComparisonRequest {
    std::string capability_id;
    bool native_success = true;
    bool candidate_success = true;
    std::string native_error_code;
    std::string candidate_error_code;
    std::vector<PolyglotParityField> fields;
    std::vector<std::string> native_order;
    std::vector<std::string> candidate_order;
};

struct PolyglotParityTelemetryEvent {
    std::string event_name;
    std::string capability_id;
    std::string reason_code;
    std::uint32_t mismatch_count = 0U;
    bool parity_match = false;
};

struct PolyglotParityComparisonResult {
    bool return_native = true;
    bool parity_match = true;
    std::uint32_t mismatch_count = 0U;
    PolyglotParityMismatchCategory first_mismatch = PolyglotParityMismatchCategory::none;
    std::string reason_code;
    std::vector<std::string> mismatch_samples;
    std::vector<PolyglotParityTelemetryEvent> telemetry;
};

[[nodiscard]] PolyglotParityComparisonResult compare_polyglot_outputs(
    const PolyglotParityPolicy& policy,
    const PolyglotParityComparisonRequest& request);
[[nodiscard]] const char* polyglot_parity_mismatch_name(
    PolyglotParityMismatchCategory category) noexcept;

}  // namespace copperfin::platform
