// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {

void test_elseif_predicate_resumption_review_gaps() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_elseif_review_gaps";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path branch_path = temp_root / "elseif_branches.prg";
    write_text(
        branch_path,
        "fallbackBranch = ''\n"
        "predicateCalls = 0\n"
        "IF falsepredicate('if')\n"
        "    fallbackBranch = 'if'\n"
        "ELSEIF falsepredicate('elseif')\n"
        "    fallbackBranch = 'elseif'\n"
        "ELSE\n"
        "    fallbackBranch = 'else'\n"
        "ENDIF\n"
        "noMatchBranch = 'unchanged'\n"
        "IF falsepredicate('no-match-if')\n"
        "    noMatchBranch = 'if'\n"
        "ELSEIF falsepredicate('no-match-elseif')\n"
        "    noMatchBranch = 'elseif'\n"
        "ENDIF\n"
        "RETURN\n"
        "FUNCTION falsepredicate\n"
        "LPARAMETERS label\n"
        "predicateCalls = predicateCalls + 1\n"
        "RETURN .F.\n");

    auto branch_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(branch_path.string(), temp_root.string(), false));
    const auto branch_state = branch_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(branch_state.completed, "false ELSEIF predicates should complete: " + branch_state.message);
    const auto expect_branch_global = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = branch_state.globals.find(name);
        expect(found != branch_state.globals.end(), name + " should remain visible");
        if (found != branch_state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };
    expect_branch_global("fallbackbranch", "else", "a false ELSEIF predicate should fall through to ELSE");
    expect_branch_global("nomatchbranch", "unchanged", "a chain with no matching IF or ELSEIF should skip its body");
    expect_branch_global("predicatecalls", "4", "each UDF IF/ELSEIF predicate should execute once");

    const fs::path reference_path = temp_root / "elseif_references.prg";
    write_text(
        reference_path,
        "SET UDFPARMS TO REFERENCE\n"
        "scalarValue = 3\n"
        "DIMENSION values[2]\n"
        "values[1] = 10\n"
        "values[2] = 20\n"
        "branch = ''\n"
        "IF .F.\n"
        "    branch = 'if'\n"
        "ELSEIF referencepredicate(@scalarValue, values)\n"
        "    branch = 'elseif'\n"
        "ELSE\n"
        "    branch = 'else'\n"
        "ENDIF\n"
        "modeAfter = SET('UDFPARMS')\n"
        "scalarAfter = scalarValue\n"
        "arrayAfter = values[2]\n"
        "RETURN\n"
        "FUNCTION referencepredicate\n"
        "LPARAMETERS forwardedScalar, forwardedValues\n"
        "RETURN referenceprobe(forwardedScalar, forwardedValues)\n"
        "FUNCTION referenceprobe\n"
        "LPARAMETERS scalarRef, arrayRef\n"
        "scalarRef = scalarRef + 1\n"
        "arrayRef[2] = arrayRef[2] + 5\n"
        "RETURN scalarRef = 4 AND arrayRef[2] = 25\n");

    auto reference_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(reference_path.string(), temp_root.string(), false));
    const auto reference_state = reference_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(reference_state.completed, "reference arguments in a suspended ELSEIF should complete: " + reference_state.message);
    const auto expect_reference_global = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = reference_state.globals.find(name);
        expect(found != reference_state.globals.end(), name + " should remain visible");
        if (found != reference_state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };
    expect_reference_global("branch", "elseif", "a resumed ELSEIF should use the reference predicate result");
    expect_reference_global("modeafter", "REFERENCE", "SET UDFPARMS REFERENCE should survive predicate suspension");
    expect_reference_global("scalarafter", "4", "an @ scalar reference should preserve caller storage");
    expect_reference_global("arrayafter", "25", "a bare-array reference should preserve caller storage");

    const fs::path try_path = temp_root / "elseif_try.prg";
    write_text(
        try_path,
        "caughtCount = 0\n"
        "finallyCalls = 0\n"
        "TRY\n"
        "IF .F.\n"
        "tryIfBranch = 1\n"
        "ELSEIF resumedpredicate()\n"
        "tryElseifBranch = 1\n"
        "ELSE\n"
        "tryElseBranch = 1\n"
        "ENDIF\n"
        "CATCH TO err_text\n"
        "caughtCount = caughtCount + 1\n"
        "FINALLY\n"
        "finallyCalls = finallyCalls + 1\n"
        "ENDTRY\n"
        "afterTry = 1\n"
        "RETURN\n"
        "FUNCTION resumedpredicate\n"
        "RETURN predicatechild()\n"
        "FUNCTION predicatechild\n"
        "DO missing_routine\n"
        "RETURN .T.\n"
        );

    auto try_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(try_path.string(), temp_root.string(), false));
    const auto try_state = try_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(try_state.completed, "TRY/CATCH/FINALLY should preserve a resumed ELSEIF predicate: " + try_state.message);
    const auto expect_try_global = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = try_state.globals.find(name);
        expect(found != try_state.globals.end(), name + " should remain visible");
        if (found != try_state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };
    expect(try_state.globals.find("err_text") != try_state.globals.end(),
           "CATCH TO should capture a fault from the resumed ELSEIF predicate");
    expect_try_global("caughtcount", "1", "CATCH should run once for a fault from a resumed ELSEIF predicate");
    expect_try_global("finallycalls", "1", "FINALLY should run during a resumed ELSEIF predicate");
    expect_try_global("aftertry", "1", "execution should continue after the handled resumed ELSEIF predicate");
    expect(try_state.globals.find("tryelseifbranch") == try_state.globals.end(),
           "a faulting resumed ELSEIF predicate should not execute its branch");
    expect(try_state.globals.find("tryelsebranch") == try_state.globals.end(),
           "a handled resumed ELSEIF predicate fault should not fall through to ELSE");

    const fs::path on_error_path = temp_root / "elseif_on_error.prg";
    write_text(
        on_error_path,
        "PUBLIC handlerCalls, handlerLine, handlerStatement\n"
        "handlerCalls = 0\n"
        "ON ERROR DO handleelseiferror\n"
        "IF .F.\n"
        "    onErrorIfBranch = 1\n"
        "ELSEIF onerrorpredicate()\n"
        "    onErrorElseifBranch = 1\n"
        "ELSE\n"
        "    onErrorElseBranch = 1\n"
        "ENDIF\n"
        "afterOnError = 1\n"
        "RETURN\n"
        "FUNCTION onerrorpredicate\n"
        "RETURN predicatechild() + 1 / 0\n"
        "FUNCTION predicatechild\n"
        "RETURN 7\n"
        "PROCEDURE handleelseiferror\n"
        "handlerCalls = handlerCalls + 1\n"
        "handlerLine = LINENO()\n"
        "handlerStatement = MESSAGE()\n"
        "RETURN\n");

    auto on_error_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(on_error_path.string(), temp_root.string(), false));
    const auto on_error_state = on_error_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(on_error_state.completed, "ON ERROR should handle a resumed ELSEIF fault: " + on_error_state.message);
    const auto expect_on_error_global = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = on_error_state.globals.find(name);
        expect(found != on_error_state.globals.end(), name + " should remain visible");
        if (found != on_error_state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };
    expect_on_error_global("handlercalls", "1", "ON ERROR should run once for a resumed ELSEIF predicate fault");
    expect_on_error_global("handlerline", "14", "ON ERROR should retain the resumed predicate UDF line");
    expect_on_error_global("handlerstatement", "Runtime fault: Division by zero",
                           "ON ERROR should retain the resumed ELSEIF predicate fault message");
    expect_on_error_global("afteronerror", "1", "execution should continue after ON ERROR handles an ELSEIF fault");
    expect(on_error_state.globals.find("onerrorelseifbranch") == on_error_state.globals.end(),
           "ON ERROR should prevent the faulting ELSEIF branch from executing");
    expect(on_error_state.globals.find("onerrorelsebranch") != on_error_state.globals.end(),
           "ON ERROR should resume the conditional through its ELSE fallback");

    const fs::path depth_path = temp_root / "elseif_depth_limit.prg";
    write_text(
        depth_path,
        "IF .F.\n"
        "    depthIfBranch = 1\n"
        "ELSEIF runawaypredicate(1)\n"
        "    depthElseifBranch = 1\n"
        "ENDIF\n"
        "RETURN\n"
        "FUNCTION runawaypredicate\n"
        "LPARAMETERS depth\n"
        "RETURN runawaypredicate(depth + 1)\n");

    auto depth_options = make_runtime_session_options(depth_path.string(), temp_root.string(), false);
    depth_options.max_call_depth = 96U;
    auto depth_session = copperfin::runtime::PrgRuntimeSession::create(depth_options);
    const auto depth_state = depth_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(depth_state.reason == copperfin::runtime::DebugPauseReason::error,
           "a deep resumed ELSEIF predicate should stop at the runtime call-depth guardrail");
    expect(depth_state.message.find("maximum call depth") != std::string::npos,
           "a deep resumed ELSEIF predicate should report max_call_depth without exhausting the native stack");

    const fs::path debugger_path = temp_root / "elseif_debugger.prg";
    write_text(
        debugger_path,
        "IF .F.\n"
        "    ifBranch = 1\n"
        "ELSEIF child()\n"
        "    elseifBranch = 1\n"
        "ENDIF\n"
        "afterElseif = 1\n"
        "RETURN\n"
        "FUNCTION child\n"
        "RETURN .T.\n");

    auto exact_budget_options = make_runtime_session_options(debugger_path.string(), temp_root.string(), false);
    exact_budget_options.max_executed_statements = 7U;
    auto exact_budget_session = copperfin::runtime::PrgRuntimeSession::create(exact_budget_options);
    const auto exact_budget_state = exact_budget_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(exact_budget_state.completed && exact_budget_state.executed_statement_count == 7U,
           "a resumed ELSEIF predicate should not consume a second statement-budget slot");

    auto exhausted_budget_options = make_runtime_session_options(debugger_path.string(), temp_root.string(), false);
    exhausted_budget_options.max_executed_statements = 6U;
    auto exhausted_budget_session = copperfin::runtime::PrgRuntimeSession::create(exhausted_budget_options);
    const auto exhausted_budget_state = exhausted_budget_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(exhausted_budget_state.reason == copperfin::runtime::DebugPauseReason::error &&
               exhausted_budget_state.executed_statement_count == 6U && exhausted_budget_state.location.line == 7U,
           "ELSEIF predicate budget exhaustion should occur at its physical statement boundary");

    auto breakpoint_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(debugger_path.string(), temp_root.string(), false));
    breakpoint_session.add_breakpoint({.file_path = debugger_path.string(), .line = 3U});
    const auto breakpoint_state = breakpoint_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(breakpoint_state.reason == copperfin::runtime::DebugPauseReason::breakpoint &&
               breakpoint_state.location.line == 3U,
           "a resumed ELSEIF predicate should honor its breakpoint before UDF evaluation");
    const auto breakpoint_completed_state = breakpoint_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(breakpoint_completed_state.completed, "a resumed ELSEIF predicate should complete after its breakpoint");
    expect(std::count_if(
               breakpoint_completed_state.events.begin(),
               breakpoint_completed_state.events.end(),
               [](const copperfin::runtime::RuntimeEvent &event) {
                   return event.category == "execute" && event.location.line == 3U;
               }) == 1,
           "an ELSEIF predicate should emit one execute event for its physical statement");

    const auto make_debug_session = [&]() {
        return copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(debugger_path.string(), temp_root.string(), true));
    };
    auto debug_session = make_debug_session();
    const auto entry_state = debug_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    const auto elseif_state = debug_session.run(copperfin::runtime::DebugResumeAction::step_into);
    const auto child_state = debug_session.run(copperfin::runtime::DebugResumeAction::step_into);
    const auto resumed_state = debug_session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(entry_state.reason == copperfin::runtime::DebugPauseReason::entry &&
               elseif_state.reason == copperfin::runtime::DebugPauseReason::step && elseif_state.location.line == 3U &&
               child_state.reason == copperfin::runtime::DebugPauseReason::step && child_state.location.line == 9U &&
               resumed_state.reason == copperfin::runtime::DebugPauseReason::step && resumed_state.location.line == 3U,
           "debugger stepping should expose the ELSEIF, child RETURN, then resumed ELSEIF predicate");
    const auto completed_state = debug_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(completed_state.completed, "continuing a debugger-paused ELSEIF predicate should complete");

    auto cancel_session = make_debug_session();
    (void)cancel_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    (void)cancel_session.run(copperfin::runtime::DebugResumeAction::step_into);
    (void)cancel_session.run(copperfin::runtime::DebugResumeAction::step_into);
    const auto cancel_resumed_state = cancel_session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(cancel_resumed_state.location.line == 3U,
           "cancellation setup should pause on the suspended ELSEIF predicate");
    cancel_session.request_cancel();
    const auto cancelled_state = cancel_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(cancelled_state.reason == copperfin::runtime::DebugPauseReason::error &&
               cancelled_state.location.line == 3U && cancelled_state.statement_text == "ELSEIF child()",
           "cancelling a suspended ELSEIF predicate should retain its source metadata");
    expect(cancelled_state.globals.find("elseifbranch") == cancelled_state.globals.end(),
           "cancelling a suspended ELSEIF predicate should not execute its branch");

    const auto expect_direct_control_transfer_abandons_predicate =
        [&](const std::string &name, const std::string &command, const std::string &event_category) {
            const fs::path control_path = temp_root / (name + ".prg");
            write_text(
                control_path,
                "IF .F.\n"
                "    ifBranch = 1\n"
                "ELSEIF controlpredicate()\n"
                "    elseifBranch = 1\n"
                "ENDIF\n"
                "afterControl = 1\n"
                "RETURN\n"
                "FUNCTION controlpredicate\n" +
                    command + "\n"
                "RETURN .T.\n");
            auto control_session = copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(control_path.string(), temp_root.string(), false));
            const auto control_state = control_session.run(copperfin::runtime::DebugResumeAction::continue_run);
            expect(control_state.completed, command + " in an ELSEIF predicate should terminate cleanly");
            expect(control_state.globals.find("elseifbranch") == control_state.globals.end(),
                   command + " should abandon the pending ELSEIF branch");
            expect(control_state.globals.find("aftercontrol") == control_state.globals.end(),
                   command + " should prevent execution after the abandoned ELSEIF predicate");
            expect(std::count_if(
                       control_state.events.begin(),
                       control_state.events.end(),
                       [&](const copperfin::runtime::RuntimeEvent &event) {
                           return event.category == event_category;
                       }) == 1,
                   command + " should preserve its runtime event while abandoning the ELSEIF predicate");
        };
    expect_direct_control_transfer_abandons_predicate("elseif_cancel_predicate", "CANCEL", "runtime.cancel");
    expect_direct_control_transfer_abandons_predicate("elseif_quit_predicate", "QUIT", "runtime.quit");

    fs::remove_all(temp_root, ignored);
}


}  // namespace cf_test_prg_engine_control_flow
