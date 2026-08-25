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

void test_on_page_assignment_query_eject_dispatch_and_clear()
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
        "PUBLIC cTrace, cAssigned, cAfterEject, cCleared\n"
        "cTrace = ''\n"
        "ON PAGE AT LINE (20 + 20) DO pagehead\n"
        "cAssigned = ON('PAGE')\n"
        "EJECT PAGE\n"
        "cAfterEject = cTrace\n"
        "ON PAGE\n"
        "EJECT PAGE\n"
        "cCleared = ON('PAGE')\n"
        "RETURN\n"
        "PROCEDURE pagehead\n"
        "cTrace = cTrace + 'P'\n"
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
    expect_global("cassigned", "DO pagehead");
    expect_global("caftereject", "P");
    expect_global("ccleared", "");
    expect_global("ctrace", "P");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
    {
        return event.category == "runtime.on_page" && event.detail == "action=assigned;line=(20 + 20)";
    }), "ON PAGE should preserve its configured line expression before EJECT PAGE dispatch");
    const auto dispatched_count = static_cast<std::size_t>(std::count_if(state.events.begin(), state.events.end(), [](const auto &event)
    {
        return event.category == "runtime.on_page" && event.detail == "handler=dispatched";
    }));
    expect(dispatched_count == 1U, "EJECT PAGE should dispatch one retained ON PAGE command before clear");
    const auto eject_count = static_cast<std::size_t>(std::count_if(state.events.begin(), state.events.end(), [](const auto &event)
    {
        return event.category == "runtime.eject_page";
    }));
    expect(eject_count == 2U, "both EJECT PAGE commands should retain deterministic runtime evidence");

    fs::remove_all(temp_root, ignored);
}

void test_eject_page_refuses_macro_backed_handlers()
{
    namespace fs = std::filesystem;
    using copperfin::runtime::DebugResumeAction;
    using copperfin::runtime::PrgRuntimeSession;
    using copperfin::runtime::format_value;
    using copperfin::test_support::expect;
    using copperfin::test_support::make_runtime_session_options;
    using copperfin::test_support::read_text;
    using copperfin::test_support::write_text;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_on_page_refusal";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path macro_path = temp_root / "on_page_macro.prg";
    write_text(macro_path,
        "PUBLIC cTrace, cAction\n"
        "cTrace = ''\n"
        "cAction = 'cTrace = cTrace + ''M'''\n"
        "ON PAGE AT LINE 1 &cAction\n"
        "EJECT PAGE\n"
        "RETURN\n");
    auto macro_options = make_runtime_session_options(macro_path, temp_root);
    macro_options.startup_source_text = read_text(macro_path);
    PrgRuntimeSession macro_session = PrgRuntimeSession::create(macro_options);
    const auto macro_state = macro_session.run(DebugResumeAction::continue_run);
    expect(macro_state.completed, "macro-backed ON PAGE fixture should complete safely: " + macro_state.message);
    const auto macro_trace = macro_state.globals.find("ctrace");
    expect(macro_trace != macro_state.globals.end() && format_value(macro_trace->second).empty(),
           "macro-backed ON PAGE action must not execute through EJECT PAGE");
    expect(std::any_of(macro_state.events.begin(), macro_state.events.end(), [](const auto &event)
    {
        return event.category == "runtime.eject_page" && event.detail == "handler=not_dispatched";
    }), "macro-backed ON PAGE action should leave deterministic non-dispatch evidence");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main()
{
    test_on_page_assignment_query_eject_dispatch_and_clear();
    test_eject_page_refuses_macro_backed_handlers();
    if (copperfin::test_support::test_failures() != 0)
    {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
