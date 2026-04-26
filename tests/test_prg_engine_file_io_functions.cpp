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
        "hMissing = FOPEN('missing/does-not-exist.txt', 0)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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
    check("hmissing", "-1");

    fs::remove_all(temp_root, ignored);
}

} // namespace

int main()
{
    test_file_io_runtime_functions();

    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
