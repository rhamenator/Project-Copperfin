// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {

void test_expression_level_function_call_works_in_if_predicates() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_expr_function_if";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "expr_function_if.prg";
    write_text(
        main_path,
        "flag = 0\n"
        "IF somepredicate('ok')\n"
        "    flag = 1\n"
        "ENDIF\n"
        "RETURN\n"
        "FUNCTION somepredicate\n"
        "LPARAMETERS tcValue\n"
        "RETURN tcValue = 'ok'\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "expression-level FUNCTION predicate script should complete");

    const auto flag = state.globals.find("flag");
    expect(flag != state.globals.end(), "expression-level FUNCTION predicate should leave flag visible");
    if (flag != state.globals.end()) {
        expect(copperfin::runtime::format_value(flag->second) == "1",
               "expression-level FUNCTION predicate should drive IF control flow");
    }

    fs::remove_all(temp_root, ignored);
}

void test_if_elseif_predicates_use_heap_backed_expression_checkpoints() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_if_predicate_continuation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path semantics_path = temp_root / "semantics.prg";
    write_text(
        semantics_path,
        "branch = ''\n"
        "gateCalls = 0\n"
        "innerCalls = 0\n"
        "firstArgCalls = 0\n"
        "secondArgCalls = 0\n"
        "thirdArgCalls = 0\n"
        "fourthArgCalls = 0\n"
        "shortCircuitCalls = 0\n"
        "IF gate(firstarg(), secondarg()) AND shortcircuit()\n"
        "    branch = 'if'\n"
        "ELSEIF gate(thirdarg(), fourtharg()) OR shortcircuit()\n"
        "    branch = 'elseif'\n"
        "ELSE\n"
        "    branch = 'else'\n"
        "ENDIF\n"
        "IF deep_predicate(1)\n"
        "    deepBranch = 'true'\n"
        "ENDIF\n"
        "RETURN\n"
        "FUNCTION gate\n"
        "LPARAMETERS leftValue, rightValue\n"
        "gateCalls = gateCalls + 1\n"
        "RETURN innergate(leftValue, rightValue)\n"
        "FUNCTION innergate\n"
        "LPARAMETERS leftValue, rightValue\n"
        "innerCalls = innerCalls + 1\n"
        "RETURN leftValue = 9 AND rightValue = 10\n"
        "FUNCTION firstarg\n"
        "firstArgCalls = firstArgCalls + 1\n"
        "RETURN 1\n"
        "FUNCTION secondarg\n"
        "secondArgCalls = secondArgCalls + 1\n"
        "RETURN 2\n"
        "FUNCTION thirdarg\n"
        "thirdArgCalls = thirdArgCalls + 1\n"
        "RETURN 9\n"
        "FUNCTION fourtharg\n"
        "fourthArgCalls = fourthArgCalls + 1\n"
        "RETURN 10\n"
        "FUNCTION shortcircuit\n"
        "shortCircuitCalls = shortCircuitCalls + 1\n"
        "RETURN .T.\n"
        "FUNCTION deep_predicate\n"
        "LPARAMETERS depth\n"
        "IF depth >= 768\n"
        "    RETURN .T.\n"
        "ENDIF\n"
        "RETURN deep_predicate(depth + 1)\n");

    auto semantics_options = make_runtime_session_options(semantics_path.string(), temp_root.string(), false);
    semantics_options.max_call_depth = 1024U;
    auto semantics_session = copperfin::runtime::PrgRuntimeSession::create(semantics_options);
    const auto semantics_state = semantics_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(semantics_state.completed,
           "IF/ELSEIF predicate continuation semantics should complete: " + semantics_state.message);

    const auto expect_global = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = semantics_state.globals.find(name);
        expect(found != semantics_state.globals.end(), name + " should remain visible");
        if (found != semantics_state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };
    expect_global("branch", "elseif", "ELSEIF should receive the false IF predicate and select its own true predicate");
    expect_global("deepbranch", "true", "a deep UDF predicate should resume the caller IF without host-stack growth");
    expect_global("gatecalls", "2", "each physical conditional predicate should invoke its outer UDF once");
    expect_global("innercalls", "2", "nested UDF predicate calls should execute once per physical predicate");
    expect_global("firstargcalls", "1", "the first IF UDF argument should execute once");
    expect_global("secondargcalls", "1", "the second IF UDF argument should execute once");
    expect_global("thirdargcalls", "1", "the ELSEIF UDF argument should execute once");
    expect_global("fourthargcalls", "1", "the second ELSEIF UDF argument should execute once");
    expect_global("shortcircuitcalls", "0", "short-circuited predicate operands should not execute");

    const fs::path debugger_path = temp_root / "debugger.prg";
    write_text(
        debugger_path,
        "IF child()\n"
        "    result = 1\n"
        "ENDIF\n"
        "after = 1\n"
        "RETURN\n"
        "FUNCTION child\n"
        "RETURN .T.\n");

    auto exact_budget_options = make_runtime_session_options(debugger_path.string(), temp_root.string(), false);
    exact_budget_options.max_executed_statements = 6U;
    auto exact_budget_session = copperfin::runtime::PrgRuntimeSession::create(exact_budget_options);
    const auto exact_budget_state = exact_budget_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(exact_budget_state.completed && exact_budget_state.executed_statement_count == 6U,
           "a resumed IF predicate should not consume a second statement-budget slot");

    auto exhausted_budget_options = make_runtime_session_options(debugger_path.string(), temp_root.string(), false);
    exhausted_budget_options.max_executed_statements = 5U;
    auto exhausted_budget_session = copperfin::runtime::PrgRuntimeSession::create(exhausted_budget_options);
    const auto exhausted_budget_state = exhausted_budget_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(exhausted_budget_state.reason == copperfin::runtime::DebugPauseReason::error &&
               exhausted_budget_state.executed_statement_count == 5U,
           "IF predicate budget exhaustion should occur at the physical statement boundary");

    auto breakpoint_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(debugger_path.string(), temp_root.string(), false));
    breakpoint_session.add_breakpoint({.file_path = debugger_path.string(), .line = 1U});
    const auto breakpoint_state = breakpoint_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(breakpoint_state.reason == copperfin::runtime::DebugPauseReason::breakpoint,
           "an IF predicate should honor its breakpoint before UDF evaluation");
    const auto breakpoint_completed_state = breakpoint_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(breakpoint_completed_state.completed,
           "a resumed IF predicate should not hit its consumed breakpoint again");
    expect(std::count_if(
               breakpoint_completed_state.events.begin(),
               breakpoint_completed_state.events.end(),
               [](const copperfin::runtime::RuntimeEvent &event) {
                   return event.category == "execute" && event.location.line == 1U;
               }) == 1,
           "an IF predicate should emit one execute event for its physical statement");

    const auto make_debug_session = [&]() {
        return copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(debugger_path.string(), temp_root.string(), true));
    };
    auto debug_session = make_debug_session();
    const auto entry_state = debug_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    const auto child_state = debug_session.run(copperfin::runtime::DebugResumeAction::step_into);
    const auto resumed_state = debug_session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(entry_state.reason == copperfin::runtime::DebugPauseReason::entry &&
               child_state.reason == copperfin::runtime::DebugPauseReason::step && child_state.location.line == 7U &&
               resumed_state.reason == copperfin::runtime::DebugPauseReason::step && resumed_state.location.line == 1U,
           "debugger stepping should expose the child RETURN then the suspended IF predicate");
    const auto completed_state = debug_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(completed_state.completed, "continuing a debugger-paused IF predicate should complete");

    auto cancel_session = make_debug_session();
    (void)cancel_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    (void)cancel_session.run(copperfin::runtime::DebugResumeAction::step_into);
    const auto cancel_resumed_state = cancel_session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(cancel_resumed_state.location.line == 1U,
           "cancellation setup should pause on the suspended IF predicate");
    cancel_session.request_cancel();
    const auto cancelled_state = cancel_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(cancelled_state.reason == copperfin::runtime::DebugPauseReason::error &&
               cancelled_state.location.line == 1U && cancelled_state.statement_text == "IF child()",
           "cancelling a suspended IF predicate should retain its source metadata");
    expect(cancelled_state.globals.find("result") == cancelled_state.globals.end(),
           "cancelling a suspended IF predicate should not execute its branch");

    const auto expect_direct_control_transfer_abandons_predicate =
        [&](const std::string &name, const std::string &command, const std::string &event_category) {
            const fs::path control_path = temp_root / (name + ".prg");
            write_text(
                control_path,
                "IF controlpredicate()\n"
                "    branch = 1\n"
                "ENDIF\n"
                "afterControl = 1\n"
                "RETURN\n"
                "FUNCTION controlpredicate\n" +
                    command + "\n"
                "RETURN .T.\n");
            auto control_session = copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(control_path.string(), temp_root.string(), false));
            const auto control_state = control_session.run(copperfin::runtime::DebugResumeAction::continue_run);
            expect(control_state.completed, command + " in an IF predicate should terminate cleanly");
            expect(control_state.globals.find("branch") == control_state.globals.end(),
                   command + " should abandon the pending IF branch");
            expect(control_state.globals.find("aftercontrol") == control_state.globals.end(),
                   command + " should prevent execution after the abandoned predicate");
            expect(std::count_if(
                       control_state.events.begin(),
                       control_state.events.end(),
                       [&](const copperfin::runtime::RuntimeEvent &event) {
                           return event.category == event_category;
                       }) == 1,
                   command + " should preserve its runtime event while abandoning the predicate");
        };
    expect_direct_control_transfer_abandons_predicate("if_cancel_predicate", "CANCEL", "runtime.cancel");
    expect_direct_control_transfer_abandons_predicate("if_quit_predicate", "QUIT", "runtime.quit");

    fs::remove_all(temp_root, ignored);
}

void test_case_predicates_use_heap_backed_expression_checkpoints() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_case_predicate_continuation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});

    const fs::path semantics_path = temp_root / "semantics.prg";
    write_text(
        semantics_path,
        "SET UDFPARMS TO REFERENCE\n"
        "scalarValue = 3\n"
        "DIMENSION values[2]\n"
        "values[1] = 10\n"
        "values[2] = 20\n"
        "branch = ''\n"
        "predicateCalls = 0\n"
        "argumentCalls = 0\n"
        "shortCircuitCalls = 0\n"
        "skippedCalls = 0\n"
        "otherwisePredicateCalls = 0\n"
        "DO CASE\n"
        "    CASE falsepredicate(@scalarValue, values) AND shortcircuit()\n"
        "        branch = 'first'\n"
        "    CASE truepredicate(@scalarValue, values, nestedargument())\n"
        "        branch = 'second'\n"
        "    CASE skippedpredicate()\n"
        "        branch = 'bad'\n"
        "    OTHERWISE\n"
        "        branch = 'otherwise'\n"
        "ENDCASE\n"
        "otherwiseBranch = ''\n"
        "DO CASE\n"
        "    CASE falseotherwise()\n"
        "        otherwiseBranch = 'bad'\n"
        "    OTHERWISE\n"
        "        otherwiseBranch = 'otherwise'\n"
        "ENDCASE\n"
        "scalarAfter = scalarValue\n"
        "arrayAfter = values[2]\n"
        "nestedBranch = ''\n"
        "DO CASE\n"
        "    CASE outercase()\n"
        "        DO CASE\n"
        "            CASE innercase()\n"
        "                nestedBranch = 'inner'\n"
        "            OTHERWISE\n"
        "                nestedBranch = 'inner-otherwise'\n"
        "        ENDCASE\n"
        "    OTHERWISE\n"
        "        nestedBranch = 'outer-otherwise'\n"
        "ENDCASE\n"
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SELECT People\n"
        "scanMatches = 0\n"
        "scanOtherwise = 0\n"
        "SCAN\n"
        "    DO CASE\n"
        "        CASE scanpredicate(AGE)\n"
        "            scanMatches = scanMatches + 1\n"
        "        OTHERWISE\n"
        "            scanOtherwise = scanOtherwise + 1\n"
        "    ENDCASE\n"
        "ENDSCAN\n"
        "RETURN\n"
        "FUNCTION falsepredicate\n"
        "LPARAMETERS scalarRef, arrayRef\n"
        "predicateCalls = predicateCalls + 1\n"
        "scalarRef = scalarRef + 1\n"
        "arrayRef[1] = arrayRef[1] + 1\n"
        "RETURN .F.\n"
        "FUNCTION truepredicate\n"
        "LPARAMETERS scalarRef, arrayRef, token\n"
        "predicateCalls = predicateCalls + 1\n"
        "arrayRef[2] = arrayRef[2] + 5\n"
        "RETURN scalarRef = 4 AND arrayRef[2] = 25 AND token = 'token'\n"
        "FUNCTION nestedargument\n"
        "argumentCalls = argumentCalls + 1\n"
        "RETURN 'token'\n"
        "FUNCTION shortcircuit\n"
        "shortCircuitCalls = shortCircuitCalls + 1\n"
        "RETURN .T.\n"
        "FUNCTION skippedpredicate\n"
        "skippedCalls = skippedCalls + 1\n"
        "RETURN .T.\n"
        "FUNCTION outercase\n"
        "RETURN outercasechild()\n"
        "FUNCTION outercasechild\n"
        "RETURN .T.\n"
        "FUNCTION innercase\n"
        "RETURN innercasechild()\n"
        "FUNCTION innercasechild\n"
        "RETURN .T.\n"
        "FUNCTION falseotherwise\n"
        "otherwisePredicateCalls = otherwisePredicateCalls + 1\n"
        "RETURN falseotherwisechild()\n"
        "FUNCTION falseotherwisechild\n"
        "RETURN .F.\n"
        "FUNCTION scanpredicate\n"
        "LPARAMETERS ageValue\n"
        "RETURN scanpredicatechild(ageValue)\n"
        "FUNCTION scanpredicatechild\n"
        "LPARAMETERS ageValue\n"
        "RETURN ageValue >= 20\n");

    auto semantics_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(semantics_path.string(), temp_root.string(), false));
    const auto semantics_state = semantics_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(semantics_state.completed,
           "CASE predicate continuation semantics should complete: " + semantics_state.message);

    const auto expect_global = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = semantics_state.globals.find(name);
        expect(found != semantics_state.globals.end(), name + " should remain visible");
        if (found != semantics_state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };
    expect_global("branch", "second", "the first false CASE should route to the next CASE and the second should match");
    expect_global("scalarafter", "4", "an explicit scalar reference should survive a resumed CASE predicate");
    expect_global("arrayafter", "25", "a bare array reference should survive a resumed CASE predicate");
    expect_global("predicatecalls", "2", "each evaluated CASE UDF should execute exactly once");
    expect_global("argumentcalls", "1", "nested CASE predicate arguments should execute exactly once");
    expect_global("shortcircuitcalls", "0", "short-circuited CASE predicate operands should not execute");
    expect_global("otherwisebranch", "otherwise", "a resumed false CASE predicate should route to OTHERWISE");
    expect_global("otherwisepredicatecalls", "1", "a resumed false CASE predicate should execute exactly once before OTHERWISE");
    {
        const auto skipped = semantics_state.globals.find("skippedcalls");
        expect(skipped != semantics_state.globals.end(), "skippedcalls should remain visible");
        if (skipped != semantics_state.globals.end()) {
            expect(copperfin::runtime::format_value(skipped->second) == "0",
                   "CASE expressions after a match should not be evaluated (actual=" +
                       copperfin::runtime::format_value(skipped->second) + ")");
        }
    }
    expect_global("nestedbranch", "inner", "nested DO CASE should preserve its own CaseState during suspension");
    expect_global("scanmatches", "3", "SCAN-embedded DO CASE should classify matching records");
    expect_global("scanotherwise", "1", "SCAN-embedded DO CASE should retain OTHERWISE behavior");

    const fs::path error_path = temp_root / "errors.prg";
    write_text(
        error_path,
        "caughtCount = 0\n"
        "finallyCalls = 0\n"
        "TRY\n"
        "    DO CASE\n"
        "        CASE faultpredicate()\n"
        "            faultBranch = 1\n"
        "        OTHERWISE\n"
        "            otherwiseBranch = 1\n"
        "    ENDCASE\n"
        "CATCH TO errorText\n"
        "    caughtCount = caughtCount + 1\n"
        "FINALLY\n"
        "    finallyCalls = finallyCalls + 1\n"
        "ENDTRY\n"
        "afterTry = 1\n"
        "cleanupBranch = ''\n"
        "DO CASE\n"
        "    CASE cleanupPredicate()\n"
        "        cleanupBranch = 'branch'\n"
        "    OTHERWISE\n"
        "        cleanupBranch = 'otherwise'\n"
        "ENDCASE\n"
        "RETURN\n"
        "FUNCTION faultpredicate\n"
        "RETURN faultchild()\n"
        "FUNCTION faultchild\n"
        "DO missing_case_routine\n"
        "RETURN .T.\n"
        "FUNCTION cleanupPredicate\n"
        "RETURN .T.\n");
    auto error_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(error_path.string(), temp_root.string(), false));
    const auto error_state = error_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(error_state.completed, "TRY/CATCH/FINALLY should handle a resumed CASE fault: " + error_state.message);
    expect(error_state.globals.find("errortext") != error_state.globals.end(),
           "CATCH TO should capture a fault from a resumed CASE predicate");
    expect(error_state.globals.find("faultbranch") == error_state.globals.end(),
           "a faulting CASE predicate should not execute its branch");
    expect(error_state.globals.find("otherwisebranch") == error_state.globals.end(),
           "a handled CASE predicate fault should not fall through to OTHERWISE");
    const auto error_caught = error_state.globals.find("caughtcount");
    const auto error_finally = error_state.globals.find("finallycalls");
    const auto error_after = error_state.globals.find("aftertry");
    expect(error_caught != error_state.globals.end() && copperfin::runtime::format_value(error_caught->second) == "1",
           "CATCH should run once for a fault from a resumed CASE predicate");
    expect(error_finally != error_state.globals.end() && copperfin::runtime::format_value(error_finally->second) == "1",
           "FINALLY should run during a resumed CASE predicate fault");
    expect(error_after != error_state.globals.end() && copperfin::runtime::format_value(error_after->second) == "1",
           "execution should continue after handling a resumed CASE predicate fault");
    expect(error_state.globals.find("cleanupbranch") != error_state.globals.end() &&
               copperfin::runtime::format_value(error_state.globals.at("cleanupbranch")) == "branch",
           "a CASE after TRY/CATCH/FINALLY should start with a clean CaseState");

    const fs::path on_error_path = temp_root / "on_error.prg";
    write_text(
        on_error_path,
        "PUBLIC handlerCalls\n"
        "handlerCalls = 0\n"
        "ON ERROR DO handlecaseerror\n"
        "DO CASE\n"
        "    CASE onerrorpredicate()\n"
        "        onErrorCaseBranch = 1\n"
        "    OTHERWISE\n"
        "        onErrorOtherwise = 1\n"
        "ENDCASE\n"
        "afterOnError = 1\n"
        "onErrorCleanup = ''\n"
        "DO CASE\n"
        "    CASE onErrorCleanupPredicate()\n"
        "        onErrorCleanup = 'branch'\n"
        "    OTHERWISE\n"
        "        onErrorCleanup = 'otherwise'\n"
        "ENDCASE\n"
        "RETURN\n"
        "FUNCTION onerrorpredicate\n"
        "RETURN casechild() + 1 / 0\n"
        "FUNCTION casechild\n"
        "RETURN 7\n"
        "PROCEDURE handlecaseerror\n"
        "handlerCalls = handlerCalls + 1\n"
        "RETURN .T.\n"
        "FUNCTION onErrorCleanupPredicate\n"
        "RETURN .T.\n");
    auto on_error_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(on_error_path.string(), temp_root.string(), false));
    const auto on_error_state = on_error_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(on_error_state.completed, "ON ERROR should handle a resumed CASE fault: " + on_error_state.message);
    const auto handler_calls = on_error_state.globals.find("handlercalls");
    expect(handler_calls != on_error_state.globals.end() && copperfin::runtime::format_value(handler_calls->second) == "1",
           "ON ERROR should run once for a resumed CASE predicate fault");
    expect(on_error_state.globals.find("onerrorcasebranch") == on_error_state.globals.end(),
           "ON ERROR should prevent the faulting CASE branch from executing");
    expect(on_error_state.globals.find("onerrorotherwise") != on_error_state.globals.end(),
           "ON ERROR should resume CASE through its OTHERWISE fallback");
    expect(on_error_state.globals.find("onerrorcleanup") != on_error_state.globals.end() &&
               copperfin::runtime::format_value(on_error_state.globals.at("onerrorcleanup")) == "branch",
           "a CASE after ON ERROR should start with a clean CaseState");

    const fs::path resume_path = temp_root / "resume.prg";
    write_text(
        resume_path,
        "resumeCalls = 0\n"
        "ON ERROR DO handleresume\n"
        "DO CASE\n"
        "    CASE resumechild() + 1 / 0\n"
        "        resumeBranch = 1\n"
        "    OTHERWISE\n"
        "        resumeOtherwise = 1\n"
        "ENDCASE\n"
        "afterResume = 1\n"
        "resumeCleanup = ''\n"
        "DO CASE\n"
        "    CASE resumeCleanupPredicate()\n"
        "        resumeCleanup = 'branch'\n"
        "    OTHERWISE\n"
        "        resumeCleanup = 'otherwise'\n"
        "ENDCASE\n"
        "RETURN\n"
        "FUNCTION resumechild\n"
        "RETURN 1\n"
        "PROCEDURE handleresume\n"
        "resumeCalls = resumeCalls + 1\n"
        "RESUME\n"
        "RETURN\n"
        "FUNCTION resumeCleanupPredicate\n"
        "RETURN .T.\n");
    auto resume_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(resume_path.string(), temp_root.string(), false));
    const auto resume_state = resume_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(resume_state.completed, "RESUME should handle a resumed CASE predicate fault: " + resume_state.message);
    expect(resume_state.globals.find("resumebranch") == resume_state.globals.end(),
           "RESUME should not execute a CASE branch after a predicate fault");
    expect(resume_state.globals.find("resumeotherwise") == resume_state.globals.end(),
           "RESUME should continue after the CASE statement rather than enter OTHERWISE");
    expect(resume_state.globals.find("afterresume") != resume_state.globals.end(),
           "RESUME should continue after the faulting CASE statement");
    expect(resume_state.globals.find("resumecleanup") != resume_state.globals.end() &&
               copperfin::runtime::format_value(resume_state.globals.at("resumecleanup")) == "branch",
           "a CASE after RESUME should start with a clean CaseState");

    const fs::path nested_resume_path = temp_root / "nested_resume.prg";
    write_text(
        nested_resume_path,
        "nestedResumeCalls = 0\n"
        "ON ERROR DO handleNestedResume\n"
        "DO CASE\n"
        "    CASE nestedResumePredicate()\n"
        "        nestedResumeBranch = 1\n"
        "    OTHERWISE\n"
        "        nestedResumeOtherwise = 1\n"
        "ENDCASE\n"
        "afterNestedResume = 1\n"
        "RETURN\n"
        "FUNCTION nestedResumePredicate\n"
        "RETURN nestedResumeChild() + 1 / 0\n"
        "FUNCTION nestedResumeChild\n"
        "RETURN 1\n"
        "PROCEDURE handleNestedResume\n"
        "nestedResumeCalls = nestedResumeCalls + 1\n"
        "RESUME\n"
        "RETURN\n");
    auto nested_resume_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(nested_resume_path.string(), temp_root.string(), false));
    const auto nested_resume_state =
        nested_resume_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(nested_resume_state.completed,
           "RESUME should abandon a CASE when its predicate UDF faults: " + nested_resume_state.message);
    expect(nested_resume_state.globals.find("nestedresumebranch") == nested_resume_state.globals.end(),
           "RESUME should not execute a CASE branch after a nested predicate UDF fault");
    expect(nested_resume_state.globals.find("nestedresumeotherwise") == nested_resume_state.globals.end(),
           "RESUME should skip the entire CASE after a nested predicate UDF fault");
    expect(nested_resume_state.globals.find("afternestedresume") != nested_resume_state.globals.end(),
           "RESUME should continue after a CASE whose predicate UDF faulted");

    const fs::path retry_path = temp_root / "retry.prg";
    write_text(
        retry_path,
        "retryCalls = 0\n"
        "predicateCalls = 0\n"
        "ON ERROR DO handleretry\n"
        "DO CASE\n"
        "    CASE retrypredicate()\n"
        "        retryBranch = 1\n"
        "    OTHERWISE\n"
        "        retryOtherwise = 1\n"
        "ENDCASE\n"
        "afterRetry = 1\n"
        "retryCleanup = ''\n"
        "DO CASE\n"
        "    CASE retryCleanupPredicate()\n"
        "        retryCleanup = 'branch'\n"
        "    OTHERWISE\n"
        "        retryCleanup = 'otherwise'\n"
        "ENDCASE\n"
        "RETURN\n"
        "FUNCTION retrypredicate\n"
        "predicateCalls = predicateCalls + 1\n"
        "RETURN retrychild() + 1 / 0\n"
        "FUNCTION retrychild\n"
        "RETURN 1\n"
        "PROCEDURE handleretry\n"
        "retryCalls = retryCalls + 1\n"
        "IF retryCalls < 2\n"
        "    RETRY\n"
        "ENDIF\n"
        "RETURN\n"
        "FUNCTION retryCleanupPredicate\n"
        "RETURN .T.\n");
    auto retry_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(retry_path.string(), temp_root.string(), false));
    const auto retry_state = retry_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(retry_state.completed, "RETRY should re-execute a CASE predicate fault: " + retry_state.message);
    expect(retry_state.globals.find("retrybranch") == retry_state.globals.end(),
           "RETRY should not execute a CASE branch when the predicate remains faulty");
    expect(retry_state.globals.find("retryotherwise") != retry_state.globals.end(),
           "RETRY should resume the CASE through OTHERWISE after the handler returns");
    const auto retry_calls = retry_state.globals.find("retrycalls");
    const auto predicate_calls = retry_state.globals.find("predicatecalls");
    expect(retry_calls != retry_state.globals.end() && copperfin::runtime::format_value(retry_calls->second) == "2",
           "RETRY should invoke its handler twice");
    expect(predicate_calls != retry_state.globals.end() && copperfin::runtime::format_value(predicate_calls->second) == "1",
           "RETRY should not replay completed predicate side effects before the faulting RETURN");
    expect(retry_state.globals.find("afterretry") != retry_state.globals.end(),
           "RETRY should continue after the predicate fault is handled");
    expect(retry_state.globals.find("retrycleanup") != retry_state.globals.end() &&
               copperfin::runtime::format_value(retry_state.globals.at("retrycleanup")) == "branch",
           "a CASE after RETRY should start with a clean CaseState");

    const fs::path depth_path = temp_root / "depth.prg";
    write_text(
        depth_path,
        "DO CASE\n"
        "    CASE deepcase(1)\n"
        "        depthBranch = 1\n"
        "ENDCASE\n"
        "RETURN\n"
        "FUNCTION deepcase\n"
        "LPARAMETERS depth\n"
        "RETURN deepcase(depth + 1)\n");
    auto depth_options = make_runtime_session_options(depth_path.string(), temp_root.string(), false);
    depth_options.max_call_depth = 96U;
    auto depth_session = copperfin::runtime::PrgRuntimeSession::create(depth_options);
    const auto depth_state = depth_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(depth_state.reason == copperfin::runtime::DebugPauseReason::error,
           "a deep CASE predicate should stop at the runtime call-depth guardrail");
    expect(depth_state.message.find("maximum call depth") != std::string::npos,
           "a deep CASE predicate should report max_call_depth without exhausting the native stack");

    const fs::path debugger_path = temp_root / "debugger.prg";
    write_text(
        debugger_path,
        "DO CASE\n"
        "    CASE child()\n"
        "        caseBranch = 1\n"
        "    OTHERWISE\n"
        "        caseOtherwise = 1\n"
        "ENDCASE\n"
        "afterCase = 1\n"
        "RETURN\n"
        "FUNCTION child\n"
        "RETURN .T.\n");

    auto exact_budget_options = make_runtime_session_options(debugger_path.string(), temp_root.string(), false);
    exact_budget_options.max_executed_statements = 7U;
    auto exact_budget_session = copperfin::runtime::PrgRuntimeSession::create(exact_budget_options);
    const auto exact_budget_state = exact_budget_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(exact_budget_state.completed && exact_budget_state.executed_statement_count == 7U,
           "a resumed CASE predicate should not consume a second statement-budget slot");

    auto exhausted_budget_options = make_runtime_session_options(debugger_path.string(), temp_root.string(), false);
    exhausted_budget_options.max_executed_statements = 6U;
    auto exhausted_budget_session = copperfin::runtime::PrgRuntimeSession::create(exhausted_budget_options);
    const auto exhausted_budget_state = exhausted_budget_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(exhausted_budget_state.reason == copperfin::runtime::DebugPauseReason::error &&
               exhausted_budget_state.executed_statement_count == 6U,
           "CASE predicate budget exhaustion should occur at the physical statement boundary");

    auto breakpoint_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(debugger_path.string(), temp_root.string(), false));
    breakpoint_session.add_breakpoint({.file_path = debugger_path.string(), .line = 2U});
    const auto breakpoint_state = breakpoint_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(breakpoint_state.reason == copperfin::runtime::DebugPauseReason::breakpoint &&
               breakpoint_state.location.line == 2U,
           "a CASE predicate should honor its breakpoint before UDF evaluation");
    const auto breakpoint_completed_state = breakpoint_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(breakpoint_completed_state.completed, "a resumed CASE predicate should complete after its breakpoint");
    expect(std::count_if(
               breakpoint_completed_state.events.begin(),
               breakpoint_completed_state.events.end(),
               [](const copperfin::runtime::RuntimeEvent &event) {
                   return event.category == "execute" && event.location.line == 2U;
               }) == 1,
           "a CASE predicate should emit one execute event for its physical statement");

    const auto make_debug_session = [&]() {
        return copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(debugger_path.string(), temp_root.string(), true));
    };
    auto debug_session = make_debug_session();
    const auto entry_state = debug_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    const auto case_state = debug_session.run(copperfin::runtime::DebugResumeAction::step_into);
    const auto child_state = debug_session.run(copperfin::runtime::DebugResumeAction::step_into);
    const auto resumed_state = debug_session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(entry_state.reason == copperfin::runtime::DebugPauseReason::entry &&
               case_state.reason == copperfin::runtime::DebugPauseReason::step && case_state.location.line == 2U &&
               child_state.reason == copperfin::runtime::DebugPauseReason::step && child_state.location.line == 10U &&
               resumed_state.reason == copperfin::runtime::DebugPauseReason::step && resumed_state.location.line == 2U,
           "debugger stepping should expose the CASE, child RETURN, then resumed CASE predicate");
    const auto completed_state = debug_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(completed_state.completed, "continuing a debugger-paused CASE predicate should complete");

    auto cancel_session = make_debug_session();
    (void)cancel_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    (void)cancel_session.run(copperfin::runtime::DebugResumeAction::step_into);
    (void)cancel_session.run(copperfin::runtime::DebugResumeAction::step_into);
    const auto cancel_resumed_state = cancel_session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(cancel_resumed_state.location.line == 2U,
           "cancellation setup should pause on the suspended CASE predicate");
    cancel_session.request_cancel();
    const auto cancelled_state = cancel_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(cancelled_state.reason == copperfin::runtime::DebugPauseReason::error &&
               cancelled_state.location.line == 2U && cancelled_state.statement_text == "CASE child()",
           "cancelling a suspended CASE predicate should retain its source metadata");
    expect(cancelled_state.globals.find("casebranch") == cancelled_state.globals.end(),
           "cancelling a suspended CASE predicate should not execute its branch");

    const auto expect_direct_control_transfer_abandons_predicate =
        [&](const std::string &name, const std::string &command, const std::string &event_category) {
            const fs::path control_path = temp_root / (name + ".prg");
            write_text(
                control_path,
                "DO CASE\n"
                "    CASE controlpredicate()\n"
                "        controlBranch = 1\n"
                "ENDCASE\n"
                "afterControl = 1\n"
                "RETURN\n"
                "FUNCTION controlpredicate\n" +
                    command + "\n"
                "RETURN .T.\n");
            auto control_session = copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(control_path.string(), temp_root.string(), false));
            const auto control_state = control_session.run(copperfin::runtime::DebugResumeAction::continue_run);
            expect(control_state.completed, command + " in a CASE predicate should terminate cleanly");
            expect(control_state.globals.find("controlbranch") == control_state.globals.end(),
                   command + " should abandon the pending CASE branch");
            expect(control_state.globals.find("aftercontrol") == control_state.globals.end(),
                   command + " should prevent execution after the abandoned CASE predicate");
            expect(std::count_if(
                       control_state.events.begin(),
                       control_state.events.end(),
                       [&](const copperfin::runtime::RuntimeEvent &event) {
                           return event.category == event_category;
                       }) == 1,
                   command + " should preserve its runtime event while abandoning the CASE predicate");
        };
    expect_direct_control_transfer_abandons_predicate("case_cancel_predicate", "CANCEL", "runtime.cancel");
    expect_direct_control_transfer_abandons_predicate("case_quit_predicate", "QUIT", "runtime.quit");

    fs::remove_all(temp_root, ignored);
}


}  // namespace cf_test_prg_engine_control_flow
