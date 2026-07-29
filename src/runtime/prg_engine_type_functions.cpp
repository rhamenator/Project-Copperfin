// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "prg_engine_type_functions.h"

#include "prg_engine_helpers.h"
#include "prg_engine_locale_code_page.h"

#include <charconv>
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
        return value.is_null ? "X" : "U";
    }
    if (value.kind == PrgValueKind::boolean) {
        return "L";
    }
    if (value.kind == PrgValueKind::number) {
        return "N";
    }
    if (value.kind == PrgValueKind::currency) {
        return "Y";
    }
    if (value.kind == PrgValueKind::int64 || value.kind == PrgValueKind::uint64) {
        return "I";
    }
    if (value.kind == PrgValueKind::string) {
        if (value.string_flavor == PrgStringFlavor::date) {
            return "D";
        }
        if (value.string_flavor == PrgStringFlavor::datetime) {
            return "T";
        }
    }
    int object_handle = 0;
    std::string object_prog_id;
    if (parse_object_handle_reference(value, object_handle, object_prog_id)) {
        return "O";
    }
    return "C";
}

}  // namespace

std::optional<PrgValue> evaluate_type_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments,
    const std::function<bool(const std::string&)>& array_exists_callback,
    const std::function<PrgValue(const std::string&)>& eval_expression_callback,
    const std::function<std::string(const std::string&)>& set_callback) {
    if (function == "empty" && !arguments.empty()) {
        const PrgValue& value = arguments[0];
        if (value.kind == PrgValueKind::empty) {
            return make_boolean_value(true);
        }
        if (value.kind == PrgValueKind::string) {
            return make_boolean_value(trim_copy(value.string_value).empty());
        }
        if (value.kind == PrgValueKind::number) {
            return make_boolean_value(value.number_value == 0.0);
        }
        if (value.kind == PrgValueKind::currency) {
            return make_boolean_value(value.currency_value == 0);
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
    if (function == "isnull" && !arguments.empty()) {
        return make_boolean_value(arguments[0].is_null);
    }
    if (function == "isempty" && !arguments.empty()) {
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
        if (array_exists_callback(expr)) {
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
    if (function == "evl" && arguments.size() >= 2U) {
        const PrgValue& value = arguments[0];
        bool is_empty = false;
        if (value.kind == PrgValueKind::empty) {
            is_empty = true;
        } else if (value.kind == PrgValueKind::string) {
            is_empty = trim_copy(value.string_value).empty();
        } else if (value.kind == PrgValueKind::number) {
            is_empty = value.number_value == 0.0;
        } else if (value.kind == PrgValueKind::currency) {
            is_empty = value.currency_value == 0;
        } else if (value.kind == PrgValueKind::boolean) {
            is_empty = !value.boolean_value;
        } else if (value.kind == PrgValueKind::int64) {
            is_empty = value.int64_value == 0;
        } else if (value.kind == PrgValueKind::uint64) {
            is_empty = value.uint64_value == 0U;
        } else {
            is_empty = true;
        }
        return is_empty ? arguments[1] : arguments[0];
    }
    if (function == "between" && arguments.size() >= 3U) {
        if (arguments[0].kind == PrgValueKind::string ||
            arguments[1].kind == PrgValueKind::string ||
            arguments[2].kind == PrgValueKind::string) {
            const std::string value = value_as_string(arguments[0]);
            const std::string lower = value_as_string(arguments[1]);
            const std::string upper = value_as_string(arguments[2]);
            return make_boolean_value(value >= lower && value <= upper);
        }
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
        const std::string value = value_as_string(arguments[0]);
        if (value.empty()) {
            return make_boolean_value(false);
        }

        int code_page = detail::default_host_code_page();
        const std::string configured_code_page = trim_copy(set_callback("CODEPAGE"));
        int parsed_code_page = 0;
        const auto [end, error] = std::from_chars(
            configured_code_page.data(),
            configured_code_page.data() + configured_code_page.size(),
            parsed_code_page);
        if (!configured_code_page.empty() && error == std::errc{} &&
            end == configured_code_page.data() + configured_code_page.size()) {
            code_page = parsed_code_page;
        }

        return make_boolean_value(detail::is_lead_byte_for_code_page(
            code_page,
            static_cast<unsigned char>(value.front())));
    }

    return std::nullopt;
}

}  // namespace copperfin::runtime
