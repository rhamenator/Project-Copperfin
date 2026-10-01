// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"

#include <cstddef>
#include <ctime>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace copperfin::runtime {

std::string trim_copy(std::string value);
std::string ltrim_space_copy(std::string value);
std::string rtrim_space_copy(std::string value);
std::string trim_space_copy(std::string value);
std::string lowercase_copy(std::string value);
bool starts_with_insensitive(const std::string& value, const std::string& prefix);
bool paths_equal_insensitive(const std::string& left, const std::string& right);
std::string normalize_identifier(std::string value);
bool declared_dll_type_uses_64_bit_integer(std::string type_name);
bool declared_dll_type_is_single(std::string type_name);
bool declared_dll_type_is_short(std::string type_name);
bool declared_dll_type_is_numeric_parameter(std::string type_name);
bool declared_dll_parameter_list_contains_type(
    const std::string& parameter_types,
    const std::string& requested_type);
std::size_t declared_dll_x86_stdcall_stack_bytes(const std::string& parameter_types);
std::string normalize_memory_variable_identifier(std::string value);
std::string normalize_path(const std::string& value);
bool paths_equal_for_platform(const std::string& left, const std::string& right);
bool is_index_file_path(const std::string& value);
std::string unquote_string(std::string value);
std::string take_first_token(std::string value);
std::string unquote_asset_path_token(std::string value);
std::string take_first_asset_path_token(std::string value);
std::string take_first_command_target_token(std::string value);
std::pair<std::string, std::string> split_first_word(std::string value);
std::string take_keyword_value(const std::string& text, const std::string& keyword);
std::string runtime_error_parameter(const std::string& message);
std::string uppercase_copy(std::string value);
bool is_bare_identifier_text(const std::string& value);
bool is_memory_variable_reference_text(const std::string& value);
std::string collapse_identifier(const std::string& value);
std::string unquote_identifier(std::string value);
std::string normalize_index_value(std::string value);
std::optional<double> try_parse_invariant_double(
    std::string_view value,
    bool allow_nonfinite = false);
std::optional<std::int64_t> try_parse_invariant_currency(std::string_view value);
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
PrgValue make_null_value();

// The VFP data-type class an operator sees for a value: Character, Numeric (Number, Integer, Double),
// Currency, Logical, Date, DateTime or Object. `empty` is a value with no type at all (an unresolved
// identifier; VFP raises error 12 for that, so no operator accepts it). A NULL value is classified by the
// caller first, because NULL propagates before any type check.
enum class PrgOperandClass { empty, character, numeric, currency, logical, date, datetime, object };
PrgOperandClass classify_operand(const PrgValue& value);

// The condition of a statement or clause. Installed VFP9 (probe retained at
// ~/temp/vfp9-probes/null-semantics-c24/vfp9-result-flow-msgs.txt) accepts only a Logical value or NULL
// (NULL is false) and raises a different error per statement: IF 9, DO CASE 10, a FOR or WHILE clause
// (DO WHILE, LOCATE, COUNT, SCAN, REPLACE, DELETE, GATHER ...) 1127, SET FILTER 37, SQL WHERE 1833.
// ELSEIF is the one exception (VFP accepts any value there), so it keeps value_as_bool. A condition with no
// value at all (empty) stays false; only the operators report an unset value as error 12.
enum class StatementConditionKind { if_statement, do_case, for_while, filter, sql_where };
bool statement_condition_value(const PrgValue& value, StatementConditionKind kind);

// Numeric equality as installed VFP9 behaves at its default settings (probes retained under
// ~/temp/vfp9-probes/numeric-eq-c24/, #6034): two numbers are equal when they are the same double or differ by
// about one unit in the last place, so a computed 0.1+0.2 equals 0.3 and 4.35*100 equals 435, while values that
// differ in the 15th or 16th significant digit, or by the former absolute 0.000001, are distinct
// (1.000000000000001 <> 1, 1.0000005 <> 1). The ordering operators use the same test, so a number that is equal
// is neither less nor greater.
bool numeric_values_equal(double left, double right);

// Equality of two numeric PrgValues: Integer against Integer and Currency against Currency compare their exact
// integer payloads (a conversion through double would merge adjacent integers above 2^53 and large scaled
// Currency values); every other numeric pairing uses numeric_values_equal on the doubles.
bool numeric_prg_values_equal(const PrgValue& left, const PrgValue& right);

// The shortest plain-decimal text (no exponent) that reads back as exactly `value`: 15, 16 or 17 significant
// digits, whichever is the first to round-trip. value_as_string() keeps six significant digits for display, so
// it must not be used to store a number (REPLACE n WITH 123.456789 kept 123.457 and 1.0000005 became 1).
std::string format_round_trip_decimal(double value);

// A key for COUNT(DISTINCT expr): two non-NULL values share a key when installed VFP9 counts them once
// (probe retained at ~/temp/vfp9-probes/typed-null-c24/result3.txt: COUNT(DISTINCT x) over 10, 10, 20, NULL is 2
// and over '', 'x', NULL, '' is 2). Numbers use their value, Character values ignore trailing blanks.
std::string aggregate_distinct_key(const PrgValue& value);

PrgValue make_boolean_value(bool value);
PrgValue make_number_value(double value);
PrgValue make_string_value(std::string value);
PrgValue make_object_reference_value(std::string value);
PrgValue make_date_value(std::string value);
PrgValue make_date_value(std::string value, int year, int month, int day);
PrgValue make_datetime_value(std::string value);
PrgValue make_datetime_value(
    std::string value,
    int year,
    int month,
    int day,
    int hour,
    int minute,
    int second);
PrgValue make_int64_value(std::int64_t value);
PrgValue make_uint64_value(std::uint64_t value);
PrgValue make_currency_value(std::int64_t scaled_value);

// A plain decimal ([-]digits[.digits], at least one digit, optional surrounding blanks) as a Currency value scaled
// by 10,000 and rounded half away from zero at the fifth decimal, for a `$` literal or a Y field's text. The range
// is +-922337203685477.5807 (installed VFP9, probe retained at ~/temp/vfp9-probes/currency-c25/result.txt:
// $922337203685477.5807 is valid, $922337203685477.5808 and $-922337203685477.5808 are error 1988).
enum class CurrencyDecimalStatus { ok, malformed, out_of_range };
struct CurrencyDecimal {
    CurrencyDecimalStatus status = CurrencyDecimalStatus::malformed;
    std::int64_t scaled = 0;
};
CurrencyDecimal parse_currency_decimal(const std::string& text);

// A Currency value as plain decimal text ("-1234.57") with `decimals` places, rounded half away from zero from the
// four stored (installed VFP9 shows two places by default and follows SET DECIMALS: 0 gives 1235, 4 gives 1234.5678
// for $1234.5678). The caller applies the point, separator and currency symbol.
std::string format_currency_decimal_text(std::int64_t scaled, int decimals);
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
