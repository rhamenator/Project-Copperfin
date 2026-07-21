// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

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

struct RushmoreCostModelOptions {
    std::uint64_t table_scan_base_cpu_units = 8;
    std::uint64_t index_seek_base_cpu_units = 3;
    std::uint64_t index_range_base_cpu_units = 5;
    std::uint64_t row_cpu_units = 1;
    std::uint64_t predicate_cpu_units = 2;
    std::uint64_t residual_cpu_units = 3;
    std::uint64_t temporary_row_memory_units = 1;
    std::uint64_t conservative_selectivity_divisor = 10;
    std::uint64_t range_selectivity_divisor = 4;

    friend bool operator==(const RushmoreCostModelOptions&, const RushmoreCostModelOptions&) = default;
};

struct RushmoreCostModelInput {
    RushmorePlanKind kind = RushmorePlanKind::table_scan;
    RushmoreCursorMetadata cursor{};
    std::optional<RushmorePredicateDescriptor> predicate;
    std::uint32_t residual_complexity_units = 0;

    friend bool operator==(const RushmoreCostModelInput&, const RushmoreCostModelInput&) = default;
};

enum class RushmoreRemoteCapabilityState : std::uint8_t {
    unknown = 0,
    limited = 1,
    known = 2,
    unsupported = 3
};

struct RushmoreRemoteProviderCapabilities {
    RushmoreRemoteCapabilityState state = RushmoreRemoteCapabilityState::unknown;
    bool predicate_pushdown = false;
    bool equality_pushdown = false;
    bool range_pushdown = false;
    bool like_pushdown = false;
    bool order_pushdown = false;
    bool collation_preservation = false;
    std::uint32_t maximum_predicate_complexity = 0;

    friend bool operator==(
        const RushmoreRemoteProviderCapabilities&,
        const RushmoreRemoteProviderCapabilities&) = default;
};

enum class RushmoreRemoteRoundTripRisk : std::uint8_t {
    none = 0,
    unknown = 1,
    elevated = 2
};

enum class RushmoreRemoteFallbackReason : std::uint8_t {
    none = 0,
    not_remote_cursor = 1,
    unknown_capabilities = 2,
    unsupported_capability = 3,
    local_residual_required = 4
};

struct RushmoreRemotePlanningInput {
    bool remote_cursor = true;
    std::string provider_identity;
    RushmoreCursorMetadata cursor{};
    RushmoreRemoteProviderCapabilities capabilities{};
    std::vector<RushmorePredicateDescriptor> predicates;
    std::vector<RushmoreResidualPredicateDescriptor> residual_predicates;

    friend bool operator==(const RushmoreRemotePlanningInput&, const RushmoreRemotePlanningInput&) = default;
};

struct RushmoreRemotePlanningDecision {
    bool provider_pushdown_allowed = false;
    std::string provider_identity;
    std::vector<RushmorePredicateDescriptor> pushdown_predicates;
    std::vector<RushmoreResidualPredicateDescriptor> local_residual_predicates;
    RushmoreRemoteFallbackReason fallback_reason = RushmoreRemoteFallbackReason::none;
    RushmoreRemoteRoundTripRisk round_trip_risk = RushmoreRemoteRoundTripRisk::none;

    friend bool operator==(const RushmoreRemotePlanningDecision&, const RushmoreRemotePlanningDecision&) = default;
};

[[nodiscard]] constexpr bool rushmore_remote_operation_is_pushdown_safe(
    const RushmorePredicateDescriptor& predicate,
    const RushmoreRemoteProviderCapabilities& capabilities) noexcept {
    if (!capabilities.predicate_pushdown ||
        (capabilities.maximum_predicate_complexity != 0U &&
         predicate.complexity_units > capabilities.maximum_predicate_complexity)) {
        return false;
    }
    if (predicate.operation == "=" || predicate.operation == "==") {
        return capabilities.equality_pushdown;
    }
    if (predicate.operation == "<" || predicate.operation == "<=" ||
        predicate.operation == ">" || predicate.operation == ">=") {
        return capabilities.range_pushdown;
    }
    if (predicate.operation == "LIKE") {
        return capabilities.like_pushdown;
    }
    return false;
}

[[nodiscard]] inline RushmoreRemotePlanningDecision rushmore_plan_remote_predicates(
    const RushmoreRemotePlanningInput& input) {
    RushmoreRemotePlanningDecision decision;
    decision.provider_identity = input.provider_identity;
    decision.local_residual_predicates = input.residual_predicates;
    if (!input.remote_cursor) {
        decision.fallback_reason = RushmoreRemoteFallbackReason::not_remote_cursor;
        decision.local_residual_predicates.reserve(
            decision.local_residual_predicates.size() + input.predicates.size());
    }

    const auto preserve_as_residual = [&](const RushmorePredicateDescriptor& predicate) {
        decision.local_residual_predicates.push_back({
            predicate.normalized_expression,
            predicate.complexity_units});
    };
    if (!input.remote_cursor) {
        for (const auto& predicate : input.predicates) {
            preserve_as_residual(predicate);
        }
        return decision;
    }

    if (input.capabilities.state == RushmoreRemoteCapabilityState::unknown) {
        decision.fallback_reason = RushmoreRemoteFallbackReason::unknown_capabilities;
        decision.round_trip_risk = RushmoreRemoteRoundTripRisk::unknown;
        for (const auto& predicate : input.predicates) {
            preserve_as_residual(predicate);
        }
        return decision;
    }
    if (input.capabilities.state == RushmoreRemoteCapabilityState::unsupported) {
        decision.fallback_reason = RushmoreRemoteFallbackReason::unsupported_capability;
        decision.round_trip_risk = RushmoreRemoteRoundTripRisk::elevated;
        for (const auto& predicate : input.predicates) {
            preserve_as_residual(predicate);
        }
        return decision;
    }

    for (const auto& predicate : input.predicates) {
        if (rushmore_remote_operation_is_pushdown_safe(predicate, input.capabilities)) {
            decision.pushdown_predicates.push_back(predicate);
        }
        else {
            preserve_as_residual(predicate);
        }
    }
    decision.provider_pushdown_allowed = !decision.pushdown_predicates.empty();
    if (!decision.provider_pushdown_allowed) {
        decision.fallback_reason = RushmoreRemoteFallbackReason::unsupported_capability;
        decision.round_trip_risk = RushmoreRemoteRoundTripRisk::elevated;
    }
    else if (!decision.local_residual_predicates.empty()) {
        decision.fallback_reason = RushmoreRemoteFallbackReason::local_residual_required;
        decision.round_trip_risk = RushmoreRemoteRoundTripRisk::elevated;
    }
    else if (!input.capabilities.order_pushdown || !input.capabilities.collation_preservation) {
        decision.round_trip_risk = RushmoreRemoteRoundTripRisk::elevated;
    }
    return decision;
}

[[nodiscard]] constexpr std::uint64_t rushmore_saturating_add(
    std::uint64_t left,
    std::uint64_t right) noexcept {
    const auto maximum = std::numeric_limits<std::uint64_t>::max();
    return left > maximum - right ? maximum : left + right;
}

[[nodiscard]] constexpr std::uint64_t rushmore_saturating_multiply(
    std::uint64_t left,
    std::uint64_t right) noexcept {
    const auto maximum = std::numeric_limits<std::uint64_t>::max();
    return left != 0U && right > maximum / left ? maximum : left * right;
}

[[nodiscard]] constexpr std::uint64_t rushmore_at_least_one(std::uint64_t value) noexcept {
    return value == 0U ? 1U : value;
}

[[nodiscard]] constexpr std::uint64_t rushmore_ceil_divide(
    std::uint64_t numerator,
    std::uint64_t denominator) noexcept {
    if (denominator == 0U) {
        return std::numeric_limits<std::uint64_t>::max();
    }
    return numerator == 0U ? 0U : 1U + (numerator - 1U) / denominator;
}

[[nodiscard]] constexpr std::uint64_t rushmore_cost_model_row_count(
    const RushmoreCostModelInput& input) noexcept {
    if (input.cursor.statistics.has_value() &&
        rushmore_statistics_state_is_usable(input.cursor.statistics->state) &&
        rushmore_statistics_are_structurally_valid(*input.cursor.statistics) &&
        input.cursor.statistics->record_count.has_value()) {
        return input.cursor.statistics->record_count.value();
    }
    return input.cursor.row_count;
}

[[nodiscard]] constexpr std::uint64_t rushmore_cost_model_estimated_rows(
    const RushmoreCostModelInput& input,
    const RushmoreCostModelOptions& options) noexcept {
    const auto row_count = rushmore_cost_model_row_count(input);
    if (input.kind == RushmorePlanKind::table_scan) {
        return rushmore_at_least_one(row_count);
    }

    const auto& statistics = input.cursor.statistics;
    if (statistics.has_value() && rushmore_statistics_state_is_usable(statistics->state) &&
        rushmore_statistics_are_structurally_valid(*statistics)) {
        if (input.predicate.has_value() && input.predicate->exact_match &&
            statistics->approximate_distinct_count.has_value() &&
            statistics->approximate_distinct_count.value() != 0U) {
            return rushmore_at_least_one(rushmore_ceil_divide(
                row_count,
                statistics->approximate_distinct_count.value()));
        }
        if (input.kind == RushmorePlanKind::index_range_scan &&
            statistics->density_parts_per_million.has_value()) {
            const auto weighted_rows = rushmore_saturating_multiply(
                row_count,
                statistics->density_parts_per_million.value());
            return rushmore_at_least_one(rushmore_ceil_divide(weighted_rows, 1'000'000U));
        }
    }

    const auto divisor = input.kind == RushmorePlanKind::index_range_scan
        ? options.range_selectivity_divisor
        : options.conservative_selectivity_divisor;
    return rushmore_at_least_one(rushmore_ceil_divide(row_count, divisor));
}

[[nodiscard]] constexpr RushmorePlanCost rushmore_estimate_plan_cost(
    const RushmoreCostModelInput& input,
    const RushmoreCostModelOptions& options = {}) noexcept {
    const auto estimated_rows = rushmore_cost_model_estimated_rows(input, options);
    const auto base_cpu_units = input.kind == RushmorePlanKind::table_scan
        ? options.table_scan_base_cpu_units
        : input.kind == RushmorePlanKind::index_range_scan
        ? options.index_range_base_cpu_units
        : options.index_seek_base_cpu_units;
    const auto predicate_units = input.predicate.has_value()
        ? rushmore_at_least_one(input.predicate->complexity_units)
        : 1U;
    const auto row_cpu_units = rushmore_saturating_multiply(estimated_rows, options.row_cpu_units);
    const auto predicate_cpu_units = rushmore_saturating_multiply(predicate_units, options.predicate_cpu_units);
    const auto residual_cpu_units = rushmore_saturating_multiply(
        input.residual_complexity_units,
        options.residual_cpu_units);
    const auto cpu_units = rushmore_saturating_add(
        rushmore_saturating_add(base_cpu_units, row_cpu_units),
        rushmore_saturating_add(predicate_cpu_units, residual_cpu_units));
    const auto memory_units = rushmore_saturating_multiply(
        estimated_rows,
        options.temporary_row_memory_units);
    return RushmorePlanCost{
        estimated_rows,
        cpu_units,
        memory_units,
        rushmore_saturating_add(cpu_units, memory_units)};
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

enum class RushmoreExplainFallbackReason : std::uint8_t {
    none = 0,
    planning_disabled = 1,
    unsupported_expression = 2,
    ambiguous_expression = 3,
    no_matching_index = 4,
    metadata_insufficient = 5,
    cost_rejected = 6,
    execution_fallback = 7
};

struct RushmoreExplainPlan {
    RushmoreCursorMetadata cursor{};
    std::string normalized_expression;
    std::vector<RushmorePredicateDescriptor> indexable_predicates;
    std::vector<RushmoreResidualPredicateDescriptor> residual_predicates;
    std::vector<RushmoreExplainRecord> candidates;
    std::optional<RushmoreExplainRecord> selected_candidate;
    RushmoreExplainFallbackReason fallback_reason = RushmoreExplainFallbackReason::none;
    std::uint64_t options_version = 0;

    friend bool operator==(const RushmoreExplainPlan&, const RushmoreExplainPlan&) = default;
};

struct RushmorePlanningOptions {
    bool enabled = false;
    bool allow_legacy_fallback = true;
    std::uint64_t options_version = 0;
    RushmoreCostModelOptions cost_model{};

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

[[nodiscard]] constexpr const char* rushmore_explain_fallback_reason_name(
    RushmoreExplainFallbackReason reason) noexcept {
    switch (reason) {
    case RushmoreExplainFallbackReason::none:
        return "none";
    case RushmoreExplainFallbackReason::planning_disabled:
        return "planning_disabled";
    case RushmoreExplainFallbackReason::unsupported_expression:
        return "unsupported_expression";
    case RushmoreExplainFallbackReason::ambiguous_expression:
        return "ambiguous_expression";
    case RushmoreExplainFallbackReason::no_matching_index:
        return "no_matching_index";
    case RushmoreExplainFallbackReason::metadata_insufficient:
        return "metadata_insufficient";
    case RushmoreExplainFallbackReason::cost_rejected:
        return "cost_rejected";
    case RushmoreExplainFallbackReason::execution_fallback:
        return "execution_fallback";
    }
    return "execution_fallback";
}

[[nodiscard]] constexpr const char* rushmore_explain_fallback_reason_catalog_key(
    RushmoreExplainFallbackReason reason) noexcept {
    switch (reason) {
    case RushmoreExplainFallbackReason::none:
        return "Runtime.IndexSeek.Explain.Fallback.None";
    case RushmoreExplainFallbackReason::planning_disabled:
        return "Runtime.IndexSeek.Explain.Fallback.PlanningDisabled";
    case RushmoreExplainFallbackReason::unsupported_expression:
        return "Runtime.IndexSeek.Explain.Fallback.UnsupportedExpression";
    case RushmoreExplainFallbackReason::ambiguous_expression:
        return "Runtime.IndexSeek.Explain.Fallback.AmbiguousExpression";
    case RushmoreExplainFallbackReason::no_matching_index:
        return "Runtime.IndexSeek.Explain.Fallback.NoMatchingIndex";
    case RushmoreExplainFallbackReason::metadata_insufficient:
        return "Runtime.IndexSeek.Explain.Fallback.MetadataInsufficient";
    case RushmoreExplainFallbackReason::cost_rejected:
        return "Runtime.IndexSeek.Explain.Fallback.CostRejected";
    case RushmoreExplainFallbackReason::execution_fallback:
        return "Runtime.IndexSeek.Explain.Fallback.ExecutionFallback";
    }
    return "Runtime.IndexSeek.Explain.Fallback.ExecutionFallback";
}

[[nodiscard]] constexpr const char* rushmore_remote_fallback_reason_name(
    RushmoreRemoteFallbackReason reason) noexcept {
    switch (reason) {
    case RushmoreRemoteFallbackReason::none:
        return "none";
    case RushmoreRemoteFallbackReason::not_remote_cursor:
        return "not_remote_cursor";
    case RushmoreRemoteFallbackReason::unknown_capabilities:
        return "unknown_capabilities";
    case RushmoreRemoteFallbackReason::unsupported_capability:
        return "unsupported_capability";
    case RushmoreRemoteFallbackReason::local_residual_required:
        return "local_residual_required";
    }
    return "unsupported_capability";
}

[[nodiscard]] constexpr const char* rushmore_remote_fallback_reason_catalog_key(
    RushmoreRemoteFallbackReason reason) noexcept {
    switch (reason) {
    case RushmoreRemoteFallbackReason::none:
        return "Runtime.IndexSeek.RemotePlan.Reason.None";
    case RushmoreRemoteFallbackReason::not_remote_cursor:
        return "Runtime.IndexSeek.RemotePlan.Reason.NotRemoteCursor";
    case RushmoreRemoteFallbackReason::unknown_capabilities:
        return "Runtime.IndexSeek.RemotePlan.Reason.UnknownCapabilities";
    case RushmoreRemoteFallbackReason::unsupported_capability:
        return "Runtime.IndexSeek.RemotePlan.Reason.UnsupportedCapability";
    case RushmoreRemoteFallbackReason::local_residual_required:
        return "Runtime.IndexSeek.RemotePlan.Reason.LocalResidualRequired";
    }
    return "Runtime.IndexSeek.RemotePlan.Reason.UnsupportedCapability";
}

[[nodiscard]] constexpr const char* rushmore_remote_round_trip_risk_name(
    RushmoreRemoteRoundTripRisk risk) noexcept {
    switch (risk) {
    case RushmoreRemoteRoundTripRisk::none:
        return "none";
    case RushmoreRemoteRoundTripRisk::unknown:
        return "unknown";
    case RushmoreRemoteRoundTripRisk::elevated:
        return "elevated";
    }
    return "unknown";
}

}  // namespace copperfin::runtime
