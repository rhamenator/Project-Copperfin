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

    void test_portable_path_expression_functions()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_functions_paths";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "path_functions.prg";
        write_text(
            main_path,
            "cWinPath = 'E:\\Project-Copperfin\\src\\runtime\\prg_engine.cpp'\n"
            "cUncPath = '\\\\server\\share\\reports\\invoice.frx'\n"
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
            "cWinFullPath = FULLPATH(cWinPath)\n"
            "cUncFullPath = FULLPATH(cUncPath)\n"
            "cPosixFullPath = FULLPATH(cPosixPath)\n"
            "cRelativeWinFullPath = FULLPATH('forms\\main.prg')\n"
            "cRelativePosixFullPath = FULLPATH('forms/report.prg')\n"
            "cForcedExt = FORCEEXT(cWinPath, 'h')\n"
            "cForcedExtWithDot = FORCEEXT(cWinPath, '.hpp')\n"
            "cDefaultExtAdded = DEFAULTEXT('D:\\generated\\report', 'frx')\n"
            "cDefaultExtKept = DEFAULTEXT('D:\\generated\\report.frx', 'bak')\n"
            "cForcedPath = FORCEPATH(cWinPath, 'D:\\generated')\n"
            "cForcedPosixPath = FORCEPATH(cPosixPath, '/tmp/generated')\n"
            "cCurrentDir = CURDIR()\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

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
        const auto win_full_path = state.globals.find("cwinfullpath");
        const auto unc_full_path = state.globals.find("cuncfullpath");
        const auto posix_full_path = state.globals.find("cposixfullpath");
        const auto relative_win_full_path = state.globals.find("crelativewinfullpath");
        const auto relative_posix_full_path = state.globals.find("crelativeposixfullpath");
        const auto forced_ext = state.globals.find("cforcedext");
        const auto forced_ext_with_dot = state.globals.find("cforcedextwithdot");
        const auto default_ext_added = state.globals.find("cdefaultextadded");
        const auto default_ext_kept = state.globals.find("cdefaultextkept");
        const auto forced_path = state.globals.find("cforcedpath");
        const auto forced_posix_path = state.globals.find("cforcedposixpath");
        const auto current_dir = state.globals.find("ccurrentdir");

        expect(drive != state.globals.end(), "JUSTDRIVE result should be captured");
        expect(win_dir != state.globals.end(), "Windows JUSTPATH result should be captured");
        expect(win_name != state.globals.end(), "Windows JUSTFNAME result should be captured");
        expect(win_stem != state.globals.end(), "Windows JUSTSTEM result should be captured");
        expect(win_ext != state.globals.end(), "Windows JUSTEXT result should be captured");
        expect(posix_dir != state.globals.end(), "POSIX JUSTPATH result should be captured");
        expect(posix_name != state.globals.end(), "POSIX JUSTFNAME result should be captured");
        expect(posix_stem != state.globals.end(), "POSIX JUSTSTEM result should be captured");
        expect(posix_ext != state.globals.end(), "POSIX JUSTEXT result should be captured");
        expect(win_full_path != state.globals.end(), "Windows FULLPATH result should be captured");
        expect(unc_full_path != state.globals.end(), "UNC FULLPATH result should be captured");
        expect(posix_full_path != state.globals.end(), "POSIX FULLPATH result should be captured");
        expect(relative_win_full_path != state.globals.end(), "relative Windows-style FULLPATH result should be captured");
        expect(relative_posix_full_path != state.globals.end(), "relative POSIX FULLPATH result should be captured");
        expect(forced_ext != state.globals.end(), "FORCEEXT result should be captured");
        expect(forced_ext_with_dot != state.globals.end(), "FORCEEXT dotted-extension result should be captured");
        expect(default_ext_added != state.globals.end(), "DEFAULTEXT add result should be captured");
        expect(default_ext_kept != state.globals.end(), "DEFAULTEXT keep result should be captured");
        expect(forced_path != state.globals.end(), "FORCEPATH Windows result should be captured");
        expect(forced_posix_path != state.globals.end(), "FORCEPATH POSIX result should be captured");
        expect(current_dir != state.globals.end(), "CURDIR result should be captured");

        if (drive != state.globals.end())
        {
            expect(copperfin::runtime::format_value(drive->second) == "E:", "JUSTDRIVE should parse drive-letter roots on every host");
        }
        if (win_dir != state.globals.end())
        {
            expect(copperfin::runtime::format_value(win_dir->second) == "E:\\Project-Copperfin\\src\\runtime",
                   "JUSTPATH should parse Windows-style backslash paths on every host");
        }
        if (win_name != state.globals.end())
        {
            expect(copperfin::runtime::format_value(win_name->second) == "prg_engine.cpp",
                   "JUSTFNAME should parse Windows-style file names on every host");
        }
        if (win_stem != state.globals.end())
        {
            expect(copperfin::runtime::format_value(win_stem->second) == "prg_engine",
                   "JUSTSTEM should parse Windows-style stems on every host");
        }
        if (win_ext != state.globals.end())
        {
            expect(copperfin::runtime::format_value(win_ext->second) == "cpp",
                   "JUSTEXT should parse Windows-style extensions on every host");
        }
        if (posix_dir != state.globals.end())
        {
            expect(copperfin::runtime::format_value(posix_dir->second) == "/home/rich/dev/Project-Copperfin/src/runtime",
                   "JUSTPATH should continue parsing POSIX-style paths");
        }
        if (posix_name != state.globals.end())
        {
            expect(copperfin::runtime::format_value(posix_name->second) == "prg_engine.cpp",
                   "JUSTFNAME should continue parsing POSIX-style file names");
        }
        if (posix_stem != state.globals.end())
        {
            expect(copperfin::runtime::format_value(posix_stem->second) == "prg_engine",
                   "JUSTSTEM should continue parsing POSIX-style stems");
        }
        if (posix_ext != state.globals.end())
        {
            expect(copperfin::runtime::format_value(posix_ext->second) == "cpp",
                   "JUSTEXT should continue parsing POSIX-style extensions");
        }
        if (win_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(win_full_path->second) == "E:\\Project-Copperfin\\src\\runtime\\prg_engine.cpp",
                   "FULLPATH should preserve Windows drive-rooted absolute paths on every host");
        }
        if (unc_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(unc_full_path->second) == "\\\\server\\share\\reports\\invoice.frx",
                   "FULLPATH should preserve UNC absolute paths on every host");
        }
        if (posix_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(posix_full_path->second) == "/home/rich/dev/Project-Copperfin/src/runtime/prg_engine.cpp",
                   "FULLPATH should preserve POSIX absolute paths");
        }
        if (relative_win_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(relative_win_full_path->second) == (temp_root / "forms" / "main.prg").string(),
                   "FULLPATH should treat backslashes as separators for relative VFP paths");
        }
        if (relative_posix_full_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(relative_posix_full_path->second) == (temp_root / "forms" / "report.prg").string(),
                   "FULLPATH should continue resolving relative POSIX-style paths against CURDIR");
        }
        if (forced_ext != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_ext->second) == "E:\\Project-Copperfin\\src\\runtime\\prg_engine.h",
                   "FORCEEXT should replace an extension on Windows-style paths");
        }
        if (forced_ext_with_dot != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_ext_with_dot->second) == "E:\\Project-Copperfin\\src\\runtime\\prg_engine.hpp",
                   "FORCEEXT should accept a leading dot in the requested extension");
        }
        if (default_ext_added != state.globals.end())
        {
            expect(copperfin::runtime::format_value(default_ext_added->second) == "D:\\generated\\report.frx",
                   "DEFAULTEXT should append an extension only when one is missing");
        }
        if (default_ext_kept != state.globals.end())
        {
            expect(copperfin::runtime::format_value(default_ext_kept->second) == "D:\\generated\\report.frx",
                   "DEFAULTEXT should preserve an existing extension");
        }
        if (forced_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_path->second) == "D:\\generated\\prg_engine.cpp",
                   "FORCEPATH should replace a Windows-style directory on every host");
        }
        if (forced_posix_path != state.globals.end())
        {
            expect(copperfin::runtime::format_value(forced_posix_path->second) == "/tmp/generated/prg_engine.cpp",
                   "FORCEPATH should replace a POSIX-style directory");
        }
        if (current_dir != state.globals.end())
        {
            expect(copperfin::runtime::format_value(current_dir->second) == temp_root.string(),
                   "CURDIR should expose the runtime working directory");
        }

        fs::remove_all(temp_root, ignored);
    }


} // namespace

int main()
{
    test_portable_path_expression_functions();

    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
