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

    void test_expression_runtime_surface_extensions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_runtime_surface_extensions";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_surface_extensions.prg";
        write_text(
            main_path,
            "x = 5\n"
            "DIMENSION aValues[2]\n"
            "aValues[1] = 'A'\n"
            "aValues[2] = 'B'\n"
            "nEval = EVALUATE('x + 7')\n"
            "cTransformDefault = TRANSFORM(x)\n"
            "cTransformPicture = TRANSFORM(3.14159, '999.00')\n"
            "cTransformUpper = TRANSFORM('legacy', '@!')\n"
            "cTypeArray = TYPE('aValues')\n"
            "cTypeUnknown = TYPE('notDefinedAnywhere')\n"
            "nCastInt = CAST(7.9 AS INTEGER)\n"
            "nCastByte = CAST(513 AS BYTE)\n"
            "cCastString = CAST(123 AS STRING)\n"
            "lCastBool = CAST(0 AS LOGICAL)\n"
            "nBitAnd = BITAND(6, 3)\n"
            "nBitOr = BITOR(6, 3)\n"
            "nBitXor = BITXOR(6, 3)\n"
            "nBitNot = BITNOT(0)\n"
            "nBitLShift = BITLSHIFT(3, 2)\n"
            "nBitRShift = BITRSHIFT(16, 2)\n"
            "cPacked = BINTOC(16909060, 4)\n"
            "nUnpacked = CTOBIN(cPacked, 'N')\n"
            "nCursorProp = CURSORGETPROP('Buffering')\n"
            "lNumLock = NUMLOCK()\n"
            "SET PATH TO '/tmp/copperfin'\n"
            "cPathValue = SET('PATH')\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
            .startup_path = main_path.string(),
            .working_directory = temp_root.string(),
            .stop_on_entry = false
        });

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "runtime surface extension script should complete");

        const auto check = [&](const std::string& name, const std::string& expected)
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

        check("neval", "12");
        check("ctransformdefault", "5");
        check("ctransformpicture", "3.14");
        check("ctransformupper", "LEGACY");
        check("ctypearray", "A");
        check("ctypeunknown", "U");
        check("ncastint", "7");
        check("ncastbyte", "1");
        check("ccaststring", "123");
        check("lcastbool", "false");
        check("nbitand", "2");
        check("nbitor", "7");
        check("nbitxor", "5");
        check("nbitnot", "-1");
        check("nbitlshift", "12");
        check("nbitrshift", "4");
        check("nunpacked", "16909060");
        check("ncursorprop", "0");
        check("lnumlock", "false");
        check("cpathvalue", "TO '/tmp/copperfin'");

        fs::remove_all(temp_root, ignored);
    }

} // namespace

int main()
{
    test_expression_runtime_surface_extensions();

    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
