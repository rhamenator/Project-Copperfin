// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/runtime/index_seek_optimizer.h"
#include "copperfin/platform/invariant_numeric.h"
#include "localized_text.h"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <limits>
#include <optional>
#include <string_view>
#include <tuple>
#include <utility>

namespace copperfin::runtime {

namespace {

std::string index_seek_text(
    std::string_view key,
    const localization::PlaceholderMap& placeholders = {}) {
    return runtime_text(key, placeholders);
}

std::string trim_copy(std::string text) {
    const auto first = std::find_if(text.begin(), text.end(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    });
    const auto last = std::find_if(text.rbegin(), text.rend(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    }).base();
    if (first >= last) {
        return {};
    }
    return std::string(first, last);
}

std::string uppercase_copy(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return text;
}

bool is_identifier_char(unsigned char ch) {
    return std::isalnum(ch) != 0 || ch == '_';
}

std::string collapse_identifier(const std::string& value) {
    std::string normalized;
    normalized.reserve(value.size());
    for (unsigned char ch : value) {
        if (!is_identifier_char(ch)) {
            continue;
        }
        normalized.push_back(static_cast<char>(std::toupper(ch)));
    }
    return normalized;
}

bool starts_with_insensitive(const std::string& text, const std::string& prefix) {
    if (text.size() < prefix.size()) {
        return false;
    }
    for (std::size_t index = 0; index < prefix.size(); ++index) {
        if (std::toupper(static_cast<unsigned char>(text[index])) != std::toupper(static_cast<unsigned char>(prefix[index]))) {
            return false;
        }
    }
    return true;
}

bool is_quoted_literal(const std::string& text) {
    return text.size() >= 2U &&
           ((text.front() == '\'' && text.back() == '\'') ||
            (text.front() == '"' && text.back() == '"'));
}

bool is_numeric_literal(const std::string& text) {
    const std::string trimmed = trim_copy(text);
    if (trimmed.empty()) {
        return false;
    }
    return copperfin::platform::try_parse_invariant_double(trimmed).has_value();
}

bool is_literal_text(const std::string& text) {
    const std::string trimmed = trim_copy(text);
    return is_quoted_literal(trimmed) || is_numeric_literal(trimmed);
}

bool is_keyword_boundary(const std::string& text, std::size_t start, std::size_t length) {
    const bool left_ok = start == 0U || !is_identifier_char(static_cast<unsigned char>(text[start - 1U]));
    const std::size_t end = start + length;
    const bool right_ok = end >= text.size() || !is_identifier_char(static_cast<unsigned char>(text[end]));
    return left_ok && right_ok;
}

std::optional<std::size_t> find_top_level_keyword(const std::string& text, const std::string& keyword) {
    int parentheses_depth = 0;
    char quote_delimiter = '\0';
    for (std::size_t index = 0; index < text.size(); ++index) {
        const char ch = text[index];
        if (quote_delimiter != '\0') {
            if (ch == quote_delimiter) {
                quote_delimiter = '\0';
            }
            continue;
        }

        if (ch == '\'' || ch == '"') {
            quote_delimiter = ch;
            continue;
        }
        if (ch == '(') {
            ++parentheses_depth;
            continue;
        }
        if (ch == ')' && parentheses_depth > 0) {
            --parentheses_depth;
            continue;
        }
        if (parentheses_depth != 0) {
            continue;
        }
        if (starts_with_insensitive(text.substr(index), keyword) && is_keyword_boundary(text, index, keyword.size())) {
            return index;
        }
    }
    return std::nullopt;
}

std::vector<std::string> split_top_level_keyword(const std::string& text, const std::string& keyword) {
    std::vector<std::string> parts;
    int parentheses_depth = 0;
    char quote_delimiter = '\0';
    std::size_t part_start = 0U;
    bool matched = false;

    for (std::size_t index = 0; index < text.size(); ++index) {
        const char ch = text[index];
        if (quote_delimiter != '\0') {
            if (ch == quote_delimiter) {
                quote_delimiter = '\0';
            }
            continue;
        }

        if (ch == '\'' || ch == '"') {
            quote_delimiter = ch;
            continue;
        }
        if (ch == '(') {
            ++parentheses_depth;
            continue;
        }
        if (ch == ')' && parentheses_depth > 0) {
            --parentheses_depth;
            continue;
        }
        if (parentheses_depth != 0) {
            continue;
        }

        if (starts_with_insensitive(text.substr(index), keyword) && is_keyword_boundary(text, index, keyword.size())) {
            parts.push_back(trim_copy(text.substr(part_start, index - part_start)));
            index += keyword.size() - 1U;
            part_start = index + 1U;
            matched = true;
        }
    }

    if (!matched) {
        return {};
    }

    parts.push_back(trim_copy(text.substr(part_start)));
    return parts;
}

bool is_field_reference(const std::string& token, const std::vector<std::string>& available_fields) {
    const std::string trimmed = trim_copy(token);
    if (trimmed.empty() || is_quoted_literal(trimmed) || is_numeric_literal(trimmed)) {
        return false;
    }
    const std::string normalized = collapse_identifier(trimmed);
    if (normalized.empty()) {
        return false;
    }
    if (available_fields.empty()) {
        return true;
    }
    return std::any_of(available_fields.begin(), available_fields.end(), [&](const std::string& field) {
        return collapse_identifier(field) == normalized;
    });
}

std::optional<std::tuple<std::string, std::string, std::string>> extract_comparison(
    const std::string& text,
    const std::vector<std::string>& available_fields)
{
    static const std::vector<std::string> operators = {"<>", "<=", ">=", "LIKE", "IN", "==", "=", "<", ">"};

    int parentheses_depth = 0;
    char quote_delimiter = '\0';
    for (std::size_t index = 0; index < text.size(); ++index) {
        const char ch = text[index];
        if (quote_delimiter != '\0') {
            if (ch == quote_delimiter) {
                quote_delimiter = '\0';
            }
            continue;
        }

        if (ch == '\'' || ch == '"') {
            quote_delimiter = ch;
            continue;
        }
        if (ch == '(') {
            ++parentheses_depth;
            continue;
        }
        if (ch == ')' && parentheses_depth > 0) {
            --parentheses_depth;
            continue;
        }
        if (parentheses_depth != 0) {
            continue;
        }

        for (const auto& op : operators) {
            if (!starts_with_insensitive(text.substr(index), op)) {
                continue;
            }
            if (!is_keyword_boundary(text, index, op.size())) {
                continue;
            }

            const std::string left = trim_copy(text.substr(0, index));
            const std::string right = trim_copy(text.substr(index + op.size()));
            if (left.empty() || right.empty() || !is_field_reference(left, available_fields)) {
                continue;
            }
            return std::make_tuple(left, uppercase_copy(op), right);
        }
    }

    return std::nullopt;
}

OptimizationConfidence confidence_for_score(int score) {
    if (score >= 90) {
        return OptimizationConfidence::high;
    }
    if (score >= 60) {
        return OptimizationConfidence::medium;
    }
    if (score > 0) {
        return OptimizationConfidence::low;
    }
    return OptimizationConfidence::not_applicable;
}

OptimizationConfidence min_confidence(OptimizationConfidence left, OptimizationConfidence right) {
    return static_cast<int>(left) <= static_cast<int>(right) ? left : right;
}

const char* descriptor_operator_name(IndexOperatorKind kind) {
    switch (kind) {
    case IndexOperatorKind::equal:
        return "=";
    case IndexOperatorKind::not_equal:
        return "<>";
    case IndexOperatorKind::less_than:
        return "<";
    case IndexOperatorKind::less_than_or_equal:
        return "<=";
    case IndexOperatorKind::greater_than:
        return ">";
    case IndexOperatorKind::greater_than_or_equal:
        return ">=";
    case IndexOperatorKind::between:
        return "BETWEEN";
    case IndexOperatorKind::in_list:
        return "IN";
    case IndexOperatorKind::like:
        return "LIKE";
    case IndexOperatorKind::and_chain:
        return "AND";
    case IndexOperatorKind::or_chain:
        return "OR";
    case IndexOperatorKind::not_pattern:
        return "NOT";
    case IndexOperatorKind::unsupported:
        return "UNSUPPORTED";
    }
    return "UNSUPPORTED";
}

void append_predicate_descriptors(
    const IndexExpressionPattern& pattern,
    std::vector<RushmorePredicateDescriptor>& recognized,
    std::vector<RushmoreResidualPredicateDescriptor>& residual) {
    if (pattern.operator_kind == IndexOperatorKind::and_chain) {
        for (const auto& child : pattern.sub_patterns) {
            append_predicate_descriptors(child, recognized, residual);
        }
        return;
    }

    const bool has_direct_predicate =
        pattern.field.has_value() && pattern.operands.size() >= 2U &&
        pattern.operator_kind != IndexOperatorKind::unsupported &&
        pattern.operator_kind != IndexOperatorKind::not_pattern;
    if (has_direct_predicate) {
        RushmorePredicateDescriptor descriptor;
        descriptor.field_name = pattern.field->field_name;
        descriptor.operation = descriptor_operator_name(pattern.operator_kind);
        descriptor.complexity_units = static_cast<std::uint32_t>(pattern.operands.size());
        descriptor.exact_match = pattern.operator_kind == IndexOperatorKind::equal;
        descriptor.normalized_expression = descriptor.field_name + " " + descriptor.operation;
        for (std::size_t index = 1U; index < pattern.operands.size(); ++index) {
            descriptor.normalized_expression += " " + trim_copy(pattern.operands[index].raw_text);
        }
        recognized.push_back(std::move(descriptor));
        return;
    }

    const std::string expression = trim_copy(pattern.raw_expression);
    if (!expression.empty() && expression != ".T." && expression != ".F.") {
        residual.push_back(RushmoreResidualPredicateDescriptor{
            .normalized_expression = expression,
            .complexity_units = 1U});
    }
}

void populate_predicate_descriptors(IndexExpressionPattern& pattern) {
    pattern.recognized_predicates.clear();
    pattern.residual_predicates.clear();
    append_predicate_descriptors(pattern, pattern.recognized_predicates, pattern.residual_predicates);
}

}  // namespace

bool IndexExpressionAnalyzer::recognize_simple_comparison(
    const std::string& expr,
    const std::vector<std::string>& available_fields,
    IndexExpressionPattern& out_pattern)
{
    const auto comparison = extract_comparison(expr, available_fields);
    if (!comparison.has_value()) {
        return false;
    }

    const auto& [left, op, right] = *comparison;
    out_pattern.operator_kind = IndexOperatorKind::unsupported;
    if (op == "=" || op == "==") {
        out_pattern.operator_kind = IndexOperatorKind::equal;
    } else if (op == "<>") {
        out_pattern.operator_kind = IndexOperatorKind::not_equal;
    } else if (op == "<") {
        out_pattern.operator_kind = IndexOperatorKind::less_than;
    } else if (op == "<=") {
        out_pattern.operator_kind = IndexOperatorKind::less_than_or_equal;
    } else if (op == ">") {
        out_pattern.operator_kind = IndexOperatorKind::greater_than;
    } else if (op == ">=") {
        out_pattern.operator_kind = IndexOperatorKind::greater_than_or_equal;
    } else if (op == "LIKE") {
        out_pattern.operator_kind = IndexOperatorKind::like;
    } else if (op == "IN") {
        out_pattern.operator_kind = IndexOperatorKind::in_list;
    }

    out_pattern.field = IndexFieldReference{
        .field_name = collapse_identifier(left),
        .is_upper_cased = left == uppercase_copy(left),
        .is_macro_expanded = left.find('&') != std::string::npos
    };
    out_pattern.operands.push_back(IndexOperand{.raw_text = left, .evaluated_value = {}, .is_literal = false, .is_field_reference = true});
    out_pattern.operands.push_back(IndexOperand{.raw_text = right, .evaluated_value = {}, .is_literal = is_literal_text(right)});
    out_pattern.confidence = is_literal_text(right) ? OptimizationConfidence::high : OptimizationConfidence::low;
    out_pattern.reason = is_literal_text(right)
                             ? index_seek_text("Runtime.IndexSeek.PatternReason.SimpleFieldToLiteralComparison")
                             : index_seek_text("Runtime.IndexSeek.PatternReason.SimpleFieldRuntimeEvaluatedComparison");
    out_pattern.is_dnf_compatible = true;
    return true;
}

bool IndexExpressionAnalyzer::recognize_between_pattern(
    const std::string& expr,
    const std::vector<std::string>& available_fields,
    IndexExpressionPattern& out_pattern)
{
    const auto between_pos = find_top_level_keyword(expr, "BETWEEN");
    if (!between_pos.has_value()) {
        return false;
    }

    const std::string left = trim_copy(expr.substr(0, *between_pos));
    if (!is_field_reference(left, available_fields)) {
        return false;
    }

    const std::string right = trim_copy(expr.substr(*between_pos + 7U));
    const auto and_pos = find_top_level_keyword(right, "AND");
    if (!and_pos.has_value()) {
        return false;
    }

    const std::string lower = trim_copy(right.substr(0, *and_pos));
    const std::string upper = trim_copy(right.substr(*and_pos + 3U));
    if (lower.empty() || upper.empty()) {
        return false;
    }

    out_pattern.operator_kind = IndexOperatorKind::between;
    out_pattern.field = IndexFieldReference{
        .field_name = collapse_identifier(left),
        .is_upper_cased = left == uppercase_copy(left),
        .is_macro_expanded = left.find('&') != std::string::npos
    };
    out_pattern.operands.push_back(IndexOperand{.raw_text = left, .evaluated_value = {}, .is_literal = false, .is_field_reference = true});
    out_pattern.operands.push_back(IndexOperand{.raw_text = lower, .evaluated_value = {}, .is_literal = is_literal_text(lower)});
    out_pattern.operands.push_back(IndexOperand{.raw_text = upper, .evaluated_value = {}, .is_literal = is_literal_text(upper)});
    out_pattern.confidence = (is_literal_text(lower) && is_literal_text(upper))
                                 ? OptimizationConfidence::high
                                 : OptimizationConfidence::medium;
    out_pattern.reason = index_seek_text("Runtime.IndexSeek.PatternReason.FieldBetweenRangeComparison");
    out_pattern.is_dnf_compatible = true;
    return true;
}

bool IndexExpressionAnalyzer::recognize_and_chain(
    const std::string& expr,
    const std::vector<std::string>& available_fields,
    IndexExpressionPattern& out_pattern)
{
    const auto parts = split_top_level_keyword(expr, "AND");
    if (parts.size() < 2U) {
        return false;
    }

    std::vector<IndexExpressionPattern> sub_patterns;
    sub_patterns.reserve(parts.size());
    OptimizationConfidence combined_confidence = OptimizationConfidence::high;
    for (const auto& part : parts) {
        IndexExpressionPattern child = analyze_expression(part, available_fields);
        if (child.confidence == OptimizationConfidence::not_applicable) {
            return false;
        }
        combined_confidence = min_confidence(combined_confidence, child.confidence);
        sub_patterns.push_back(std::move(child));
    }

    out_pattern.operator_kind = IndexOperatorKind::and_chain;
    out_pattern.sub_patterns = std::move(sub_patterns);
    out_pattern.confidence = combined_confidence == OptimizationConfidence::high
                                 ? OptimizationConfidence::medium
                                 : combined_confidence;
    out_pattern.reason = index_seek_text("Runtime.IndexSeek.PatternReason.TopLevelAndChain");
    out_pattern.is_dnf_compatible = true;
    return true;
}

IndexExpressionPattern IndexExpressionAnalyzer::analyze_expression(
    const std::string& expression_text,
    const std::vector<std::string>& available_fields)
{
    IndexExpressionPattern result;
    result.raw_expression = expression_text;

    const auto finalize = [&]() {
        populate_predicate_descriptors(result);
        return result;
    };

    std::string trimmed = trim_copy(expression_text);
    if (trimmed.empty() || trimmed == ".T." || trimmed == ".F.") {
        result.confidence = OptimizationConfidence::not_applicable;
        result.reason = index_seek_text("Runtime.IndexSeek.PatternReason.EmptyOrBooleanConstant");
        return finalize();
    }

    while (trimmed.size() >= 2U && trimmed.front() == '(' && trimmed.back() == ')') {
        const std::string inner = trim_copy(trimmed.substr(1U, trimmed.size() - 2U));
        if (inner.size() + 2U != trimmed.size()) {
            break;
        }
        trimmed = inner;
    }

    if (starts_with_insensitive(trimmed, ".NOT.")) {
        const std::string inner_text = trim_copy(trimmed.substr(5U));
        if (!inner_text.empty()) {
            IndexExpressionPattern child = analyze_expression(inner_text, available_fields);
            if (child.confidence != OptimizationConfidence::not_applicable) {
                result.operator_kind = IndexOperatorKind::not_pattern;
                result.sub_patterns.push_back(std::move(child));
                result.confidence = result.sub_patterns.front().confidence;
                result.reason = index_seek_text("Runtime.IndexSeek.PatternReason.TopLevelNotPattern");
                result.is_dnf_compatible = false;
                return finalize();
            }
        }
    }

    if (recognize_between_pattern(trimmed, available_fields, result)) {
        return finalize();
    }

    if (recognize_and_chain(trimmed, available_fields, result)) {
        return finalize();
    }

    if (recognize_simple_comparison(trimmed, available_fields, result)) {
        return finalize();
    }

    result.operator_kind = IndexOperatorKind::unsupported;
    result.confidence = OptimizationConfidence::not_applicable;
    result.reason = index_seek_text("Runtime.IndexSeek.PatternReason.UnrecognizedOptimizationPattern");
    return finalize();
}

int IndexSeekMatcher::score_order_match(
    const IndexExpressionPattern& pattern,
    const IndexOrderCandidate& order_candidate)
{
    const IndexExpressionPattern* primary_pattern = &pattern;
    if (!primary_pattern->field.has_value() && !primary_pattern->sub_patterns.empty()) {
        primary_pattern = &primary_pattern->sub_patterns.front();
    }
    if (!primary_pattern->field.has_value()) {
        return 0;
    }

    const std::string field_norm = collapse_identifier(primary_pattern->field->field_name);
    if (field_norm.empty()) {
        return 0;
    }

    const std::string order_expression_norm = collapse_identifier(order_candidate.order_expression);
    const std::string order_name_norm = collapse_identifier(order_candidate.order_name);
    const std::string order_for_norm = collapse_identifier(order_candidate.order_for_expression);

    int score = 0;
    if (order_expression_norm == field_norm) {
        score = 100;
    } else if (order_name_norm == field_norm) {
        score = 95;
    } else if (order_expression_norm.find(field_norm) != std::string::npos) {
        score = 80;
    } else if (order_for_norm.find(field_norm) != std::string::npos) {
        score = 60;
    }

    if (score > 0 && !order_candidate.is_descending) {
        score += 2;
    }

    if (score > 0 && order_candidate.optimization_confidence != OptimizationConfidence::not_applicable) {
        score += static_cast<int>(order_candidate.optimization_confidence);
    }

    return std::min(score, 100);
}

IndexSeekPlan IndexSeekMatcher::create_plan(
    const IndexExpressionPattern& pattern,
    const std::vector<IndexOrderCandidate>& available_orders,
    const std::string& active_order_name)
{
    IndexSeekPlan plan;
    plan.parsed_pattern = pattern;
    plan.candidate_orders = available_orders;

    if (pattern.confidence == OptimizationConfidence::not_applicable) {
        plan.can_optimize = false;
        plan.strategy = IndexSeekPlan::ExecutionStrategy::linear_scan;
        plan.decision_rationale = index_seek_text("Runtime.IndexSeek.PlanDecision.PatternNotRecognized");
        return plan;
    }

    std::vector<IndexOrderCandidate> ranked_candidates;
    ranked_candidates.reserve(available_orders.size());
    for (auto candidate : available_orders) {
        candidate.match_score = score_order_match(pattern, candidate);
        if (!active_order_name.empty() && candidate.order_name == active_order_name) {
            candidate.match_score = std::min(100, candidate.match_score + 5);
        }
        candidate.optimization_confidence = confidence_for_score(candidate.match_score);
        if (candidate.match_score >= 90) {
            candidate.match_reason = index_seek_text("Runtime.IndexSeek.MatchReason.HighConfidenceRushmoreMatch");
        } else if (candidate.match_score >= 60) {
            candidate.match_reason = index_seek_text("Runtime.IndexSeek.MatchReason.ModerateConfidenceRushmoreMatch");
        } else if (candidate.match_score > 0) {
            candidate.match_reason = index_seek_text("Runtime.IndexSeek.MatchReason.PossibleRushmoreMatch");
        } else {
            candidate.match_reason = index_seek_text("Runtime.IndexSeek.MatchReason.NoMatch");
        }
        ranked_candidates.push_back(std::move(candidate));
    }

    if (ranked_candidates.empty()) {
        plan.can_optimize = false;
        plan.strategy = IndexSeekPlan::ExecutionStrategy::linear_scan;
        plan.decision_rationale = index_seek_text("Runtime.IndexSeek.PlanDecision.NoMatchingIndexes");
        plan.candidate_orders.clear();
        return plan;
    }

    std::sort(ranked_candidates.begin(), ranked_candidates.end(), [](const IndexOrderCandidate& left, const IndexOrderCandidate& right) {
        if (left.match_score != right.match_score) {
            return left.match_score > right.match_score;
        }
        return left.order_name < right.order_name;
    });

    if (ranked_candidates.size() > 3U) {
        ranked_candidates.resize(3U);
    }

    plan.candidate_orders = ranked_candidates;
    if (ranked_candidates.front().match_score > 0) {
        plan.selected_order = ranked_candidates.front();
        plan.can_optimize = true;
        plan.strategy = IndexSeekPlan::ExecutionStrategy::index_seek;
        plan.decision_rationale = index_seek_text(
            "Runtime.IndexSeek.PlanDecision.PatternFieldMatchedOrder",
            {{"fieldName", pattern.field ? pattern.field->field_name : index_seek_text("Runtime.IndexSeek.Field.Compound")},
                {"orderName", plan.selected_order->order_name}});
    } else {
        plan.can_optimize = false;
        plan.strategy = IndexSeekPlan::ExecutionStrategy::linear_scan;
        plan.decision_rationale = index_seek_text("Runtime.IndexSeek.PlanDecision.NoMatchingIndexes");
    }
    return plan;
}

}  // namespace copperfin::runtime
