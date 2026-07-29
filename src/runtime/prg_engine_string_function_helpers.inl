// Copyright 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

// Included inside copperfin::runtime's private helper namespace.
bool wildcard_match_case_sensitive_local(const std::string& pattern, const std::string& text) {
    const std::string& p = pattern;
    const std::string& t = text;
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

std::size_t utf8_scalar_width_local(const std::string& text, std::size_t offset) {
    const auto byte_at = [&](std::size_t index) {
        return static_cast<unsigned char>(text[index]);
    };
    if (offset >= text.size() || byte_at(offset) < 0x80U) {
        return 1U;
    }

    const unsigned char lead = byte_at(offset);
    std::size_t width = 0U;
    if (lead >= 0xC2U && lead <= 0xDFU) {
        width = 2U;
    } else if (lead >= 0xE0U && lead <= 0xEFU) {
        width = 3U;
    } else if (lead >= 0xF0U && lead <= 0xF4U) {
        width = 4U;
    } else {
        return 1U;
    }
    if (offset + width > text.size()) {
        return 1U;
    }
    for (std::size_t index = 1U; index < width; ++index) {
        if ((byte_at(offset + index) & 0xC0U) != 0x80U) {
            return 1U;
        }
    }

    const unsigned char second = byte_at(offset + 1U);
    if ((lead == 0xE0U && second < 0xA0U) ||
        (lead == 0xEDU && second > 0x9FU) ||
        (lead == 0xF0U && second < 0x90U) ||
        (lead == 0xF4U && second > 0x8FU)) {
        return 1U;
    }
    return width;
}

std::vector<std::size_t> utf8_scalar_offsets_local(const std::string& text) {
    std::vector<std::size_t> offsets;
    offsets.reserve(text.size() + 1U);
    offsets.push_back(0U);
    std::size_t offset = 0U;
    while (offset < text.size()) {
        offset += utf8_scalar_width_local(text, offset);
        offsets.push_back(offset);
    }
    return offsets;
}

std::vector<std::string> utf8_scalars_local(const std::string& text) {
    const std::vector<std::size_t> offsets = utf8_scalar_offsets_local(text);
    std::vector<std::string> scalars;
    scalars.reserve(offsets.size() - 1U);
    for (std::size_t index = 1U; index < offsets.size(); ++index) {
        scalars.push_back(text.substr(offsets[index - 1U], offsets[index] - offsets[index - 1U]));
    }
    return scalars;
}

bool wildcard_match_utf8_scalar_case_sensitive_local(
    const std::string& pattern,
    const std::string& text) {
    const std::vector<std::string> pattern_scalars = utf8_scalars_local(pattern);
    const std::vector<std::string> text_scalars = utf8_scalars_local(text);
    std::size_t pattern_index = 0U;
    std::size_t text_index = 0U;
    std::size_t star_index = std::string::npos;
    std::size_t star_text_index = 0U;
    while (text_index < text_scalars.size()) {
        if (pattern_index < pattern_scalars.size() &&
            (pattern_scalars[pattern_index] == "?" ||
             pattern_scalars[pattern_index] == text_scalars[text_index])) {
            ++pattern_index;
            ++text_index;
        } else if (pattern_index < pattern_scalars.size() && pattern_scalars[pattern_index] == "*") {
            star_index = pattern_index++;
            star_text_index = text_index;
        } else if (star_index != std::string::npos) {
            pattern_index = star_index + 1U;
            text_index = ++star_text_index;
        } else {
            return false;
        }
    }
    while (pattern_index < pattern_scalars.size() && pattern_scalars[pattern_index] == "*") {
        ++pattern_index;
    }
    return pattern_index == pattern_scalars.size();
}

std::string fold_ascii_case_preserving_utf8_local(const std::string& text) {
    std::string folded = text;
    for (char& ch : folded) {
        const auto byte = static_cast<unsigned char>(ch);
        if (byte >= static_cast<unsigned char>('A') && byte <= static_cast<unsigned char>('Z')) {
            ch = static_cast<char>(byte + (static_cast<unsigned char>('a') - static_cast<unsigned char>('A')));
        }
    }
    return folded;
}

std::string utf8_scalar_slice_local(
    const std::string& source,
    std::size_t start_one_based,
    std::size_t length) {
    const std::vector<std::size_t> offsets = utf8_scalar_offsets_local(source);
    const std::size_t scalar_count = offsets.size() - 1U;
    if (start_one_based == 0U || start_one_based > scalar_count) {
        return {};
    }
    const std::size_t begin = offsets[start_one_based - 1U];
    const std::size_t remaining = scalar_count - (start_one_based - 1U);
    const std::size_t end_scalar = length >= remaining ? scalar_count : start_one_based - 1U + length;
    return source.substr(begin, offsets[end_scalar] - begin);
}

std::size_t find_utf8_scalar_occurrence_local(
    const std::string& needle,
    const std::string& haystack,
    std::size_t occurrence,
    bool reverse,
    bool case_insensitive = true) {
    std::vector<std::string> needle_scalars = utf8_scalars_local(needle);
    std::vector<std::string> folded_haystack = utf8_scalars_local(haystack);
    if (needle_scalars.empty() || folded_haystack.size() < needle_scalars.size()) {
        return 0U;
    }
    if (case_insensitive) {
        for (std::string& scalar : needle_scalars) {
            scalar = fold_ascii_case_preserving_utf8_local(scalar);
        }
        for (std::string& scalar : folded_haystack) {
            scalar = fold_ascii_case_preserving_utf8_local(scalar);
        }
    }

    const std::size_t last_start = folded_haystack.size() - needle_scalars.size();
    std::size_t found_count = 0U;
    if (!reverse) {
        std::size_t start = 0U;
        while (start <= last_start) {
            if (std::equal(needle_scalars.begin(), needle_scalars.end(), folded_haystack.begin() + start)) {
                if (++found_count == occurrence) {
                    return start + 1U;
                }
                start += needle_scalars.size();
            } else {
                ++start;
            }
        }
        return 0U;
    }

    std::size_t limit = last_start;
    while (true) {
        for (std::size_t start = limit + 1U; start-- > 0U;) {
            if (std::equal(needle_scalars.begin(), needle_scalars.end(), folded_haystack.begin() + start)) {
                if (++found_count == occurrence) {
                    return start + 1U;
                }
                if (start < needle_scalars.size()) {
                    return 0U;
                }
                limit = start - needle_scalars.size();
                break;
            }
            if (start == 0U) {
                return 0U;
            }
        }
    }
}

std::string replace_utf8_scalars_local(
    const std::string& source,
    std::size_t start_one_based,
    std::size_t length,
    const std::string& replacement) {
    const std::vector<std::size_t> offsets = utf8_scalar_offsets_local(source);
    const std::size_t scalar_count = offsets.size() - 1U;
    if (start_one_based == 0U || start_one_based > scalar_count + 1U) {
        return source;
    }
    const std::size_t begin = offsets[start_one_based - 1U];
    const std::size_t remaining = scalar_count - (start_one_based - 1U);
    const std::size_t end_scalar = length >= remaining ? scalar_count : start_one_based - 1U + length;
    return source.substr(0U, begin) + replacement + source.substr(offsets[end_scalar]);
}

bool expression_values_equal(const PrgValue& left, const PrgValue& right, bool exact_string_compare) {
    if (left.kind == PrgValueKind::string || right.kind == PrgValueKind::string) {
        const std::string left_value = value_as_string(left);
        const std::string right_value = value_as_string(right);
        if (exact_string_compare) {
            return left_value == right_value;
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
    if (left.kind == PrgValueKind::currency && right.kind == PrgValueKind::currency) {
        return left.currency_value == right.currency_value;
    }
    return std::abs(value_as_number(left) - value_as_number(right)) < 0.000001;
}

std::optional<std::int64_t> parse_currency_scaled_value(const std::string& text) {
    std::size_t position = 0U;
    bool negative = false;
    if (position < text.size() && (text[position] == '+' || text[position] == '-')) {
        negative = text[position] == '-';
        ++position;
    }

    std::uint64_t coefficient = 0U;
    std::size_t digit_count = 0U;
    while (position < text.size() && std::isdigit(static_cast<unsigned char>(text[position])) != 0) {
        const auto digit = static_cast<std::uint64_t>(text[position] - '0');
        if (coefficient > (std::numeric_limits<std::uint64_t>::max() - digit) / 10U) {
            return std::nullopt;
        }
        coefficient = coefficient * 10U + digit;
        ++position;
        ++digit_count;
    }

    std::int64_t decimal_places = 0;
    if (position < text.size() && text[position] == '.') {
        ++position;
        while (position < text.size() && std::isdigit(static_cast<unsigned char>(text[position])) != 0) {
            const auto digit = static_cast<std::uint64_t>(text[position] - '0');
            if (coefficient > (std::numeric_limits<std::uint64_t>::max() - digit) / 10U) {
                return std::nullopt;
            }
            coefficient = coefficient * 10U + digit;
            ++position;
            ++digit_count;
            ++decimal_places;
        }
    }
    if (digit_count == 0U) {
        return std::nullopt;
    }

    std::int64_t exponent = 0;
    if (position < text.size() && (text[position] == 'e' || text[position] == 'E')) {
        ++position;
        bool exponent_negative = false;
        if (position < text.size() && (text[position] == '+' || text[position] == '-')) {
            exponent_negative = text[position] == '-';
            ++position;
        }
        const std::size_t exponent_start = position;
        while (position < text.size() && std::isdigit(static_cast<unsigned char>(text[position])) != 0) {
            if (exponent < 10000) {
                exponent = exponent * 10 + static_cast<std::int64_t>(text[position] - '0');
            }
            ++position;
        }
        if (position == exponent_start) {
            return std::nullopt;
        }
        if (exponent_negative) {
            exponent = -exponent;
        }
    }

    const std::int64_t shift = 4 - (decimal_places - exponent);
    std::uint64_t magnitude = coefficient;
    if (shift >= 0) {
        if (shift > 19) {
            if (magnitude != 0U) {
                return std::nullopt;
            }
        } else {
            for (std::int64_t index = 0; index < shift; ++index) {
                if (magnitude > std::numeric_limits<std::uint64_t>::max() / 10U) {
                    return std::nullopt;
                }
                magnitude *= 10U;
            }
        }
    } else {
        const std::int64_t divisor_digits = -shift;
        if (divisor_digits <= 19) {
            std::uint64_t divisor = 1U;
            for (std::int64_t index = 0; index < divisor_digits; ++index) {
                divisor *= 10U;
            }
            const std::uint64_t remainder = magnitude % divisor;
            magnitude /= divisor;
            if (remainder >= (divisor + 1U) / 2U) {
                ++magnitude;
            }
        } else {
            magnitude = 0U;
        }
    }

    const std::uint64_t maximum = negative
                                      ? static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) + 1U
                                      : static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());
    if (magnitude > maximum) {
        return std::nullopt;
    }
    if (!negative) {
        return static_cast<std::int64_t>(magnitude);
    }
    if (magnitude == maximum) {
        return std::numeric_limits<std::int64_t>::min();
    }
    return -static_cast<std::int64_t>(magnitude);
}

char soundex_digit(char ch) {
    switch (static_cast<char>(std::toupper(static_cast<unsigned char>(ch)))) {
        case 'B':
        case 'F':
        case 'P':
        case 'V':
            return '1';
        case 'C':
        case 'G':
        case 'J':
        case 'K':
        case 'Q':
        case 'S':
        case 'X':
        case 'Z':
            return '2';
        case 'D':
        case 'T':
            return '3';
        case 'L':
            return '4';
        case 'M':
        case 'N':
            return '5';
        case 'R':
            return '6';
        default:
            return '0';
    }
}

std::string soundex_code(const std::string& source) {
    std::string letters;
    letters.reserve(source.size());
    for (const char ch : source) {
        if (std::isalpha(static_cast<unsigned char>(ch)) != 0) {
            letters.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
        }
    }
    if (letters.empty()) {
        return "0000";
    }

    std::string result;
    result.reserve(4U);
    result.push_back(letters.front());
    char previous_digit = soundex_digit(letters.front());
    for (std::size_t index = 1U; index < letters.size() && result.size() < 4U; ++index) {
        const char digit = soundex_digit(letters[index]);
        if (digit != '0' && digit != previous_digit) {
            result.push_back(digit);
        }
        previous_digit = digit;
    }
    result.resize(4U, '0');
    return result;
}

std::vector<std::string> memo_width_lines(const std::string& source, std::size_t width = 50U) {
    width = std::max<std::size_t>(1U, width);
    std::vector<std::string> lines;
    std::size_t pos = 0U;
    while (pos < source.size()) {
        if (source[pos] == '\r') {
            lines.emplace_back();
            ++pos;
            if (pos < source.size() && source[pos] == '\n') {
                ++pos;
            }
            continue;
        }

        const std::size_t cr_pos = source.find('\r', pos);
        const std::size_t hard_end = cr_pos == std::string::npos ? source.size() : cr_pos;
        std::size_t remaining = hard_end - pos;
        if (remaining == 0U) {
            lines.emplace_back();
            pos = hard_end + 1U;
            if (pos < source.size() && source[pos] == '\n') {
                ++pos;
            }
            continue;
        }

        const std::size_t take_limit = std::min(width, remaining);
        std::size_t take = take_limit;
        if (remaining > width) {
            const std::string_view window(source.data() + pos, take_limit);
            const std::size_t space = window.find_last_of(' ');
            if (space != std::string_view::npos && space > 0U) {
                take = space;
            }
        }

        std::string line = source.substr(pos, take);
        while (!line.empty() && line.back() == ' ') {
            line.pop_back();
        }
        lines.push_back(std::move(line));
        pos += take;
        while (pos < hard_end && source[pos] == ' ') {
            ++pos;
        }
        if (pos >= hard_end && cr_pos != std::string::npos) {
            pos = hard_end + 1U;
            if (pos < source.size() && source[pos] == '\n') {
                ++pos;
            }
        }
    }
    if (source.empty()) {
        lines.emplace_back();
    }
    return lines;
}

std::string expand_tabs(const std::string& source, std::size_t tab_width) {
    tab_width = std::max<std::size_t>(1U, tab_width);
    std::string expanded;
    expanded.reserve(source.size());
    std::size_t column = 0U;
    for (const char ch : source) {
        if (ch == '\t') {
            const std::size_t spaces = tab_width - (column % tab_width);
            expanded.append(spaces, ' ');
            column += spaces;
            continue;
        }
        expanded.push_back(ch);
        if (ch == '\r' || ch == '\n') {
            column = 0U;
        } else {
            ++column;
        }
    }
    return expanded;
}

std::vector<std::string> memo_width_lines(
    const std::string& source,
    std::size_t width,
    std::size_t tab_width,
    std::size_t flags) {
    std::string normalized = tab_width == 0U ? source : expand_tabs(source, tab_width);
    if ((flags & 1U) != 0U) {
        std::string lf_normalized;
        lf_normalized.reserve(normalized.size());
        bool previous_was_cr = false;
        for (const char ch : normalized) {
            if (ch == '\n') {
                if (!previous_was_cr) {
                    lf_normalized.push_back('\r');
                }
                previous_was_cr = false;
                continue;
            }
            previous_was_cr = ch == '\r';
            lf_normalized.push_back(ch);
        }
        normalized = std::move(lf_normalized);
    }
    return memo_width_lines(normalized, width);
}

std::size_t optional_positive_size_argument(
    const std::vector<PrgValue>& arguments,
    std::size_t argument_index,
    std::size_t fallback_value,
    double minimum_value = 1.0) {
    if (argument_index >= arguments.size()) {
        return fallback_value;
    }
    const double parsed_value = value_as_number(arguments[argument_index]);
    if (parsed_value < minimum_value) {
        return fallback_value;
    }
    return static_cast<std::size_t>(parsed_value);
}

std::size_t optional_flag_argument(
    const std::vector<PrgValue>& arguments,
    std::size_t argument_index,
    std::size_t fallback_value) {
    if (argument_index >= arguments.size()) {
        return fallback_value;
    }
    return static_cast<std::size_t>(std::max(0.0, value_as_number(arguments[argument_index])));
}

std::vector<std::string> memo_width_lines_with_options(
    const std::string& source,
    const std::vector<PrgValue>& arguments,
    std::size_t width_argument_index,
    std::size_t default_width,
    std::size_t tab_width_argument_index,
    std::size_t flags_argument_index) {
    const std::size_t line_width = optional_positive_size_argument(
        arguments,
        width_argument_index,
        default_width,
        0.000001);
    const std::size_t tab_width = optional_positive_size_argument(
        arguments,
        tab_width_argument_index,
        0U,
        1.0);
    const std::size_t flags = optional_flag_argument(arguments, flags_argument_index, 0U);
    return memo_width_lines(source, line_width, tab_width, flags);
}

std::string set_symbol(
    const std::function<std::string(const std::string&)>& set_callback,
    const std::string& option_name,
    const std::string& default_value) {
    const std::string value = set_callback(option_name);
    const std::string trimmed = trim_copy(value);
    return trimmed.empty() || uppercase_copy(trimmed) == "OFF" ? default_value : value;
}

std::string apply_numeric_picture_symbols(
    std::string value,
    bool group_thousands,
    bool currency_picture,
    const std::function<std::string(const std::string&)>& set_callback) {
    const std::string point = set_symbol(set_callback, "POINT", ".");
    const std::string separator = set_symbol(set_callback, "SEPARATOR", ",");
    const std::string currency = set_symbol(set_callback, "CURRENCY", "$");
    const std::size_t decimal_pos = value.find('.');
    std::string integer_part = decimal_pos == std::string::npos ? value : value.substr(0U, decimal_pos);
    const std::string fraction_part = decimal_pos == std::string::npos ? std::string{} : value.substr(decimal_pos + 1U);

    if (group_thousands) {
        std::size_t start = integer_part.empty() || integer_part.front() != '-' ? 0U : 1U;
        for (std::size_t insert_pos = integer_part.size(); insert_pos > start + 3U; insert_pos -= 3U) {
            integer_part.insert(insert_pos - 3U, separator);
        }
    }

    if (decimal_pos == std::string::npos) {
        return currency_picture ? currency + integer_part : integer_part;
    }
    std::string formatted = integer_part + point + fraction_part;
    return currency_picture ? currency + formatted : formatted;
}

bool picture_has_numeric_placeholders(const std::string& picture) {
    return picture.find_first_of("9#0") != std::string::npos;
}

bool picture_is_digit_only_numeric(const std::string& picture) {
    return !picture.empty() &&
        std::all_of(
            picture.begin(),
            picture.end(),
            [](char ch) { return ch == '9' || ch == '#' || ch == '0'; });
}

std::string format_digit_only_numeric_picture(double value, const std::string& picture) {
    const std::size_t width = picture.size();
    if (width == 0U || !std::isfinite(value)) {
        return std::string(width, '*');
    }

    const bool negative = value < 0.0;
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(0) << std::abs(value);
    const std::string digits = stream.str();
    if (digits.size() > width || (negative && digits.size() >= width)) {
        return std::string(width, '*');
    }

    std::string transformed(width, ' ');
    std::size_t digit_start = width - digits.size();
    std::copy(digits.begin(), digits.end(), transformed.begin() + static_cast<std::ptrdiff_t>(digit_start));
    for (std::size_t index = 0U; index < digit_start; ++index) {
        if (picture[index] == '0') {
            transformed[index] = '0';
        }
    }

    if (negative) {
        const bool leading_zero_picture =
            std::all_of(picture.begin(), picture.begin() + static_cast<std::ptrdiff_t>(digit_start),
                        [](char ch) { return ch == '0'; });
        const std::size_t sign_position = leading_zero_picture ? 0U : digit_start - 1U;
        transformed[sign_position] = '-';
    }
    return transformed;
}

bool picture_has_flag(const std::string& picture, const std::string& flag) {
    return picture.find(flag) != std::string::npos;
}

std::string picture_payload_after_flag(
    const std::string& picture,
    const std::string& uppercase_picture,
    const std::string& flag) {
    const std::size_t flag_pos = uppercase_picture.find(flag);
    if (flag_pos == std::string::npos) {
        return picture;
    }

    std::size_t payload_pos = flag_pos + flag.size();
    if (payload_pos < picture.size() && picture[payload_pos] == ' ') {
        ++payload_pos;
    }
    return picture.substr(payload_pos);
}

std::string apply_literal_picture_template(const std::string& source, const std::string& picture) {
    std::string transformed;
    transformed.reserve(picture.size());

    std::size_t source_index = 0U;
    for (const char picture_ch : picture) {
        if (picture_ch == '9' || picture_ch == '#' || picture_ch == '0') {
            while (source_index < source.size() &&
                   std::isdigit(static_cast<unsigned char>(source[source_index])) == 0) {
                ++source_index;
            }
            if (source_index < source.size()) {
                transformed.push_back(source[source_index]);
                ++source_index;
            } else {
                transformed.push_back(' ');
            }
            continue;
        }

        transformed.push_back(picture_ch);
    }

    return transformed;
}

bool is_zeroish_transform_value(const PrgValue& value) {
    switch (value.kind) {
        case PrgValueKind::number:
            return std::abs(value.number_value) < 0.000001;
        case PrgValueKind::int64:
            return value.int64_value == 0;
        case PrgValueKind::uint64:
            return value.uint64_value == 0U;
        case PrgValueKind::currency:
            return value.currency_value == 0;
        default:
            return false;
    }
}

std::string left_justified_trim(std::string value) {
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    }));
    value.erase(std::find_if(value.rbegin(), value.rend(), [](unsigned char ch) {
                    return std::isspace(ch) == 0;
                }).base(),
                value.end());
    return value;
}
