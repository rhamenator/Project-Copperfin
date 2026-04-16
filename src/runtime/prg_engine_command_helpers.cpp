#include "prg_engine_command_helpers.h"

#include "prg_engine_helpers.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <utility>

namespace copperfin::runtime {

std::vector<std::string> split_csv_like(const std::string& text) {
    std::vector<std::string> parts;
    std::string current;
    int nesting = 0;
    bool in_string = false;

    for (char ch : text) {
        if (ch == '\'') {
            in_string = !in_string;
            current.push_back(ch);
            continue;
        }
        if (!in_string) {
            if (ch == '(' || ch == '[') {
                ++nesting;
            } else if ((ch == ')' || ch == ']') && nesting > 0) {
                --nesting;
            } else if (ch == ',' && nesting == 0) {
                parts.push_back(trim_copy(current));
                current.clear();
                continue;
            }
        }
        current.push_back(ch);
    }

    if (!current.empty()) {
        parts.push_back(trim_copy(current));
    }
    return parts;
}

std::size_t find_keyword_top_level(const std::string& text, const std::string& keyword) {
    const std::string upper = uppercase_copy(text);
    const std::string target = uppercase_copy(keyword);
    int nesting = 0;
    bool in_string = false;

    for (std::size_t index = 0; index < upper.size(); ++index) {
        const char ch = upper[index];
        if (ch == '\'') {
            in_string = !in_string;
            continue;
        }
        if (in_string) {
            continue;
        }
        if (ch == '(') {
            ++nesting;
            continue;
        }
        if (ch == ')' && nesting > 0) {
            --nesting;
            continue;
        }
        if (nesting != 0) {
            continue;
        }
        if ((index == 0U || std::isspace(static_cast<unsigned char>(upper[index - 1U])) != 0) &&
            upper.compare(index, target.size(), target) == 0) {
            const std::size_t tail = index + target.size();
            if (tail >= upper.size() || std::isspace(static_cast<unsigned char>(upper[tail])) != 0) {
                return index;
            }
        }
    }

    return std::string::npos;
}

std::string trim_command_keyword(const std::string& text, const std::string& keyword) {
    const std::size_t position = find_keyword_top_level(text, keyword);
    if (position == std::string::npos) {
        return {};
    }
    return trim_copy(text.substr(position + keyword.size()));
}

std::size_t find_keyword_top_level_from(
    const std::string& text,
    const std::string& keyword,
    std::size_t start_index) {
    const std::string upper = uppercase_copy(text);
    const std::string target = uppercase_copy(keyword);

    bool in_string = false;
    int nesting = 0;
    for (std::size_t index = start_index; index < upper.size(); ++index) {
        const char ch = upper[index];
        if (ch == '\'' && (index == 0U || upper[index - 1U] != '\\')) {
            in_string = !in_string;
            continue;
        }
        if (in_string) {
            continue;
        }
        if (ch == '(') {
            ++nesting;
            continue;
        }
        if (ch == ')' && nesting > 0) {
            --nesting;
            continue;
        }
        if (nesting != 0) {
            continue;
        }
        if ((index == 0U || std::isspace(static_cast<unsigned char>(upper[index - 1U])) != 0) &&
            upper.compare(index, target.size(), target) == 0) {
            const std::size_t tail = index + target.size();
            if (tail >= upper.size() || std::isspace(static_cast<unsigned char>(upper[tail])) != 0) {
                return index;
            }
        }
    }

    return std::string::npos;
}

std::size_t find_first_keyword_top_level(
    const std::string& text,
    std::initializer_list<std::string> keywords,
    std::size_t start_index) {
    std::size_t result = std::string::npos;
    for (const std::string& keyword : keywords) {
        const std::size_t position = find_keyword_top_level_from(text, keyword, start_index);
        if (position == std::string::npos) {
            continue;
        }
        result = result == std::string::npos ? position : std::min(result, position);
    }
    return result;
}

std::size_t find_last_keyword_top_level(
    const std::string& text,
    std::initializer_list<std::string> keywords) {
    std::size_t result = std::string::npos;
    for (const std::string& keyword : keywords) {
        std::size_t search_index = 0U;
        while (search_index < text.size()) {
            const std::size_t position = find_keyword_top_level_from(text, keyword, search_index);
            if (position == std::string::npos) {
                break;
            }
            result = result == std::string::npos ? position : std::max(result, position);
            search_index = position + 1U;
        }
    }
    return result;
}

std::string extract_command_clause(
    const std::string& text,
    const std::string& keyword,
    std::initializer_list<std::string> stop_keywords) {
    const std::size_t position = find_keyword_top_level(text, keyword);
    if (position == std::string::npos) {
        return {};
    }

    const std::size_t value_start = position + keyword.size();
    const std::size_t value_end = find_first_keyword_top_level(text, stop_keywords, value_start);
    if (value_end == std::string::npos) {
        return trim_copy(text.substr(value_start));
    }
    return trim_copy(text.substr(value_start, value_end - value_start));
}

std::vector<ReplaceAssignment> parse_replace_assignments(const std::string& text) {
    std::vector<ReplaceAssignment> assignments;
    for (const std::string& part : split_csv_like(text)) {
        const std::size_t with_position = find_keyword_top_level(part, "WITH");
        if (with_position == std::string::npos) {
            continue;
        }
        assignments.push_back({
            .field_name = trim_copy(part.substr(0U, with_position)),
            .expression = trim_copy(part.substr(with_position + 4U))
        });
    }
    return assignments;
}

std::vector<CalculateAssignment> parse_calculate_assignments(const std::string& text) {
    std::vector<CalculateAssignment> assignments;
    for (const std::string& part : split_csv_like(text)) {
        std::size_t to_position = find_keyword_top_level(part, "INTO");
        std::size_t keyword_length = 4U;
        if (to_position == std::string::npos) {
            to_position = find_keyword_top_level(part, "TO");
            keyword_length = 2U;
        }
        if (to_position == std::string::npos) {
            continue;
        }
        assignments.push_back({
            .aggregate_expression = trim_copy(part.substr(0U, to_position)),
            .variable_name = trim_copy(part.substr(to_position + keyword_length))
        });
    }
    return assignments;
}

AggregateScopeClause parse_aggregate_scope_clause(const std::string& text, std::string& expression_text) {
    AggregateScopeClause scope;
    expression_text = trim_copy(text);
    if (expression_text.empty()) {
        return scope;
    }

    const std::size_t keyword_position = find_last_keyword_top_level(expression_text, {"NEXT", "RECORD", "REST", "ALL"});
    if (keyword_position == std::string::npos) {
        return scope;
    }

    const std::string keyword_and_tail = trim_copy(expression_text.substr(keyword_position));
    const auto [keyword, tail] = split_first_word(keyword_and_tail);
    const std::string normalized_keyword = normalize_identifier(keyword);

    if ((normalized_keyword == "all" || normalized_keyword == "rest") && !tail.empty()) {
        return scope;
    }
    if ((normalized_keyword == "next" || normalized_keyword == "record") && tail.empty()) {
        return scope;
    }
    if (normalized_keyword != "all" &&
        normalized_keyword != "rest" &&
        normalized_keyword != "next" &&
        normalized_keyword != "record") {
        return scope;
    }

    expression_text = trim_copy(expression_text.substr(0U, keyword_position));
    if (normalized_keyword == "rest") {
        scope.kind = AggregateScopeKind::rest_records;
    } else if (normalized_keyword == "next") {
        scope.kind = AggregateScopeKind::next_records;
        scope.raw_value = tail;
    } else if (normalized_keyword == "record") {
        scope.kind = AggregateScopeKind::record;
        scope.raw_value = tail;
    }
    return scope;
}

std::string format_total_numeric_value(double value, std::uint8_t decimal_count) {
    std::ostringstream stream;
    if (decimal_count == 0U) {
        stream << static_cast<long long>(std::llround(value));
    } else {
        stream << std::fixed << std::setprecision(decimal_count) << value;
    }
    return stream.str();
}

std::optional<TotalCommandPlan> parse_total_command_plan(const std::string& body, std::string& error_message) {
    TotalCommandPlan plan;
    plan.target_expression = extract_command_clause(body, "TO", {"ON"});
    plan.on_field_name = extract_command_clause(body, "ON", {"FIELDS", "FOR", "WHILE", "IN", "NOOPTIMIZE", "ALL", "REST", "NEXT", "RECORD"});
    const std::string fields_text = extract_command_clause(body, "FIELDS", {"FOR", "WHILE", "IN", "NOOPTIMIZE", "ALL", "REST", "NEXT", "RECORD"});
    if (!fields_text.empty()) {
        plan.field_names = split_csv_like(fields_text);
        for (std::string& field_name : plan.field_names) {
            field_name = trim_copy(std::move(field_name));
        }
        plan.field_names.erase(
            std::remove_if(plan.field_names.begin(), plan.field_names.end(), [](const std::string& field_name) {
                return field_name.empty();
            }),
            plan.field_names.end());
    }

    plan.for_expression = extract_command_clause(body, "FOR", {"WHILE", "IN", "NOOPTIMIZE"});
    plan.while_expression = extract_command_clause(body, "WHILE", {"IN", "NOOPTIMIZE"});
    plan.in_expression = extract_command_clause(body, "IN", {"NOOPTIMIZE"});

    std::string scope_text = trim_copy(body);
    const std::size_t for_position = find_first_keyword_top_level(scope_text, {"FOR", "WHILE", "IN", "NOOPTIMIZE"});
    if (for_position != std::string::npos) {
        scope_text = trim_copy(scope_text.substr(0U, for_position));
    }
    std::string ignored_expression;
    plan.scope = parse_aggregate_scope_clause(scope_text, ignored_expression);

    if (plan.target_expression.empty()) {
        error_message = "TOTAL requires a TO target";
        return std::nullopt;
    }
    if (plan.on_field_name.empty()) {
        error_message = "TOTAL requires an ON field";
        return std::nullopt;
    }
    return plan;
}

}  // namespace copperfin::runtime
