// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

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

void test_set_relation_key_expression_closing_child_fails_catchably()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_relation_closes_child";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path parent_path = temp_root / "parent.dbf";
    const fs::path child_path = temp_root / "child.dbf";
    write_people_dbf(parent_path, {{"PARENT10", 10}});
    write_people_dbf(child_path, {{"CHILD10", 10}});

    const fs::path main_path = temp_root / "relation_closes_child.prg";
    write_text(
        main_path,
        "USE '" + parent_path.string() + "' ALIAS Parent IN 0\n"
        "USE '" + child_path.string() + "' ALIAS Child IN 0\n"
        "SET ORDER TO AGE IN Child\n"
        "SELECT Parent\n"
        "GO TOP IN Parent\n"
        "lErrorCaught = .F.\n"
        "TRY\n"
        "    SET RELATION TO dropchild() INTO Child\n"
        "CATCH TO oErr\n"
        "    lErrorCaught = .T.\n"
        "ENDTRY\n"
        "lStillOpen = USED('Child')\n"
        "after = 1\n"
        "RETURN\n"
        "FUNCTION dropchild\n"
        "    USE IN Child\n"
        "    RETURN 'Alice'\n"
        "ENDFUNC\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

    expect(state.completed,
           "#6247: SET RELATION whose key expression closes its own child should complete without crashing: " +
               state.message);

    const auto error_caught_it = state.globals.find("lerrorcaught");
    expect(error_caught_it != state.globals.end() && error_caught_it->second.boolean_value,
           "#6247: SET RELATION must raise a catchable error instead of continuing through the closed child");

    const auto still_open_it = state.globals.find("lstillopen");
    expect(still_open_it != state.globals.end() && !still_open_it->second.boolean_value,
           "#6247: the key expression's USE IN Child should genuinely have closed the child");

    const auto after_it = state.globals.find("after");
    expect(after_it != state.globals.end(), "#6247: script execution should continue after the caught error");
    fs::remove_all(temp_root, ignored);
}

void test_set_relation_registration_closing_earlier_child_fails_atomically()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_relation_registration_closes_earlier_child";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path parent_path = temp_root / "parent.dbf";
    const fs::path first_child_path = temp_root / "first_child.dbf";
    const fs::path second_child_path = temp_root / "second_child.dbf";
    write_people_dbf(parent_path, {{"PARENT10", 10}});
    write_people_dbf(first_child_path, {{"FIRST10", 10}});
    write_people_dbf(second_child_path, {{"SECOND10", 10}});

    const fs::path main_path = temp_root / "relation_registration_closes_earlier_child.prg";
    write_text(
        main_path,
        "USE '" + parent_path.string() + "' ALIAS Parent IN 0\n"
        "USE '" + first_child_path.string() + "' ALIAS FirstChild IN 0\n"
        "USE '" + second_child_path.string() + "' ALIAS SecondChild IN 0\n"
        "SET ORDER TO AGE IN FirstChild\n"
        "SET ORDER TO AGE IN SecondChild\n"
        "SELECT Parent\n"
        "lErrorCaught = .F.\n"
        "TRY\n"
        "    SET RELATION TO AGE INTO FirstChild, AGE INTO dropfirstchild()\n"
        "CATCH TO oErr\n"
        "    lErrorCaught = .T.\n"
        "ENDTRY\n"
        "lFirstStillOpen = USED('FirstChild')\n"
        "cRelationAfter = SET('RELATION')\n"
        "after = 1\n"
        "RETURN\n"
        "FUNCTION dropfirstchild\n"
        "    USE IN FirstChild\n"
        "    RETURN 'SecondChild'\n"
        "ENDFUNC\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

    expect(state.completed,
           "#6247: SET RELATION whose second designator closes an already-resolved earlier child should "
           "complete without crashing: " + state.message);

    const auto error_caught_it = state.globals.find("lerrorcaught");
    expect(error_caught_it != state.globals.end() && error_caught_it->second.boolean_value,
           "#6247: SET RELATION must raise a catchable error instead of registering through a freed earlier child");

    const auto first_still_open_it = state.globals.find("lfirststillopen");
    expect(first_still_open_it != state.globals.end() && !first_still_open_it->second.boolean_value,
           "#6247: the second designator's USE IN FirstChild should genuinely have closed FirstChild");

    const auto relation_after_it = state.globals.find("crelationafter");
    expect(relation_after_it != state.globals.end() && format_value(relation_after_it->second).empty(),
           "#6247: no relation should be registered when a later designator invalidates an earlier participant "
           "-- registration must be atomic, not partially applied");

    const auto after_it = state.globals.find("after");
    expect(after_it != state.globals.end(), "#6247: script execution should continue after the caught error");
    fs::remove_all(temp_root, ignored);
}

void test_set_relation_registration_sync_failure_rolls_back_sibling_relations()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_relation_sync_failure_rollback";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path parent_path = temp_root / "parent.dbf";
    const fs::path first_child_path = temp_root / "first_child.dbf";
    const fs::path second_child_path = temp_root / "second_child.dbf";
    write_people_dbf(parent_path, {{"PARENT10", 10}});
    write_people_dbf(first_child_path, {{"FIRST10", 10}});
    write_people_dbf(second_child_path, {{"SECOND10", 10}});

    const fs::path main_path = temp_root / "relation_sync_failure_rollback.prg";
    write_text(
        main_path,
        "USE '" + parent_path.string() + "' ALIAS Parent IN 0\n"
        "USE '" + first_child_path.string() + "' ALIAS FirstChild IN 0\n"
        "USE '" + second_child_path.string() + "' ALIAS SecondChild IN 0\n"
        "SET ORDER TO AGE IN SecondChild\n"
        "SELECT Parent\n"
        "GO TOP IN Parent\n"
        "lErrorCaught = .F.\n"
        "TRY\n"
        // Both designators resolve cleanly at parse time (no function-call
        // designators here) -- the closure only happens once the first
        // relation's own key expression is evaluated during the *final*
        // initial synchronization, after both relations are already
        // inserted into the session's relation list.
        "    SET RELATION TO dropfirstchild() INTO FirstChild, AGE INTO SecondChild\n"
        "CATCH TO oErr\n"
        "    lErrorCaught = .T.\n"
        "ENDTRY\n"
        "lFirstStillOpen = USED('FirstChild')\n"
        "cRelationAfter = SET('RELATION')\n"
        "after = 1\n"
        "RETURN\n"
        "FUNCTION dropfirstchild\n"
        "    USE IN FirstChild\n"
        "    RETURN 'Alice'\n"
        "ENDFUNC\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

    expect(state.completed,
           "#6247 review: SET RELATION whose first relation's key expression closes its own child during "
           "the final initial synchronization should complete without crashing: " + state.message);

    const auto error_caught_it = state.globals.find("lerrorcaught");
    expect(error_caught_it != state.globals.end() && error_caught_it->second.boolean_value,
           "#6247 review: SET RELATION must raise a catchable error when initial synchronization closes a "
           "relation's own child");

    const auto first_still_open_it = state.globals.find("lfirststillopen");
    expect(first_still_open_it != state.globals.end() && !first_still_open_it->second.boolean_value,
           "#6247 review: dropfirstchild()'s USE IN FirstChild should genuinely have closed FirstChild");

    const auto relation_after_it = state.globals.find("crelationafter");
    expect(relation_after_it != state.globals.end() && format_value(relation_after_it->second).empty(),
           "#6247 review: when initial synchronization fails after both relations were already inserted, "
           "the second (unrelated) relation must be rolled back too -- SET RELATION is all-or-nothing, not "
           "just atomic with respect to designator parsing");

    const auto after_it = state.globals.find("after");
    expect(after_it != state.globals.end(), "#6247 review: script execution should continue after the caught error");
    fs::remove_all(temp_root, ignored);
}

void test_set_skip_walked_parent_closed_by_sibling_relation_fails_catchably()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_relation_skip_closes_walked_parent";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path parent_path = temp_root / "parent.dbf";
    const fs::path child_path = temp_root / "child.dbf";
    const fs::path other_path = temp_root / "other.dbf";
    write_people_dbf(parent_path, {{"PARENT10", 10}, {"PARENT20", 20}});
    write_people_dbf(child_path, {{"CHILD10", 10}});
    write_people_dbf(other_path, {{"OTHER10", 10}});

    const fs::path main_path = temp_root / "relation_skip_closes_walked_parent.prg";
    write_text(
        main_path,
        "USE '" + parent_path.string() + "' ALIAS Parent IN 0\n"
        "USE '" + child_path.string() + "' ALIAS Child IN 0\n"
        "USE '" + other_path.string() + "' ALIAS Other IN 0\n"
        "SET ORDER TO AGE IN Child\n"
        "SET ORDER TO AGE IN Other\n"
        "SELECT Parent\n"
        "GO TOP IN Parent\n"
        "GO TOP IN Child\n"
        "GO TOP IN Other\n"
        "SET RELATION TO AGE INTO Child\n"
        "SET RELATION TO dropparent() INTO Other ADDITIVE\n"
        "SET SKIP TO Child\n"
        "lErrorCaught = .F.\n"
        "TRY\n"
        "    SKIP 1 IN Child\n"
        "CATCH TO oErr\n"
        "    lErrorCaught = .T.\n"
        "ENDTRY\n"
        "lParentStillOpen = USED('Parent')\n"
        "after = 1\n"
        "RETURN\n"
        // Only fires once Parent has been walked to record 2 by the
        // one-to-many SKIP logic below -- the sibling Other relation is
        // evaluated by the *nested* synchronize_relations_for_parent() call
        // inside synchronize_skip_parent_for_child(), which the walked
        // parent itself never otherwise revisits.
        "FUNCTION dropparent\n"
        "    IF RECNO('Parent') == 2\n"
        "        USE IN Parent\n"
        "    ENDIF\n"
        "    RETURN 'Alice'\n"
        "ENDFUNC\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

    expect(state.completed,
           "#6247 review: SKIP whose one-to-many parent walk triggers a sibling relation that closes the "
           "walked parent itself should complete without crashing: " + state.message);

    const auto error_caught_it = state.globals.find("lerrorcaught");
    expect(error_caught_it != state.globals.end() && error_caught_it->second.boolean_value,
           "#6247 review: SKIP must raise a catchable error instead of swallowing a closed walked parent as "
           "\"nothing to adjust\"");

    const auto still_open_it = state.globals.find("lparentstillopen");
    expect(still_open_it != state.globals.end() && !still_open_it->second.boolean_value,
           "#6247 review: the sibling relation's USE IN Parent should genuinely have closed Parent");

    const auto after_it = state.globals.find("after");
    expect(after_it != state.globals.end(), "#6247 review: script execution should continue after the caught error");
    fs::remove_all(temp_root, ignored);
}

void test_set_skip_relation_match_expression_closing_parent_fails_catchably()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_relation_skip_match_closes_parent";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path parent_path = temp_root / "parent.dbf";
    const fs::path child_path = temp_root / "child.dbf";
    write_people_dbf(parent_path, {{"PARENT10", 10}});
    write_people_dbf(child_path, {{"CHILD10", 10}, {"CHILD20", 20}});

    const fs::path main_path = temp_root / "relation_skip_match_closes_parent.prg";
    write_text(
        main_path,
        "USE '" + parent_path.string() + "' ALIAS Parent IN 0\n"
        "USE '" + child_path.string() + "' ALIAS Child IN 0\n"
        "SELECT Parent\n"
        "GO TOP IN Parent\n"
        "nCalls = 0\n"
        "SET RELATION TO matchkey() INTO Child\n"
        "SET SKIP TO Child\n"
        // Child has no active order, so the registration's own initial
        // synchronization above unconditionally forces it to EOF (the
        // "no order" fallback in synchronize_relations_for_parent()) --
        // reposition it explicitly so the SKIP below actually enters the
        // non-bof/eof matching path this test targets, instead of
        // short-circuiting on an already-EOF child.
        "GO 1 IN Child\n"
        "lErrorCaught = .F.\n"
        "TRY\n"
        "    SKIP 1 IN Child\n"
        "CATCH TO oErr\n"
        "    lErrorCaught = .T.\n"
        "ENDTRY\n"
        "lParentStillOpen = USED('Parent')\n"
        "after = 1\n"
        "RETURN\n"
        // Call #1: SET RELATION's own initial synchronization (Parent is
        // non-bof/eof, key expression evaluated once). Calls #2 and #3:
        // relation_matches_current_parent()'s own two evaluations (parent
        // side, then child side) at the very top of the one-to-many SKIP
        // walk -- both Parent and Child are non-bof/eof at that point,
        // since Child has two records and SKIP just moved onto the second
        // one. Closing Parent on call #3 and returning a mismatched value
        // forces the walk to proceed into its own parent-dereferencing
        // code with a cursor the *matcher itself* just freed.
        "FUNCTION matchkey\n"
        "    nCalls = nCalls + 1\n"
        "    IF nCalls >= 3\n"
        "        USE IN Parent\n"
        "        RETURN 'MISMATCH'\n"
        "    ENDIF\n"
        "    RETURN 'K'\n"
        "ENDFUNC\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

    expect(state.completed,
           "#6247 review: SKIP whose relation-key match expression closes the parent during its own "
           "matching evaluation should complete without crashing: " + state.message);

    const auto error_caught_it = state.globals.find("lerrorcaught");
    expect(error_caught_it != state.globals.end() && error_caught_it->second.boolean_value,
           "#6247 review: SKIP must raise a catchable error instead of continuing through a parent the "
           "match expression itself just closed");

    const auto still_open_it = state.globals.find("lparentstillopen");
    expect(still_open_it != state.globals.end() && !still_open_it->second.boolean_value,
           "#6247 review: matchkey()'s USE IN Parent should genuinely have closed Parent");

    const auto after_it = state.globals.find("after");
    expect(after_it != state.globals.end(), "#6247 review: script execution should continue after the caught error");
    fs::remove_all(temp_root, ignored);
}

void test_set_skip_sibling_relation_closing_original_child_fails_catchably()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_relation_skip_closes_original_child";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path parent_path = temp_root / "parent.dbf";
    const fs::path child_path = temp_root / "child.dbf";
    const fs::path other_path = temp_root / "other.dbf";
    write_people_dbf(parent_path, {{"PARENT10", 10}, {"PARENT20", 20}});
    write_people_dbf(child_path, {{"CHILD10", 10}});
    write_people_dbf(other_path, {{"OTHER10", 10}});

    const fs::path main_path = temp_root / "relation_skip_closes_original_child.prg";
    write_text(
        main_path,
        "USE '" + parent_path.string() + "' ALIAS Parent IN 0\n"
        "USE '" + child_path.string() + "' ALIAS Child IN 0\n"
        "USE '" + other_path.string() + "' ALIAS Other IN 0\n"
        "SET ORDER TO AGE IN Child\n"
        "SET ORDER TO AGE IN Other\n"
        "SELECT Parent\n"
        "GO TOP IN Parent\n"
        "GO TOP IN Child\n"
        "GO TOP IN Other\n"
        "SET RELATION TO AGE INTO Child\n"
        "SET RELATION TO dropchild() INTO Other ADDITIVE\n"
        "SET SKIP TO Child\n"
        "lErrorCaught = .F.\n"
        "TRY\n"
        "    SKIP 1 IN Child\n"
        "CATCH TO oErr\n"
        "    lErrorCaught = .T.\n"
        "ENDTRY\n"
        "lChildStillOpen = USED('Child')\n"
        "after = 1\n"
        "RETURN\n"
        // Only fires once Parent has been walked to record 2 -- exclusively
        // reachable from inside the one-to-many SKIP's nested parent-relation
        // synchronization, not from any of the earlier registration/GO TOP
        // syncs (Parent stays on record 1 for all of those).
        "FUNCTION dropchild\n"
        "    IF RECNO('Parent') == 2\n"
        "        USE IN Child\n"
        "    ENDIF\n"
        "    RETURN 'Alice'\n"
        "ENDFUNC\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

    expect(state.completed,
           "#6247: SKIP whose one-to-many parent walk triggers a sibling relation's key expression that "
           "closes the original SKIP-target child should complete without crashing: " + state.message);

    const auto error_caught_it = state.globals.find("lerrorcaught");
    expect(error_caught_it != state.globals.end() && error_caught_it->second.boolean_value,
           "#6247: SKIP must raise a catchable error instead of continuing through the closed original child");

    const auto still_open_it = state.globals.find("lchildstillopen");
    expect(still_open_it != state.globals.end() && !still_open_it->second.boolean_value,
           "#6247: the sibling relation's USE IN Child should genuinely have closed Child");

    const auto after_it = state.globals.find("after");
    expect(after_it != state.globals.end(), "#6247: script execution should continue after the caught error");
    fs::remove_all(temp_root, ignored);
}

void test_set_skip_registration_revalidates_callback_cursors()
{
    // RQ-CF-PRG-SET-SKIP-001: registration is evaluated before it mutates
    // relation flags, and expression callbacks cannot leave stale cursors.
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / "copperfin_set_skip_registration_callbacks";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path parent_path = root / "parent.dbf";
    const fs::path child_path = root / "child.dbf";
    const fs::path second_path = root / "second.dbf";
    write_people_dbf(parent_path, {{"PARENT10", 10}});
    write_people_dbf(child_path, {{"CHILD10", 10}});
    write_people_dbf(second_path, {{"SECOND10", 10}});

    const auto run_case = [&](const std::string &name, const std::string &command,
                              const std::string &callback, bool existing_skip = false)
    {
        const fs::path script = root / (name + ".prg");
        write_text(script,
            "USE '" + parent_path.string() + "' ALIAS Parent IN 0\n"
            "USE '" + child_path.string() + "' ALIAS Child IN 0\n"
            "USE '" + second_path.string() + "' ALIAS SecondChild IN 0\n"
            "SET ORDER TO AGE IN Parent\n"
            "SET ORDER TO AGE IN Child\n"
            "SET ORDER TO AGE IN SecondChild\n"
            "SELECT Parent\n"
            "SET RELATION TO AGE INTO Child\n"
            "SET RELATION TO AGE INTO SecondChild ADDITIVE\n"
            + (existing_skip ? "SET SKIP TO SecondChild\n" : "") +
            "nSelectedBefore = SELECT()\n"
            "cSkipBefore = SET('SKIP')\n"
            "cRelationBefore = SET('RELATION')\n"
            "lFileLockBefore = FLOCK()\n"
            "lCaught = .F.\n"
            "TRY\n"
            "    " + command + "\n"
            "CATCH TO oErr\n"
            "    lCaught = .T.\n"
            "ENDTRY\n"
            "lParentOpen = USED('Parent')\n"
            "cSkipAfter = SET('SKIP')\n"
            "nSelectedAfter = SELECT()\n"
            "cRelationAfter = SET('RELATION')\n"
            "lFileLockAfter = ISFLOCKED()\n"
            "after = 1\n"
            "RETURN\n" + callback);
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(script.string(), root.string()));
        return session.run(copperfin::runtime::DebugResumeAction::continue_run);
    };

    const auto closed = run_case(
        "parent_closed", "SET SKIP TO closeparent()",
        "FUNCTION closeparent\nUSE IN Parent\nRETURN 'Child'\nENDFUNC\n");
    expect(closed.completed, "#6460: closing parent in SET SKIP designator must remain catchable");
    expect(closed.globals.contains("lcaught") && closed.globals.at("lcaught").boolean_value,
           "#6460: closed parent must raise through TRY/CATCH");
    expect(closed.globals.contains("lparentopen") && !closed.globals.at("lparentopen").boolean_value,
           "#6460: callback must have genuinely closed the parent");
    expect(closed.globals.contains("after"), "#6460: script must continue after caught failure");

    const auto replaced_parent = run_case(
        "parent_replaced", "SET SKIP TO replaceparent()",
        "FUNCTION replaceparent\nUSE IN Parent\nUSE '" + parent_path.string() +
            "' ALIAS Parent IN 1\nRETURN 'Child'\nENDFUNC\n");
    expect(replaced_parent.completed,
           "#6460: replacing the parent work area during designator evaluation must remain catchable");
    expect(replaced_parent.globals.contains("lcaught") &&
               replaced_parent.globals.at("lcaught").boolean_value,
           "#6460: a replacement at the same parent work area must fail generation validation");
    expect(replaced_parent.globals.contains("lparentopen") &&
               replaced_parent.globals.at("lparentopen").boolean_value,
           "#6460: replacement parent should remain open after rejecting the old generation");

    const auto switched_session = run_case(
        "session_switched", "SET SKIP TO switchsession()",
        "FUNCTION switchsession\nSET DATASESSION TO 2\nRETURN 'Child'\nENDFUNC\n");
    expect(switched_session.completed,
           "#6460: data-session switch during designator evaluation must remain catchable");
    expect(switched_session.globals.contains("lcaught") &&
               switched_session.globals.at("lcaught").boolean_value,
           "#6460: SET SKIP must reject a callback that switched data sessions");

    const auto closed_child = run_case(
        "child_closed", "SET SKIP TO closechild()",
        "FUNCTION closechild\nUSE IN Child\nRETURN 'Child'\nENDFUNC\n");
    expect(closed_child.completed,
           "#6460: closing a child during designator evaluation must remain catchable");
    expect(closed_child.globals.contains("lcaught") &&
               closed_child.globals.at("lcaught").boolean_value,
           "#6460: closed child must raise through TRY/CATCH");

    const auto replaced_earlier_child = run_case(
        "earlier_child_replaced", "SET SKIP TO Child, replacechild()",
        "FUNCTION replacechild\nUSE IN Child\nUSE '" + child_path.string() +
            "' ALIAS Child IN 2\nRETURN 'SecondChild'\nENDFUNC\n");
    expect(replaced_earlier_child.completed,
           "#6460: replacing an earlier child between designators must remain catchable");
    expect(replaced_earlier_child.globals.contains("lcaught") &&
               replaced_earlier_child.globals.at("lcaught").boolean_value,
           "#6460: earlier child replacement must fail generation validation");
    expect(replaced_earlier_child.globals.contains("cskipafter") &&
               format_value(replaced_earlier_child.globals.at("cskipafter")) == "",
           "#6460: failed multi-child SET SKIP must not enable a later relation");

    const auto raised = run_case(
        "callback_error", "SET SKIP TO Child, errorchild()",
        "FUNCTION errorchild\nRETURN 1 / 0\nENDFUNC\n");
    expect(raised.completed, "#6460: a callback error must remain catchable");
    expect(raised.globals.contains("lcaught") && raised.globals.at("lcaught").boolean_value,
           "#6460: a callback error must route through TRY/CATCH");
    expect(raised.globals.contains("cskipafter") &&
               format_value(raised.globals.at("cskipafter")) == "",
           "#6460: callback error must leave prior skip flags unchanged");

    const auto missing = run_case("missing_second", "SET SKIP TO Child, 'Missing'", "");
    expect(missing.completed, "#6460: failed second designator must remain catchable");
    expect(missing.globals.contains("lcaught") && missing.globals.at("lcaught").boolean_value,
           "#6460: missing second child must raise through TRY/CATCH");
    expect(missing.globals.contains("cskipafter") &&
               format_value(missing.globals.at("cskipafter")) == "",
           "#6460: failure must not enable an earlier child's skip flag");
    expect(missing.globals.contains("nselectedbefore") &&
               missing.globals.contains("nselectedafter") &&
               format_value(missing.globals.at("nselectedbefore")) ==
                   format_value(missing.globals.at("nselectedafter")),
           "#6460: failed registration must preserve the selected work area");
    expect(missing.globals.contains("crelationbefore") &&
               missing.globals.contains("crelationafter") &&
               format_value(missing.globals.at("crelationbefore")) ==
                   format_value(missing.globals.at("crelationafter")),
           "#6460: failed registration must preserve the relation graph");
    expect(missing.globals.contains("lfilelockbefore") &&
               missing.globals.at("lfilelockbefore").boolean_value &&
               missing.globals.contains("lfilelockafter") &&
               missing.globals.at("lfilelockafter").boolean_value,
           "#6460: failed registration must retain the caller's parent file lock");

    const auto existing_skip = run_case(
        "existing_skip_preserved", "SET SKIP TO Child, 'Missing'", "", true);
    expect(existing_skip.completed && existing_skip.globals.contains("lcaught") &&
               existing_skip.globals.at("lcaught").boolean_value,
           "#6460: failed registration over an existing SET SKIP must be catchable");
    expect(existing_skip.globals.contains("cskipbefore") &&
               format_value(existing_skip.globals.at("cskipbefore")) == "SecondChild" &&
               existing_skip.globals.contains("cskipafter") &&
               format_value(existing_skip.globals.at("cskipafter")) == "SecondChild",
           "#6460: failure must preserve the previous child's enabled skip flag");

    const auto valid = run_case(
        "valid_expression", "SET SKIP TO choosechild()",
        "FUNCTION choosechild\nRETURN 'Child'\nENDFUNC\n");
    expect(valid.completed, "#6460: valid expression-backed SET SKIP must complete");
    expect(valid.globals.contains("lcaught") && !valid.globals.at("lcaught").boolean_value,
           "#6460: valid child designator must not raise an error");
    expect(valid.globals.contains("cskipafter") &&
               format_value(valid.globals.at("cskipafter")) == "Child",
           "#6460: valid child designator must enable its relation");

    const fs::path on_error_script = root / "parent_closed_on_error.prg";
    write_text(on_error_script,
        "PUBLIC handlerCalls, handlerLine, handlerMessage\n"
        "handlerCalls = 0\n"
        "USE '" + parent_path.string() + "' ALIAS Parent IN 0\n"
        "USE '" + child_path.string() + "' ALIAS Child IN 0\n"
        "SELECT Parent\n"
        "SET RELATION TO AGE INTO Child\n"
        "ON ERROR DO handleskip\n"
        "SET SKIP TO closeparent()\n"
        "after = 1\n"
        "RETURN\n"
        "FUNCTION closeparent\n"
        "USE IN Parent\n"
        "RETURN 'Child'\n"
        "ENDFUNC\n"
        "PROCEDURE handleskip\n"
        "handlerCalls = handlerCalls + 1\n"
        "handlerLine = LINENO()\n"
        "handlerMessage = MESSAGE()\n"
        "RETURN\n");
    auto on_error_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(on_error_script.string(), root.string()));
    const auto on_error = on_error_session.run(
        copperfin::runtime::DebugResumeAction::continue_run);
    expect(on_error.completed, "#6460: ON ERROR must handle invalidated SET SKIP cursor");
    expect(on_error.globals.contains("handlercalls") &&
               format_value(on_error.globals.at("handlercalls")) == "1",
           "#6460: ON ERROR must run exactly once for invalidated SET SKIP cursor");
    expect(on_error.globals.contains("handlerline") &&
               format_value(on_error.globals.at("handlerline")) == "8",
           "#6460: ON ERROR must retain the SET SKIP source line");
    expect(on_error.globals.contains("handlermessage") &&
               format_value(on_error.globals.at("handlermessage")).find("SET SKIP") != std::string::npos,
           "#6460: ON ERROR must report the SET SKIP command diagnostic");
    expect(on_error.globals.contains("after"),
           "#6460: ON ERROR must continue after the failed SET SKIP statement");
    fs::remove_all(root, ignored);
}
}

int main()
{
    test_local_set_relation_tracks_parent_navigation();
    test_local_set_skip_tracks_child_group_navigation();
    test_local_set_relation_refreshes_after_parent_mutation();
    test_local_set_relation_additive_and_explicit_parent();
    test_local_relation_introspection_preserves_order_and_session_state();
    test_set_relation_key_expression_closing_child_fails_catchably();
    test_set_relation_registration_closing_earlier_child_fails_atomically();
    test_set_relation_registration_sync_failure_rolls_back_sibling_relations();
    test_set_skip_walked_parent_closed_by_sibling_relation_fails_catchably();
    test_set_skip_relation_match_expression_closing_parent_fails_catchably();
    test_set_skip_sibling_relation_closing_original_child_fails_catchably();
    test_set_skip_registration_revalidates_callback_cursors();
    if (copperfin::test_support::test_failures() != 0)
    {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
