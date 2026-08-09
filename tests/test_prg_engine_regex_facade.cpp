// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <iostream>
#include <system_error>

namespace {

using namespace copperfin::test_support;

void test_prg_regex_control_plane_facade() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_prg_regex_control_plane";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "regex_facade.prg";
    write_text(
        main_path,
        "cPayload = 'state=READY; rows=9007199254740993'\n"
        "lValid = CFREGEXVALID('rows=\\d+')\n"
        "lInvalid = CFREGEXVALID('(rows)')\n"
        "lReady = CFREGEXTEST(cPayload, 'state=ready', .T.)\n"
        "nRowsAt = CFREGEXFIND(cPayload, 'rows=\\d+')\n"
        "cRows = CFREGEXGET(cPayload, 'rows=\\d+')\n"
        "cDigits = CFREGEXGET(cPayload, '\\d+', nRowsAt)\n"
        "nMissing = CFREGEXFIND(cPayload, '^missing$')\n"
        "cMissing = CFREGEXGET(cPayload, 'missing', 1, .F., 'fallback')\n"
        "nTypedMissing = CFREGEXGET(cPayload, 'missing', 1, .F., 42)\n"
        "nBadStart = CFREGEXFIND(cPayload, 'state', 0)\n"
        "nBadStringStart = CFREGEXFIND(cPayload, 'state', 'not-a-position')\n"
        "nUnsupported = CFREGEXFIND(cPayload, 'READY|ERROR')\n"
        "nEmptyAtEnd = CFREGEXFIND(cPayload, '$', LEN(cPayload) + 1)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("PRG regex facade should complete: ") + state.message);

    const auto formatted = [&](const std::string& name) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        return found == state.globals.end()
            ? std::string{}
            : copperfin::runtime::format_value(found->second);
    };
    expect(formatted("lvalid") == "true", "CFREGEXVALID should accept supported syntax");
    expect(formatted("linvalid") == "false", "CFREGEXVALID should reject unsupported syntax");
    expect(formatted("lready") == "true", "CFREGEXTEST should support invariant ASCII folding");
    expect(formatted("nrowsat") == "14", "CFREGEXFIND should return a one-based byte position");
    expect(formatted("crows") == "rows=9007199254740993",
           "CFREGEXGET should preserve exact matched bytes");
    expect(formatted("cdigits") == "9007199254740993",
           "CFREGEXGET should preserve large numeric spelling as text");
    expect(formatted("nmissing") == "0", "CFREGEXFIND should return zero for no match");
    expect(formatted("cmissing") == "fallback", "CFREGEXGET should preserve the caller fallback");
    const auto typed_missing = state.globals.find("ntypedmissing");
    expect(typed_missing != state.globals.end() &&
               typed_missing->second.kind == copperfin::runtime::PrgValueKind::number &&
               typed_missing->second.number_value == 42.0,
           "CFREGEXGET should preserve the caller fallback type and value");
    expect(formatted("nbadstart") == "0", "CFREGEXFIND should reject a zero start position");
    expect(formatted("nbadstringstart") == "0",
           "CFREGEXFIND should fail closed for a nonnumeric start position");
    expect(formatted("nunsupported") == "0", "unsupported syntax should fail closed");
    expect(formatted("nemptyatend") == "35", "terminal empty match should use LEN plus one");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_prg_regex_control_plane_facade();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
