// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {

void test_expression_level_function_call_can_chain_nested_user_routines() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_expr_function_nested";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "expr_function_nested.prg";
    write_text(
        main_path,
        "result = outer(4)\n"
        "RETURN\n"
        "FUNCTION outer\n"
        "LPARAMETERS tnValue\n"
        "RETURN inner(tnValue) + 1\n"
        "FUNCTION inner\n"
        "LPARAMETERS tnValue\n"
        "RETURN tnValue * 2\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "nested expression-level FUNCTION script should complete");

    const auto result = state.globals.find("result");
    expect(result != state.globals.end(), "nested expression-level FUNCTION call should assign result");
    if (result != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(result->second);
        expect(actual == "9",
               "expression-level FUNCTION calls should chain through nested user-defined routines (actual " + actual + ")");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_procedure_registers_external_function_for_expression_calls() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_procedure_function";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_text(
        temp_root / "helpers.prg",
        "FUNCTION addvals\n"
        "LPARAMETERS a, b\n"
        "RETURN a + b\n");
    write_text(
        temp_root / "main.prg",
        "cHelperPath = 'helpers'\n"
        "SET PROCEDURE TO &cHelperPath\n"
        "result = addvals(6, 7)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((temp_root / "main.prg").string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET PROCEDURE expression-level helper script should complete");

    const auto result = state.globals.find("result");
    expect(result != state.globals.end(), "SET PROCEDURE function script should assign result");
    if (result != state.globals.end()) {
        expect(copperfin::runtime::format_value(result->second) == "13",
               "SET PROCEDURE should expose helper functions to expression-level calls");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_procedure_registers_external_procedure_for_do_calls() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_procedure_do";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_text(
        temp_root / "helpers.prg",
        "PROCEDURE bump\n"
        "LPARAMETERS pcount\n"
        "pcount = pcount + 4\n"
        "RETURN\n");
    write_text(
        temp_root / "main.prg",
        "SET PROCEDURE TO helpers\n"
        "counter = 3\n"
        "DO bump WITH @counter\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((temp_root / "main.prg").string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET PROCEDURE DO helper script should complete");

    const auto counter = state.globals.find("counter");
    expect(counter != state.globals.end(), "SET PROCEDURE DO helper should preserve caller variable");
    if (counter != state.globals.end()) {
        expect(copperfin::runtime::format_value(counter->second) == "7",
               "SET PROCEDURE should expose helper procedures to unqualified DO calls");
    }

    fs::remove_all(temp_root, ignored);
}

void test_do_procedure_in_program_invokes_procedure_from_named_file() {
    // #6443: DO ProcedureName IN ProgramName2 [WITH ParameterList] selects a
    // procedure from a specific program file without any SET PROCEDURE
    // registration. The parser previously stored "ProcedureName IN
    // ProgramName2" as one undifferentiated identifier, so dispatch never
    // resolved the IN clause at all and the statement paused/faulted
    // instead of invoking the requested procedure.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_do_procedure_in_program";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_text(
        temp_root / "external_proc.prg",
        "PROCEDURE TargetProc\n"
        "LPARAMETERS value\n"
        "do_in_marker = value\n"
        "RETURN\n"
        "ENDPROC\n");
    write_text(
        temp_root / "main.prg",
        "PUBLIC do_in_marker\n"
        "do_in_marker = 0\n"
        "DO TargetProc IN external_proc.prg WITH 7\n"
        "after_do_in = 1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((temp_root / "main.prg").string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6443: DO ProcedureName IN ProgramName2 script should complete: " + state.message);

    const auto marker = state.globals.find("do_in_marker");
    expect(marker != state.globals.end(), "#6443: DO...IN should invoke the named procedure in the target program");
    if (marker != state.globals.end()) {
        expect(copperfin::runtime::format_value(marker->second) == "7",
               "#6443: DO...IN WITH should pass the parameter through to the selected procedure");
    }

    const auto after = state.globals.find("after_do_in");
    expect(after != state.globals.end(),
           "#6443: execution should continue past the DO...IN statement instead of pausing/faulting");

    fs::remove_all(temp_root, ignored);
}

void test_do_procedure_in_missing_program_raises_catchable_error() {
    // #6443: a nonexistent IN target must fail catchably, not pause/fault
    // the host or silently succeed.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_do_procedure_in_missing";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_text(
        temp_root / "main.prg",
        "TRY\n"
        "  DO TargetProc IN missing_proc.prg\n"
        "  lReached = .T.\n"
        "CATCH TO oErr\n"
        "  nCaughtErrorNo = oErr.ErrorNo\n"
        "ENDTRY\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((temp_root / "main.prg").string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6443: missing DO...IN target script should complete: " + state.message);

    expect(state.globals.find("lreached") == state.globals.end(),
           "#6443: a missing DO...IN program should raise a catchable error rather than continue");
    expect(state.globals.find("ncaughterrorno") != state.globals.end(),
           "#6443: a missing DO...IN program should be caught by an enclosing TRY/CATCH");

    fs::remove_all(temp_root, ignored);
}

void test_do_procedure_in_existing_program_missing_procedure_raises_catchable_error() {
    // #6465 review (Copilot, P2): a present IN file with no matching
    // procedure must independently exercise the same catchable
    // target-resolution error as a missing file, not the missing-file case
    // alone.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_do_procedure_in_missing_routine";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_text(
        temp_root / "external_proc.prg",
        "PROCEDURE SomeOtherProc\n"
        "RETURN\n"
        "ENDPROC\n");
    write_text(
        temp_root / "main.prg",
        "TRY\n"
        "  DO TargetProc IN external_proc.prg\n"
        "  lReached = .T.\n"
        "CATCH TO oErr\n"
        "  nCaughtErrorNo = oErr.ErrorNo\n"
        "ENDTRY\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((temp_root / "main.prg").string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6465: missing-procedure DO...IN script should complete: " + state.message);

    expect(state.globals.find("lreached") == state.globals.end(),
           "#6465: an IN program present but missing the requested procedure should raise a catchable "
           "error rather than continue");
    expect(state.globals.find("ncaughterrorno") != state.globals.end(),
           "#6465: a missing procedure in an existing DO...IN program should be caught by an "
           "enclosing TRY/CATCH");

    fs::remove_all(temp_root, ignored);
}

void test_do_procedure_in_program_resolves_via_set_path() {
    // #6465 review (Codex, P2): RQ-CF-PRG-040 documents SET PATH resolution
    // for the IN clause; resolve_native_prg_program_path() alone never
    // searches SET PATH, so a program that exists only in a SET PATH
    // directory (not the current default directory) must still resolve.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_do_procedure_in_set_path";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path working_dir = temp_root / "working";
    const fs::path library_dir = temp_root / "library";
    fs::create_directories(working_dir);
    fs::create_directories(library_dir);

    write_text(
        library_dir / "external_proc.prg",
        "PROCEDURE TargetProc\n"
        "LPARAMETERS value\n"
        "do_in_marker = value\n"
        "RETURN\n"
        "ENDPROC\n");
    write_text(
        working_dir / "main.prg",
        "PUBLIC do_in_marker\n"
        "do_in_marker = 0\n"
        "SET PATH TO '" + library_dir.string() + "'\n"
        "DO TargetProc IN external_proc.prg WITH 9\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((working_dir / "main.prg").string(), working_dir.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6465: SET PATH DO...IN script should complete: " + state.message);

    const auto marker = state.globals.find("do_in_marker");
    expect(marker != state.globals.end(), "#6465: SET PATH DO...IN should invoke the named procedure");
    if (marker != state.globals.end()) {
        expect(copperfin::runtime::format_value(marker->second) == "9",
               "#6465: DO...IN should resolve a program found only via SET PATH, not just the default directory");
    }

    fs::remove_all(temp_root, ignored);
}

void test_do_procedure_in_program_accepts_quoted_and_macro_operand() {
    // #6465 review (Copilot, P2): a quoted literal or &macro IN operand was
    // previously treated as raw filesystem text (including the quote
    // characters), so it never resolved.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_do_procedure_in_quoted";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_text(
        temp_root / "external_proc.prg",
        "PROCEDURE TargetProc\n"
        "LPARAMETERS value\n"
        "do_in_marker = value\n"
        "RETURN\n"
        "ENDPROC\n");
    write_text(
        temp_root / "main.prg",
        "PUBLIC do_in_marker, cQuotedResult, cMacroResult\n"
        "do_in_marker = 0\n"
        "DO TargetProc IN 'external_proc.prg' WITH 3\n"
        "cQuotedResult = do_in_marker\n"
        "cProgram = 'external_proc.prg'\n"
        "DO TargetProc IN &cProgram WITH 5\n"
        "cMacroResult = do_in_marker\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((temp_root / "main.prg").string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6465: quoted/macro DO...IN script should complete: " + state.message);

    const auto quoted_result = state.globals.find("cquotedresult");
    expect(quoted_result != state.globals.end(), "#6465: quoted IN operand should invoke the named procedure");
    if (quoted_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(quoted_result->second) == "3",
               "#6465: quoted IN operand should resolve and invoke the named procedure");
    }
    const auto macro_result = state.globals.find("cmacroresult");
    expect(macro_result != state.globals.end(), "#6465: &macro IN operand should invoke the named procedure");
    if (macro_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(macro_result->second) == "5",
               "#6465: &macro IN operand should resolve and invoke the named procedure");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_procedure_macro_off_clears_saved_procedure_state() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_procedure_macro_off";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_text(
        temp_root / "helpers.prg",
        "FUNCTION helper\n"
        "RETURN 7\n");
    write_text(
        temp_root / "main.prg",
        "SET PROCEDURE TO helpers\n"
        "nBefore = helper()\n"
        "cSavedProcedure = SET(\"PROCEDURE\")\n"
        "cSavedProcedure = \"OFF\"\n"
        "SET PROCEDURE TO &cSavedProcedure\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((temp_root / "main.prg").string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "macro-expanded SET PROCEDURE OFF should clear the procedure list");

    const auto before = state.globals.find("nbefore");
    expect(before != state.globals.end(), "SET PROCEDURE macro OFF fixture should call the helper before clearing");
    if (before != state.globals.end()) {
        expect(copperfin::runtime::format_value(before->second) == "7",
               "SET PROCEDURE macro OFF fixture should preserve the pre-clear helper call");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_procedure_registers_external_event_handler() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_procedure_event";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_text(
        temp_root / "helpers.prg",
        "PROCEDURE AppShutdown\n"
        "x = 7\n"
        "RETURN\n");
    write_text(
        temp_root / "main.prg",
        "SET PROCEDURE TO helpers\n"
        "PUBLIC x\n"
        "ACTIVATE POPUP Shortcut\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((temp_root / "main.prg").string(), temp_root.string(), false));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop,
           "SET PROCEDURE event-handler script should pause in the event loop");
    expect(session.dispatch_event_handler("AppShutdown"),
           "event dispatch should resolve a handler from an external SET PROCEDURE file");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop,
           "external event handler should return to the event loop");
    const auto x = state.globals.find("x");
    expect(x != state.globals.end(), "external event handler should be able to set a public variable");
    if (x != state.globals.end()) {
        expect(copperfin::runtime::format_value(x->second) == "7",
               "external event handler should update the caller-visible public variable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_procedure_registers_external_error_handler() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_procedure_error";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_text(
        temp_root / "helpers.prg",
        "PROCEDURE HandleError\n"
        "handled = handled + 1\n"
        "RETURN\n");
    write_text(
        temp_root / "main.prg",
        "SET PROCEDURE TO helpers\n"
        "PUBLIC handled\n"
        "handled = 0\n"
        "ON ERROR DO HandleError\n"
        "result = 1 / 0\n"
        "after_error = 1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((temp_root / "main.prg").string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET PROCEDURE external ON ERROR script should complete: " + state.message);

    const auto handled = state.globals.find("handled");
    expect(handled != state.globals.end(), "external ON ERROR handler should update its public counter");
    if (handled != state.globals.end()) {
        expect(copperfin::runtime::format_value(handled->second) == "1",
               "external ON ERROR handler should run once");
    }
    expect(state.globals.find("after_error") != state.globals.end(),
           "external ON ERROR handler should allow execution to continue");

    fs::remove_all(temp_root, ignored);
}

void test_set_procedure_additive_uses_first_opened_precedence_and_replace_resets_lookup() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_procedure_additive";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_text(
        temp_root / "first.prg",
        "FUNCTION sharedvalue\n"
        "RETURN 'first'\n");
    write_text(
        temp_root / "second.prg",
        "FUNCTION sharedvalue\n"
        "RETURN 'second'\n");
    write_text(
        temp_root / "main.prg",
        "SET PROCEDURE TO first\n"
        "SET PROCEDURE TO second ADDITIVE\n"
        "result_first = sharedvalue()\n"
        "SET PROCEDURE TO second\n"
        "result_second = sharedvalue()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options((temp_root / "main.prg").string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET PROCEDURE ADDITIVE precedence script should complete");

    const auto first = state.globals.find("result_first");
    expect(first != state.globals.end(), "SET PROCEDURE ADDITIVE script should assign first result");
    if (first != state.globals.end()) {
        expect(copperfin::runtime::format_value(first->second) == "first",
               "SET PROCEDURE ADDITIVE should keep first-opened duplicate routine precedence");
    }

    const auto second = state.globals.find("result_second");
    expect(second != state.globals.end(), "SET PROCEDURE replace script should assign second result");
    if (second != state.globals.end()) {
        expect(copperfin::runtime::format_value(second->second) == "second",
               "non-additive SET PROCEDURE should replace the helper lookup list");
    }

    fs::remove_all(temp_root, ignored);
}

void test_loop_predicates_and_bounds_use_heap_backed_expression_checkpoints() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_loop_expression_continuation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path semantics_path = temp_root / "semantics.prg";
    write_text(
        semantics_path,
        "whileCalls = 0\n"
        "whileBody = 0\n"
        "DO WHILE whilePredicate()\n"
        "    whileBody = whileBody + 1\n"
        "ENDDO\n"
        "startCalls = 0\n"
        "endCalls = 0\n"
        "stepCalls = 0\n"
        "forBody = 0\n"
        "FOR forIndex = forStart() TO forEnd() STEP forStep()\n"
        "    forBody = forBody + 1\n"
        "ENDFOR\n"
        "eachCalls = 0\n"
        "eachBody = ''\n"
        "FOR EACH eachItem IN eachCollection()\n"
        "    eachBody = eachItem\n"
        "ENDFOR\n"
        "nestedBody = 0\n"
        "outerCalls = 0\n"
        "DO WHILE outerPredicate()\n"
        "    FOR nestedIndex = 1 TO 2\n"
        "        nestedBody = nestedBody + 1\n"
        "    ENDFOR\n"
        "ENDDO\n"
        "RETURN\n"
        "FUNCTION whilePredicate\n"
        "whileCalls = whileCalls + 1\n"
        "RETURN whilePredicateChild()\n"
        "FUNCTION whilePredicateChild\n"
        "RETURN whileCalls <= 3\n"
        "FUNCTION forStart\n"
        "startCalls = startCalls + 1\n"
        "RETURN forStartChild()\n"
        "FUNCTION forStartChild\n"
        "RETURN 1\n"
        "FUNCTION forEnd\n"
        "endCalls = endCalls + 1\n"
        "RETURN forEndChild()\n"
        "FUNCTION forEndChild\n"
        "RETURN 3\n"
        "FUNCTION forStep\n"
        "stepCalls = stepCalls + 1\n"
        "RETURN forStepChild()\n"
        "FUNCTION forStepChild\n"
        "RETURN 1\n"
        "FUNCTION eachCollection\n"
        "eachCalls = eachCalls + 1\n"
        "RETURN eachCollectionChild()\n"
        "FUNCTION eachCollectionChild\n"
        "RETURN 'value'\n"
        "FUNCTION outerPredicate\n"
        "outerCalls = outerCalls + 1\n"
        "RETURN outerPredicateChild()\n"
        "FUNCTION outerPredicateChild\n"
        "RETURN outerCalls <= 2\n");

    auto semantics_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(semantics_path.string(), temp_root.string(), false));
    const auto semantics_state = semantics_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(semantics_state.completed,
           "loop predicate and bound continuation semantics should complete: " + semantics_state.message);

    const auto expect_global = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = semantics_state.globals.find(name);
        expect(found != semantics_state.globals.end(), name + " should remain visible");
        if (found != semantics_state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };
    expect_global("whilecalls", "4", "DO WHILE should evaluate a nested predicate exactly once per boundary");
    expect_global("whilebody", "3", "DO WHILE should preserve its body count across predicate suspension");
    expect_global("startcalls", "1", "FOR start bounds should execute exactly once");
    expect_global("endcalls", "1", "FOR end bounds should execute exactly once");
    expect_global("stepcalls", "1", "FOR STEP bounds should execute exactly once");
    expect_global("forbody", "3", "FOR should use resumed start/end/step values for its iterations");
    expect_global("eachcalls", "1", "FOR EACH collection expressions should execute exactly once");
    expect_global("eachbody", "value", "FOR EACH should assign a resumed collection result to its loop variable");
    expect_global("outercalls", "3", "nested loop predicates should be evaluated at each outer-loop boundary");
    expect_global("nestedbody", "4", "nested FOR loops should preserve their loop state after outer predicate suspension");

    const fs::path resume_path = temp_root / "resume.prg";
    write_text(
        resume_path,
        "resumeCalls = 0\n"
        "resumeBody = 0\n"
        "ON ERROR DO handleResume\n"
        "DO WHILE resumePredicate()\n"
        "    resumeBody = resumeBody + 1\n"
        "ENDDO\n"
        "afterResume = 1\n"
        "RETURN\n"
        "FUNCTION resumePredicate\n"
        "RETURN resumeChild() + 1 / 0\n"
        "FUNCTION resumeChild\n"
        "RETURN 1\n"
        "PROCEDURE handleResume\n"
        "resumeCalls = resumeCalls + 1\n"
        "RESUME\n"
        "RETURN\n");
    auto resume_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(resume_path.string(), temp_root.string(), false));
    const auto resume_state = resume_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(resume_state.completed, "RESUME should abandon a faulting loop predicate: " + resume_state.message);
    expect(resume_state.globals.find("resumebody") != resume_state.globals.end() &&
               copperfin::runtime::format_value(resume_state.globals.at("resumebody")) == "0",
           "RESUME should not execute a loop body after a predicate fault");
    expect(resume_state.globals.find("afterresume") != resume_state.globals.end(),
           "RESUME should continue after a faulting loop predicate");

    const fs::path retry_path = temp_root / "retry.prg";
    write_text(
        retry_path,
        "PUBLIC handlerCalls\n"
        "handlerCalls = 0\n"
        "predicateCalls = 0\n"
        "retryBody = 0\n"
        "ON ERROR DO handleRetry\n"
        "DO WHILE retryPredicate()\n"
        "    retryBody = retryBody + 1\n"
        "ENDDO\n"
        "afterRetry = 1\n"
        "RETURN\n"
        "FUNCTION retryPredicate\n"
        "predicateCalls = predicateCalls + 1\n"
        "RETURN retryChild() + 1 / 0\n"
        "FUNCTION retryChild\n"
        "RETURN 1\n"
        "PROCEDURE handleRetry\n"
        "handlerCalls = handlerCalls + 1\n"
        "IF handlerCalls < 2\n"
        "    RETRY\n"
        "ENDIF\n"
        "RETURN\n");
    auto retry_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(retry_path.string(), temp_root.string(), false));
    const auto retry_state = retry_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(retry_state.completed, "RETRY should re-execute a faulting loop predicate: " + retry_state.message);
    expect(retry_state.globals.find("retrybody") != retry_state.globals.end() &&
               copperfin::runtime::format_value(retry_state.globals.at("retrybody")) == "0",
           "RETRY should not execute a loop body while the predicate remains faulty");
    expect(retry_state.globals.find("afterretry") != retry_state.globals.end(),
           "RETRY should continue after the loop predicate handler returns");
    expect(retry_state.globals.find("handlercalls") != retry_state.globals.end() &&
               copperfin::runtime::format_value(retry_state.globals.at("handlercalls")) == "2",
           "RETRY should invoke the loop predicate handler twice");
    expect(retry_state.globals.find("predicatecalls") != retry_state.globals.end() &&
               copperfin::runtime::format_value(retry_state.globals.at("predicatecalls")) == "1",
           "RETRY should not replay completed loop predicate side effects");

    const fs::path inline_resume_path = temp_root / "inline_loop_resume.prg";
    write_text(
        inline_resume_path,
        "inlineResumeBody = 0\n"
        "inlineResumeChildCalls = 0\n"
        "ON ERROR DO handleInlineResume\n"
        "DO WHILE inlineResumeChild() + 1 / 0\n"
        "    inlineResumeBody = inlineResumeBody + 1\n"
        "ENDDO\n"
        "afterInlineResume = 1\n"
        "RETURN\n"
        "FUNCTION inlineResumeChild\n"
        "inlineResumeChildCalls = inlineResumeChildCalls + 1\n"
        "RETURN .F.\n"
        "PROCEDURE handleInlineResume\n"
        "RESUME\n"
        "RETURN\n");
    auto inline_resume_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(inline_resume_path.string(), temp_root.string(), false));
    const auto inline_resume_state = inline_resume_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(inline_resume_state.completed,
           "RESUME should skip a same-frame faulting loop predicate: " + inline_resume_state.message);
    expect(inline_resume_state.globals.find("inlineresumebody") != inline_resume_state.globals.end() &&
               copperfin::runtime::format_value(inline_resume_state.globals.at("inlineresumebody")) == "0",
           "same-frame RESUME should not execute a loop body after a predicate fault");
    expect(inline_resume_state.globals.find("afterinlineresume") != inline_resume_state.globals.end(),
           "same-frame RESUME should continue after a faulting loop predicate");
    expect(inline_resume_state.globals.find("inlineresumechildcalls") != inline_resume_state.globals.end() &&
               copperfin::runtime::format_value(inline_resume_state.globals.at("inlineresumechildcalls")) == "1",
           "same-frame RESUME should not replay a completed predicate call");

    const fs::path inline_for_resume_path = temp_root / "inline_for_loop_resume.prg";
    write_text(
        inline_for_resume_path,
        "inlineForBody = 0\n"
        "inlineForChildCalls = 0\n"
        "ON ERROR DO handleInlineForResume\n"
        "FOR inlineForIndex = 1 TO inlineForChild() + 1 / 0\n"
        "    inlineForBody = inlineForBody + 1\n"
        "ENDFOR\n"
        "afterInlineForResume = 1\n"
        "RETURN\n"
        "FUNCTION inlineForChild\n"
        "inlineForChildCalls = inlineForChildCalls + 1\n"
        "RETURN 1\n"
        "PROCEDURE handleInlineForResume\n"
        "RESUME\n"
        "RETURN\n");
    auto inline_for_resume_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(inline_for_resume_path.string(), temp_root.string(), false));
    const auto inline_for_resume_state =
        inline_for_resume_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(inline_for_resume_state.completed,
           "RESUME should skip a same-frame faulting FOR bound: " + inline_for_resume_state.message);
    expect(inline_for_resume_state.globals.find("inlineforbody") != inline_for_resume_state.globals.end() &&
               copperfin::runtime::format_value(inline_for_resume_state.globals.at("inlineforbody")) == "0" &&
               inline_for_resume_state.globals.find("afterinlineforresume") != inline_for_resume_state.globals.end(),
           "same-frame RESUME should skip a FOR body after a bound fault");
    expect(inline_for_resume_state.globals.find("inlineforchildcalls") != inline_for_resume_state.globals.end() &&
               copperfin::runtime::format_value(inline_for_resume_state.globals.at("inlineforchildcalls")) == "1",
           "same-frame FOR RESUME should not replay a completed bound call");

    const fs::path inline_each_resume_path = temp_root / "inline_for_each_resume.prg";
    write_text(
        inline_each_resume_path,
        "inlineEachBody = 0\n"
        "inlineEachChildCalls = 0\n"
        "ON ERROR DO handleInlineEachResume\n"
        "FOR EACH inlineEachItem IN inlineEachChild() + 1 / 0\n"
        "    inlineEachBody = inlineEachBody + 1\n"
        "ENDFOR\n"
        "afterInlineEachResume = 1\n"
        "RETURN\n"
        "FUNCTION inlineEachChild\n"
        "inlineEachChildCalls = inlineEachChildCalls + 1\n"
        "RETURN 1\n"
        "PROCEDURE handleInlineEachResume\n"
        "RESUME\n"
        "RETURN\n");
    auto inline_each_resume_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(inline_each_resume_path.string(), temp_root.string(), false));
    const auto inline_each_resume_state =
        inline_each_resume_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(inline_each_resume_state.completed,
           "RESUME should skip a same-frame faulting FOR EACH collection: " + inline_each_resume_state.message);
    expect(inline_each_resume_state.globals.find("inlineeachbody") != inline_each_resume_state.globals.end() &&
               copperfin::runtime::format_value(inline_each_resume_state.globals.at("inlineeachbody")) == "0" &&
               inline_each_resume_state.globals.find("afterinlineeachresume") != inline_each_resume_state.globals.end(),
           "same-frame RESUME should skip a FOR EACH body after a collection fault");
    expect(inline_each_resume_state.globals.find("inlineeachchildcalls") != inline_each_resume_state.globals.end() &&
               copperfin::runtime::format_value(inline_each_resume_state.globals.at("inlineeachchildcalls")) == "1",
           "same-frame FOR EACH RESUME should not replay a completed collection call");

    const fs::path inline_retry_path = temp_root / "inline_loop_retry.prg";
    write_text(
        inline_retry_path,
        "inlineRetryBody = 0\n"
        "inlineRetryChildCalls = 0\n"
        "inlineRetryHandlers = 0\n"
        "ON ERROR DO handleInlineRetry\n"
        "DO WHILE inlineRetryChild() + 1 / 0\n"
        "    inlineRetryBody = inlineRetryBody + 1\n"
        "ENDDO\n"
        "afterInlineRetry = 1\n"
        "RETURN\n"
        "FUNCTION inlineRetryChild\n"
        "inlineRetryChildCalls = inlineRetryChildCalls + 1\n"
        "RETURN .F.\n"
        "PROCEDURE handleInlineRetry\n"
        "inlineRetryHandlers = inlineRetryHandlers + 1\n"
        "IF inlineRetryHandlers = 1\n"
        "    RETRY\n"
        "ELSE\n"
        "    RESUME\n"
        "ENDIF\n"
        "RETURN\n");
    auto inline_retry_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(inline_retry_path.string(), temp_root.string(), false));
    const auto inline_retry_state = inline_retry_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(inline_retry_state.completed,
           "RETRY should re-enter a same-frame faulting loop predicate: " + inline_retry_state.message);
    expect(inline_retry_state.globals.find("inlineretrybody") != inline_retry_state.globals.end() &&
               copperfin::runtime::format_value(inline_retry_state.globals.at("inlineretrybody")) == "0",
           "same-frame RETRY should not execute a loop body while the predicate remains faulty");
    expect(inline_retry_state.globals.find("afterinlineretry") != inline_retry_state.globals.end(),
           "same-frame RETRY should continue after the loop predicate handler returns");
    expect(inline_retry_state.globals.find("inlineretrychildcalls") != inline_retry_state.globals.end() &&
               copperfin::runtime::format_value(inline_retry_state.globals.at("inlineretrychildcalls")) == "2",
           "same-frame RETRY should re-evaluate the faulting loop predicate");

    const fs::path loop_debugger_path = temp_root / "loop_debugger.prg";
    write_text(
        loop_debugger_path,
        "DO WHILE loopDebuggerChild()\n"
        "    loopDebuggerBody = 1\n"
        "ENDDO\n"
        "afterLoopDebugger = 1\n"
        "RETURN\n"
        "FUNCTION loopDebuggerChild\n"
        "loopDebuggerChildCalls = loopDebuggerChildCalls + 1\n"
        "RETURN .F.\n");
    auto loop_baseline_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(loop_debugger_path.string(), temp_root.string(), false));
    const auto loop_baseline_state = loop_baseline_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(loop_baseline_state.completed, "loop debugger baseline should complete: " + loop_baseline_state.message);

    auto loop_exact_budget_options = make_runtime_session_options(loop_debugger_path.string(), temp_root.string(), false);
    loop_exact_budget_options.max_executed_statements = loop_baseline_state.executed_statement_count;
    auto loop_exact_budget_session = copperfin::runtime::PrgRuntimeSession::create(loop_exact_budget_options);
    const auto loop_exact_budget_state = loop_exact_budget_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(loop_exact_budget_state.completed &&
               loop_exact_budget_state.executed_statement_count == loop_baseline_state.executed_statement_count,
           "a resumed loop predicate should not consume a duplicate statement-budget slot");

    auto loop_exhausted_budget_options = make_runtime_session_options(loop_debugger_path.string(), temp_root.string(), false);
    loop_exhausted_budget_options.max_executed_statements = loop_baseline_state.executed_statement_count - 1U;
    auto loop_exhausted_budget_session = copperfin::runtime::PrgRuntimeSession::create(loop_exhausted_budget_options);
    const auto loop_exhausted_budget_state =
        loop_exhausted_budget_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(loop_exhausted_budget_state.reason == copperfin::runtime::DebugPauseReason::error &&
               loop_exhausted_budget_state.executed_statement_count == loop_baseline_state.executed_statement_count - 1U,
           "loop predicate budget exhaustion should occur at the physical statement boundary");

    auto loop_breakpoint_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(loop_debugger_path.string(), temp_root.string(), false));
    loop_breakpoint_session.add_breakpoint({.file_path = loop_debugger_path.string(), .line = 1U});
    const auto loop_breakpoint_state = loop_breakpoint_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(loop_breakpoint_state.reason == copperfin::runtime::DebugPauseReason::breakpoint &&
               loop_breakpoint_state.location.line == 1U,
           "a DO WHILE predicate should honor its breakpoint before UDF evaluation");
    const auto loop_breakpoint_completed_state =
        loop_breakpoint_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(loop_breakpoint_completed_state.completed,
           "a resumed DO WHILE predicate should complete after its breakpoint");
    expect(std::count_if(
               loop_breakpoint_completed_state.events.begin(),
               loop_breakpoint_completed_state.events.end(),
               [](const copperfin::runtime::RuntimeEvent &event) {
                   return event.category == "execute" && event.location.line == 1U;
               }) == 1,
           "a DO WHILE predicate should emit one execute event for its physical statement");

    const fs::path loop_cancel_path = temp_root / "loop_cancel_predicate.prg";
    write_text(
        loop_cancel_path,
        "DO WHILE loopCancelPredicate()\n"
        "    loopCancelBody = 1\n"
        "ENDDO\n"
        "afterLoopCancel = 1\n"
        "RETURN\n"
        "FUNCTION loopCancelPredicate\n"
        "CANCEL\n"
        "RETURN .T.\n");
    auto loop_cancel_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(loop_cancel_path.string(), temp_root.string(), false));
    const auto loop_cancel_state = loop_cancel_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(loop_cancel_state.completed && loop_cancel_state.globals.find("loopcancelbody") == loop_cancel_state.globals.end() &&
               loop_cancel_state.globals.find("afterloopcancel") == loop_cancel_state.globals.end() &&
               std::count_if(
                   loop_cancel_state.events.begin(),
                   loop_cancel_state.events.end(),
                   [](const copperfin::runtime::RuntimeEvent &event) {
                       return event.category == "runtime.cancel";
                   }) == 1,
           "CANCEL in a loop predicate should abandon the pending loop and preserve its runtime event");

    const fs::path loop_quit_path = temp_root / "loop_quit_predicate.prg";
    write_text(
        loop_quit_path,
        "DO WHILE loopQuitPredicate()\n"
        "    loopQuitBody = 1\n"
        "ENDDO\n"
        "afterLoopQuit = 1\n"
        "RETURN\n"
        "FUNCTION loopQuitPredicate\n"
        "QUIT\n"
        "RETURN .T.\n");
    auto loop_quit_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(loop_quit_path.string(), temp_root.string(), false));
    const auto loop_quit_state = loop_quit_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(loop_quit_state.completed && loop_quit_state.globals.find("loopquitbody") == loop_quit_state.globals.end() &&
               loop_quit_state.globals.find("afterloopquit") == loop_quit_state.globals.end() &&
               std::count_if(
                   loop_quit_state.events.begin(),
                   loop_quit_state.events.end(),
                   [](const copperfin::runtime::RuntimeEvent &event) {
                       return event.category == "runtime.quit";
                   }) == 1,
           "QUIT in a loop predicate should abandon the pending loop and preserve its runtime event");

    const fs::path try_path = temp_root / "try.prg";
    write_text(
        try_path,
        "caughtCount = 0\n"
        "finallyCalls = 0\n"
        "TRY\n"
        "    DO WHILE tryPredicate()\n"
        "        badBody = 1\n"
        "    ENDDO\n"
        "CATCH TO errorText\n"
        "    caughtCount = caughtCount + 1\n"
        "FINALLY\n"
        "    finallyCalls = finallyCalls + 1\n"
        "ENDTRY\n"
        "afterTry = 1\n"
        "cleanupBranch = ''\n"
        "DO WHILE cleanupPredicate()\n"
        "    cleanupBranch = 'body'\n"
        "ENDDO\n"
        "RETURN\n"
        "FUNCTION tryPredicate\n"
        "RETURN tryChild() + 1 / 0\n"
        "FUNCTION tryChild\n"
        "RETURN 1\n"
        "FUNCTION cleanupPredicate\n"
        "RETURN .F.\n");
    auto try_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(try_path.string(), temp_root.string(), false));
    const auto try_state = try_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(try_state.completed, "TRY/CATCH/FINALLY should handle a loop predicate fault: " + try_state.message);
    expect(try_state.globals.find("badbody") == try_state.globals.end(),
           "a faulting loop predicate should not execute its body");
    expect(try_state.globals.find("caughtcount") != try_state.globals.end() &&
               copperfin::runtime::format_value(try_state.globals.at("caughtcount")) == "1",
           "CATCH should run once for a loop predicate fault");
    expect(try_state.globals.find("finallycalls") != try_state.globals.end() &&
               copperfin::runtime::format_value(try_state.globals.at("finallycalls")) == "1",
           "FINALLY should run once for a loop predicate fault");
    expect(try_state.globals.find("aftertry") != try_state.globals.end(),
           "execution should continue after handling a loop predicate fault");
    expect(try_state.globals.find("cleanupbranch") != try_state.globals.end(),
           "a loop after TRY should be able to start after loop predicate cleanup");

    const fs::path scan_table_path = temp_root / "scan_people.dbf";
    write_people_dbf(scan_table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});
    const fs::path scan_path = temp_root / "scan_predicates.prg";
    write_text(
        scan_path,
        "USE '" + scan_table_path.string() + "' ALIAS ScanPeople IN 0\n"
        "SET FILTER TO scanCursorFilter()\n"
        "GO TOP IN ScanPeople\n"
        "scanFilterCalls = 0\n"
        "scanForCalls = 0\n"
        "scanWhileCalls = 0\n"
        "scanHits = 0\n"
        "scanNames = ''\n"
        "SCAN FOR scanForPredicate() WHILE scanWhilePredicate() IN ScanPeople\n"
        "    scanHits = scanHits + 1\n"
        "    scanNames = scanNames + NAME\n"
        "ENDSCAN\n"
        "afterScan = 1\n"
        "RETURN\n"
        "FUNCTION scanCursorFilter\n"
        "scanFilterCalls = scanFilterCalls + 1\n"
        "RETURN AGE >= 20\n"
        "FUNCTION scanForPredicate\n"
        "scanForCalls = scanForCalls + 1\n"
        "RETURN AGE >= 20\n"
        "FUNCTION scanWhilePredicate\n"
        "scanWhileCalls = scanWhileCalls + 1\n"
        "RETURN .T.\n");
    auto scan_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(scan_path.string(), temp_root.string(), false));
    const auto scan_state = scan_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(scan_state.completed, "SCAN predicates should complete through resumable cursor search: " + scan_state.message);
    expect(scan_state.globals.find("scanhits") != scan_state.globals.end() &&
               copperfin::runtime::format_value(scan_state.globals.at("scanhits")) == "3",
           "SCAN FOR should execute its body for each visible matching record");
    expect(scan_state.globals.find("scannames") != scan_state.globals.end() &&
               copperfin::runtime::format_value(scan_state.globals.at("scannames")) == "BRAVOCHARLIEDELTA",
           "SCAN should preserve targeted cursor order across suspended filters");
    expect(scan_state.globals.find("scanfiltercalls") != scan_state.globals.end() &&
               copperfin::runtime::format_value(scan_state.globals.at("scanfiltercalls")) == "3" &&
               scan_state.globals.find("scanforcalls") != scan_state.globals.end() &&
               copperfin::runtime::format_value(scan_state.globals.at("scanforcalls")) == "3" &&
               scan_state.globals.find("scanwhilecalls") != scan_state.globals.end() &&
               copperfin::runtime::format_value(scan_state.globals.at("scanwhilecalls")) == "3",
           "SCAN filter, FOR, and WHILE user routines should run once per examined record");
    expect(scan_state.globals.find("afterscan") != scan_state.globals.end(),
           "execution should continue after resumable SCAN completion");
    expect(std::any_of(
               scan_state.events.begin(),
               scan_state.events.end(),
               [](const copperfin::runtime::RuntimeEvent &event) {
                   return event.category == "runtime.rushmore" &&
                          event.detail.find("resumable scan filter") != std::string::npos;
               }),
           "resumable SCAN search should record its explicit linear fallback");

    const fs::path scan_control_path = temp_root / "scan_control.prg";
    write_text(
        scan_control_path,
        "USE '" + scan_table_path.string() + "' ALIAS ScanControlPeople IN 0\n"
        "scanControlHits = 0\n"
        "SCAN FOR scanControlPredicate() IN ScanControlPeople\n"
        "    IF NAME = 'BRAVO'\n"
        "        LOOP\n"
        "    ENDIF\n"
        "    scanControlHits = scanControlHits + 1\n"
        "    IF NAME = 'CHARLIE'\n"
        "        EXIT\n"
        "    ENDIF\n"
        "ENDSCAN\n"
        "afterScanControl = 1\n"
        "RETURN\n"
        "FUNCTION scanControlPredicate\n"
        "RETURN AGE >= 20\n");
    auto scan_control_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(scan_control_path.string(), temp_root.string(), false));
    const auto scan_control_state =
        scan_control_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(scan_control_state.completed,
           "LOOP and EXIT should complete from a predicate-bearing SCAN: " + scan_control_state.message);
    expect(scan_control_state.globals.find("scancontrolhits") != scan_control_state.globals.end() &&
               copperfin::runtime::format_value(scan_control_state.globals.at("scancontrolhits")) == "1" &&
               scan_control_state.globals.find("afterscancontrol") != scan_control_state.globals.end(),
           "predicate-bearing SCAN should preserve LOOP/EXIT control flow");

    const fs::path scan_resume_path = temp_root / "scan_resume.prg";
    write_text(
        scan_resume_path,
        "USE '" + scan_table_path.string() + "' ALIAS ScanResumePeople IN 0\n"
        "GO TOP IN ScanResumePeople\n"
        "scanResumeBody = 0\n"
        "ON ERROR DO handleScanResume\n"
        "SCAN FOR scanResumeChild() + 1 / 0 IN ScanResumePeople\n"
        "    scanResumeBody = scanResumeBody + 1\n"
        "ENDSCAN\n"
        "afterScanResume = 1\n"
        "RETURN\n"
        "FUNCTION scanResumeChild\n"
        "RETURN .T.\n"
        "PROCEDURE handleScanResume\n"
        "RESUME\n"
        "RETURN\n");
    auto scan_resume_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(scan_resume_path.string(), temp_root.string(), false));
    const auto scan_resume_state =
        scan_resume_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(scan_resume_state.completed,
           "RESUME should abandon a faulting SCAN predicate: " + scan_resume_state.message);
    expect(scan_resume_state.globals.find("scanresumebody") != scan_resume_state.globals.end() &&
               copperfin::runtime::format_value(scan_resume_state.globals.at("scanresumebody")) == "0" &&
               scan_resume_state.globals.find("afterscanresume") != scan_resume_state.globals.end(),
           "RESUME should skip a SCAN body after a predicate fault");

    const fs::path scan_depth_path = temp_root / "scan_depth.prg";
    write_text(
        scan_depth_path,
        "USE '" + scan_table_path.string() + "' ALIAS ScanDepthPeople IN 0\n"
        "SCAN FOR scanDepthPredicate() IN ScanDepthPeople\n"
        "    scanDepthBody = 1\n"
        "ENDSCAN\n"
        "afterScanDepth = 1\n"
        "RETURN\n"
        "FUNCTION scanDepthPredicate\n"
        "RETURN scanDepthPredicate()\n");
    auto scan_depth_options = make_runtime_session_options(scan_depth_path.string(), temp_root.string(), false);
    scan_depth_options.max_call_depth = 96U;
    auto scan_depth_session = copperfin::runtime::PrgRuntimeSession::create(scan_depth_options);
    const auto scan_depth_state =
        scan_depth_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(scan_depth_state.reason == copperfin::runtime::DebugPauseReason::error &&
               scan_depth_state.message.find("maximum call depth") != std::string::npos &&
               scan_depth_state.globals.find("scandepthbody") == scan_depth_state.globals.end() &&
               scan_depth_state.globals.find("afterscandepth") == scan_depth_state.globals.end(),
           "recursive SCAN predicates should stop at max_call_depth without entering the body");

    const fs::path depth_path = temp_root / "depth.prg";
    write_text(
        depth_path,
        "DO WHILE deepPredicate()\n"
        "ENDDO\n"
        "RETURN\n"
        "FUNCTION deepPredicate\n"
        "RETURN deepPredicate()\n");
    auto depth_options = make_runtime_session_options(depth_path.string(), temp_root.string(), false);
    depth_options.max_call_depth = 96U;
    auto depth_session = copperfin::runtime::PrgRuntimeSession::create(depth_options);
    const auto depth_state = depth_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(depth_state.reason == copperfin::runtime::DebugPauseReason::error,
           "a deep loop predicate should stop at the runtime call-depth guardrail");
    expect(depth_state.message.find("maximum call depth") != std::string::npos,
           "a deep loop predicate should report max_call_depth without exhausting the native stack");

    fs::remove_all(temp_root, ignored);
}

void test_scan_predicate_preserves_rest_scope_and_exhaustion_state() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_scan_rest_boundaries";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    const fs::path script_path = temp_root / "scan_rest_boundaries.prg";
    write_text(
        script_path,
        "USE '" + table_path.string() + "' ALIAS ScanBoundary IN 0\n"
        "GO 2 IN ScanBoundary\n"
        "scanRestNames = ''\n"
        "SCAN REST FOR scanBoundaryMatch() IN ScanBoundary\n"
        "    scanRestNames = scanRestNames + NAME\n"
        "ENDSCAN\n"
        "scanRestEof = EOF()\n"
        "scanRestRecno = RECNO()\n"
        "GO TOP IN ScanBoundary\n"
        "SCAN FOR scanBoundaryMiss() IN ScanBoundary\n"
        "ENDSCAN\n"
        "scanMissEof = EOF()\n"
        "scanMissRecno = RECNO()\n"
        "RETURN\n"
        "FUNCTION scanBoundaryMatch\n"
        "RETURN .T.\n"
        "FUNCTION scanBoundaryMiss\n"
        "RETURN .F.\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(script_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SCAN REST boundary script should complete: " + state.message);
    expect(state.globals.find("scanrestnames") != state.globals.end() &&
               copperfin::runtime::format_value(state.globals.at("scanrestnames")) == "BRAVOCHARLIE",
           "SCAN REST should begin at the current record and preserve later records");
    expect(state.globals.find("scanresteof") != state.globals.end() &&
               copperfin::runtime::format_value(state.globals.at("scanresteof")) == "true" &&
               state.globals.find("scanrestrecno") != state.globals.end() &&
               copperfin::runtime::format_value(state.globals.at("scanrestrecno")) == "4",
           "exhausted SCAN REST should leave the cursor at EOF");
    expect(state.globals.find("scanmisseof") != state.globals.end() &&
               copperfin::runtime::format_value(state.globals.at("scanmisseof")) == "true" &&
               state.globals.find("scanmissrecno") != state.globals.end() &&
               copperfin::runtime::format_value(state.globals.at("scanmissrecno")) == "4",
           "a resumable SCAN predicate with no match should leave the cursor at EOF");

    fs::remove_all(temp_root, ignored);
}

void test_cancel_releases_frame_owned_native_objects() {
    // #6445: CANCEL previously unwound the call stack with a manual
    // restore_private_declarations() + stack.pop_back() loop that bypassed
    // pop_frame()'s release_frame_object_bindings() call. A LOCAL native
    // object created in a cancelled frame was therefore abandoned in
    // session-owned state with Destroy never called and its resources
    // never released, even though every VFP variable that could reach it
    // was gone (installed VFP9 SP2's CANCEL help says CANCEL releases all
    // private variables; releasing an object-bearing binding must not
    // suppress that object's normal destruction cleanup). This matches
    // the normal-return path's own behavior, which already calls Destroy
    // correctly.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_cancel_object_cleanup";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path script_path = temp_root / "cancel_object_cleanup.prg";
    write_text(
        script_path,
        "DO InnerCancel\n"
        "RETURN\n"
        "PROCEDURE InnerCancel\n"
        "LOCAL oLocal\n"
        "oLocal = CREATEOBJECT('CleanupProbe')\n"
        "CANCEL\n"
        "ENDPROC\n"
        "DEFINE CLASS CleanupProbe AS Custom\n"
        "PROCEDURE Destroy\n"
        "PUBLIC cancel_destroy_called\n"
        "cancel_destroy_called = .T.\n"
        "ENDPROC\n"
        "ENDDEFINE\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(script_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6445: CANCEL object-cleanup script should complete: " + state.message);

    expect(std::any_of(
               state.events.begin(),
               state.events.end(),
               [](const copperfin::runtime::RuntimeEvent &event) {
                   return event.category == "prg.object.destroy";
               }),
           "#6445: CANCEL should call Destroy on a frame-owned local object before unwinding it");
    expect(std::any_of(
               state.events.begin(),
               state.events.end(),
               [](const copperfin::runtime::RuntimeEvent &event) {
                   return event.category == "prg.object.release";
               }),
           "#6445: CANCEL should release a frame-owned local object's native resources");
    expect(state.globals.find("cancel_destroy_called") != state.globals.end() &&
               copperfin::runtime::format_value(state.globals.at("cancel_destroy_called")) == "true",
           "#6445: the cancelled object's own Destroy method should have run");

    fs::remove_all(temp_root, ignored);
}

void test_cancel_releases_frame_owned_private_native_objects() {
    // #6445 review fix: release_frame_object_bindings() only scanned
    // frame.locals/local_arrays, but a PRIVATE-declared object lives
    // directly in `globals` for the lifetime of the declaring frame (only
    // the shadowed prior value is kept in frame.private_saved_values, for
    // restore_private_declarations() to put back afterward). This gap
    // applied equally to an ordinary frame return, not just CANCEL, but
    // was only caught here because the #6445 fix and its own regression
    // test covered LOCAL, not PRIVATE.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_cancel_private_object_cleanup";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path script_path = temp_root / "cancel_private_object_cleanup.prg";
    write_text(
        script_path,
        "DO InnerCancel\n"
        "RETURN\n"
        "PROCEDURE InnerCancel\n"
        "PRIVATE oPrivate\n"
        "oPrivate = CREATEOBJECT('PrivateCleanupProbe')\n"
        "CANCEL\n"
        "ENDPROC\n"
        "DEFINE CLASS PrivateCleanupProbe AS Custom\n"
        "PROCEDURE Destroy\n"
        "PUBLIC private_cancel_destroy_called\n"
        "private_cancel_destroy_called = .T.\n"
        "ENDPROC\n"
        "ENDDEFINE\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(script_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6445: CANCEL PRIVATE-object-cleanup script should complete: " + state.message);

    expect(std::any_of(
               state.events.begin(),
               state.events.end(),
               [](const copperfin::runtime::RuntimeEvent &event) {
                   return event.category == "prg.object.destroy";
               }),
           "#6445: CANCEL should call Destroy on a frame-owned PRIVATE object before unwinding it");
    expect(std::any_of(
               state.events.begin(),
               state.events.end(),
               [](const copperfin::runtime::RuntimeEvent &event) {
                   return event.category == "prg.object.release";
               }),
           "#6445: CANCEL should release a frame-owned PRIVATE object's native resources");
    expect(state.globals.find("private_cancel_destroy_called") != state.globals.end() &&
               copperfin::runtime::format_value(state.globals.at("private_cancel_destroy_called")) == "true",
           "#6445: the cancelled PRIVATE object's own Destroy method should have run");

    fs::remove_all(temp_root, ignored);
}

void test_retry_releases_frame_owned_native_objects() {
    // #6446: RETRY previously unwound intervening frames back to the
    // saved fault frame with a manual restore_private_declarations() +
    // stack.pop_back() loop that bypassed pop_frame()'s
    // release_frame_object_bindings() call. A LOCAL native object created
    // in an ON ERROR handler that then successfully RETRYs (after making
    // the failed operation viable) was therefore abandoned in
    // session-owned state with Destroy never called and its resources
    // never released, even though every VFP variable that could reach it
    // was gone. Matches the issue's own trigger: the handler copies a
    // prepared .prg into the missing DO target, then RETRYs.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_retry_object_cleanup";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    // DO retry_target resolving to a standalone retry_target.prg file runs
    // that file's own top-level body (not a same-named PROCEDURE inside
    // it), so the prepared file must be plain top-level code.
    write_text(
        temp_root / "prepared_retry.prg",
        "retry_target_ran = .T.\n");

    const fs::path script_path = temp_root / "retry_object_cleanup.prg";
    write_text(
        script_path,
        "PUBLIC retry_target_ran\n"
        "ON ERROR DO ErrorHandler\n"
        "DO retry_target\n"
        "after_retry = .T.\n"
        "RETURN\n"
        "PROCEDURE ErrorHandler\n"
        "LOCAL oLocal\n"
        "oLocal = CREATEOBJECT('RetryCleanupProbe')\n"
        "COPY FILE 'prepared_retry.prg' TO 'retry_target.prg'\n"
        "RETRY\n"
        "ENDPROC\n"
        "DEFINE CLASS RetryCleanupProbe AS Custom\n"
        "PROCEDURE Destroy\n"
        "PUBLIC retry_destroy_called\n"
        "retry_destroy_called = .T.\n"
        "ENDPROC\n"
        "ENDDEFINE\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(script_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6446: RETRY object-cleanup script should complete: " + state.message);

    expect(state.globals.find("retry_target_ran") != state.globals.end() &&
               copperfin::runtime::format_value(state.globals.at("retry_target_ran")) == "true",
           "#6446: RETRY should successfully re-execute the faulting DO after recovery");
    expect(state.globals.find("after_retry") != state.globals.end() &&
               copperfin::runtime::format_value(state.globals.at("after_retry")) == "true",
           "#6446: the program should continue normally after a successful RETRY");

    expect(std::any_of(
               state.events.begin(),
               state.events.end(),
               [](const copperfin::runtime::RuntimeEvent &event) {
                   return event.category == "prg.object.destroy";
               }),
           "#6446: RETRY should call Destroy on the error handler's frame-owned local object");
    expect(std::any_of(
               state.events.begin(),
               state.events.end(),
               [](const copperfin::runtime::RuntimeEvent &event) {
                   return event.category == "prg.object.release";
               }),
           "#6446: RETRY should release the error handler's frame-owned local object's native resources");
    expect(state.globals.find("retry_destroy_called") != state.globals.end() &&
               copperfin::runtime::format_value(state.globals.at("retry_destroy_called")) == "true",
           "#6446: the unwound error handler's object Destroy method should have run");

    fs::remove_all(temp_root, ignored);
}

void test_resume_releases_frame_owned_native_objects() {
    // #6446: RESUME's fault-frame unwind loop had the identical bug as
    // RETRY's (same manual restore_private_declarations() +
    // stack.pop_back() bypass of release_frame_object_bindings()). The ON
    // ERROR handler's own frame -- created deeper than the faulting
    // master frame -- owns a local object; when RESUME continues after
    // the original faulting statement, the handler's own frame is the
    // one unwound, and its local object must be released the same way a
    // normal return from ErrorHandler would release it.
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_resume_object_cleanup";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path script_path = temp_root / "resume_object_cleanup.prg";
    write_text(
        script_path,
        "ON ERROR DO ErrorHandler\n"
        "DO NoSuchProcedureXyz\n"
        "after_resume = .T.\n"
        "RETURN\n"
        "PROCEDURE ErrorHandler\n"
        "LOCAL oLocal\n"
        "oLocal = CREATEOBJECT('ResumeCleanupProbe')\n"
        "RESUME\n"
        "ENDPROC\n"
        "DEFINE CLASS ResumeCleanupProbe AS Custom\n"
        "PROCEDURE Destroy\n"
        "PUBLIC resume_destroy_called\n"
        "resume_destroy_called = .T.\n"
        "ENDPROC\n"
        "ENDDEFINE\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(script_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6446: RESUME object-cleanup script should complete: " + state.message);

    expect(state.globals.find("after_resume") != state.globals.end() &&
               copperfin::runtime::format_value(state.globals.at("after_resume")) == "true",
           "#6446: the program should continue normally after RESUME");

    expect(std::any_of(
               state.events.begin(),
               state.events.end(),
               [](const copperfin::runtime::RuntimeEvent &event) {
                   return event.category == "prg.object.destroy";
               }),
           "#6446: RESUME should call Destroy on the error handler's frame-owned local object");
    expect(std::any_of(
               state.events.begin(),
               state.events.end(),
               [](const copperfin::runtime::RuntimeEvent &event) {
                   return event.category == "prg.object.release";
               }),
           "#6446: RESUME should release the error handler's frame-owned local object's native resources");
    expect(state.globals.find("resume_destroy_called") != state.globals.end() &&
               copperfin::runtime::format_value(state.globals.at("resume_destroy_called")) == "true",
           "#6446: the unwound error handler's object Destroy method should have run");

    fs::remove_all(temp_root, ignored);
}


// #6331: SCAN used to resume by bare work-area number. A callback that closes
// the scanned cursor and opens another one reuses that number immediately,
// so the loop continued on the unrelated replacement cursor.
void test_scan_does_not_resume_on_cursor_reusing_its_work_area() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_scan_6331";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto active_catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(),
        copperfin::localization::select_locale());
    const std::string expected_message = active_catalog.translate(
        "Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", {{"command", "SCAN"}});

    const auto run_script = [&](const std::string &name, const std::string &body) {
        const fs::path script_path = temp_root / (name + ".prg");
        write_text(script_path, body);
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(script_path.string(), temp_root.string(), false));
        return session.run(copperfin::runtime::DebugResumeAction::continue_run);
    };
    const auto global_text = [](const auto &state, const std::string &name) -> std::string {
        const auto found = state.globals.find(name);
        return found == state.globals.end() ? std::string("<missing>") : copperfin::runtime::format_value(found->second);
    };
    const std::string swap_function =
        "FUNCTION SwapCursor\n"
        "IF !lSwapped\n"
        "    lSwapped = .T.\n"
        "    USE IN Source\n"
        "    CREATE CURSOR Replacement (ID I)\n"
        "    INSERT INTO Replacement VALUES (99)\n"
        "    INSERT INTO Replacement VALUES (98)\n"
        "    GO TOP IN Replacement\n"
        "ENDIF\n"
        "RETURN .T.\n"
        "ENDFUNC\n";
    const std::string prologue =
        "PUBLIC lSwapped\n"
        "lSwapped = .F.\n"
        "nSeen = 0\n"
        "cAliases = ''\n"
        "lErrorCaught = .F.\n"
        "cErrMsg = ''\n"
        "CREATE CURSOR Source (ID I)\n"
        "INSERT INTO Source VALUES (1)\n"
        "INSERT INTO Source VALUES (2)\n"
        "SELECT Source\n"
        "GO TOP\n";
    const std::string epilogue =
        "nReplacementSum = 0\n"
        "IF USED('Replacement')\n"
        "    SELECT Replacement\n"
        "    SUM ID TO nReplacementSum\n"
        "ENDIF\n"
        "lAfter = .T.\n"
        "RETURN\n";

    // Issue repro: the FOR predicate swaps the cursor. The resumable predicate
    // path must raise the catchable SCAN error, never run the body on it.
    {
        const auto state = run_script("scan_for_swap",
            prologue +
            "TRY\n"
            "    SCAN FOR SwapCursor()\n"
            "        nSeen = nSeen + 1\n"
            "        cAliases = cAliases + ALIAS() + ':' + TRANSFORM(ID) + ';'\n"
            "        REPLACE ID WITH -1\n"
            "    ENDSCAN\n"
            "CATCH TO oErr\n"
            "    lErrorCaught = .T.\n"
            "    cErrMsg = oErr.Message\n"
            "ENDTRY\n" +
            epilogue + swap_function);
        expect(state.completed, "#6331 FOR swap: script should complete: " + state.message);
        expect(global_text(state, "lafter") == "true", "#6331 FOR swap: execution should continue after the SCAN");
        expect(global_text(state, "lerrorcaught") == "true",
            "#6331 FOR swap: losing the scanned cursor should raise a catchable error");
        expect(global_text(state, "cerrmsg").find(expected_message) != std::string::npos,
            "#6331 FOR swap: error should report the lost SCAN target, got: " + global_text(state, "cerrmsg"));
        expect(global_text(state, "nseen") == "0" && global_text(state, "caliases").empty(),
            "#6331 FOR swap: the body must not run against the replacement cursor, got: " +
                global_text(state, "caliases"));
        expect(global_text(state, "nreplacementsum") == "197",
            "#6331 FOR swap: the replacement cursor must not be mutated, got sum: " +
                global_text(state, "nreplacementsum"));
    }

    // The body swaps the cursor: ENDSCAN must end the loop the same way it
    // does when the cursor is simply closed, not iterate the replacement.
    {
        const auto state = run_script("scan_body_swap",
            prologue +
            "SCAN\n"
            "    nSeen = nSeen + 1\n"
            "    SwapCursor()\n"
            "ENDSCAN\n"
            "SELECT Replacement\n"
            "REPLACE ALL ID WITH ID\n" +
            epilogue + swap_function);
        expect(state.completed, "#6331 body swap: script should complete: " + state.message);
        expect(global_text(state, "nseen") == "1",
            "#6331 body swap: ENDSCAN must not continue on the replacement cursor, got iterations: " +
                global_text(state, "nseen"));
        expect(global_text(state, "nreplacementsum") == "197",
            "#6331 body swap: the replacement cursor must be untouched, got sum: " +
                global_text(state, "nreplacementsum"));
    }

    // Same body swap, but with a UDF FOR clause so ENDSCAN takes the
    // resumable continuation path.
    {
        const auto state = run_script("scan_body_swap_resumable",
            prologue +
            "TRY\n"
            "    SCAN FOR KeepRow()\n"
            "        nSeen = nSeen + 1\n"
            "        SwapCursor()\n"
            "    ENDSCAN\n"
            "CATCH TO oErr\n"
            "    lErrorCaught = .T.\n"
            "ENDTRY\n" +
            epilogue + swap_function +
            "FUNCTION KeepRow\n"
            "RETURN .T.\n"
            "ENDFUNC\n");
        expect(state.completed, "#6331 resumable body swap: script should complete: " + state.message);
        expect(global_text(state, "nseen") == "1",
            "#6331 resumable body swap: the loop must not continue on the replacement cursor, got iterations: " +
                global_text(state, "nseen"));
        expect(global_text(state, "nreplacementsum") == "197",
            "#6331 resumable body swap: the replacement cursor must be untouched, got sum: " +
                global_text(state, "nreplacementsum"));
    }

    // #6331 review: a predicate that leaves the cursor open but switches
    // DATASESSION must stop the SCAN, not keep iterating the original.
    {
        const auto state = run_script("scan_for_session_switch",
            prologue +
            "TRY\n"
            "    SCAN FOR SwitchSession()\n"
            "        nSeen = nSeen + 1\n"
            "    ENDSCAN\n"
            "CATCH TO oErr\n"
            "    lErrorCaught = .T.\n"
            "    cErrMsg = oErr.Message\n"
            "ENDTRY\n"
            "SET DATASESSION TO 1\n" +
            epilogue + swap_function +
            "FUNCTION SwitchSession\n"
            "SET DATASESSION TO 2\n"
            "RETURN .T.\n"
            "ENDFUNC\n");
        expect(state.completed, "#6331 session switch: script should complete: " + state.message);
        expect(global_text(state, "lerrorcaught") == "true" &&
                   global_text(state, "cerrmsg").find(expected_message) != std::string::npos,
            "#6331 session switch: switching DATASESSION should raise the catchable SCAN error, got: " +
                global_text(state, "cerrmsg"));
        expect(global_text(state, "nseen") == "0",
            "#6331 session switch: the body must not run after the session switch, got: " + global_text(state, "nseen"));
    }

    // #6331 review: ON ERROR ... RESUME abandoning a faulting predicate must
    // not park the replacement cursor that reused the work area at EOF.
    {
        const auto state = run_script("scan_resume_after_swap",
            prologue +
            "ON ERROR DO HandleScanFault\n"
            "SCAN FOR SwapCursor() + 1 / 0\n"
            "    nSeen = nSeen + 1\n"
            "ENDSCAN\n"
            "ON ERROR\n"
            "nReplacementRecno = RECNO('Replacement')\n"
            "lReplacementEof = EOF('Replacement')\n" +
            epilogue + swap_function +
            "PROCEDURE HandleScanFault\n"
            "RESUME\n"
            "RETURN\n");
        expect(state.completed, "#6331 RESUME: script should complete: " + state.message);
        expect(global_text(state, "nseen") == "0", "#6331 RESUME: the SCAN body should be skipped");
        expect(global_text(state, "nreplacementrecno") == "1" && global_text(state, "lreplacementeof") == "false",
            "#6331 RESUME: the replacement cursor must keep its own position, got RECNO " +
                global_text(state, "nreplacementrecno") + " EOF " + global_text(state, "lreplacementeof"));
    }

    fs::remove_all(temp_root, ignored);
}

// #6242: LOCATE/CONTINUE (and SCAN's direct search) share
// locate_next_matching_record(), which kept writing FOUND()/position state
// through a cursor its own FOR/WHILE/index-key evaluation had just closed.
void test_locate_predicate_closing_cursor_fails_catchably() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_locate_6242";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto active_catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(),
        copperfin::localization::select_locale());
    const auto expected_message = [&](const std::string &command) {
        return active_catalog.translate(
            "Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", {{"command", command}});
    };
    const auto global_text = [](const auto &state, const std::string &name) -> std::string {
        const auto found = state.globals.find(name);
        return found == state.globals.end() ? std::string("<missing>") : copperfin::runtime::format_value(found->second);
    };

    const std::string routines =
        "FUNCTION DropCursor\n"
        "    nCalls = nCalls + 1\n"
        "    IF nCalls >= nDropOnCall\n"
        "        USE IN Source\n"
        "        IF lReopen\n"
        "            CREATE CURSOR Replacement (ID I)\n"
        "            INSERT INTO Replacement VALUES (99)\n"
        "            INSERT INTO Replacement VALUES (98)\n"
        "            GO TOP IN Replacement\n"
        "        ENDIF\n"
        "    ENDIF\n"
        "    RETURN .T.\n"
        "ENDFUNC\n"
        "FUNCTION DropKey\n"
        "    DropCursor()\n"
        "    RETURN 2\n"
        "ENDFUNC\n";

    struct Scenario {
        std::string label;
        std::string command;  // expected command name in the error
        std::string statements;
        int drop_on_call = 1;
        bool reopen = false;
        bool indexed = false;
        std::string success_event = "runtime.locate";
        std::size_t prior_success_events = 0U;  // emitted by an earlier, successful command
    };
    const std::vector<Scenario> scenarios = {
        {"locate_for", "LOCATE", "LOCATE FOR DropCursor()\n"},
        {"locate_while", "LOCATE", "LOCATE FOR .T. WHILE DropCursor()\n"},
        {"locate_for_reopen", "LOCATE", "LOCATE FOR DropCursor()\n", 1, true},
        {"continue_for", "CONTINUE", "LOCATE FOR DropCursor()\nCONTINUE\n", 2, false, false, "runtime.locate", 1U},
        {"continue_for_reopen", "CONTINUE", "LOCATE FOR DropCursor()\nCONTINUE\n", 2, true, false, "runtime.locate", 1U},
        {"locate_ordered", "LOCATE", "SET ORDER TO ID\nLOCATE FOR DropCursor()\n", 1, false, true},
        {"locate_index_key", "LOCATE", "SET ORDER TO ID\nLOCATE FOR ID = DropKey()\n", 1, true, true},
        {"scan_evaluate", "SCAN",
            "SCAN FOR EVALUATE('DropCursor()')\n"
            "    nBody = nBody + 1\n"
            "ENDSCAN\n", 1, true, false, "runtime.scan"},
        // #6529 review: an active SET FILTER that closes/replaces the cursor,
        // on the linear and indexed LOCATE paths and on GO/SKIP navigation.
        {"filter_locate", "LOCATE", "SET FILTER TO DropCursor()\nLOCATE FOR .T.\n", 1, true},
        {"filter_locate_indexed", "LOCATE", "SET ORDER TO ID\nSET FILTER TO DropCursor()\nLOCATE FOR ID = 2\n", 1, true, true},
        // The prologue GO TOP and DropCursor's own GO TOP IN Replacement are
        // the two expected runtime.go events; the failed GO TOP adds none.
        {"filter_go_top", "GO", "SET FILTER TO DropCursor()\nGO TOP\n", 1, true, false, "runtime.go", 2U},
        {"filter_skip", "SKIP", "SET FILTER TO DropCursor()\nSKIP\n", 1, true, false, "runtime.skip"},
    };

    for (const auto &scenario : scenarios) {
        const fs::path script_path = temp_root / (scenario.label + ".prg");
        write_text(
            script_path,
            "nCalls = 0\n"
            "nBody = 0\n"
            "nDropOnCall = " + std::to_string(scenario.drop_on_call) + "\n"
            "lReopen = " + (scenario.reopen ? ".T." : ".F.") + "\n"
            "lErrorCaught = .F.\n"
            "cErrMsg = ''\n"
            "CREATE CURSOR Source (ID I)\n"
            "INSERT INTO Source VALUES (1)\n"
            "INSERT INTO Source VALUES (2)\n"
            "INSERT INTO Source VALUES (3)\n" +
            std::string(scenario.indexed ? "INDEX ON ID TAG ID\n" : "") +
            "SELECT Source\n"
            "GO TOP\n"
            "TRY\n" +
            scenario.statements +
            "CATCH TO oErr\n"
            "    lErrorCaught = .T.\n"
            "    cErrMsg = oErr.Message\n"
            "ENDTRY\n"
            "lSourceOpen = USED('Source')\n"
            "nReplacementRecno = IIF(USED('Replacement'), RECNO('Replacement'), -1)\n"
            "lReplacementFound = IIF(USED('Replacement'), FOUND('Replacement'), .F.)\n"
            "lAfter = .T.\n"
            "RETURN\n" +
            routines);
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(script_path.string(), temp_root.string(), false));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        const std::string prefix = "#6242 " + scenario.label + ": ";
        expect(state.completed, prefix + "script should complete: " + state.message);
        expect(global_text(state, "lafter") == "true", prefix + "execution should continue after the command");
        expect(global_text(state, "lerrorcaught") == "true" &&
                   global_text(state, "cerrmsg").find(expected_message(scenario.command)) != std::string::npos,
            prefix + "closing the searched cursor should raise the catchable " + scenario.command +
                " error, got: " + global_text(state, "cerrmsg"));
        expect(global_text(state, "lsourceopen") == "false", prefix + "the closed cursor should stay closed");
        expect(global_text(state, "nbody") == "0", prefix + "no SCAN body should run");
        const auto count_events = [&](const std::string &category) {
            return static_cast<std::size_t>(std::count_if(state.events.begin(), state.events.end(),
                [&](const auto &event) { return event.category == category; }));
        };
        expect(count_events(scenario.success_event) == scenario.prior_success_events,
            prefix + "the failed command must not emit " + scenario.success_event + ", got " +
                std::to_string(count_events(scenario.success_event)) + " event(s)");
        const std::size_t prior_rushmore_events =
            scenario.success_event == "runtime.locate" ? scenario.prior_success_events : 0U;
        expect(count_events("runtime.rushmore") == prior_rushmore_events,
            prefix + "the failed search must not emit runtime.rushmore, got " +
                std::to_string(count_events("runtime.rushmore")) + " event(s)");
        if (scenario.reopen) {
            expect(global_text(state, "nreplacementrecno") == "1" && global_text(state, "lreplacementfound") == "false",
                prefix + "the replacement cursor must keep its own position and FOUND(), got RECNO " +
                    global_text(state, "nreplacementrecno") + " FOUND " + global_text(state, "lreplacementfound"));
        }
    }

    fs::remove_all(temp_root, ignored);
}


// #6241: COPY TO ARRAY and row-producing COPY TO kept moving, reading, and
// finally restoring a source cursor that their own FOR/filter evaluation had
// closed. They must fail catchably with no array assignment, no output file,
// and no success event.
void test_copy_to_predicate_closing_source_fails_catchably() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_copy_to_6241";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto active_catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(),
        copperfin::localization::select_locale());
    const auto global_text = [](const auto &state, const std::string &name) -> std::string {
        const auto found = state.globals.find(name);
        return found == state.globals.end() ? std::string("<missing>") : copperfin::runtime::format_value(found->second);
    };

    struct Scenario {
        std::string label;
        std::string command;
        std::string statement;  // uses <DEST> for the output path
        std::string destination_name;  // empty for COPY TO ARRAY
        int drop_on_call = 1;
        bool reopen = false;
        bool filter = false;
        bool close_all = false;
    };
    const std::vector<Scenario> scenarios = {
        {"array_for_first", "COPY TO ARRAY", "COPY TO ARRAY aResult FOR DropCursor()", "", 1},
        // #6534 review: CLOSE ALL from the predicate.
        {"array_for_close_all", "COPY TO ARRAY", "COPY TO ARRAY aResult FOR DropCursor()", "", 2, false, false, true},
        {"csv_filter_close_all", "COPY TO", "COPY TO '<DEST>' TYPE CSV", "out.csv", 1, false, true, true},
        {"array_for_middle_reopen", "COPY TO ARRAY", "COPY TO ARRAY aResult FOR DropCursor()", "", 2, true},
        {"array_filter_last", "COPY TO ARRAY", "COPY TO ARRAY aResult", "", 3, false, true},
        {"dbf_for_first", "COPY TO", "COPY TO '<DEST>' FOR DropCursor()", "out.dbf", 1},
        {"dbf_filter_middle_reopen", "COPY TO", "COPY TO '<DEST>'", "out.dbf", 2, true, true},
        {"csv_for_last", "COPY TO", "COPY TO '<DEST>' TYPE CSV FOR DropCursor()", "out.csv", 3},
        {"sdf_for_first_reopen", "COPY TO", "COPY TO '<DEST>' TYPE SDF FOR DropCursor()", "out.txt", 1, true},
    };

    for (const auto &scenario : scenarios) {
        const fs::path destination = scenario.destination_name.empty()
            ? fs::path{}
            : temp_root / (scenario.label + "_" + scenario.destination_name);
        std::string statement = scenario.statement;
        if (const auto marker = statement.find("<DEST>"); marker != std::string::npos) {
            statement.replace(marker, 6U, destination.string());
        }
        const fs::path script_path = temp_root / (scenario.label + ".prg");
        write_text(
            script_path,
            "nCalls = 0\n"
            "nDropOnCall = " + std::to_string(scenario.drop_on_call) + "\n"
            "lReopen = " + (scenario.reopen ? ".T." : ".F.") + "\n"
            "lCloseAll = " + (scenario.close_all ? ".T." : ".F.") + "\n"
            "lErrorCaught = .F.\n"
            "cErrMsg = ''\n"
            "DIMENSION aResult[1]\n"
            "aResult[1] = 'keep'\n"
            "CREATE CURSOR Source (ID I)\n"
            "INSERT INTO Source VALUES (1)\n"
            "INSERT INTO Source VALUES (2)\n"
            "INSERT INTO Source VALUES (3)\n"
            "SELECT Source\n"
            "GO TOP\n" +
            std::string(scenario.filter ? "SET FILTER TO DropCursor()\n" : "") +
            "TRY\n"
            "    " + statement + "\n"
            "CATCH TO oErr\n"
            "    lErrorCaught = .T.\n"
            "    cErrMsg = oErr.Message\n"
            "ENDTRY\n"
            "cKeep = aResult[1]\n"
            "nKeepLen = ALEN(aResult)\n"
            "lSourceOpen = USED('Source')\n"
            "nReplacementRecno = IIF(USED('Replacement'), RECNO('Replacement'), -1)\n"
            "lAfter = .T.\n"
            "RETURN\n"
            "FUNCTION DropCursor\n"
            "    nCalls = nCalls + 1\n"
            "    IF nCalls = nDropOnCall\n"
            "        IF lCloseAll\n"
            "            CLOSE ALL\n"
            "        ELSE\n"
            "            USE IN Source\n"
            "        ENDIF\n"
            "        IF lReopen\n"
            "            CREATE CURSOR Replacement (ID I)\n"
            "            INSERT INTO Replacement VALUES (99)\n"
            "            INSERT INTO Replacement VALUES (98)\n"
            "            GO TOP IN Replacement\n"
            "        ENDIF\n"
            "    ENDIF\n"
            "    RETURN .T.\n"
            "ENDFUNC\n");
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(script_path.string(), temp_root.string(), false));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        const std::string prefix = "#6241 " + scenario.label + ": ";
        expect(state.completed, prefix + "script should complete: " + state.message);
        expect(global_text(state, "lafter") == "true", prefix + "execution should continue after the command");
        const std::string expected_message = active_catalog.translate(
            "Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", {{"command", scenario.command}});
        expect(global_text(state, "lerrorcaught") == "true" &&
                   global_text(state, "cerrmsg").find(expected_message) != std::string::npos,
            prefix + "closing the source should raise the catchable " + scenario.command + " error, got: " +
                global_text(state, "cerrmsg"));
        expect(global_text(state, "ncalls") == std::to_string(scenario.drop_on_call),
            prefix + "evaluation should stop at the closing call, got calls: " + global_text(state, "ncalls"));
        expect(global_text(state, "ckeep") == "keep" && global_text(state, "nkeeplen") == "1",
            prefix + "the destination array must keep its previous contents, got: " + global_text(state, "ckeep") +
                " (ALEN " + global_text(state, "nkeeplen") + ")");
        expect(global_text(state, "lsourceopen") == "false", prefix + "the closed source should stay closed");
        if (!destination.empty()) {
            expect(!fs::exists(destination), prefix + "no output file should be published: " + destination.string());
        }
        if (scenario.reopen) {
            expect(global_text(state, "nreplacementrecno") == "1",
                prefix + "the replacement cursor must keep its own position, got RECNO " +
                    global_text(state, "nreplacementrecno"));
        }
        const auto success_events = std::count_if(state.events.begin(), state.events.end(), [](const auto &event) {
            return event.category == "runtime.copy_to" || event.category == "runtime.copy_to_array";
        });
        expect(success_events == 0, prefix + "the failed copy must not emit a success event");
    }

    fs::remove_all(temp_root, ignored);
}


// #6534 review: the predicate closes the source and then faults inside an
// ON ERROR ... RESUME handler. Resuming the predicate must not resume the copy
// through the freed cursor; the lost-target error reaches the same handler.
void test_copy_to_predicate_closing_source_under_on_error_resume() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_copy_to_6241_on_error";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto active_catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(),
        copperfin::localization::select_locale());
    const auto global_text = [](const auto &state, const std::string &name) -> std::string {
        const auto found = state.globals.find(name);
        return found == state.globals.end() ? std::string("<missing>") : copperfin::runtime::format_value(found->second);
    };

    const fs::path destination = temp_root / "out.dbf";
    const fs::path script_path = temp_root / "copy_on_error.prg";
    write_text(
        script_path,
        "PUBLIC nHandled, cMessages\n"
        "nHandled = 0\n"
        "cMessages = ''\n"
        "nCalls = 0\n"
        "DIMENSION aResult[1]\n"
        "aResult[1] = 'keep'\n"
        "CREATE CURSOR Source (ID I)\n"
        "INSERT INTO Source VALUES (1)\n"
        "INSERT INTO Source VALUES (2)\n"
        "SELECT Source\n"
        "ON ERROR DO HandleCopyFault WITH MESSAGE()\n"
        "COPY TO ARRAY aResult FOR DropThenFault()\n"
        "SELECT 0\n"
        "CREATE CURSOR Source2 (ID I)\n"
        "INSERT INTO Source2 VALUES (1)\n"
        "nCalls = 0\n"
        "COPY TO '" + destination.string() + "' FOR DropThenFault2()\n"
        "ON ERROR\n"
        "cKeep = aResult[1]\n"
        "lAfter = .T.\n"
        "RETURN\n"
        "PROCEDURE HandleCopyFault\n"
        "    LPARAMETERS cMessage\n"
        "    nHandled = nHandled + 1\n"
        "    cMessages = cMessages + cMessage + '|'\n"
        "    RESUME\n"
        "ENDPROC\n"
        "FUNCTION DropThenFault\n"
        "    nCalls = nCalls + 1\n"
        "    USE IN Source\n"
        "    nBad = 1 / 0\n"
        "    RETURN .T.\n"
        "ENDFUNC\n"
        "FUNCTION DropThenFault2\n"
        "    nCalls = nCalls + 1\n"
        "    USE IN Source2\n"
        "    nBad = 1 / 0\n"
        "    RETURN .T.\n"
        "ENDFUNC\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(script_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6241 ON ERROR: script should complete: " + state.message);
    expect(global_text(state, "lafter") == "true", "#6241 ON ERROR: execution should continue after both copies");
    const std::string messages = global_text(state, "cmessages");
    for (const std::string command : {"COPY TO ARRAY", "COPY TO"}) {
        const std::string expected = active_catalog.translate(
            "Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", {{"command", command}});
        expect(messages.find(expected) != std::string::npos,
            "#6241 ON ERROR: the lost-source " + command + " error should reach the handler, got: " + messages);
    }
    expect(global_text(state, "ckeep") == "keep", "#6241 ON ERROR: the destination array must keep its contents");
    expect(!fs::exists(destination), "#6241 ON ERROR: no output file should be published");
    const auto success_events = std::count_if(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.copy_to" || event.category == "runtime.copy_to_array";
    });
    expect(success_events == 0, "#6241 ON ERROR: neither failed copy may emit a success event");

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_control_flow
