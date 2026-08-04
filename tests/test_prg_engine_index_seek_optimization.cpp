// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/localization/localization.h"
#include "copperfin/runtime/index_seek_optimizer.h"
#include "test_environment_support.h"
#include "prg_engine_test_support.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#if defined(_WIN32)
#include <cstdlib>
#else
#include <cstdlib>
#endif

namespace {

using namespace copperfin::test_support;

using copperfin::test_support::ScopedEnvironmentValue;
using copperfin::test_support::set_env_value;

std::size_t count_missing_locale_keys(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view locale,
    const std::vector<std::string_view>& keys) {
    const auto locale_entries = catalog.catalogs.find(std::string(locale));
    if (locale_entries == catalog.catalogs.end()) {
        return keys.size();
    }

    std::size_t missing = 0U;
    for (const auto key : keys) {
        if (locale_entries->second.find(std::string(key)) == locale_entries->second.end()) {
            ++missing;
        }
    }
    return missing;
}

void test_index_operator_kind_values() {
    expect(static_cast<int>(copperfin::runtime::IndexOperatorKind::equal) == 0,
        "IndexOperatorKind::equal should remain stable for serialization and diagnostics");
    expect(static_cast<int>(copperfin::runtime::IndexOperatorKind::unsupported) == 12,
        "IndexOperatorKind::unsupported should remain stable for fallback planning");
}

void test_optimization_confidence_values() {
    expect(static_cast<int>(copperfin::runtime::OptimizationConfidence::not_applicable) == 0,
        "OptimizationConfidence::not_applicable should map to the lowest confidence");
    expect(static_cast<int>(copperfin::runtime::OptimizationConfidence::high) == 3,
        "OptimizationConfidence::high should map to the highest confidence");
}

void test_execution_strategy_values() {
    expect(static_cast<int>(copperfin::runtime::IndexSeekPlan::ExecutionStrategy::linear_scan) == 0,
        "ExecutionStrategy::linear_scan should remain stable for diagnostics");
    expect(static_cast<int>(copperfin::runtime::IndexSeekPlan::ExecutionStrategy::index_seek) == 1,
        "ExecutionStrategy::index_seek should remain stable for diagnostics");
    expect(static_cast<int>(copperfin::runtime::IndexSeekPlan::ExecutionStrategy::index_range_seek) == 2,
        "ExecutionStrategy::index_range_seek should remain stable for future planning");
    expect(static_cast<int>(copperfin::runtime::IndexSeekPlan::ExecutionStrategy::filtered_index_scan) == 3,
        "ExecutionStrategy::filtered_index_scan should remain stable for future planning");
}

void test_expression_pattern_struct_defaults() {
    copperfin::runtime::IndexExpressionPattern pattern{};
    expect(pattern.operator_kind == copperfin::runtime::IndexOperatorKind::unsupported,
        "IndexExpressionPattern should default to unsupported operator");
    expect(pattern.confidence == copperfin::runtime::OptimizationConfidence::not_applicable,
        "IndexExpressionPattern should default to not_applicable confidence");
    expect(!pattern.field.has_value(),
        "IndexExpressionPattern should default to no selected field");
    expect(pattern.operands.empty(),
        "IndexExpressionPattern should default to no operands");
}

void test_index_order_candidate_struct_defaults() {
    copperfin::runtime::IndexOrderCandidate candidate{};
    expect(candidate.order_name.empty(),
        "IndexOrderCandidate should default to no order name");
    expect(candidate.order_expression.empty(),
        "IndexOrderCandidate should default to no order expression");
    expect(!candidate.is_descending,
        "IndexOrderCandidate should default to ascending order");
    expect(candidate.match_score == 0,
        "IndexOrderCandidate should default to a zero match score");
    expect(candidate.optimization_confidence == copperfin::runtime::OptimizationConfidence::not_applicable,
        "IndexOrderCandidate should default to not_applicable confidence");
}

void test_index_seek_plan_struct_defaults() {
    copperfin::runtime::IndexSeekPlan plan{};
    expect(!plan.can_optimize,
        "IndexSeekPlan should default to non-optimized execution");
    expect(plan.strategy == copperfin::runtime::IndexSeekPlan::ExecutionStrategy::linear_scan,
        "IndexSeekPlan should default to linear scan strategy");
    expect(!plan.selected_order.has_value(),
        "IndexSeekPlan should default to no selected order");
}

void test_index_seek_plan_can_hold_selected_order() {
    copperfin::runtime::IndexOrderCandidate candidate{};
    candidate.order_name = "NAME";
    candidate.order_expression = "UPPER(name)";
    candidate.match_reason = "expression contains normalized field";

    copperfin::runtime::IndexSeekPlan plan{};
    plan.can_optimize = true;
    plan.strategy = copperfin::runtime::IndexSeekPlan::ExecutionStrategy::index_seek;
    plan.selected_order = candidate;

    expect(plan.can_optimize,
        "IndexSeekPlan should preserve explicit optimization enablement");
    expect(plan.selected_order.has_value(),
        "IndexSeekPlan should preserve selected order candidate");
    if (plan.selected_order.has_value()) {
        expect(plan.selected_order->order_name == "NAME",
            "Selected order candidate name should round-trip through IndexSeekPlan");
        expect(plan.selected_order->match_reason == "expression contains normalized field",
            "Selected order candidate match reason should round-trip through IndexSeekPlan");
    }
}

void test_index_expression_analyzer_recognizes_comparison_between_and_not_chain() {
    copperfin::runtime::IndexExpressionAnalyzer analyzer;
    const std::vector<std::string> fields = {"NAME", "AGE", "DOB"};

    const auto equality = analyzer.analyze_expression("NAME = 'BRAVO'", fields);
    expect(equality.operator_kind == copperfin::runtime::IndexOperatorKind::equal,
        "analyzer should recognize simple equality comparisons");
    expect(equality.confidence == copperfin::runtime::OptimizationConfidence::high,
        "analyzer should rate literal equality as high confidence");
    expect(equality.field.has_value() && equality.field->field_name == "NAME",
        "analyzer should normalize the comparison field");
    expect(equality.operands.size() == 2U,
        "analyzer should capture both operands for a simple comparison");
    expect(equality.reason == "Simple field-to-literal comparison",
        "analyzer should route equality rationale through the default locale catalog");
    expect(equality.recognized_predicates.size() == 1U && equality.residual_predicates.empty(),
        "analyzer should expose equality as one recognized predicate without residuals");
    if (equality.recognized_predicates.size() == 1U) {
        expect(equality.recognized_predicates.front().normalized_expression == "NAME = 'BRAVO'",
            "analyzer should normalize equality fields and preserve literal operands");
        expect(equality.recognized_predicates.front().exact_match,
            "analyzer should mark equality descriptors as exact matches");
    }

    const auto between = analyzer.analyze_expression("AGE BETWEEN 10 AND 20", fields);
    expect(between.operator_kind == copperfin::runtime::IndexOperatorKind::between,
        "analyzer should recognize BETWEEN comparisons");
    expect(between.confidence == copperfin::runtime::OptimizationConfidence::high,
        "analyzer should rate literal BETWEEN comparisons as high confidence");
    expect(between.field.has_value() && between.field->field_name == "AGE",
        "analyzer should normalize the BETWEEN field");
    expect(between.operands.size() == 3U,
        "analyzer should capture all BETWEEN operands");
    expect(between.reason == "Field BETWEEN lower AND upper range comparison",
        "analyzer should route BETWEEN rationale through the default locale catalog");
    expect(between.recognized_predicates.size() == 1U && between.residual_predicates.empty(),
        "analyzer should expose BETWEEN as one recognized predicate without residuals");

    const auto like = analyzer.analyze_expression("NAME LIKE 'BR*'", fields);
    expect(like.recognized_predicates.size() == 1U &&
            like.recognized_predicates.front().operation == "LIKE",
        "analyzer should expose safe LIKE candidates as normalized predicates");

    const auto in_list = analyzer.analyze_expression("NAME IN ('ALPHA','BRAVO')", fields);
    expect(in_list.recognized_predicates.size() == 1U &&
            in_list.recognized_predicates.front().operation == "IN",
        "analyzer should expose IN-list candidates as normalized predicates");

    const auto and_chain = analyzer.analyze_expression("NAME = 'BRAVO' AND AGE > 20", fields);
    expect(and_chain.operator_kind == copperfin::runtime::IndexOperatorKind::and_chain,
        "analyzer should recognize top-level AND chains");
    expect(and_chain.confidence == copperfin::runtime::OptimizationConfidence::medium,
        "analyzer should downgrade compound AND chains to medium confidence");
    expect(and_chain.sub_patterns.size() == 2U,
        "analyzer should preserve both AND-chain subpatterns");
    expect(and_chain.sub_patterns[0].operator_kind == copperfin::runtime::IndexOperatorKind::equal,
        "first AND-chain branch should remain an equality pattern");
    expect(and_chain.sub_patterns[1].operator_kind == copperfin::runtime::IndexOperatorKind::greater_than,
        "second AND-chain branch should remain a range pattern");
    expect(and_chain.reason == "Top-level AND chain with recognized sub-patterns",
        "analyzer should route AND-chain rationale through the default locale catalog");
    expect(and_chain.recognized_predicates.size() == 2U && and_chain.residual_predicates.empty(),
        "analyzer should flatten recognized AND branches into predicate descriptors");

    const auto not_pattern = analyzer.analyze_expression(".NOT. NAME = 'BRAVO'", fields);
    expect(not_pattern.operator_kind == copperfin::runtime::IndexOperatorKind::not_pattern,
        "analyzer should recognize NOT wrappers");
    expect(not_pattern.sub_patterns.size() == 1U,
        "analyzer should preserve the wrapped NOT subpattern");
    expect(not_pattern.sub_patterns[0].operator_kind == copperfin::runtime::IndexOperatorKind::equal,
        "NOT wrapper should preserve the inner comparison pattern");
    expect(not_pattern.reason == "Top-level NOT pattern",
        "analyzer should route NOT rationale through the default locale catalog");

    const auto unsupported = analyzer.analyze_expression("RECNO() > 10", fields);
    expect(unsupported.operator_kind == copperfin::runtime::IndexOperatorKind::unsupported,
        "analyzer should leave unsupported expressions unsupported");
    expect(unsupported.reason == "Expression does not match recognized optimization patterns",
        "analyzer should route unsupported-expression rationale through the default locale catalog");
    expect(unsupported.recognized_predicates.empty() && unsupported.residual_predicates.size() == 1U,
        "analyzer should preserve unsupported expressions as residual predicates");
    if (unsupported.residual_predicates.size() == 1U) {
        expect(unsupported.residual_predicates.front().normalized_expression == "RECNO() > 10",
            "analyzer should preserve the unsupported residual expression text");
    }
}

void test_index_seek_matcher_ranks_and_limits_candidate_orders() {
    copperfin::runtime::IndexExpressionAnalyzer analyzer;
    copperfin::runtime::IndexSeekMatcher matcher;

    const auto pattern = analyzer.analyze_expression("NAME = 'BRAVO'", {"NAME", "AGE"});

    copperfin::runtime::IndexOrderCandidate exact;
    exact.order_name = "NAME";
    exact.order_expression = "NAME";
    exact.optimization_confidence = copperfin::runtime::OptimizationConfidence::medium;

    copperfin::runtime::IndexOrderCandidate wrapped;
    wrapped.order_name = "NAME_UPPER";
    wrapped.order_expression = "UPPER(NAME)";

    copperfin::runtime::IndexOrderCandidate age;
    age.order_name = "AGE";
    age.order_expression = "AGE";

    copperfin::runtime::IndexOrderCandidate unmatched;
    unmatched.order_name = "DELETED";
    unmatched.order_expression = "DELETED";

    const auto plan = matcher.create_plan(pattern, {age, wrapped, unmatched, exact}, "NAME");
    expect(plan.can_optimize, "matcher should choose an optimized plan for a direct field comparison");
    expect(plan.strategy == copperfin::runtime::IndexSeekPlan::ExecutionStrategy::index_seek,
        "matcher should select index seek for a direct field comparison");
    expect(plan.selected_order.has_value(), "matcher should select a best candidate order");
    if (plan.selected_order.has_value()) {
        expect(plan.selected_order->order_name == "NAME",
            "matcher should prefer the exact field order");
        expect(plan.selected_order->match_score >= 90,
            "exact field order should score as a high-confidence match");
        expect(plan.selected_order->match_reason == "high-confidence Rushmore match",
            "matcher should route selected candidate match reason through the default locale catalog");
    }
    expect(plan.candidate_orders.size() == 3U,
        "matcher should return only the top three candidate orders");
    if (plan.candidate_orders.size() == 3U) {
        expect(plan.candidate_orders[0].match_score >= plan.candidate_orders[1].match_score,
            "candidate orders should be sorted by descending match score");
        expect(plan.candidate_orders[1].match_score >= plan.candidate_orders[2].match_score,
            "candidate orders should keep descending score order after truncation");
        expect(plan.candidate_orders[1].match_reason == "moderate-confidence Rushmore match",
            "matcher should localize moderate-confidence match reasons");
        expect(plan.candidate_orders[2].match_reason == "no match",
            "matcher should localize no-match reasons");
    }
    expect(plan.decision_rationale == "Pattern field 'NAME' matched order 'NAME'",
        "matcher should interpolate selected field and order names through the default locale catalog");

    const auto no_indexes = matcher.create_plan(pattern, {}, "NAME");
    expect(!no_indexes.can_optimize,
        "matcher should fall back when no orders are available");
    expect(no_indexes.decision_rationale == "No indexes available that match pattern fields",
        "matcher should route missing-index rationale through the default locale catalog");

    copperfin::runtime::IndexExpressionPattern unsupported_pattern{};
    unsupported_pattern.confidence = copperfin::runtime::OptimizationConfidence::not_applicable;
    const auto rejected = matcher.create_plan(unsupported_pattern, {exact}, "");
    expect(!rejected.can_optimize,
        "matcher should reject not-applicable patterns");
    expect(rejected.decision_rationale == "Pattern not recognized for optimization",
        "matcher should route rejected-pattern rationale through the default locale catalog");
}

void test_index_seek_localization_catalogs_and_runtime_locale_switching() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::vector<std::string_view> keys{
        "Runtime.IndexSeek.Field.Compound",
        "Runtime.IndexSeek.MatchReason.HighConfidenceRushmoreMatch",
        "Runtime.IndexSeek.MatchReason.ModerateConfidenceRushmoreMatch",
        "Runtime.IndexSeek.MatchReason.NoMatch",
        "Runtime.IndexSeek.MatchReason.PossibleRushmoreMatch",
        "Runtime.IndexSeek.PatternReason.EmptyOrBooleanConstant",
        "Runtime.IndexSeek.PatternReason.FieldBetweenRangeComparison",
        "Runtime.IndexSeek.PatternReason.SimpleFieldRuntimeEvaluatedComparison",
        "Runtime.IndexSeek.PatternReason.SimpleFieldToLiteralComparison",
        "Runtime.IndexSeek.PatternReason.TopLevelAndChain",
        "Runtime.IndexSeek.PatternReason.TopLevelNotPattern",
        "Runtime.IndexSeek.PatternReason.UnrecognizedOptimizationPattern",
        "Runtime.IndexSeek.PlanDecision.NoMatchingIndexes",
        "Runtime.IndexSeek.PlanDecision.CostModelRejected",
        "Runtime.IndexSeek.PlanDecision.SeekCostModelSelected",
        "Runtime.IndexSeek.PlanDecision.SeekCostModelRejected",
        "Runtime.IndexSeek.PlanDecision.SeekMetadataUnavailable",
        "Runtime.IndexSeek.Explain.Fallback.None",
        "Runtime.IndexSeek.Explain.Fallback.PlanningDisabled",
        "Runtime.IndexSeek.Explain.Fallback.UnsupportedExpression",
        "Runtime.IndexSeek.Explain.Fallback.AmbiguousExpression",
        "Runtime.IndexSeek.Explain.Fallback.NoMatchingIndex",
        "Runtime.IndexSeek.Explain.Fallback.MetadataInsufficient",
        "Runtime.IndexSeek.Explain.Fallback.CostRejected",
        "Runtime.IndexSeek.Explain.Fallback.ExecutionFallback",
        "Runtime.IndexSeek.Explain.PlanKind.TableScan",
        "Runtime.IndexSeek.Explain.PlanKind.IndexSeek",
        "Runtime.IndexSeek.Explain.PlanKind.IndexRangeScan",
        "Runtime.IndexSeek.Explain.Statistics.Absent",
        "Runtime.IndexSeek.Explain.Statistics.Fresh",
        "Runtime.IndexSeek.Explain.Statistics.Stale",
        "Runtime.IndexSeek.Explain.Statistics.Corrupted",
        "Runtime.IndexSeek.RemotePlan.Reason.None",
        "Runtime.IndexSeek.RemotePlan.Reason.NotRemoteCursor",
        "Runtime.IndexSeek.RemotePlan.Reason.UnknownCapabilities",
        "Runtime.IndexSeek.RemotePlan.Reason.UnsupportedCapability",
        "Runtime.IndexSeek.RemotePlan.Reason.LocalResidualRequired",
        "Runtime.IndexSeek.Bitmap.Fallback.None",
        "Runtime.IndexSeek.Bitmap.Fallback.InsufficientCandidates",
        "Runtime.IndexSeek.Bitmap.Fallback.UnsupportedPredicate",
        "Runtime.IndexSeek.Bitmap.Fallback.DuplicateOrder",
        "Runtime.IndexSeek.Bitmap.Fallback.IncompatiblePredicates",
        "Runtime.IndexSeek.Bitmap.Fallback.MemoryRiskUnknown",
        "Runtime.IndexSeek.Bitmap.Fallback.MemoryLimit",
        "Runtime.IndexSeek.PlanDecision.PatternFieldMatchedOrder",
        "Runtime.IndexSeek.PlanDecision.PatternNotRecognized"};

    expect(
        english_catalog.translate("Runtime.IndexSeek.PatternReason.SimpleFieldToLiteralComparison") ==
            "Simple field-to-literal comparison",
        "#2603: index-seek equality rationale should preserve the en-US catalog text");
    expect(
        spanish_catalog.translate("Runtime.IndexSeek.MatchReason.HighConfidenceRushmoreMatch") ==
            "coincidencia Rushmore de alta confianza",
        "#2603: index-seek match reasons should resolve through the es-419 catalog");
    expect(
        portuguese_catalog.translate(
            "Runtime.IndexSeek.PlanDecision.PatternFieldMatchedOrder",
            {{"fieldName", "NAME"}, {"orderName", "NAME"}}) ==
            "O campo de padrao 'NAME' correspondeu a ordem 'NAME'",
        "#2603: index-seek plan decisions should preserve placeholders through the pt-BR catalog");
    expect(
        pseudo_catalog.translate("Runtime.IndexSeek.PatternReason.TopLevelNotPattern") ==
            copperfin::localization::pseudo_localize("Top-level NOT pattern"),
        "#2603: index-seek qps-ploc strings should route through the pseudo-localization transform");

    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", keys) == 0U,
        "#2603: es-419 should define every remaining Runtime.IndexSeek localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", keys) == 0U,
        "#2603: pt-BR should define every remaining Runtime.IndexSeek localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", keys) == 0U,
        "#2603: qps-ploc should define every remaining Runtime.IndexSeek localization key");

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    copperfin::runtime::IndexExpressionAnalyzer analyzer;
    copperfin::runtime::IndexSeekMatcher matcher;
    copperfin::runtime::IndexOrderCandidate exact;
    exact.order_name = "NAME";
    exact.order_expression = "NAME";

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    const auto spanish_pattern = analyzer.analyze_expression("NAME = 'BRAVO'", {"NAME"});
    expect(
        spanish_pattern.reason == "Comparacion simple de campo con literal",
        "#2603: index-seek analyzer should emit es-419 rationale after locale selection");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    const auto portuguese_plan = matcher.create_plan(spanish_pattern, {exact}, "NAME");
    expect(
        portuguese_plan.selected_order.has_value() &&
            portuguese_plan.selected_order->match_reason ==
                "correspondencia Rushmore de alta confianca",
        "#2603: index-seek matcher should refresh localized match reasons when the runtime locale changes in-process");
    expect(
        portuguese_plan.decision_rationale ==
            "O campo de padrao 'NAME' correspondeu a ordem 'NAME'",
        "#2603: index-seek matcher should refresh localized plan decisions when the runtime locale changes in-process");
    expect(
        portuguese_plan.selected_order.has_value() &&
            portuguese_plan.selected_order->order_name == "NAME",
        "#2603: index-seek localization should preserve invariant order names");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    const auto pseudo_pattern = analyzer.analyze_expression(".NOT. NAME = 'BRAVO'", {"NAME"});
    expect(
        pseudo_pattern.reason == copperfin::localization::pseudo_localize("Top-level NOT pattern"),
        "#2603: index-seek analyzer should refresh qps-ploc rationale when the runtime locale changes in-process");
}

}  // namespace

int main() {
    test_index_operator_kind_values();
    test_optimization_confidence_values();
    test_execution_strategy_values();
    test_expression_pattern_struct_defaults();
    test_index_order_candidate_struct_defaults();
    test_index_seek_plan_struct_defaults();
    test_index_seek_plan_can_hold_selected_order();
    test_index_expression_analyzer_recognizes_comparison_between_and_not_chain();
    test_index_seek_matcher_ranks_and_limits_candidate_orders();
    test_index_seek_localization_catalogs_and_runtime_locale_switching();

    if (copperfin::test_support::test_failures() != 0) {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
