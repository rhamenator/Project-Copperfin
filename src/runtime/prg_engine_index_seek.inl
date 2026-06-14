// prg_engine_index_seek.inl
// Index Seek Optimizer implementation as free functions.
// Included in prg_engine.cpp anonymous namespace; does not compile separately.

// ============================================================================
// Index Expression Analyzer - Free Functions
// ============================================================================

IndexExpressionPattern analyze_filter_expression(
    const std::string &expression_text,
    const std::vector<std::string> &available_fields)
{
    IndexExpressionAnalyzer analyzer;
    return analyzer.analyze_expression(expression_text, available_fields);
}

// ============================================================================
// Index Seek Matcher - Free Functions
// ============================================================================

int score_order_for_pattern(
    const IndexExpressionPattern &pattern,
    const IndexOrderCandidate &order_candidate)
{
    int score = 0;
    
    if (pattern.field && !pattern.field->field_name.empty())
    {
        const auto field_norm = collapse_identifier(pattern.field->field_name);
        const auto order_norm = collapse_identifier(order_candidate.order_expression);
        
        if (order_norm == field_norm)
        {
            score += 95;  // Perfect match
        }
        else if (order_candidate.order_expression.find(field_norm) != std::string::npos)
        {
            score += 60;  // Partial match (e.g., UPPER(field))
        }
    }
    
    return score;
}

IndexSeekPlan create_index_seek_plan(
    const IndexExpressionPattern &pattern,
    const std::vector<IndexOrderCandidate> &available_orders,
    const std::string &active_order_name)
{
    IndexSeekMatcher matcher;
    return matcher.create_plan(pattern, available_orders, active_order_name);
}
