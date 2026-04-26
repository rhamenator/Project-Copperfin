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


} // namespace

int main()
{
    test_type_and_null_expression_functions();

    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
