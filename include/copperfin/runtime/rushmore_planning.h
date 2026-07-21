// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include <cstdint>
#include <limits>
#include <optional>
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

enum class RushmoreStatisticsState : std::uint8_t {
    absent = 0,
    fresh = 1,
    stale = 2,
    corrupted = 3
};

struct RushmoreCursorStatisticsDescriptor {
    RushmoreStatisticsState state = RushmoreStatisticsState::absent;
    std::uint64_t version = 0;
    std::optional<std::uint64_t> record_count;
    std::optional<std::uint32_t> record_length;
    std::optional<std::uint64_t> approximate_distinct_count;
    // Density is represented in parts per million to avoid platform-dependent floating point behavior.
    std::optional<std::uint32_t> density_parts_per_million;

    friend bool operator==(
        const RushmoreCursorStatisticsDescriptor&,
        const RushmoreCursorStatisticsDescriptor&) = default;
};

struct RushmoreCursorMetadata {
    std::string cursor_identity;
    std::string index_signature;
    std::uint64_t row_count = 0;
    std::uint64_t stats_version = 0;
    std::optional<RushmoreCursorStatisticsDescriptor> statistics;

    friend bool operator==(const RushmoreCursorMetadata&, const RushmoreCursorMetadata&) = default;
};

[[nodiscard]] constexpr bool rushmore_statistics_state_is_usable(RushmoreStatisticsState state) noexcept {
    return state == RushmoreStatisticsState::fresh;
}

[[nodiscard]] constexpr bool rushmore_statistics_are_structurally_valid(
    const RushmoreCursorStatisticsDescriptor& statistics) noexcept {
    if (statistics.state == RushmoreStatisticsState::absent) {
        return statistics.version == 0 && !statistics.record_count.has_value() &&
            !statistics.record_length.has_value() && !statistics.approximate_distinct_count.has_value() &&
            !statistics.density_parts_per_million.has_value();
    }
    if (statistics.state == RushmoreStatisticsState::corrupted || statistics.version == 0) {
        return false;
    }
    if (statistics.record_length.has_value() && statistics.record_length.value() == 0U) {
        return false;
    }
    if (statistics.approximate_distinct_count.has_value() && statistics.record_count.has_value() &&
        statistics.approximate_distinct_count.value() > statistics.record_count.value()) {
        return false;
    }
    return !statistics.density_parts_per_million.has_value() ||
        statistics.density_parts_per_million.value() <= 1'000'000U;
}

[[nodiscard]] constexpr std::uint64_t rushmore_next_statistics_version(
    std::uint64_t current_version) noexcept {
    return current_version == std::numeric_limits<std::uint64_t>::max() ? 1U : current_version + 1U;
}

constexpr void rushmore_invalidate_statistics(RushmoreCursorStatisticsDescriptor& statistics) noexcept {
    statistics.state = RushmoreStatisticsState::stale;
    statistics.version = rushmore_next_statistics_version(statistics.version);
}

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
