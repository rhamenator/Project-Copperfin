// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"
#include "../src/runtime/prg_engine_file_io_functions.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "../src/runtime/prg_compatibility_error.h"

#include <cstdlib>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <limits>
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

// Governing requirement: RQ-CF-PRG-FSEEK-NUMERIC-BOUNDS-001 (#5611/#6776).
void test_fseek_numeric_boundaries()
{
    using namespace copperfin::runtime;
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / "copperfin_prg_engine_fseek_boundaries";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    write_text(root / "data.txt", "abcdef");

    struct Row {
        const char* name;
        PrgValue value;
        bool origin;
        bool strict_error;
        bool vfp_error;
        double vfp_position;
    };
    const std::vector<Row> rows{
        {"offset fraction", make_number_value(1.9), false, true, false, 1},
        {"offset negative fraction", make_number_value(-0.9), false, true, false, 0},
        {"offset positive wrap", make_number_value(4294967297.0), false, true, false, 1},
        {"offset negative wrap", make_number_value(-4294967296.0), false, true, false, 0},
        {"offset huge", make_number_value(1E300), false, true, false, 0},
        {"offset negative huge", make_number_value(-1E300), false, true, false, 0},
        {"offset infinity", make_number_value(std::numeric_limits<double>::infinity()), false, true, false, 0},
        {"offset negative infinity", make_number_value(-std::numeric_limits<double>::infinity()), false, true, false, 0},
        {"offset NaN", make_number_value(std::numeric_limits<double>::quiet_NaN()), false, true, true, 0},
        {"offset Currency fraction", make_currency_value(19000), false, true, false, 1},
        {"offset Currency subunit", make_currency_value(9000), false, true, true, 0},
        {"offset Currency negative subunit", make_currency_value(-9000), false, true, true, 0},
        {"offset Currency negative wrap", make_currency_value(-42949672960000LL), false, true, false, 0},
        {"offset Currency negative limit fraction", make_currency_value(-42949672969000LL), false, true, false, 0},
        {"offset Currency positive limit", make_currency_value(42949672960000LL), false, true, true, 0},
        {"offset Currency negative overflow", make_currency_value(-42949672970000LL), false, true, true, 0},
        {"offset exact int64 low bits", make_int64_value(9007199254740993LL), false, true, false, 1},
        {"offset exact uint64 low bits", make_uint64_value(9007199254740993ULL), false, true, false, 1},
        {"offset below int32", make_number_value(-2147483649.0), false, true, false, 2147483647},
        {"offset maximum", make_number_value(2147483647.0), false, false, false, 2147483647},
        {"offset immediately above maximum", make_number_value(std::nextafter(2147483647.0, 2147483648.0)), false, true, false, 2147483647},
        {"offset above int32", make_number_value(2147483648.0), false, true, false, -1},
        {"offset minimum", make_number_value(-2147483648.0), false, false, false, -1},
        {"offset immediately below minimum", make_number_value(std::nextafter(-2147483648.0, -2147483649.0)), false, true, false, -1},
        {"offset max Currency", make_currency_value(21474836479000LL), false, true, false, 2147483647},
        {"origin zero", make_number_value(0), true, false, false, 1},
        {"origin current", make_number_value(1), true, false, false, 4},
        {"origin end", make_number_value(2), true, false, false, 7},
        {"origin fraction", make_number_value(1.9), true, true, false, 4},
        {"origin above ceiling", make_number_value(2.9), true, true, true, 0},
        {"origin negative fraction", make_number_value(-0.9), true, true, false, 1},
        {"origin negative", make_number_value(-1), true, true, true, 0},
        {"origin positive wrap", make_number_value(4294967296.0), true, true, true, 0},
        {"origin negative wrap", make_number_value(-4294967295.0), true, true, false, 4},
        {"origin huge", make_number_value(1E300), true, true, true, 0},
        {"origin negative huge", make_number_value(-1E300), true, true, false, 1},
        {"origin infinity", make_number_value(std::numeric_limits<double>::infinity()), true, true, true, 0},
        {"origin negative infinity", make_number_value(-std::numeric_limits<double>::infinity()), true, true, false, 1},
        {"origin NaN", make_number_value(std::numeric_limits<double>::quiet_NaN()), true, true, true, 0},
        {"origin Currency fraction", make_currency_value(29000), true, true, false, 7},
        {"origin Currency subunit", make_currency_value(9000), true, true, true, 0},
        {"origin Currency negative wrap", make_currency_value(-42949672960000LL), true, true, false, 1},
        {"origin Currency negative limit fraction", make_currency_value(-42949672969000LL), true, true, false, 1},
        {"origin exact positive", make_uint64_value(std::numeric_limits<std::uint64_t>::max()), true, true, true, 0},
        {"origin exact negative", make_int64_value(-9007199254740991LL), true, true, false, 4},
    };
    for (const bool verified : {false, true}) {
        for (const bool vfp : {false, true}) {
            const auto call = [&](const std::string& function, const std::vector<PrgValue>& arguments) {
                return evaluate_file_io_function(function, arguments, root.string(), verified,
                    [](const fs::path&) -> std::optional<std::string> { return "abcdef"; }, {},
                    [vfp](const std::string&) { return vfp ? "VFP9" : "COPPERFIN"; }).value();
            };
            const auto handle = call("fopen", {make_string_value("data.txt"), make_number_value(0)});
            expect(value_as_number(handle) > 0, "FSEEK fixture must open");
            const auto check_row = [&](const Row& row) {
                call("fseek", {handle, make_number_value(3), make_number_value(0)});
                bool rejected = false;
                try {
                    const auto result = row.origin
                        ? call("fseek", {handle, make_number_value(1), row.value})
                        : call("fseek", {handle, row.value, make_number_value(0)});
                    if (vfp || !row.strict_error) {
                        expect(value_as_number(result) == row.vfp_position, std::string(row.name) + " VFP9 result");
                    }
                } catch (const PrgCompatibilityError& error) {
                    rejected = true;
                    expect(error.error_code() == 11, std::string(row.name) + " must raise error 11");
                }
                expect(rejected == (vfp ? row.vfp_error : row.strict_error), std::string(row.name) + " admission");
                if (rejected || (vfp && row.vfp_position == -1)) {
                    expect(value_as_number(call("ftell", {handle})) == 3, std::string(row.name) + " must preserve position");
                }
            };
            for (const auto& row : rows) {
                check_row(row);
            }
            for (const auto& invalid : {make_boolean_value(true), make_string_value("1"), make_null_value(), PrgValue{}}) {
                check_row({"offset invalid type", invalid, false, true, true, 0});
                check_row({"origin invalid type", invalid, true, true, true, 0});
            }
            call("fseek", {handle, make_number_value(3)});
            expect(value_as_number(call("ftell", {handle})) == 3, "omitted origin selects start");
            call("fclose", {handle});
        }
    }
    const fs::path script = root / "fseek_boundaries.prg";
    write_text(script,
        "h = FOPEN('data.txt', 0)\n"
        "=FSEEK(h, 3)\n"
        "TRY\n"
        "  nUnexpected = FSEEK(h, 1E300)\n"
        "CATCH TO oError\n"
        "  nOffsetError = oError.ErrorNo\n"
        "ENDTRY\n"
        "nPosition = FTELL(h)\n"
        "TRY\n"
        "  nUnexpected = FSEEK(-999, .T., 0)\n"
        "CATCH TO oError\n"
        "  nBadHandleOffsetError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  nUnexpected = FSEEK(-999, 0, .T.)\n"
        "CATCH TO oError\n"
        "  nBadHandleOriginError = oError.ErrorNo\n"
        "ENDTRY\n"
        "SET NUMERICBEHAVIOR TO VFP9\n"
        "nWrapped = FSEEK(h, 4294967297, 0)\n"
        "=FCLOSE(h)\n"
        "RETURN\n");
    auto session = PrgRuntimeSession::create(make_runtime_session_options(script, root));
    const auto state = session.run(DebugResumeAction::continue_run);
    expect(state.completed, "FSEEK boundary errors must be catchable: " + state.message);
    for (const auto* name : {"noffseterror", "nbadhandleoffseterror", "nbadhandleoriginerror"}) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end() && format_value(found->second) == "11", std::string(name) + " error 11");
    }
    const auto position = state.globals.find("nposition");
    expect(position != state.globals.end() && format_value(position->second) == "3", "script rejection preserves position");
    const auto wrapped = state.globals.find("nwrapped");
    expect(wrapped != state.globals.end() && format_value(wrapped->second) == "1", "script selects VFP9 conversion");
    fs::remove_all(root, ignored);
}

// Governing requirement: RQ-CF-PRG-FILE-HANDLE-NUMERIC-001 (#5611/#6776).
void test_file_handle_numeric_boundaries()
{
    using namespace copperfin::runtime;
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / "copperfin_prg_engine_file_handle_boundaries";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    struct Row {
        const char* name;
        std::function<PrgValue(std::int64_t)> argument;
        bool strict_error;
        bool vfp_error;
        bool strict_live;
        bool vfp_live;
    };
    const std::vector<Row> rows{
        {"exact", [](std::int64_t h) { (void)h; return make_number_value(h); }, false, false, true, true},
        {"exact int64", [](std::int64_t h) { (void)h; return make_int64_value(h); }, false, false, true, true},
        {"exact uint64", [](std::int64_t h) { (void)h; return make_uint64_value(static_cast<std::uint64_t>(h)); }, false, false, true, true},
        {"exact Currency", [](std::int64_t h) { (void)h; return make_currency_value(h * INT64_C(10000)); }, false, false, true, true},
        {"fraction", [](std::int64_t h) { (void)h; return make_number_value(h + 0.9); }, true, false, false, true},
        {"immediately above", [](std::int64_t h) { (void)h; return make_number_value(std::nextafter(static_cast<double>(h), h + 1.0)); }, true, false, false, true},
        {"fraction below", [](std::int64_t h) { (void)h; return make_number_value(h - 0.1); }, true, false, false, false},
        {"immediately below", [](std::int64_t h) { (void)h; return make_number_value(std::nextafter(static_cast<double>(h), h - 1.0)); }, true, false, false, false},
        {"negative fraction", [](std::int64_t h) { (void)h; return make_number_value(-0.9); }, true, false, false, false},
        {"zero", [](std::int64_t h) { (void)h; return make_number_value(0); }, false, false, false, false},
        {"negative", [](std::int64_t h) { (void)h; return make_number_value(-999); }, false, false, false, false},
        {"int32 maximum", [](std::int64_t h) { (void)h; return make_number_value(2147483647); }, false, false, false, false},
        {"int32 minimum", [](std::int64_t h) { (void)h; return make_number_value(-2147483648.0); }, false, false, false, false},
        {"above int32", [](std::int64_t h) { (void)h; return make_number_value(2147483648.0); }, true, false, false, false},
        {"below int32", [](std::int64_t h) { (void)h; return make_number_value(-2147483649.0); }, true, false, false, false},
        {"above int32 uint64", [](std::int64_t h) { (void)h; return make_uint64_value(UINT64_C(2147483648)); }, true, false, false, false},
        {"below int32 int64", [](std::int64_t h) { (void)h; return make_int64_value(-INT64_C(2147483649)); }, true, false, false, false},
        {"positive wrap", [](std::int64_t h) { (void)h; return make_number_value(h + 4294967296.0); }, true, false, false, true},
        {"negative wrap", [](std::int64_t h) { (void)h; return make_number_value(h - 4294967296.0); }, true, false, false, true},
        {"negative double wrap", [](std::int64_t h) { (void)h; return make_number_value(h - 8589934592.0); }, true, false, false, true},
        {"uint16 not a wrap", [](std::int64_t h) { (void)h; return make_number_value(h + 65536.0); }, false, false, false, false},
        {"huge", [](std::int64_t h) { (void)h; return make_number_value(1E300); }, true, false, false, false},
        {"negative huge", [](std::int64_t h) { (void)h; return make_number_value(-1E300); }, true, false, false, false},
        {"infinity", [](std::int64_t h) { (void)h; return make_number_value(std::numeric_limits<double>::infinity()); }, true, false, false, false},
        {"negative infinity", [](std::int64_t h) { (void)h; return make_number_value(-std::numeric_limits<double>::infinity()); }, true, false, false, false},
        {"NaN", [](std::int64_t h) { (void)h; return make_number_value(std::numeric_limits<double>::quiet_NaN()); }, true, true, false, false},
        {"exact int64 low bits", [](std::int64_t h) { (void)h; return make_int64_value(INT64_C(9007199254740992) + h); }, true, false, false, true},
        {"exact uint64 low bits", [](std::int64_t h) { (void)h; return make_uint64_value(UINT64_C(9007199254740992) + static_cast<std::uint64_t>(h)); }, true, false, false, true},
        {"exact unsigned high low bits", [](std::int64_t h) { (void)h; return make_uint64_value(UINT64_C(9223372036854775808) + static_cast<std::uint64_t>(h)); }, true, false, false, true},
        {"int64 maximum", [](std::int64_t h) { (void)h; return make_int64_value(std::numeric_limits<std::int64_t>::max()); }, true, false, false, false},
        {"int64 minimum", [](std::int64_t h) { (void)h; return make_int64_value(std::numeric_limits<std::int64_t>::min()); }, true, false, false, false},
        {"uint64 maximum", [](std::int64_t h) { (void)h; return make_uint64_value(std::numeric_limits<std::uint64_t>::max()); }, true, false, false, false},
        {"Currency fraction", [](std::int64_t h) { (void)h; return make_currency_value(h * INT64_C(10000) + 9000); }, true, false, false, true},
        {"Currency subunit", [](std::int64_t h) { (void)h; return make_currency_value(9000); }, true, false, false, false},
        {"Currency negative subunit", [](std::int64_t h) { (void)h; return make_currency_value(-9000); }, true, false, false, false},
        {"Currency positive wrap invalid", [](std::int64_t h) { (void)h; return make_currency_value((h + INT64_C(4294967296)) * INT64_C(10000)); }, true, false, false, false},
        {"Currency negative wrap", [](std::int64_t h) { (void)h; return make_currency_value((h - INT64_C(4294967296)) * INT64_C(10000)); }, true, false, false, true},
        {"Currency negative double wrap invalid", [](std::int64_t h) { (void)h; return make_currency_value((h - INT64_C(8589934592)) * INT64_C(10000)); }, true, false, false, false},
        {"Currency positive limit", [](std::int64_t h) { (void)h; return make_currency_value(INT64_C(42949672960000)); }, true, false, false, false},
        {"Currency negative limit fraction", [](std::int64_t h) { (void)h; return make_currency_value(-INT64_C(42949672969000)); }, true, false, false, false},
        {"Currency huge", [](std::int64_t h) { (void)h; return make_currency_value(std::numeric_limits<std::int64_t>::max()); }, true, false, false, false},
        {"immediately above int32 maximum", [](std::int64_t) { return make_number_value(std::nextafter(2147483647.0, 2147483648.0)); }, true, false, false, false},
        {"immediately below int32 minimum", [](std::int64_t) { return make_number_value(std::nextafter(-2147483648.0, -2147483649.0)); }, true, false, false, false},
        {"Currency int32 maximum", [](std::int64_t) { return make_currency_value(INT64_C(21474836470000)); }, false, false, false, false},
        {"Currency int32 minimum", [](std::int64_t) { return make_currency_value(-INT64_C(21474836480000)); }, false, false, false, false},
        {"Currency above int32", [](std::int64_t) { return make_currency_value(INT64_C(21474836480000)); }, true, false, false, false},
        {"Currency below int32", [](std::int64_t) { return make_currency_value(-INT64_C(21474836490000)); }, true, false, false, false},
        {"Logical", [](std::int64_t h) { (void)h; return make_boolean_value(true); }, false, false, false, false},
        {"numeric Character", [](std::int64_t h) { (void)h; return make_string_value(std::to_string(h)); }, false, false, false, false},
        {"NULL", [](std::int64_t h) { (void)h; return make_null_value(); }, false, false, false, false},
        {"Empty", [](std::int64_t h) { (void)h; return PrgValue{}; }, false, false, false, false},
    };
    for (const bool verified : {false, true}) {
        for (const bool vfp : {false, true}) {
            const auto call = [&](const std::string& function, const std::vector<PrgValue>& arguments) {
                return evaluate_file_io_function(function, arguments, root.string(), verified,
                    [](const fs::path&) -> std::optional<std::string> { return "abcdef"; }, {},
                    [vfp](const std::string&) { return vfp ? "VFP9" : "COPPERFIN"; }).value();
            };
            for (const std::string function : {"fclose", "fread", "fwrite", "fgets", "fputs",
                     "fseek", "ftell", "feof", "fflush", "fchsize"}) {
                for (const auto& row : rows) {
                    write_text(root / "data.txt", "abcdef");
                    const auto handle = call("fopen", {make_string_value("data.txt"), make_number_value(verified ? 0 : 2)});
                    call("fseek", {handle, make_number_value(3)});
                    std::vector<PrgValue> arguments{row.argument(static_cast<std::int64_t>(value_as_number(handle)))};
                    if (function == "fread" || function == "fgets" || function == "fseek") {
                        arguments.push_back(make_number_value(1));
                    } else if (function == "fwrite" || function == "fputs") {
                        arguments.push_back(make_string_value("Z"));
                    } else if (function == "fchsize") {
                        arguments.push_back(make_number_value(3));
                    }
                    const std::string label = function + " " + row.name + (vfp ? " VFP9" : " COPPERFIN") +
                        (verified ? " verified" : " native");
                    const bool should_reject = vfp ? row.vfp_error : row.strict_error;
                    const bool live = vfp ? row.vfp_live : row.strict_live;
                    const bool blocked_write = verified &&
                        (function == "fwrite" || function == "fputs" || function == "fchsize");
                    bool rejected = false;
                    try {
                        const auto result = call(function, arguments);
                        const auto file_error = call("ferror", {});
                        if (!should_reject) {
                            std::string expected;
                            if (!live || blocked_write) {
                                // Existing return gaps #5913/#5914/#6959 remain
                                // separate from conversion; pin current results.
                                expected = function == "fread" || function == "fgets" ? "" :
                                    function == "feof" ? "true" : "-1";
                            } else {
                                expected = function == "fread" || function == "fgets" ? "d" :
                                    function == "feof" ? "false" : function == "ftell" ? "3" :
                                    function == "fwrite" || function == "fseek" ? "1" :
                                    function == "fputs" ? "2" : "0";
                            }
                            expect(format_value(result) == expected, label + " result");
                            expect(value_as_number(file_error) == ((!live || blocked_write) ? 6 : 0), label + " FERROR");
                        }
                    } catch (const PrgCompatibilityError& error) {
                        rejected = true;
                        expect(error.error_code() == 11, label + " error 11");
                    }
                    expect(rejected == should_reject, label + " admission");
                    if (rejected) {
                        expect(value_as_number(call("ferror", {})) == 0, label + " must not mutate FERROR on argument error");
                        // Review regression: testing only an initially clear
                        // FERROR cannot detect clearing it before rejection.
                        call("ftell", {make_number_value(0)});
                        expect(value_as_number(call("ferror", {})) == 6, label + " seed existing FERROR");
                        bool rejected_again = false;
                        try {
                            call(function, arguments);
                        } catch (const PrgCompatibilityError& error) {
                            rejected_again = true;
                            expect(error.error_code() == 11, label + " repeated error 11");
                        }
                        expect(rejected_again, label + " repeat rejection with existing FERROR");
                        expect(value_as_number(call("ferror", {})) == 6, label + " preserve existing FERROR on argument error");
                    }
                    const bool closed = !rejected && live && function == "fclose";
                    const double expected_position = closed ? -1 :
                        rejected || !live || blocked_write ? 3 :
                        function == "fseek" ? 1 : function == "fread" || function == "fgets" || function == "fwrite" ? 4 :
                        function == "fputs" ? 5 : 3;
                    expect(value_as_number(call("ftell", {handle})) == expected_position, label + " target position/lifetime");
                    call("fclose", {handle});
                    const std::string expected_bytes = rejected || !live || verified ? "abcdef" :
                        function == "fwrite" ? "abcZef" : function == "fputs" ? "abcZ\nf" :
                        function == "fchsize" ? "abc" : "abcdef";
                    expect(read_text(root / "data.txt") == expected_bytes, label + " persistent bytes");
                }
            }
        }
    }
    const fs::path script = root / "file_handle_boundaries.prg";
    write_text(root / "data.txt", "abcdef");
    write_text(script,
        "h = FOPEN('data.txt', 2)\n"
        "=FSEEK(h, 3)\n"
        "TRY\n"
        "  cUnexpected = FREAD(h + 0.9, 1)\n"
        "CATCH TO oError\n"
        "  nReadError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  nUnexpected = FCLOSE(h + 4294967296)\n"
        "CATCH TO oError\n"
        "  nCloseError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  nUnexpected = FCHSIZE(h + 4294967296, 0)\n"
        "CATCH TO oError\n"
        "  nResizeError = oError.ErrorNo\n"
        "ENDTRY\n"
        "nPosition = FTELL(h)\n"
        "cPreserved = FILETOSTR('data.txt')\n"
        "cInvalid = FREAD(TRANSFORM(h), 1)\n"
        "nTypeError = FERROR()\n"
        "SET NUMERICBEHAVIOR TO VFP9\n"
        "cFraction = FREAD(h + 0.9, 1)\n"
        "nWrappedClose = FCLOSE(h + 4294967296)\n"
        "nAfterClose = FTELL(h)\n"
        "RETURN\n");
    auto session = PrgRuntimeSession::create(make_runtime_session_options(script, root));
    const auto state = session.run(DebugResumeAction::continue_run);
    expect(state.completed, "file-handle script errors must be catchable: " + state.message);
    for (const auto& [name, expected] : std::vector<std::pair<std::string, std::string>>{
             {"nreaderror", "11"}, {"ncloseerror", "11"}, {"nresizeerror", "11"},
             {"nposition", "3"}, {"cpreserved", "abcdef"}, {"cinvalid", ""}, {"ntypeerror", "6"},
             {"cfraction", "d"}, {"nwrappedclose", "0"}, {"nafterclose", "-1"}}) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end() && format_value(found->second) == expected, name + " script result");
    }
    fs::remove_all(root, ignored);
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
        "LOCAL uEmpty\n"
        "nCreated = STRTOFILE('data', 'created.txt')\n"
        "dFileDate = FDATE('created.txt')\n"
        "dFileDateDefault = FDATE('created.txt', 0)\n"
        "tFileDateTime = FDATE('created.txt', 1)\n"
        "cFileTime = FTIME('created.txt')\n"
        "lDateMatchesToday = (dFileDate == DATE())\n"
        "lDateDefaultMatchesToday = (dFileDateDefault == DATE())\n"
        "lDateTimeDateMatchesToday = (TTOD(tFileDateTime) == DATE())\n"
        "SET NUMERICBEHAVIOR TO COPPERFIN\n"
        "TRY\n"
        "  dUnexpectedCopperfinFraction = FDATE('created.txt', 0.9)\n"
        "  nCopperfinFractionError = 0\n"
        "CATCH TO oError\n"
        "  nCopperfinFractionError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  dUnexpectedCopperfinMissingFraction = FDATE('missing-flag.txt', 0.9)\n"
        "  nCopperfinMissingFractionError = 0\n"
        "CATCH TO oError\n"
        "  nCopperfinMissingFractionError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  dUnexpectedCopperfinInfinite = FDATE('created.txt', EXP(1000))\n"
        "  nCopperfinInfiniteError = 0\n"
        "CATCH TO oError\n"
        "  nCopperfinInfiniteError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  dUnexpectedCopperfinCurrencyFraction = FDATE('created.txt', $1.9000)\n"
        "  nCopperfinCurrencyFractionError = 0\n"
        "CATCH TO oError\n"
        "  nCopperfinCurrencyFractionError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  dUnexpectedCopperfinLogical = FDATE('created.txt', .T.)\n"
        "  nCopperfinLogicalError = 0\n"
        "CATCH TO oError\n"
        "  nCopperfinLogicalError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  dUnexpectedCopperfinCharacter = FDATE('created.txt', '1')\n"
        "  nCopperfinCharacterError = 0\n"
        "CATCH TO oError\n"
        "  nCopperfinCharacterError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  dUnexpectedCopperfinEmpty = FDATE('created.txt', uEmpty)\n"
        "  nCopperfinEmptyError = 0\n"
        "CATCH TO oError\n"
        "  nCopperfinEmptyError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  dUnexpectedCopperfinNull = FDATE('created.txt', .NULL.)\n"
        "  nCopperfinNullError = 0\n"
        "CATCH TO oError\n"
        "  nCopperfinNullError = oError.ErrorNo\n"
        "ENDTRY\n"
        "SET NUMERICBEHAVIOR TO VFP9\n"
        "lVfpFractionDate = (FDATE('created.txt', 0.9) == DATE())\n"
        "lVfpNegativeFractionDate = (FDATE('created.txt', -0.9) == DATE())\n"
        "lVfpNegativeWrapDate = (FDATE('created.txt', -4294967296) == DATE())\n"
        "lVfpNegativeHugeDate = (FDATE('created.txt', -1E300) == DATE())\n"
        "lVfpNegativeInfiniteDate = (FDATE('created.txt', -EXP(1000)) == DATE())\n"
        "lVfpCurrencyFractionDateTime = (TTOD(FDATE('created.txt', $1.9000)) == DATE())\n"
        "lVfpCurrencyNegativeWrapDate = (FDATE('created.txt', $-4294967296.0000) == DATE())\n"
        "TRY\n"
        "  dUnexpectedVfpAboveOne = FDATE('created.txt', 1.1)\n"
        "  nVfpAboveOneError = 0\n"
        "CATCH TO oError\n"
        "  nVfpAboveOneError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  dUnexpectedVfpHuge = FDATE('created.txt', 1E300)\n"
        "  nVfpHugeError = 0\n"
        "CATCH TO oError\n"
        "  nVfpHugeError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  dUnexpectedVfpInfinite = FDATE('created.txt', EXP(1000))\n"
        "  nVfpInfiniteError = 0\n"
        "CATCH TO oError\n"
        "  nVfpInfiniteError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  dUnexpectedVfpCurrencySubunit = FDATE('created.txt', $0.9000)\n"
        "  nVfpCurrencySubunitError = 0\n"
        "CATCH TO oError\n"
        "  nVfpCurrencySubunitError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  dUnexpectedVfpCurrencyPositiveWrap = FDATE('created.txt', $4294967296.0000)\n"
        "  nVfpCurrencyPositiveWrapError = 0\n"
        "CATCH TO oError\n"
        "  nVfpCurrencyPositiveWrapError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  dUnexpectedVfpCharacter = FDATE('created.txt', '1')\n"
        "  nVfpCharacterError = 0\n"
        "CATCH TO oError\n"
        "  nVfpCharacterError = oError.ErrorNo\n"
        "ENDTRY\n"
        "SET NUMERICBEHAVIOR TO COPPERFIN\n"
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
    check("ncopperfinfractionerror", "11");
    check("ncopperfinmissingfractionerror", "11");
    check("ncopperfininfiniteerror", "11");
    check("ncopperfincurrencyfractionerror", "11");
    check("ncopperfinlogicalerror", "11");
    check("ncopperfincharactererror", "11");
    check("ncopperfinemptyerror", "11");
    check("ncopperfinnullerror", "11");
    check("lvfpfractiondate", "true");
    check("lvfpnegativefractiondate", "true");
    check("lvfpnegativewrapdate", "true");
    check("lvfpnegativehugedate", "true");
    check("lvfpnegativeinfinitedate", "true");
    check("lvfpcurrencyfractiondatetime", "true");
    check("lvfpcurrencynegativewrapdate", "true");
    check("nvfpaboveoneerror", "11");
    check("nvfphugeerror", "11");
    check("nvfpinfiniteerror", "11");
    check("nvfpcurrencysubuniterror", "11");
    check("nvfpcurrencypositivewraperror", "11");
    check("nvfpcharactererror", "11");
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

void test_fopen_mode_boundaries()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_fopen_mode_boundaries";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto seed = [&](const std::string& name) {
        write_text(temp_root / name, "seed");
    };
    seed("strict.txt");
    seed("vfp-fraction-read.txt");
    seed("vfp-fraction-write.txt");
    seed("vfp-fraction-read-write.txt");
    seed("vfp-wrap-read.txt");
    seed("vfp-wrap-write.txt");
    seed("vfp-negative-huge.txt");
    seed("vfp-negative-infinite.txt");
    seed("vfp-currency-write.txt");
    seed("vfp-currency-wrap-read.txt");

    const fs::path main_path = temp_root / "fopen_mode_boundaries.prg";
    write_text(
        main_path,
        "SET NUMERICBEHAVIOR TO COPPERFIN\n"
        "TRY\n"
        "  hUnexpectedStrictFraction = FOPEN('strict.txt', 0.9)\n"
        "  nStrictFractionError = 0\n"
        "CATCH TO oError\n"
        "  nStrictFractionError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  hUnexpectedStrictMissingFraction = FOPEN('missing.txt', 0.9)\n"
        "  nStrictMissingFractionError = 0\n"
        "CATCH TO oError\n"
        "  nStrictMissingFractionError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  hUnexpectedStrictHuge = FOPEN('strict.txt', 1E300)\n"
        "  nStrictHugeError = 0\n"
        "CATCH TO oError\n"
        "  nStrictHugeError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  hUnexpectedStrictInfinite = FOPEN('strict.txt', EXP(1000))\n"
        "  nStrictInfiniteError = 0\n"
        "CATCH TO oError\n"
        "  nStrictInfiniteError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  hUnexpectedStrictLogical = FOPEN('strict.txt', .T.)\n"
        "  nStrictLogicalError = 0\n"
        "CATCH TO oError\n"
        "  nStrictLogicalError = oError.ErrorNo\n"
        "ENDTRY\n"
        "SET NUMERICBEHAVIOR TO VFP9\n"
        "hVfpFractionRead = FOPEN('vfp-fraction-read.txt', 0.9)\n"
        "nVfpFractionReadWrite = FWRITE(hVfpFractionRead, 'x')\n"
        "=FCLOSE(hVfpFractionRead)\n"
        "hVfpFractionWrite = FOPEN('vfp-fraction-write.txt', 1.9)\n"
        "nVfpFractionWrite = FWRITE(hVfpFractionWrite, 'x')\n"
        "=FCLOSE(hVfpFractionWrite)\n"
        "hVfpFractionReadWrite = FOPEN('vfp-fraction-read-write.txt', 2.9)\n"
        "nVfpFractionReadWriteWrite = FWRITE(hVfpFractionReadWrite, 'x')\n"
        "=FCLOSE(hVfpFractionReadWrite)\n"
        "hVfpWrapRead = FOPEN('vfp-wrap-read.txt', -4294967296)\n"
        "nVfpWrapReadWrite = FWRITE(hVfpWrapRead, 'x')\n"
        "=FCLOSE(hVfpWrapRead)\n"
        "hVfpWrapWrite = FOPEN('vfp-wrap-write.txt', -4294967295)\n"
        "nVfpWrapWrite = FWRITE(hVfpWrapWrite, 'x')\n"
        "=FCLOSE(hVfpWrapWrite)\n"
        "hVfpNegativeHuge = FOPEN('vfp-negative-huge.txt', -1E300)\n"
        "nVfpNegativeHugeWrite = FWRITE(hVfpNegativeHuge, 'x')\n"
        "=FCLOSE(hVfpNegativeHuge)\n"
        "hVfpNegativeInfinite = FOPEN('vfp-negative-infinite.txt', -EXP(1000))\n"
        "nVfpNegativeInfiniteWrite = FWRITE(hVfpNegativeInfinite, 'x')\n"
        "=FCLOSE(hVfpNegativeInfinite)\n"
        "TRY\n"
        "  hUnexpectedVfpThirteen = FOPEN('strict.txt', 13)\n"
        "  nVfpThirteenError = 0\n"
        "CATCH TO oError\n"
        "  nVfpThirteenError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  hUnexpectedVfpHuge = FOPEN('strict.txt', 1E300)\n"
        "  nVfpHugeError = 0\n"
        "CATCH TO oError\n"
        "  nVfpHugeError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  hUnexpectedVfpInfinite = FOPEN('strict.txt', EXP(1000))\n"
        "  nVfpInfiniteError = 0\n"
        "CATCH TO oError\n"
        "  nVfpInfiniteError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  hUnexpectedVfpCurrencySubunit = FOPEN('strict.txt', $0.9000)\n"
        "  nVfpCurrencySubunitError = 0\n"
        "CATCH TO oError\n"
        "  nVfpCurrencySubunitError = oError.ErrorNo\n"
        "ENDTRY\n"
        "hVfpCurrencyWrite = FOPEN('vfp-currency-write.txt', $1.9000)\n"
        "nVfpCurrencyWrite = FWRITE(hVfpCurrencyWrite, 'x')\n"
        "=FCLOSE(hVfpCurrencyWrite)\n"
        "hVfpCurrencyWrapRead = FOPEN('vfp-currency-wrap-read.txt', $-4294967296.0000)\n"
        "nVfpCurrencyWrapReadWrite = FWRITE(hVfpCurrencyWrapRead, 'x')\n"
        "=FCLOSE(hVfpCurrencyWrapRead)\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "FOPEN mode-boundary script should complete: " + state.message);

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " variable should be present for FOPEN mode-boundary test");
        if (it != state.globals.end()) {
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(it->second) + "'");
        }
    };

    check("nstrictfractionerror", "11");
    check("nstrictmissingfractionerror", "11");
    check("nstricthugeerror", "11");
    check("nstrictinfiniteerror", "11");
    check("nstrictlogicalerror", "11");
    check("nvfpfractionreadwrite", "0");
    check("nvfpfractionwrite", "1");
    check("nvfpfractionreadwritewrite", "1");
    check("nvfpwrapreadwrite", "0");
    check("nvfpwrapwrite", "1");
    check("nvfpnegativehugewrite", "0");
    check("nvfpnegativeinfinitewrite", "0");
    check("nvfpthirteenerror", "11");
    check("nvfphugeerror", "11");
    check("nvfpinfiniteerror", "11");
    check("nvfpcurrencysubuniterror", "11");
    check("nvfpcurrencywrite", "1");
    check("nvfpcurrencywrapreadwrite", "0");

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

void test_fcreate_attribute_boundaries()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_fcreate_attribute_boundaries";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "fcreate_attribute_boundaries.prg";
    write_text(
        main_path,
        "SET NUMERICBEHAVIOR TO COPPERFIN\n"
        "TRY\n"
        "  hUnexpectedStrictFraction = FCREATE('strict-fraction.txt', 0.9)\n"
        "  nStrictFractionError = 0\n"
        "CATCH TO oError\n"
        "  nStrictFractionError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  hUnexpectedStrictMissingFraction = FCREATE('missing/strict.txt', 0.9)\n"
        "  nStrictMissingFractionError = 0\n"
        "CATCH TO oError\n"
        "  nStrictMissingFractionError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  hUnexpectedStrictEight = FCREATE('strict-eight.txt', 8)\n"
        "  nStrictEightError = 0\n"
        "CATCH TO oError\n"
        "  nStrictEightError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  hUnexpectedStrictInfinite = FCREATE('strict-infinite.txt', EXP(1000))\n"
        "  nStrictInfiniteError = 0\n"
        "CATCH TO oError\n"
        "  nStrictInfiniteError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  hUnexpectedStrictLogical = FCREATE('strict-logical.txt', .T.)\n"
        "  nStrictLogicalError = 0\n"
        "CATCH TO oError\n"
        "  nStrictLogicalError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  hUnexpectedStrictCharacter = FCREATE('strict-character.txt', '1')\n"
        "  nStrictCharacterError = 0\n"
        "CATCH TO oError\n"
        "  nStrictCharacterError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  hUnexpectedStrictEmpty = FCREATE('strict-empty.txt', uEmpty)\n"
        "  nStrictEmptyError = 0\n"
        "CATCH TO oError\n"
        "  nStrictEmptyError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  hUnexpectedStrictNull = FCREATE('strict-null.txt', .NULL.)\n"
        "  nStrictNullError = 0\n"
        "CATCH TO oError\n"
        "  nStrictNullError = oError.ErrorNo\n"
        "ENDTRY\n"
        "SET NUMERICBEHAVIOR TO VFP9\n"
        "hVfpFractionZero = FCREATE('vfp-fraction-zero.txt', 0.9)\n"
        "nVfpFractionZeroWrite = FWRITE(hVfpFractionZero, 'x')\n"
        "=FCLOSE(hVfpFractionZero)\n"
        "hVfpFractionOne = FCREATE('vfp-fraction-one.txt', 1.9)\n"
        "nVfpFractionOneWrite = FWRITE(hVfpFractionOne, 'x')\n"
        "=FCLOSE(hVfpFractionOne)\n"
        "hVfpNegativeFraction = FCREATE('vfp-negative-fraction.txt', -0.9)\n"
        "nVfpNegativeFractionWrite = FWRITE(hVfpNegativeFraction, 'x')\n"
        "=FCLOSE(hVfpNegativeFraction)\n"
        "hVfpWrapZero = FCREATE('vfp-wrap-zero.txt', -4294967296)\n"
        "nVfpWrapZeroWrite = FWRITE(hVfpWrapZero, 'x')\n"
        "=FCLOSE(hVfpWrapZero)\n"
        "hVfpWrapOne = FCREATE('vfp-wrap-one.txt', -4294967295)\n"
        "nVfpWrapOneWrite = FWRITE(hVfpWrapOne, 'x')\n"
        "=FCLOSE(hVfpWrapOne)\n"
        "hVfpNegativeHuge = FCREATE('vfp-negative-huge.txt', -1E300)\n"
        "nVfpNegativeHugeWrite = FWRITE(hVfpNegativeHuge, 'x')\n"
        "=FCLOSE(hVfpNegativeHuge)\n"
        "hVfpNegativeInfinite = FCREATE('vfp-negative-infinite.txt', -EXP(1000))\n"
        "nVfpNegativeInfiniteWrite = FWRITE(hVfpNegativeInfinite, 'x')\n"
        "=FCLOSE(hVfpNegativeInfinite)\n"
        "TRY\n"
        "  hUnexpectedVfpEight = FCREATE('vfp-eight.txt', 8)\n"
        "  nVfpEightError = 0\n"
        "CATCH TO oError\n"
        "  nVfpEightError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  hUnexpectedVfpPositiveWrap = FCREATE('vfp-positive-wrap.txt', 4294967296)\n"
        "  nVfpPositiveWrapError = 0\n"
        "CATCH TO oError\n"
        "  nVfpPositiveWrapError = oError.ErrorNo\n"
        "ENDTRY\n"
        "TRY\n"
        "  hUnexpectedVfpCurrencySubunit = FCREATE('vfp-currency-subunit.txt', $0.9000)\n"
        "  nVfpCurrencySubunitError = 0\n"
        "CATCH TO oError\n"
        "  nVfpCurrencySubunitError = oError.ErrorNo\n"
        "ENDTRY\n"
        "hVfpCurrencyOne = FCREATE('vfp-currency-one.txt', $1.9000)\n"
        "nVfpCurrencyOneWrite = FWRITE(hVfpCurrencyOne, 'x')\n"
        "=FCLOSE(hVfpCurrencyOne)\n"
        "hVfpCurrencyWrapZero = FCREATE('vfp-currency-wrap-zero.txt', $-4294967296.0000)\n"
        "nVfpCurrencyWrapZeroWrite = FWRITE(hVfpCurrencyWrapZero, 'x')\n"
        "=FCLOSE(hVfpCurrencyWrapZero)\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "FCREATE attribute-boundary script should complete: " + state.message);

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " variable should be present for FCREATE attribute-boundary test");
        if (it != state.globals.end()) {
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(it->second) + "'");
        }
    };

    check("nstrictfractionerror", "11");
    check("nstrictmissingfractionerror", "11");
    check("nstricteighterror", "11");
    check("nstrictinfiniteerror", "11");
    check("nstrictlogicalerror", "11");
    check("nstrictcharactererror", "11");
    check("nstrictemptyerror", "11");
    check("nstrictnullerror", "11");
    check("nvfpfractionzerowrite", "1");
    check("nvfpfractiononewrite", "-1");
    check("nvfpnegativefractionwrite", "1");
    check("nvfpwrapzerowrite", "1");
    check("nvfpwraponewrite", "-1");
    check("nvfpnegativehugewrite", "1");
    check("nvfpnegativeinfinitewrite", "1");
    check("nvfpeighterror", "11");
    check("nvfppositivewraperror", "11");
    check("nvfpcurrencysubuniterror", "11");
    check("nvfpcurrencyonewrite", "-1");
    check("nvfpcurrencywrapzerowrite", "1");

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

void test_fwrite_fputs_negative_count_writes_everything()
{
    // #6060: real VFP9 SP2 treats a negative nCharacters as a write-all
    // sentinel for FWRITE() and FPUTS(), not zero -- FWRITE(h, 'abc', -1)
    // writes all 3 bytes and returns 3, the same as omitting the count
    // entirely; FWRITE(h, 'abc', 0) writes nothing; and a positive
    // fractional count truncates toward zero (0.9 also writes nothing)
    // (confirmed against actual VFP9 output, retained differential
    // evidence: /home/rich/temp/vfp9-probes/fwrite-count-6060/ and
    // fputs-negative-6060/). FPUTS()'s CRLF-vs-LF terminator divergence
    // is a separate, already-tracked issue (#5887/#5911) and is not
    // covered by this test.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_fwrite_fputs_negative_count";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "negative_count.prg";
    write_text(
        main_path,
        "hOmit = FCREATE('omit.bin')\n"
        "nOmit = FWRITE(hOmit, 'abc')\n"
        "=FCLOSE(hOmit)\n"
        "cOmit = FILETOSTR('omit.bin')\n"
        "hZero = FCREATE('zero.bin')\n"
        "nZero = FWRITE(hZero, 'abc', 0)\n"
        "=FCLOSE(hZero)\n"
        "cZero = FILETOSTR('zero.bin')\n"
        "hNeg = FCREATE('neg.bin')\n"
        "nNeg = FWRITE(hNeg, 'abc', -1)\n"
        "=FCLOSE(hNeg)\n"
        "cNeg = FILETOSTR('neg.bin')\n"
        "hFrac = FCREATE('frac.bin')\n"
        "nFrac = FWRITE(hFrac, 'abc', 0.9)\n"
        "=FCLOSE(hFrac)\n"
        "cFrac = FILETOSTR('frac.bin')\n"
        "hPutsNeg = FCREATE('puts_neg.bin')\n"
        "nPutsNeg = FPUTS(hPutsNeg, 'abc', -1)\n"
        "=FCLOSE(hPutsNeg)\n"
        "cPutsNeg = FILETOSTR('puts_neg.bin')\n"
        // Review round (copilot-pull-request-reviewer): a finite count
        // vastly exceeding SIZE_MAX (e.g. 1e308) must not reach an
        // undefined-behavior narrowing cast -- it should behave the same
        // as any other oversized count and write the complete expression.
        "hHuge = FCREATE('huge.bin')\n"
        "nHuge = FWRITE(hHuge, 'abc', 1e308)\n"
        "=FCLOSE(hHuge)\n"
        "cHuge = FILETOSTR('huge.bin')\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "FWRITE/FPUTS negative-count script should complete: " + state.message);
    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " variable should be present for negative-count test");
        if (it != state.globals.end()) {
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " should equal '" + expected + "' for negative-count test");
        }
    };
    check("nomit", "3");
    check("comit", "abc");
    check("nzero", "0");
    check("czero", "");
    check("nneg", "3");
    check("cneg", "abc");
    check("nfrac", "0");
    check("cfrac", "");
    check("nputsneg", "4");
    check("cputsneg", "abc\n");
    check("nhuge", "3");
    check("chuge", "abc");

    fs::remove_all(temp_root, ignored);
}

void test_filetostr_reports_missing_and_non_file_inputs()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_filetostr_errors";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "input-directory");
    write_text(temp_root / "empty.bin", "");

    const fs::path main_path = temp_root / "filetostr_errors.prg";
    write_text(
        main_path,
        "cEmpty = FILETOSTR('empty.bin')\n"
        "nEmptyFError = FERROR()\n"
        "lMissingCaught = .F.\n"
        "lMissingCallReturned = .F.\n"
        "TRY\n"
        "  cMissing = FILETOSTR('missing.bin')\n"
        "  lMissingCallReturned = .T.\n"
        "CATCH TO oMissing\n"
        "  lMissingCaught = .T.\n"
        "  nMissingError = oMissing.ErrorNo\n"
        "  cMissingMessage = oMissing.Message\n"
        "  nMissingFError = FERROR()\n"
        "ENDTRY\n"
        "lAfterMissing = .T.\n"
        "lDirectoryCaught = .F.\n"
        "TRY\n"
        "  cDirectory = FILETOSTR('input-directory')\n"
        "CATCH TO oDirectory\n"
        "  lDirectoryCaught = .T.\n"
        "  nDirectoryError = oDirectory.ErrorNo\n"
        "  cDirectoryMessage = oDirectory.Message\n"
        "ENDTRY\n"
        "lAfterDirectory = .T.\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "FILETOSTR error script should complete through TRY/CATCH: " + state.message);
    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " variable should be present for FILETOSTR error test");
        if (it != state.globals.end()) {
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(it->second) + "'");
        }
    };

    check("cempty", "");
    check("nemptyferror", "0");
    check("lmissingcaught", "true");
    check("lmissingcallreturned", "false");
    check("nmissingerror", "1");
    check("cmissingmessage", "File does not exist.");
    check("nmissingferror", "0");
    check("laftermissing", "true");
    check("ldirectorycaught", "true");
    check("ndirectoryerror", "1705");
    check("cdirectorymessage", "File access is denied");
    check("lafterdirectory", "true");

    fs::remove_all(temp_root, ignored);
}

} // namespace

int main()
{
    test_file_io_runtime_functions();
    test_fseek_numeric_boundaries();
    test_file_handle_numeric_boundaries();
    test_fdate_ftime_runtime_functions();
    test_fopen_mode_boundaries();
    test_fcreate_runtime_function();
    test_fcreate_attribute_boundaries();
    test_unicode_paths_survive_prg_file_io_and_includes();
    test_fwrite_fputs_negative_count_writes_everything();
    test_filetostr_reports_missing_and_non_file_inputs();

    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
