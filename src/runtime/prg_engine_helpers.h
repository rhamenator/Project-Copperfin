#pragma once

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"

#include <ctime>
#include <optional>
#include <string>
#include <utility>

namespace copperfin::runtime {

std::string trim_copy(std::string value);
std::string lowercase_copy(std::string value);
bool starts_with_insensitive(const std::string& value, const std::string& prefix);
std::string normalize_identifier(std::string value);
std::string normalize_memory_variable_identifier(std::string value);
std::string normalize_path(const std::string& value);
bool is_index_file_path(const std::string& value);
std::string unquote_string(std::string value);
std::string take_first_token(std::string value);
std::pair<std::string, std::string> split_first_word(std::string value);
std::string take_keyword_value(const std::string& text, const std::string& keyword);
std::string uppercase_copy(std::string value);
bool is_bare_identifier_text(const std::string& value);
std::string collapse_identifier(const std::string& value);
std::string unquote_identifier(std::string value);
std::string normalize_index_value(std::string value);
std::optional<double> try_parse_numeric_index_value(const std::string& value);
int compare_index_keys(
    const std::string& left,
    const std::string& right,
    const std::string& key_domain_hint);
std::optional<std::string> record_field_value(const vfp::DbfRecord& record, const std::string& field_name);
std::string evaluate_index_expression(const std::string& expression, const vfp::DbfRecord& record);
bool has_keyword(const std::string& text, const std::string& keyword);
bool parse_object_handle_reference(const PrgValue& value, int& handle, std::string& prog_id);
PrgValue make_empty_value();
PrgValue make_boolean_value(bool value);
PrgValue make_number_value(double value);
PrgValue make_string_value(std::string value);
PrgValue make_int64_value(std::int64_t value);
PrgValue make_uint64_value(std::uint64_t value);
bool value_as_bool(const PrgValue& value);
double value_as_number(const PrgValue& value);
std::string value_as_string(const PrgValue& value);

int date_to_julian(int year, int month, int day);
void julian_to_date(int julian, int& year, int& month, int& day);
bool julian_to_runtime_date(int julian, int& year, int& month, int& day);
std::size_t portable_path_separator_position(const std::string& path);
std::string portable_path_drive(const std::string& path);
std::string portable_path_parent(const std::string& path);
std::string portable_path_filename(const std::string& path);
std::string portable_path_extension(const std::string& path);
std::string portable_path_stem(const std::string& path);
std::string portable_force_extension(const std::string& path, std::string extension);
std::string portable_force_path(const std::string& path, std::string directory);
bool parse_runtime_date_string(const std::string& raw, int& year, int& month, int& day);
std::string format_runtime_date_string(int year, int month, int day);
bool parse_runtime_time_string(const std::string& raw, int& hour, int& minute, int& second);
bool parse_runtime_datetime_string(
    const std::string& raw,
    int& year,
    int& month,
    int& day,
    int& hour,
    int& minute,
    int& second);
std::string format_runtime_datetime_string(int year, int month, int day, int hour, int minute, int second);
int weekday_number_sunday_first(int year, int month, int day);
std::tm local_time_from_time_t(std::time_t raw_time);
bool is_leap_year(int year);
int days_in_month(int year, int month);
std::vector<std::string> split_text_lines(const std::string& contents);

}  // namespace copperfin::runtime
