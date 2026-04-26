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

std::string format_runtime_time_for_set(
    int hour,
    int minute,
    int second,
    const std::function<std::string(const std::string&)>& set_callback) {
    std::ostringstream stream;
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

bool parse_runtime_date_for_set(
    const std::string& raw,
    int& year,
    int& month,
    int& day,
    const std::function<std::string(const std::string&)>& set_callback) {
    if (parse_runtime_date_string(raw, year, month, day)) {
        if (year >= 0 && year < 100) {
            year += year < 50 ? 2000 : 1900;
        }
        return true;
    }

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

        if (year >= 0 && year < 100) {
            year += year < 50 ? 2000 : 1900;
        }
        return valid_runtime_date(year, month, day);
    };

    if (parse_with_mark(mark)) {
        return true;
    }
    return mark != "/" && parse_with_mark("/");
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
    if (parse_runtime_datetime_string(raw, year, month, day, hour, minute, second)) {
        if (year >= 0 && year < 100) {
            year += year < 50 ? 2000 : 1900;
        }
        return true;
    }

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
    return time_part.empty() || parse_runtime_time_string(time_part, hour, minute, second);
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

}  // namespace

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
                return make_string_value(std::string{});
            }
            return make_string_value(format_runtime_date_for_set(year, month, day, set_callback));
        }
        int year = 0;
        int month = 0;
        int day = 0;
        return parse_runtime_date_string(current_runtime_date(), year, month, day)
                   ? make_string_value(format_runtime_date_for_set(year, month, day, set_callback))
                   : make_string_value(current_runtime_date());
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
                return make_string_value(std::string{});
            }
            return make_string_value(format_runtime_datetime_for_set(year, month, day, hour, minute, second, set_callback));
        }
        int year = 0;
        int month = 0;
        int day = 0;
        int hour = 0;
        int minute = 0;
        int second = 0;
        return parse_runtime_datetime_string(current_runtime_datetime(), year, month, day, hour, minute, second)
                   ? make_string_value(format_runtime_datetime_for_set(year, month, day, hour, minute, second, set_callback))
                   : make_string_value(current_runtime_datetime());
    }
    if (function == "seconds") {
        return make_number_value(static_cast<double>(current_second_of_day()));
    }
    if (function == "mdy" && arguments.size() >= 3U) {
        const int month = static_cast<int>(std::llround(value_as_number(arguments[0])));
        const int day = static_cast<int>(std::llround(value_as_number(arguments[1])));
        const int year = static_cast<int>(std::llround(value_as_number(arguments[2])));
        if (!valid_runtime_date(year, month, day)) {
            return make_string_value(std::string{});
        }
        return make_string_value(format_runtime_date_for_set(year, month, day, set_callback));
    }
    if (function == "dow" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        if (!parse_runtime_date_for_set(value_as_string(arguments[0]), year, month, day, set_callback)) {
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
        if (!parse_runtime_date_for_set(value_as_string(arguments[0]), year, month, day, set_callback)) {
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
        if (!parse_runtime_date_for_set(value_as_string(arguments[0]), year, month, day, set_callback)) {
            return make_string_value(std::string{});
        }
        return make_string_value(kNames[static_cast<std::size_t>(month - 1)]);
    }
    if (function == "gomonth" && arguments.size() >= 2U) {
        int year = 0;
        int month = 0;
        int day = 0;
        if (!parse_runtime_date_string(value_as_string(arguments[0]), year, month, day)) {
            return make_string_value(std::string{});
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
        return make_string_value(format_runtime_date_string(static_cast<int>(adjusted_year), adjusted_month, adjusted_day));
    }
    if (function == "year" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        return make_number_value(parse_runtime_date_string(value_as_string(arguments[0]), year, month, day)
                                     ? static_cast<double>(year)
                                     : 0.0);
    }
    if (function == "month" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        return make_number_value(parse_runtime_date_string(value_as_string(arguments[0]), year, month, day)
                                     ? static_cast<double>(month)
                                     : 0.0);
    }
    if (function == "day" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        return make_number_value(parse_runtime_date_string(value_as_string(arguments[0]), year, month, day)
                                     ? static_cast<double>(day)
                                     : 0.0);
    }
    if (function == "quarter" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        if (!parse_runtime_date_string(value_as_string(arguments[0]), year, month, day)) {
            return make_number_value(0.0);
        }
        return make_number_value(static_cast<double>(((month - 1) / 3) + 1));
    }
    if (function == "week" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        if (!parse_runtime_date_for_set(value_as_string(arguments[0]), year, month, day, set_callback)) {
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
        if (!parse_runtime_date_string(value_as_string(arguments[0]), year, month, day)) {
            return make_string_value(std::string{});
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
        return make_string_value(format_runtime_date_string(static_cast<int>(adjusted_year), adjusted_month, adjusted_day));
    }
    if (function == "dtos" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        if (!parse_runtime_date_string(value_as_string(arguments[0]), year, month, day)) {
            return make_string_value(std::string{});
        }
        std::ostringstream stream;
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
            return make_string_value(std::string{});
        }

        int year = 0;
        int month = 0;
        int day = 0;
        if (!parse_runtime_date_string(source, year, month, day)) {
            return make_string_value(std::string{});
        }
        return make_string_value(format_runtime_date_string(year, month, day));
    }
    if (function == "ctod" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        if (parse_runtime_date_for_set(value_as_string(arguments[0]), year, month, day, set_callback)) {
            return make_string_value(format_runtime_date_for_set(year, month, day, set_callback));
        }
        return make_string_value(std::string{});
    }
    if (function == "dtoc" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        if (parse_runtime_date_for_set(value_as_string(arguments[0]), year, month, day, set_callback)) {
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
        const std::string value = value_as_string(arguments[0]);
        if (parse_runtime_datetime_for_set(value, year, month, day, hour, minute, second, set_callback)) {
            if (arguments.size() >= 2U && static_cast<int>(std::llround(value_as_number(arguments[1]))) == 1) {
                return make_string_value(format_sortable_datetime(year, month, day, hour, minute, second));
            }
            return make_string_value(format_runtime_datetime_for_set(year, month, day, hour, minute, second, set_callback));
        }
        if (parse_runtime_date_for_set(value, year, month, day, set_callback)) {
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
        const std::string value = value_as_string(arguments[0]);
        if (parse_runtime_datetime_for_set(value, year, month, day, hour, minute, second, set_callback) ||
            parse_runtime_date_for_set(value, year, month, day, set_callback)) {
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
            return make_string_value(std::string{});
        }
        return make_string_value(format_runtime_datetime_for_set(year, month, day, hour, minute, second, set_callback));
    }
    if (function == "dtot" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        if (!parse_runtime_date_for_set(value_as_string(arguments[0]), year, month, day, set_callback)) {
            return make_string_value(std::string{});
        }
        return make_string_value(format_runtime_datetime_for_set(year, month, day, 0, 0, 0, set_callback));
    }
    if (function == "ttod" && !arguments.empty()) {
        int year = 0;
        int month = 0;
        int day = 0;
        int hour = 0;
        int minute = 0;
        int second = 0;
        const std::string value = value_as_string(arguments[0]);
        if (!parse_runtime_datetime_for_set(value, year, month, day, hour, minute, second, set_callback) &&
            !parse_sortable_datetime(value, year, month, day, hour, minute, second)) {
            return make_string_value(std::string{});
        }
        return make_string_value(format_runtime_date_for_set(year, month, day, set_callback));
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
        if (parse_runtime_datetime_string(value_as_string(arguments[0]), year, month, day, hour, minute, second)) {
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
        if (parse_runtime_datetime_string(value_as_string(arguments[0]), year, month, day, hour, minute, second)) {
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
        if (parse_runtime_datetime_string(value_as_string(arguments[0]), year, month, day, hour, minute, second)) {
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
            if (!parse_runtime_datetime_string(value_as_string(arguments[0]), year, month, day, hour, minute, second)) {
                return make_number_value(0.0);
            }
        } else if (!parse_runtime_date_string(value_as_string(arguments[0]), year, month, day)) {
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
            return make_string_value(std::string{});
        }
        return make_string_value(format_runtime_date_string(year, month, day));
    }
    if (function == "dmy" && arguments.size() >= 3U) {
        int day = static_cast<int>(value_as_number(arguments[0]));
        int month = static_cast<int>(value_as_number(arguments[1]));
        int year = static_cast<int>(value_as_number(arguments[2]));
        if (!valid_runtime_date(year, month, day)) {
            return make_string_value(std::string{});
        }
        return make_string_value(format_runtime_date_string(year, month, day));
    }
    if (function == "isleapyear" && !arguments.empty()) {
        int year = static_cast<int>(value_as_number(arguments[0]));
        return make_boolean_value(is_leap_year(year));
    }

    return std::nullopt;
}

}  // namespace copperfin::runtime
