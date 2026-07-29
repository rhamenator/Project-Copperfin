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

    void test_macro_expression_indirection()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_macro_expression_indirection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "macro_expression_indirection.prg";
        write_text(
            main_path,
            "x = 5\n"
            "cExpr = 'x + 1'\n"
            "cMacroName = 'cExpr'\n"
            "cMacroNameHolder = 'cMacroName'\n"
            "cMacroNameDeepHolder = 'cMacroNameHolder'\n"
            "cSelfRef = 'cSelfRef'\n"
            "cSelfRefHolder = 'cSelfRef'\n"
            "cSelfRefDeepHolder = 'cSelfRefHolder'\n"
            "cFallbackExpr = \"'plain text value'\"\n"
            "cFallbackExprHolder = 'cFallbackExpr'\n"
            "cFallbackExprDeepHolder = 'cFallbackExprHolder'\n"
            "nDirect = &cExpr\n"
            "nIndirect = &cMacroName\n"
            "nIndirectSecondHop = &cMacroNameDeepHolder\n"
            "cSelfResult = &cSelfRefDeepHolder\n"
            "cFallback = &cFallbackExprDeepHolder\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "macro expression indirection script should complete");

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

        check("ndirect", "6");
        check("nindirect", "6");
        check("nindirectsecondhop", "6");
        check("cselfresult", "cSelfRef");
        check("cfallback", "plain text value");

        fs::remove_all(temp_root, ignored);
    }

    void test_double_ampersand_comment_stripping_respects_nested_text()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_double_ampersand";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "double_ampersand.prg";
        write_text(
            main_path,
            "x = 5\n"
            "cExpr = 'x + 2'\n"
            "cQuoted = \"A && B\"\n"
            "cBracket = '[&& inside text]'\n"
            "cBraced = '{|x| \"&& kept\"}'\n"
            "nMacro = &cExpr && trailing comment should strip\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "double-ampersand parsing script should complete");

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

        check("cquoted", "A && B");
        check("cbracket", "[&& inside text]");
        check("cbraced", "{|x| \"&& kept\"}");
        check("nmacro", "7");

        fs::remove_all(temp_root, ignored);
    }

    void test_type_and_null_expression_functions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_type_null";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "type_null.prg";
        write_text(
            main_path,
            "x = 5\n"
            "t_num = VARTYPE(x)\n"
            "t_str = VARTYPE('hello')\n"
            "t_bool = VARTYPE(.T.)\n"
            "t_date = VARTYPE(DATE(2026, 4, 18))\n"
            "t_datetime = VARTYPE(DATETIME(2026, 4, 18, 1, 2, 3))\n"
            "t_null = VARTYPE(.NULL.)\n"
            "t_missing = TYPE('missingVar')\n"
            "null_expr = ISNULL(.NULL.)\n"
            "null_value = .NULL.\n"
            "null_var = ISNULL(null_value)\n"
            "null_missing = ISNULL(missingVar)\n"
            "em = EMPTY('')\n"
            "em2 = EMPTY(0)\n"
            "em_tiny_num = EMPTY(0.0000001)\n"
            "not_em = EMPTY('hi')\n"
            "blank_empty = ISBLANK('')\n"
            "blank_spaces = ISBLANK('   ')\n"
            "blank_text = ISBLANK('hi')\n"
            "blank_zero = ISBLANK(0)\n"
            "blank_false = ISBLANK(.F.)\n"
            "nvl_result = NVL('', 'fallback')\n"
            "nvl_ok = NVL('value', 'fallback')\n"
            "evl_empty_text = EVL('', 'fallback')\n"
            "evl_zero = EVL(0, 17)\n"
            "evl_false = EVL(.F., .T.)\n"
            "evl_null = EVL(.NULL., 'fallback-null')\n"
            "evl_text = EVL('value', 'fallback')\n"
            "isdigit_yes = ISDIGIT('5abc')\n"
            "isdigit_no = ISDIGIT('abc')\n"
            "isalpha_yes = ISALPHA('abc')\n"
            "isalpha_no = ISALPHA('5abc')\n"
            "islower_yes = ISLOWER('abc')\n"
            "islower_no = ISLOWER('ABC')\n"
            "isupper_yes = ISUPPER('ABC')\n"
            "isupper_no = ISUPPER('abc')\n"
            "isleadbyte_ascii = ISLEADBYTE('A')\n"
            "isleadbyte_empty = ISLEADBYTE('')\n"
            "n_lenc = LENC('café猫')\n"
            "c_leftc = LEFTC('café猫', 4)\n"
            "c_rightc = RIGHTC('café猫', 2)\n"
            "n_atcc = ATCC('FE', 'caféFE猫FE', 2)\n"
            "n_at_c = AT_C('FE', 'caféFE猫FE', 2)\n"
            "n_at_c_unicode = AT_C('é', 'café猫')\n"
            "n_at_c_case = AT_C('fe', 'caféFE猫FE')\n"
            "n_at_c_zero = AT_C('FE', 'caféFE猫FE', 0)\n"
            "n_at_c_missing = AT_C('FE', 'caféFE猫FE', 3)\n"
            "n_ratc = RATC('FE', 'caféFE猫FE', 1)\n"
            "c_substrc = SUBSTRC('café猫', 4, 2)\n"
            "c_substrc_zero = SUBSTRC('café猫', 0, 2)\n"
            "c_substrc_negative = SUBSTRC('café猫', -2, 2)\n"
            "c_stuffc = STUFFC('café猫', 4, 2, 'X')\n"
            "c_stuffc_zero = STUFFC('café猫', 0, 2, 'X')\n"
            "c_stuffc_negative = STUFFC('café猫', -2, 2, 'X')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "type/null function script should complete");

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

        check("t_num", "N");
        check("t_str", "C");
        check("t_bool", "L");
        check("t_date", "D");
        check("t_datetime", "T");
        check("t_null", "X");
        check("t_missing", "U");
        check("null_expr", "true");
        check("null_var", "true");
        check("null_missing", "false");
        check("em", "true");
        check("em2", "true");
        check("em_tiny_num", "false");
        check("not_em", "false");
        check("blank_empty", "true");
        check("blank_spaces", "true");
        check("blank_text", "false");
        check("blank_zero", "false");
        check("blank_false", "false");
        check("nvl_result", "");
        check("nvl_ok", "value");
        check("evl_empty_text", "fallback");
        check("evl_zero", "17");
        check("evl_false", "true");
        check("evl_null", "fallback-null");
        check("evl_text", "value");
        check("isdigit_yes", "true");
        check("isdigit_no", "false");
        check("isalpha_yes", "true");
        check("isalpha_no", "false");
        check("islower_yes", "true");
        check("islower_no", "false");
        check("isupper_yes", "true");
        check("isupper_no", "false");
        check("isleadbyte_ascii", "false");
        check("isleadbyte_empty", "false");
        check("n_lenc", "5");
        check("c_leftc", "café");
        check("c_rightc", "é猫");
        check("n_atcc", "8");
        check("n_at_c", "8");
        check("n_at_c_unicode", "4");
        check("n_at_c_case", "0");
        check("n_at_c_zero", "5");
        check("n_at_c_missing", "0");
        check("n_ratc", "8");
        check("c_substrc", "é猫");
        check("c_substrc_zero", "ca");
        check("c_substrc_negative", "ca");
        check("c_stuffc", "cafX");
        check("c_stuffc_zero", "Xcafé猫");
        check("c_stuffc_negative", "Xcafé猫");

        fs::remove_all(temp_root, ignored);
    }

    void test_type_and_transform_expression_depth()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_type_transform_depth";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "type_transform_depth.prg";
        write_text(
            main_path,
            "nValue = 42\n"
            "DIMENSION aValues[2]\n"
            "aValues[1] = 7\n"
            "aValues[2] = ' spaced text ' \n"
            "cTypeArray = TYPE('aValues')\n"
            "cTypeArrayElement = TYPE('aValues[1]')\n"
            "cTypeArrayElementParen = TYPE('(aValues[1])')\n"
            "cTypeParenthesized = TYPE('(nValue + 1)')\n"
            "cTransformBlankZero = TRANSFORM(0, '@Z 999.99')\n"
            "cTransformLeftTrim = TRANSFORM('  padded text  ', '@B')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "type/transform depth script should complete");

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

        check("ctypearray", "A");
        check("ctypearrayelement", "N");
        check("ctypearrayelementparen", "N");
        check("ctypeparenthesized", "N");
        check("ctransformblankzero", "");
        check("ctransformlefttrim", "padded text");

        fs::remove_all(temp_root, ignored);
    }


    void test_macro_dot_suffix_form()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_macro_dot_suffix";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "macro_dot_suffix.prg";
        write_text(
            main_path,
            // Basic &stem.suffix: cType="First", FirstName="John" => &cType.Name = "FirstName" variable
            "cType = 'First'\n"
            "cTypeHolder = 'cType'\n"
            "cTypeDeepHolder = 'cTypeHolder'\n"
            "FirstName = 'John'\n"
            "cResult1 = &cType.Name\n"
            "cResult1Nested = &cTypeHolder.Name\n"
            "cResult1SecondHop = &cTypeDeepHolder.Name\n"
            // Trailing dot (no suffix): dot is terminator, expands cleanly
            "cField = 'FirstName'\n"
            "cFieldHolder = 'cField'\n"
            "cFieldDeepHolder = 'cFieldHolder'\n"
            "cResult2 = &cField.\n"
            "cResult2Nested = &cFieldHolder.\n"
            "cResult2SecondHop = &cFieldDeepHolder.\n"
            // m&cType.ID embedded macro form: mCustomerID = 99, cType="Customer"
            "cType2 = 'Customer'\n"
            "cType2Holder = 'cType2'\n"
            "cType2DeepHolder = 'cType2Holder'\n"
            "mCustomerID = 99\n"
            "nResult3 = m&cType2.ID\n"
            "nResult3Nested = m&cType2Holder.ID\n"
            "nResult3SecondHop = m&cType2DeepHolder.ID\n"
            // &stem.suffix where result is used as a string (stem resolves to non-var)
            "cStem = 'Hello'\n"
            "cStemHolder = 'cStem'\n"
            "cStemDeepHolder = 'cStemHolder'\n"
            "cResult4 = &cStemDeepHolder.World\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "macro dot-suffix script should complete");

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

        // &cType.Name → "FirstName" → value of FirstName = "John"
        check("cresult1", "John");
        // &cTypeHolder.Name → "cType" → "FirstName" → value of FirstName = "John"
        check("cresult1nested", "John");
        // &cTypeDeepHolder.Name → "cTypeHolder" → "cType" → "FirstName" → value of FirstName = "John"
        check("cresult1secondhop", "John");
        // &cField. → "FirstName" (trailing dot, no suffix) → value of FirstName = "John"
        check("cresult2", "John");
        // &cFieldHolder. → "cField" → "FirstName" → value of FirstName = "John"
        check("cresult2nested", "John");
        // &cFieldDeepHolder. → "cFieldHolder" → "cField" → "FirstName" → value of FirstName = "John"
        check("cresult2secondhop", "John");
        // m&cType2.ID → "mCustomerID" → value of mCustomerID = 99
        check("nresult3", "99");
        // m&cType2Holder.ID → "mcType2ID" → "mCustomerID" → value of mCustomerID = 99
        check("nresult3nested", "99");
        // m&cType2DeepHolder.ID → "mcType2HolderID" → "mcType2ID" → "mCustomerID" → value of mCustomerID = 99
        check("nresult3secondhop", "99");
        // &cStem.World → "Hello" + "World" = "HelloWorld" (no such variable, returns expanded string)
        check("cresult4", "HelloWorld");

        fs::remove_all(temp_root, ignored);
    }

    void test_parameter_default_expressions_support_macros()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_parameter_defaults";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "parameter_defaults.prg";
        write_text(
            main_path,
            "PUBLIC nProvidedSeen, cProvidedLabel, nDefaultSeen, cDefaultLabel\n"
            "cDefaultExpr = '40 + 2'\n"
            "cDefaultExprHolder = 'cDefaultExpr'\n"
            "cDefaultExprDeepHolder = 'cDefaultExprHolder'\n"
            "DO FillProvided WITH 7\n"
            "DO FillDefault\n"
            "RETURN\n"
            "PROCEDURE FillProvided\n"
            "LPARAMETERS nValue = &cDefaultExprDeepHolder, cLabel = 'fallback'\n"
            "nProvidedSeen = nValue\n"
            "cProvidedLabel = cLabel\n"
            "RETURN\n"
            "PROCEDURE FillDefault\n"
            "LPARAMETERS nValue = &cDefaultExprDeepHolder, cLabel = 'fallback'\n"
            "nDefaultSeen = nValue\n"
            "cDefaultLabel = cLabel\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "parameter default macro script should complete");

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

        check("nprovidedseen", "7");
        check("cprovidedlabel", "fallback");
        check("ndefaultseen", "42");
        check("cdefaultlabel", "fallback");

        fs::remove_all(temp_root, ignored);
    }

    void test_macro_alias_qualified_field_access()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_macro_alias_field_access";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path table_path = temp_root / "people.dbf";
        write_people_dbf(table_path, {{"ALPHA", 11}, {"BRAVO", 22}});

        const fs::path main_path = temp_root / "macro_alias_field_access.prg";
        write_text(
            main_path,
            "USE '" + table_path.string() + "' ALIAS People IN 0\n"
            "cAlias = 'People'\n"
            "cAliasHolder = 'cAlias'\n"
            "cAliasDeepHolder = 'cAliasHolder'\n"
            "cNameField = 'NAME'\n"
            "cNameFieldHolder = 'cNameField'\n"
            "cAgeField = 'AGE'\n"
            "cAgeFieldHolder = 'cAgeField'\n"
            "cNestedField = 'cNameField'\n"
            "cNestedFieldHolder = 'cNestedField'\n"
            "cAgeNestedField = 'cAgeField'\n"
            "cAgeNestedFieldHolder = 'cAgeNestedField'\n"
            "cNameFromAlias = &cAlias..NAME\n"
            "cNameFromAliasNested = &cAliasHolder..NAME\n"
            "cNameFromAliasSecondHop = &cAliasDeepHolder..NAME\n"
            "nAgeFromAlias = &cAlias..AGE\n"
            "nAgeFromAliasNested = &cAliasHolder..AGE\n"
            "nAgeFromAliasSecondHop = &cAliasDeepHolder..AGE\n"
            "cNameFromDynamicField = &cAlias..&cNameField\n"
            "cNameFromDynamicFieldNested = &cAliasHolder..&cNameFieldHolder\n"
            "cNameFromDynamicFieldSecondHop = &cAliasDeepHolder..&cNestedFieldHolder\n"
            "nAgeFromDynamicField = &cAlias..&cAgeField\n"
            "nAgeFromDynamicFieldNested = &cAliasHolder..&cAgeFieldHolder\n"
            "nAgeFromDynamicFieldSecondHop = &cAliasDeepHolder..&cAgeNestedFieldHolder\n"
            "LOCATE FOR &cAlias..&cNestedField = 'BRAVO'\n"
            "LOCATE FOR &cAliasHolder..&cNestedFieldHolder = 'BRAVO'\n"
            "LOCATE FOR &cAliasDeepHolder..&cNestedFieldHolder = 'BRAVO'\n"
            "lFoundDynamic = FOUND()\n"
            "cLocatedName = NAME\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "macro alias-qualified field-access script should complete");

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

        check("cnamefromalias", "ALPHA");
        check("cnamefromaliasnested", "ALPHA");
        check("cnamefromaliassecondhop", "ALPHA");
        check("nagefromalias", "11");
        check("nagefromaliasnested", "11");
        check("nagefromaliassecondhop", "11");
        check("cnamefromdynamicfield", "ALPHA");
        check("cnamefromdynamicfieldnested", "ALPHA");
        check("cnamefromdynamicfieldsecondhop", "ALPHA");
        check("nagefromdynamicfield", "11");
        check("nagefromdynamicfieldnested", "11");
        check("nagefromdynamicfieldsecondhop", "11");
        check("lfounddynamic", "true");
        check("clocatedname", "BRAVO");

        fs::remove_all(temp_root, ignored);
    }

    void test_return_expression_values_are_preserved_in_runtime_state()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_return_expression_values";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path numeric_path = temp_root / "return_numeric.prg";
        write_text(
            numeric_path,
            "nLeft = 40\n"
            "nRight = 2\n"
            "RETURN nLeft + nRight\n");

        copperfin::runtime::PrgRuntimeSession numeric_session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(numeric_path.string(), temp_root.string()));
        const auto numeric_state = numeric_session.run(copperfin::runtime::DebugResumeAction::continue_run);

        expect(numeric_state.completed, "numeric RETURN expression script should complete");
        expect(numeric_state.last_return_value.has_value(),
               "numeric RETURN expression should be preserved in runtime state");
        if (numeric_state.last_return_value.has_value())
        {
            expect(copperfin::runtime::format_value(*numeric_state.last_return_value) == "42",
                   "numeric RETURN expression should preserve the evaluated numeric result");
        }

        const fs::path string_path = temp_root / "return_string.prg";
        write_text(
            string_path,
            "cPrefix = 'Copper'\n"
            "RETURN cPrefix + 'fin'\n");

        copperfin::runtime::PrgRuntimeSession string_session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(string_path.string(), temp_root.string()));
        const auto string_state = string_session.run(copperfin::runtime::DebugResumeAction::continue_run);

        expect(string_state.completed, "string RETURN expression script should complete");
        expect(string_state.last_return_value.has_value(),
               "string RETURN expression should be preserved in runtime state");
        if (string_state.last_return_value.has_value())
        {
            expect(copperfin::runtime::format_value(*string_state.last_return_value) == "Copperfin",
                   "string RETURN expression should preserve the evaluated string result");
        }

        const fs::path empty_path = temp_root / "return_empty.prg";
        write_text(empty_path, "RETURN\n");

        copperfin::runtime::PrgRuntimeSession empty_session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(empty_path.string(), temp_root.string()));
        const auto empty_state = empty_session.run(copperfin::runtime::DebugResumeAction::continue_run);

        expect(empty_state.completed, "bare RETURN script should complete");
        expect(empty_state.last_return_value.has_value(),
               "bare RETURN should preserve an empty runtime return value");
        if (empty_state.last_return_value.has_value())
        {
            expect(copperfin::runtime::format_value(*empty_state.last_return_value).empty(),
                   "bare RETURN should preserve an empty value representation");
        }

        fs::remove_all(temp_root, ignored);
    }

} // namespace

int main()
{
    test_macro_expression_indirection();
    test_double_ampersand_comment_stripping_respects_nested_text();
    test_type_and_null_expression_functions();
    test_type_and_transform_expression_depth();
    test_macro_dot_suffix_form();
    test_parameter_default_expressions_support_macros();
    test_macro_alias_qualified_field_access();
    test_return_expression_values_are_preserved_in_runtime_state();

    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
