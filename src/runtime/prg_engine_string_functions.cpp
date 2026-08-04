// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "prg_engine_string_functions.h"

#include "prg_engine_helpers.h"
#include "localized_text.h"
#include "prg_compatibility_error.h"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <functional>
#include <iomanip>
#include <limits>
#include <locale>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace copperfin::runtime {

namespace {

#include "prg_engine_string_function_helpers.inl"

}  // namespace

std::string format_value_for_display(
    const PrgValue& value,
    const std::function<std::string(const std::string&)>& set_callback) {
    if (value.kind != PrgValueKind::number || !std::isfinite(value.number_value)) {
        return value_as_string(value);
    }

    int decimals = 2;
    try {
        decimals = std::clamp(std::stoi(trim_copy(set_callback("DECIMALS"))), 0, 18);
    } catch (...) {
    }

    const std::string fixed_setting = normalize_identifier(trim_copy(set_callback("FIXED")));
    const bool fixed = fixed_setting == "on" || fixed_setting == "true" || fixed_setting == "yes" || fixed_setting == "1";

    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::fixed << std::setprecision(decimals) << value.number_value;
    std::string formatted = stream.str();
    if (!fixed) {
        const std::size_t decimal_pos = formatted.find('.');
        if (decimal_pos != std::string::npos) {
            while (!formatted.empty() && formatted.back() == '0') {
                formatted.pop_back();
            }
            if (!formatted.empty() && formatted.back() == '.') {
                formatted.pop_back();
            }
        }
    }

    return apply_numeric_picture_symbols(std::move(formatted), true, false, set_callback);
}

std::optional<PrgValue> evaluate_string_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments,
    bool exact_string_compare,
    std::size_t memo_width,
    const std::function<std::string(const std::string&)>& set_callback) {
    if (function == "len" && !arguments.empty()) {
        return make_number_value(static_cast<double>(value_as_string(arguments[0]).size()));
    }
    if (function == "lenc" && !arguments.empty()) {
        return make_number_value(static_cast<double>(utf8_scalar_offsets_local(value_as_string(arguments[0])).size() - 1U));
    }
    if (function == "left" && arguments.size() >= 2U) {
        const std::string src = value_as_string(arguments[0]);
        const std::size_t n = static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[1])));
        return make_string_value(src.substr(0U, std::min(n, src.size())));
    }
    if (function == "right" && arguments.size() >= 2U) {
        const std::string src = value_as_string(arguments[0]);
        const std::size_t n = static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[1])));
        return make_string_value(n >= src.size() ? src : src.substr(src.size() - n));
    }
    if (function == "leftc" && arguments.size() >= 2U) {
        return make_string_value(utf8_scalar_slice_local(
            value_as_string(arguments[0]), 1U,
            static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[1])))));
    }
    if (function == "rightc" && arguments.size() >= 2U) {
        const std::string src = value_as_string(arguments[0]);
        const std::size_t scalar_count = utf8_scalar_offsets_local(src).size() - 1U;
        const std::size_t n = static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[1])));
        return make_string_value(n >= scalar_count ? src : utf8_scalar_slice_local(src, scalar_count - n + 1U, n));
    }
    if (function == "upper" && !arguments.empty()) {
        return make_string_value(uppercase_copy(value_as_string(arguments[0])));
    }
    if (function == "lower" && !arguments.empty()) {
        std::string s = value_as_string(arguments[0]);
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return make_string_value(std::move(s));
    }
    if (function == "ltrim" && !arguments.empty()) {
        return make_string_value(ltrim_space_copy(value_as_string(arguments[0])));
    }
    if ((function == "rtrim" || function == "trim") && !arguments.empty()) {
        return make_string_value(rtrim_space_copy(value_as_string(arguments[0])));
    }
    if (function == "space" && !arguments.empty()) {
        const std::size_t n = static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[0])));
        return make_string_value(std::string(n, ' '));
    }
    if (function == "replicate" && arguments.size() >= 2U) {
        const std::string src = value_as_string(arguments[0]);
        const std::size_t n = static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[1])));
        std::string result;
        result.reserve(src.size() * n);
        for (std::size_t i = 0; i < n; ++i) {
            result += src;
        }
        return make_string_value(std::move(result));
    }
    if (function == "strtran" && arguments.size() >= 2U) {
        std::string src = value_as_string(arguments[0]);
        const std::string find = value_as_string(arguments[1]);
        const std::string repl = arguments.size() >= 3U ? value_as_string(arguments[2]) : std::string{};
        const std::size_t start_occurrence = arguments.size() >= 4U
                                                 ? static_cast<std::size_t>(std::max(1.0, value_as_number(arguments[3])))
                                                 : 1U;
        const double raw_occurrence_limit = arguments.size() >= 5U
                                                 ? value_as_number(arguments[4])
                                                 : -1.0;
        const std::size_t occurrence_limit = raw_occurrence_limit < 0.0
                                                 ? std::numeric_limits<std::size_t>::max()
                                                 : static_cast<std::size_t>(raw_occurrence_limit);
        const std::size_t flags = arguments.size() >= 6U
                                      ? static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[5])))
                                      : 0U;
        const bool case_insensitive = (flags & 1U) != 0U;
        if (!find.empty()) {
            std::string result;
            const std::string search_source = case_insensitive ? uppercase_copy(src) : src;
            const std::string search_find = case_insensitive ? uppercase_copy(find) : find;
            std::size_t pos = 0U;
            std::size_t match_index = 0U;
            std::size_t replaced_count = 0U;
            while (pos < src.size()) {
                const std::size_t found = search_source.find(search_find, pos);
                if (found == std::string::npos) {
                    result += src.substr(pos);
                    break;
                }
                ++match_index;
                result += src.substr(pos, found - pos);
                if (match_index >= start_occurrence && replaced_count < occurrence_limit) {
                    result += repl;
                    ++replaced_count;
                } else {
                    result += src.substr(found, find.size());
                }
                pos = found + find.size();
            }
            src = std::move(result);
        }
        return make_string_value(std::move(src));
    }
    if (function == "stuff" && arguments.size() >= 4U) {
        std::string src = value_as_string(arguments[0]);
        const double raw_start = value_as_number(arguments[1]);
        const std::size_t start = static_cast<std::size_t>(std::max(1.0, raw_start)) - 1U;
        const std::size_t length = raw_start <= 0.0
                                       ? 0U
                                       : static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[2])));
        const std::string replacement = value_as_string(arguments[3]);
        if (start <= src.size()) {
            src.replace(start, std::min(length, src.size() - start), replacement);
        }
        return make_string_value(std::move(src));
    }
    if (function == "asc" && !arguments.empty()) {
        const std::string src = value_as_string(arguments[0]);
        return make_number_value(src.empty() ? 0.0 : static_cast<double>(static_cast<unsigned char>(src[0])));
    }
    if (function == "val" && !arguments.empty()) {
        const std::string src = trim_copy(value_as_string(arguments[0]));
        if (src.empty()) {
            return make_number_value(0.0);
        }
        const bool currency = src.front() == '$';
        const std::size_t numeric_start = currency ? 1U : 0U;
        std::size_t numeric_end = numeric_start;
        if (numeric_end < src.size() && (src[numeric_end] == '+' || src[numeric_end] == '-')) {
            ++numeric_end;
        }
        const std::size_t integer_start = numeric_end;
        while (numeric_end < src.size() && std::isdigit(static_cast<unsigned char>(src[numeric_end]))) {
            ++numeric_end;
        }
        if (numeric_end == integer_start) {
            return currency ? make_currency_value(0) : make_number_value(0.0);
        }
        if (numeric_end < src.size() && src[numeric_end] == '.') {
            ++numeric_end;
            while (numeric_end < src.size() && std::isdigit(static_cast<unsigned char>(src[numeric_end]))) {
                ++numeric_end;
            }
        }
        if (numeric_end < src.size() && (src[numeric_end] == 'E' || src[numeric_end] == 'e')) {
            const std::size_t exponent_start = numeric_end;
            ++numeric_end;
            if (numeric_end < src.size() && (src[numeric_end] == '+' || src[numeric_end] == '-')) {
                ++numeric_end;
            }
            const std::size_t exponent_digits_start = numeric_end;
            while (numeric_end < src.size() && std::isdigit(static_cast<unsigned char>(src[numeric_end]))) {
                ++numeric_end;
            }
            if (numeric_end == exponent_digits_start) {
                numeric_end = exponent_start;
            }
        }
        double result = 0.0;
        const std::string numeric_text = src.substr(numeric_start, numeric_end - numeric_start);
        if (currency) {
            return make_currency_value(parse_currency_scaled_value(numeric_text).value_or(0));
        }
        result = try_parse_invariant_double(numeric_text).value_or(0.0);
        return make_number_value(result);
    }
    if (function == "occurs" && arguments.size() >= 2U) {
        const std::string needle = value_as_string(arguments[0]);
        const std::string haystack = value_as_string(arguments[1]);
        if (needle.empty()) {
            return make_number_value(0.0);
        }
        std::size_t count = 0U;
        std::size_t pos = 0U;
        while ((pos = haystack.find(needle, pos)) != std::string::npos) {
            ++count;
            pos += needle.size();
        }
        return make_number_value(static_cast<double>(count));
    }
    if ((function == "padl" || function == "padr" || function == "padc") && arguments.size() >= 2U) {
        std::string src = value_as_string(arguments[0]);
        const std::size_t width = static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[1])));
        const char pad_char = (arguments.size() >= 3U && !value_as_string(arguments[2]).empty())
                                  ? value_as_string(arguments[2])[0]
                                  : ' ';
        if (src.size() > width) {
            if (function == "padl") {
                src = src.substr(src.size() - width);
            } else if (function == "padr") {
                src = src.substr(0U, width);
            } else {
                src = src.substr((src.size() - width) / 2U, width);
            }
        }
        if (function == "padl") {
            src = std::string(width - src.size(), pad_char) + src;
        } else if (function == "padr") {
            src += std::string(width - src.size(), pad_char);
        } else {
            const std::size_t total_pad = width - src.size();
            const std::size_t left_pad = total_pad / 2U;
            const std::size_t right_pad = total_pad - left_pad;
            src = std::string(left_pad, pad_char) + src + std::string(right_pad, pad_char);
        }
        return make_string_value(std::move(src));
    }
    if ((function == "chrtran" || function == "chrtranc") && arguments.size() >= 3U) {
        const bool case_insensitive = function == "chrtranc";
        const std::string src = value_as_string(arguments[0]);
        const std::string from_chars = value_as_string(arguments[1]);
        const std::string to_chars = value_as_string(arguments[2]);
        const std::string from_lookup = case_insensitive ? uppercase_copy(from_chars) : from_chars;
        std::string result;
        result.reserve(src.size());
        for (const char c : src) {
            const char lookup = case_insensitive ? static_cast<char>(std::toupper(static_cast<unsigned char>(c))) : c;
            const auto pos = from_lookup.find(lookup);
            if (pos == std::string::npos) {
                result += c;
            } else if (pos < to_chars.size()) {
                result += to_chars[pos];
            }
        }
        return make_string_value(std::move(result));
    }
    if (function == "proper" && !arguments.empty()) {
        std::string src = value_as_string(arguments[0]);
        bool start_word = true;
        for (char& ch : src) {
            const auto raw = static_cast<unsigned char>(ch);
            if (std::isalpha(raw) != 0) {
                ch = static_cast<char>(start_word ? std::toupper(raw) : std::tolower(raw));
                start_word = false;
            } else if (std::isdigit(raw) == 0) {
                start_word = true;
            }
        }
        return make_string_value(std::move(src));
    }
    if (function == "strconv" && arguments.size() >= 2U) {
        std::string src = value_as_string(arguments[0]);
        const int mode = static_cast<int>(std::llround(value_as_number(arguments[1])));
        if (mode == 7) {
            std::transform(src.begin(), src.end(), src.begin(), [](unsigned char ch) {
                return static_cast<char>(std::tolower(ch));
            });
        } else if (mode == 8) {
            std::transform(src.begin(), src.end(), src.begin(), [](unsigned char ch) {
                return static_cast<char>(std::toupper(ch));
            });
        }
        return make_string_value(std::move(src));
    }
    if (function == "soundex" && !arguments.empty()) {
        return make_string_value(soundex_code(value_as_string(arguments[0])));
    }
    if (function == "difference" && arguments.size() >= 2U) {
        const std::string left = soundex_code(value_as_string(arguments[0]));
        const std::string right = soundex_code(value_as_string(arguments[1]));
        int score = 0;
        for (std::size_t index = 0U; index < std::min(left.size(), right.size()); ++index) {
            if (left[index] == right[index]) {
                ++score;
            }
        }
        return make_number_value(static_cast<double>(score));
    }
    if (function == "likec" && arguments.size() >= 2U) {
        return make_boolean_value(wildcard_match_utf8_scalar_case_sensitive_local(
            value_as_string(arguments[0]),
            value_as_string(arguments[1])));
    }
    if (function == "like" && arguments.size() >= 2U) {
        return make_boolean_value(wildcard_match_case_sensitive_local(
            value_as_string(arguments[0]),
            value_as_string(arguments[1])));
    }
    if (function == "inlist" && arguments.size() >= 2U) {
        for (std::size_t index = 1U; index < arguments.size(); ++index) {
            if (expression_values_equal(arguments[0], arguments[index], exact_string_compare)) {
                return make_boolean_value(true);
            }
        }
        return make_boolean_value(false);
    }
    if ((function == "getwordcount" || function == "getwordnum") && !arguments.empty()) {
        const std::string src = value_as_string(arguments[0]);
        const std::string delim = (arguments.size() >= (function == "getwordcount" ? 2U : 3U))
                                      ? value_as_string(arguments[function == "getwordcount" ? 1U : 2U])
                                      : std::string{" \t\r\n"};
        if (delim.empty()) {
            return function == "getwordcount" ? make_number_value(1.0) : make_string_value(src);
        }
        std::vector<std::string> words;
        std::size_t start = 0U;
        while (start < src.size()) {
            start = src.find_first_not_of(delim, start);
            if (start == std::string::npos) {
                break;
            }
            const std::size_t end = src.find_first_of(delim, start);
            words.push_back(end == std::string::npos ? src.substr(start) : src.substr(start, end - start));
            if (end == std::string::npos) {
                break;
            }
            start = end + 1U;
        }
        if (function == "getwordcount") {
            return make_number_value(static_cast<double>(words.size()));
        }
        const double requested_index = value_as_number(arguments[1]);
        if (requested_index <= 0.0) {
            return make_string_value(std::string{});
        }
        const std::size_t n = static_cast<std::size_t>(requested_index);
        return make_string_value(n <= words.size() ? words[n - 1U] : std::string{});
    }
    if (function == "memlines" && !arguments.empty()) {
        const std::vector<std::string> lines = memo_width_lines_with_options(
            value_as_string(arguments[0]),
            arguments,
            1U,
            memo_width,
            2U,
            3U);
        return make_number_value(static_cast<double>(lines.size()));
    }
    if (function == "mline" && arguments.size() >= 2U) {
        const std::string source = value_as_string(arguments[0]);
        const double requested_line = value_as_number(arguments[1]);
        if (requested_line <= 0.0) {
            return make_string_value(std::string{});
        }
        const std::size_t start = arguments.size() >= 3U
                                      ? static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[2])))
                                      : 0U;
        if (start >= source.size()) {
            return make_string_value(std::string{});
        }
        const std::vector<std::string> lines = memo_width_lines_with_options(
            source.substr(start),
            arguments,
            3U,
            memo_width,
            4U,
            5U);
        const std::size_t line_index = static_cast<std::size_t>(requested_line);
        return make_string_value(line_index >= 1U && line_index <= lines.size() ? lines[line_index - 1U] : std::string{});
    }
    if ((function == "at_c" || function == "atc" || function == "atcc") && arguments.size() >= 2U) {
        if (function == "at_c" && arguments.size() >= 3U) {
            const double requested_occurrence = value_as_number(arguments[2]);
            if (!std::isfinite(requested_occurrence) || requested_occurrence <= 0.0) {
                throw PrgCompatibilityError(
                    runtime_text("Runtime.Prg.String.Error.InvalidOccurrence"),
                    11);
            }
        }
        const std::size_t occurrence = arguments.size() >= 3U
                                           ? static_cast<std::size_t>(std::max(1.0, value_as_number(arguments[2])))
                                           : 1U;
        if (function == "at_c" || function == "atcc") {
            return make_number_value(static_cast<double>(find_utf8_scalar_occurrence_local(
                value_as_string(arguments[0]),
                value_as_string(arguments[1]),
                occurrence,
                false,
                function == "atcc")));
        }
        std::string needle = uppercase_copy(value_as_string(arguments[0]));
        std::string haystack = uppercase_copy(value_as_string(arguments[1]));
        if (needle.empty()) {
            return make_number_value(0.0);
        }
        std::size_t found_count = 0U;
        std::size_t search_pos = 0U;
        while (search_pos <= haystack.size()) {
            const auto found = haystack.find(needle, search_pos);
            if (found == std::string::npos) {
                break;
            }
            if (++found_count == occurrence) {
                return make_number_value(static_cast<double>(found + 1U));
            }
            search_pos = found + needle.size();
        }
        return make_number_value(0.0);
    }
    if (function == "at" && arguments.size() >= 2U) {
        std::string needle = value_as_string(arguments[0]);
        std::string haystack = value_as_string(arguments[1]);
        const std::size_t occurrence = arguments.size() >= 3U
                                           ? static_cast<std::size_t>(std::max(1.0, value_as_number(arguments[2])))
                                           : 1U;
        if (needle.empty()) {
            return make_number_value(0.0);
        }
        std::size_t found_count = 0U;
        std::size_t search_pos = 0U;
        while (search_pos <= haystack.size()) {
            const auto found = haystack.find(needle, search_pos);
            if (found == std::string::npos) {
                break;
            }
            ++found_count;
            if (found_count == occurrence) {
                return make_number_value(static_cast<double>(found + 1U));
            }
            search_pos = found + needle.size();
        }
        return make_number_value(0.0);
    }
    if (function == "ratc" && arguments.size() >= 2U) {
        const std::size_t occurrence = arguments.size() >= 3U
                                           ? static_cast<std::size_t>(std::max(1.0, value_as_number(arguments[2])))
                                           : 1U;
        return make_number_value(static_cast<double>(find_utf8_scalar_occurrence_local(
            value_as_string(arguments[0]), value_as_string(arguments[1]), occurrence, true)));
    }
    if (function == "rat" && arguments.size() >= 2U) {
        std::string needle = value_as_string(arguments[0]);
        std::string haystack = value_as_string(arguments[1]);
        const std::size_t occurrence = arguments.size() >= 3U
                                           ? static_cast<std::size_t>(std::max(1.0, value_as_number(arguments[2])))
                                           : 1U;
        if (needle.empty()) {
            return make_number_value(0.0);
        }
        std::size_t found_count = 0U;
        std::size_t search_pos = std::string::npos;
        while (true) {
            const auto found = haystack.rfind(needle, search_pos);
            if (found == std::string::npos) {
                break;
            }
            ++found_count;
            if (found_count == occurrence) {
                return make_number_value(static_cast<double>(found + 1U));
            }
            if (found < needle.size()) {
                break;
            }
            search_pos = found - needle.size();
        }
        return make_number_value(0.0);
    }
    if ((function == "atline" || function == "atcline" || function == "ratline") && arguments.size() >= 2U) {
        const bool reverse = function == "ratline";
        const bool case_insensitive = function == "atcline";
        std::string needle = value_as_string(arguments[0]);
        std::vector<std::string> lines = split_text_lines(value_as_string(arguments[1]));
        const std::size_t occurrence = arguments.size() >= 3U
                                           ? static_cast<std::size_t>(std::max(1.0, value_as_number(arguments[2])))
                                           : 1U;
        if (case_insensitive) {
            needle = uppercase_copy(std::move(needle));
            for (std::string& line : lines) {
                line = uppercase_copy(std::move(line));
            }
        }
        if (needle.empty()) {
            return make_number_value(0.0);
        }
        std::size_t found_count = 0U;
        if (reverse) {
            for (std::size_t index = lines.size(); index > 0U; --index) {
                if (lines[index - 1U].find(needle) == std::string::npos) {
                    continue;
                }
                ++found_count;
                if (found_count == occurrence) {
                    return make_number_value(static_cast<double>(index));
                }
            }
        } else {
            for (std::size_t index = 0U; index < lines.size(); ++index) {
                if (lines[index].find(needle) == std::string::npos) {
                    continue;
                }
                ++found_count;
                if (found_count == occurrence) {
                    return make_number_value(static_cast<double>(index + 1U));
                }
            }
        }
        return make_number_value(0.0);
    }
    if (function == "substr" && arguments.size() >= 2U) {
        const std::string source = value_as_string(arguments[0]);
        const std::size_t start = static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[1]) - 1.0));
        const std::size_t length = arguments.size() >= 3U
                                       ? static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[2])))
                                       : std::string::npos;
        return make_string_value(start >= source.size() ? std::string{} : source.substr(start, length));
    }
    if (function == "substrc" && arguments.size() >= 2U) {
        const std::string source = value_as_string(arguments[0]);
        const double raw_start = value_as_number(arguments[1]);
        const std::size_t start = static_cast<std::size_t>(std::max(1.0, raw_start));
        const std::size_t length = arguments.size() >= 3U
                                       ? static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[2])))
                                       : std::numeric_limits<std::size_t>::max();
        return make_string_value(utf8_scalar_slice_local(source, start, length));
    }
    if (function == "stuffc" && arguments.size() >= 4U) {
        const double raw_start = value_as_number(arguments[1]);
        const std::size_t start = static_cast<std::size_t>(std::max(1.0, raw_start));
        const std::size_t length = raw_start <= 0.0
                                       ? 0U
                                       : static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[2])));
        return make_string_value(replace_utf8_scalars_local(
            value_as_string(arguments[0]), start, length, value_as_string(arguments[3])));
    }
    if (function == "alltrim" && !arguments.empty()) {
        return make_string_value(trim_space_copy(value_as_string(arguments[0])));
    }
    if (function == "chr" && !arguments.empty()) {
        return make_string_value(std::string(1U, static_cast<char>(std::llround(value_as_number(arguments[0])))));
    }
    if (function == "str" && !arguments.empty()) {
        const int decimals = arguments.size() >= 3U
                                 ? static_cast<int>(std::max(0.0, value_as_number(arguments[2])))
                                 : 0;
        std::ostringstream stream;
        stream.imbue(std::locale::classic());
        stream << std::fixed << std::setprecision(decimals) << value_as_number(arguments[0]);
        std::string result = stream.str();
        const int width = arguments.size() >= 2U
                              ? static_cast<int>(std::llround(value_as_number(arguments[1])))
                              : 10;
        if (width > 0) {
            if (result.size() > static_cast<std::size_t>(width)) {
                return make_string_value(std::string(static_cast<std::size_t>(width), '*'));
            }
            if (result.size() < static_cast<std::size_t>(width)) {
                result.insert(result.begin(), static_cast<std::size_t>(width) - result.size(), ' ');
            }
        }
        return make_string_value(std::move(result));
    }
    if (function == "transform" && !arguments.empty()) {
        const std::string raw_picture = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{};
        const std::string picture = uppercase_copy(raw_picture);
        if (picture_has_flag(picture, "@Z") && is_zeroish_transform_value(arguments[0])) {
            return make_string_value("");
        }

        std::string transformed;
        if (!picture.empty()) {
            if (picture_is_digit_only_numeric(picture)) {
                transformed = format_digit_only_numeric_picture(
                    value_as_number(arguments[0]),
                    picture);
            } else if (picture_has_flag(picture, "@!")) {
                transformed = uppercase_copy(value_as_string(arguments[0]));
            } else if (picture_has_flag(picture, "@L")) {
                transformed = value_as_string(arguments[0]);
                std::transform(transformed.begin(), transformed.end(), transformed.begin(), [](unsigned char ch) {
                    return static_cast<char>(std::tolower(ch));
                });
            } else if (picture_has_flag(picture, "@R")) {
                transformed = apply_literal_picture_template(
                    value_as_string(arguments[0]),
                    picture_payload_after_flag(raw_picture, picture, "@R"));
            } else {
                const std::size_t decimal_pos = picture.find('.');
                if (decimal_pos != std::string::npos ||
                    (picture.find(',') != std::string::npos && picture_has_numeric_placeholders(picture))) {
                    std::size_t decimals = 0U;
                    if (decimal_pos != std::string::npos) {
                        for (std::size_t index = decimal_pos + 1U; index < picture.size(); ++index) {
                            if (picture[index] == '9' || picture[index] == '#' || picture[index] == '0') {
                                ++decimals;
                            }
                        }
                    }
                    std::ostringstream stream;
                    stream.imbue(std::locale::classic());
                    stream << std::fixed << std::setprecision(static_cast<int>(decimals)) << value_as_number(arguments[0]);
                    transformed = apply_numeric_picture_symbols(
                        stream.str(),
                        picture.find(',') != std::string::npos,
                        picture.find('$') != std::string::npos,
                        set_callback);
                }
            }
        }

        if (transformed.empty() && !(picture_has_flag(picture, "@Z") && is_zeroish_transform_value(arguments[0]))) {
            transformed = format_value_for_display(arguments[0], set_callback);
        }
        if (picture_has_flag(picture, "@B")) {
            transformed = left_justified_trim(std::move(transformed));
        }
        return make_string_value(std::move(transformed));
    }
    if (function == "strextract" && arguments.size() >= 3U) {
        const std::string src = value_as_string(arguments[0]);
        const std::string begin_delim = value_as_string(arguments[1]);
        const std::string end_delim = value_as_string(arguments[2]);
        const std::size_t occurrence = arguments.size() >= 4U
                                           ? static_cast<std::size_t>(std::max(1.0, value_as_number(arguments[3])))
                                           : 1U;
        const int flags = arguments.size() >= 5U ? static_cast<int>(value_as_number(arguments[4])) : 0;
        const bool case_insensitive = (flags & 1) != 0;
        const bool end_delimiter_optional = (flags & 2) != 0;
        const bool include_delimiters = (flags & 4) != 0;
        // An empty begin delimiter is defined as a search from the start of
        // the expression to the first end delimiter. It has no later begin
        // occurrences to select, so avoid retrying the same zero-width match.
        if (begin_delim.empty() && occurrence > 1U) {
            return make_string_value(std::string{});
        }
        std::size_t search_pos = 0U;
        std::size_t found_count = 0U;
        while (search_pos <= src.size()) {
            std::size_t begin_pos;
            if (begin_delim.empty()) {
                begin_pos = 0U;
            } else if (case_insensitive) {
                const std::string src_up = uppercase_copy(src.substr(search_pos));
                const std::string bd_up = uppercase_copy(begin_delim);
                const std::size_t rel = src_up.find(bd_up);
                begin_pos = rel == std::string::npos ? std::string::npos : search_pos + rel;
            } else {
                begin_pos = src.find(begin_delim, search_pos);
            }
            if (begin_pos == std::string::npos) {
                break;
            }
            const std::size_t content_start = begin_pos + begin_delim.size();
            ++found_count;
            if (found_count == occurrence) {
                if (end_delim.empty()) {
                    return make_string_value(src.substr(content_start));
                }
                std::size_t end_pos;
                if (case_insensitive) {
                    const std::string remaining_up = uppercase_copy(src.substr(content_start));
                    const std::string ed_up = uppercase_copy(end_delim);
                    const std::size_t rel = remaining_up.find(ed_up);
                    end_pos = rel == std::string::npos ? std::string::npos : content_start + rel;
                } else {
                    end_pos = src.find(end_delim, content_start);
                }
                if (end_pos == std::string::npos) {
                    return end_delimiter_optional
                               ? make_string_value(include_delimiters ? src.substr(begin_pos) : src.substr(content_start))
                               : make_string_value(std::string{});
                }
                return make_string_value(include_delimiters
                                             ? src.substr(begin_pos, end_pos + end_delim.size() - begin_pos)
                                             : src.substr(content_start, end_pos - content_start));
            }
            search_pos = content_start;
        }
        return make_string_value(std::string{});
    }

    return std::nullopt;
}

}  // namespace copperfin::runtime
