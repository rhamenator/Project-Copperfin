#include "prg_engine_type_functions.h"

#include "prg_engine_helpers.h"

#include <cmath>

namespace copperfin::runtime {

namespace {

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
    if (function == "vartype" && !arguments.empty()) {
        return make_string_value(vartype_code(arguments[0]));
    }
    if (function == "type" && !arguments.empty()) {
        const std::string expr = trim_copy(value_as_string(arguments[0]));
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

    return std::nullopt;
}

}  // namespace copperfin::runtime
