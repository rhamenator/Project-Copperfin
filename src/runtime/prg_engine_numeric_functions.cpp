#include "prg_engine_numeric_functions.h"

#include "prg_engine_helpers.h"

#include <cmath>
#include <stdexcept>
#include <string>

namespace copperfin::runtime {

std::optional<PrgValue> evaluate_numeric_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments) {
    if (function == "int" && !arguments.empty()) {
        return make_number_value(std::trunc(value_as_number(arguments[0])));
    }
    if ((function == "abs" || function == "fabs") && !arguments.empty()) {
        return make_number_value(std::abs(value_as_number(arguments[0])));
    }
    if (function == "round" && !arguments.empty()) {
        const double value = value_as_number(arguments[0]);
        const int decimals = arguments.size() >= 2U ? static_cast<int>(std::llround(value_as_number(arguments[1]))) : 0;
        const double factor = std::pow(10.0, static_cast<double>(decimals));
        return make_number_value(std::round(value * factor) / factor);
    }
    if (function == "mod" && arguments.size() >= 2U) {
        const double a = value_as_number(arguments[0]);
        const double b = value_as_number(arguments[1]);
        if (std::abs(b) < 0.000001) {
            return make_number_value(0.0);
        }
        return make_number_value(std::fmod(a, b));
    }
    if (function == "sqrt" && !arguments.empty()) {
        const double value = value_as_number(arguments[0]);
        return make_number_value(value < 0.0 ? 0.0 : std::sqrt(value));
    }
    if (function == "ceiling" && !arguments.empty()) {
        return make_number_value(std::ceil(value_as_number(arguments[0])));
    }
    if (function == "floor" && !arguments.empty()) {
        return make_number_value(std::floor(value_as_number(arguments[0])));
    }
    if (function == "exp" && !arguments.empty()) {
        return make_number_value(std::exp(value_as_number(arguments[0])));
    }
    if (function == "log" && !arguments.empty()) {
        const double value = value_as_number(arguments[0]);
        if (value <= 0.0) {
            throw std::runtime_error("LOG() requires a positive argument (got " + std::to_string(value) + ")");
        }
        return make_number_value(std::log(value));
    }
    if (function == "pi") {
        return make_number_value(3.14159265358979323846);
    }
    if (function == "sign" && !arguments.empty()) {
        const double value = value_as_number(arguments[0]);
        return make_number_value(value > 0.0 ? 1.0 : (value < 0.0 ? -1.0 : 0.0));
    }

    return std::nullopt;
}

}  // namespace copperfin::runtime
