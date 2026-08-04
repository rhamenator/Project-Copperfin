// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "copperfin/runtime/rushmore_planning.h"

#include <string>
#include <vector>
#include <optional>
#include <map>

namespace copperfin::runtime {

// Index Seek Optimizer
// Implements index-aware query optimization for WHERE/FOR/LOCATE clauses.
// Recognizes patterns in filter expressions that can be satisfied via index seeks instead of table scans.

enum class IndexOperatorKind {
    equal,                    // field = value
    not_equal,                // field <> value
    less_than,                // field < value
    less_than_or_equal,       // field <= value
    greater_than,             // field > value
    greater_than_or_equal,    // field >= value
    between,                  // field BETWEEN val1 AND val2
    in_list,                  // field IN (v1, v2, v3)
    like,                     // field LIKE 'pattern'
    and_chain,                // (pattern1 AND pattern2)
    or_chain,                 // (pattern1 OR pattern2) -- not typically optimizable
    not_pattern,              // .NOT. pattern
    unsupported               // function calls, complex expressions, macros
};

enum class OptimizationConfidence {
    not_applicable = 0,       // Pattern rejected; use linear scan
    low = 1,                  // Pattern recognized but may not be optimal
    medium = 2,               // Pattern recognized and likely beneficial
    high = 3                  // Pattern recognized and highly beneficial
};

struct IndexFieldReference {
    std::string field_name;
    bool is_upper_cased = false;
    bool is_macro_expanded = false;
};

struct IndexOperand {
    std::string raw_text;
    std::string evaluated_value;
    bool is_literal = false;
    bool is_field_reference = false;
    bool is_expression = false;
};

struct IndexExpressionPattern {
    IndexOperatorKind operator_kind = IndexOperatorKind::unsupported;
    OptimizationConfidence confidence = OptimizationConfidence::not_applicable;
    
    // Single field (for most operators)
    std::optional<IndexFieldReference> field;
    
    // Operands
    std::vector<IndexOperand> operands;
    
    // Sub-patterns (for AND/OR chains)
    std::vector<IndexExpressionPattern> sub_patterns;
    
    // Metadata
    std::string raw_expression;
    std::string reason;                          // why pattern is this confidence level
    bool is_dnf_compatible = false;              // can be part of disjunctive normal form (useful for compound queries)
    std::vector<RushmorePredicateDescriptor> recognized_predicates;
    std::vector<RushmoreResidualPredicateDescriptor> residual_predicates;
};

struct IndexOrderCandidate {
    std::string order_name{};
    std::string order_expression{};
    std::string order_for_expression{};
    std::string order_path{};
    std::string normalization_hint{};
    std::string collation_hint{};
    std::string key_domain_hint{};
    bool is_descending = false;
    
    int match_score = 0;                         // 0-100, higher is better
    OptimizationConfidence optimization_confidence = OptimizationConfidence::not_applicable;
    std::string match_reason{};
};

struct IndexSeekPlan {
    bool can_optimize = false;
    IndexExpressionPattern parsed_pattern;
    std::vector<IndexOrderCandidate> candidate_orders;
    std::optional<IndexOrderCandidate> selected_order;
    
    // Execution strategy
    enum class ExecutionStrategy {
        linear_scan,           // Full table scan with filter evaluation (fallback)
        index_seek,            // Use SEEK on selected order, then filter remaining
        index_range_seek,      // Multi-key SEEK for compound conditions (future)
        filtered_index_scan    // Scan filtered index FOR expression (future)
    } strategy = ExecutionStrategy::linear_scan;
    
    std::string decision_rationale;
};

// Forward declarations (implementations in .inl files)
class IndexExpressionAnalyzer {
public:
    IndexExpressionPattern analyze_expression(
        const std::string &expression_text,
        const std::vector<std::string> &available_fields);
    
private:
    bool recognize_simple_comparison(
        const std::string &expr,
        const std::vector<std::string> &available_fields,
        IndexExpressionPattern &out_pattern);
    
    bool recognize_between_pattern(
        const std::string &expr,
        const std::vector<std::string> &available_fields,
        IndexExpressionPattern &out_pattern);
    
    bool recognize_and_chain(
        const std::string &expr,
        const std::vector<std::string> &available_fields,
        IndexExpressionPattern &out_pattern);
};

class IndexSeekMatcher {
public:
    IndexSeekPlan create_plan(
        const IndexExpressionPattern &pattern,
        const std::vector<IndexOrderCandidate> &available_orders,
        const std::string &active_order_name);
    
private:
    int score_order_match(
        const IndexExpressionPattern &pattern,
        const IndexOrderCandidate &order_candidate);
};

} // namespace copperfin::runtime
