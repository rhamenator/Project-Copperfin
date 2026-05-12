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
    IndexExpressionPattern result;
    result.raw_expression = expression_text;
    
    const auto trimmed = trim_copy(expression_text);
    if (trimmed.empty() || trimmed == ".T." || trimmed == ".F.")
    {
        result.confidence = OptimizationConfidence::not_applicable;
        result.reason = "Expression is empty or boolean constant";
        return result;
    }
    
    // Helper: extract binary operator
    auto extract_binary_op = [](const std::string &expr) -> std::tuple<std::string, std::string, std::string>
    {
        const auto trimmed_expr = trim_copy(expr);
        std::vector<std::string> operators = {"=", "<>", "<=", ">=", "<", ">", "LIKE", "IN"};
        
        int quote_depth = 0;
        for (size_t i = 0; i < trimmed_expr.size(); ++i)
        {
            const char c = trimmed_expr[i];
            if (c == '\'' || c == '"')
            {
                quote_depth = quote_depth > 0 ? quote_depth - 1 : quote_depth + 1;
                continue;
            }
            if (quote_depth > 0)
                continue;
            
            for (const auto &op : operators)
            {
                if (trimmed_expr.substr(i, op.size()) == op)
                {
                    return {trim_copy(trimmed_expr.substr(0, i)), op, trim_copy(trimmed_expr.substr(i + op.size()))};
                }
            }
        }
        return {"", "", ""};
    };
    
    // Helper: check if likely field
    auto is_field = [&](const std::string &text)
    {
        if (text.empty() || text.front() == '\'' || text.front() == '"')
            return false;
        const auto norm = collapse_identifier(text);
        for (const auto &f : available_fields)
            if (collapse_identifier(f) == norm)
                return true;
        return false;
    };
    
    // Helper: check if literal
    auto is_literal = [](const std::string &text)
    {
        const auto t = trim_copy(text);
        if ((t.size() >= 2U && t.front() == '\'' && t.back() == '\'') ||
            (t.size() >= 2U && t.front() == '"' && t.back() == '"'))
            return true;
        try { std::stod(t); return true; } catch (...) { return false; }
    };
    
    // Try simple comparison
    const auto [left_str, op_str, right_str] = extract_binary_op(trimmed);
    if (!left_str.empty() && !op_str.empty() && !right_str.empty() && is_field(left_str))
    {
        const auto upper_op = uppercase_copy(op_str);
        if (upper_op == "=")
            result.operator_kind = IndexOperatorKind::equal;
        else if (upper_op == "<>")
            result.operator_kind = IndexOperatorKind::not_equal;
        else if (upper_op == "<")
            result.operator_kind = IndexOperatorKind::less_than;
        else if (upper_op == "<=")
            result.operator_kind = IndexOperatorKind::less_than_or_equal;
        else if (upper_op == ">")
            result.operator_kind = IndexOperatorKind::greater_than;
        else if (upper_op == ">=")
            result.operator_kind = IndexOperatorKind::greater_than_or_equal;
        else if (upper_op == "LIKE")
            result.operator_kind = IndexOperatorKind::like;
        else if (upper_op == "IN")
            result.operator_kind = IndexOperatorKind::in_list;
        else
            return result;
        
        result.field = IndexFieldReference{
            .field_name = collapse_identifier(left_str),
            .is_upper_cased = left_str == uppercase_copy(left_str)
        };
        
        result.operands.push_back(IndexOperand{.raw_text = left_str, .is_field_reference = true});
        result.operands.push_back(IndexOperand{.raw_text = right_str, .is_literal = is_literal(right_str)});
        
        result.confidence = is_literal(right_str) ? OptimizationConfidence::high : OptimizationConfidence::low;
        result.reason = is_literal(right_str) ? "Simple field-to-literal comparison; excellent index match candidate"
                                               : "Right operand may need runtime evaluation";
        return result;
    }
    
    result.operator_kind = IndexOperatorKind::unsupported;
    result.confidence = OptimizationConfidence::not_applicable;
    result.reason = "Expression does not match recognized optimization patterns";
    return result;
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
    IndexSeekPlan plan;
    plan.parsed_pattern = pattern;
    plan.candidate_orders = available_orders;
    
    if (pattern.confidence == OptimizationConfidence::not_applicable)
    {
        plan.can_optimize = false;
        plan.strategy = IndexSeekPlan::ExecutionStrategy::linear_scan;
        plan.decision_rationale = "Pattern not recognized for optimization";
        return plan;
    }
    
    // Score all orders
    std::vector<std::pair<int, size_t>> scores;
    for (size_t i = 0; i < available_orders.size(); ++i)
    {
        const int score = score_order_for_pattern(pattern, available_orders[i]);
        if (score > 0)
            scores.push_back({score, i});
    }
    
    if (scores.empty())
    {
        plan.can_optimize = false;
        plan.strategy = IndexSeekPlan::ExecutionStrategy::linear_scan;
        plan.decision_rationale = "No indexes available that match pattern fields";
        return plan;
    }
    
    // Select top candidate
    std::sort(scores.begin(), scores.end(),
              [](const auto &a, const auto &b) { return a.first > b.first; });
    
    const auto &selected = available_orders[scores.front().second];
    plan.selected_order = selected;
    plan.can_optimize = true;
    plan.strategy = IndexSeekPlan::ExecutionStrategy::index_seek;
    plan.decision_rationale = 
        "Pattern field '" + (pattern.field ? pattern.field->field_name : "compound") + 
        "' matches order '" + selected.order_name + "'";
    
    return plan;
}
