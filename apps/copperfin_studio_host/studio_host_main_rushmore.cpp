// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "studio_host_main_support.h"

#include <charconv>
#include <limits>

namespace cf_studio_host_main_detail {
namespace {

using copperfin::runtime::RushmoreExplainFallbackReason;
using copperfin::runtime::RushmoreExplainRecord;
using copperfin::runtime::RushmorePlanKind;
using copperfin::runtime::RushmorePredicateDescriptor;
using copperfin::runtime::RushmoreStatisticsState;

std::string rushmore_parse_error(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key,
    std::string_view option) {
    return catalog.translate(key, {{"option", std::string(option)}});
}

bool parse_uint64(std::string_view text, std::uint64_t& value) {
    if (text.empty()) {
        return false;
    }
    const auto* first = text.data();
    const auto* last = first + text.size();
    const auto result = std::from_chars(first, last, value);
    return result.ec == std::errc{} && result.ptr == last;
}

bool parse_bool(std::string_view text, bool& value) {
    if (text == "true" || text == "1") {
        value = true;
        return true;
    }
    if (text == "false" || text == "0") {
        value = false;
        return true;
    }
    return false;
}

std::string trim_copy(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1U);
}

bool parse_plan_kind(std::string_view value, RushmorePlanKind& kind) {
    if (value == "table_scan") {
        kind = RushmorePlanKind::table_scan;
        return true;
    }
    if (value == "index_seek") {
        kind = RushmorePlanKind::index_seek;
        return true;
    }
    if (value == "index_range_scan") {
        kind = RushmorePlanKind::index_range_scan;
        return true;
    }
    return false;
}

bool parse_statistics_state(std::string_view value, RushmoreStatisticsState& state) {
    if (value == "absent") {
        state = RushmoreStatisticsState::absent;
        return true;
    }
    if (value == "fresh") {
        state = RushmoreStatisticsState::fresh;
        return true;
    }
    if (value == "stale") {
        state = RushmoreStatisticsState::stale;
        return true;
    }
    if (value == "corrupted") {
        state = RushmoreStatisticsState::corrupted;
        return true;
    }
    return false;
}

bool parse_fallback_reason(std::string_view value, RushmoreExplainFallbackReason& reason) {
    for (const auto candidate : {
             RushmoreExplainFallbackReason::none,
             RushmoreExplainFallbackReason::planning_disabled,
             RushmoreExplainFallbackReason::unsupported_expression,
             RushmoreExplainFallbackReason::ambiguous_expression,
             RushmoreExplainFallbackReason::no_matching_index,
             RushmoreExplainFallbackReason::metadata_insufficient,
             RushmoreExplainFallbackReason::cost_rejected,
             RushmoreExplainFallbackReason::execution_fallback}) {
        if (value == copperfin::runtime::rushmore_explain_fallback_reason_name(candidate)) {
            reason = candidate;
            return true;
        }
    }
    return false;
}

RushmorePredicateDescriptor parse_predicate(std::string expression) {
    expression = trim_copy(std::move(expression));
    RushmorePredicateDescriptor result{
        expression,
        {},
        {},
        1,
        false};
    for (const std::string_view operation : {"LIKE", ">=", "<=", "==", "=", ">", "<"}) {
        const auto position = expression.find(operation);
        if (position == std::string::npos) {
            continue;
        }
        result.field_name = trim_copy(expression.substr(0, position));
        result.operation = std::string(operation);
        result.exact_match = operation == "=" || operation == "==";
        break;
    }
    return result;
}

template <typename Value>
bool parse_uint_option(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args,
    std::size_t& index,
    std::string_view option,
    Value& target,
    std::string& error) {
    if (index + 1U >= args.size()) {
        error = rushmore_parse_error(catalog, "StudioHost.RushmoreExplain.Error.MissingValue", option);
        return false;
    }
    std::uint64_t value = 0;
    if (!parse_uint64(args[++index], value) || value > std::numeric_limits<Value>::max()) {
        error = rushmore_parse_error(catalog, "StudioHost.RushmoreExplain.Error.UnsignedInteger", option);
        return false;
    }
    target = static_cast<Value>(value);
    return true;
}

bool parse_string_option(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args,
    std::size_t& index,
    std::string_view option,
    std::string& target,
    std::string& error) {
    if (index + 1U >= args.size()) {
        error = rushmore_parse_error(catalog, "StudioHost.RushmoreExplain.Error.MissingValue", option);
        return false;
    }
    target = args[++index];
    return true;
}

void print_json_cost(const copperfin::runtime::RushmorePlanCost& cost) {
    std::cout << "{\"estimatedRows\": " << cost.estimated_rows
              << ", \"cpuUnits\": " << cost.cpu_units
              << ", \"memoryUnits\": " << cost.memory_units
              << ", \"totalUnits\": " << cost.total_units << "}";
}

void print_json_predicate(const RushmorePredicateDescriptor& predicate) {
    std::cout << "{\"normalizedExpression\": ";
    print_json_string(predicate.normalized_expression);
    std::cout << ", \"fieldName\": ";
    print_json_string(predicate.field_name);
    std::cout << ", \"operation\": ";
    print_json_string(predicate.operation);
    std::cout << ", \"complexityUnits\": " << predicate.complexity_units
              << ", \"exactMatch\": " << (predicate.exact_match ? "true" : "false") << "}";
}

void print_json_residual_predicate(
    const copperfin::runtime::RushmoreResidualPredicateDescriptor& predicate) {
    std::cout << "{\"normalizedExpression\": ";
    print_json_string(predicate.normalized_expression);
    std::cout << ", \"complexityUnits\": " << predicate.complexity_units << "}";
}

void print_json_statistics(
    const copperfin::localization::LocalizedCatalog& catalog,
    const copperfin::runtime::RushmoreCursorStatisticsDescriptor& statistics) {
    std::cout << "{\"state\": ";
    print_json_string(copperfin::runtime::rushmore_statistics_state_name(statistics.state));
    std::cout << ", \"stateDisplay\": ";
    print_json_string(catalog.translate(
        copperfin::runtime::rushmore_statistics_state_catalog_key(statistics.state)));
    std::cout << ", \"version\": " << statistics.version;
    std::cout << ", \"recordCount\": ";
    if (statistics.record_count.has_value()) {
        std::cout << statistics.record_count.value();
    } else {
        std::cout << "null";
    }
    std::cout << ", \"recordLength\": ";
    if (statistics.record_length.has_value()) {
        std::cout << statistics.record_length.value();
    } else {
        std::cout << "null";
    }
    std::cout << ", \"approximateDistinctCount\": ";
    if (statistics.approximate_distinct_count.has_value()) {
        std::cout << statistics.approximate_distinct_count.value();
    } else {
        std::cout << "null";
    }
    std::cout << ", \"densityPartsPerMillion\": ";
    if (statistics.density_parts_per_million.has_value()) {
        std::cout << statistics.density_parts_per_million.value();
    } else {
        std::cout << "null";
    }
    std::cout << "}";
}

void print_json_record(
    const copperfin::localization::LocalizedCatalog& catalog,
    const RushmoreExplainRecord& record) {
    std::cout << "{\"cursorIdentity\": ";
    print_json_string(record.cursor_identity);
    std::cout << ", \"kind\": ";
    print_json_string(copperfin::runtime::rushmore_plan_kind_name(record.kind));
    std::cout << ", \"kindDisplay\": ";
    print_json_string(catalog.translate(copperfin::runtime::rushmore_plan_kind_catalog_key(record.kind)));
    std::cout << ", \"indexName\": ";
    print_json_string(record.index_name);
    std::cout << ", \"cost\": ";
    print_json_cost(record.cost);
    std::cout << ", \"selected\": " << (record.selected ? "true" : "false") << "}";
}

}  // namespace

RushmoreExplainParseResult parse_rushmore_explain_arguments(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    RushmoreExplainParseResult result;
    if (std::find(args.begin(), args.end(), "--rushmore-explain") == args.end()) {
        return result;
    }
    result.requested = true;
    result.plan.cursor.cursor_identity = "studio";
    bool candidate_requested = false;
    bool selected = false;
    RushmoreExplainRecord candidate{};
    bool statistics_requested = false;
    RushmoreStatisticsState statistics_state = RushmoreStatisticsState::absent;
    std::uint64_t statistics_version = 0;
    std::optional<std::uint64_t> statistics_record_count;
    std::optional<std::uint32_t> statistics_record_length;
    std::optional<std::uint64_t> statistics_distinct_count;
    std::optional<std::uint32_t> statistics_density;
    std::optional<std::uint64_t> total_units;

    for (std::size_t index = 1U; index < args.size(); ++index) {
        const std::string& argument = args[index];
        if (argument == "--rushmore-explain") {
            continue;
        }
        if (argument == "--json") {
            result.output_json = true;
            continue;
        }
        if (argument == "--rushmore-cursor") {
            if (!parse_string_option(catalog, args, index, argument, result.plan.cursor.cursor_identity, result.error)) {
                result.ok = false;
                return result;
            }
            continue;
        }
        if (argument == "--rushmore-index-signature") {
            if (!parse_string_option(catalog, args, index, argument, result.plan.cursor.index_signature, result.error)) {
                result.ok = false;
                return result;
            }
            continue;
        }
        if (argument == "--rushmore-row-count") {
            if (!parse_uint_option(catalog, args, index, argument, result.plan.cursor.row_count, result.error)) {
                result.ok = false;
                return result;
            }
            continue;
        }
        if (argument == "--rushmore-expression") {
            if (!parse_string_option(catalog, args, index, argument, result.plan.normalized_expression, result.error)) {
                result.ok = false;
                return result;
            }
            continue;
        }
        if (argument == "--rushmore-plan-kind") {
            std::string value;
            if (!parse_string_option(catalog, args, index, argument, value, result.error) ||
                !parse_plan_kind(value, candidate.kind)) {
                result.ok = false;
                result.error = result.error.empty()
                    ? catalog.translate("StudioHost.RushmoreExplain.Error.InvalidPlanKind")
                    : result.error;
                return result;
            }
            candidate_requested = true;
            continue;
        }
        if (argument == "--rushmore-index-name") {
            std::string value;
            if (!parse_string_option(catalog, args, index, argument, value, result.error)) {
                result.ok = false;
                return result;
            }
            candidate.index_name = value;
            candidate_requested = true;
            continue;
        }
        if (argument == "--rushmore-estimated-rows" || argument == "--rushmore-cpu-units" ||
            argument == "--rushmore-memory-units" || argument == "--rushmore-total-units") {
            auto& cost = candidate.cost;
            bool parsed = false;
            if (argument == "--rushmore-estimated-rows") {
                parsed = parse_uint_option(catalog, args, index, argument, cost.estimated_rows, result.error);
            } else if (argument == "--rushmore-cpu-units") {
                parsed = parse_uint_option(catalog, args, index, argument, cost.cpu_units, result.error);
            } else if (argument == "--rushmore-memory-units") {
                parsed = parse_uint_option(catalog, args, index, argument, cost.memory_units, result.error);
            } else {
                parsed = parse_uint_option(catalog, args, index, argument, cost.total_units, result.error);
                total_units = cost.total_units;
            }
            if (!parsed) {
                result.ok = false;
                return result;
            }
            candidate_requested = true;
            continue;
        }
        if (argument == "--rushmore-selected") {
            std::string value;
            if (!parse_string_option(catalog, args, index, argument, value, result.error) ||
                !parse_bool(value, selected)) {
                result.ok = false;
                result.error = result.error.empty()
                    ? catalog.translate("StudioHost.RushmoreExplain.Error.BooleanValue")
                    : result.error;
                return result;
            }
            candidate_requested = true;
            continue;
        }
        if (argument == "--rushmore-fallback-reason") {
            std::string value;
            if (!parse_string_option(catalog, args, index, argument, value, result.error) ||
                !parse_fallback_reason(value, result.plan.fallback_reason)) {
                result.ok = false;
                result.error = result.error.empty()
                    ? catalog.translate("StudioHost.RushmoreExplain.Error.InvalidFallbackReason")
                    : result.error;
                return result;
            }
            continue;
        }
        if (argument == "--rushmore-options-version") {
            if (!parse_uint_option(catalog, args, index, argument, result.plan.options_version, result.error)) {
                result.ok = false;
                return result;
            }
            continue;
        }
        if (argument == "--rushmore-indexable-predicate") {
            std::string value;
            if (!parse_string_option(catalog, args, index, argument, value, result.error)) {
                result.ok = false;
                return result;
            }
            result.plan.indexable_predicates.push_back(parse_predicate(std::move(value)));
            continue;
        }
        if (argument == "--rushmore-residual-predicate") {
            std::string value;
            if (!parse_string_option(catalog, args, index, argument, value, result.error)) {
                result.ok = false;
                return result;
            }
            result.plan.residual_predicates.push_back({trim_copy(std::move(value)), 1});
            continue;
        }
        if (argument == "--rushmore-stats-state") {
            std::string value;
            if (!parse_string_option(catalog, args, index, argument, value, result.error) ||
                !parse_statistics_state(value, statistics_state)) {
                result.ok = false;
                result.error = result.error.empty()
                    ? catalog.translate("StudioHost.RushmoreExplain.Error.InvalidStatisticsState")
                    : result.error;
                return result;
            }
            statistics_requested = true;
            continue;
        }
        if (argument == "--rushmore-stats-version") {
            if (!parse_uint_option(catalog, args, index, argument, statistics_version, result.error)) {
                result.ok = false;
                return result;
            }
            statistics_requested = true;
            continue;
        }
        if (argument == "--rushmore-stats-record-count") {
            std::uint64_t value = 0;
            if (!parse_uint_option(catalog, args, index, argument, value, result.error)) {
                result.ok = false;
                return result;
            }
            statistics_record_count = value;
            statistics_requested = true;
            continue;
        }
        if (argument == "--rushmore-stats-record-length") {
            std::uint32_t value = 0;
            if (!parse_uint_option(catalog, args, index, argument, value, result.error)) {
                result.ok = false;
                return result;
            }
            statistics_record_length = value;
            statistics_requested = true;
            continue;
        }
        if (argument == "--rushmore-stats-distinct-count") {
            std::uint64_t value = 0;
            if (!parse_uint_option(catalog, args, index, argument, value, result.error)) {
                result.ok = false;
                return result;
            }
            statistics_distinct_count = value;
            statistics_requested = true;
            continue;
        }
        if (argument == "--rushmore-stats-density-ppm") {
            std::uint32_t value = 0;
            if (!parse_uint_option(catalog, args, index, argument, value, result.error)) {
                result.ok = false;
                return result;
            }
            statistics_density = value;
            statistics_requested = true;
            continue;
        }
        result.ok = false;
        result.error = catalog.translate(
            "StudioHost.RushmoreExplain.Error.UnknownOption",
            {{"option", argument}});
        return result;
    }

    if (statistics_requested) {
        if (statistics_state != RushmoreStatisticsState::absent && statistics_version == 0U) {
            statistics_version = 1U;
        }
        result.plan.cursor.statistics = copperfin::runtime::RushmoreCursorStatisticsDescriptor{
            statistics_state,
            statistics_version,
            statistics_record_count,
            statistics_record_length,
            statistics_distinct_count,
            statistics_density};
        result.plan.cursor.stats_version = statistics_version;
    }
    if (candidate_requested) {
        candidate.cursor_identity = result.plan.cursor.cursor_identity;
        candidate.selected = selected;
        if (!total_units.has_value()) {
            candidate.cost.total_units = copperfin::runtime::rushmore_saturating_add(
                candidate.cost.cpu_units,
                candidate.cost.memory_units);
        }
        result.plan.candidates.push_back(candidate);
        if (selected) {
            result.plan.selected_candidate = candidate;
        }
    }
    return result;
}

void print_json_rushmore_explain_result(
    const copperfin::localization::LocalizedCatalog& catalog,
    const RushmoreExplainParseResult& result) {
    if (!result.ok) {
        std::cout << "{\"status\": \"error\", \"rushmoreExplain\": null, \"error\": ";
        print_json_string(result.error);
        std::cout << "}\n";
        return;
    }
    const auto& plan = result.plan;
    std::cout << "{\"status\": \"ok\", \"rushmoreExplain\": {\"cursor\": {\"cursorIdentity\": ";
    print_json_string(plan.cursor.cursor_identity);
    std::cout << ", \"indexSignature\": ";
    print_json_string(plan.cursor.index_signature);
    std::cout << ", \"rowCount\": " << plan.cursor.row_count
              << ", \"statsVersion\": " << plan.cursor.stats_version << ", \"statistics\": ";
    if (plan.cursor.statistics.has_value()) {
        print_json_statistics(catalog, *plan.cursor.statistics);
    } else {
        std::cout << "null";
    }
    std::cout << "}, \"normalizedExpression\": ";
    print_json_string(plan.normalized_expression);
    std::cout << ", \"indexablePredicates\": [";
    for (std::size_t index = 0U; index < plan.indexable_predicates.size(); ++index) {
        if (index != 0U) {
            std::cout << ", ";
        }
        print_json_predicate(plan.indexable_predicates[index]);
    }
    std::cout << "], \"residualPredicates\": [";
    for (std::size_t index = 0U; index < plan.residual_predicates.size(); ++index) {
        if (index != 0U) {
            std::cout << ", ";
        }
        print_json_residual_predicate(plan.residual_predicates[index]);
    }
    std::cout << "], \"candidates\": [";
    for (std::size_t index = 0U; index < plan.candidates.size(); ++index) {
        if (index != 0U) {
            std::cout << ", ";
        }
        print_json_record(catalog, plan.candidates[index]);
    }
    std::cout << "], \"selectedCandidate\": ";
    if (plan.selected_candidate.has_value()) {
        print_json_record(catalog, *plan.selected_candidate);
    } else {
        std::cout << "null";
    }
    std::cout << ", \"fallbackReason\": ";
    print_json_string(copperfin::runtime::rushmore_explain_fallback_reason_name(plan.fallback_reason));
    std::cout << ", \"fallbackReasonDisplay\": ";
    print_json_string(catalog.translate(
        copperfin::runtime::rushmore_explain_fallback_reason_catalog_key(plan.fallback_reason)));
    std::cout << ", \"optionsVersion\": " << plan.options_version << "}}\n";
}

void print_rushmore_explain_result(
    const copperfin::localization::LocalizedCatalog& catalog,
    const RushmoreExplainParseResult& result) {
    if (!result.ok) {
        std::cout << catalog.translate("StudioHost.RushmoreExplain.Status.Error") << "\n"
                  << studio_error_prefix() << result.error << "\n";
        return;
    }
    const auto& plan = result.plan;
    std::cout << catalog.translate("StudioHost.RushmoreExplain.Status.Ok") << "\n"
              << catalog.translate("StudioHost.RushmoreExplain.Title") << "\n";
    std::cout << catalog.translate("StudioHost.RushmoreExplain.Label.Cursor") << ": "
              << plan.cursor.cursor_identity << "\n";
    std::cout << catalog.translate("StudioHost.RushmoreExplain.Label.Expression") << ": "
              << plan.normalized_expression << "\n";
    if (plan.selected_candidate.has_value()) {
        std::cout << catalog.translate("StudioHost.RushmoreExplain.Label.Selected") << ": "
                  << catalog.translate(copperfin::runtime::rushmore_plan_kind_catalog_key(
                         plan.selected_candidate->kind))
                  << " / " << plan.selected_candidate->index_name << "\n";
    }
    std::cout << catalog.translate("StudioHost.RushmoreExplain.Label.Fallback") << ": "
              << catalog.translate(copperfin::runtime::rushmore_explain_fallback_reason_catalog_key(
                     plan.fallback_reason))
              << "\n";
}

std::optional<int> try_handle_rushmore_explain(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args) {
    const auto result = parse_rushmore_explain_arguments(catalog, args);
    if (!result.requested) {
        return std::nullopt;
    }
    if (result.output_json) {
        print_json_rushmore_explain_result(catalog, result);
    } else {
        print_rushmore_explain_result(catalog, result);
    }
    return result.ok ? 0 : 2;
}

}  // namespace cf_studio_host_main_detail
