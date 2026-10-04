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
// kVfpMaxCharacterStringLength and checked_character_string_length() live in prg_engine_helpers.h.

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
        rounded_numeric_to_int64(value_as_number(arguments[1])) == 1;
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

// #5951/#5611/#6776: shared occurrence-argument validation for
// AT()/ATC()/ATCC()/RAT()/RATC(). Real VFP9 SP2 raises error 11 for a
// sub-unit, nonpositive, non-finite, or oversized occurrence rather than
// clamping or saturating it. The upper bound is the inclusive VFP Character
// string ceiling, 16,777,184. Retained differential evidence:
// ~/temp/vfp9-probes/at-occurrence-boundary-82.{prg,out},
// atcc-occurrence-boundary-83.{prg,out}, numconv-6776/probe1.txt, and
// review-6918/{out4,out5}.txt.
std::size_t require_valid_occurrence_argument(const PrgValue& argument) {
    const double requested_occurrence = value_as_number(argument);
    const auto occurrence = checked_truncated_numeric_to_int64(requested_occurrence);
    if (!occurrence.has_value() || *occurrence <= 0 ||
        *occurrence > static_cast<std::int64_t>(kVfpMaxCharacterStringLength)) {
        throw PrgCompatibilityError(runtime_text("Runtime.Prg.String.Error.InvalidOccurrence"), 11);
    }
    return static_cast<std::size_t>(*occurrence);
}

// STRTRAN uses the same fail-closed range, with -1 as its documented
// "all occurrences" sentinel. Installed VFP9 additionally converts
// 2^32-1 (and equivalent low-32-bit values) to -1. That quirk is confined to
// NUMERICBEHAVIOR VFP9; COPPERFIN rejects the out-of-range source value.
std::int64_t require_valid_strtran_occurrence_argument(
    const PrgValue& argument,
    const NumericBehavior behavior) {
    const double requested_occurrence = value_as_number(argument);
    if (!std::isfinite(requested_occurrence) || requested_occurrence < -1.0) {
        throw PrgCompatibilityError(runtime_text("Runtime.Prg.String.Error.InvalidOccurrence"), 11);
    }
    if (behavior == NumericBehavior::vfp9 &&
        requested_occurrence > static_cast<double>(std::numeric_limits<int>::max()) &&
        vfp9_numeric_to_int32(requested_occurrence) == -1) {
        return -1;
    }
    const auto occurrence = checked_truncated_numeric_to_int64(requested_occurrence);
    if (!occurrence.has_value() || *occurrence == 0 || *occurrence < -1 ||
        *occurrence > static_cast<std::int64_t>(kVfpMaxCharacterStringLength)) {
        throw PrgCompatibilityError(runtime_text("Runtime.Prg.String.Error.InvalidOccurrence"), 11);
    }
    return *occurrence;
}

}  // namespace

std::string format_value_for_display(
    const PrgValue& value,
    const std::function<std::string(const std::string&)>& set_callback) {
    if (value.kind == PrgValueKind::currency) {
        return apply_numeric_picture_symbols(value_as_string(value), true, false, set_callback);
    }
    if (value.kind != PrgValueKind::number) {
        return value_as_string(value);
    }

    int decimals = 2;
    try {
        decimals = std::clamp(std::stoi(trim_copy(set_callback("DECIMALS"))), 0, 18);
    } catch (...) {
    }

    if (!std::isfinite(value.number_value)) {
        // #6144: real VFP9 SP2 never exposes its own non-finite internal
        // representation (IEEE infinity/NaN) as literal text -- it keeps
        // the VFP Numeric result contract and fills the value's default
        // (pictureless) display width with asterisks, the same
        // convention format_digit_only_numeric_picture() already applies
        // for an explicit digit-only picture. Real VFP9 SP2's default
        // (unconstrained) Numeric-to-Character width is 9 integer digits
        // + the active SET DECIMALS count + 1 for the decimal point + 1
        // reserved for a sign -- e.g. 13 under DECIMALS=2 (confirmed
        // against actual VFP9 output for TRANSFORM(EXP(1000)) assigned
        // to a variable, retained differential evidence:
        // ~/temp/vfp9-probes/nonfinite-format-6143a/{main.prg,result.out}).
        //
        // Real VFP9 SP2 uses a DIFFERENT, much wider default width for
        // overflow produced directly by an inline arithmetic expression
        // (TRANSFORM(1E+308 * 1E+308) and TRANSFORM(1E+308 + 1E+308)
        // both use 40, not 13 -- retained differential evidence:
        // ~/temp/vfp9-probes/nonfinite-results-11.{prg,out}), which
        // reflects VFP9's own numeric-expression width-propagation rules
        // (tracked per computed value, not derived solely from SET
        // DECIMALS). Copperfin's PrgValue carries no width/decimals
        // metadata at all, so reproducing that propagation would require
        // threading new width/decimals tracking through the arithmetic
        // engine -- a materially larger, deliberately deferred
        // architectural change, disclosed explicitly rather than
        // guessed at. This fix only targets the always-reachable,
        // evidenced default-variable-width case above and
        // unconditionally eliminates the "inf"/"-inf"/"nan" leak; it
        // does not claim expression-width parity for every arithmetic
        // overflow shape.
        constexpr int kDefaultIntegerDigits = 9;
        const std::size_t width = static_cast<std::size_t>(kDefaultIntegerDigits + decimals) + 2U;
        return std::string(width, '*');
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
    if ((function == "left" || function == "right" || function == "leftc" || function == "rightc") &&
        arguments.size() >= 2U) {
        // #6776: the count is converted under SET NUMERICBEHAVIOR (Copperfin: saturating; VFP9: its 32-bit
        // conversion, so e.g. LEFT('abc', 1E20) is '' there but 'abc' here). A count of zero or less is empty.
        const std::string src = value_as_string(arguments[0]);
        const std::int64_t count = numeric_count_argument(value_as_number(arguments[1]), numeric_behavior(set_callback));
        const std::uint64_t wanted = count <= 0 ? 0U : static_cast<std::uint64_t>(count);
        if (function == "left") {
            return make_string_value(src.substr(0U, static_cast<std::size_t>(std::min<std::uint64_t>(wanted, src.size()))));
        }
        if (function == "right") {
            return make_string_value(wanted >= src.size() ? src : src.substr(src.size() - static_cast<std::size_t>(wanted)));
        }
        const std::size_t scalar_count = utf8_scalar_offsets_local(src).size() - 1U;
        const std::size_t n = static_cast<std::size_t>(std::min<std::uint64_t>(wanted, scalar_count));
        if (function == "leftc") {
            return make_string_value(utf8_scalar_slice_local(src, 1U, n));
        }
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
        // #5946/#5968: the ceiling is checked on the truncated-toward-zero value that the size_t conversion would
        // produce (16777184.5 is allowed), before any narrowing, so a huge or nonfinite count stays defined.
        const auto length = checked_character_string_length(value_as_number(arguments[0]));
        if (!length.has_value()) {
            throw PrgCompatibilityError(runtime_text("Runtime.Prg.String.Error.StringTooLong"), 1903);
        }
        return make_string_value(std::string(*length, ' '));
    }
    if (function == "replicate" && arguments.size() >= 2U) {
        const std::string src = value_as_string(arguments[0]);
        const double requested_count = value_as_number(arguments[1]);
        if (!std::isfinite(requested_count)) {
            throw PrgCompatibilityError(runtime_text("Runtime.Prg.String.Error.StringTooLong"), 1903);
        }
        // #5946/#5968 PR review (chatgpt-codex-connector, P2): same
        // truncate-before-compare fix as SPACE() above -- compare the
        // product against the truncated count that will actually be
        // used, not the raw fractional double. Still compared in double
        // precision -- src.size() * n could otherwise overflow
        // std::size_t itself for an adversarial huge count, silently
        // wrapping to a small value that would then pass a
        // post-multiplication check.
        const double truncated_count = std::trunc(std::max(0.0, requested_count));
        if (src.empty() || truncated_count == 0.0) {
            // #5595: nothing to repeat, so do not iterate `count` times (REPLICATE('', 9007199254740992) used to
            // spin for ~2^53 iterations) and do not reserve anything.
            return make_string_value(std::string{});
        }
        if (static_cast<double>(src.size()) * truncated_count > kVfpMaxCharacterStringLength) {
            throw PrgCompatibilityError(runtime_text("Runtime.Prg.String.Error.StringTooLong"), 1903);
        }
        const std::size_t n = static_cast<std::size_t>(truncated_count);
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
        // #5952/#5611/#6776: STRTRAN()'s nStartOccurrence and nCount arguments both
        // treat zero as invalid (real VFP9 SP2 raises catchable error 11
        // for STRTRAN('aaa','a','x',0) and STRTRAN('aaa','a','x',1,0)),
        // while a negative value is a distinct, valid sentinel meaning
        // "from/for all occurrences" (STRTRAN('aaa','a','x',-1) and
        // STRTRAN('aaa','a','x',1,-1) both succeed) -- confirmed against
        // actual VFP9 output (retained differential evidence:
        // /home/rich/temp/vfp9-probes/strtran-boundary-84.{prg,out}). A
        // Fresh installed-VFP9 boundary evidence additionally establishes
        // that sub-unit, values below the -1 sentinel, non-finite, and
        // oversized controls are error 11. The 2^32-1 -> -1 wrap is exposed
        // only under NUMERICBEHAVIOR VFP9.
        const NumericBehavior occurrence_behavior = numeric_behavior(set_callback);
        std::size_t start_occurrence = 1U;
        if (arguments.size() >= 4U) {
            const std::int64_t converted_start =
                require_valid_strtran_occurrence_argument(arguments[3], occurrence_behavior);
            start_occurrence = converted_start == -1 ? 1U : static_cast<std::size_t>(converted_start);
        }
        std::size_t occurrence_limit = std::numeric_limits<std::size_t>::max();
        if (arguments.size() >= 5U) {
            const std::int64_t converted_limit =
                require_valid_strtran_occurrence_argument(arguments[4], occurrence_behavior);
            occurrence_limit = converted_limit == -1 ? std::numeric_limits<std::size_t>::max()
                                                      : static_cast<std::size_t>(converted_limit);
        }
        const std::size_t flags = arguments.size() >= 6U
                                      ? saturating_size_argument(value_as_number(arguments[5]))
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
        // #3704: installed VFP9 SP2 (probes ~/temp/vfp9-probes/stuff-boundaries-6145c and numconv-6776/probe2.txt)
        // treats a start of 1 or less as 1 WITHOUT discarding the replacement count (STUFF('abcdef',0,2,'X') is
        // 'Xcdef'), appends when the start is past the end (start 8 or 1E20 both give 'abcdefX'), and inserts when the
        // count is 0 or less. The earlier "start <= 0 inserts only" rule came from documentation, not from VFP9.
        const std::size_t start = std::min(saturating_size_argument(value_as_number(arguments[1]), 1U) - 1U, src.size());
        const std::size_t length = saturating_size_argument(value_as_number(arguments[2]));
        const std::string replacement = value_as_string(arguments[3]);
        src.replace(start, std::min(length, src.size() - start), replacement);
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
        // #5954: VAL() consults the active SET POINT character as its
        // decimal separator rather than hard-coding '.', so localized
        // numeric text (e.g. after SET POINT TO ',') parses with the
        // configured separator and stops at the alternate character,
        // matching real VFP9 SP2 (confirmed against actual VFP9 output,
        // retained differential evidence:
        // /home/rich/temp/vfp9-probes/val-set-point-89.{prg,out}).
        const std::string point_symbol = set_symbol(set_callback, "POINT", ".");
        const char decimal_point = point_symbol.empty() ? '.' : point_symbol.front();

        const std::size_t integer_start = numeric_end;
        while (numeric_end < src.size() && std::isdigit(static_cast<unsigned char>(src[numeric_end]))) {
            ++numeric_end;
        }
        const bool has_integer_digits = numeric_end > integer_start;
        const std::size_t integer_digits_end = numeric_end;

        // #5953: a decimal point followed by at least one digit is a
        // valid numeric token even when the integer portion is omitted
        // (e.g. VAL('.5')). Real VFP9 SP2 parses this as 0.5 rather than
        // silently returning 0 (confirmed against actual VFP9 output,
        // retained differential evidence:
        // /home/rich/temp/vfp9-probes/val-leading-decimal-87.{prg,out}).
        bool has_fraction_digits = false;
        std::size_t fraction_digits_start = 0;
        std::size_t fraction_digits_end = 0;
        if (numeric_end < src.size() && src[numeric_end] == decimal_point) {
            fraction_digits_start = numeric_end + 1U;
            fraction_digits_end = fraction_digits_start;
            while (fraction_digits_end < src.size() && std::isdigit(static_cast<unsigned char>(src[fraction_digits_end]))) {
                ++fraction_digits_end;
            }
            has_fraction_digits = fraction_digits_end > fraction_digits_start;
            if (has_integer_digits || has_fraction_digits) {
                numeric_end = fraction_digits_end;
            }
        }
        if (!has_integer_digits && !has_fraction_digits) {
            return currency ? make_currency_value(0) : make_number_value(0.0);
        }
        // #6146: capture the explicit exponent's sign and digit span (if
        // any) so a parse failure below (outside the full IEEE-754
        // double range) can be classified as overflow versus underflow
        // by combining it with the mantissa's own significant-digit
        // place value -- see the effective-exponent computation below.
        const std::size_t mantissa_end = numeric_end;
        bool has_explicit_exponent = false;
        bool exponent_is_negative = false;
        std::size_t exponent_digits_start = 0;
        std::size_t exponent_digits_end = 0;
        if (numeric_end < src.size() && (src[numeric_end] == 'E' || src[numeric_end] == 'e')) {
            const std::size_t exponent_start = numeric_end;
            ++numeric_end;
            if (numeric_end < src.size() && (src[numeric_end] == '+' || src[numeric_end] == '-')) {
                exponent_is_negative = src[numeric_end] == '-';
                ++numeric_end;
            }
            exponent_digits_start = numeric_end;
            while (numeric_end < src.size() && std::isdigit(static_cast<unsigned char>(src[numeric_end]))) {
                ++numeric_end;
            }
            exponent_digits_end = numeric_end;
            has_explicit_exponent = exponent_digits_end > exponent_digits_start;
            if (!has_explicit_exponent) {
                numeric_end = exponent_start;
                exponent_is_negative = false;
            }
        }
        std::string numeric_text = src.substr(numeric_start, numeric_end - numeric_start);
        if (decimal_point != '.') {
            // parse_currency_scaled_value()/try_parse_invariant_double()
            // both expect an invariant '.' decimal separator; the scan
            // above already used the configured SET POINT character to
            // find the boundary, so normalize it before delegating.
            const auto point_pos = numeric_text.find(decimal_point);
            if (point_pos != std::string::npos) {
                numeric_text[point_pos] = '.';
            }
        }
        if (currency) {
            // #6146: parse_currency_scaled_value() already detects
            // Currency's own int64-scaled-by-10000 range overflow and
            // returns std::nullopt, but the caller silently mapped that
            // to a Currency value of 0 via .value_or(0) -- the same
            // silent-overflow-to-zero pattern reported for the Double
            // path below, just for Currency's own (much narrower)
            // range. Wired to the same error instead.
            //
            // #6728: installed VFP9 (probe ~/temp/vfp9-probes/currency-c25/result12.txt) reports a dollar-prefixed
            // VAL outside +-922337203685477.5807 as error 1988 "Currency value is out of range.", after rounding
            // half away from zero to four decimals, and ignores an exponent ('$1E3' is $1). The exact decimal
            // parser also refuses the stored minimum, which a source value may not use.
            std::string mantissa_text = src.substr(numeric_start, mantissa_end - numeric_start);
            if (const auto point = mantissa_text.find(decimal_point); decimal_point != '.' && point != std::string::npos) {
                mantissa_text[point] = '.';
            }
            const CurrencyDecimal parsed = parse_currency_decimal(mantissa_text);
            if (parsed.status == CurrencyDecimalStatus::out_of_range) {
                throw PrgCompatibilityError(runtime_text("Runtime.Prg.Expression.Error.CurrencyOutOfRange"), 1988);
            }
            return make_currency_value(parsed.status == CurrencyDecimalStatus::ok ? parsed.scaled : 0);
        }
        // #6146: real VFP9 SP2 raises catchable error 39 ("Numeric
        // overflow.") for a VAL() input whose magnitude exceeds VFP9's
        // own numeric ceiling -- distinct from, and narrower than, the
        // full IEEE-754 double range. VFP9 accepts 1E307 but rejects
        // 1E308, 1.7E308, 1.8E308, and any larger exponent (1E309,
        // 1E999) with error 39; it treats underflow (e.g. 1E-999) as
        // zero, not an error (confirmed against actual VFP9 output,
        // retained differential evidence:
        // ~/temp/vfp9-probes/val-overflow-threshold-1789396768987627394/
        // {main.prg,vfp.out} and
        // val-boundaries-1789396712129717159/{main.prg,vfp.out}). The
        // prior implementation collapsed every std::from_chars range
        // failure -- overflow and underflow alike -- to a silently
        // returned 0.0 via .value_or(0.0), and never checked
        // in-IEEE-range values (1E307..1.7E308) against VFP9's own
        // narrower ceiling at all. The exact ceiling constant is
        // well-established VFP9 documentation/community knowledge
        // (Numeric/Double range), not independently boundary-probed at
        // finer-than-power-of-ten granularity in this session.
        constexpr double kVfpMaxNumericMagnitude = 9.999999999999999e+307;
        const auto parsed = try_parse_invariant_double(numeric_text);
        if (parsed.has_value()) {
            if (std::fabs(*parsed) > kVfpMaxNumericMagnitude) {
                throw PrgCompatibilityError(runtime_text("Runtime.Prg.String.Error.NumericOverflow"), 39);
            }
            return make_number_value(*parsed);
        }
        // numeric_text is guaranteed syntactically valid by the scan
        // above, so a parse failure here can only be an IEEE-double
        // range failure: overflow (huge magnitude) or underflow
        // (magnitude too small to represent, including subnormals).
        // #6146 review: the explicit exponent's sign alone is NOT
        // sufficient to classify this -- a long run of significant
        // digits (e.g. 400 nines) combined with a small/negative
        // explicit exponent can still overflow, and a long run of
        // leading fraction zeros combined with a small/positive
        // explicit exponent can still underflow. Instead, find the
        // first significant (non-zero) digit in the mantissa as
        // literally written and compute ITS base-10 place value, then
        // add the explicit exponent to get the token's true effective
        // exponent.
        std::size_t first_significant_place = 0;
        bool has_significant_digit = false;
        for (std::size_t i = integer_start; i < integer_digits_end; ++i) {
            if (src[i] != '0') {
                first_significant_place = integer_digits_end - 1U - i;
                has_significant_digit = true;
                break;
            }
        }
        long long effective_exponent = 0;
        if (has_significant_digit) {
            effective_exponent = static_cast<long long>(first_significant_place);
        } else if (has_fraction_digits) {
            for (std::size_t i = fraction_digits_start; i < fraction_digits_end; ++i) {
                if (src[i] != '0') {
                    effective_exponent = -(static_cast<long long>(i - fraction_digits_start) + 1);
                    has_significant_digit = true;
                    break;
                }
            }
        }
        if (!has_significant_digit) {
            // Every mantissa digit is zero (e.g. "0.000...0E999"): the
            // value is exactly zero regardless of the exponent, not an
            // overflow or underflow.
            return make_number_value(0.0);
        }
        if (has_explicit_exponent) {
            // Saturate rather than overflow this accumulator for a
            // pathologically long exponent digit run -- any value this
            // large already puts the effective exponent far outside
            // both the overflow and underflow thresholds either way.
            long long explicit_exponent = 0;
            for (std::size_t i = exponent_digits_start; i < exponent_digits_end && explicit_exponent < 1'000'000; ++i) {
                explicit_exponent = explicit_exponent * 10 + (src[i] - '0');
            }
            if (explicit_exponent > 1'000'000) {
                explicit_exponent = 1'000'000;
            }
            effective_exponent += exponent_is_negative ? -explicit_exponent : explicit_exponent;
        }
        if (effective_exponent >= 0) {
            throw PrgCompatibilityError(runtime_text("Runtime.Prg.String.Error.NumericOverflow"), 39);
        }
        return make_number_value(0.0);
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
        // #6003: installed VFP9 raises error 11 for a width beyond the maximum Character string length (PADL, PADR
        // and PADC('x', 20000000) all give ERR11). A fractional width truncates, a negative one is 0.
        const auto checked_width = checked_character_string_length(value_as_number(arguments[1]));
        if (!checked_width.has_value()) {
            throw PrgCompatibilityError(runtime_text("Runtime.Prg.Expression.Error.InvalidArgument"), 11);
        }
        const std::size_t width = *checked_width;
        const char pad_char = (arguments.size() >= 3U && !value_as_string(arguments[2]).empty())
                                  ? value_as_string(arguments[2])[0]
                                  : ' ';
        if (src.size() > width) {
            // #5948: real VFP9 SP2 truncates an over-length source to its
            // leftmost `width` characters for PADL(), PADR(), and PADC()
            // alike, regardless of which side each function pads on when
            // the source is too short. Confirmed against actual VFP9
            // output for all three (retained differential evidence:
            // /home/rich/temp/vfp9-probes/pad-contract-78.{prg,out}):
            // PADL('abcdef',3), PADR('abcdef',3), and PADC('abcdef',3)
            // all return "abc". The prior implementation only got
            // PADR() right by coincidence -- PADL() kept the *rightmost*
            // width characters (a plausible-seeming but wrong mirror of
            // its padding side) and PADC() centered the clip -- neither
            // matches VFP9's actual, side-independent leftmost-retention
            // rule.
            src = src.substr(0U, width);
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
        const std::string src = value_as_string(arguments[0]);
        const std::string from_chars = value_as_string(arguments[1]);
        const std::string to_chars = value_as_string(arguments[2]);
        std::string result;
        result.reserve(src.size());
        for (const char c : src) {
            const auto pos = from_chars.find(c);
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
        const int mode = static_cast<int>(std::clamp<std::int64_t>(
            rounded_numeric_to_int64(value_as_number(arguments[1])),
            std::numeric_limits<int>::min(), std::numeric_limits<int>::max()));
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
        // Three-valued, as installed VFP9 (retained probe ~/temp/vfp9-probes/null-semantics-c24, #5979):
        // a NULL search value is NULL; otherwise any match is .T. even beside a NULL element, and
        // with no match a NULL element makes the result NULL rather than .F.
        if (arguments[0].is_null) {
            return make_null_value();
        }
        bool saw_null = false;
        for (std::size_t index = 1U; index < arguments.size(); ++index) {
            if (arguments[index].is_null) {
                saw_null = true;
                continue;
            }
            if (expression_values_equal(arguments[0], arguments[index], exact_string_compare)) {
                return make_boolean_value(true);
            }
        }
        return saw_null ? make_null_value() : make_boolean_value(false);
    }
    if ((function == "getwordcount" || function == "getwordnum") && !arguments.empty()) {
        const std::string src = value_as_string(arguments[0]);
        const std::string delim = (arguments.size() >= (function == "getwordcount" ? 2U : 3U))
                                      ? value_as_string(arguments[function == "getwordcount" ? 1U : 2U])
                                      : std::string{" \t\r\n"};
        if (delim.empty()) {
            // #5949: real VFP9 SP2 counts a nonempty source as exactly
            // one word and an empty source as zero words when the
            // delimiter is empty (GETWORDCOUNT('','') is 0,
            // GETWORDCOUNT('abc','') is 1); for GETWORDNUM(), only word
            // index 1 of a nonempty source returns the source -- zero,
            // negative, and any index other than 1 return an empty
            // string, matching GETWORDNUM('','',1)'s own empty result
            // (confirmed against actual VFP9 output, retained
            // differential evidence:
            // /home/rich/temp/vfp9-probes/getword-empty-delimiter-80.
            // {prg,out} and -81.{prg,out}). The prior implementation
            // counted an empty source as one word and returned the
            // source unconditionally for GETWORDNUM() regardless of the
            // requested index. A fractional index truncates toward zero,
            // matching this file's established convention for other
            // count/index arguments.
            if (function == "getwordcount") {
                return make_number_value(src.empty() ? 0.0 : 1.0);
            }
            const double truncated_index = std::trunc(value_as_number(arguments[1]));
            return make_string_value(!src.empty() && truncated_index == 1.0 ? src : std::string{});
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
        // The index is truncated toward zero; 0, a negative value and NaN are empty (VFP9 returns an empty string for
        // GETWORDNUM('a b c', 0.5), which previously indexed words[SIZE_MAX]).
        const std::size_t n = saturating_size_argument(value_as_number(arguments[1]));
        if (n == 0U) {
            return make_string_value(std::string{});
        }
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
        const std::size_t line_index = saturating_size_argument(value_as_number(arguments[1]));
        if (line_index == 0U) {
            return make_string_value(std::string{});
        }
        const std::size_t start = arguments.size() >= 3U
                                      ? saturating_size_argument(value_as_number(arguments[2]))
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
        return make_string_value(line_index <= lines.size() ? lines[line_index - 1U] : std::string{});
    }
    if ((function == "at_c" || function == "atc" || function == "atcc") && arguments.size() >= 2U) {
        const std::size_t occurrence = arguments.size() >= 3U
                                           ? require_valid_occurrence_argument(arguments[2])
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
                                           ? require_valid_occurrence_argument(arguments[2])
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
                                           ? require_valid_occurrence_argument(arguments[2])
                                           : 1U;
        return make_number_value(static_cast<double>(find_utf8_scalar_occurrence_local(
            value_as_string(arguments[0]), value_as_string(arguments[1]), occurrence, true)));
    }
    if (function == "rat" && arguments.size() >= 2U) {
        std::string needle = value_as_string(arguments[0]);
        std::string haystack = value_as_string(arguments[1]);
        const std::size_t occurrence = arguments.size() >= 3U
                                           ? require_valid_occurrence_argument(arguments[2])
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
                                           ? saturating_size_argument(value_as_number(arguments[2]), 1U)
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
        // VFP9 returns an empty string for a start below 1 after truncation (SUBSTR('abcdef',0) and 0.5 are '', 1.9 is
        // the whole string), which is what makes SUBSTR(s, AT(x, s), n) with no match harmless.
        const double raw_start = value_as_number(arguments[1]);
        if (saturating_numeric_to_int64(raw_start) < 1) {
            return make_string_value(std::string{});
        }
        std::size_t start = saturating_size_argument(raw_start) - 1U;
        // #5611/#6776: installed VFP9 takes an operation-specific path for a
        // positive start beyond INT_MAX. It first retains the low signed 32
        // bits, then returns the final byte when that value is at or before
        // the source end (including zero/negative wrap); a wrapped value past
        // the end remains empty. COPPERFIN deliberately keeps the safe,
        // saturating behavior. Retained evidence:
        // ~/temp/vfp9-probes/numconv-6776/{probe1.txt,substr-wrap-probe.prg}.
        if (numeric_behavior(set_callback) == NumericBehavior::vfp9 &&
            raw_start > static_cast<double>(std::numeric_limits<std::int32_t>::max()) && !source.empty()) {
            const std::int64_t wrapped_start = vfp9_numeric_to_int32(raw_start);
            const bool at_or_before_end =
                wrapped_start <= 0 || static_cast<std::size_t>(wrapped_start) <= source.size();
            start = at_or_before_end ? source.size() - 1U : source.size();
        }
        const std::size_t length = arguments.size() >= 3U
                                       ? saturating_size_argument(value_as_number(arguments[2]))
                                       : std::string::npos;
        return make_string_value(start >= source.size() ? std::string{} : source.substr(start, length));
    }
    if (function == "substrc" && arguments.size() >= 2U) {
        const std::string source = value_as_string(arguments[0]);
        const double raw_start = value_as_number(arguments[1]);
        if (saturating_numeric_to_int64(raw_start) < 1) {
            return make_string_value(std::string{});  // as SUBSTR(): a start below 1 is empty in VFP9
        }
        std::size_t start = saturating_size_argument(raw_start, 1U);
        // Match SUBSTR's verified huge-positive VFP9 wrap/clamp quirk, but
        // apply the end test and final-position clamp in Unicode scalar units.
        if (numeric_behavior(set_callback) == NumericBehavior::vfp9 &&
            raw_start > static_cast<double>(std::numeric_limits<std::int32_t>::max()) && !source.empty()) {
            const std::size_t scalar_count = utf8_scalar_offsets_local(source).size() - 1U;
            const std::int64_t wrapped_start = vfp9_numeric_to_int32(raw_start);
            const bool at_or_before_end =
                wrapped_start <= 0 || static_cast<std::size_t>(wrapped_start) <= scalar_count;
            start = at_or_before_end ? scalar_count : scalar_count + 1U;
        }
        const std::size_t length = arguments.size() >= 3U
                                       ? saturating_size_argument(value_as_number(arguments[2]))
                                       : std::numeric_limits<std::size_t>::max();
        return make_string_value(utf8_scalar_slice_local(source, start, length));
    }
    if (function == "stuffc" && arguments.size() >= 4U) {
        // As STUFF() (#3704): VFP9 treats a start below 1 as 1 without dropping the count (probe
        // numconv-6776/probe5.txt: STUFFC('abcdef',0,2,'X') is 'Xcdef'), appends past the end, and inserts when the
        // count is 0 or less.
        const std::string source = value_as_string(arguments[0]);
        const std::size_t scalar_count = utf8_scalar_offsets_local(source).size() - 1U;
        const std::size_t start = std::min(saturating_size_argument(value_as_number(arguments[1]), 1U), scalar_count + 1U);
        const std::size_t length = saturating_size_argument(value_as_number(arguments[2]));
        return make_string_value(replace_utf8_scalars_local(source, start, length, value_as_string(arguments[3])));
    }
    if (function == "alltrim" && !arguments.empty()) {
        std::string result;
        if (arguments.size() >= 3U) {
            result = trim_with_parse_characters(
                value_as_string(arguments[0]), true, true, arguments);
        } else {
            result = trim_space_copy(value_as_string(arguments[0]));
        }
        if (arguments[0].is_object_reference && result == arguments[0].string_value) {
            return make_object_reference_value(std::move(result));
        }
        return make_string_value(std::move(result));
    }
    if (function == "chr" && !arguments.empty()) {
        // Governing requirement: RQ-CF-PRG-CHR-BOUNDARIES-001 (#5611/#6776).
        // Ordinary fractional arguments truncate toward zero. COPPERFIN validates the raw truncated value against
        // the byte range. Installed VFP9 first rejects a positive value above 255, but converts negative values
        // through its signed-32-bit path before applying the byte check: negative multiples of 2^32 (and negative
        // integer-indefinite values) therefore become NUL, while -4294967295 becomes byte 1. Keep that compatibility
        // quirk behind SET NUMERICBEHAVIOR; every conversion here is defined and free of out-of-range C++ casts.
        const double raw_code = value_as_number(arguments[0]);
        const double truncated_code = std::trunc(raw_code);
        std::int64_t character_code = 0;
        if (numeric_behavior(set_callback) == NumericBehavior::vfp9) {
            character_code = vfp9_numeric_to_int32(truncated_code);
            if (truncated_code > 255.0 || character_code < 0 || character_code > 255) {
                throw PrgCompatibilityError(runtime_text("Runtime.Prg.String.Error.InvalidCharacterCode"), 11);
            }
        } else if (!std::isfinite(truncated_code) || truncated_code < 0.0 || truncated_code > 255.0) {
            throw PrgCompatibilityError(runtime_text("Runtime.Prg.String.Error.InvalidCharacterCode"), 11);
        } else {
            character_code = static_cast<std::int64_t>(truncated_code);
        }
        return make_string_value(
            std::string(1U, static_cast<char>(static_cast<unsigned char>(character_code))));
    }
    if (function == "str" && !arguments.empty()) {
        // Governing requirement: RQ-CF-PRG-STR-ARGUMENT-BOUNDS-001 (#5611/#6776).
        // Installed VFP9 accepts widths 0..237 and decimal counts 0..18 after truncation, and raises error 1908
        // outside those ranges. Its negative out-of-int32 conversion still has the shared low-32-bit quirk: for
        // example -4294967295 becomes 1 and -4294967296 becomes 0. Keep that compatibility behind
        // SET NUMERICBEHAVIOR; COPPERFIN mode rejects every non-finite or out-of-range raw truncated value.
        const NumericBehavior behavior = numeric_behavior(set_callback);
        const auto str_argument = [&](const PrgValue& argument, const int maximum) {
            const double raw_value = value_as_number(argument);
            const double truncated = std::trunc(raw_value);
            std::int64_t converted = 0;
            if (behavior == NumericBehavior::vfp9) {
                // Positive infinity follows the above-maximum error path. Negative infinity and NaN reproduce the
                // shared VFP9 integer-indefinite low bits (0), just like finite values outside signed-int64 range.
                if (truncated > static_cast<double>(maximum)) {
                    throw PrgCompatibilityError(runtime_text("Runtime.Prg.String.Error.InvalidStrWidth"), 1908);
                }
                converted = vfp9_numeric_to_int32(argument);
            } else {
                if (!std::isfinite(truncated) || truncated < 0.0 ||
                    truncated > static_cast<double>(maximum)) {
                    throw PrgCompatibilityError(runtime_text("Runtime.Prg.String.Error.InvalidStrWidth"), 1908);
                }
                converted = static_cast<std::int64_t>(truncated);
            }
            if (converted < 0 || converted > maximum) {
                throw PrgCompatibilityError(runtime_text("Runtime.Prg.String.Error.InvalidStrWidth"), 1908);
            }
            return static_cast<int>(converted);
        };

        int decimals = 0;
        if (arguments.size() >= 3U) {
            decimals = str_argument(arguments[2], 18);
        }
        // Validate the required numeric expression even when a zero width will suppress all output.
        const double numeric_value = value_as_number(arguments[0]);
        // #5900: VFP9 truncates a fractional width argument toward zero
        // (STR(12, 4.9) is 4 characters wide, not 5), the same rule
        // already applied correctly to the decimals argument above.
        //
        // #5947: reject before padding so no width can reach an unbounded allocation. The exact installed-VFP9
        // ceiling is 237, not the earlier conservative DBF-derived 255 limit.
        const int width = arguments.size() >= 2U
                              ? str_argument(arguments[1], 237)
                              : 10;
        if (width == 0) {
            return make_string_value("");
        }
        std::ostringstream stream;
        stream.imbue(std::locale::classic());
        stream << std::fixed << std::setprecision(decimals) << numeric_value;
        std::string result = stream.str();
        if (!std::isfinite(numeric_value)) {
            // #6144: real VFP9 SP2 never exposes the C++ stream spelling
            // of a non-finite double ("inf"/"-inf"/"nan") -- it fills
            // the requested (or default 10-character) width with
            // asterisks instead, the same convention STR() already
            // applies when an ordinary too-large finite value overflows
            // its width below (confirmed against actual VFP9 output for
            // STR(EXP(1000)), retained differential evidence:
            // ~/temp/vfp9-probes/nonfinite-format-6143a/{main.prg,result.out}).
            const std::size_t fill_width = width > 0 ? static_cast<std::size_t>(width) : 10U;
            return make_string_value(std::string(fill_width, '*'));
        }
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
            } else if (picture_is_dollar_digit_only(numeric_picture_template_mask(picture)) &&
                       (arguments[0].kind == PrgValueKind::number || arguments[0].kind == PrgValueKind::int64 ||
                        arguments[0].kind == PrgValueKind::uint64 || arguments[0].kind == PrgValueKind::currency)) {
                transformed = format_dollar_digit_only_picture(
                    arguments[0],
                    numeric_picture_template_mask(picture));
            } else {
                const std::size_t decimal_pos = picture.find('.');
                if (decimal_pos != std::string::npos ||
                    (picture.find(',') != std::string::npos && picture_has_numeric_placeholders(picture))) {
                    if (!std::isfinite(value_as_number(arguments[0]))) {
                        // #6144: same "never leak inf/-inf/nan" policy
                        // already verified for a bare digit-only picture
                        // (format_digit_only_numeric_picture()), extended
                        // to the symbol-picture shapes this branch already
                        // recognizes (a numeric grouping comma and/or a
                        // decimal point, optionally with a currency sign)
                        // by filling the template width with asterisks
                        // rather than streaming the raw double into
                        // apply_numeric_picture_symbols(). Not
                        // independently VFP9-probed for a symbol picture
                        // specifically (only the digit-only and
                        // pictureless cases have retained differential
                        // evidence); this is a same-mechanism
                        // extrapolation, disclosed as such.
                        //
                        // #6144 PR review (chatgpt-codex-connector, P2):
                        // a leading function-code prefix (e.g. "@B ")
                        // is not part of the rendered field and must be
                        // excluded from the fill width, or a picture
                        // like "@B 999,999.99" would wrongly asterisk-
                        // fill the whole "@B 999,999.99" token instead
                        // of just its "999,999.99" template mask.
                        transformed = std::string(numeric_picture_template_mask(picture).size(), '*');
                    } else {
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
        }

        if (transformed.empty() && !(picture_has_flag(picture, "@Z") && is_zeroish_transform_value(arguments[0]))) {
            if (arguments[0].kind == PrgValueKind::currency && !arguments[0].is_null) {
                // TRANSFORM(Y) shows the currency symbol, grouped digits and SET DECIMALS places (two by
                // default), honoring SET POINT, SET SEPARATOR and SET CURRENCY (installed VFP9, probe
                // ~/temp/vfp9-probes/currency-c25/result2.txt); SET CURRENCY RIGHT placement is not applied.
                int decimals = 2;
                try {
                    decimals = std::clamp(std::stoi(trim_copy(set_callback("DECIMALS"))), 0, 18);
                } catch (...) {
                }
                transformed = apply_numeric_picture_symbols(
                    format_currency_decimal_text(arguments[0].currency_value, decimals), true, true, set_callback);
            } else {
                transformed = format_value_for_display(arguments[0], set_callback);
            }
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
                                           ? saturating_size_argument(value_as_number(arguments[3]), 1U)
                                           : 1U;
        const int flags = arguments.size() >= 5U
                              ? saturating_int_argument(value_as_number(arguments[4]),
                                                        std::numeric_limits<int>::min(), std::numeric_limits<int>::max())
                              : 0;
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
