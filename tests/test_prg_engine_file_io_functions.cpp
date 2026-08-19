// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

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

void test_file_io_runtime_functions()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_file_io_functions";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "file_io_functions.prg";
    write_text(
        main_path,
        "nWrite = STRTOFILE('line1' + CHR(10) + 'line2', 'rw.txt')\n"
        "cWhole = FILETOSTR('rw.txt')\n"
        "nWriteBackslash = STRTOFILE('nested-data', 'nested\\backslash.txt')\n"
        "cBackslashWhole = FILETOSTR('nested\\backslash.txt')\n"
        "hBackslashRead = FOPEN('nested\\backslash.txt', 0)\n"
        "cBackslashChunk = FREAD(hBackslashRead, 6)\n"
        "nCloseBackslashRead = FCLOSE(hBackslashRead)\n"
        "hRead = FOPEN('rw.txt', 0)\n"
        "cChunk = FREAD(hRead, 4)\n"
        "nTellChunk = FTELL(hRead)\n"
        "nSeekStart = FSEEK(hRead, 0, 0)\n"
        "cLine1 = FGETS(hRead, 64)\n"
        "cLine2 = FGETS(hRead, 64)\n"
        "cLine3 = FGETS(hRead, 64)\n"
        "lEofRead = FEOF(hRead)\n"
        "nCloseRead = FCLOSE(hRead)\n"
        "hTail = FOPEN('rw.txt', 0)\n"
        "nSeekTail = FSEEK(hTail, -5, 2)\n"
        "cTail = FREAD(hTail, 5)\n"
        "nCloseTail = FCLOSE(hTail)\n"
        "hWrite = FOPEN('write.txt', 1)\n"
        "nPut = FPUTS(hWrite, 'abc')\n"
        "nFlush = FFLUSH(hWrite)\n"
        "nTellWrite = FTELL(hWrite)\n"
        "nCloseWrite = FCLOSE(hWrite)\n"
        "nAppend = STRTOFILE('ZZ', 'write.txt', 1)\n"
        "cWriteAfterAppend = FILETOSTR('write.txt')\n"
        "hResize = FOPEN('write.txt', 2)\n"
        "nResize = FCHSIZE(hResize, 2)\n"
        "nCloseResize = FCLOSE(hResize)\n"
        "cWriteAfterResize = FILETOSTR('write.txt')\n"
        "hCreate = FOPEN('created-read-write.txt', 2)\n"
        "nCreateWrite = FWRITE(hCreate, 'created')\n"
        "nCloseCreate = FCLOSE(hCreate)\n"
        "cCreated = FILETOSTR('created-read-write.txt')\n"
        "hSharedRead = FOPEN('write.txt', 10)\n"
        "cSharedRead = FREAD(hSharedRead, 2)\n"
        "nCloseSharedRead = FCLOSE(hSharedRead)\n"
        "hSharedWrite = FOPEN('shared-write.txt', 11)\n"
        "nSharedWrite = FWRITE(hSharedWrite, 'shared')\n"
        "nCloseSharedWrite = FCLOSE(hSharedWrite)\n"
        "cSharedWrite = FILETOSTR('shared-write.txt')\n"
        "hSharedReadWrite = FOPEN('shared-read-write.txt', 12)\n"
        "nSharedReadWrite = FWRITE(hSharedReadWrite, 'both')\n"
        "nCloseSharedReadWrite = FCLOSE(hSharedReadWrite)\n"
        "cSharedReadWrite = FILETOSTR('shared-read-write.txt')\n"
        "hMissing = FOPEN('missing/does-not-exist.txt', 0)\n"
        "nMissingError = FERROR()\n"
        "hErrorReset = FOPEN('rw.txt', 0)\n"
        "nResetError = FERROR()\n"
        "nInvalidRead = FREAD(-999, 1)\n"
        "nInvalidHandleError = FERROR()\n"
        "hSeekError = FOPEN('rw.txt', 0)\n"
        "nBadSeek = FSEEK(hSeekError, -1, 0)\n"
        "nSeekError = FERROR()\n"
        "nCloseSeekError = FCLOSE(hSeekError)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "file I/O function script should complete: " + state.message);

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

    check("nwrite", "11");
    check("cwhole", "line1\nline2");
    check("nwritebackslash", "11");
    check("cbackslashwhole", "nested-data");
    check("cbackslashchunk", "nested");
    check("nclosebackslashread", "0");
    check("cchunk", "line");
    check("ntellchunk", "4");
    check("nseekstart", "0");
    check("cline1", "line1");
    check("cline2", "line2");
    check("cline3", "");
    check("leofread", "true");
    check("ncloseread", "0");
    check("ctail", "line2");
    check("nclosetail", "0");
    check("nput", "4");
    check("nflush", "0");
    check("ntellwrite", "4");
    check("nclosewrite", "0");
    check("nappend", "2");
    check("cwriteafterappend", "abc\nZZ");
    check("nresize", "0");
    check("ncloseresize", "0");
    check("cwriteafterresize", "ab");
    check("ncreatewrite", "7");
    check("nclosecreate", "0");
    check("ccreated", "created");
    check("csharedread", "ab");
    check("nclosesharedread", "0");
    check("nsharedwrite", "6");
    check("nclosesharedwrite", "0");
    check("csharedwrite", "shared");
    check("nsharedreadwrite", "4");
    check("nclosesharedreadwrite", "0");
    check("csharedreadwrite", "both");
    check("hmissing", "-1");
    check("nmissingerror", "2");
    check("nreseterror", "0");
    check("ninvalidhandleerror", "6");
    check("nseekerror", "25");

    fs::remove_all(temp_root, ignored);
}

void test_fdate_ftime_runtime_functions()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_fdate_ftime_functions";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    // Only discoverable through SET PATH -- not in the default directory.
    const fs::path path_probe_dir = temp_root / "path_probe";
    fs::create_directories(path_probe_dir);
    write_text(path_probe_dir / "path_only.txt", "found-via-set-path");

    const fs::path main_path = temp_root / "fdate_ftime_functions.prg";
    write_text(
        main_path,
        "nCreated = STRTOFILE('data', 'created.txt')\n"
        "dFileDate = FDATE('created.txt')\n"
        "dFileDateDefault = FDATE('created.txt', 0)\n"
        "tFileDateTime = FDATE('created.txt', 1)\n"
        "cFileTime = FTIME('created.txt')\n"
        "lDateMatchesToday = (dFileDate == DATE())\n"
        "lDateDefaultMatchesToday = (dFileDateDefault == DATE())\n"
        "lDateTimeDateMatchesToday = (TTOD(tFileDateTime) == DATE())\n"
        "nTimeLength = LEN(cFileTime)\n"
        "cTimeColonOne = SUBSTR(cFileTime, 3, 1)\n"
        "cTimeColonTwo = SUBSTR(cFileTime, 6, 1)\n"
        "dMissingDate = FDATE('does-not-exist-xyz.txt')\n"
        "cMissingTime = FTIME('does-not-exist-xyz.txt')\n"
        "lMissingDateEmpty = EMPTY(dMissingDate)\n"
        "lMissingTimeEmpty = EMPTY(cMissingTime)\n"
        "dPathOnlyBefore = FDATE('path_only.txt')\n"
        "cPathOnlyTimeBefore = FTIME('path_only.txt')\n"
        "SET PATH TO '" + path_probe_dir.string() + "'\n"
        "dPathOnlyAfter = FDATE('path_only.txt')\n"
        "cPathOnlyTimeAfter = FTIME('path_only.txt')\n"
        "lPathOnlyBeforeEmpty = EMPTY(dPathOnlyBefore)\n"
        "lPathOnlyTimeBeforeEmpty = EMPTY(cPathOnlyTimeBefore)\n"
        "lPathOnlyAfterMatchesToday = (dPathOnlyAfter == DATE())\n"
        "nPathOnlyTimeAfterLength = LEN(cPathOnlyTimeAfter)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "FDATE/FTIME function script should complete: " + state.message);

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

    check("ldatematchestoday", "true");
    check("ldatedefaultmatchestoday", "true");
    check("ldatetimedatematchestoday", "true");
    check("ntimelength", "8");
    check("ctimecolonone", ":");
    check("ctimecolontwo", ":");
    check("lmissingdateempty", "true");
    check("lmissingtimeempty", "true");
    check("lpathonlybeforeempty", "true");
    check("lpathonlytimebeforeempty", "true");
    check("lpathonlyaftermatchestoday", "true");
    check("npathonlytimeafterlength", "8");

    fs::remove_all(temp_root, ignored);
}

void test_fcreate_runtime_function()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_fcreate_functions";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    // Pre-existing content that FCREATE() must overwrite without warning.
    const fs::path existing_path = temp_root / "existing.txt";
    write_text(existing_path, "stale content that should be discarded");

    const fs::path main_path = temp_root / "fcreate_functions.prg";
    write_text(
        main_path,
        "hDefault = FCREATE('created.txt')\n"
        "nDefaultWrite = FWRITE(hDefault, 'default-attribute')\n"
        "nCloseDefault = FCLOSE(hDefault)\n"
        "cDefaultContent = FILETOSTR('created.txt')\n"
        "hOverwrite = FCREATE('existing.txt')\n"
        "nOverwriteWrite = FWRITE(hOverwrite, 'fresh')\n"
        "nCloseOverwrite = FCLOSE(hOverwrite)\n"
        "cOverwriteContent = FILETOSTR('existing.txt')\n"
        "hAttributed = FCREATE('attributed.txt', 1)\n"
        "nBlockedWrite = FWRITE(hAttributed, 'should not land')\n"
        "nBlockedError = FERROR()\n"
        "nBlockedPut = FPUTS(hAttributed, 'still blocked')\n"
        "nCloseAttributed = FCLOSE(hAttributed)\n"
        "cAttributedAfterFirstClose = FILETOSTR('attributed.txt')\n"
        "hReopened = FOPEN('attributed.txt', 12)\n"
        "nReopenedWrite = FWRITE(hReopened, 'now writable')\n"
        "nCloseReopened = FCLOSE(hReopened)\n"
        "cAttributedAfterReopen = FILETOSTR('attributed.txt')\n"
        "hMissingDir = FCREATE('missing-dir/nested.txt')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "FCREATE() function script should complete: " + state.message);

    const auto check = [&](const std::string &name, const std::string &expected)
    {
        const auto it = state.globals.find(name);
        if (it == state.globals.end())
        {
            expect(false, name + " variable should be present for FCREATE() test");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
               name + " should equal '" + expected + "' for FCREATE() test");
    };
    // File handle numbers come from a process-wide counter shared with earlier
    // tests in this executable, so only their positivity (a valid handle) is
    // meaningful here, not a specific value.
    const auto check_valid_handle = [&](const std::string &name)
    {
        const auto it = state.globals.find(name);
        if (it == state.globals.end())
        {
            expect(false, name + " variable should be present for FCREATE() test");
            return;
        }
        double handle_value = -1.0;
        try
        {
            handle_value = std::stod(copperfin::runtime::format_value(it->second));
        }
        catch (...)
        {
        }
        expect(handle_value > 0.0,
               name + " should be a valid (positive) file handle for FCREATE() test");
    };

    check_valid_handle("hdefault");
    check("ndefaultwrite", "17");
    check("nclosedefault", "0");
    check("cdefaultcontent", "default-attribute");
    check_valid_handle("hoverwrite");
    check("noverwritewrite", "5");
    check("ncloseoverwrite", "0");
    check("coverwritecontent", "fresh");
    check_valid_handle("hattributed");
    check("nblockedwrite", "-1");
    check("nblockederror", "6");
    check("nblockedput", "-1");
    check("ncloseattributed", "0");
    check("cattributedafterfirstclose", std::string{});
    check_valid_handle("hreopened");
    check("nreopenedwrite", "12");
    check("nclosereopened", "0");
    check("cattributedafterreopen", "now writable");
    check("hmissingdir", "-1");

    fs::remove_all(temp_root, ignored);
}

void test_unicode_paths_survive_prg_file_io_and_includes()
{
    namespace fs = std::filesystem;
    std::string unicode_root_name = "copperfin_prg_engine_unicode_";
    unicode_root_name += "\xC3\xA9";
    std::string unicode_file_name = "caf";
    unicode_file_name += "\xC3\xA9.inc";
    std::string unicode_data_name = "r";
    unicode_data_name += "\xC3\xA9sum\xC3\xA9.txt";

    const fs::path temp_root =
        fs::temp_directory_path() / copperfin::platform::path_from_utf8_string(unicode_root_name);
    const fs::path main_path = temp_root / "main.prg";
    const fs::path include_path =
        temp_root / "includes" / copperfin::platform::path_from_utf8_string(unicode_file_name);
    const fs::path data_path = temp_root / copperfin::platform::path_from_utf8_string(unicode_data_name);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(include_path.parent_path());

    write_text(include_path, "#DEFINE IncludedValue 42\n");
    std::string source = "#INCLUDE \"includes\\";
    source += unicode_file_name;
    source += "\"\n";
    source += "nIncluded = IncludedValue\n";
    source += "nWrite = STRTOFILE('payload', '";
    source += unicode_data_name;
    source += "')\n";
    source += "cRead = FILETOSTR('";
    source += unicode_data_name;
    source += "')\n";
    source += "hRead = FOPEN('";
    source += unicode_data_name;
    source += "', 0)\n";
    source += "cChunk = FREAD(hRead, 7)\n";
    source += "nClose = FCLOSE(hRead)\nRETURN\n";
    write_text(main_path, source);

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "Unicode-path PRG script should complete: " + state.message);
    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " variable should be present for Unicode-path PRG test");
        if (it != state.globals.end()) {
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " should equal '" + expected + "' for Unicode-path PRG test");
        }
    };
    check("nincluded", "42");
    check("nwrite", "7");
    check("cread", "payload");
    check("cchunk", "payload");
    check("nclose", "0");
    expect(fs::exists(data_path), "Unicode-path STRTOFILE should create the expected file");

    fs::remove_all(temp_root, ignored);
}

} // namespace

int main()
{
    test_file_io_runtime_functions();
    test_fdate_ftime_runtime_functions();
    test_fcreate_runtime_function();
    test_unicode_paths_survive_prg_file_io_and_includes();

    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
