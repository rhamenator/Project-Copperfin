#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <system_error>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

namespace {

using namespace copperfin::test_support;

void test_portable_path_expression_functions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_functions_paths";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "path_functions.prg";
    write_text(
        main_path,
        "cWinPath = 'E:\\Project-Copperfin\\src\\runtime\\prg_engine.cpp'\n"
        "cPosixPath = '/home/rich/dev/Project-Copperfin/src/runtime/prg_engine.cpp'\n"
        "cDrive = JUSTDRIVE(cWinPath)\n"
        "cWinDir = JUSTPATH(cWinPath)\n"
        "cWinName = JUSTFNAME(cWinPath)\n"
        "cWinStem = JUSTSTEM(cWinPath)\n"
        "cWinExt = JUSTEXT(cWinPath)\n"
        "cPosixDir = JUSTPATH(cPosixPath)\n"
        "cPosixName = JUSTFNAME(cPosixPath)\n"
        "cPosixStem = JUSTSTEM(cPosixPath)\n"
        "cPosixExt = JUSTEXT(cPosixPath)\n"
        "cForcedExt = FORCEEXT(cWinPath, 'h')\n"
        "cForcedExtWithDot = FORCEEXT(cWinPath, '.hpp')\n"
        "cForcedPath = FORCEPATH(cWinPath, 'D:\\generated')\n"
        "cForcedPosixPath = FORCEPATH(cPosixPath, '/tmp/generated')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "portable path function script should complete");

    const auto drive = state.globals.find("cdrive");
    const auto win_dir = state.globals.find("cwindir");
    const auto win_name = state.globals.find("cwinname");
    const auto win_stem = state.globals.find("cwinstem");
    const auto win_ext = state.globals.find("cwinext");
    const auto posix_dir = state.globals.find("cposixdir");
    const auto posix_name = state.globals.find("cposixname");
    const auto posix_stem = state.globals.find("cposixstem");
    const auto posix_ext = state.globals.find("cposixext");
    const auto forced_ext = state.globals.find("cforcedext");
    const auto forced_ext_with_dot = state.globals.find("cforcedextwithdot");
    const auto forced_path = state.globals.find("cforcedpath");
    const auto forced_posix_path = state.globals.find("cforcedposixpath");

    expect(drive != state.globals.end(), "JUSTDRIVE result should be captured");
    expect(win_dir != state.globals.end(), "Windows JUSTPATH result should be captured");
    expect(win_name != state.globals.end(), "Windows JUSTFNAME result should be captured");
    expect(win_stem != state.globals.end(), "Windows JUSTSTEM result should be captured");
    expect(win_ext != state.globals.end(), "Windows JUSTEXT result should be captured");
    expect(posix_dir != state.globals.end(), "POSIX JUSTPATH result should be captured");
    expect(posix_name != state.globals.end(), "POSIX JUSTFNAME result should be captured");
    expect(posix_stem != state.globals.end(), "POSIX JUSTSTEM result should be captured");
    expect(posix_ext != state.globals.end(), "POSIX JUSTEXT result should be captured");
    expect(forced_ext != state.globals.end(), "FORCEEXT result should be captured");
    expect(forced_ext_with_dot != state.globals.end(), "FORCEEXT dotted-extension result should be captured");
    expect(forced_path != state.globals.end(), "FORCEPATH Windows result should be captured");
    expect(forced_posix_path != state.globals.end(), "FORCEPATH POSIX result should be captured");

    if (drive != state.globals.end()) {
        expect(copperfin::runtime::format_value(drive->second) == "E:", "JUSTDRIVE should parse drive-letter roots on every host");
    }
    if (win_dir != state.globals.end()) {
        expect(copperfin::runtime::format_value(win_dir->second) == "E:\\Project-Copperfin\\src\\runtime",
            "JUSTPATH should parse Windows-style backslash paths on every host");
    }
    if (win_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(win_name->second) == "prg_engine.cpp",
            "JUSTFNAME should parse Windows-style file names on every host");
    }
    if (win_stem != state.globals.end()) {
        expect(copperfin::runtime::format_value(win_stem->second) == "prg_engine",
            "JUSTSTEM should parse Windows-style stems on every host");
    }
    if (win_ext != state.globals.end()) {
        expect(copperfin::runtime::format_value(win_ext->second) == "cpp",
            "JUSTEXT should parse Windows-style extensions on every host");
    }
    if (posix_dir != state.globals.end()) {
        expect(copperfin::runtime::format_value(posix_dir->second) == "/home/rich/dev/Project-Copperfin/src/runtime",
            "JUSTPATH should continue parsing POSIX-style paths");
    }
    if (posix_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(posix_name->second) == "prg_engine.cpp",
            "JUSTFNAME should continue parsing POSIX-style file names");
    }
    if (posix_stem != state.globals.end()) {
        expect(copperfin::runtime::format_value(posix_stem->second) == "prg_engine",
            "JUSTSTEM should continue parsing POSIX-style stems");
    }
    if (posix_ext != state.globals.end()) {
        expect(copperfin::runtime::format_value(posix_ext->second) == "cpp",
            "JUSTEXT should continue parsing POSIX-style extensions");
    }
    if (forced_ext != state.globals.end()) {
        expect(copperfin::runtime::format_value(forced_ext->second) == "E:\\Project-Copperfin\\src\\runtime\\prg_engine.h",
            "FORCEEXT should replace an extension on Windows-style paths");
    }
    if (forced_ext_with_dot != state.globals.end()) {
        expect(copperfin::runtime::format_value(forced_ext_with_dot->second) == "E:\\Project-Copperfin\\src\\runtime\\prg_engine.hpp",
            "FORCEEXT should accept a leading dot in the requested extension");
    }
    if (forced_path != state.globals.end()) {
        expect(copperfin::runtime::format_value(forced_path->second) == "D:\\generated\\prg_engine.cpp",
            "FORCEPATH should replace a Windows-style directory on every host");
    }
    if (forced_posix_path != state.globals.end()) {
        expect(copperfin::runtime::format_value(forced_posix_path->second) == "/tmp/generated/prg_engine.cpp",
            "FORCEPATH should replace a POSIX-style directory");
    }

    fs::remove_all(temp_root, ignored);
}

void test_string_and_math_expression_functions() {
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
        "proper_value = PROPER('legacy fox-pro APP')\n"
        "like_hit = LIKE('A?C*', 'abc legacy')\n"
        "like_miss = LIKE('A?D*', 'abc legacy')\n"
        "inlist_hit = INLIST('beta', 'alpha', 'beta', 'gamma')\n"
        "inlist_miss = INLIST(4, 1, 2, 3)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "string/math function script should complete");

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
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
    check("proper_value", "Legacy Fox-Pro App");
    check("like_hit", "true");
    check("like_miss", "false");
    check("inlist_hit", "true");
    check("inlist_miss", "false");

    fs::remove_all(temp_root, ignored);
}

void test_type_and_null_expression_functions() {
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
        "nvl_result = NVL('', 'fallback')\n"
        "nvl_ok = NVL('value', 'fallback')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "type/null function script should complete");

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
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
    check("nvl_result", "");
    check("nvl_ok", "value");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_portable_path_expression_functions();
    test_string_and_math_expression_functions();
    test_type_and_null_expression_functions();

    if (test_failures() != 0) {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
