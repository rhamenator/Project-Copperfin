// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>

namespace
{
using namespace copperfin::test_support;

void test_local_set_relation_tracks_parent_navigation()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_relations";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path parent_path = temp_root / "parent.dbf";
    const fs::path child_path = temp_root / "child.dbf";
    write_people_dbf(parent_path, {{"PARENT10", 10}, {"PARENT20", 20}, {"PARENT30", 30}});
    write_people_dbf(child_path, {{"CHILD30", 30}, {"CHILD10", 10}, {"CHILD20", 20}});

    const fs::path main_path = temp_root / "relations.prg";
    write_text(
        main_path,
        "USE '" + parent_path.string() + "' ALIAS Parent IN 0\n"
        "USE '" + child_path.string() + "' ALIAS Child IN 0\n"
        "SET ORDER TO AGE IN Parent\n"
        "SET ORDER TO AGE IN Child\n"
        "SELECT Parent\n"
        "SET RELATION TO AGE INTO Child\n"
        "GO TOP IN Parent\n"
        "nChildAfterTop = RECNO('Child')\n"
        "SKIP 1 IN Parent\n"
        "nChildAfterSkip = RECNO('Child')\n"
        "SEEK 30 IN Parent\n"
        "nChildAfterSeek = RECNO('Child')\n"
        "SET RELATION OFF INTO Child\n"
        "SKIP -1 IN Parent\n"
        "nChildAfterOff = RECNO('Child')\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

    expect(state.completed, "local SET RELATION script should complete");
    const auto child_after_top = state.globals.find("nchildaftertop");
    const auto child_after_skip = state.globals.find("nchildafterskip");
    const auto child_after_seek = state.globals.find("nchildafterseek");
    const auto child_after_off = state.globals.find("nchildafteroff");
    expect(child_after_top != state.globals.end(), "SET RELATION should synchronize after GO TOP");
    expect(child_after_skip != state.globals.end(), "SET RELATION should synchronize after SKIP");
    expect(child_after_seek != state.globals.end(), "SET RELATION should synchronize after SEEK");
    expect(child_after_off != state.globals.end(), "SET RELATION OFF should preserve a readable child pointer");

    if (child_after_top != state.globals.end())
    {
        expect(format_value(child_after_top->second) == "2", "parent AGE 10 should select child record 2");
    }
    if (child_after_skip != state.globals.end())
    {
        expect(format_value(child_after_skip->second) == "3", "parent AGE 20 should select child record 3");
    }
    if (child_after_seek != state.globals.end())
    {
        expect(format_value(child_after_seek->second) == "1", "parent AGE 30 should select child record 1");
    }
    if (child_after_off != state.globals.end())
    {
        expect(format_value(child_after_off->second) == "1", "SET RELATION OFF should stop changing the child pointer");
    }

    expect(has_runtime_event(state.events, "runtime.relation", "AGE -> Child"),
           "SET RELATION should emit a stable relation event");
    expect(has_runtime_event(state.events, "runtime.relation", "OFF -> Child"),
           "SET RELATION OFF should emit a stable relation event");
    fs::remove_all(temp_root, ignored);
}
}

int main()
{
    test_local_set_relation_tracks_parent_navigation();
    if (copperfin::test_support::test_failures() != 0)
    {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
