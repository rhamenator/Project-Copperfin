// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"

#include <cstddef>
#include <ctime>
#include <functional>
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

// #6035: exact arithmetic and ordering for the 64-bit integer kinds. An operand is "exact" when it is an int64 or
// uint64 value, or a Numeric whose value is a finite integer in [-2^63, 2^64); the operations below apply when at
// least one operand is a 64-bit kind and both operands are exact, and never round-trip through double.
//   + - *   the exact result: int64 when it fits, uint64 only when an operand is uint64 and it exceeds INT64_MAX,
//           otherwise numeric overflow (error 39) -- never signed overflow or an out-of-range conversion;
//   /       truncating integer division, only when both operands are 64-bit kinds (a Numeric divisor keeps the
//           Numeric result); division by zero is the existing integer-division-by-zero error;
// std::nullopt means the operands are not eligible and the caller keeps its existing (double) handling.
std::optional<PrgValue> try_exact_integer_arithmetic(char operation, const PrgValue& left, const PrgValue& right);

// Exact three-way compare (-1, 0, 1) under the same eligibility rule; std::nullopt when not eligible.
std::optional<int> try_exact_integer_compare(const PrgValue& left, const PrgValue& right);

// Unary minus of an int64 or uint64: exact, with numeric overflow (error 39) for -INT64_MIN and for a uint64
// above 2^63.
PrgValue negate_exact_integer(const PrgValue& value);

// #5595/#5611/#6003/#6004: the largest Character string VFP9 holds (16,777,184 bytes) and a validated length from a
// Numeric argument. The value is truncated toward zero (a negative value becomes 0). std::nullopt means the value is
// NaN, infinite or above the ceiling, so callers raise their compatibility error instead of converting an out-of-range
// double to std::size_t (undefined) or allocating without a bound.
inline constexpr double kVfpMaxCharacterStringLength = 16'777'184.0;
std::optional<std::size_t> checked_character_string_length(double requested);

// #6776: how a Numeric argument that is out of range for the integer a function needs is handled. COPPERFIN (the
// default, `SET NUMERICBEHAVIOR TO COPPERFIN`) is defined and consistent: the value is truncated toward zero and
// saturates, so a huge count means "all" and a huge negative one means "none". VFP9 (`SET NUMERICBEHAVIOR TO VFP9`)
// reproduces what installed VFP9 SP2 does for functions that convert through a 32-bit integer: the value is truncated
// to int64 (anything NaN, infinite or outside int64 becomes the CPU's "integer indefinite", whose low 32 bits are 0)
// and its low 32 bits are taken as a signed int. Probe evidence: ~/temp/vfp9-probes/numconv-6776/probe1.txt.
enum class NumericBehavior { copperfin, vfp9 };
NumericBehavior numeric_behavior(const std::function<std::string(const std::string&)>& set_callback);
// Truncate toward zero; NaN is 0; saturate to [INT64_MIN, INT64_MAX].
std::int64_t saturating_numeric_to_int64(double value);
// VFP9's 32-bit conversion described above, widened back to int64 for the caller.
std::int64_t vfp9_numeric_to_int32(double value);
// Preserve exact int64/uint64 low bits instead of first rounding them through double.
std::int64_t vfp9_numeric_to_int32(const PrgValue& value);
// A signed count or length argument under `behavior`.
std::int64_t numeric_count_argument(double value, NumericBehavior behavior);
// Truncate toward zero only when `value` is finite and representable as int64. This is the fail-closed conversion for
// array positions/selectors and other arguments where saturation would turn invalid input into an ordinary value.
std::optional<std::int64_t> checked_truncated_numeric_to_int64(double value);
// RQ-CF-PRG-ALINES-FLAGS-NUMERIC-001: Numeric/exact-integer flags truncate to
// 0..31. COPPERFIN checks admission before conversion; only VFP9 retains
// signed-low-32-bit aliases and integer-indefinite zero. Other coercions keep
// existing half-away rounding with a checked int32 boundary, not native type
// parity. Missing result means localized error 11 before array mutation.
std::optional<std::int32_t> checked_alines_flags_argument(const PrgValue& value, NumericBehavior behavior);
// RQ-CF-PRG-ADIR-DISPLAY-NUMERIC-001: Numeric/exact-integer display flags
// truncate to 0..3 (including 3). Only explicit VFP9 retains low-32-bit
// aliases and indefinite zero. Other coercions preserve checked int32 rounding.
// Missing result means localized error 11 before enumeration/array mutation.
std::optional<std::int32_t> checked_adir_display_argument(const PrgValue& value, NumericBehavior behavior);
// RQ-CF-PRG-AFONT-SIZE-NUMERIC-001: Numeric/exact-integer sizes truncate to
// signed int32, including negative sizes. Only explicit VFP9 retains low-32-bit
// aliases and shared-model indefinite zero. Other coercions keep checked int32
// half-away rounding. Missing result means error 11 before font/array work.
std::optional<std::int32_t> checked_afont_size_argument(const PrgValue& value, NumericBehavior behavior);
// RQ-CF-PRG-FIELD-INDEX-NUMERIC-001: Numeric/exact integers truncate with
// checked signed-int64 and size_t admission in COPPERFIN, clamping negatives
// to the existing empty-field index zero. VFP9 keeps negative low-32-bit
// aliases but positive oversized indices stay empty. Other coercions keep
// checked half-away rounding; NaN is rejected. Missing result means error 11.
std::optional<std::size_t> checked_field_index_argument(const PrgValue& value, NumericBehavior behavior);
// RQ-CF-PRG-FSIZE-INDEX-NUMERIC-001: installed VFP9 rejects Numeric
// FSIZE indices rather than converting them; exact extended integers follow
// the same derived admission. Other existing non-Character coercions retain
// checked half-away rounding. Missing result means catchable error 11.
std::optional<std::size_t> checked_fsize_index_argument(const PrgValue& value);
// RQ-CF-PRG-SELECT-SELECTOR-NUMERIC-001: truncate Numeric selectors,
// admit 0..32767, and retain low-32 aliases only in explicit VFP9 mode.
// Other coercions retain checked half-away rounding. Missing means error 17.
std::optional<std::int32_t> checked_select_selector_argument(const PrgValue& value, NumericBehavior behavior);
// RQ-CF-PRG-SQLGETPROP-HANDLE-NUMERIC-001: truncate Numeric handles;
// only negative Numeric operands use explicit VFP9 low-32/indefinite aliases.
// Positive/default and other coercions require checked signed-int32 admission.
// Missing means catchable error 1466; property/backend behavior is separate.
std::optional<std::int32_t> checked_sqlgetprop_handle_argument(const PrgValue& value, NumericBehavior behavior);
// RQ-CF-PRG-SQLSETPROP-HANDLE-NUMERIC-001: independently observed identical
// handle conversion; property-value conversion and setter behavior are separate.
std::optional<std::int32_t> checked_sqlsetprop_handle_argument(const PrgValue& value, NumericBehavior behavior);
// RQ-CF-PRG-SQLDISCONNECT-HANDLE-NUMERIC-001: independently observed identical
// conversion; disconnect-all/absent-handle/type and lifecycle behavior are separate.
std::optional<std::int32_t> checked_sqldisconnect_handle_argument(const PrgValue& value, NumericBehavior behavior);
// RQ-CF-PRG-SQLCANCEL-HANDLE-NUMERIC-001: checked finite/truncating int32
// in both modes. Connection-free VFP9 errors do not distinguish converted
// indices, so no unobserved legacy aliases are inferred. Missing means 1466.
std::optional<std::int32_t> checked_sqlcancel_handle_argument(const PrgValue& value, NumericBehavior behavior);
// RQ-CF-PRG-SQLCOMMIT-HANDLE-NUMERIC-001: finite/truncating signed int32
// in both modes. Native absent errors cannot distinguish converted indices;
// no SQLGETPROP aliases inferred. Rejection precedes transaction callbacks.
std::optional<std::int32_t> checked_sqlcommit_handle_argument(const PrgValue& value, NumericBehavior behavior);
// RQ-CF-PRG-SQLROLLBACK-HANDLE-NUMERIC-001: finite/truncating signed int32
// in both modes. Native absent errors cannot distinguish converted indices;
// no SQLGETPROP aliases inferred. Rejection precedes transaction callbacks.
std::optional<std::int32_t> checked_sqlrollback_handle_argument(const PrgValue& value, NumericBehavior behavior);
// RQ-CF-PRG-SQLTABLES-HANDLE-NUMERIC-001: finite/truncating signed int32
// in both modes. Native absent errors cannot distinguish converted indices;
// no aliases inferred. Rejection precedes metadata cursor/state/events.
std::optional<std::int32_t> checked_sqltables_handle_argument(const PrgValue& value, NumericBehavior behavior);
// RQ-CF-PRG-SQLDATABASES-HANDLE-NUMERIC-001: extension-owned finite/
// truncating signed int32 in both modes, with no native conversion oracle.
// Rejection precedes metadata cursor/state/events; no aliases inferred.
std::optional<std::int32_t> checked_sqldatabases_handle_argument(const PrgValue& value, NumericBehavior behavior);
std::optional<std::int32_t> checked_sqlprimarykeys_handle_argument(const PrgValue& value, NumericBehavior behavior);
// RQ-CF-PRG-SQLFOREIGNKEYS-HANDLE-NUMERIC-001: owner-derived checked
// extension policy in both modes; reject before metadata/cursor/state/events.
std::optional<std::int32_t> checked_sqlforeignkeys_handle_argument(const PrgValue& value, NumericBehavior behavior);
// RQ-CF-PRG-SQLCOLUMNS-HANDLE-NUMERIC-001: derived finite/truncating
// signed int32 in both modes; native absent errors reveal no converted indices.
std::optional<std::int32_t> checked_sqlcolumns_handle_argument(const PrgValue& value, NumericBehavior behavior);
// RQ-CF-PRG-SQLROWCOUNT-HANDLE-NUMERIC-001: owner-derived extension
// policy, finite/truncating signed int32 in both modes, before row-count reads.
std::optional<std::int32_t> checked_sqlrowcount_handle_argument(const PrgValue& value, NumericBehavior behavior);
// RQ-CF-PRG-SET-DATASESSION-NUMERIC-001: checked positive session selector.
// Existence/lifecycle stay separate; other coercions retain rounding/clamping.
std::optional<std::int32_t> checked_datasession_selector_argument(const PrgValue& value, NumericBehavior behavior);
// RQ-CF-PRG-SET-DECIMALS-NUMERIC-001: Numeric/exact integers truncate to
// 0..18. Explicit VFP9 keeps low-32 aliases and indefinite zero; other existing
// coercions preserve checked rounding/clamping. Missing means error 10.
std::optional<std::int32_t> checked_set_decimals_argument(const PrgValue& value, NumericBehavior behavior);
// RQ-CF-PRG-SET-FDOW-NUMERIC-001: Numeric/exact integers truncate to
// 1..7. Explicit VFP9 keeps negative-only low-32 aliases; missing means 46.
// Other existing coercions preserve checked rounding/clamping/fallback to 1.
std::optional<std::int32_t> checked_set_fdow_argument(const PrgValue& value, NumericBehavior behavior);
// RQ-CF-PRG-SET-FWEEK-NUMERIC-001: Numeric/exact integers truncate to
// 1..3. Explicit VFP9 keeps negative-only low-32 aliases; missing means 46.
// Other existing coercions preserve checked rounding/clamping/fallback to 1.
std::optional<std::int32_t> checked_set_fweek_argument(const PrgValue& value, NumericBehavior behavior);
// RQ-CF-PRG-SET-EPOCH-NUMERIC-001: intentional extension in both numeric
// modes. Numeric/exact integers truncate to 1..9999 without wrapping or
// saturation; missing means localized error 10 before mutation. Ordinary
// other coercions retain checked rounding/clamping/fallback to 1950.
std::optional<std::int32_t> checked_set_epoch_argument(const PrgValue& value);
// RQ-CF-PRG-SET-CENTURY-NUMERIC-001 (#3698): Numeric/Currency/exact
// integers truncate into the caller's century, rollover or selector domain.
// VFP9 admits independently observed low-32 aliases; COPPERFIN never wraps.
std::optional<std::int32_t> checked_set_century_argument(
    const PrgValue& value, NumericBehavior behavior, int minimum, int maximum);
// RQ-CF-PRG-SET-CENTURY-WINDOW-001: initial/bare-TO window starts fifty
// years before the current local year. EPOCH's explicit reset remains 1950.
int default_set_century_epoch_for_year(int year);
int default_set_century_epoch();
// #6050: integer arguments crossing a DECLARE boundary. Ordinary VFP9
// INTEGER/LONG parameters receive the low 32 bits after truncation toward
// zero. Exact int64/uint64 values keep their low bits without a floating round
// trip. In COPPERFIN mode a finite Numeric outside int64 is rejected; VFP9
// mode reproduces the integer-indefinite low bits (zero). Non-finite values
// remain rejected in both modes. INTEGER64 is a Copperfin extension and
// accepts only values representable as signed int64. A missing result means
// the caller must raise localized, catchable error 11 before invoking native
// or managed code.
std::optional<std::int32_t> checked_declared_int32_argument(const PrgValue& value, NumericBehavior behavior);
std::optional<std::int64_t> checked_declared_int64_argument(const PrgValue& value);
// #6776: defined conversions for the sites that previously cast a double straight to an integer (undefined when the
// value is out of range). A size or count: truncated toward zero, never below `minimum`, saturating at the largest
// value this build supports: INT64_MAX, or SIZE_MAX where size_t is narrower (NaN gives `minimum`). A rounded int64 (llround semantics for in-range values; NaN is 0 and
// out-of-range saturates). A truncated int clamped to [minimum, maximum].
std::size_t saturating_size_argument(double value, std::size_t minimum = 0U);
std::int64_t rounded_numeric_to_int64(double value);
int saturating_int_argument(double value, int minimum, int maximum);

// The shortest plain-decimal text (no exponent) that reads back as exactly `value`: 15, 16 or 17 significant
// digits, whichever is the first to round-trip. value_as_string() keeps six significant digits for display, so
// it must not be used to store a number (REPLACE n WITH 123.456789 kept 123.457 and 1.0000005 became 1).
std::string format_round_trip_decimal(double value);

// A key for COUNT(DISTINCT expr): two non-NULL values share a key when installed VFP9 counts them once
// (probe retained at ~/temp/vfp9-probes/typed-null-c24/result3.txt: COUNT(DISTINCT x) over 10, 10, 20, NULL is 2
// and over '', 'x', NULL, '' is 2). Numbers use their value, Character values ignore trailing blanks.
std::string aggregate_distinct_key(const PrgValue& value);

// The running state of SUM, AVG, MIN and MAX over the non-NULL values of an expression, shared by CALCULATE, the
// SUM/AVERAGE commands and SQL SELECT (plain and grouped), each of which used to keep its own double accumulators
// (#6040, #6041). Installed VFP9 (probes retained at ~/temp/vfp9-probes/currency-c25/result9.txt and result10.txt):
// a NULL is skipped; SUM and AVG over Currency stay Currency, summed exactly in scaled integers (overflow is error
// 1988) and averaged exactly (truncated toward zero in CALCULATE and the commands, rounded half away from zero in SQL); over Numeric, Integer, Double and Float they are Numeric;
// SUM and AVG over any other type (Character, Date, DateTime, Logical) are error 27 "Not a numeric expression." in
// CALCULATE and the commands and error 1811 in SQL; MIN and MAX compare Date, DateTime, Character, Logical and
// numbers of one type and return the selected value with its own type.
class AggregateAccumulator {
public:
    explicit AggregateAccumulator(bool sql_semantics);
    // `function` is "sum", "avg"/"average", "min" or "max".
    void add(const std::string& function, const PrgValue& value);
    std::size_t count() const { return count_; }
    // `empty_result` is returned when no value was added.
    PrgValue result(const std::string& function, const PrgValue& empty_result) const;

private:
    bool sql_semantics_;
    std::size_t count_ = 0U;
    bool currency_only_ = true;
    std::int64_t currency_sum_ = 0;
    double double_sum_ = 0.0;
    PrgValue minimum_;
    PrgValue maximum_;
    // The domain of the first value compared by MIN or MAX: numbers of any kind, Character, Date or DateTime, or
    // Logical. A later value from another domain is error 107 (installed VFP9: MIN(IIF(RECNO()=1,1,'a')) is
    // "Operator/operand type mismatch."); a Date next to a DateTime and a Numeric next to a Currency compare.
    int compare_domain_ = 0;
};

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

// VFP-compatible binary conversion used by BINTOC()/CTOBIN(). `selector` is
// null when the optional argument was omitted. Both functions validate the
// complete selector grammar and byte width before conversion or allocation.
PrgValue bintoc_value(const PrgValue& value, const PrgValue* selector);
PrgValue ctobin_value(const PrgValue& value, const PrgValue* selector);

// A plain decimal ([-]digits[.digits], at least one digit, optional surrounding blanks) as a Currency value scaled
// by 10,000 and rounded half away from zero at the fifth decimal, for a `$` literal or a Y field's text. The range
// is +-922337203685477.5807 (installed VFP9, probe retained at ~/temp/vfp9-probes/currency-c25/result.txt:
// $922337203685477.5807 is valid, $922337203685477.5808 and $-922337203685477.5808 are error 1988).
enum class CurrencyDecimalStatus { ok, malformed, out_of_range };
struct CurrencyDecimal {
    CurrencyDecimalStatus status = CurrencyDecimalStatus::malformed;
    std::int64_t scaled = 0;
};
// `allow_storage_minimum` also accepts -922337203685477.5808, the stored INT64_MIN of a Y field, which a source
// literal may not use.
CurrencyDecimal parse_currency_decimal(const std::string& text, bool allow_storage_minimum = false);

// Exact Currency multiply and divide (#6742). The result is rounded half away from zero at the fourth decimal and
// computed in portable 128-bit integers, so it is the same on every platform and never loses digits above 2^53 scaled
// units the way a double (or a 53-bit `long double` on MSVC and some ARM targets) does. A Number operand is read as its
// shortest round-trip decimal text. `unsupported` means an operand has no exact small decimal form (not finite, more
// than 18 fractional digits or 19 integer digits), and the caller falls back to floating point.
enum class CurrencyArithmeticStatus { ok, out_of_range, unsupported };
struct CurrencyArithmeticResult {
    CurrencyArithmeticStatus status = CurrencyArithmeticStatus::unsupported;
    std::int64_t scaled = 0;
};
// `operation` is '*' or '/'; at least one operand is a Currency. A zero divisor is the caller's to reject.
CurrencyArithmeticResult currency_multiply_divide_exact(const PrgValue& left, const PrgValue& right, char operation);

// A Currency value as plain decimal text ("-1234.57") with `decimals` places, rounded half away from zero from the
// four stored (installed VFP9 shows two places by default and follows SET DECIMALS: 0 gives 1235, 4 gives 1234.5678
// for $1234.5678). The caller applies the point, separator and currency symbol.
std::string format_currency_decimal_text(std::int64_t scaled, int decimals);
bool value_as_bool(const PrgValue& value);
double value_as_number(const PrgValue& value);
std::string value_as_string(const PrgValue& value);

int date_to_julian(int year, int month, int day);
// Seconds since midnight for VFP's stored milliseconds, rounded to the nearest second (half up); a rounding that
// reaches a full day advances `julian_day` by one.
int stored_millis_to_seconds_of_day(int& julian_day, long long millis);
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
