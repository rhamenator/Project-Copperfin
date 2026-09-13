// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

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

// #5946: VFP9's well-known, documented maximum length for a Character
// string (not a memo/general field, which has its own much larger limit)
// is 16,777,184 bytes. Real VFP9 SP2 rejects SPACE(300000000) with error
// 1903 ("String is too long to fit."), confirmed against actual VFP9
// output (retained differential evidence:
// /home/rich/temp/vfp9-probes/space-allocation-74.{prg,out}); the exact
// boundary value itself is well-established VFP9 documentation/community
// knowledge rather than something freshly boundary-probed in this
// session (no VFP9 access was available to test exactly at, one below,
// and one above the limit). This is a data-type-level constraint on any
// Character string, not specific to one function, so the same ceiling
// applies to SPACE() and REPLICATE() alike.
constexpr double kVfpMaxCharacterStringLength = 16'777'184.0;

// #5928/#5956 PR review (chatgpt-codex-connector, P1): a first version of
// this function treated every character across every cParseStringN
// argument as a union/character-class, so a multi-character parse string
// like "xy" would strip a leading lone 'x' even when the complete "xy"
// token isn't actually present (e.g. LTRIM('xHello', 0, 'xy') wrongly
// returned "Hello" instead of leaving "xHello" untouched). Real VFP9
// treats each cParseStringN as a whole removable token: at the requested
// edge, repeatedly try every provided token (in the order given) against
// the current edge and remove the first one that matches completely,
// continuing until no full token matches. This still reproduces every
// verified real VFP9 SP2 vector from the original fix (retained
// differential evidence: /home/rich/temp/vfp9-probes/trim-parse-56.
// {prg,out}) since every one of those tokens happens to be exactly one
// character long, where "whole token" and "character class" degenerate
// to the same behavior -- the two models only diverge for a
// multi-character token, which is exactly the case this review caught.
// Flag 0 or omitted compares case-sensitively, flag 1 case-insensitively.
std::string trim_with_parse_characters(
    const std::string& source,
    bool trim_left,
    bool trim_right,
    const std::vector<PrgValue>& arguments) {
    const bool case_insensitive = arguments.size() >= 2U &&
        static_cast<long long>(std::llround(value_as_number(arguments[1]))) == 1;
    std::vector<std::string> parse_tokens;
    for (std::size_t index = 2U; index < arguments.size(); ++index) {
        std::string token = value_as_string(arguments[index]);
        if (!token.empty()) {
            parse_tokens.push_back(std::move(token));
        }
    }
    const auto tokens_equal = [&](std::string_view a, std::string_view b) {
        if (a.size() != b.size()) {
            return false;
        }
        for (std::size_t index = 0U; index < a.size(); ++index) {
            const bool matches = case_insensitive
                ? std::tolower(static_cast<unsigned char>(a[index])) ==
                      std::tolower(static_cast<unsigned char>(b[index]))
                : a[index] == b[index];
            if (!matches) {
                return false;
            }
        }
        return true;
    };
    std::size_t start = 0U;
    std::size_t end = source.size();
    if (trim_left) {
        for (bool matched = true; matched;) {
            matched = false;
            for (const auto& token : parse_tokens) {
                if (end - start >= token.size() &&
                    tokens_equal(std::string_view(source).substr(start, token.size()), token)) {
                    start += token.size();
                    matched = true;
                    break;
                }
            }
        }
    }
    if (trim_right) {
        for (bool matched = true; matched;) {
            matched = false;
            for (const auto& token : parse_tokens) {
                if (end - start >= token.size() &&
                    tokens_equal(std::string_view(source).substr(end - token.size(), token.size()), token)) {
                    end -= token.size();
                    matched = true;
                    break;
                }
            }
        }
    }
    return source.substr(start, end - start);
}

}  // namespace

std::string format_value_for_display(
    const PrgValue& value,
    const std::function<std::string(const std::string&)>& set_callback) {
    if (value.kind == PrgValueKind::currency) {
        return apply_numeric_picture_symbols(value_as_string(value), true, false, set_callback);
    }
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
        if (arguments.size() >= 3U) {
            return make_string_value(trim_with_parse_characters(
                value_as_string(arguments[0]), true, false, arguments));
        }
        return make_string_value(ltrim_space_copy(value_as_string(arguments[0])));
    }
    if ((function == "rtrim" || function == "trim") && !arguments.empty()) {
        if (arguments.size() >= 3U) {
            return make_string_value(trim_with_parse_characters(
                value_as_string(arguments[0]), false, true, arguments));
        }
        return make_string_value(rtrim_space_copy(value_as_string(arguments[0])));
    }
    if (function == "space" && !arguments.empty()) {
        // #5946: check the finite double directly against the VFP ceiling
        // before ever narrowing to std::size_t -- a huge or nonfinite
        // requested count would otherwise be undefined behavior on the
        // narrowing cast, not just an oversized allocation.
        const double requested = value_as_number(arguments[0]);
        if (!std::isfinite(requested) || requested > kVfpMaxCharacterStringLength) {
            throw PrgCompatibilityError(runtime_text("Runtime.Prg.String.Error.StringTooLong"), 1903);
        }
        const std::size_t n = static_cast<std::size_t>(std::max(0.0, requested));
        return make_string_value(std::string(n, ' '));
    }
    if (function == "replicate" && arguments.size() >= 2U) {
        const std::string src = value_as_string(arguments[0]);
        const double requested_count = value_as_number(arguments[1]);
        if (!std::isfinite(requested_count)) {
            throw PrgCompatibilityError(runtime_text("Runtime.Prg.String.Error.StringTooLong"), 1903);
        }
        // #5946: compare the product in double precision -- src.size() *
        // n could otherwise overflow std::size_t itself for an
        // adversarial huge count, silently wrapping to a small value that
        // would then pass a post-multiplication check.
        if (static_cast<double>(src.size()) * std::max(0.0, requested_count) >
            kVfpMaxCharacterStringLength) {
            throw PrgCompatibilityError(runtime_text("Runtime.Prg.String.Error.StringTooLong"), 1903);
        }
        const std::size_t n = static_cast<std::size_t>(std::max(0.0, requested_count));
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
        if (arguments.size() >= 3U) {
            return make_string_value(trim_with_parse_characters(
                value_as_string(arguments[0]), true, true, arguments));
        }
        return make_string_value(trim_space_copy(value_as_string(arguments[0])));
    }
    if (function == "chr" && !arguments.empty()) {
        // #5900: VFP9 truncates a fractional character-code argument
        // toward zero (CHR(65.9) is 'A', character code 65, not 66) and
        // raises error 11 for any code outside its accepted [0, 255]
        // byte range (confirmed against real VFP9 output for CHR(-1),
        // CHR(256), and CHR(300), all ERR11) rather than narrowing it.
        // A nonfinite argument cannot round-trip through any valid VFP
        // literal or reach here without a prior division-by-zero-style
        // fault, but is treated the same way (out of range -> error 11)
        // for a checked, UB-free conversion rather than an unchecked cast.
        const double truncated_code = std::trunc(value_as_number(arguments[0]));
        if (!std::isfinite(truncated_code) || truncated_code < 0.0 || truncated_code > 255.0) {
            throw PrgCompatibilityError(runtime_text("Runtime.Prg.String.Error.InvalidCharacterCode"), 11);
        }
        return make_string_value(
            std::string(1U, static_cast<char>(static_cast<unsigned char>(truncated_code))));
    }
    if (function == "str" && !arguments.empty()) {
        const int decimals = arguments.size() >= 3U
                                 ? static_cast<int>(std::max(0.0, value_as_number(arguments[2])))
                                 : 0;
        std::ostringstream stream;
        stream.imbue(std::locale::classic());
        stream << std::fixed << std::setprecision(decimals) << value_as_number(arguments[0]);
        std::string result = stream.str();
        // #5900: VFP9 truncates a fractional width argument toward zero
        // (STR(12, 4.9) is 4 characters wide, not 5), the same rule
        // already applied correctly to the decimals argument above.
        //
        // #5947 PR review (chatgpt-codex-connector, P2): an earlier
        // version of this fix saturated an out-of-range width to
        // std::numeric_limits<int>::max() rather than rejecting it, so a
        // call like STR(1, 1e100) reached the result.insert() below with
        // a width around 2^31, attempting to allocate roughly 2 GiB of
        // spaces -- a real, well-defined-but-unbounded-allocation denial
        // of service, not just a narrowing-cast correctness concern.
        // kMaxStrWidth is a conservative safety bound (matching this
        // codebase's own VFP field-width convention -- DBF field lengths
        // are a std::uint8_t, max 255 -- not a specifically VFP9-probed
        // STR()-width limit), well above any realistic legitimate width;
        // anything beyond it is rejected the same way CHR() rejects an
        // out-of-range code, rather than attempting the allocation.
        constexpr double kMaxStrWidth = 255.0;
        const double truncated_width = arguments.size() >= 2U
                                           ? std::trunc(value_as_number(arguments[1]))
                                           : 10.0;
        if (std::isfinite(truncated_width) && truncated_width > kMaxStrWidth) {
            throw PrgCompatibilityError(runtime_text("Runtime.Prg.String.Error.InvalidStrWidth"), 11);
        }
        // Every width <= 0 takes the same "no padding/truncation" path
        // below regardless of exact magnitude, so an out-of-int-range
        // negative value (equally capable of triggering the same class of
        // narrowing-cast UB as the too-large case above) is clamped to -1
        // rather than rejected -- behavior-preserving, not just safety-
        // preserving, since it was already indistinguishable from any
        // other negative width before this fix.
        const int width = !std::isfinite(truncated_width)
                              ? 0
                              : truncated_width < 0.0
                                    ? -1
                                    : static_cast<int>(truncated_width);
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
