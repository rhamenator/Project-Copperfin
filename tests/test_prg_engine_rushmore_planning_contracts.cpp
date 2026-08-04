// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <limits>
#include <map>
#include <string>
#include <vector>

using namespace copperfin::test_support;
using copperfin::runtime::RushmorePlanCandidate;
using copperfin::runtime::RushmorePlanCacheKey;
using copperfin::runtime::RushmorePlanCost;
using copperfin::runtime::RushmorePlanKind;
using copperfin::runtime::RushmorePlanningOptions;
using copperfin::runtime::RushmoreCursorStatisticsDescriptor;
using copperfin::runtime::RushmoreCursorMetadata;
using copperfin::runtime::RushmoreStatisticsState;
using copperfin::runtime::RushmoreCostModelInput;
using copperfin::runtime::RushmoreCostModelOptions;
using copperfin::runtime::RushmorePredicateDescriptor;
using copperfin::runtime::RushmoreExplainFallbackReason;
using copperfin::runtime::RushmoreExplainPlan;
using copperfin::runtime::RushmoreExplainRecord;
using copperfin::runtime::RushmoreResidualPredicateDescriptor;
using copperfin::runtime::RushmoreRemoteCapabilityState;
using copperfin::runtime::RushmoreRemoteFallbackReason;
using copperfin::runtime::RushmoreRemotePlanningInput;
using copperfin::runtime::RushmoreRemoteRoundTripRisk;
using copperfin::runtime::RushmoreBitmapCombination;
using copperfin::runtime::RushmoreBitmapFallbackReason;
using copperfin::runtime::RushmoreBitmapOrderCandidate;
using copperfin::runtime::RushmoreBitmapPlanningInput;

void test_rushmore_planning_contracts() {
    const RushmorePlanningOptions defaults{};
    expect(!defaults.enabled, "Rushmore planning must remain disabled by default");
    expect(defaults.allow_legacy_fallback, "Rushmore planning must preserve legacy fallback by default");
    expect(defaults.options_version == 0, "Rushmore planning defaults must use the baseline options version");

    RushmorePlanCacheKey first{
        "people",
        "name = 'BRAVO'",
        "name:upper(name)",
        7,
        2};
    RushmorePlanCacheKey second = first;
    second.options_version = 3;
    expect(first < second, "Rushmore cache keys must order by options version after shared identity fields");
    std::map<RushmorePlanCacheKey, int> cache;
    cache.emplace(first, 1);
    cache.emplace(second, 2);
    expect(cache.size() == 2, "Rushmore cache keys must distinguish stats and options versions");

    const RushmorePlanCost seek_cost{2, 4, 1, 5};
    const RushmorePlanCost scan_cost{3, 4, 1, 5};
    RushmorePlanCandidate seek{RushmorePlanKind::index_seek, "name", seek_cost, {}};
    RushmorePlanCandidate scan{RushmorePlanKind::table_scan, {}, scan_cost, {}};
    std::vector<RushmorePlanCandidate> candidates{scan, seek};
    std::sort(candidates.begin(), candidates.end());
    expect(candidates.front().kind == RushmorePlanKind::index_seek,
        "Rushmore candidate ordering must prefer the lower deterministic total cost");
    expect(std::string(copperfin::runtime::rushmore_plan_kind_name(RushmorePlanKind::index_range_scan)) ==
            "index_range_scan",
        "Rushmore plan kind names must remain stable machine-readable identities");
    expect(std::string(copperfin::runtime::rushmore_plan_kind_catalog_key(RushmorePlanKind::index_seek)) ==
            "Runtime.IndexSeek.Explain.PlanKind.IndexSeek" &&
            std::string(copperfin::runtime::rushmore_statistics_state_name(RushmoreStatisticsState::fresh)) ==
            "fresh" &&
            std::string(copperfin::runtime::rushmore_statistics_state_catalog_key(
                RushmoreStatisticsState::stale)) == "Runtime.IndexSeek.Explain.Statistics.Stale",
        "Rushmore explain plan and statistics display identities must remain stable and localizable");

    copperfin::runtime::RuntimeSessionOptions session_options;
    expect(session_options.rushmore_planning == defaults,
        "Runtime session options must preserve legacy Rushmore behavior by default");

    RushmoreCursorStatisticsDescriptor missing{};
    expect(!copperfin::runtime::rushmore_statistics_state_is_usable(missing.state),
        "Missing cursor statistics must not be usable");
    expect(copperfin::runtime::rushmore_statistics_are_structurally_valid(missing),
        "Missing cursor statistics must be a valid absent descriptor");

    RushmoreCursorStatisticsDescriptor stale{
        RushmoreStatisticsState::stale,
        4,
        100,
        64,
        80,
        250'000};
    expect(!copperfin::runtime::rushmore_statistics_state_is_usable(stale.state),
        "Stale cursor statistics must not be usable without refresh");
    expect(copperfin::runtime::rushmore_statistics_are_structurally_valid(stale),
        "Stale cursor statistics must remain structurally inspectable");

    copperfin::runtime::rushmore_invalidate_statistics(stale);
    expect(stale.state == RushmoreStatisticsState::stale && stale.version == 5,
        "Cursor statistics invalidation must mark data stale and advance its version");

    RushmoreCursorStatisticsDescriptor corrupted{
        RushmoreStatisticsState::corrupted,
        5,
        100,
        64,
        101,
        1'000'001};
    expect(!copperfin::runtime::rushmore_statistics_are_structurally_valid(corrupted),
        "Corrupted cursor statistics must be rejected");

    RushmoreCursorStatisticsDescriptor refreshed{
        RushmoreStatisticsState::fresh,
        copperfin::runtime::rushmore_next_statistics_version(stale.version),
        100,
        64,
        80,
        250'000};
    expect(refreshed.state == RushmoreStatisticsState::fresh && refreshed.version == 6,
        "Refreshed cursor statistics must advance the version and become fresh");
    expect(copperfin::runtime::rushmore_statistics_state_is_usable(refreshed.state) &&
            copperfin::runtime::rushmore_statistics_are_structurally_valid(refreshed),
        "Refreshed cursor statistics must be usable when structurally valid");

    RushmoreCursorMetadata metadata{
        "people",
        "name:upper(name)",
        100,
        refreshed.version,
        refreshed};
    expect(metadata.statistics.has_value() && metadata.statistics->version == metadata.stats_version,
        "Cursor metadata must carry optional statistics without changing its stable identity fields");

    RushmoreCostModelInput table_scan_input{
        RushmorePlanKind::table_scan,
        {"people", "name:upper(name)", 100, 0, std::nullopt},
        std::nullopt,
        0};
    RushmoreCostModelInput seek_input{
        RushmorePlanKind::index_seek,
        {"people", "name:upper(name)", 100, refreshed.version, refreshed},
        RushmorePredicateDescriptor{"NAME = 'BRAVO'", "NAME", "=", 1, true},
        0};
    const auto table_scan_cost = copperfin::runtime::rushmore_estimate_plan_cost(table_scan_input);
    const auto seek_cost_estimate = copperfin::runtime::rushmore_estimate_plan_cost(seek_input);
    expect(table_scan_cost.estimated_rows == 100 && seek_cost_estimate.estimated_rows == 2,
        "Rushmore cost model must use conservative fallback and fresh distinct-count estimates");
    expect(seek_cost_estimate < table_scan_cost,
        "Rushmore cost model must prefer a clearly cheaper single-index seek");

    RushmoreCostModelOptions tuned_options{};
    tuned_options.predicate_cpu_units = 100;
    const auto tuned_seek_cost = copperfin::runtime::rushmore_estimate_plan_cost(seek_input, tuned_options);
    expect(tuned_seek_cost.cpu_units > seek_cost_estimate.cpu_units,
        "Rushmore cost model options must deterministically tune predicate CPU cost");

    RushmoreCostModelInput stale_input = seek_input;
    stale_input.cursor.statistics->state = RushmoreStatisticsState::stale;
    const auto stale_cost = copperfin::runtime::rushmore_estimate_plan_cost(stale_input);
    expect(stale_cost.estimated_rows == 10,
        "Rushmore cost model must use conservative estimates for stale statistics");

    RushmoreCostModelInput overflow_input = table_scan_input;
    overflow_input.cursor.row_count = std::numeric_limits<std::uint64_t>::max();
    RushmoreCostModelOptions overflow_options{};
    overflow_options.row_cpu_units = std::numeric_limits<std::uint64_t>::max();
    const auto overflow_cost = copperfin::runtime::rushmore_estimate_plan_cost(overflow_input, overflow_options);
    expect(overflow_cost.cpu_units == std::numeric_limits<std::uint64_t>::max() &&
            overflow_cost.total_units == std::numeric_limits<std::uint64_t>::max(),
        "Rushmore cost model must saturate arithmetic instead of wrapping costs");

    const RushmoreExplainRecord selected_record{
        "people",
        RushmorePlanKind::index_seek,
        "NAME",
        seek_cost_estimate,
        true};
    const RushmoreExplainRecord fallback_record{
        "people",
        RushmorePlanKind::table_scan,
        {},
        table_scan_cost,
        false};
    RushmoreExplainPlan optimized_explain{
        metadata,
        "NAME = 'BRAVO'",
        {RushmorePredicateDescriptor{"NAME = 'BRAVO'", "NAME", "=", 1, true}},
        {},
        {fallback_record, selected_record},
        selected_record,
        RushmoreExplainFallbackReason::none,
        2};
    expect(optimized_explain.selected_candidate.has_value() &&
            optimized_explain.selected_candidate->kind == RushmorePlanKind::index_seek &&
            optimized_explain.selected_candidate->selected &&
            optimized_explain.cursor.statistics.has_value(),
        "Rushmore explain plans must expose the selected kind, candidate, cost, and statistics metadata");
    expect(optimized_explain.indexable_predicates.size() == 1U &&
            optimized_explain.residual_predicates.empty() &&
            optimized_explain.options_version == 2U,
        "Rushmore explain plans must preserve structured predicates and options version without localized keys");

    RushmoreExplainPlan ambiguous_explain{
        RushmoreCursorMetadata{"people", {}, 100, 0, std::nullopt},
        "NAME LIKE '%BR%'",
        {},
        {RushmoreResidualPredicateDescriptor{"NAME LIKE '%BR%'", 1}},
        {},
        std::nullopt,
        RushmoreExplainFallbackReason::ambiguous_expression,
        2};
    expect(!ambiguous_explain.selected_candidate.has_value() &&
            ambiguous_explain.cursor.statistics.has_value() == false &&
            std::string(copperfin::runtime::rushmore_explain_fallback_reason_name(
                ambiguous_explain.fallback_reason)) == "ambiguous_expression",
        "Rushmore explain fallback output must preserve ambiguous expressions and missing statistics structurally");
    expect(std::string(copperfin::runtime::rushmore_explain_fallback_reason_catalog_key(
                RushmoreExplainFallbackReason::cost_rejected)) ==
            "Runtime.IndexSeek.Explain.Fallback.CostRejected",
        "Rushmore explain fallback reasons must map to stable localized catalog keys");

    const RushmorePredicateDescriptor equality_predicate{
        "NAME = 'BRAVO'", "NAME", "=", 1, true};
    const RushmorePredicateDescriptor range_predicate{
        "AGE > 18", "AGE", ">", 1, false};
    RushmoreRemotePlanningInput unknown_remote{
        true,
        "provider:northwind",
        {"sqlcust", "NAME", 100, 0, std::nullopt},
        {},
        {equality_predicate},
        {RushmoreResidualPredicateDescriptor{"DELETED()", 1}}};
    const auto unknown_decision = copperfin::runtime::rushmore_plan_remote_predicates(unknown_remote);
    expect(!unknown_decision.provider_pushdown_allowed &&
            unknown_decision.pushdown_predicates.empty() &&
            unknown_decision.local_residual_predicates.size() == 2U &&
            unknown_decision.fallback_reason == RushmoreRemoteFallbackReason::unknown_capabilities &&
            unknown_decision.round_trip_risk == RushmoreRemoteRoundTripRisk::unknown,
        "Unknown remote provider capabilities must preserve all predicates for local evaluation");

    RushmoreRemotePlanningInput limited_remote = unknown_remote;
    limited_remote.capabilities = {
        RushmoreRemoteCapabilityState::limited,
        true,
        true,
        false,
        false,
        false,
        false,
        1};
    limited_remote.predicates = {equality_predicate, range_predicate};
    const auto limited_decision = copperfin::runtime::rushmore_plan_remote_predicates(limited_remote);
    expect(limited_decision.provider_pushdown_allowed &&
            limited_decision.pushdown_predicates.size() == 1U &&
            limited_decision.pushdown_predicates.front().field_name == "NAME" &&
            limited_decision.local_residual_predicates.size() == 2U &&
            limited_decision.fallback_reason == RushmoreRemoteFallbackReason::local_residual_required &&
            limited_decision.round_trip_risk == RushmoreRemoteRoundTripRisk::elevated,
        "Limited remote capabilities must push down only safe predicates and preserve residuals");

    RushmoreRemotePlanningInput local_input = limited_remote;
    local_input.remote_cursor = false;
    const auto local_decision = copperfin::runtime::rushmore_plan_remote_predicates(local_input);
    expect(!local_decision.provider_pushdown_allowed &&
            local_decision.pushdown_predicates.empty() &&
            local_decision.local_residual_predicates.size() == 3U &&
            local_decision.fallback_reason == RushmoreRemoteFallbackReason::not_remote_cursor &&
            local_decision.round_trip_risk == RushmoreRemoteRoundTripRisk::none,
        "Local cursors must not receive remote pushdown decisions");
    expect(std::string(copperfin::runtime::rushmore_remote_fallback_reason_name(
                RushmoreRemoteFallbackReason::unknown_capabilities)) == "unknown_capabilities" &&
            std::string(copperfin::runtime::rushmore_remote_fallback_reason_catalog_key(
                RushmoreRemoteFallbackReason::local_residual_required)) ==
                "Runtime.IndexSeek.RemotePlan.Reason.LocalResidualRequired" &&
            std::string(copperfin::runtime::rushmore_remote_round_trip_risk_name(
                RushmoreRemoteRoundTripRisk::elevated)) == "elevated",
        "Remote planning reason and round-trip risk identities must remain stable and localizable");

    const RushmoreBitmapOrderCandidate name_candidate{
        "NAME_TAG",
        "NAME:UPPER",
        {"NAME = 'BRAVO'", "NAME", "=", 1, true},
        {10, 4, 1, 5},
        true};
    const RushmoreBitmapOrderCandidate age_candidate{
        "AGE_TAG",
        "AGE",
        {"AGE > 18", "AGE", ">", 1, false},
        {20, 5, 2, 7},
        true};
    const RushmoreBitmapOrderCandidate city_candidate{
        "CITY_TAG",
        "CITY:UPPER",
        {"CITY = 'DETROIT'", "CITY", "=", 1, true},
        {5, 3, 1, 4},
        true};
    RushmoreBitmapPlanningInput bitmap_input{
        {"people", "name:upper(name)", 100, refreshed.version, refreshed},
        RushmoreBitmapCombination::conjunction,
        {name_candidate, age_candidate, city_candidate},
        {RushmoreResidualPredicateDescriptor{"DELETED()", 1}},
        5,
        {4, 1, 1}};
    const auto bitmap_decision = copperfin::runtime::rushmore_plan_bitmap_candidates(bitmap_input);
    expect(bitmap_decision.allowed &&
            bitmap_decision.candidates.size() == 3U &&
            bitmap_decision.selected_candidate.has_value() &&
            bitmap_decision.selected_candidate->components.size() == 2U &&
            bitmap_decision.selected_candidate->cost.estimated_rows == 1U &&
            bitmap_decision.selected_candidate->cost.memory_units <= 5U &&
            bitmap_decision.selected_candidate->residual_predicates.size() == 1U,
        "Bitmap planning must enumerate deterministic AND candidates with bounded cost and residual metadata");

    RushmoreBitmapPlanningInput disjunction_input = bitmap_input;
    disjunction_input.combination = RushmoreBitmapCombination::disjunction;
    disjunction_input.memory_budget_units = 105;
    const auto disjunction_decision = copperfin::runtime::rushmore_plan_bitmap_candidates(disjunction_input);
    expect(disjunction_decision.allowed &&
            disjunction_decision.selected_candidate.has_value() &&
            disjunction_decision.selected_candidate->combination == RushmoreBitmapCombination::disjunction &&
            disjunction_decision.selected_candidate->cost.estimated_rows == 15U,
        "Bitmap planning must support deterministic OR candidate scoring without execution changes");

    RushmoreBitmapPlanningInput unknown_memory = bitmap_input;
    unknown_memory.memory_budget_units.reset();
    const auto unknown_memory_decision = copperfin::runtime::rushmore_plan_bitmap_candidates(unknown_memory);
    expect(!unknown_memory_decision.allowed &&
            unknown_memory_decision.candidates.size() == 3U &&
            unknown_memory_decision.fallback_reason == RushmoreBitmapFallbackReason::memory_risk_unknown &&
            unknown_memory_decision.residual_predicates.size() == 4U,
        "Bitmap planning must preserve local evaluation when memory risk is unbounded");

    RushmoreBitmapPlanningInput incompatible_input = bitmap_input;
    incompatible_input.order_candidates.resize(2U);
    incompatible_input.order_candidates[1].predicate.field_name = "NAME";
    const auto incompatible_decision = copperfin::runtime::rushmore_plan_bitmap_candidates(incompatible_input);
    expect(!incompatible_decision.allowed &&
            incompatible_decision.candidates.empty() &&
            incompatible_decision.fallback_reason == RushmoreBitmapFallbackReason::incompatible_predicates,
        "AND bitmap planning must reject same-field combinations conservatively");

    RushmoreBitmapPlanningInput memory_limited = bitmap_input;
    memory_limited.memory_budget_units = 0;
    const auto memory_limited_decision = copperfin::runtime::rushmore_plan_bitmap_candidates(memory_limited);
    expect(!memory_limited_decision.allowed &&
            memory_limited_decision.fallback_reason == RushmoreBitmapFallbackReason::memory_limit,
        "Bitmap planning must reject candidates that exceed the explicit memory budget");

    RushmoreBitmapPlanningInput duplicate_order = bitmap_input;
    duplicate_order.order_candidates.resize(2U);
    duplicate_order.order_candidates[1].order_name = duplicate_order.order_candidates[0].order_name;
    const auto duplicate_order_decision = copperfin::runtime::rushmore_plan_bitmap_candidates(duplicate_order);
    expect(!duplicate_order_decision.allowed &&
            duplicate_order_decision.fallback_reason == RushmoreBitmapFallbackReason::duplicate_order,
        "Bitmap planning must reject duplicate order identities");

    RushmoreBitmapPlanningInput unsupported_predicate = bitmap_input;
    unsupported_predicate.order_candidates.resize(2U);
    unsupported_predicate.order_candidates[1].predicate.operation = "IN";
    const auto unsupported_decision = copperfin::runtime::rushmore_plan_bitmap_candidates(unsupported_predicate);
    expect(!unsupported_decision.allowed &&
            unsupported_decision.fallback_reason == RushmoreBitmapFallbackReason::unsupported_predicate,
        "Bitmap planning must preserve local evaluation for unsupported predicates");

    RushmoreBitmapPlanningInput insufficient = bitmap_input;
    insufficient.order_candidates.resize(1U);
    const auto insufficient_decision = copperfin::runtime::rushmore_plan_bitmap_candidates(insufficient);
    expect(!insufficient_decision.allowed &&
            insufficient_decision.fallback_reason == RushmoreBitmapFallbackReason::insufficient_candidates,
        "Bitmap planning must require at least two usable order candidates");
    expect(std::string(copperfin::runtime::rushmore_bitmap_fallback_reason_name(
                RushmoreBitmapFallbackReason::memory_limit)) == "memory_limit" &&
            std::string(copperfin::runtime::rushmore_bitmap_fallback_reason_catalog_key(
                RushmoreBitmapFallbackReason::incompatible_predicates)) ==
                "Runtime.IndexSeek.Bitmap.Fallback.IncompatiblePredicates",
        "Bitmap fallback identities must remain stable and localizable");
}
