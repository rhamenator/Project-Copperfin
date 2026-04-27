#include "prg_engine_type_functions.h"

#include "prg_engine_helpers.h"

#include <cctype>
#include <cmath>

namespace copperfin::runtime {

namespace {

bool has_wrapping_parentheses_local(const std::string& text) {
    const std::string trimmed = trim_copy(text);
    if (trimmed.size() < 2U || trimmed.front() != '(' || trimmed.back() != ')') {
        return false;
    }

    int nesting = 0;
    char quote_delimiter = '\0';
    for (std::size_t index = 0; index < trimmed.size(); ++index) {
        const char ch = trimmed[index];
        if ((ch == '\'' || ch == '"') && (quote_delimiter == '\0' || quote_delimiter == ch)) {
            quote_delimiter = quote_delimiter == '\0' ? ch : '\0';
            continue;
        }
        if (quote_delimiter != '\0') {
            continue;
        }

        if (ch == '(') {
            ++nesting;
            continue;
        }
        if (ch == ')') {
            --nesting;
            if (nesting == 0 && index + 1U < trimmed.size()) {
                return false;
            }
        }
    }

    return nesting == 0;
}

std::string normalize_type_expression(std::string expression_text) {
    expression_text = trim_copy(std::move(expression_text));
    while (has_wrapping_parentheses_local(expression_text)) {
        expression_text = trim_copy(expression_text.substr(1U, expression_text.size() - 2U));
    }
    return expression_text;
}

std::string vartype_code(const PrgValue& value) {
    if (value.kind == PrgValueKind::empty) {
        return "U";
    }
    if (value.kind == PrgValueKind::boolean) {
        return "L";
    }
    if (value.kind == PrgValueKind::number) {
        return "N";
    }
    if (value.kind == PrgValueKind::int64 || value.kind == PrgValueKind::uint64) {
        return "I";
    }
    return "C";
}

}  // namespace

std::optional<PrgValue> evaluate_type_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments,
    const std::function<bool(const std::string&)>& array_exists_callback,
    const std::function<PrgValue(const std::string&)>& eval_expression_callback) {
    if (function == "empty" && !arguments.empty()) {
        const PrgValue& value = arguments[0];
        if (value.kind == PrgValueKind::empty) {
            return make_boolean_value(true);
        }
        if (value.kind == PrgValueKind::string) {
            return make_boolean_value(trim_copy(value.string_value).empty());
        }
        if (value.kind == PrgValueKind::number) {
            return make_boolean_value(std::abs(value.number_value) < 0.000001);
        }
        if (value.kind == PrgValueKind::boolean) {
            return make_boolean_value(!value.boolean_value);
        }
        if (value.kind == PrgValueKind::int64) {
            return make_boolean_value(value.int64_value == 0);
        }
        if (value.kind == PrgValueKind::uint64) {
            return make_boolean_value(value.uint64_value == 0U);
        }
        return make_boolean_value(true);
    }
    if ((function == "isnull" || function == "isempty") && !arguments.empty()) {
        return make_boolean_value(arguments[0].kind == PrgValueKind::empty);
    }
    if (function == "isblank" && !arguments.empty()) {
        const PrgValue& value = arguments[0];
        if (value.kind == PrgValueKind::empty) {
            return make_boolean_value(true);
        }
        if (value.kind == PrgValueKind::string) {
            return make_boolean_value(trim_copy(value.string_value).empty());
        }
        return make_boolean_value(false);
    }
    if (function == "vartype" && !arguments.empty()) {
        return make_string_value(vartype_code(arguments[0]));
    }
    if (function == "type" && !arguments.empty()) {
        const std::string expr = normalize_type_expression(value_as_string(arguments[0]));
        if (is_bare_identifier_text(expr) && array_exists_callback(expr)) {
            return make_string_value("A");
        }
        return make_string_value(vartype_code(eval_expression_callback(expr)));
    }
    if (function == "iif" && arguments.size() >= 3U) {
        return value_as_bool(arguments[0]) ? arguments[1] : arguments[2];
    }
    if (function == "nvl" && arguments.size() >= 2U) {
        return arguments[0].kind == PrgValueKind::empty ? arguments[1] : arguments[0];
    }
    if (function == "between" && arguments.size() >= 3U) {
        const double value = value_as_number(arguments[0]);
        return make_boolean_value(value >= value_as_number(arguments[1]) && value <= value_as_number(arguments[2]));
    }
    if (function == "isdigit" && !arguments.empty()) {
        const std::string s = value_as_string(arguments[0]);
        return make_boolean_value(!s.empty() && std::isdigit(static_cast<unsigned char>(s[0])) != 0);
    }
    if (function == "isalpha" && !arguments.empty()) {
        const std::string s = value_as_string(arguments[0]);
        return make_boolean_value(!s.empty() && std::isalpha(static_cast<unsigned char>(s[0])) != 0);
    }
    if (function == "islower" && !arguments.empty()) {
        const std::string s = value_as_string(arguments[0]);
        return make_boolean_value(!s.empty() && std::islower(static_cast<unsigned char>(s[0])) != 0);
    }
    if (function == "isupper" && !arguments.empty()) {
        const std::string s = value_as_string(arguments[0]);
        return make_boolean_value(!s.empty() && std::isupper(static_cast<unsigned char>(s[0])) != 0);
    }
    if (function == "isleadbyte" && !arguments.empty()) {
        return make_boolean_value(false);
    }

    return std::nullopt;
}

}  // namespace copperfin::runtime
