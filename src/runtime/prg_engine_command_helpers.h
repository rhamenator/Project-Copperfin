#pragma once

#include <cstdint>
#include <initializer_list>
#include <optional>
#include <string>
#include <vector>

namespace copperfin::runtime {

struct ReplaceAssignment {
    std::string field_name;
    std::string expression;
};

struct CalculateAssignment {
    std::string aggregate_expression;
    std::string variable_name;
};

enum class AggregateScopeKind {
    all_records,
    rest_records,
    next_records,
    record
};

struct AggregateScopeClause {
    AggregateScopeKind kind = AggregateScopeKind::all_records;
    std::string raw_value;
};

struct TotalCommandPlan {
    std::string target_expression;
    std::string on_field_name;
    std::vector<std::string> field_names;
    AggregateScopeClause scope;
    std::string for_expression;
    std::string while_expression;
    std::string in_expression;
};

std::vector<std::string> split_csv_like(const std::string& text);
std::size_t find_keyword_top_level(const std::string& text, const std::string& keyword);
std::string trim_command_keyword(const std::string& text, const std::string& keyword);
std::size_t find_keyword_top_level_from(
    const std::string& text,
    const std::string& keyword,
    std::size_t start_index);
std::size_t find_first_keyword_top_level(
    const std::string& text,
    std::initializer_list<std::string> keywords,
    std::size_t start_index = 0U);
std::size_t find_last_keyword_top_level(
    const std::string& text,
    std::initializer_list<std::string> keywords);
std::string extract_command_clause(
    const std::string& text,
    const std::string& keyword,
    std::initializer_list<std::string> stop_keywords = {});
std::vector<ReplaceAssignment> parse_replace_assignments(const std::string& text);
std::vector<CalculateAssignment> parse_calculate_assignments(const std::string& text);
AggregateScopeClause parse_aggregate_scope_clause(const std::string& text, std::string& expression_text);
std::string format_total_numeric_value(double value, std::uint8_t decimal_count);
std::optional<TotalCommandPlan> parse_total_command_plan(const std::string& body, std::string& error_message);

}  // namespace copperfin::runtime
