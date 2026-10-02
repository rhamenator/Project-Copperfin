// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

namespace {

using namespace copperfin::test_support;

namespace fs = std::filesystem;

// Governing requirement: RQ-CF-PRG-STRING-LENGTH-BOUNDS-001 (#5595, #6003, #6004, part of #5611).
//
// A Numeric length or width argument is validated -- finite, truncated toward zero, no larger than VFP9's maximum
// Character string (16,777,184 bytes) -- before it is converted to std::size_t or used to allocate. SPACE() and
// REPLICATE() raise error 1903 past the limit; PADL(), PADR(), PADC(), FREAD() and FGETS() raise error 11, as installed
// VFP9 SP2 does for a 20,000,000 width or length (issue evidence). REPLICATE() of an empty source returns at once
// instead of looping `count` times. The expected text is "<VARTYPE>:<value>" or "ERR<number>".

struct Row {
    const char *expression;
    const char *expected;
};

const std::vector<Row> kRows = {
    // SPACE: the existing limit, now through the shared validator.
    {"LEN(SPACE(16777184))", "N:16777184"},
    {"LEN(SPACE(16777184.9))", "N:16777184"},
    {"LEN(SPACE(16777185))", "ERR1903"},
    {"LEN(SPACE(-3))", "N:0"},
    // REPLICATE: an empty source or a zero/negative count returns immediately (#5595).
    {"LEN(REPLICATE('', 9007199254740992))", "N:0"},
    {"LEN(REPLICATE('', 1E300))", "N:0"},
    {"LEN(REPLICATE('x', 0))", "N:0"},
    {"LEN(REPLICATE('x', -2))", "N:0"},
    {"LEN(REPLICATE('ab', 3))", "N:6"},
    {"LEN(REPLICATE('x', 16777184))", "N:16777184"},
    {"LEN(REPLICATE('x', 16777185))", "ERR1903"},
    {"LEN(REPLICATE('ab', 8388592))", "N:16777184"},
    {"LEN(REPLICATE('ab', 8388593))", "ERR1903"},
    // PADL / PADR / PADC (#6003): the largest accepted width, one past it, the issue's 20 MB case, a huge value.
    {"LEN(PADL('x', 16777184))", "N:16777184"},
    {"LEN(PADR('x', 16777184))", "N:16777184"},
    {"LEN(PADC('x', 16777184))", "N:16777184"},
    {"LEN(PADL('x', 16777185))", "ERR11"},
    {"LEN(PADR('x', 16777185))", "ERR11"},
    {"LEN(PADC('x', 16777185))", "ERR11"},
    {"LEN(PADL('x', 20000000))", "ERR11"},
    {"LEN(PADR('x', 20000000))", "ERR11"},
    {"LEN(PADC('x', 20000000))", "ERR11"},
    {"LEN(PADR('x', 1E300))", "ERR11"},
    {"PADL('x', 3.9)", "C:  x"},
    {"PADR('x', 3.9)", "C:x  "},
    {"PADC('x', 4)", "C: x  "},
    {"PADR('x', 0)", "C:"},
    {"PADR('x', -5)", "C:"},
    {"PADL('abcdef', 3)", "C:abc"},
    // FREAD / FGETS (#6004) on a 15-byte file "abcdefghij\r\nxyz". A rejected length reads nothing.
    {"LEN(FREAD(h, 20000000))", "ERR11"},
    {"LEN(FREAD(h, 16777185))", "ERR11"},
    {"LEN(FREAD(h, 1E300))", "ERR11"},
    {"LEN(FGETS(h, 20000000))", "ERR11"},
    {"LEN(FGETS(h, 1E300))", "ERR11"},
    {"LEN(FREAD(h, 0))", "N:0"},
    {"LEN(FREAD(h, -5))", "N:0"},
    {"FREAD(h, 3.7)", "C:abc"},
    {"FGETS(h)", "C:defghij"},
    {"LEN(FREAD(h, 16777184))", "N:3"},
};

std::string run_rows(const fs::path &dir) {
    std::string body =
        "LOCAL cOut, oEx, x, h\ncOut = ''\n"
        "h = FCREATE('data.bin')\nFWRITE(h, 'abcdefghij' + CHR(13) + CHR(10) + 'xyz')\nFCLOSE(h)\nh = FOPEN('data.bin')\n";
    for (const Row &row : kRows) {
        body += "TRY\n";
        body += "x = " + std::string(row.expression) + "\n";
        body += "cOut = cOut + VARTYPE(x) + ':' + IIF(VARTYPE(x) = 'N', ALLTRIM(STR(x, 20, 0)), TRANSFORM(x)) + CHR(10)\n";
        body += "CATCH TO oEx\n";
        body += "cOut = cOut + 'ERR' + ALLTRIM(STR(oEx.ErrorNo)) + CHR(10)\n";
        body += "ENDTRY\n";
    }
    body += "FCLOSE(h)\nSTRTOFILE(cOut, 'results.txt')\nRETURN\n";
    fs::create_directories(dir / "script");
    write_text(dir / "script" / "rows.prg", body);
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((dir / "script" / "rows.prg").string(), dir.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    if (!state.completed) {
        return "<incomplete: " + state.message + ">";
    }
    return read_text(dir / "results.txt");
}

void test_string_length_arguments_are_bounded() {
    const fs::path dir = fs::temp_directory_path() / "copperfin_string_length_bounds";
    std::error_code ignored;
    fs::remove_all(dir, ignored);
    const std::string output = run_rows(dir);
    expect(output.rfind("<incomplete", 0U) != 0U, "string length bounds: the script should complete: " + output);
    std::vector<std::string> lines;
    for (std::size_t start = 0U; start < output.size();) {
        const std::size_t end = output.find('\n', start);
        if (end == std::string::npos) {
            break;
        }
        lines.push_back(output.substr(start, end - start));
        start = end + 1U;
    }
    expect(lines.size() == kRows.size(),
           "string length bounds: expected " + std::to_string(kRows.size()) + " result lines, got " +
               std::to_string(lines.size()) + " from [" + output + "]");
    for (std::size_t index = 0U; index < kRows.size() && index < lines.size(); ++index) {
        expect(lines[index] == kRows[index].expected,
               std::string("string length bounds: ") + kRows[index].expression + " expected [" + kRows[index].expected +
                   "], got [" + lines[index] + "]");
    }
    fs::remove_all(dir, ignored);
}

}  // namespace

int main() {
    test_string_length_arguments_are_bounded();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
