// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <algorithm>
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

[[nodiscard]] constexpr const char* rushmore_statistics_state_name(
    RushmoreStatisticsState state) noexcept {
    switch (state) {
    case RushmoreStatisticsState::absent:
        return "absent";
    case RushmoreStatisticsState::fresh:
        return "fresh";
    case RushmoreStatisticsState::stale:
        return "stale";
    case RushmoreStatisticsState::corrupted:
        return "corrupted";
    }
    return "corrupted";
}

[[nodiscard]] constexpr const char* rushmore_statistics_state_catalog_key(
    RushmoreStatisticsState state) noexcept {
    switch (state) {
    case RushmoreStatisticsState::absent:
        return "Runtime.IndexSeek.Explain.Statistics.Absent";
    case RushmoreStatisticsState::fresh:
        return "Runtime.IndexSeek.Explain.Statistics.Fresh";
    case RushmoreStatisticsState::stale:
        return "Runtime.IndexSeek.Explain.Statistics.Stale";
    case RushmoreStatisticsState::corrupted:
        return "Runtime.IndexSeek.Explain.Statistics.Corrupted";
    }
    return "Runtime.IndexSeek.Explain.Statistics.Corrupted";
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

enum class RushmoreBitmapCombination : std::uint8_t {
    conjunction = 0,
    disjunction = 1
};

struct RushmoreBitmapOrderCandidate {
    std::string order_name;
    std::string order_signature;
    RushmorePredicateDescriptor predicate{};
    RushmorePlanCost single_index_cost{};
    bool usable = true;

    friend bool operator==(const RushmoreBitmapOrderCandidate&, const RushmoreBitmapOrderCandidate&) = default;
};

struct RushmoreBitmapCostOptions {
    std::uint64_t merge_base_cpu_units = 4;
    std::uint64_t merge_row_cpu_units = 1;
    std::uint64_t bitmap_row_memory_units = 1;

    friend bool operator==(const RushmoreBitmapCostOptions&, const RushmoreBitmapCostOptions&) = default;
};

struct RushmoreBitmapPlanningInput {
    RushmoreCursorMetadata cursor{};
    RushmoreBitmapCombination combination = RushmoreBitmapCombination::conjunction;
    std::vector<RushmoreBitmapOrderCandidate> order_candidates;
    std::vector<RushmoreResidualPredicateDescriptor> residual_predicates;
    std::optional<std::uint64_t> memory_budget_units;
    RushmoreBitmapCostOptions cost_options{};

    friend bool operator==(const RushmoreBitmapPlanningInput&, const RushmoreBitmapPlanningInput&) = default;
};

struct RushmoreBitmapPlanCandidate {
    RushmoreBitmapCombination combination = RushmoreBitmapCombination::conjunction;
    std::vector<RushmoreBitmapOrderCandidate> components;
    RushmorePlanCost cost{};
    std::vector<RushmoreResidualPredicateDescriptor> residual_predicates;

    friend bool operator==(const RushmoreBitmapPlanCandidate&, const RushmoreBitmapPlanCandidate&) = default;
    friend bool operator<(const RushmoreBitmapPlanCandidate& left, const RushmoreBitmapPlanCandidate& right) noexcept {
        if (left.cost != right.cost) {
            return left.cost < right.cost;
        }
        if (left.combination != right.combination) {
            return left.combination < right.combination;
        }
        const auto component_count = std::min(left.components.size(), right.components.size());
        for (std::size_t index = 0; index < component_count; ++index) {
            const auto& left_component = left.components[index];
            const auto& right_component = right.components[index];
            const auto left_key = std::tie(
                left_component.order_name,
                left_component.order_signature,
                left_component.predicate.normalized_expression);
            const auto right_key = std::tie(
                right_component.order_name,
                right_component.order_signature,
                right_component.predicate.normalized_expression);
            if (left_key != right_key) {
                return left_key < right_key;
            }
        }
        return left.components.size() < right.components.size();
    }
};

enum class RushmoreBitmapFallbackReason : std::uint8_t {
    none = 0,
    insufficient_candidates = 1,
    unsupported_predicate = 2,
    duplicate_order = 3,
    incompatible_predicates = 4,
    memory_risk_unknown = 5,
    memory_limit = 6
};

struct RushmoreBitmapPlanningDecision {
    bool allowed = false;
    RushmoreBitmapCombination combination = RushmoreBitmapCombination::conjunction;
    std::vector<RushmoreBitmapPlanCandidate> candidates;
    std::optional<RushmoreBitmapPlanCandidate> selected_candidate;
    std::vector<RushmoreResidualPredicateDescriptor> residual_predicates;
    RushmoreBitmapFallbackReason fallback_reason = RushmoreBitmapFallbackReason::none;

    friend bool operator==(const RushmoreBitmapPlanningDecision&, const RushmoreBitmapPlanningDecision&) = default;
};

[[nodiscard]] constexpr std::uint64_t rushmore_saturating_add(
    std::uint64_t left,
    std::uint64_t right) noexcept;

[[nodiscard]] constexpr std::uint64_t rushmore_saturating_multiply(
    std::uint64_t left,
    std::uint64_t right) noexcept;

[[nodiscard]] constexpr std::uint64_t rushmore_at_least_one(std::uint64_t value) noexcept;

[[nodiscard]] constexpr std::uint64_t rushmore_ceil_divide(
    std::uint64_t numerator,
    std::uint64_t denominator) noexcept;

[[nodiscard]] constexpr bool rushmore_bitmap_operation_is_supported(
    const std::string& operation) noexcept {
    return operation == "=" || operation == "==" || operation == "<" || operation == "<=" ||
        operation == ">" || operation == ">=" || operation == "LIKE";
}

[[nodiscard]] constexpr std::uint64_t rushmore_bitmap_component_rows(
    const RushmoreBitmapOrderCandidate& candidate,
    const RushmoreCursorMetadata& cursor) noexcept {
    return candidate.single_index_cost.estimated_rows != 0U
        ? candidate.single_index_cost.estimated_rows
        : rushmore_at_least_one(cursor.row_count);
}

[[nodiscard]] inline RushmorePlanCost rushmore_estimate_bitmap_cost(
    const RushmoreBitmapPlanningInput& input,
    const std::vector<RushmoreBitmapOrderCandidate>& components) noexcept {
    if (components.size() < 2U) {
        return {};
    }
    const auto row_count = rushmore_at_least_one(input.cursor.row_count);
    const auto left_rows = rushmore_bitmap_component_rows(components[0], input.cursor);
    const auto right_rows = rushmore_bitmap_component_rows(components[1], input.cursor);
    std::uint64_t estimated_rows = 0;
    if (input.combination == RushmoreBitmapCombination::conjunction) {
        estimated_rows = rushmore_ceil_divide(
            rushmore_saturating_multiply(left_rows, right_rows), row_count);
    }
    else {
        estimated_rows = std::min(
            row_count,
            rushmore_saturating_add(left_rows, right_rows));
    }
    estimated_rows = rushmore_at_least_one(estimated_rows);
    const auto component_cpu_units = rushmore_saturating_add(
        components[0].single_index_cost.cpu_units,
        components[1].single_index_cost.cpu_units);
    const auto merge_cpu_units = rushmore_saturating_add(
        input.cost_options.merge_base_cpu_units,
        rushmore_saturating_multiply(estimated_rows, input.cost_options.merge_row_cpu_units));
    const auto memory_units = rushmore_saturating_add(
        rushmore_saturating_add(
            components[0].single_index_cost.memory_units,
            components[1].single_index_cost.memory_units),
        rushmore_saturating_multiply(estimated_rows, input.cost_options.bitmap_row_memory_units));
    const auto cpu_units = rushmore_saturating_add(component_cpu_units, merge_cpu_units);
    return RushmorePlanCost{
        estimated_rows,
        cpu_units,
        memory_units,
        rushmore_saturating_add(cpu_units, memory_units)};
}

[[nodiscard]] inline RushmoreBitmapPlanningDecision rushmore_plan_bitmap_candidates(
    const RushmoreBitmapPlanningInput& input) {
    RushmoreBitmapPlanningDecision decision;
    decision.combination = input.combination;
    decision.residual_predicates = input.residual_predicates;
    const auto preserve_all_predicates = [&]() {
        for (const auto& candidate : input.order_candidates) {
            decision.residual_predicates.push_back({
                candidate.predicate.normalized_expression,
                candidate.predicate.complexity_units});
        }
    };
    if (input.combination != RushmoreBitmapCombination::conjunction &&
        input.combination != RushmoreBitmapCombination::disjunction) {
        decision.fallback_reason = RushmoreBitmapFallbackReason::incompatible_predicates;
        preserve_all_predicates();
        return decision;
    }
    if (input.order_candidates.size() < 2U) {
        decision.fallback_reason = RushmoreBitmapFallbackReason::insufficient_candidates;
        preserve_all_predicates();
        return decision;
    }
    for (const auto& candidate : input.order_candidates) {
        if (!candidate.usable || candidate.order_name.empty() || candidate.order_signature.empty()) {
            decision.fallback_reason = RushmoreBitmapFallbackReason::unsupported_predicate;
            preserve_all_predicates();
            return decision;
        }
        if (!rushmore_bitmap_operation_is_supported(candidate.predicate.operation) ||
            candidate.predicate.field_name.empty() || candidate.predicate.normalized_expression.empty()) {
            decision.fallback_reason = RushmoreBitmapFallbackReason::unsupported_predicate;
            preserve_all_predicates();
            return decision;
        }
        for (const auto& prior : input.order_candidates) {
            if (&prior != &candidate && prior.order_name == candidate.order_name) {
                decision.fallback_reason = RushmoreBitmapFallbackReason::duplicate_order;
                preserve_all_predicates();
                return decision;
            }
        }
    }
    for (std::size_t left = 0; left < input.order_candidates.size(); ++left) {
        for (std::size_t right = left + 1U; right < input.order_candidates.size(); ++right) {
            const auto& left_candidate = input.order_candidates[left];
            const auto& right_candidate = input.order_candidates[right];
            if (left_candidate.predicate.normalized_expression ==
                right_candidate.predicate.normalized_expression) {
                continue;
            }
            if (input.combination == RushmoreBitmapCombination::conjunction &&
                left_candidate.predicate.field_name == right_candidate.predicate.field_name) {
                continue;
            }
            std::vector<RushmoreBitmapOrderCandidate> components{left_candidate, right_candidate};
            const auto cost = rushmore_estimate_bitmap_cost(input, components);
            decision.candidates.push_back({
                input.combination,
                components,
                cost,
                input.residual_predicates});
        }
    }
    if (decision.candidates.empty()) {
        decision.fallback_reason = RushmoreBitmapFallbackReason::incompatible_predicates;
        preserve_all_predicates();
        return decision;
    }
    std::sort(decision.candidates.begin(), decision.candidates.end());
    if (!input.memory_budget_units.has_value()) {
        decision.fallback_reason = RushmoreBitmapFallbackReason::memory_risk_unknown;
        preserve_all_predicates();
        return decision;
    }
    const auto candidate = std::find_if(
        decision.candidates.begin(),
        decision.candidates.end(),
        [&](const RushmoreBitmapPlanCandidate& plan) {
            return plan.cost.memory_units <= input.memory_budget_units.value();
        });
    if (candidate == decision.candidates.end()) {
        decision.fallback_reason = RushmoreBitmapFallbackReason::memory_limit;
        preserve_all_predicates();
        return decision;
    }
    decision.allowed = true;
    decision.selected_candidate = *candidate;
    return decision;
}

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

[[nodiscard]] constexpr const char* rushmore_plan_kind_catalog_key(
    RushmorePlanKind kind) noexcept {
    switch (kind) {
    case RushmorePlanKind::table_scan:
        return "Runtime.IndexSeek.Explain.PlanKind.TableScan";
    case RushmorePlanKind::index_seek:
        return "Runtime.IndexSeek.Explain.PlanKind.IndexSeek";
    case RushmorePlanKind::index_range_scan:
        return "Runtime.IndexSeek.Explain.PlanKind.IndexRangeScan";
    }
    return "Runtime.IndexSeek.Explain.PlanKind.TableScan";
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

[[nodiscard]] constexpr const char* rushmore_bitmap_fallback_reason_name(
    RushmoreBitmapFallbackReason reason) noexcept {
    switch (reason) {
    case RushmoreBitmapFallbackReason::none:
        return "none";
    case RushmoreBitmapFallbackReason::insufficient_candidates:
        return "insufficient_candidates";
    case RushmoreBitmapFallbackReason::unsupported_predicate:
        return "unsupported_predicate";
    case RushmoreBitmapFallbackReason::duplicate_order:
        return "duplicate_order";
    case RushmoreBitmapFallbackReason::incompatible_predicates:
        return "incompatible_predicates";
    case RushmoreBitmapFallbackReason::memory_risk_unknown:
        return "memory_risk_unknown";
    case RushmoreBitmapFallbackReason::memory_limit:
        return "memory_limit";
    }
    return "memory_limit";
}

[[nodiscard]] constexpr const char* rushmore_bitmap_fallback_reason_catalog_key(
    RushmoreBitmapFallbackReason reason) noexcept {
    switch (reason) {
    case RushmoreBitmapFallbackReason::none:
        return "Runtime.IndexSeek.Bitmap.Fallback.None";
    case RushmoreBitmapFallbackReason::insufficient_candidates:
        return "Runtime.IndexSeek.Bitmap.Fallback.InsufficientCandidates";
    case RushmoreBitmapFallbackReason::unsupported_predicate:
        return "Runtime.IndexSeek.Bitmap.Fallback.UnsupportedPredicate";
    case RushmoreBitmapFallbackReason::duplicate_order:
        return "Runtime.IndexSeek.Bitmap.Fallback.DuplicateOrder";
    case RushmoreBitmapFallbackReason::incompatible_predicates:
        return "Runtime.IndexSeek.Bitmap.Fallback.IncompatiblePredicates";
    case RushmoreBitmapFallbackReason::memory_risk_unknown:
        return "Runtime.IndexSeek.Bitmap.Fallback.MemoryRiskUnknown";
    case RushmoreBitmapFallbackReason::memory_limit:
        return "Runtime.IndexSeek.Bitmap.Fallback.MemoryLimit";
    }
    return "Runtime.IndexSeek.Bitmap.Fallback.MemoryLimit";
}

}  // namespace copperfin::runtime
