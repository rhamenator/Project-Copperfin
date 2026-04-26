#include "prg_engine_numeric_functions.h"

#include "prg_engine_helpers.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string>

namespace copperfin::runtime {

namespace {

int color_component(const PrgValue& value) {
    return std::clamp(static_cast<int>(std::llround(value_as_number(value))), 0, 255);
}

}  // namespace

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
    if (function == "log10" && !arguments.empty()) {
        const double value = value_as_number(arguments[0]);
        if (value <= 0.0) {
            throw std::runtime_error("LOG10() requires a positive argument (got " + std::to_string(value) + ")");
        }
        return make_number_value(std::log10(value));
    }
    if (function == "pi") {
        return make_number_value(3.14159265358979323846);
    }
    if (function == "sin" && !arguments.empty()) {
        return make_number_value(std::sin(value_as_number(arguments[0])));
    }
    if (function == "cos" && !arguments.empty()) {
        return make_number_value(std::cos(value_as_number(arguments[0])));
    }
    if (function == "tan" && !arguments.empty()) {
        return make_number_value(std::tan(value_as_number(arguments[0])));
    }
    if (function == "asin" && !arguments.empty()) {
        const double value = value_as_number(arguments[0]);
        if (value < -1.0 || value > 1.0) {
            throw std::runtime_error("ASIN() requires an argument between -1 and 1 (got " + std::to_string(value) + ")");
        }
        return make_number_value(std::asin(value));
    }
    if (function == "acos" && !arguments.empty()) {
        const double value = value_as_number(arguments[0]);
        if (value < -1.0 || value > 1.0) {
            throw std::runtime_error("ACOS() requires an argument between -1 and 1 (got " + std::to_string(value) + ")");
        }
        return make_number_value(std::acos(value));
    }
    if (function == "atan" && !arguments.empty()) {
        return make_number_value(std::atan(value_as_number(arguments[0])));
    }
    if (function == "atn2" && arguments.size() >= 2U) {
        return make_number_value(std::atan2(value_as_number(arguments[0]), value_as_number(arguments[1])));
    }
    if (function == "dtor" && !arguments.empty()) {
        return make_number_value(value_as_number(arguments[0]) * (3.14159265358979323846 / 180.0));
    }
    if (function == "rtod" && !arguments.empty()) {
        return make_number_value(value_as_number(arguments[0]) * (180.0 / 3.14159265358979323846));
    }
    if (function == "sign" && !arguments.empty()) {
        const double value = value_as_number(arguments[0]);
        return make_number_value(value > 0.0 ? 1.0 : (value < 0.0 ? -1.0 : 0.0));
    }
    if (function == "rgb" && arguments.size() >= 3U) {
        const int red = color_component(arguments[0]);
        const int green = color_component(arguments[1]);
        const int blue = color_component(arguments[2]);
        return make_number_value(static_cast<double>(red + (green * 256) + (blue * 65536)));
    }
    if (function == "rand") {
        static thread_local std::mt19937 generator{5489U};
        if (!arguments.empty()) {
            const int seed = static_cast<int>(std::llround(value_as_number(arguments[0])));
            if (seed < 0) {
                generator.seed(static_cast<std::uint32_t>(-seed));
            } else if (seed > 0) {
                generator.seed(static_cast<std::uint32_t>(seed));
            }
        }
        return make_number_value(std::generate_canonical<double, 53>(generator));
    }

    return std::nullopt;
}

}  // namespace copperfin::runtime
