// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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
}
