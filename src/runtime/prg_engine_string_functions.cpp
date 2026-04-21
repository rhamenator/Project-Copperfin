#include "prg_engine_string_functions.h"

#include "prg_engine_helpers.h"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace copperfin::runtime {

namespace {

bool wildcard_match_insensitive_local(const std::string& pattern, const std::string& text) {
    const std::string p = lowercase_copy(pattern);
    const std::string t = lowercase_copy(text);
    std::size_t pattern_index = 0U;
    std::size_t text_index = 0U;
    std::size_t star_index = std::string::npos;
    std::size_t star_text_index = 0U;
    while (text_index < t.size()) {
        if (pattern_index < p.size() && (p[pattern_index] == '?' || p[pattern_index] == t[text_index])) {
            ++pattern_index;
            ++text_index;
        } else if (pattern_index < p.size() && p[pattern_index] == '*') {
            star_index = pattern_index++;
            star_text_index = text_index;
        } else if (star_index != std::string::npos) {
            pattern_index = star_index + 1U;
            text_index = ++star_text_index;
        } else {
            return false;
        }
    }
    while (pattern_index < p.size() && p[pattern_index] == '*') {
        ++pattern_index;
    }
    return pattern_index == p.size();
}

bool expression_values_equal(const PrgValue& left, const PrgValue& right, bool exact_string_compare) {
    if (left.kind == PrgValueKind::string || right.kind == PrgValueKind::string) {
        const std::string left_value = value_as_string(left);
        const std::string right_value = value_as_string(right);
        if (exact_string_compare) {
            return trim_copy(left_value) == trim_copy(right_value);
        }
        return left_value.rfind(right_value, 0U) == 0U;
    }
    if (left.kind == PrgValueKind::boolean || right.kind == PrgValueKind::boolean) {
        return value_as_bool(left) == value_as_bool(right);
    }
    if ((left.kind == PrgValueKind::int64 || left.kind == PrgValueKind::uint64) &&
        (right.kind == PrgValueKind::int64 || right.kind == PrgValueKind::uint64)) {
        return left.kind == PrgValueKind::int64
                   ? (right.kind == PrgValueKind::int64
                          ? left.int64_value == right.int64_value
                          : left.int64_value >= 0 && static_cast<std::uint64_t>(left.int64_value) == right.uint64_value)
                   : (right.kind == PrgValueKind::uint64
                          ? left.uint64_value == right.uint64_value
                          : right.int64_value >= 0 && left.uint64_value == static_cast<std::uint64_t>(right.int64_value));
    }
    return std::abs(value_as_number(left) - value_as_number(right)) < 0.000001;
}

}  // namespace

std::optional<PrgValue> evaluate_string_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments,
    bool exact_string_compare) {
    if (function == "len" && !arguments.empty()) {
        return make_number_value(static_cast<double>(value_as_string(arguments[0]).size()));
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
    if (function == "upper" && !arguments.empty()) {
        return make_string_value(uppercase_copy(value_as_string(arguments[0])));
    }
    if (function == "lower" && !arguments.empty()) {
        std::string s = value_as_string(arguments[0]);
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return make_string_value(std::move(s));
    }
    if ((function == "ltrim" || function == "trim") && !arguments.empty()) {
        const std::string src = value_as_string(arguments[0]);
        const std::size_t start = src.find_first_not_of(' ');
        return make_string_value(start == std::string::npos ? std::string{} : src.substr(start));
    }
    if (function == "rtrim" && !arguments.empty()) {
        const std::string src = value_as_string(arguments[0]);
        const std::size_t end = src.find_last_not_of(' ');
        return make_string_value(end == std::string::npos ? std::string{} : src.substr(0U, end + 1U));
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
    if (function == "strtran" && arguments.size() >= 3U) {
        std::string src = value_as_string(arguments[0]);
        const std::string find = value_as_string(arguments[1]);
        const std::string repl = value_as_string(arguments[2]);
        if (!find.empty()) {
            std::string result;
            std::size_t pos = 0U;
            while (pos < src.size()) {
                const std::size_t found = src.find(find, pos);
                if (found == std::string::npos) {
                    result += src.substr(pos);
                    break;
                }
                result += src.substr(pos, found - pos);
                result += repl;
                pos = found + find.size();
            }
            src = std::move(result);
        }
        return make_string_value(std::move(src));
    }
    if (function == "stuff" && arguments.size() >= 4U) {
        std::string src = value_as_string(arguments[0]);
        const std::size_t start = static_cast<std::size_t>(std::max(1.0, value_as_number(arguments[1]))) - 1U;
        const std::size_t length = static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[2])));
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
        double result = 0.0;
        try {
            result = std::stod(src);
        } catch (...) {
            result = 0.0;
        }
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
    if (function == "like" && arguments.size() >= 2U) {
        return make_boolean_value(wildcard_match_insensitive_local(
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
        const std::size_t n = static_cast<std::size_t>(std::max(1.0, value_as_number(arguments[1])));
        return make_string_value(n <= words.size() ? words[n - 1U] : std::string{});
    }
    if ((function == "at" || function == "atc") && arguments.size() >= 2U) {
        const bool case_insensitive = function == "atc";
        std::string needle = value_as_string(arguments[0]);
        std::string haystack = value_as_string(arguments[1]);
        const std::size_t occurrence = arguments.size() >= 3U
                                           ? static_cast<std::size_t>(std::max(1.0, value_as_number(arguments[2])))
                                           : 1U;
        if (case_insensitive) {
            needle = uppercase_copy(std::move(needle));
            haystack = uppercase_copy(std::move(haystack));
        }
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
    if ((function == "rat" || function == "ratc") && arguments.size() >= 2U) {
        const bool case_insensitive = function == "ratc";
        std::string needle = value_as_string(arguments[0]);
        std::string haystack = value_as_string(arguments[1]);
        const std::size_t occurrence = arguments.size() >= 3U
                                           ? static_cast<std::size_t>(std::max(1.0, value_as_number(arguments[2])))
                                           : 1U;
        if (case_insensitive) {
            needle = uppercase_copy(std::move(needle));
            haystack = uppercase_copy(std::move(haystack));
        }
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
            if (found == 0U) {
                break;
            }
            search_pos = found - 1U;
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
    if (function == "alltrim" && !arguments.empty()) {
        return make_string_value(trim_copy(value_as_string(arguments[0])));
    }
    if (function == "chr" && !arguments.empty()) {
        return make_string_value(std::string(1U, static_cast<char>(std::llround(value_as_number(arguments[0])))));
    }
    if (function == "str" && !arguments.empty()) {
        const int decimals = arguments.size() >= 3U
                                 ? static_cast<int>(std::max(0.0, value_as_number(arguments[2])))
                                 : 0;
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(decimals) << value_as_number(arguments[0]);
        std::string result = stream.str();
        if (arguments.size() >= 2U) {
            const int width = static_cast<int>(std::llround(value_as_number(arguments[1])));
            if (width > 0) {
                if (result.size() > static_cast<std::size_t>(width)) {
                    return make_string_value(std::string(static_cast<std::size_t>(width), '*'));
                }
                if (result.size() < static_cast<std::size_t>(width)) {
                    result.insert(result.begin(), static_cast<std::size_t>(width) - result.size(), ' ');
                }
            }
        }
        return make_string_value(std::move(result));
    }
    if (function == "transform" && !arguments.empty()) {
        const std::string picture = arguments.size() >= 2U ? uppercase_copy(value_as_string(arguments[1])) : std::string{};
        if (!picture.empty()) {
            if (picture.find("@!") != std::string::npos) {
                return make_string_value(uppercase_copy(value_as_string(arguments[0])));
            }
            if (picture.find("@L") != std::string::npos) {
                std::string value = value_as_string(arguments[0]);
                std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
                    return static_cast<char>(std::tolower(ch));
                });
                return make_string_value(std::move(value));
            }
            const std::size_t decimal_pos = picture.find('.');
            if (decimal_pos != std::string::npos) {
                std::size_t decimals = 0U;
                for (std::size_t index = decimal_pos + 1U; index < picture.size(); ++index) {
                    if (picture[index] == '9' || picture[index] == '#' || picture[index] == '0') {
                        ++decimals;
                    }
                }
                std::ostringstream stream;
                stream << std::fixed << std::setprecision(static_cast<int>(decimals)) << value_as_number(arguments[0]);
                return make_string_value(stream.str());
            }
        }
        return make_string_value(value_as_string(arguments[0]));
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