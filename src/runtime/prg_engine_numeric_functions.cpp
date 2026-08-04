// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "prg_engine_numeric_functions.h"

#include "localized_text.h"
#include "prg_engine_helpers.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <limits>
#include <iomanip>
#include <locale>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace copperfin::runtime {

namespace {

int color_component(const PrgValue& value) {
    return std::clamp(static_cast<int>(std::llround(value_as_number(value))), 0, 255);
}

std::string numeric_domain_error(
    const std::string& key,
    const std::string& function_name,
    const double value) {
    return runtime_text(
        key,
        {
            {"function", function_name},
            {"value", std::to_string(value)}
        });
}

std::optional<std::string> format_floating_text(const double value) {
#if defined(__APPLE__) && defined(_LIBCPP_VERSION)
    // Apple libc++ may not provide floating-point std::to_chars/std::from_chars.
    // Use a locale-stable fallback only on that platform/STL combination.
    std::ostringstream formatter;
    formatter.imbue(std::locale::classic());
    formatter << std::setprecision(std::numeric_limits<double>::digits10) << value;
    const std::string text = formatter.str();
    if (text.empty()) {
        return std::nullopt;
    }
    return text;
#else
    std::array<char, 128> buffer{};
    const auto converted = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
    if (converted.ec != std::errc{}) {
        return std::nullopt;
    }
    return std::string(buffer.data(), converted.ptr);
#endif
}

std::optional<double> parse_floating_text(const std::string_view text) {
#if defined(__APPLE__) && defined(_LIBCPP_VERSION)
    std::istringstream parser{std::string{text}};
    parser.imbue(std::locale::classic());
    double value = 0.0;
    parser >> value;
    if (parser.fail()) {
        return std::nullopt;
    }
    parser >> std::ws;
    if (!parser.eof()) {
        return std::nullopt;
    }
    return value;
#else
    double value = 0.0;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size()) {
        return std::nullopt;
    }
    return value;
#endif
}

std::optional<double> round_decimal_value(const double value, const int decimal_places) {
    if (!std::isfinite(value) || decimal_places < -308 || decimal_places > 308) {
        return std::nullopt;
    }

    const auto text_value = format_floating_text(value);
    if (!text_value.has_value()) {
        return std::nullopt;
    }
    std::string text = *text_value;
    const bool negative = !text.empty() && text.front() == '-';
    if (negative) {
        text.erase(text.begin());
    }

    int exponent = 0;
    const std::size_t exponent_position = text.find_first_of("eE");
    if (exponent_position != std::string::npos) {
        const std::string exponent_text = text.substr(exponent_position + 1U);
        const auto parsed_exponent = std::from_chars(
            exponent_text.data(), exponent_text.data() + exponent_text.size(), exponent);
        if (parsed_exponent.ec != std::errc{} || parsed_exponent.ptr != exponent_text.data() + exponent_text.size()) {
            return std::nullopt;
        }
        text.erase(exponent_position);
    }

    const std::size_t decimal_position = text.find('.');
    const int digits_before_decimal = decimal_position == std::string::npos
                                          ? static_cast<int>(text.size())
                                          : static_cast<int>(decimal_position);
    std::string digits;
    digits.reserve(text.size());
    for (const char character : text) {
        if (character != '.') {
            digits.push_back(character);
        }
    }
    if (digits.empty()) {
        return std::copysign(0.0, negative ? -1.0 : 1.0);
    }

    const long long decimal_index = static_cast<long long>(digits_before_decimal) + exponent;
    const long long cut = decimal_index + decimal_places;
    std::string rounded_digits;
    long long rounded_decimal_index = decimal_index;

    if (cut <= 0LL) {
        if (!digits.empty() && digits.front() >= '5') {
            rounded_digits = "1";
            rounded_decimal_index = 1LL - decimal_places;
        }
    } else if (cut >= static_cast<long long>(digits.size())) {
        rounded_digits = digits;
    } else {
        rounded_digits = digits.substr(0U, static_cast<std::size_t>(cut));
        bool carried = false;
        if (digits[static_cast<std::size_t>(cut)] >= '5') {
            bool carry = true;
            for (auto digit = rounded_digits.rbegin(); digit != rounded_digits.rend() && carry; ++digit) {
                if (*digit == '9') {
                    *digit = '0';
                } else {
                    ++*digit;
                    carry = false;
                }
            }
            if (carry) {
                rounded_digits.insert(rounded_digits.begin(), '1');
                carried = true;
            }
        }
        rounded_decimal_index = cut - decimal_places + (carried ? 1LL : 0LL);
    }

    if (rounded_digits.empty() || rounded_digits.find_first_not_of('0') == std::string::npos) {
        return std::copysign(0.0, negative ? -1.0 : 1.0);
    }

    std::string rounded_text;
    if (rounded_decimal_index <= 0LL) {
        rounded_text = "0.";
        rounded_text.append(static_cast<std::size_t>(-rounded_decimal_index), '0');
        rounded_text += rounded_digits;
    } else if (rounded_decimal_index >= static_cast<long long>(rounded_digits.size())) {
        rounded_text = rounded_digits;
        rounded_text.append(
            static_cast<std::size_t>(rounded_decimal_index - static_cast<long long>(rounded_digits.size())), '0');
    } else {
        rounded_text = rounded_digits;
        rounded_text.insert(static_cast<std::size_t>(rounded_decimal_index), 1U, '.');
    }

    const auto rounded_value = parse_floating_text(rounded_text);
    if (!rounded_value.has_value()) {
        return std::nullopt;
    }
    return negative ? -*rounded_value : *rounded_value;
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
        const double requested_decimals = arguments.size() >= 2U ? value_as_number(arguments[1]) : 0.0;
        const double rounded_decimals = std::round(requested_decimals);
        const int decimals = !std::isfinite(rounded_decimals)
                                 ? 0
                                 : rounded_decimals > static_cast<double>(std::numeric_limits<int>::max())
                                       ? std::numeric_limits<int>::max()
                                       : rounded_decimals < static_cast<double>(std::numeric_limits<int>::min())
                                             ? std::numeric_limits<int>::min()
                                             : static_cast<int>(rounded_decimals);
        if (const auto rounded = round_decimal_value(value, decimals); rounded.has_value()) {
            return make_number_value(*rounded);
        }
        const double factor = std::pow(10.0, static_cast<double>(decimals));
        return make_number_value(std::round(value * factor) / factor);
    }
    if (function == "mod" && arguments.size() >= 2U) {
        const double a = value_as_number(arguments[0]);
        const double b = value_as_number(arguments[1]);
        if (b == 0.0) {
            return make_number_value(0.0);
        }
        double remainder = std::fmod(a, b);
        if (remainder != 0.0 && std::signbit(remainder) != std::signbit(b)) {
            remainder += b;
        }
        return make_number_value(remainder);
    }
    if (function == "sqrt" && !arguments.empty()) {
        const double value = value_as_number(arguments[0]);
        if (value < 0.0) {
            throw std::runtime_error(numeric_domain_error(
                "Runtime.Prg.Numeric.Error.NonNegativeArgumentRequired",
                "SQRT()",
                value));
        }
        return make_number_value(std::sqrt(value));
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
            throw std::runtime_error(numeric_domain_error(
                "Runtime.Prg.Numeric.Error.PositiveArgumentRequired",
                "LOG()",
                value));
        }
        return make_number_value(std::log(value));
    }
    if (function == "log10" && !arguments.empty()) {
        const double value = value_as_number(arguments[0]);
        if (value <= 0.0) {
            throw std::runtime_error(numeric_domain_error(
                "Runtime.Prg.Numeric.Error.PositiveArgumentRequired",
                "LOG10()",
                value));
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
            throw std::runtime_error(numeric_domain_error(
                "Runtime.Prg.Numeric.Error.UnitRangeArgumentRequired",
                "ASIN()",
                value));
        }
        return make_number_value(std::asin(value));
    }
    if (function == "acos" && !arguments.empty()) {
        const double value = value_as_number(arguments[0]);
        if (value < -1.0 || value > 1.0) {
            throw std::runtime_error(numeric_domain_error(
                "Runtime.Prg.Numeric.Error.UnitRangeArgumentRequired",
                "ACOS()",
                value));
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
    // HEX(nNumber) — return uppercase hexadecimal representation of a non-negative integer
    if (function == "hex" && !arguments.empty()) {
        const auto n = static_cast<std::uint64_t>(
            static_cast<std::int64_t>(std::max(0.0, std::trunc(value_as_number(arguments[0])))));
        if (n == 0U) {
            return make_string_value("0");
        }
        std::ostringstream oss;
        oss << std::uppercase << std::hex << n;
        return make_string_value(oss.str());
    }
    // FV(nRate, nPeriods, nPayment [, nPV [, nType]])
    // Future value of an annuity.  nType 0 = end-of-period (default), 1 = beginning.
    if (function == "fv" && arguments.size() >= 3U) {
        const double rate    = value_as_number(arguments[0]);
        const double nper    = value_as_number(arguments[1]);
        const double payment = value_as_number(arguments[2]);
        const double pv      = arguments.size() >= 4U ? value_as_number(arguments[3]) : 0.0;
        const int    type    = arguments.size() >= 5U ? static_cast<int>(std::llround(value_as_number(arguments[4]))) : 0;
        double fv = 0.0;
        if (std::abs(rate) < 1e-15) {
            fv = -(pv + payment * nper);
        } else {
            const double factor = std::pow(1.0 + rate, nper);
            fv = -(pv * factor + payment * (type == 1 ? (1.0 + rate) : 1.0) * (factor - 1.0) / rate);
        }
        return make_number_value(fv);
    }
    // PV(nRate, nPeriods, nPayment [, nFV [, nType]])
    // Present value of an annuity.  nType 0 = end-of-period (default), 1 = beginning.
    if (function == "pv" && arguments.size() >= 3U) {
        const double rate    = value_as_number(arguments[0]);
        const double nper    = value_as_number(arguments[1]);
        const double payment = value_as_number(arguments[2]);
        const double fv      = arguments.size() >= 4U ? value_as_number(arguments[3]) : 0.0;
        const int    type    = arguments.size() >= 5U ? static_cast<int>(std::llround(value_as_number(arguments[4]))) : 0;
        double pv = 0.0;
        if (std::abs(rate) < 1e-15) {
            pv = -(fv + payment * nper);
        } else {
            const double factor = std::pow(1.0 + rate, nper);
            pv = -(fv / factor + payment * (type == 1 ? (1.0 + rate) : 1.0) * (1.0 - 1.0 / factor) / rate);
        }
        return make_number_value(pv);
    }
    // PAYMENT(nPrincipal, nRate, nPeriods)
    // Periodic payment for a loan (principal > 0, rate per period, nPeriods > 0).
    if (function == "payment" && arguments.size() >= 3U) {
        const double principal = value_as_number(arguments[0]);
        const double rate      = value_as_number(arguments[1]);
        const double nper      = value_as_number(arguments[2]);
        if (std::abs(rate) < 1e-15 || nper < 1.0) {
            return make_number_value(nper >= 1.0 ? principal / nper : 0.0);
        }
        const double factor = std::pow(1.0 + rate, nper);
        return make_number_value(principal * rate * factor / (factor - 1.0));
    }

    return std::nullopt;
}

}  // namespace copperfin::runtime
