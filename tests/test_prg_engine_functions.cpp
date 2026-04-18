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

    expect(drive != state.globals.end(), "JUSTDRIVE result should be captured");
    expect(win_dir != state.globals.end(), "Windows JUSTPATH result should be captured");
    expect(win_name != state.globals.end(), "Windows JUSTFNAME result should be captured");
    expect(win_stem != state.globals.end(), "Windows JUSTSTEM result should be captured");
    expect(win_ext != state.globals.end(), "Windows JUSTEXT result should be captured");
    expect(posix_dir != state.globals.end(), "POSIX JUSTPATH result should be captured");
    expect(posix_name != state.globals.end(), "POSIX JUSTFNAME result should be captured");
    expect(posix_stem != state.globals.end(), "POSIX JUSTSTEM result should be captured");
    expect(posix_ext != state.globals.end(), "POSIX JUSTEXT result should be captured");

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

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_portable_path_expression_functions();

    if (test_failures() != 0) {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
