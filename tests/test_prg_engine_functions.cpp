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
            "cSelfRef = 'cSelfRef'\n"
            "cFallbackExpr = 'plain text value'\n"
            "nDirect = &cExpr\n"
            "nIndirect = &cMacroName\n"
            "cSelfResult = &cSelfRef\n"
            "cFallback = &cFallbackExpr\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({.startup_path = main_path.string(),
                                                                                                       .working_directory = temp_root.string(),
                                                                                                       .stop_on_entry = false});

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
        check("cselfresult", "cSelfRef");
        check("cfallback", "plain text value");

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
            "em = EMPTY('')\n"
            "em2 = EMPTY(0)\n"
            "not_em = EMPTY('hi')\n"
            "blank_empty = ISBLANK('')\n"
            "blank_spaces = ISBLANK('   ')\n"
            "blank_text = ISBLANK('hi')\n"
            "blank_zero = ISBLANK(0)\n"
            "blank_false = ISBLANK(.F.)\n"
            "nvl_result = NVL('', 'fallback')\n"
            "nvl_ok = NVL('value', 'fallback')\n"
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
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({.startup_path = main_path.string(),
                                                                                                       .working_directory = temp_root.string(),
                                                                                                       .stop_on_entry = false});

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
        check("em", "true");
        check("em2", "true");
        check("not_em", "false");
        check("blank_empty", "true");
        check("blank_spaces", "true");
        check("blank_text", "false");
        check("blank_zero", "false");
        check("blank_false", "false");
        check("nvl_result", "");
        check("nvl_ok", "value");
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

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({.startup_path = main_path.string(),
                                                                                                       .working_directory = temp_root.string(),
                                                                                                       .stop_on_entry = false});

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
            "FirstName = 'John'\n"
            "cResult1 = &cType.Name\n"
            // Trailing dot (no suffix): dot is terminator, expands cleanly
            "cField = 'FirstName'\n"
            "cResult2 = &cField.\n"
            // m&cType.ID embedded macro form: mCustomerID = 99, cType="Customer"
            "cType2 = 'Customer'\n"
            "mCustomerID = 99\n"
            "nResult3 = m&cType2.ID\n"
            // &stem.suffix where result is used as a string (stem resolves to non-var)
            "cStem = 'Hello'\n"
            "cResult4 = &cStem.World\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            {.startup_path = main_path.string(),
             .working_directory = temp_root.string(),
             .stop_on_entry = false});

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
        // &cField. → "FirstName" (trailing dot, no suffix) → value of FirstName = "John"
        check("cresult2", "John");
        // m&cType2.ID → "mCustomerID" → value of mCustomerID = 99
        check("nresult3", "99");
        // &cStem.World → "Hello" + "World" = "HelloWorld" (no such variable, returns expanded string)
        check("cresult4", "HelloWorld");

        fs::remove_all(temp_root, ignored);
    }

} // namespace

int main()
{
    test_macro_expression_indirection();
    test_type_and_null_expression_functions();
    test_type_and_transform_expression_depth();
    test_macro_dot_suffix_form();

    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
