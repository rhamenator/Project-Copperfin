// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/platform/invariant_numeric.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "../src/runtime/prg_engine_string_functions.h"
#include "prg_engine_test_support.h"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <locale>
#include <system_error>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

namespace
{

    using namespace copperfin::test_support;

    class comma_decimal_numpunct final : public std::numpunct<char>
    {
    protected:
        char do_decimal_point() const override { return ','; }
        char do_thousands_sep() const override { return '.'; }
        std::string do_grouping() const override { return "\3"; }
    };

    class global_locale_guard final
    {
    public:
        explicit global_locale_guard(const std::locale &replacement)
            : previous_(std::locale::global(replacement))
        {
        }

        ~global_locale_guard()
        {
            std::locale::global(previous_);
        }

        global_locale_guard(const global_locale_guard &) = delete;
        global_locale_guard &operator=(const global_locale_guard &) = delete;

    private:
        std::locale previous_;
    };

    void test_numeric_formatting_ignores_host_global_locale()
    {
        const std::locale comma_locale(std::locale::classic(), new comma_decimal_numpunct());
        global_locale_guard locale_guard(comma_locale);

        const auto set_callback = [](const std::string &name) {
            if (name == "DECIMALS") {
                return std::string{"2"};
            }
            if (name == "FIXED") {
                return std::string{"OFF"};
            }
            if (name == "POINT") {
                return std::string{"."};
            }
            if (name == "SEPARATOR") {
                return std::string{","};
            }
            return std::string{};
        };

        expect(
            copperfin::runtime::value_as_string(copperfin::runtime::make_number_value(1234.5)) == "1234.5",
            "#4832: ordinary numeric-to-string conversion should use invariant punctuation");
        expect(
            copperfin::runtime::value_as_string(copperfin::runtime::make_currency_value(12345678)) == "1234.5678" &&
                copperfin::runtime::value_as_string(copperfin::runtime::make_currency_value(-12345678)) == "-1234.5678" &&
                copperfin::runtime::value_as_string(copperfin::runtime::make_currency_value(0)) == "0.0000" &&
                copperfin::runtime::value_as_string(copperfin::runtime::make_currency_value(1)) == "0.0001",
            "#4839: currency stringification should ignore host digit grouping and preserve scaled values");

        const std::string display = copperfin::runtime::format_value_for_display(
            copperfin::runtime::make_number_value(1234.5),
            set_callback);
        expect(
            display == "1,234.5",
            "#4832: display formatting should apply VFP separators after invariant conversion, got " + display);

        const auto str = copperfin::runtime::evaluate_string_function(
            "str",
            {copperfin::runtime::make_number_value(1234.5),
             copperfin::runtime::make_number_value(10.0),
             copperfin::runtime::make_number_value(2.0)},
            false,
            80U,
            set_callback);
        expect(
            str.has_value() && copperfin::runtime::value_as_string(*str) == "   1234.50",
            "#4832: STR() should not inherit a host comma decimal separator");

        const auto transform = copperfin::runtime::evaluate_string_function(
            "transform",
            {copperfin::runtime::make_number_value(1234.5),
             copperfin::runtime::make_string_value("999,999.99")},
            false,
            80U,
            set_callback);
        expect(
            transform.has_value() && copperfin::runtime::value_as_string(*transform) == "1,234.50",
            "#4832: numeric TRANSFORM() pictures should parse invariant intermediate text");

        const auto digit_picture = copperfin::runtime::evaluate_string_function(
            "transform",
            {copperfin::runtime::make_number_value(1234.6),
             copperfin::runtime::make_string_value("999999")},
            false,
            80U,
            set_callback);
        expect(
            digit_picture.has_value() && copperfin::runtime::value_as_string(*digit_picture) == "  1235",
            "#4832: digit-only numeric pictures should not count a host decimal separator as a digit, got " +
                (digit_picture.has_value() ? copperfin::runtime::value_as_string(*digit_picture) : "<none>"));
    }

    void test_currency_stringification_ignores_grouping_locale()
    {
        const std::locale grouping_locale(std::locale::classic(), new comma_decimal_numpunct());
        global_locale_guard locale_guard(grouping_locale);

        expect(
            copperfin::runtime::value_as_string(copperfin::runtime::make_currency_value(123456789)) == "12345.6789",
            "#4839: positive currency stringification should not group the whole part under a host locale");
        expect(
            copperfin::runtime::value_as_string(copperfin::runtime::make_currency_value(-234565000)) == "-23456.5000",
            "#4839: negative currency stringification should preserve the invariant whole/fraction boundary");
        expect(
            copperfin::runtime::value_as_string(copperfin::runtime::make_currency_value(0)) == "0.0000",
            "#4839: zero currency stringification should preserve four fractional digits");
    }

    void test_string_and_math_expression_functions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_str_math";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "str_math.prg";
        write_text(
            main_path,
                "l = LEN('hello')\n"
            "lft = LEFT('hello', 3)\n"
            "rgt = RIGHT('hello', 2)\n"
            "up = UPPER('hello')\n"
            "lo = LOWER('WORLD')\n"
            "sp = LEN(SPACE(5))\n"
            "repl = REPLICATE('ab', 3)\n"
            "trimmed = LTRIM('  hi  ')\n"
            "rtrimmed = RTRIM('  hi  ')\n"
            "trim_basic = TRIM('  hi  ')\n"
            "ltrim_tab_pos = AT(CHR(9), LTRIM(' ' + CHR(9) + 'hi'))\n"
            "rtrim_lf_pos = AT(CHR(10), RTRIM('hi' + CHR(10) + ' '))\n"
            "alltrim_tab_pos = AT(CHR(9), ALLTRIM(' ' + CHR(9) + 'hi' + CHR(9) + ' '))\n"
            "alltrim_tab_len = LEN(ALLTRIM(' ' + CHR(9) + 'hi' + CHR(9) + ' '))\n"
            "a = ABS(-7)\n"
            "b = INT(3.9)\n"
            "c = MOD(10, 3)\n"
            "mod_negative_dividend = MOD(-7, 2)\n"
            "mod_negative_divisor = MOD(7, -2)\n"
            "mod_both_negative = MOD(-7, -2)\n"
            "mod_small_divisor = MOD(0.00000025, 0.0000005)\n"
            "mod_small_negative_divisor = MOD(0.00000025, -0.0000005)\n"
            "mod_below_former_cutoff = MOD(0.00000075, 0.000001)\n"
            "d = ROUND(3.567, 2)\n"
            "e = SIGN(-5)\n"
            "log10_value = LOG10(1000)\n"
            "sin_value = ROUND(SIN(DTOR(30)), 4)\n"
            "cos_value = ROUND(COS(DTOR(60)), 4)\n"
            "tan_value = ROUND(TAN(DTOR(45)), 4)\n"
            "asin_value = ROUND(RTOD(ASIN(0.5)), 4)\n"
            "acos_value = ROUND(RTOD(ACOS(0.5)), 4)\n"
            "atan_value = ROUND(RTOD(ATAN(1)), 4)\n"
            "atn2_value = ROUND(RTOD(ATN2(0, -1)), 4)\n"
            "dtor_value = ROUND(DTOR(180), 4)\n"
            "rtod_value = ROUND(RTOD(PI()), 4)\n"
            "min_num = MIN(8, 3, 5)\n"
            "max_num = MAX(8, 3, 5)\n"
            "min_str = MIN('beta', 'alpha', 'gamma')\n"
            "max_str = MAX('beta', 'alpha', 'gamma')\n"
            "rgb_black = RGB(0, 0, 0)\n"
            "rgb_sample = RGB(1, 2, 3)\n"
            "rgb_clamped = RGB(-1, 300, 255)\n"
            "rand_seeded = RAND(-123)\n"
            "rand_next = RAND()\n"
            "f = IIF(.T., 'yes', 'no')\n"
            "iif_true_guard = IIF(.T., 'guard-true', 1 / 0)\n"
            "iif_false_guard = IIF(.F., 1 / 0, 'guard-false')\n"
            "icase_first_guard = ICASE(.T., 'guard-first', .T., 1 / 0, 'otherwise')\n"
            "icase_second_guard = ICASE(.F., 1 / 0, .T., 'guard-second', 'otherwise')\n"
            "icase_null_condition = ICASE(.NULL., 'null-branch', .T., 'after-null', 'otherwise')\n"
            "icase_otherwise = ICASE(.F., 1 / 0, .NULL., 'ignored', 'otherwise')\n"
            "icase_no_match = ICASE(.F., 'miss')\n"
            "and_true = (1 = 1) AND (2 = 2)\n"
            "and_false = (1 = 1) AND (2 = 3)\n"
            "or_true = (1 = 2) OR (2 = 2)\n"
            "or_false = (1 = 2) OR (2 = 3)\n"
            "and_dotted = .T. .AND. .F.\n"
            "or_dotted = .F. .OR. .T.\n"
            "not_keyword = NOT (1 = 2)\n"
            "not_dotted = .NOT. (1 = 2)\n"
            "substr_hit = 'ab' $ 'abcdef'\n"
            "substr_miss = 'zz' $ 'abcdef'\n"
            "neq_hash = 1 # 2\n"
            "neq_bang_equal = 1 != 2\n"
            "pow_caret = 2 ^ 3\n"
            "pow_starstar = 2 ** 3\n"
            "pow_negative = 2 ^ -1\n"
            "pow_mul_mix = 2 * 2 ^ 3\n"
            "logic_precedence = .T. OR .F. AND .F.\n"
            "and_short_guard = .F. AND (1 / 0)\n"
            "or_short_guard = .T. OR (1 / 0)\n"
            "g = BETWEEN(5, 1, 10)\n"
            "h = OCCURS('l', 'hello world')\n"
            "v = VAL('42')\n"
            "val_plus = VAL('+5')\n"
            "val_exponent = VAL('1.25E3')\n"
            "currency_value = VAL('$12.34567')\n"
            "currency_negative = VAL('$-0.00005')\n"
            "currency_sum = currency_value + VAL('$0.0050')\n"
            "currency_product = VAL('$2.0000') * 1.5\n"
            "currency_division = VAL('$3.0000') / 2\n"
            "currency_compare = currency_value == VAL('$12.3457')\n"
            "currency_type = VARTYPE(currency_value)\n"
            "currency_type_expression = TYPE('currency_value')\n"
            "currency_empty = EMPTY(VAL('$0'))\n"
            "val_nan = VAL('nan')\n"
            "val_infinity = VAL('inf')\n"
            "val_hex = VAL('0x1A')\n"
            "at_second = AT('ha', 'ha ha ha', 2)\n"
            "rat_second = RAT('ha', 'ha ha ha', 2)\n"
            "rat_overlap_second = RAT('aa', 'aaaa', 2)\n"
            "rat_overlap_third = RAT('aa', 'aaaa', 3)\n"
            "atc_hit = ATC('FOX', 'red fox')\n"
            "ratc_hit = RATC('FOX', 'fox red fox')\n"
            "line_text = 'alpha' + CHR(13) + CHR(10) + 'Beta fox' + CHR(10) + 'gamma fox'\n"
            "atline_hit = ATLINE('fox', line_text)\n"
            "atcline_hit = ATCLINE('BETA', line_text)\n"
            "atline_second = ATLINE('fox', line_text, 2)\n"
            "ratline_hit = RATLINE('fox', line_text)\n"
            "chrtran_value = CHRTRAN('a1b2c3', '123', 'xyz')\n"
            "chrtranc_value = CHRTRANC('aAbBcc', 'AB', 'xy')\n"
            "chrtranc_delete = CHRTRANC('Alpha Beta', 'AE', 'x')\n"
            "strtran_start = STRTRAN('abcabcabc', 'abc', 'X', 2)\n"
            "strtran_count = STRTRAN('abcabcabc', 'abc', 'X', 2, 1)\n"
            "strtran_none = STRTRAN('abcabcabc', 'abc', 'X', 5, 1)\n"
            "strtran_remove = STRTRAN('a-b-c', '-')\n"
            "strtran_remove_no_match = STRTRAN('abc', '-')\n"
            "strtran_flags_sensitive = STRTRAN('Hello World', 'WORLD', 'There', 1, 1, 0)\n"
            "strtran_flags_insensitive = STRTRAN('Hello World', 'WORLD', 'There', 1, 1, 1)\n"
            "strtran_flags_start_count = STRTRAN('aBc ABC abc', 'abc', 'X', 2, 1, 1)\n"
            "strtran_negative_count = STRTRAN('abcabcabc', 'abc', 'X', 1, -1)\n"
            "strtran_negative_count_flags = STRTRAN('aBc ABC abc', 'abc', 'X', 1, -1, 1)\n"
            "proper_value = PROPER('legacy fox-pro APP')\n"
            "strconv_lower = STRCONV('MiXeD', 7)\n"
            "strconv_upper = STRCONV('MiXeD', 8)\n"
            "strconv_passthrough = STRCONV('MiXeD', 1)\n"
            "soundex_tamar = SOUNDEX('Tamar')\n"
            "soundex_ted = SOUNDEX('ted')\n"
            "soundex_smith = SOUNDEX('Smith')\n"
            "soundex_schmidt = SOUNDEX('Schmidt')\n"
            "difference_tamar_ted = DIFFERENCE('tamar', 'ted')\n"
            "difference_ted_teddy = DIFFERENCE('ted', 'teddy')\n"
            "str_default = STR(42.7)\n"
            "str_width = STR(42, 5)\n"
            "str_decimals = STR(42.678, 8, 2)\n"
            "str_overflow = STR(123456, 3)\n"
            "padl_default = PADL('fox', 5)\n"
            "padr_default = PADR('fox', 5)\n"
            "padc_default = PADC('fox', 7)\n"
            "padl_truncate = PADL('abcdef', 3)\n"
            "padr_truncate = PADR('abcdef', 3)\n"
            "padc_truncate = PADC('abcdef', 3)\n"
            "pad_custom = PADL('7', 3, '0')\n"
            "like_hit = LIKE('a?c*', 'abc legacy')\n"
            "like_miss = LIKE('A?D*', 'abc legacy')\n"
            "like_case_miss_exact_default = LIKE('A?C*', 'abc legacy')\n"
            "inlist_hit = INLIST('beta', 'alpha', 'beta', 'gamma')\n"
            "inlist_miss = INLIST(4, 1, 2, 3)\n"
            "SET EXACT ON\n"
            "like_case_miss_exact_on = LIKE('A?C*', 'abc legacy')\n"
            "inlist_exact_leading_miss = INLIST(' beta', 'beta')\n"
            "inlist_exact_leading_hit = INLIST(' beta', ' beta')\n"
            "inlist_exact_trailing_miss = INLIST('beta  ', 'beta')\n"
            "inlist_exact_all_space_miss = INLIST('   ', '')\n"
            "inlist_exact_tab_miss = INLIST('beta' + CHR(9), 'beta')\n"
            "inlist_exact_nul_miss = INLIST('beta' + CHR(0), 'beta')\n"
            "SET EXACT OFF\n"
            "like_case_miss_exact_off = LIKE('A?C*', 'abc legacy')\n"
            "inlist_off_prefix_hit = INLIST('alphabet', 'alpha')\n"
            "inlist_off_reverse_miss = INLIST('alpha', 'alphabet')\n"
            "getwordcount_1 = GETWORDCOUNT('one two three')\n"
            "getwordcount_2 = GETWORDCOUNT('a,b,c', ',')\n"
            "getwordcount_multi_delim = GETWORDCOUNT('a,b;c', ',;')\n"
            "getwordcount_tab = GETWORDCOUNT('one' + CHR(9) + 'two')\n"
            "getwordnum_1 = GETWORDNUM('one two three', 2)\n"
            "getwordnum_2 = GETWORDNUM('a,b,c', 3, ',')\n"
            "getwordnum_multi_delim = GETWORDNUM('a,b;c', 3, ',;')\n"
            "getwordnum_tab = GETWORDNUM('one' + CHR(9) + 'two', 2)\n"
            "getwordnum_zero = GETWORDNUM('one two', 0)\n"
            "getwordnum_negative = GETWORDNUM('one two', -1)\n"
            "getwordnum_out_of_range = GETWORDNUM('one two', 3)\n"
            "memo_text = 'first line' + CHR(13) + CHR(10) + 'second line' + CHR(10) + 'still second' + CHR(13) + 'third line'\n"
            "memline_count = MEMLINES(memo_text)\n"
            "mline_one = MLINE(memo_text, 1)\n"
            "mline_two = MLINE(memo_text, 2)\n"
            "mline_three = MLINE(memo_text, 3)\n"
            "mline_missing = MLINE(memo_text, 4)\n"
            "wrap_text = 'one two three four five six seven eight nine ten eleven twelve'\n"
            "wrap_count = MEMLINES(wrap_text)\n"
            "wrap_count_width8 = MEMLINES(wrap_text, 8)\n"
            "wrap_count_width0 = MEMLINES(wrap_text, 0)\n"
            "wrap_first = MLINE(wrap_text, 1)\n"
            "wrap_second = MLINE(wrap_text, 2)\n"
            "wrap_second_width8 = MLINE(wrap_text, 2, 0, 8)\n"
            "offset_line = MLINE(memo_text, 1, 12)\n"
            "tab_text = 'aa' + CHR(9) + 'bb' + CHR(9) + 'cc'\n"
            "tab_default = MLINE(tab_text, 1, 0, 40)\n"
            "tab_expanded = MLINE(tab_text, 1, 0, 40, 4)\n"
            "tab_count_expanded = MEMLINES(tab_text, 40, 4)\n"
            "lf_text = 'alpha' + CHR(10) + 'beta' + CHR(10) + 'gamma'\n"
            "lf_count_default = MEMLINES(lf_text, 80, 4, 0)\n"
            "lf_count_break = MEMLINES(lf_text, 80, 4, 1)\n"
            "lf_second_default = MLINE(lf_text, 2, 0, 80, 4, 0)\n"
            "lf_second_break = MLINE(lf_text, 2, 0, 80, 4, 1)\n"
            "memo_width_set_default = SET('MEMOWIDTH')\n"
            "nMemoWidthCalls = 0\n"
            "SET MEMOWIDTH TO memo_width_resolver(10)\n"
            "memo_width_value = _MLINE\n"
            "memo_width_set_after = SET('MEMOWIDTH')\n"
            "narrow_wrap_text = 'abc def ghi jkl mno'\n"
            "narrow_count = MEMLINES(narrow_wrap_text)\n"
            "narrow_first = MLINE(narrow_wrap_text, 1)\n"
            "narrow_second = MLINE(narrow_wrap_text, 2)\n"
            "narrow_third = MLINE(narrow_wrap_text, 3)\n"
            "strextract_case_sensitive = STREXTRACT('<Name>Beta</Name>', '<name>', '</name>')\n"
            "strextract_case_insensitive = STREXTRACT('<Name>Beta</Name>', '<name>', '</name>', 1, 1)\n"
            "strextract_empty_begin_first = STREXTRACT('hello world', '', 'o')\n"
            "strextract_empty_begin_later = STREXTRACT('hello world', '', 'o', 2)\n"
            "strextract_missing_end_default = STREXTRACT('a=one;b=two', 'b=', ';')\n"
            "strextract_missing_end_allowed = STREXTRACT('a=one;b=two', 'b=', ';', 1, 2)\n"
            "strextract_include_delims = STREXTRACT('<id>42</id>', '<id>', '</id>', 1, 4)\n"
            "stuff_zero_start = STUFF('abcdef', 0, 2, 'XY')\n"
            "stuff_negative_start = STUFF('abcdef', -3, 4, 'XY')\n"
            "stuff_insert_only = STUFF('abcdef', 3, 0, 'XY')\n"
            "stuff_clamped_length = STUFF('abcdef', 5, 99, 'XY')\n"
            "substr_basic = SUBSTR('hello', 2, 3)\n"
            "substr_no_len = SUBSTR('hello', 3)\n"
            "alltrim_basic = ALLTRIM('  hi  ')\n"
            "transform_decimal = TRANSFORM(3.14159, '9.9')\n"
            "point_default = SET('POINT')\n"
            "separator_default = SET('SEPARATOR')\n"
            "currency_default = SET('CURRENCY')\n"
            "transform_group_default = TRANSFORM(1234.5, '999,999.99')\n"
            "transform_currency_default = TRANSFORM(1234.5, '$999,999.99')\n"
            "SET POINT TO ','\n"
            "SET SEPARATOR TO '.'\n"
            "SET CURRENCY TO 'USD '\n"
            "point_after = SET('POINT')\n"
            "separator_after = SET('SEPARATOR')\n"
            "currency_after = SET('CURRENCY')\n"
            "SET DATASESSION TO 2\n"
            "memo_width_session2 = SET('MEMOWIDTH')\n"
            "point_session2 = SET('POINT')\n"
            "separator_session2 = SET('SEPARATOR')\n"
            "currency_session2 = SET('CURRENCY')\n"
            "SET DATASESSION TO 1\n"
            "memo_width_restored = SET('MEMOWIDTH')\n"
            "point_restored = SET('POINT')\n"
            "separator_restored = SET('SEPARATOR')\n"
            "currency_restored = SET('CURRENCY')\n"
            "transform_group_euro = TRANSFORM(1234.5, '999,999.99')\n"
            "transform_currency_custom = TRANSFORM(1234.5, '$999,999.99')\n"
            "transform_decimal_euro = TRANSFORM(3.14159, '9.9')\n"
            "transform_literal_phone = TRANSFORM(5551234567, '@R (999) 999-9999')\n"
            "transform_literal_id = TRANSFORM('12-34-5678', '@R 99/99/9999')\n"
            "transform_group_integer = TRANSFORM(1234567, '999,999')\n"
            "transform_upper = TRANSFORM('hello', '@!')\n"
            "SET DECIMALS TO 4\n"
            "display_fraction = 1 / 3\n"
            "display_transform = TRANSFORM(display_fraction)\n"
            "display_concat = 'value=' + TRANSFORM(display_fraction)\n"
            "display_group = TRANSFORM(12345.6789)\n"
            "SET FIXED ON\n"
            "fixed_after = SET('FIXED')\n"
            "display_fixed = TRANSFORM(1.5)\n"
            "RETURN\n"
            "FUNCTION memo_width_resolver\n"
            "LPARAMETERS value\n"
            "nMemoWidthCalls = nMemoWidthCalls + 1\n"
            "RETURN value\n"
            "ENDFUNC\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "string/math function script should complete");

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
        };

        const auto check_number = [&](const std::string &name, const double expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            const double actual = copperfin::runtime::value_as_number(it->second);
            expect(std::abs(actual - expected) < 1.0e-15,
                   name + " expected numeric value " + std::to_string(expected) + " got " + std::to_string(actual));
        };

        check("l", "5");
        check("lft", "hel");
        check("rgt", "lo");
        check("up", "HELLO");
        check("lo", "world");
        check("sp", "5");
        check("repl", "ababab");
        check("trimmed", "hi  ");
        check("rtrimmed", "  hi");
        check("trim_basic", "  hi");
        check("ltrim_tab_pos", "1");
        check("rtrim_lf_pos", "3");
        check("alltrim_tab_pos", "1");
        check("alltrim_tab_len", "4");
        check("a", "7");
        check("b", "3");
        check("c", "1");
        check("d", "3.57");
        check("e", "-1");
        check("log10_value", "3");
        check("sin_value", "0.5");
        check("cos_value", "0.5");
        check("tan_value", "1");
        check("asin_value", "30");
        check("acos_value", "60");
        check("atan_value", "45");
        check("atn2_value", "180");
        check("dtor_value", "3.1416");
        check("rtod_value", "180");
        check("min_num", "3");
        check("max_num", "8");
        check("min_str", "alpha");
        check("max_str", "gamma");
        check("rgb_black", "0");
        check("rgb_sample", "197121");
        check("rgb_clamped", "16776960");
        check("f", "yes");
        check("iif_true_guard", "guard-true");
        check("iif_false_guard", "guard-false");
        check("icase_first_guard", "guard-first");
        check("icase_second_guard", "guard-second");
        check("icase_null_condition", "after-null");
        check("icase_otherwise", "otherwise");
        check("and_true", "true");
        check("and_false", "false");
        check("or_true", "true");
        check("or_false", "false");
        check("and_dotted", "false");
        check("or_dotted", "true");
        check("not_keyword", "true");
        check("not_dotted", "true");
        check("substr_hit", "true");
        check("substr_miss", "false");
        check("neq_hash", "true");
        check("neq_bang_equal", "true");
        check("pow_caret", "8");
        check("pow_starstar", "8");
        check("pow_negative", "0.5");
        check("pow_mul_mix", "16");
        check("logic_precedence", "true");
        check("and_short_guard", "false");
        check("or_short_guard", "true");
        check("g", "true");
        check("h", "3");
        check("v", "42");
        check("val_plus", "5");
        check("val_exponent", "1250");
        check("currency_value", "12.3457");
        check("currency_negative", "-0.0001");
        check("currency_sum", "12.3507");
        check("currency_product", "3.0000");
        check("currency_division", "1.5000");
        check("currency_compare", "true");
        check("currency_type", "Y");
        check("currency_type_expression", "Y");
        check("currency_empty", "true");
        const auto currency_value = state.globals.find("currency_value");
        expect(currency_value != state.globals.end() &&
                   currency_value->second.kind == copperfin::runtime::PrgValueKind::currency &&
                   currency_value->second.currency_value == 123457,
               "VAL() with a leading dollar should preserve a fixed-point Currency value");
        check("val_nan", "0");
        check("val_infinity", "0");
        check("val_hex", "0");
        check("at_second", "4");
        check("rat_second", "4");
        check("rat_overlap_second", "1");
        check("rat_overlap_third", "0");
        check("atc_hit", "5");
        check("ratc_hit", "9");
        check("atline_hit", "2");
        check("atcline_hit", "2");
        check("atline_second", "3");
        check("ratline_hit", "3");
        check("chrtran_value", "axbycz");
        check("chrtranc_value", "xxyycc");
        check("chrtranc_delete", "xlphx Btx");
        check("strtran_start", "abcXX");
        check("strtran_count", "abcXabc");
        check("strtran_none", "abcabcabc");
        check("strtran_remove", "abc");
        check("strtran_remove_no_match", "abc");
        check("strtran_flags_sensitive", "Hello World");
        check("strtran_flags_insensitive", "Hello There");
        check("strtran_flags_start_count", "aBc X abc");
        check("strtran_negative_count", "XXX");
        check("strtran_negative_count_flags", "X X X");
        check("proper_value", "Legacy Fox-Pro App");
        check("strconv_lower", "mixed");
        check("strconv_upper", "MIXED");
        check("strconv_passthrough", "MiXeD");
        check("soundex_tamar", "T560");
        check("soundex_ted", "T300");
        check("soundex_smith", "S530");
        check("soundex_schmidt", "S530");
        check("difference_tamar_ted", "2");
        check("difference_ted_teddy", "4");
        check("str_default", "        43");
        check("str_width", "   42");
        check("str_decimals", "   42.68");
        check("str_overflow", "***");
        check("padl_default", "  fox");
        check("padr_default", "fox  ");
        check("padc_default", "  fox  ");
        check("padl_truncate", "def");
        check("padr_truncate", "abc");
        check("padc_truncate", "bcd");
        check("pad_custom", "007");
        check("like_hit", "true");
        check("like_miss", "false");
        check("like_case_miss_exact_default", "false");
        check("like_case_miss_exact_on", "false");
        check("like_case_miss_exact_off", "false");
        check("inlist_hit", "true");
        check("inlist_miss", "false");
        check("inlist_exact_leading_miss", "false");
        check("inlist_exact_leading_hit", "true");
        check("inlist_exact_trailing_miss", "false");
        check("inlist_exact_all_space_miss", "false");
        check("inlist_exact_tab_miss", "false");
        check("inlist_exact_nul_miss", "false");
        check("inlist_off_prefix_hit", "true");
        check("inlist_off_reverse_miss", "false");
        check("getwordcount_1", "3");
        check("getwordcount_2", "3");
        check("getwordcount_multi_delim", "3");
        check("getwordcount_tab", "2");
        check("getwordnum_1", "two");
        check("getwordnum_2", "c");
        check("getwordnum_multi_delim", "c");
        check("getwordnum_tab", "two");
        check("getwordnum_zero", "");
        check("getwordnum_negative", "");
        check("getwordnum_out_of_range", "");
        check("memline_count", "3");
        check("mline_one", "first line");
        check("mline_two", "second line\nstill second");
        check("mline_three", "third line");
        check("mline_missing", "");
        check("wrap_count", "2");
        check("wrap_count_width8", "11");
        check("wrap_count_width0", "2");
        check("wrap_first", "one two three four five six seven eight nine ten");
        check("wrap_second", "eleven twelve");
        check("wrap_second_width8", "three");
        check("offset_line", "second line\nstill second");
        check("tab_default", "aa\tbb\tcc");
        check("tab_expanded", "aa  bb  cc");
        check("tab_count_expanded", "1");
        check("lf_count_default", "1");
        check("lf_count_break", "3");
        check("lf_second_default", "");
        check("lf_second_break", "beta");
        check("memo_width_set_default", "50");
        check("memo_width_value", "10");
        check("memo_width_set_after", "10");
        check("nmemowidthcalls", "1");
        check("memo_width_session2", "50");
        check("memo_width_restored", "10");
        check("narrow_count", "3");
        check("narrow_first", "abc def");
        check("narrow_second", "ghi jkl");
        check("narrow_third", "mno");
        check("mod_negative_dividend", "1");
        check("mod_negative_divisor", "-1");
        check("mod_both_negative", "-1");
        check_number("mod_small_divisor", 0.00000025);
        check_number("mod_small_negative_divisor", -0.00000025);
        check_number("mod_below_former_cutoff", 0.00000075);
        check("strextract_case_sensitive", "");
        check("strextract_case_insensitive", "Beta");
        check("strextract_empty_begin_first", "hell");
        check("strextract_empty_begin_later", "");
        check("strextract_missing_end_default", "");
        check("strextract_missing_end_allowed", "two");
        check("strextract_include_delims", "<id>42</id>");
        check("stuff_zero_start", "XYabcdef");
        check("stuff_negative_start", "XYabcdef");
        check("stuff_insert_only", "abXYcdef");
        check("stuff_clamped_length", "abcdXY");
        check("substr_basic", "ell");
        check("substr_no_len", "llo");
        check("alltrim_basic", "hi");
        check("transform_decimal", "3.1");
        check("point_default", ".");
        check("separator_default", ",");
        check("currency_default", "$");
        check("point_session2", ".");
        check("separator_session2", ",");
        check("currency_session2", "$");
        check("transform_group_default", "1,234.50");
        check("transform_currency_default", "$1,234.50");
        check("point_after", ",");
        check("separator_after", ".");
        check("currency_after", "USD ");
        check("point_restored", ",");
        check("separator_restored", ".");
        check("currency_restored", "USD ");
        check("transform_group_euro", "1.234,50");
        check("transform_currency_custom", "USD 1.234,50");
        check("transform_decimal_euro", "3,1");
        check("transform_literal_phone", "(555) 123-4567");
        check("transform_literal_id", "12/34/5678");
        check("transform_group_integer", "1.234.567");
        check("transform_upper", "HELLO");
        check("display_transform", "0,3333");
        check("display_concat", "value=0,3333");
        check("display_group", "12.345,6789");
        check("fixed_after", "ON");
        check("display_fixed", "1,5000");

        for (const char *name : {"rand_seeded", "rand_next"})
        {
            const std::string name_text{name};
            const auto it = state.globals.find(name_text);
            if (it == state.globals.end())
            {
                expect(false, name_text + " variable not found");
                continue;
            }
            const double value = copperfin::platform::try_parse_invariant_double(
                                     copperfin::runtime::format_value(it->second))
                                     .value_or(-1.0);
            expect(value >= 0.0 && value < 1.0, name_text + " should be in the RAND() range [0, 1)");
        }

        const auto icase_no_match = state.globals.find("icase_no_match");
        expect(icase_no_match != state.globals.end(),
               "#3746: ICASE without an otherwise result should still assign a variable");
        if (icase_no_match != state.globals.end())
        {
            expect(icase_no_match->second.is_null &&
                       icase_no_match->second.kind == copperfin::runtime::PrgValueKind::empty,
                   "#3746: ICASE without an otherwise result should return .NULL. when no condition matches");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_index_expression_trim_functions_preserve_non_space_whitespace()
    {
        copperfin::vfp::DbfRecord record;

        expect(
            copperfin::runtime::evaluate_index_expression("ALLTRIM(' \tALPHA\t ')", record) == "\tALPHA\t",
            "index-expression ALLTRIM() should trim only outer spaces and preserve tabs");
        expect(
            copperfin::runtime::evaluate_index_expression("LTRIM(' \tALPHA\t ')", record) == "\tALPHA\t ",
            "index-expression LTRIM() should trim only leading spaces and preserve tabs");
        expect(
            copperfin::runtime::evaluate_index_expression("RTRIM(' \tALPHA\t ')", record) == " \tALPHA\t",
            "index-expression RTRIM() should trim only trailing spaces and preserve tabs");
    }

    void test_index_expression_padl_truncation_matches_runtime_padl()
    {
        copperfin::vfp::DbfRecord bravo_record;
        bravo_record.values.push_back({"NAME", 'C', false, "BRAVO", 0});

        copperfin::vfp::DbfRecord charlie_record;
        charlie_record.values.push_back({"NAME", 'C', false, "CHARLIE", 0});

        expect(
            copperfin::runtime::evaluate_index_expression("UPPER(PADL(NAME, 3))", bravo_record) == "AVO",
            "index-expression PADL() should keep the rightmost characters when truncating BRAVO to width 3");
        expect(
            copperfin::runtime::evaluate_index_expression("UPPER(PADL(NAME, 3))", charlie_record) == "LIE",
            "index-expression PADL() should keep the rightmost characters when truncating CHARLIE to width 3");
    }

    void test_financial_and_misc_expression_functions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_financial_misc";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "fin.prg";
        write_text(
            main_path,
            // HEX
            "cHexZero    = HEX(0)\n"
            "cHex10      = HEX(10)\n"
            "cHex255     = HEX(255)\n"
            "cHex65536   = HEX(65536)\n"
            // Financial: PAYMENT
            "nPayment = ROUND(PAYMENT(1000, 0.01, 12), 4)\n"
            // Financial: FV (save for 10 periods at 5% with $100 payment, PV=0)
            "nFV = ROUND(FV(0.05, 10, -100), 4)\n"
            // Financial: PV (present value of 12 payments of $100 at 1% per period)
            "nPV = ROUND(PV(0.01, 12, -100), 4)\n"
            // FV zero-rate edge case
            "nFVZeroRate = FV(0, 5, -200)\n"
            // TEXTMERGE basic
            "cName = 'World'\n"
            "cMerged = TEXTMERGE('Hello <<cName>>!')\n"
            // TEXTMERGE custom delimiters
            "cMergedCustom = TEXTMERGE('Value={|1+1|}', .F., '{|', '|}')\n"
            "cFieldExpr = 'cName'\n"
            "cFieldExprHolder = 'cFieldExpr'\n"
            "cFieldExprDeepHolder = 'cFieldExprHolder'\n"
            "cNameExpr = 'cName'\n"
            "cNameExprHolder = 'cNameExpr'\n"
            "cNameExprDeepHolder = 'cNameExprHolder'\n"
            "cTemplateText = 'Template <<cName>>'\n"
            "cTemplateExpr = 'cTemplateText'\n"
            "cTemplateHolder = 'cTemplateExpr'\n"
            "cTemplateDeepHolder = 'cTemplateHolder'\n"
            "cMergedMacroSource = TEXTMERGE(&cTemplateExpr)\n"
            "cMergedMacroSourceSecondHop = TEXTMERGE(&cTemplateDeepHolder)\n"
            "cLiteralAngleExpr = '\"A<<B\"'\n"
            "cLiteralAngleExprHolder = 'cLiteralAngleExpr'\n"
            "cLiteralAngleMacro = &cLiteralAngleExpr\n"
            "cLiteralAngleMacroSecondHop = &cLiteralAngleExprHolder\n"
            "cRecursiveCustomExpr = '{|EVAL(&cNameExprDeepHolder)|}'\n"
            "cRecursiveCustomExprHolder = 'cRecursiveCustomExpr'\n"
            "cRecursiveCustomExprDeepHolder = 'cRecursiveCustomExprHolder'\n"
            "cMergedCustomNested = TEXTMERGE('Eval={|EVAL(cNameExpr)|}; Macro={|&cFieldExpr|}; Recursive={|cRecursiveCustomExpr|}', .T., '{|', '|}')\n"
            "cMergedCustomSecondHop = TEXTMERGE('Eval={|EVAL(&cNameExprDeepHolder)|}; Macro={|&cFieldExprDeepHolder|}; Recursive={|&cRecursiveCustomExprDeepHolder|}', .T., '{|', '|}')\n"
            "cLeftDelim = '{{'\n"
            "cRightDelim = '}}'\n"
            "cLeftDelimExpr = 'cLeftDelim'\n"
            "cRightDelimExpr = 'cRightDelim'\n"
            "cLeftDelimHolder = 'cLeftDelimExpr'\n"
            "cRightDelimHolder = 'cRightDelimExpr'\n"
            "cLeftDelimDeepHolder = 'cLeftDelimHolder'\n"
            "cRightDelimDeepHolder = 'cRightDelimHolder'\n"
            "cMergedMacroDelims = TEXTMERGE('Eval={{EVAL(cNameExpr)}}; Macro={{&cFieldExpr}}', .F., &cLeftDelimExpr, &cRightDelimExpr)\n"
            "cMergedMacroDelimsSecondHop = TEXTMERGE('Eval={{EVAL(&cNameExprDeepHolder)}}; Macro={{&cFieldExprDeepHolder}}', .F., &cLeftDelimDeepHolder, &cRightDelimDeepHolder)\n"
            // TEXTMERGE no delimiters found
            "cMergedPlain = TEXTMERGE('no markers here')\n"
            // TEXTMERGE recursive nested placeholders
            "cStage1 = '<<cStage2>>'\n"
            "cStage2 = '<<cStage3>>'\n"
            "cStage3 = 'done'\n"
            "cMergedRecursive = TEXTMERGE('Value: <<cStage1>>', .T.)\n"
            // EXECSCRIPT simple RETURN
            "nExecResult = EXECSCRIPT('RETURN 7 + 3')\n"
            // EXECSCRIPT RETURN string
            "cExecStr = EXECSCRIPT('RETURN LEFT(\"hello\", 3)')\n"
            "nExecBase = 6\n"
            "cExecReturnExpr = 'nExecBase * 4'\n"
            "cExecReturnHolder = 'cExecReturnExpr'\n"
            "cExecReturnDeepHolder = 'cExecReturnHolder'\n"
            "nExecMacroReturn = EXECSCRIPT('RETURN &cExecReturnExpr')\n"
            "nExecMacroReturnSecondHop = EXECSCRIPT('RETURN &cExecReturnDeepHolder')\n"
            "cExecScriptText = 'RETURN nExecBase + 5'\n"
            "cExecScriptExpr = 'cExecScriptText'\n"
            "cExecScriptHolder = 'cExecScriptExpr'\n"
            "cExecScriptDeepHolder = 'cExecScriptHolder'\n"
            "nExecMacroSource = EXECSCRIPT(&cExecScriptExpr)\n"
            "nExecMacroSourceSecondHop = EXECSCRIPT(&cExecScriptDeepHolder)\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "financial/misc function script should complete");

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            const std::string actual = copperfin::runtime::format_value(it->second);
            expect(actual == expected, name + ": expected \"" + expected + "\", got \"" + actual + "\"");
        };

        check("chexzero",  "0");
        check("chex10",    "A");
        check("chex255",   "FF");
        check("chex65536", "10000");
        check("npayment",  "88.8488");
        check("nfv",       "1257.79");
        check("npv",       "1125.51");
        check("nfvzerorate", "1000");
        check("cmerged",       "Hello World!");
        check("cmergedcustom", "Value=2");
        check("cmergedmacrosource", "Template World");
        check("cmergedmacrosourcesecondhop", "Template World");
        check("cliteralanglemacro", "A<<B");
        check("cliteralanglemacrosecondhop", "A<<B");
        check("cmergedcustomnested", "Eval=World; Macro=World; Recursive=World");
        check("cmergedcustomsecondhop", "Eval=World; Macro=World; Recursive=World");
        check("cmergedmacrodelims", "Eval=World; Macro=World");
        check("cmergedmacrodelimssecondhop", "Eval=World; Macro=World");
        check("cmergedplain",  "no markers here");
        check("cmergedrecursive", "Value: done");
        check("nexecresult",   "10");
        check("cexecstr",      "hel");
        check("nexecmacroreturn", "24");
        check("nexecmacroreturnsecondhop", "24");
        check("nexecmacrosource", "11");
        check("nexecmacrosourcesecondhop", "11");

        fs::remove_all(temp_root, ignored);
    }

    void test_textmerge_set_state_and_delimiters()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_textmerge_set_state";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "textmerge_set.prg";
        write_text(
            main_path,
            "cTextmergeDefault = SET('TEXTMERGE')\n"
            "cDelimsDefault = SET('TEXTMERGE', 1)\n"
            "SET TEXTMERGE ON NOSHOW\n"
            "cTextmergeOn = SET('TEXTMERGE')\n"
            "SET TEXTMERGE DELIMITERS TO '{|', '|}'\n"
            "cDelimsCustom = SET('TEXTMERGE', 1)\n"
            "cMergedFromSet = TEXTMERGE('Value={|1+1|}')\n"
            "SET DATASESSION TO 2\n"
            "cTextmergeSession2 = SET('TEXTMERGE')\n"
            "cDelimsSession2 = SET('TEXTMERGE', 1)\n"
            "SET DATASESSION TO 1\n"
            "cDelimsRestored = SET('TEXTMERGE', 1)\n"
            "SET TEXTMERGE DELIMITERS TO '<@'\n"
            "cDelimsShared = SET('TEXTMERGE', 1)\n"
            "cMergedShared = TEXTMERGE('Shared=<@3+4<@')\n"
            "SET TEXTMERGE DELIMITERS TO\n"
            "cDelimsResetTo = SET('TEXTMERGE', 1)\n"
            "cMergedResetTo = TEXTMERGE('ResetTo=<<2+1>>')\n"
            "SET TEXTMERGE DELIMITERS TO '[', ']'\n"
            "SET TEXTMERGE DELIMITERS\n"
            "cDelimsResetBare = SET('TEXTMERGE', 1)\n"
            "cMergedResetBare = TEXTMERGE('ResetBare=<<4+1>>')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "SET TEXTMERGE delimiter script should complete");

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            const std::string actual = copperfin::runtime::format_value(it->second);
            expect(actual == expected, name + ": expected \"" + expected + "\", got \"" + actual + "\"");
        };

        check("ctextmergedefault", "OFF");
        check("cdelimsdefault", "<<,>>");
        check("ctextmergeon", "ON");
        check("cdelimscustom", "{|,|}");
        check("cmergedfromset", "Value=2");
        check("ctextmergesession2", "OFF");
        check("cdelimssession2", "<<,>>");
        check("cdelimsrestored", "{|,|}");
        check("cdelimsshared", "<@,<@");
        check("cmergedshared", "Shared=7");
        check("cdelimsresetto", "<<,>>");
        check("cmergedresetto", "ResetTo=3");
        check("cdelimsresetbare", "<<,>>");
        check("cmergedresetbare", "ResetBare=5");

        fs::remove_all(temp_root, ignored);
    }

    void test_nested_macro_eval_textmerge_execscript_semantics()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_nested_macro_eval_surfaces";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "nested_macro_eval_surfaces.prg";
        write_text(
            main_path,
            "cName = 'Copperfin'\n"
            "cEvalExpr = 'LEFT(cName, 9)'\n"
            "cEvalExprHolder = 'cEvalExpr'\n"
            "cEvalExprDeepHolder = 'cEvalExprHolder'\n"
            "cEvalNested = EVAL(EVAL(cEvalExprDeepHolder))\n"
            "cEvalNestedSecondHop = EVAL(&cEvalExprDeepHolder)\n"
            "cMergedFromEval = TEXTMERGE('Hello <<EVAL(cEvalExpr)>>!')\n"
            "cMergedFromEvalSecondHop = TEXTMERGE('Hello <<EVAL(&cEvalExprDeepHolder)>>!')\n"
            "nBase = 10\n"
            "cScriptExpr = 'nBase + 2'\n"
            "cScriptExprHolder = 'cScriptExpr'\n"
            "cScriptExprDeepHolder = 'cScriptExprHolder'\n"
            "cScriptText = 'RETURN EVAL(&cScriptExprDeepHolder)'\n"
            "cScriptHolder = 'cScriptText'\n"
            "cScriptDeepHolder = 'cScriptHolder'\n"
            "nExecNested = EXECSCRIPT(&cScriptHolder)\n"
            "nExecNestedSecondHop = EXECSCRIPT(&cScriptDeepHolder)\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "nested macro/eval surface script should complete");

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            const std::string actual = copperfin::runtime::format_value(it->second);
            expect(actual == expected, name + ": expected \"" + expected + "\", got \"" + actual + "\"");
        };

        check("cevalnested", "Copperfin");
        check("cevalnestedsecondhop", "Copperfin");
        check("cmergedfromeval", "Hello Copperfin!");
        check("cmergedfromevalsecondhop", "Hello Copperfin!");
        check("nexecnested", "12");
        check("nexecnestedsecondhop", "12");

        fs::remove_all(temp_root, ignored);
    }

    void test_numeric_domain_errors_route_through_runtime_catalog()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_numeric_domain_errors";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const auto run_fault = [&](const std::string &script_name, const std::string &source)
        {
            const fs::path main_path = temp_root / script_name;
            write_text(main_path, source);
            copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
            return session.run(copperfin::runtime::DebugResumeAction::continue_run);
        };

        const auto log_state = run_fault("log_domain.prg", "n = LOG(0)\nRETURN\n");
        expect(!log_state.completed, "LOG(0) should pause with a runtime error");
        expect(log_state.message == "Runtime fault: LOG() requires a positive argument (got 0.000000)",
               "#2540: LOG() domain error should route through the default locale catalog");

        const auto log10_state = run_fault("log10_domain.prg", "n = LOG10(-1)\nRETURN\n");
        expect(!log10_state.completed, "LOG10(-1) should pause with a runtime error");
        expect(log10_state.message == "Runtime fault: LOG10() requires a positive argument (got -1.000000)",
               "#2540: LOG10() domain error should preserve function and value placeholders");

        const auto asin_state = run_fault("asin_domain.prg", "n = ASIN(2)\nRETURN\n");
        expect(!asin_state.completed, "ASIN(2) should pause with a runtime error");
        expect(asin_state.message == "Runtime fault: ASIN() requires an argument between -1 and 1 (got 2.000000)",
               "#2540: ASIN() domain error should route through the default locale catalog");

        const auto acos_state = run_fault("acos_domain.prg", "n = ACOS(-2)\nRETURN\n");
        expect(!acos_state.completed, "ACOS(-2) should pause with a runtime error");
        expect(acos_state.message == "Runtime fault: ACOS() requires an argument between -1 and 1 (got -2.000000)",
               "#2540: ACOS() domain error should preserve function and value placeholders");

        const auto sqrt_state = run_fault("sqrt_domain.prg", "n = SQRT(-1)\nRETURN\n");
        expect(!sqrt_state.completed, "SQRT(-1) should pause with a runtime error");
        expect(sqrt_state.message == "Runtime fault: SQRT() requires a non-negative argument (got -1.000000)",
               "#4268: SQRT() domain error should route through the default locale catalog");

        const auto sqrt_valid_state = run_fault(
            "sqrt_valid.prg",
            "nZero = SQRT(0)\n"
            "nPositive = SQRT(9)\n"
            "RETURN\n");
        expect(sqrt_valid_state.completed,
               "#4268: SQRT() should retain valid non-negative behavior: " + sqrt_valid_state.message);
        expect(copperfin::runtime::format_value(sqrt_valid_state.globals.at("nzero")) == "0",
               "#4268: SQRT(0) should remain zero");
        expect(copperfin::runtime::format_value(sqrt_valid_state.globals.at("npositive")) == "3",
               "#4268: SQRT(9) should remain three");

        fs::remove_all(temp_root, ignored);
    }

    void test_round_uses_decimal_half_away_from_zero_behavior()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_round_decimal_edges";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "round_edges.prg";
        write_text(
            main_path,
            "nBinary = ROUND(1.005, 2)\n"
            "nBinarySmall = ROUND(0.145, 2)\n"
            "nPositiveTie = ROUND(2.5, 0)\n"
            "nNegativeTie = ROUND(-2.5, 0)\n"
            "nNegativePlaces = ROUND(1234.5678, -2)\n"
            "nThreePlaces = ROUND(1.2345, 3)\n"
            "nCarry = ROUND(9.995, 2)\n"
            "nNegativeCarry = ROUND(-9.995, 2)\n"
            "nSmallTie = ROUND(0.0005, 3)\n"
            "nBelowNegativePlace = ROUND(0.004, -2)\n"
            "nBelowNegativeTie = ROUND(149, -2)\n"
            "nNegativePlaceTie = ROUND(150, -2)\n"
            "nNegativePlaceNegativeTie = ROUND(-150, -2)\n"
            "nNegativePlaceCarry = ROUND(999, -2)\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "ROUND decimal edge script should complete: " + state.message);

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " should be present");
            if (it != state.globals.end())
            {
                expect(copperfin::runtime::format_value(it->second) == expected,
                       name + ": expected \"" + expected + "\", got \"" +
                           copperfin::runtime::format_value(it->second) + "\"");
            }
        };

        check("nbinary", "1.01");
        check("nbinarysmall", "0.15");
        check("npositivetie", "3");
        check("nnegativetie", "-3");
        check("nnegativeplaces", "1200");
        check("nthreeplaces", "1.235");
        check("ncarry", "10");
        check("nnegativecarry", "-10");
        check("nsmalltie", "0.001");
        check("nbelownegativeplace", "0");
        check("nbelownegativetie", "100");
        check("nnegativeplacetie", "200");
        check("nnegativeplacenegativetie", "-200");
        check("nnegativeplacecarry", "1000");

        fs::remove_all(temp_root, ignored);
    }

    void test_numeric_coercion_of_blank_padded_string_does_not_fault()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_numeric_coercion_blank_string";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "coerce.prg";
        write_text(
            main_path,
            "nBlank = ROUND(SPACE(10), 2)\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

        expect(state.completed,
               "#numeric-coercion: ROUND() on a space-padded blank string should not fault: " + state.message);

        const auto it = state.globals.find("nblank");
        if (it == state.globals.end())
        {
            expect(false, "nblank variable not found");
        }
        else
        {
            const std::string actual = copperfin::runtime::format_value(it->second);
            expect(actual == "0", "nblank: expected \"0\", got \"" + actual + "\"");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_ordering_comparisons_on_non_numeric_strings_do_not_fault()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_string_ordering_comparisons";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "ordering.prg";
        write_text(
            main_path,
            "cA = 'BRAVO'\n"
            "cB = 'APPLE'\n"
            "lGreater = cA > cB\n"
            "lLess = cA < cB\n"
            "lGreaterOrEqual = cA >= 'BRAVO'\n"
            "lLessOrEqual = cB <= cA\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

        expect(state.completed,
               "#ordering-comparison: comparing two non-numeric strings with </>/</=/>= should not fault: " + state.message);

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            const std::string actual = copperfin::runtime::format_value(it->second);
            expect(actual == expected, name + ": expected \"" + expected + "\", got \"" + actual + "\"");
        };

        check("lgreater", "true");
        check("lless", "false");
        check("lgreaterorequal", "true");
        check("llessorequal", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_between_on_non_numeric_strings_does_not_fault()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_string_between_non_numeric";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "between.prg";
        write_text(
            main_path,
            "lHit = BETWEEN('b', 'a', 'c')\n"
            "lMissLow = BETWEEN('a', 'b', 'c')\n"
            "lMissHigh = BETWEEN('z', 'a', 'c')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

        expect(state.completed,
               "#3697: BETWEEN() on non-numeric strings should not fault: " + state.message);

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            const std::string actual = copperfin::runtime::format_value(it->second);
            expect(actual == expected, name + ": expected \"" + expected + "\", got \"" + actual + "\"");
        };

        check("lhit", "true");
        check("lmisslow", "false");
        check("lmisshigh", "false");

        fs::remove_all(temp_root, ignored);
    }

} // namespace

int main()
{
    test_numeric_formatting_ignores_host_global_locale();
    test_currency_stringification_ignores_grouping_locale();
    test_string_and_math_expression_functions();
    test_index_expression_trim_functions_preserve_non_space_whitespace();
    test_index_expression_padl_truncation_matches_runtime_padl();
    test_financial_and_misc_expression_functions();
    test_textmerge_set_state_and_delimiters();
    test_nested_macro_eval_textmerge_execscript_semantics();
    test_round_uses_decimal_half_away_from_zero_behavior();
    test_numeric_domain_errors_route_through_runtime_catalog();
    test_numeric_coercion_of_blank_padded_string_does_not_fault();
    test_ordering_comparisons_on_non_numeric_strings_do_not_fault();
    test_between_on_non_numeric_strings_does_not_fault();

    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
