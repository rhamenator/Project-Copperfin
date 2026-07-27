// Copyright 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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


}  // namespace cf_test_prg_engine_control_flow
