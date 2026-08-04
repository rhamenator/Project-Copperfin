// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

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

void test_local_set_skip_tracks_child_group_navigation()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_skip";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path parent_path = temp_root / "parent.dbf";
    const fs::path child_path = temp_root / "child.dbf";
    write_people_dbf(parent_path, {{"PARENT10", 10}, {"PARENT20", 20}, {"PARENT30", 30}});
    write_people_dbf(child_path, {
        {"CHILD10A", 10}, {"CHILD10B", 10}, {"CHILD20", 20}, {"CHILD30A", 30}, {"CHILD30B", 30}});

    const fs::path main_path = temp_root / "set_skip.prg";
    write_text(
        main_path,
        "USE '" + parent_path.string() + "' ALIAS Parent IN 0\n"
        "USE '" + child_path.string() + "' ALIAS Child IN 0\n"
        "SET ORDER TO AGE IN Parent\n"
        "SET ORDER TO AGE IN Child\n"
        "SELECT Parent\n"
        "SET RELATION TO AGE INTO Child\n"
        "SET SKIP TO Child\n"
        "GO TOP IN Parent\n"
        "nInitialParent = RECNO('Parent')\n"
        "nInitialChild = RECNO('Child')\n"
        "SKIP 1 IN Child\n"
        "nWithinGroupParent = RECNO('Parent')\n"
        "nWithinGroupChild = RECNO('Child')\n"
        "SKIP 1 IN Child\n"
        "nForwardParent = RECNO('Parent')\n"
        "nForwardChild = RECNO('Child')\n"
        "SKIP 1 IN Child\n"
        "nSecondForwardParent = RECNO('Parent')\n"
        "nSecondForwardChild = RECNO('Child')\n"
        "SKIP -1 IN Child\n"
        "nBackwardParent = RECNO('Parent')\n"
        "nBackwardChild = RECNO('Child')\n"
        "SET SKIP TO\n"
        "SKIP 1 IN Child\n"
        "nAfterClearParent = RECNO('Parent')\n"
        "nAfterClearChild = RECNO('Child')\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

    expect(state.completed, "SET SKIP script should complete");
    const auto check = [&](const std::string &name, const std::string &expected, const std::string &message)
    {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), message + " should be captured");
        if (found != state.globals.end())
        {
            expect(format_value(found->second) == expected, message);
        }
    };
    check("ninitialparent", "1", "SET SKIP initial parent");
    check("ninitialchild", "1", "SET SKIP initial child");
    check("nwithingroupparent", "1", "SET SKIP should stay within a child group");
    check("nwithingroupchild", "2", "SET SKIP should advance within a child group");
    check("nforwardparent", "2", "forward child SKIP should advance the parent");
    check("nforwardchild", "3", "forward child SKIP should select the next child group");
    check("nsecondforwardparent", "3", "second forward child SKIP should advance the parent");
    check("nsecondforwardchild", "4", "second forward child SKIP should select the next group");
    check("nbackwardparent", "2", "reverse child SKIP should move the parent backward");
    check("nbackwardchild", "3", "reverse child SKIP should select the prior child group");
    check("nafterclearparent", "2", "SET SKIP TO should preserve the parent pointer when cleared");
    check("nafterclearchild", "4", "SET SKIP TO should allow ordinary child navigation after clearing");
    expect(has_runtime_event(state.events, "runtime.set_skip", "Parent -> Child"),
           "SET SKIP should emit an enabled relation event");
    expect(has_runtime_event(state.events, "runtime.set_skip", "OFF -> Parent"),
           "SET SKIP TO with no aliases should emit a clear event");
    fs::remove_all(temp_root, ignored);
}

void test_local_set_relation_refreshes_after_parent_mutation()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_relation_mutation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path parent_path = temp_root / "parent.dbf";
    const fs::path child_path = temp_root / "child.dbf";
    write_people_dbf(parent_path, {{"PARENT10", 10}, {"PARENT20", 20}});
    write_people_dbf(child_path, {{"CHILD10", 10}, {"CHILD20", 20}});

    const fs::path main_path = temp_root / "relation_mutation.prg";
    write_text(
        main_path,
        "USE '" + parent_path.string() + "' ALIAS Parent IN 0\n"
        "USE '" + child_path.string() + "' ALIAS Child IN 0\n"
        "SET ORDER TO AGE IN Parent\n"
        "SET ORDER TO AGE IN Child\n"
        "SELECT Parent\n"
        "SET RELATION TO AGE INTO Child\n"
        "GO TOP IN Parent\n"
        "nBeforeMutation = RECNO('Child')\n"
        "REPLACE AGE WITH 20 IN Parent\n"
        "nAfterMutation = RECNO('Child')\n"
        "REPLACE AGE WITH 99 IN Parent\n"
        "nAfterMissing = RECNO('Child')\n"
        "REPLACE AGE WITH 10 IN Parent\n"
        "nAfterRestore = RECNO('Child')\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

    expect(state.completed, "SET RELATION mutation script should complete");
    const auto check = [&](const std::string &name, const std::string &expected, const std::string &message)
    {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), message + " should be captured");
        if (found != state.globals.end())
        {
            expect(format_value(found->second) == expected, message);
        }
    };
    check("nbeforemutation", "1", "relation mutation should preserve the initial child match");
    check("naftermutation", "2", "parent key mutation should refresh the child relation match");
    check("naftermissing", "3", "a missing mutated parent key should move the child to EOF");
    check("nafterrestore", "1", "restoring a parent key should refresh the child relation again");
    fs::remove_all(temp_root, ignored);
}

void test_local_set_relation_additive_and_explicit_parent()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_relation_additive";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path parent_path = temp_root / "parent.dbf";
    const fs::path child_path = temp_root / "child.dbf";
    const fs::path second_child_path = temp_root / "second_child.dbf";
    write_people_dbf(parent_path, {{"PARENT10", 10}, {"PARENT20", 20}, {"PARENT30", 30}});
    write_people_dbf(child_path, {{"CHILD10", 10}, {"CHILD20", 20}, {"CHILD30", 30}});
    write_people_dbf(second_child_path, {{"SECOND10", 10}, {"SECOND20", 20}, {"SECOND30", 30}});

    const fs::path main_path = temp_root / "relation_additive.prg";
    write_text(
        main_path,
        "USE '" + parent_path.string() + "' ALIAS Parent IN 0\n"
        "USE '" + child_path.string() + "' ALIAS Child IN 0\n"
        "USE '" + second_child_path.string() + "' ALIAS SecondChild IN 0\n"
        "SET ORDER TO AGE IN Parent\n"
        "SET ORDER TO AGE IN Child\n"
        "SET ORDER TO AGE IN SecondChild\n"
        "SELECT Parent\n"
        "SET RELATION TO AGE INTO Child\n"
        "SET RELATION TO AGE INTO SecondChild, AGE INTO Child ADDITIVE\n"
        "GO TOP IN Parent\n"
        "nBothAfterAdditive = RECNO('SecondChild')\n"
        "SELECT Child\n"
        "SET RELATION TO AGE INTO Child IN Parent\n"
        "GO TOP IN Parent\n"
        "SKIP 1 IN Parent\n"
        "nChildAfterExplicitParent = RECNO('Child')\n"
        "nSecondChildAfterReplacement = RECNO('SecondChild')\n"
        "SELECT Parent\n"
        "SET RELATION TO\n"
        "SKIP 1 IN Parent\n"
        "nChildAfterClear = RECNO('Child')\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

    expect(state.completed, "SET RELATION ADDITIVE script should complete");
    const auto check = [&](const std::string &name, const std::string &expected, const std::string &message)
    {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), message + " should be captured");
        if (found != state.globals.end())
        {
            expect(format_value(found->second) == expected, message);
        }
    };
    check("nbothafteradditive", "1", "ADDITIVE should retain and synchronize the second child relation");
    check("nchildafterexplicitparent", "2", "explicit parent target should replace and synchronize its child relation");
    check("nsecondchildafterreplacement", "1", "non-ADDITIVE replacement should remove the prior second-child relation");
    check("nchildafterclear", "2", "SET RELATION TO should clear all relations for the selected parent");
    expect(has_runtime_event(state.events, "runtime.relation", "AGE -> SecondChild"),
           "ADDITIVE relation should emit a stable second-child event");
    expect(has_runtime_event(state.events, "runtime.relation", "OFF -> Parent"),
           "SET RELATION TO should emit a stable clear event");
    fs::remove_all(temp_root, ignored);
}

void test_local_relation_introspection_preserves_order_and_session_state()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_relation_introspection";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path parent_path = temp_root / "parent.dbf";
    const fs::path child_path = temp_root / "child.dbf";
    const fs::path second_child_path = temp_root / "second_child.dbf";
    write_people_dbf(parent_path, {{"PARENT10", 10}, {"PARENT20", 20}});
    write_people_dbf(child_path, {{"CHILD10", 10}, {"CHILD20", 20}});
    write_people_dbf(second_child_path, {{"SECOND10", 10}, {"SECOND20", 20}});

    const fs::path main_path = temp_root / "relation_introspection.prg";
    write_text(
        main_path,
        "USE '" + parent_path.string() + "' ALIAS Parent IN 0\n"
        "USE '" + child_path.string() + "' ALIAS Child IN 0\n"
        "USE '" + second_child_path.string() + "' ALIAS SecondChild IN 0\n"
        "SET ORDER TO AGE IN Parent\n"
        "SET ORDER TO AGE IN Child\n"
        "SET ORDER TO AGE IN SecondChild\n"
        "SELECT Parent\n"
        "SET RELATION TO AGE INTO Child\n"
        "SET RELATION TO AGE INTO SecondChild ADDITIVE\n"
        "cRelationOne = RELATION(1)\n"
        "cRelationTwo = RELATION(2, 'Parent')\n"
        "cRelationMissing = RELATION(3, 'Parent')\n"
        "cTargetOne = TARGET(1)\n"
        "cTargetTwo = TARGET(2, 'Parent')\n"
        "cTargetMissing = TARGET(3, 'Parent')\n"
        "cRelationClause = SET('RELATION')\n"
        "SET SKIP TO Child\n"
        "cSkipClause = SET('SKIP')\n"
        "SET DATASESSION TO 2\n"
        "cFreshRelation = SET('RELATION')\n"
        "cFreshTarget = TARGET(1)\n"
        "SET DATASESSION 1\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

    expect(state.completed, "relation introspection script should complete");
    const auto check = [&](const std::string &name, const std::string &expected, const std::string &message)
    {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), message + " should be captured");
        if (found != state.globals.end())
        {
            expect(format_value(found->second) == expected, message);
        }
    };
    check("crelationone", "AGE", "RELATION should preserve the first expression");
    check("crelationtwo", "AGE", "RELATION should resolve an alias-selected parent");
    check("crelationmissing", "", "RELATION should return empty for an out-of-range relation");
    check("ctargetone", "Child", "TARGET should return the first child alias");
    check("ctargettwo", "SecondChild", "TARGET should resolve an alias-selected parent");
    check("ctargetmissing", "", "TARGET should return empty for an out-of-range relation");
    check("crelationclause", "AGE INTO Child, AGE INTO SecondChild", "SET RELATION should emit a restorable clause");
    check("cskipclause", "Child", "SET SKIP should emit only enabled child aliases");
    check("cfreshrelation", "", "relation introspection should be data-session scoped");
    check("cfreshtarget", "", "TARGET should be empty in a fresh data session");
    fs::remove_all(temp_root, ignored);
}
}

int main()
{
    test_local_set_relation_tracks_parent_navigation();
    test_local_set_skip_tracks_child_group_navigation();
    test_local_set_relation_refreshes_after_parent_mutation();
    test_local_set_relation_additive_and_explicit_parent();
    test_local_relation_introspection_preserves_order_and_session_state();
    if (copperfin::test_support::test_failures() != 0)
    {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
