// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <system_error>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

namespace
{

    using namespace copperfin::test_support;

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
            "a = ABS(-7)\n"
            "b = INT(3.9)\n"
            "c = MOD(10, 3)\n"
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
            "g = BETWEEN(5, 1, 10)\n"
            "h = OCCURS('l', 'hello world')\n"
            "v = VAL('42')\n"
            "at_second = AT('ha', 'ha ha ha', 2)\n"
            "rat_second = RAT('ha', 'ha ha ha', 2)\n"
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
            "like_hit = LIKE('A?C*', 'abc legacy')\n"
            "like_miss = LIKE('A?D*', 'abc legacy')\n"
            "inlist_hit = INLIST('beta', 'alpha', 'beta', 'gamma')\n"
            "inlist_miss = INLIST(4, 1, 2, 3)\n"
            "getwordcount_1 = GETWORDCOUNT('one two three')\n"
            "getwordcount_2 = GETWORDCOUNT('a,b,c', ',')\n"
            "getwordcount_multi_delim = GETWORDCOUNT('a,b;c', ',;')\n"
            "getwordcount_tab = GETWORDCOUNT('one' + CHR(9) + 'two')\n"
            "getwordnum_1 = GETWORDNUM('one two three', 2)\n"
            "getwordnum_2 = GETWORDNUM('a,b,c', 3, ',')\n"
            "getwordnum_multi_delim = GETWORDNUM('a,b;c', 3, ',;')\n"
            "getwordnum_tab = GETWORDNUM('one' + CHR(9) + 'two', 2)\n"
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
            "SET MEMOWIDTH TO 10\n"
            "memo_width_value = _MLINE\n"
            "memo_width_set_after = SET('MEMOWIDTH')\n"
            "narrow_wrap_text = 'abc def ghi jkl mno'\n"
            "narrow_count = MEMLINES(narrow_wrap_text)\n"
            "narrow_first = MLINE(narrow_wrap_text, 1)\n"
            "narrow_second = MLINE(narrow_wrap_text, 2)\n"
            "narrow_third = MLINE(narrow_wrap_text, 3)\n"
            "strextract_case_sensitive = STREXTRACT('<Name>Beta</Name>', '<name>', '</name>')\n"
            "strextract_case_insensitive = STREXTRACT('<Name>Beta</Name>', '<name>', '</name>', 1, 1)\n"
            "strextract_missing_end_default = STREXTRACT('a=one;b=two', 'b=', ';')\n"
            "strextract_missing_end_allowed = STREXTRACT('a=one;b=two', 'b=', ';', 1, 2)\n"
            "strextract_include_delims = STREXTRACT('<id>42</id>', '<id>', '</id>', 1, 4)\n"
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
            "transform_upper = TRANSFORM('hello', '@!')\n"
            "RETURN\n");

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

        check("l", "5");
        check("lft", "hel");
        check("rgt", "lo");
        check("up", "HELLO");
        check("lo", "world");
        check("sp", "5");
        check("repl", "ababab");
        check("trimmed", "hi  ");
        check("rtrimmed", "  hi");
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
        check("g", "true");
        check("h", "3");
        check("v", "42");
        check("at_second", "4");
        check("rat_second", "4");
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
        check("str_default", "43");
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
        check("inlist_hit", "true");
        check("inlist_miss", "false");
        check("getwordcount_1", "3");
        check("getwordcount_2", "3");
        check("getwordcount_multi_delim", "3");
        check("getwordcount_tab", "2");
        check("getwordnum_1", "two");
        check("getwordnum_2", "c");
        check("getwordnum_multi_delim", "c");
        check("getwordnum_tab", "two");
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
        check("memo_width_session2", "50");
        check("memo_width_restored", "10");
        check("narrow_count", "3");
        check("narrow_first", "abc def");
        check("narrow_second", "ghi jkl");
        check("narrow_third", "mno");
        check("strextract_case_sensitive", "");
        check("strextract_case_insensitive", "Beta");
        check("strextract_missing_end_default", "");
        check("strextract_missing_end_allowed", "two");
        check("strextract_include_delims", "<id>42</id>");
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
        check("transform_upper", "HELLO");

        for (const char *name : {"rand_seeded", "rand_next"})
        {
            const std::string name_text{name};
            const auto it = state.globals.find(name_text);
            if (it == state.globals.end())
            {
                expect(false, name_text + " variable not found");
                continue;
            }
            double value = -1.0;
            try
            {
                value = std::stod(copperfin::runtime::format_value(it->second));
            }
            catch (...)
            {
                value = -1.0;
            }
            expect(value >= 0.0 && value < 1.0, name_text + " should be in the RAND() range [0, 1)");
        }

        fs::remove_all(temp_root, ignored);
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

} // namespace

int main()
{
    test_string_and_math_expression_functions();
    test_financial_and_misc_expression_functions();
    test_nested_macro_eval_textmerge_execscript_semantics();
    test_numeric_domain_errors_route_through_runtime_catalog();
    test_numeric_coercion_of_blank_padded_string_does_not_fault();
    test_ordering_comparisons_on_non_numeric_strings_do_not_fault();

    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
