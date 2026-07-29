// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "prg_engine_date_time_functions.h"

#include "prg_engine_helpers.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cctype>
#include <functional>
#include <iomanip>
#include <sstream>

namespace copperfin::runtime {

namespace {

bool valid_runtime_date(int year, int month, int day) {
    const int max_day = days_in_month(year, month);
    return max_day != 0 && day >= 1 && day <= max_day;
}

bool parse_sortable_datetime(
    const std::string& value,
    int& year,
    int& month,
    int& day,
    int& hour,
    int& minute,
    int& second) {
    const std::string source = trim_copy(value);
    if (source.size() != 14U ||
        !std::all_of(source.begin(), source.end(), [](unsigned char ch) { return std::isdigit(ch) != 0; })) {
        return false;
    }

    try {
        year = std::stoi(source.substr(0U, 4U));
        month = std::stoi(source.substr(4U, 2U));
        day = std::stoi(source.substr(6U, 2U));
        hour = std::stoi(source.substr(8U, 2U));
        minute = std::stoi(source.substr(10U, 2U));
        second = std::stoi(source.substr(12U, 2U));
    } catch (...) {
        return false;
    }

    return valid_runtime_date(year, month, day) &&
           hour >= 0 && hour <= 23 &&
           minute >= 0 && minute <= 59 &&
           second >= 0 && second <= 59;
}

std::string format_sortable_datetime(int year, int month, int day, int hour, int minute, int second) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::setfill('0')
           << std::setw(4) << year
           << std::setw(2) << month
           << std::setw(2) << day
           << std::setw(2) << hour
           << std::setw(2) << minute
           << std::setw(2) << second;
    return stream.str();
}

std::string format_sortable_date(int year, int month, int day) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::setfill('0')
           << std::setw(4) << year
           << std::setw(2) << month
           << std::setw(2) << day;
    return stream.str();
}

std::string normalize_date_order(const std::function<std::string(const std::string&)>& set_callback) {
    std::string value = uppercase_copy(trim_copy(set_callback("DATE")));
    if (value == "BRITISH" || value == "FRENCH" || value == "GERMAN" || value == "ITALIAN") {
        return "DMY";
    }
    if (value == "ANSI" || value == "JAPAN" || value == "JAPANESE") {
        return "YMD";
    }
    if (value == "MDY" || value == "DMY" || value == "YMD") {
        return value;
    }
    return "MDY";
}

bool is_century_enabled(const std::function<std::string(const std::string&)>& set_callback) {
    return uppercase_copy(trim_copy(set_callback("CENTURY"))) != "OFF";
}

std::string date_mark(const std::function<std::string(const std::string&)>& set_callback) {
    const std::string mark = trim_copy(set_callback("MARK"));
    return mark.empty() || uppercase_copy(mark) == "OFF" ? std::string{"/"} : mark;
}

bool use_twelve_hour_clock(const std::function<std::string(const std::string&)>& set_callback) {
    return trim_copy(set_callback("HOURS")) == "12";
}

bool include_seconds(const std::function<std::string(const std::string&)>& set_callback) {
    return uppercase_copy(trim_copy(set_callback("SECONDS"))) != "OFF";
}

int set_int_value(
    const std::function<std::string(const std::string&)>& set_callback,
    const std::string& option_name,
    int default_value,
    int min_value,
    int max_value) {
    int value = default_value;
    try {
        value = std::stoi(trim_copy(set_callback(option_name)));
    } catch (...) {
        value = default_value;
    }
    return value < min_value || value > max_value ? default_value : value;
}

int epoch_year(const std::function<std::string(const std::string&)>& set_callback) {
    return set_int_value(set_callback, "EPOCH", 1950, 1, 9999);
}

int expand_two_digit_year_for_set(int year, const std::function<std::string(const std::string&)>& set_callback) {
    if (year < 0 || year >= 100) {
        return year;
    }

    const int epoch = epoch_year(set_callback);
    int expanded_year = (epoch / 100) * 100 + year;
    if (expanded_year < epoch) {
        expanded_year += 100;
    }
    return expanded_year;
}

std::string format_runtime_time_for_set(
    int hour,
    int minute,
    int second,
    const std::function<std::string(const std::string&)>& set_callback) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::setfill('0');
    if (use_twelve_hour_clock(set_callback)) {
        const bool afternoon = hour >= 12;
        int display_hour = hour % 12;
        if (display_hour == 0) {
            display_hour = 12;
        }
        stream << std::setw(2) << display_hour << ':' << std::setw(2) << minute;
        if (include_seconds(set_callback)) {
            stream << ':' << std::setw(2) << second;
        }
        stream << (afternoon ? " PM" : " AM");
    } else {
        stream << std::setw(2) << hour << ':' << std::setw(2) << minute;
        if (include_seconds(set_callback)) {
            stream << ':' << std::setw(2) << second;
        }
    }
    return stream.str();
}

std::string format_runtime_date_for_set(
    int year,
    int month,
    int day,
    const std::function<std::string(const std::string&)>& set_callback) {
    const std::string order = normalize_date_order(set_callback);
    const bool century = is_century_enabled(set_callback);
    const std::string mark = date_mark(set_callback);

    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::setfill('0');
    const auto write_year = [&]() {
        if (century) {
            stream << std::setw(4) << year;
        } else {
            stream << std::setw(2) << (std::abs(year) % 100);
        }
    };

    if (order == "DMY") {
        stream << std::setw(2) << day << mark << std::setw(2) << month << mark;
        write_year();
    } else if (order == "YMD") {
        write_year();
        stream << mark << std::setw(2) << month << mark << std::setw(2) << day;
    } else {
        stream << std::setw(2) << month << mark << std::setw(2) << day << mark;
        write_year();
    }
    return stream.str();
}

std::string format_runtime_datetime_for_set(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    int second,
    const std::function<std::string(const std::string&)>& set_callback) {
    std::ostringstream stream;
    stream << format_runtime_date_for_set(year, month, day, set_callback)
           << ' ' << format_runtime_time_for_set(hour, minute, second, set_callback);
    return stream.str();
}

bool parse_runtime_time_for_set(
    const std::string& raw,
    int& hour,
    int& minute,
    int& second,
    const std::function<std::string(const std::string&)>& set_callback) {
    std::string value = trim_copy(raw);
    if (value.empty()) {
        return false;
    }
    if (parse_runtime_time_string(value, hour, minute, second)) {
        return true;
    }

    bool afternoon = false;
    bool has_meridiem = false;
    if (value.size() > 3U && value[value.size() - 3U] == ' ') {
        const std::string suffix = uppercase_copy(value.substr(value.size() - 2U));
        if (suffix == "AM" || suffix == "PM") {
            has_meridiem = true;
            afternoon = suffix == "PM";
            value = trim_copy(value.substr(0U, value.size() - 3U));
        }
    }
    if (has_meridiem != use_twelve_hour_clock(set_callback)) {
        return false;
    }

    const std::size_t first_colon = value.find(':');
    if (first_colon == std::string::npos) {
        return false;
    }
    const std::size_t second_colon = value.find(':', first_colon + 1U);
    if (second_colon != std::string::npos && value.find(':', second_colon + 1U) != std::string::npos) {
        return false;
    }
    const auto parse_component = [&](std::size_t start, std::size_t length, int& output) -> bool {
        const std::string component = value.substr(start, length);
        if (component.empty() ||
            !std::all_of(component.begin(), component.end(), [](unsigned char ch) { return std::isdigit(ch) != 0; })) {
            return false;
        }
        try {
            output = std::stoi(component);
        } catch (...) {
            return false;
        }
        return true;
    };

    if (!parse_component(0U, first_colon, hour) ||
        !parse_component(
            first_colon + 1U,
            (second_colon == std::string::npos ? value.size() : second_colon) - first_colon - 1U,
            minute)) {
        return false;
    }
    second = 0;
    if (second_colon != std::string::npos &&
        !parse_component(second_colon + 1U, value.size() - second_colon - 1U, second)) {
        return false;
    }

    if (has_meridiem) {
        if (hour < 1 || hour > 12) {
            return false;
        }
        hour %= 12;
        if (afternoon) {
            hour += 12;
        }
    } else if (hour < 0 || hour > 23) {
        return false;
    }
    return minute >= 0 && minute <= 59 && second >= 0 && second <= 59;
}

bool parse_runtime_date_for_set(
    const std::string& raw,
    int& year,
    int& month,
    int& day,
    const std::function<std::string(const std::string&)>& set_callback) {
    const std::string value = trim_copy(raw);
    const std::string mark = date_mark(set_callback);
    const auto parse_component = [](const std::string& component, int& output) -> bool {
        if (component.empty() ||
            !std::all_of(component.begin(), component.end(), [](unsigned char ch) { return std::isdigit(ch) != 0; })) {
            return false;
        }
        try {
            output = std::stoi(component);
        } catch (...) {
            return false;
        }
        return true;
    };

    const auto parse_with_mark = [&](const std::string& delimiter) -> bool {
        if (delimiter.empty()) {
            return false;
        }
        const auto first_sep = value.find(delimiter);
        if (first_sep == std::string::npos) {
            return false;
        }
        const auto second_sep = value.find(delimiter, first_sep + delimiter.size());
        if (second_sep == std::string::npos || value.find(delimiter, second_sep + delimiter.size()) != std::string::npos) {
            return false;
        }

        int first = 0;
        int second = 0;
        int third = 0;
        if (!parse_component(value.substr(0U, first_sep), first) ||
            !parse_component(value.substr(first_sep + delimiter.size(), second_sep - first_sep - delimiter.size()), second) ||
            !parse_component(value.substr(second_sep + delimiter.size()), third)) {
            return false;
        }

        const std::string order = normalize_date_order(set_callback);
        if (order == "DMY") {
            day = first;
            month = second;
            year = third;
        } else if (order == "YMD") {
            year = first;
            month = second;
            day = third;
        } else {
            month = first;
            day = second;
            year = third;
        }

        year = expand_two_digit_year_for_set(year, set_callback);
        return valid_runtime_date(year, month, day);
    };

    if (parse_with_mark(mark)) {
        return true;
    }
    if (mark != "/" && parse_with_mark("/")) {
        return true;
    }

    if (parse_runtime_date_string(raw, year, month, day)) {
        year = expand_two_digit_year_for_set(year, set_callback);
        return true;
    }
    return false;
}

bool parse_runtime_datetime_for_set(
    const std::string& raw,
    int& year,
    int& month,
    int& day,
    int& hour,
    int& minute,
    int& second,
    const std::function<std::string(const std::string&)>& set_callback) {
    const std::string value = trim_copy(raw);
    const auto separator = value.find_first_of(" T");
    const std::string date_part = separator == std::string::npos ? value : value.substr(0U, separator);
    const std::string time_part = separator == std::string::npos ? std::string{} : trim_copy(value.substr(separator + 1U));
    if (!parse_runtime_date_for_set(date_part, year, month, day, set_callback)) {
        return false;
    }

    hour = 0;
    minute = 0;
    second = 0;
    return time_part.empty() || parse_runtime_time_for_set(time_part, hour, minute, second, set_callback);
}

bool temporal_components_from_scalar(
    const PrgValue& value,
    int& year,
    int& month,
    int& day,
    int& hour,
    int& minute,
    int& second) {
    if (value.string_flavor == PrgStringFlavor::none || value.int64_value == 0) {
        return false;
    }

    constexpr std::int64_t seconds_per_day = 24LL * 60LL * 60LL;
    const int julian = static_cast<int>(value.int64_value / seconds_per_day);
    const int second_of_day = static_cast<int>(value.int64_value % seconds_per_day);
    if (!julian_to_runtime_date(julian, year, month, day)) {
        return false;
    }
    hour = second_of_day / 3600;
    minute = (second_of_day % 3600) / 60;
    second = second_of_day % 60;
    return true;
}

bool parse_date_value_for_set(
    const PrgValue& value,
    int& year,
    int& month,
    int& day,
    const std::function<std::string(const std::string&)>& set_callback) {
    int hour = 0;
    int minute = 0;
    int second = 0;
    return temporal_components_from_scalar(value, year, month, day, hour, minute, second) ||
           parse_runtime_date_for_set(value_as_string(value), year, month, day, set_callback);
}

bool parse_datetime_value_for_set(
    const PrgValue& value,
    int& year,
    int& month,
    int& day,
    int& hour,
    int& minute,
    int& second,
    const std::function<std::string(const std::string&)>& set_callback) {
    return temporal_components_from_scalar(value, year, month, day, hour, minute, second) ||
           parse_runtime_datetime_for_set(
               value_as_string(value), year, month, day, hour, minute, second, set_callback);
}

std::string current_runtime_date() {
    const auto now = std::chrono::system_clock::now();
    const auto tt = std::chrono::system_clock::to_time_t(now);
    const std::tm local_tm = local_time_from_time_t(tt);
    return format_runtime_date_string(local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday);
}

std::string current_runtime_datetime() {
    const auto now = std::chrono::system_clock::now();
    const auto tt = std::chrono::system_clock::to_time_t(now);
    const std::tm local_tm = local_time_from_time_t(tt);
    return format_runtime_datetime_string(
        local_tm.tm_year + 1900,
        local_tm.tm_mon + 1,
        local_tm.tm_mday,
        local_tm.tm_hour,
        local_tm.tm_min,
        local_tm.tm_sec);
}

int current_second_of_day() {
    const auto now = std::chrono::system_clock::now();
    const auto tt = std::chrono::system_clock::to_time_t(now);
    const std::tm local_tm = local_time_from_time_t(tt);
    return (local_tm.tm_hour * 3600) + (local_tm.tm_min * 60) + local_tm.tm_sec;
}

std::optional<std::int64_t> date_time_ordering_key(
    const PrgValue& value,
    const std::function<std::string(const std::string&)>& set_callback) {
    const bool is_date = value.string_flavor == PrgStringFlavor::date;
    const bool is_datetime = value.string_flavor == PrgStringFlavor::datetime;
    if (!is_date && !is_datetime) {
        return std::nullopt;
    }

    constexpr std::int64_t seconds_per_day = 24LL * 60LL * 60LL;
    if (trim_copy(value_as_string(value)).empty()) {
        return (static_cast<std::int64_t>(date_to_julian(1, 1, 1)) * seconds_per_day) - 1LL;
    }
    if (value.int64_value != 0) {
        return value.int64_value;
    }

    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    const bool parsed = is_datetime
        ? parse_runtime_datetime_for_set(
              value_as_string(value),
              year,
              month,
              day,
              hour,
              minute,
              second,
              set_callback)
        : parse_runtime_date_for_set(value_as_string(value), year, month, day, set_callback);
    if (!parsed) {
        return std::nullopt;
    }

    return (static_cast<std::int64_t>(date_to_julian(year, month, day)) * seconds_per_day) +
           (hour * 3600LL) + (minute * 60LL) + second;
}

}  // namespace

std::optional<int> compare_date_time_values(
    const PrgValue& left,
    const PrgValue& right,
    const std::function<std::string(const std::string&)>& set_callback) {
    const auto left_key = date_time_ordering_key(left, set_callback);
    const auto right_key = date_time_ordering_key(right, set_callback);
    if (!left_key.has_value() || !right_key.has_value()) {
        return std::nullopt;
    }
    if (*left_key < *right_key) {
        return -1;
    }
    if (*left_key > *right_key) {
        return 1;
    }
    return 0;
}

std::optional<PrgValue> evaluate_date_time_additive(
    const PrgValue& left,
    const PrgValue& right,
    bool subtract,
    const std::function<std::string(const std::string&)>& set_callback) {
    const bool left_is_numeric = left.kind == PrgValueKind::number ||
                                 left.kind == PrgValueKind::int64 ||
                                 left.kind == PrgValueKind::uint64 ||
                                 left.kind == PrgValueKind::currency;
    if (!subtract && left.string_flavor == PrgStringFlavor::none &&
        right.string_flavor != PrgStringFlavor::none && left_is_numeric) {
        return evaluate_date_time_additive(right, left, false, set_callback);
    }

    const bool left_is_date = left.string_flavor == PrgStringFlavor::date;
    const bool left_is_datetime = left.string_flavor == PrgStringFlavor::datetime;
    const bool right_is_date = right.string_flavor == PrgStringFlavor::date;
    const bool right_is_datetime = right.string_flavor == PrgStringFlavor::datetime;
    const bool right_is_numeric = right.kind == PrgValueKind::number ||
                                  right.kind == PrgValueKind::int64 ||
                                  right.kind == PrgValueKind::uint64 ||
                                  right.kind == PrgValueKind::currency;

    if ((!left_is_date && !left_is_datetime) ||
        ((right_is_date || right_is_datetime) && (!subtract || left.string_flavor != right.string_flavor)) ||
        (!right_is_date && !right_is_datetime && !right_is_numeric)) {
        return std::nullopt;
    }

    int left_year = 0;
    int left_month = 0;
    int left_day = 0;
    int left_hour = 0;
    int left_minute = 0;
    int left_second = 0;
    const bool parsed_left = left_is_datetime
        ? parse_datetime_value_for_set(
              left,
              left_year,
              left_month,
              left_day,
              left_hour,
              left_minute,
              left_second,
              set_callback)
        : parse_date_value_for_set(left, left_year, left_month, left_day, set_callback);
    if (!parsed_left) {
        if (subtract && left.string_flavor == right.string_flavor) {
            return make_number_value(0.0);
        }
        return left_is_datetime ? make_datetime_value(std::string{}) : make_date_value(std::string{});
    }

    constexpr std::int64_t seconds_per_day = 24LL * 60LL * 60LL;
    const std::int64_t minimum_julian = date_to_julian(1, 1, 1);
    const std::int64_t maximum_julian = date_to_julian(9999, 12, 31);
    const std::int64_t left_julian = date_to_julian(left_year, left_month, left_day);
    if (right_is_date || right_is_datetime) {
        int right_year = 0;
        int right_month = 0;
        int right_day = 0;
        int right_hour = 0;
        int right_minute = 0;
        int right_second = 0;
        const bool parsed_right = right_is_datetime
            ? parse_datetime_value_for_set(
                  right,
                  right_year,
                  right_month,
                  right_day,
                  right_hour,
                  right_minute,
                  right_second,
                  set_callback)
            : parse_date_value_for_set(right, right_year, right_month, right_day, set_callback);
        if (!parsed_right) {
            return make_number_value(0.0);
        }

        const std::int64_t right_julian = date_to_julian(right_year, right_month, right_day);
        if (left_is_date) {
            return make_number_value(static_cast<double>(left_julian - right_julian));
        }

        const std::int64_t left_total = (left_julian * seconds_per_day) +
            (left_hour * 3600LL) + (left_minute * 60LL) + left_second;
        const std::int64_t right_total = (right_julian * seconds_per_day) +
            (right_hour * 3600LL) + (right_minute * 60LL) + right_second;
        return make_number_value(static_cast<double>(left_total - right_total));
    }

    const double raw_delta = value_as_number(right);
    const std::int64_t maximum_delta = left_is_datetime
        ? ((maximum_julian - minimum_julian) * seconds_per_day) + seconds_per_day - 1LL
        : maximum_julian - minimum_julian;
    if (!std::isfinite(raw_delta) ||
        raw_delta < -static_cast<double>(maximum_delta) ||
        raw_delta > static_cast<double>(maximum_delta)) {
        return left_is_datetime ? make_datetime_value(std::string{}) : make_date_value(std::string{});
    }
    const std::int64_t signed_delta = left_is_date
        ? static_cast<std::int64_t>(std::floor(subtract ? -raw_delta : raw_delta))
        : (subtract
              ? -static_cast<std::int64_t>(std::llround(raw_delta))
              : static_cast<std::int64_t>(std::llround(raw_delta)));

    if (left_is_date) {
        const std::int64_t result_julian = left_julian + signed_delta;
        int result_year = 0;
        int result_month = 0;
        int result_day = 0;
        if (!julian_to_runtime_date(static_cast<int>(result_julian), result_year, result_month, result_day)) {
            return make_date_value(std::string{});
        }
        return make_date_value(
            format_runtime_date_for_set(result_year, result_month, result_day, set_callback),
            result_year,
            result_month,
            result_day);
    }

    const std::int64_t minimum_total = minimum_julian * seconds_per_day;
    const std::int64_t maximum_total =
        (maximum_julian * seconds_per_day) + seconds_per_day - 1LL;
    const std::int64_t left_total = (left_julian * seconds_per_day) +
        (left_hour * 3600LL) + (left_minute * 60LL) + left_second;
    if ((signed_delta > 0 && left_total > maximum_total - signed_delta) ||
        (signed_delta < 0 && left_total < minimum_total - signed_delta)) {
        return make_datetime_value(std::string{});
    }
    const std::int64_t result_total = left_total + signed_delta;
    if (result_total < minimum_total || result_total > maximum_total) {
        return make_datetime_value(std::string{});
    }

    const int result_julian = static_cast<int>(result_total / seconds_per_day);
    const int result_second_of_day = static_cast<int>(result_total % seconds_per_day);
    int result_year = 0;
    int result_month = 0;
    int result_day = 0;
    if (!julian_to_runtime_date(result_julian, result_year, result_month, result_day)) {
        return make_datetime_value(std::string{});
    }
    const int result_hour = result_second_of_day / 3600;
    const int result_minute = (result_second_of_day % 3600) / 60;
    const int result_second = result_second_of_day % 60;
    return make_datetime_value(
        format_runtime_datetime_for_set(
            result_year,
            result_month,
            result_day,
            result_hour,
            result_minute,
            result_second,
            set_callback),
        result_year,
        result_month,
        result_day,
        result_hour,
        result_minute,
        result_second);
}

std::optional<PrgValue> evaluate_date_time_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments,
    const std::function<std::string(const std::string&)>& set_callback) {
    if (function == "date") {
        if (arguments.size() >= 3U) {
            const int year = static_cast<int>(std::llround(value_as_number(arguments[0])));
            const int month = static_cast<int>(std::llround(value_as_number(arguments[1])));
            const int day = static_cast<int>(std::llround(value_as_number(arguments[2])));
            if (!valid_runtime_date(year, month, day)) {
                return make_date_value(std::string{});
            }
            return make_date_value(format_runtime_date_for_set(year, month, day, set_callback), year, month, day);
        }
        int year = 0;
        int month = 0;
        int day = 0;
        return parse_runtime_date_string(current_runtime_date(), year, month, day)
                   ? make_date_value(format_runtime_date_for_set(year, month, day, set_callback), year, month, day)
                   : make_date_value(current_runtime_date());
    }
    if (function == "time") {
        const auto now = std::chrono::system_clock::now();
        const auto tt = std::chrono::system_clock::to_time_t(now);
        const std::tm local_tm = local_time_from_time_t(tt);
        return make_string_value(format_runtime_time_for_set(local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec, set_callback));
    }
    if (function == "datetime") {
        if (arguments.size() >= 3U) {
            const int year = static_cast<int>(std::llround(value_as_number(arguments[0])));
            const int month = static_cast<int>(std::llround(value_as_number(arguments[1])));
            const int day = static_cast<int>(std::llround(value_as_number(arguments[2])));
            const int hour = arguments.size() >= 4U ? static_cast<int>(std::llround(value_as_number(arguments[3]))) : 0;
            const int minute = arguments.size() >= 5U ? static_cast<int>(std::llround(value_as_number(arguments[4]))) : 0;
            const int second = arguments.size() >= 6U ? static_cast<int>(std::llround(value_as_number(arguments[5]))) : 0;
            if (!valid_runtime_date(year, month, day) ||
                hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
                return make_datetime_value(std::string{});
            }
            return make_datetime_value(
                format_runtime_datetime_for_set(year, month, day, hour, minute, second, set_callback),
                year,
                month,
                day,
                hour,
                minute,
                second);
        }
        int year = 0;
        int month = 0;
        int day = 0;
        int hour = 0;
        int minute = 0;
        int second = 0;
        return parse_runtime_datetime_string(current_runtime_datetime(), year, month, day, hour, minute, second)
                   ? make_datetime_value(
                         format_runtime_datetime_for_set(year, month, day, hour, minute, second, set_callback),
                         year,
                         month,
                         day,
                         hour,
                         minute,
                         second)
                   : make_datetime_value(current_runtime_datetime());
    }
    if (function == "seconds") {
        return make_number_value(static_cast<double>(current_second_of_day()));
    }
    if (function == "mdy" && arguments.size() >= 3U) {
        const int month = static_cast<int>(std::llround(value_as_number(arguments[0])));
        const int day = static_cast<int>(std::llround(value_as_number(arguments[1])));
        const int year = static_cast<int>(std::llround(value_as_number(arguments[2])));
        if (!valid_runtime_date(year, month, day)) {
            return make_date_value(std::string{});
        }
        return make_date_value(format_runtime_date_for_set(year, month, day, set_callback), year, month, day);
    }
    if (function == "dow" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        if (!parse_date_value_for_set(arguments[0], year, month, day, set_callback)) {
            return make_number_value(0.0);
        }
        int weekday = weekday_number_sunday_first(year, month, day);
        if (arguments.size() >= 2U) {
            int first_day = static_cast<int>(std::llround(value_as_number(arguments[1])));
            if (first_day < 1 || first_day > 7) {
                first_day = 1;
            }
            weekday = ((weekday - first_day + 7) % 7) + 1;
        }
        return make_number_value(static_cast<double>(weekday));
    }
    if (function == "cdow" && !arguments.empty()) {
        static constexpr std::array<const char*, 7U> kNames = {
            "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
        int year = 0;
        int month = 0;
        int day = 0;
        if (!parse_date_value_for_set(arguments[0], year, month, day, set_callback)) {
            return make_string_value(std::string{});
        }
        const int weekday = weekday_number_sunday_first(year, month, day);
        if (weekday < 1 || weekday > 7) {
            return make_string_value(std::string{});
        }
        return make_string_value(kNames[static_cast<std::size_t>(weekday - 1)]);
    }
    if (function == "cmonth" && !arguments.empty()) {
        static constexpr std::array<const char*, 12U> kNames = {
            "January", "February", "March", "April", "May", "June",
            "July", "August", "September", "October", "November", "December"};
        int year = 0;
        int month = 0;
        int day = 0;
        if (!parse_date_value_for_set(arguments[0], year, month, day, set_callback)) {
            return make_string_value(std::string{});
        }
        return make_string_value(kNames[static_cast<std::size_t>(month - 1)]);
    }
    if (function == "gomonth" && arguments.size() >= 2U) {
        int year = 0;
        int month = 0;
        int day = 0;
        if (!parse_date_value_for_set(arguments[0], year, month, day, set_callback)) {
            return make_date_value(std::string{});
        }
        const long long delta = static_cast<long long>(std::llround(value_as_number(arguments[1])));
        long long month_index = static_cast<long long>(year) * 12LL + static_cast<long long>(month - 1) + delta;
        long long adjusted_year = month_index / 12LL;
        long long adjusted_month_index = month_index % 12LL;
        if (adjusted_month_index < 0LL) {
            adjusted_month_index += 12LL;
            --adjusted_year;
        }
        const int adjusted_month = static_cast<int>(adjusted_month_index + 1LL);
        const int adjusted_day = std::min(day, days_in_month(static_cast<int>(adjusted_year), adjusted_month));
        return make_date_value(
            format_runtime_date_for_set(
                static_cast<int>(adjusted_year), adjusted_month, adjusted_day, set_callback),
            static_cast<int>(adjusted_year),
            adjusted_month,
            adjusted_day);
    }
    if (function == "year" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        return make_number_value(parse_date_value_for_set(arguments[0], year, month, day, set_callback)
                                     ? static_cast<double>(year)
                                     : 0.0);
    }
    if (function == "month" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        return make_number_value(parse_date_value_for_set(arguments[0], year, month, day, set_callback)
                                     ? static_cast<double>(month)
                                     : 0.0);
    }
    if (function == "day" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        return make_number_value(parse_date_value_for_set(arguments[0], year, month, day, set_callback)
                                     ? static_cast<double>(day)
                                     : 0.0);
    }
    if (function == "quarter" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        if (!parse_date_value_for_set(arguments[0], year, month, day, set_callback)) {
            return make_number_value(0.0);
        }
        return make_number_value(static_cast<double>(((month - 1) / 3) + 1));
    }
    if (function == "week" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        if (!parse_date_value_for_set(arguments[0], year, month, day, set_callback)) {
            return make_number_value(0.0);
        }

        int first_day = set_int_value(set_callback, "FDOW", 1, 1, 7);
        if (arguments.size() >= 2U) {
            first_day = static_cast<int>(std::llround(value_as_number(arguments[1])));
            if (first_day < 1 || first_day > 7) {
                first_day = 1;
            }
        }

        int first_week_mode = set_int_value(set_callback, "FWEEK", 1, 1, 3);
        if (arguments.size() >= 3U) {
            first_week_mode = static_cast<int>(std::llround(value_as_number(arguments[2])));
            if (first_week_mode < 1 || first_week_mode > 3) {
                first_week_mode = 1;
            }
        }

        if (first_week_mode == 1) {
            const int day_of_year = date_to_julian(year, month, day) - date_to_julian(year, 1, 1) + 1;
            const int jan1_weekday = weekday_number_sunday_first(year, 1, 1);
            const int offset = (jan1_weekday - first_day + 7) % 7;
            return make_number_value(static_cast<double>(((day_of_year + offset - 1) / 7) + 1));
        }

        const auto week_one_start_julian = [&](int week_year) {
            const int jan1_julian = date_to_julian(week_year, 1, 1);
            const int jan1_weekday = weekday_number_sunday_first(week_year, 1, 1);
            const int offset = (jan1_weekday - first_day + 7) % 7;
            if (first_week_mode == 2) {
                return jan1_julian + ((offset == 0) ? 0 : (7 - offset));
            }

            const int days_in_jan1_week = 7 - offset;
            return days_in_jan1_week >= 4
                       ? jan1_julian - offset
                       : jan1_julian + days_in_jan1_week;
        };

        const int date_julian = date_to_julian(year, month, day);
        const int current_start = week_one_start_julian(year);
        const int next_start = week_one_start_julian(year + 1);
        if (date_julian < current_start) {
            const int previous_start = week_one_start_julian(year - 1);
            return make_number_value(static_cast<double>(((date_julian - previous_start) / 7) + 1));
        }
        if (date_julian >= next_start) {
            return make_number_value(static_cast<double>(((date_julian - next_start) / 7) + 1));
        }
        return make_number_value(static_cast<double>(((date_julian - current_start) / 7) + 1));
    }
    if (function == "eomonth" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        if (!parse_date_value_for_set(arguments[0], year, month, day, set_callback)) {
            return make_date_value(std::string{});
        }

        long long delta = 0;
        if (arguments.size() >= 2U) {
            delta = static_cast<long long>(std::llround(value_as_number(arguments[1])));
        }

        long long month_index = static_cast<long long>(year) * 12LL + static_cast<long long>(month - 1) + delta;
        long long adjusted_year = month_index / 12LL;
        long long adjusted_month_index = month_index % 12LL;
        if (adjusted_month_index < 0LL) {
            adjusted_month_index += 12LL;
            --adjusted_year;
        }

        const int adjusted_month = static_cast<int>(adjusted_month_index + 1LL);
        const int adjusted_day = days_in_month(static_cast<int>(adjusted_year), adjusted_month);
        return make_date_value(
            format_runtime_date_for_set(
                static_cast<int>(adjusted_year), adjusted_month, adjusted_day, set_callback),
            static_cast<int>(adjusted_year),
            adjusted_month,
            adjusted_day);
    }
    if (function == "dtos" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        if (!parse_date_value_for_set(arguments[0], year, month, day, set_callback)) {
            return make_string_value(std::string{});
        }
        std::ostringstream stream;
        stream.imbue(std::locale::classic());
        stream << std::setfill('0')
               << std::setw(4) << year
               << std::setw(2) << month
               << std::setw(2) << day;
        return make_string_value(stream.str());
    }
    if (function == "stod" && !arguments.empty()) {
        const std::string source = trim_copy(value_as_string(arguments[0]));
        if (source.size() != 8U ||
            !std::all_of(source.begin(), source.end(), [](unsigned char ch) { return std::isdigit(ch) != 0; })) {
            return make_date_value(std::string{});
        }

        int year = 0;
        int month = 0;
        int day = 0;
        if (!parse_runtime_date_string(source, year, month, day)) {
            return make_date_value(std::string{});
        }
        return make_date_value(format_runtime_date_for_set(year, month, day, set_callback), year, month, day);
    }
    if (function == "ctod" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        if (parse_date_value_for_set(arguments[0], year, month, day, set_callback)) {
            return make_date_value(format_runtime_date_for_set(year, month, day, set_callback), year, month, day);
        }
        return make_date_value(std::string{});
    }
    if (function == "dtoc" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        if (parse_date_value_for_set(arguments[0], year, month, day, set_callback)) {
            if (arguments.size() >= 2U && static_cast<int>(std::llround(value_as_number(arguments[1]))) == 1) {
                return make_string_value(format_sortable_date(year, month, day));
            }
            return make_string_value(format_runtime_date_for_set(year, month, day, set_callback));
        }
        return make_string_value(value_as_string(arguments[0]));
    }
    if (function == "ttoc" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        int hour = 0;
        int minute = 0;
        int second = 0;
        if (parse_datetime_value_for_set(arguments[0], year, month, day, hour, minute, second, set_callback)) {
            if (arguments.size() >= 2U && static_cast<int>(std::llround(value_as_number(arguments[1]))) == 1) {
                return make_string_value(format_sortable_datetime(year, month, day, hour, minute, second));
            }
            return make_string_value(format_runtime_datetime_for_set(year, month, day, hour, minute, second, set_callback));
        }
        if (parse_date_value_for_set(arguments[0], year, month, day, set_callback)) {
            if (arguments.size() >= 2U && static_cast<int>(std::llround(value_as_number(arguments[1]))) == 1) {
                return make_string_value(format_sortable_datetime(year, month, day, 0, 0, 0));
            }
            return make_string_value(format_runtime_datetime_for_set(year, month, day, 0, 0, 0, set_callback));
        }
        return make_string_value(std::string{});
    }
    if (function == "ttos" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        int hour = 0;
        int minute = 0;
        int second = 0;
        if (parse_datetime_value_for_set(arguments[0], year, month, day, hour, minute, second, set_callback) ||
            parse_date_value_for_set(arguments[0], year, month, day, set_callback)) {
            return make_string_value(format_sortable_datetime(year, month, day, hour, minute, second));
        }
        return make_string_value(std::string{});
    }
    if (function == "ctot" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        int hour = 0;
        int minute = 0;
        int second = 0;
        const std::string value = value_as_string(arguments[0]);
        if (!parse_runtime_datetime_for_set(value, year, month, day, hour, minute, second, set_callback) &&
            !parse_sortable_datetime(value, year, month, day, hour, minute, second)) {
            return make_datetime_value(std::string{});
        }
        return make_datetime_value(
            format_runtime_datetime_for_set(year, month, day, hour, minute, second, set_callback),
            year,
            month,
            day,
            hour,
            minute,
            second);
    }
    if (function == "dtot" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        if (!parse_date_value_for_set(arguments[0], year, month, day, set_callback)) {
            return make_datetime_value(std::string{});
        }
        return make_datetime_value(
            format_runtime_datetime_for_set(year, month, day, 0, 0, 0, set_callback),
            year,
            month,
            day,
            0,
            0,
            0);
    }
    if (function == "ttod" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        int hour = 0;
        int minute = 0;
        int second = 0;
        const std::string value = value_as_string(arguments[0]);
        if (!parse_datetime_value_for_set(arguments[0], year, month, day, hour, minute, second, set_callback) &&
            !parse_sortable_datetime(value, year, month, day, hour, minute, second)) {
            return make_date_value(std::string{});
        }
        return make_date_value(format_runtime_date_for_set(year, month, day, set_callback), year, month, day);
    }
    if (function == "hour" && !arguments.empty()) {
        int hour = 0;
        int minute = 0;
        int second = 0;
        if (parse_runtime_time_string(value_as_string(arguments[0]), hour, minute, second)) {
            return make_number_value(static_cast<double>(hour));
        }
        int year = 0;
        int month = 0;
        int day = 0;
        if (parse_datetime_value_for_set(
                arguments[0], year, month, day, hour, minute, second, set_callback)) {
            return make_number_value(static_cast<double>(hour));
        }
        return make_number_value(0.0);
    }
    if (function == "minute" && !arguments.empty()) {
        int hour = 0;
        int minute = 0;
        int second = 0;
        if (parse_runtime_time_string(value_as_string(arguments[0]), hour, minute, second)) {
            return make_number_value(static_cast<double>(minute));
        }
        int year = 0;
        int month = 0;
        int day = 0;
        if (parse_datetime_value_for_set(
                arguments[0], year, month, day, hour, minute, second, set_callback)) {
            return make_number_value(static_cast<double>(minute));
        }
        return make_number_value(0.0);
    }
    if (function == "sec" && !arguments.empty()) {
        int hour = 0;
        int minute = 0;
        int second = 0;
        if (parse_runtime_time_string(value_as_string(arguments[0]), hour, minute, second)) {
            return make_number_value(static_cast<double>(second));
        }
        int year = 0;
        int month = 0;
        int day = 0;
        if (parse_datetime_value_for_set(
                arguments[0], year, month, day, hour, minute, second, set_callback)) {
            return make_number_value(static_cast<double>(second));
        }
        return make_number_value(0.0);
    }
    if ((function == "ttoj" || function == "dtoj") && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        if (function == "ttoj") {
            int hour = 0;
            int minute = 0;
            int second = 0;
            if (!parse_datetime_value_for_set(
                    arguments[0], year, month, day, hour, minute, second, set_callback)) {
                return make_number_value(0.0);
            }
        } else if (!parse_date_value_for_set(arguments[0], year, month, day, set_callback)) {
            return make_number_value(0.0);
        }
        return make_number_value(static_cast<double>(date_to_julian(year, month, day)));
    }
    if ((function == "jtot" || function == "jtod") && !arguments.empty()) {
        int julian = static_cast<int>(value_as_number(arguments[0]));
        int year = 0;
        int month = 0;
        int day = 0;
        if (!julian_to_runtime_date(julian, year, month, day)) {
            return function == "jtot" ? make_datetime_value(std::string{}) : make_date_value(std::string{});
        }
        if (function == "jtot") {
            return make_datetime_value(
                format_runtime_datetime_for_set(year, month, day, 0, 0, 0, set_callback),
                year,
                month,
                day,
                0,
                0,
                0);
        }
        return make_date_value(format_runtime_date_for_set(year, month, day, set_callback), year, month, day);
    }
    if (function == "dmy" && arguments.size() >= 3U) {
        int day = static_cast<int>(value_as_number(arguments[0]));
        int month = static_cast<int>(value_as_number(arguments[1]));
        int year = static_cast<int>(value_as_number(arguments[2]));
        if (!valid_runtime_date(year, month, day)) {
            return make_date_value(std::string{});
        }
        return make_date_value(format_runtime_date_for_set(year, month, day, set_callback), year, month, day);
    }
    if (function == "isleapyear" && !arguments.empty()) {
        int year = static_cast<int>(value_as_number(arguments[0]));
        return make_boolean_value(is_leap_year(year));
    }

    return std::nullopt;
}

}  // namespace copperfin::runtime
