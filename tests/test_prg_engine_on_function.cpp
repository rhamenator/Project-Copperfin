// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "prg_engine_test_support.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <system_error>

namespace {

void test_on_function_reads_supported_event_assignments()
{
    namespace fs = std::filesystem;
    using copperfin::runtime::DebugResumeAction;
    using copperfin::runtime::PrgRuntimeSession;
    using copperfin::runtime::format_value;
    using copperfin::test_support::expect;
    using copperfin::test_support::make_runtime_session_options;
    using copperfin::test_support::write_text;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_on_function";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "on_function.prg";
    write_text(
        main_path,
        "cErrorDefault = ON('ERROR')\n"
        "cKeyDefault = ON('KEY', 'F5')\n"
        "cKeyMissingLabel = ON('KEY')\n"
        "cEscapeDefault = ON('ESCAPE')\n"
        "cPageDefault = ON('PAGE')\n"
        "ON ERROR DO error_handler WITH 7\n"
        "ON KEY LABEL f5 WAIT WINDOW 'F5' NOWAIT\n"
        "cErrorAssigned = ON('error')\n"
        "cKeyAssigned = ON('key', 'F5')\n"
        "SET DATASESSION TO 2\n"
        "cKeySessionTwoDefault = ON('KEY', 'F5')\n"
        "ON KEY LABEL F5 WAIT WINDOW 'session two' NOWAIT\n"
        "cKeySessionTwoAssigned = ON('KEY', 'f5')\n"
        "SET DATASESSION TO 1\n"
        "cKeySessionOneRestored = ON('KEY', 'F5')\n"
        "ON KEY LABEL F5\n"
        "cKeyCleared = ON('KEY', 'F5')\n"
        "cUnknownTopic = ON('not-an-on-topic')\n"
        "RETURN\n");

    const auto state = PrgRuntimeSession::create(
        make_runtime_session_options(main_path, temp_root)).run(DebugResumeAction::continue_run);
    expect(state.completed, "ON() introspection fixture should complete: " + state.message);

    const auto check = [&](const std::string &name, const std::string &expected)
    {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), "ON() fixture should define " + name);
        if (found != state.globals.end())
        {
            expect(format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" + format_value(found->second) + "'");
        }
    };

    check("cerrordefault", "");
    check("ckeydefault", "");
    check("ckeymissinglabel", "");
    check("cescapedefault", "");
    check("cpagedefault", "");
    check("cerrorassigned", "DO error_handler WITH 7");
    check("ckeyassigned", "WAIT WINDOW 'F5' NOWAIT");
    check("ckeysessiontwodefault", "");
    check("ckeysessiontwoassigned", "WAIT WINDOW 'session two' NOWAIT");
    check("ckeysessiononerestored", "WAIT WINDOW 'F5' NOWAIT");
    check("ckeycleared", "");
    check("cunknowntopic", "");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main()
{
    test_on_function_reads_supported_event_assignments();
    if (copperfin::test_support::test_failures() != 0)
    {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
