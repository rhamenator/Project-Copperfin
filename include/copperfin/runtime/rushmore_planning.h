// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include <cstdint>
#include <string>
#include <tuple>

namespace copperfin::runtime {

enum class RushmorePlanKind : std::uint8_t {
    table_scan = 0,
    index_seek = 1,
    index_range_scan = 2
};

struct RushmorePlanCost {
    std::uint64_t estimated_rows = 0;
    std::uint64_t cpu_units = 0;
    std::uint64_t memory_units = 0;
    std::uint64_t total_units = 0;

    friend bool operator==(const RushmorePlanCost&, const RushmorePlanCost&) = default;
    friend bool operator<(const RushmorePlanCost& left, const RushmorePlanCost& right) noexcept {
        return std::tie(left.total_units, left.estimated_rows, left.cpu_units, left.memory_units) <
            std::tie(right.total_units, right.estimated_rows, right.cpu_units, right.memory_units);
    }
};

struct RushmorePredicateDescriptor {
    std::string normalized_expression;
    std::string field_name;
    std::string operation;
    std::uint32_t complexity_units = 0;
    bool exact_match = false;

    friend bool operator==(const RushmorePredicateDescriptor&, const RushmorePredicateDescriptor&) = default;
};

struct RushmoreResidualPredicateDescriptor {
    std::string normalized_expression;
    std::uint32_t complexity_units = 0;

    friend bool operator==(const RushmoreResidualPredicateDescriptor&, const RushmoreResidualPredicateDescriptor&) = default;
};

struct RushmoreCursorMetadata {
    std::string cursor_identity;
    std::string index_signature;
    std::uint64_t row_count = 0;
    std::uint64_t stats_version = 0;

    friend bool operator==(const RushmoreCursorMetadata&, const RushmoreCursorMetadata&) = default;
};

struct RushmorePlanCacheKey {
    std::string cursor_identity;
    std::string normalized_expression;
    std::string index_signature;
    std::uint64_t stats_version = 0;
    std::uint64_t options_version = 0;

    friend bool operator==(const RushmorePlanCacheKey&, const RushmorePlanCacheKey&) = default;
    friend bool operator<(const RushmorePlanCacheKey& left, const RushmorePlanCacheKey& right) noexcept {
        return std::tie(
                   left.cursor_identity,
                   left.normalized_expression,
                   left.index_signature,
                   left.stats_version,
                   left.options_version) <
            std::tie(
                right.cursor_identity,
                right.normalized_expression,
                right.index_signature,
                right.stats_version,
                right.options_version);
    }
};

struct RushmorePlanCandidate {
    RushmorePlanKind kind = RushmorePlanKind::table_scan;
    std::string index_name;
    RushmorePlanCost cost{};
    RushmoreResidualPredicateDescriptor residual{};

    friend bool operator==(const RushmorePlanCandidate&, const RushmorePlanCandidate&) = default;
    friend bool operator<(const RushmorePlanCandidate& left, const RushmorePlanCandidate& right) noexcept {
        return std::tie(left.cost, left.kind, left.index_name, left.residual.normalized_expression) <
            std::tie(right.cost, right.kind, right.index_name, right.residual.normalized_expression);
    }
};

struct RushmoreExplainRecord {
    std::string cursor_identity;
    RushmorePlanKind kind = RushmorePlanKind::table_scan;
    std::string index_name;
    RushmorePlanCost cost{};
    bool selected = false;

    friend bool operator==(const RushmoreExplainRecord&, const RushmoreExplainRecord&) = default;
};

struct RushmorePlanningOptions {
    bool enabled = false;
    bool allow_legacy_fallback = true;
    std::uint64_t options_version = 0;

    friend bool operator==(const RushmorePlanningOptions&, const RushmorePlanningOptions&) = default;
};

[[nodiscard]] constexpr const char* rushmore_plan_kind_name(RushmorePlanKind kind) noexcept {
    switch (kind) {
    case RushmorePlanKind::table_scan:
        return "table_scan";
    case RushmorePlanKind::index_seek:
        return "index_seek";
    case RushmorePlanKind::index_range_scan:
        return "index_range_scan";
    }
    return "table_scan";
}

}  // namespace copperfin::runtime
