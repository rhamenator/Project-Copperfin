// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "prg_engine_test_support.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>

namespace {

void test_on_page_assignment_query_and_clear_without_report_dispatch()
{
    namespace fs = std::filesystem;
    using copperfin::runtime::DebugResumeAction;
    using copperfin::runtime::PrgRuntimeSession;
    using copperfin::runtime::format_value;
    using copperfin::test_support::expect;
    using copperfin::test_support::make_runtime_session_options;
    using copperfin::test_support::read_text;
    using copperfin::test_support::write_text;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_on_page";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path program_path = temp_root / "on_page.prg";
    write_text(program_path,
        "PUBLIC cTrace, cAssigned, cCleared\n"
        "cTrace = ''\n"
        "ON PAGE AT LINE 37 cTrace = cTrace + 'P'\n"
        "cAssigned = ON('PAGE')\n"
        "ON PAGE\n"
        "cCleared = ON('PAGE')\n"
        "RETURN\n");

    auto options = make_runtime_session_options(program_path, temp_root);
    options.startup_source_text = read_text(program_path);
    PrgRuntimeSession session = PrgRuntimeSession::create(options);
    const auto state = session.run(DebugResumeAction::continue_run);
    expect(state.completed, "ON PAGE configuration fixture should complete: " + state.message);

    const auto expect_global = [&](const std::string &name, const std::string &expected)
    {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end() && format_value(found->second) == expected,
               name + " should retain the expected ON PAGE state");
    };
    expect_global("cassigned", "cTrace = cTrace + 'P'");
    expect_global("ccleared", "");
    expect_global("ctrace", "");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
    {
        return event.category == "runtime.on_page" && event.detail == "action=assigned;line=37";
    }), "ON PAGE should preserve its configured line expression without dispatching its command");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main()
{
    test_on_page_assignment_query_and_clear_without_report_dispatch();
    if (copperfin::test_support::test_failures() != 0)
    {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
