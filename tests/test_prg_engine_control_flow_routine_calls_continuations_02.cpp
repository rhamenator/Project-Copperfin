// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {

void test_compound_return_uses_heap_backed_expression_checkpoints() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_compound_return";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path depth_path = temp_root / "compound_depth_limit.prg";
    write_text(
        depth_path,
        "result = recurse(1)\n"
        "RETURN\n"
        "FUNCTION recurse\n"
        "LPARAMETERS depth\n"
        "RETURN 1 + recurse(depth + 1)\n");

    auto depth_options = make_runtime_session_options(depth_path.string(), temp_root.string(), false);
    depth_options.max_call_depth = 2048U;
    copperfin::runtime::PrgRuntimeSession depth_session =
        copperfin::runtime::PrgRuntimeSession::create(depth_options);
    const auto depth_state = depth_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(
        depth_state.reason == copperfin::runtime::DebugPauseReason::error,
        "deep compound recursive RETURN should stop at the runtime guardrail");
    expect(
        depth_state.message.find("maximum call depth") != std::string::npos,
        "deep compound recursive RETURN should report the configured call-depth diagnostic");

    const fs::path semantics_path = temp_root / "compound_checkpoint_semantics.prg";
    write_text(
        semantics_path,
        "SET UDFPARMS TO REFERENCE\n"
        "value = 3\n"
        "seed = 7\n"
        "beforeCalls = 0\n"
        "explicitCalls = 0\n"
        "bareCalls = 0\n"
        "outerCalls = 0\n"
        "arrayCalls = 0\n"
        "bumpCalls = 0\n"
        "finallyCalls = 0\n"
        "caughtCalls = 0\n"
        "resumedCaughtCalls = 0\n"
        "DIMENSION values[2]\n"
        "values[1] = 10\n"
        "values[2] = 20\n"
        "result = combine(@value, @values)\n"
        "snapshotResult = snapshotvalue()\n"
        "caughtResult = catchcompoundfault()\n"
        "resumedFaultResult = catchresumedfault()\n"
        "valueAfter = value\n"
        "arrayAfter = values[2]\n"
        "seedAfter = seed\n"
        "RETURN\n"
        "FUNCTION combine\n"
        "LPARAMETERS forwarded, forwardedValues\n"
        "TRY\n"
        "RETURN before() + outer(explicitref(@forwarded), bareref(forwarded), barearray(forwardedValues))\n"
        "FINALLY\n"
        "finallyCalls = finallyCalls + 1\n"
        "ENDTRY\n"
        "FUNCTION before\n"
        "beforeCalls = beforeCalls + 1\n"
        "RETURN 10\n"
        "FUNCTION explicitref\n"
        "LPARAMETERS target\n"
        "explicitCalls = explicitCalls + 1\n"
        "target = target + 1\n"
        "RETURN target\n"
        "FUNCTION bareref\n"
        "LPARAMETERS target\n"
        "bareCalls = bareCalls + 1\n"
        "target = target + 2\n"
        "RETURN target\n"
        "FUNCTION barearray\n"
        "LPARAMETERS target\n"
        "arrayCalls = arrayCalls + 1\n"
        "target[2] = target[2] + 3\n"
        "RETURN target[2]\n"
        "FUNCTION outer\n"
        "LPARAMETERS firstValue, secondValue, thirdValue\n"
        "outerCalls = outerCalls + 1\n"
        "RETURN firstValue * 10 + secondValue + thirdValue\n"
        "FUNCTION snapshotvalue\n"
        "RETURN seed + bumpseed()\n"
        "FUNCTION bumpseed\n"
        "bumpCalls = bumpCalls + 1\n"
        "seed = 100\n"
        "RETURN 5\n"
        "FUNCTION catchcompoundfault\n"
        "TRY\n"
        "RETURN 1 + throwfromchild()\n"
        "CATCH\n"
        "caughtCalls = caughtCalls + 1\n"
        "ENDTRY\n"
        "RETURN 42\n"
        "FUNCTION throwfromchild\n"
        "THROW 'compound child fault'\n"
        "FUNCTION catchresumedfault\n"
        "TRY\n"
        "RETURN childvalue() + 1 / 0\n"
        "CATCH\n"
        "resumedCaughtCalls = resumedCaughtCalls + 1\n"
        "ENDTRY\n"
        "RETURN 84\n"
        "FUNCTION childvalue\n"
        "RETURN 5\n");

    copperfin::runtime::PrgRuntimeSession semantics_session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(semantics_path.string(), temp_root.string(), false));
    const auto semantics_state = semantics_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(
        semantics_state.completed,
        "compound RETURN checkpoint semantics script should complete: " + semantics_state.message);

    const auto expect_global = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = semantics_state.globals.find(name);
        expect(found != semantics_state.globals.end(), name + " should remain visible");
        if (found != semantics_state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };
    expect_global("result", "79", "compound nested calls should preserve left-to-right results");
    const auto snapshot_found = semantics_state.globals.find("snapshotresult");
    const std::string snapshot_actual = snapshot_found == semantics_state.globals.end()
                                            ? std::string("<missing>")
                                            : copperfin::runtime::format_value(snapshot_found->second);
    expect_global(
        "snapshotresult",
        "12",
        "a resumed expression should retain variable values read before suspension (actual " + snapshot_actual + ")");
    expect_global("caughtresult", "42", "a caller CATCH should replace an aborted compound return");
    expect_global("resumedfaultresult", "84", "a fault raised after resumption should enter the caller CATCH");
    expect_global("valueafter", "6", "explicit and SET UDFPARMS references should reach caller storage once");
    expect_global("arrayafter", "23", "a bare-array reference should retain ultimate caller storage");
    expect_global("seedafter", "100", "the resumed child mutation should remain visible after RETURN completion");
    expect_global("beforecalls", "1", "an earlier user-routine operand should not be replayed");
    expect_global("explicitcalls", "1", "an explicit-reference argument routine should run once");
    expect_global("barecalls", "1", "a SET UDFPARMS reference argument routine should run once");
    expect_global("outercalls", "1", "the outer argument-nested routine should run once");
    expect_global("arraycalls", "1", "a bare-array argument routine should run once");
    expect_global("bumpcalls", "1", "a later side-effecting routine should run once");
    expect_global("finallycalls", "1", "the suspended compound return should run FINALLY once");
    expect_global("caughtcalls", "1", "a compound child fault should enter the caller CATCH once");
    expect_global("resumedcaughtcalls", "1", "a resumed-expression fault should enter the caller CATCH once");

    const fs::path on_error_path = temp_root / "compound_return_on_error.prg";
    write_text(
        on_error_path,
        "PUBLIC handlerCount, handlerMessage, handlerLine, handlerRows, handlerAErrorMessage, handlerAErrorLine, handlerStatement\n"
        "handlerCount = 0\n"
        "ON ERROR DO handleerr\n"
        "result = resumedonerror()\n"
        "afterError = 1\n"
        "RETURN\n"
        "FUNCTION resumedonerror\n"
        "RETURN childvalue() + 1 / 0\n"
        "FUNCTION childvalue\n"
        "RETURN 5\n"
        "PROCEDURE handleerr\n"
        "handlerCount = handlerCount + 1\n"
        "handlerMessage = MESSAGE()\n"
        "handlerLine = LINENO()\n"
        "handlerRows = AERROR(handlerError)\n"
        "handlerAErrorMessage = handlerError[1,2]\n"
        "handlerAErrorLine = handlerError[1,5]\n"
        "handlerStatement = handlerError[1,7]\n"
        "RETURN\n");

    auto on_error_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(on_error_path.string(), temp_root.string(), false));
    const auto on_error_state = on_error_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(on_error_state.completed,
           "ON ERROR should handle a fault raised after compound RETURN resumption: " + on_error_state.message);
    const auto expect_on_error_global = [&](const std::string &name, const std::string &expected) {
        const auto found = on_error_state.globals.find(name);
        expect(found != on_error_state.globals.end(), name + " should be captured by the resumed RETURN handler");
        if (found != on_error_state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " should equal '" + expected + "'");
        }
    };
    expect_on_error_global("handlercount", "1");
    expect_on_error_global("handlerrows", "1");
    expect_on_error_global("handlerline", "8");
    expect_on_error_global("handleraerrorline", "8");
    expect_on_error_global("handlermessage", "Runtime fault: Division by zero");
    expect_on_error_global("handleraerrormessage", "Runtime fault: Division by zero");
    expect_on_error_global("handlerstatement", "RETURN childvalue() + 1 / 0");
    expect_on_error_global("aftererror", "1");

    const fs::path debugger_path = temp_root / "compound_return_debugger.prg";
    write_text(
        debugger_path,
        "RETURN child() + 1\n"
        "FUNCTION child\n"
        "RETURN 2\n");

    auto exact_budget_options =
        make_runtime_session_options(debugger_path.string(), temp_root.string(), false);
    exact_budget_options.max_executed_statements = 2U;
    auto exact_budget_session = copperfin::runtime::PrgRuntimeSession::create(exact_budget_options);
    const auto exact_budget_state =
        exact_budget_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(exact_budget_state.completed && exact_budget_state.executed_statement_count == 2U,
           "compound RETURN resumption should not consume a third statement-budget slot");

    auto exhausted_budget_options = exact_budget_options;
    exhausted_budget_options.max_executed_statements = 1U;
    auto exhausted_budget_session = copperfin::runtime::PrgRuntimeSession::create(exhausted_budget_options);
    const auto exhausted_budget_state =
        exhausted_budget_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(exhausted_budget_state.reason == copperfin::runtime::DebugPauseReason::error &&
               exhausted_budget_state.executed_statement_count == 1U &&
               exhausted_budget_state.location.line == 3U,
           "statement-budget exhaustion should stop before the child RETURN without double-counting its caller");

    auto breakpoint_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(debugger_path.string(), temp_root.string(), false));
    breakpoint_session.add_breakpoint({.file_path = debugger_path.string(), .line = 1U});
    const auto breakpoint_state =
        breakpoint_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(breakpoint_state.reason == copperfin::runtime::DebugPauseReason::breakpoint,
           "a compound RETURN should honor its breakpoint before initial execution");
    const auto breakpoint_completed_state =
        breakpoint_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(breakpoint_completed_state.completed,
           "a resumed compound RETURN should not hit its already-consumed breakpoint a second time");
    const auto execute_count_for_line = [&](std::size_t line) {
        return std::count_if(
            breakpoint_completed_state.events.begin(),
            breakpoint_completed_state.events.end(),
            [&](const copperfin::runtime::RuntimeEvent &event) {
                return event.category == "execute" && event.location.line == line;
            });
    };
    expect(execute_count_for_line(1U) == 1 && execute_count_for_line(3U) == 1,
           "compound RETURN debugger events should record each physical statement exactly once");

    const auto make_debug_session = [&]() {
        return copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(debugger_path.string(), temp_root.string(), true));
    };

    auto debug_session = make_debug_session();
    const auto entry_state = debug_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(entry_state.reason == copperfin::runtime::DebugPauseReason::entry,
           "compound RETURN debugger test should stop on entry");
    const auto child_state = debug_session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(child_state.reason == copperfin::runtime::DebugPauseReason::step && child_state.location.line == 3U,
           "step-into should pause at the child RETURN before it completes");
    const auto resumed_state = debug_session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(resumed_state.reason == copperfin::runtime::DebugPauseReason::step,
           "completing the child should leave the suspended caller available to the debugger");
    expect(resumed_state.location.line == 1U && resumed_state.statement_text == "RETURN child() + 1",
           "a suspended compound RETURN should retain its source location and statement text");
    expect(!resumed_state.call_stack.empty() && resumed_state.call_stack.front().line == 1U,
           "a suspended compound RETURN should report the same line in the top call-stack frame");
    const auto completed_state = debug_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(completed_state.completed, "continuing a debugger-paused compound RETURN should complete");

    auto cancel_session = make_debug_session();
    const auto cancel_entry_state = cancel_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    const auto cancel_child_state = cancel_session.run(copperfin::runtime::DebugResumeAction::step_into);
    const auto cancel_resumed_state = cancel_session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(cancel_entry_state.reason == copperfin::runtime::DebugPauseReason::entry &&
               cancel_child_state.location.line == 3U &&
               cancel_resumed_state.location.line == 1U,
           "cancellation setup should pause on the suspended caller RETURN");
    cancel_session.request_cancel();
    const auto cancelled_state = cancel_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(cancelled_state.reason == copperfin::runtime::DebugPauseReason::error,
           "cancelling a suspended compound RETURN should stop with the existing error contract");
    expect(cancelled_state.location.line == 1U && cancelled_state.statement_text == "RETURN child() + 1",
           "cancellation should retain the suspended compound RETURN source metadata");
    expect(!cancelled_state.call_stack.empty() && cancelled_state.call_stack.front().line == 1U,
           "cancellation should retain the suspended RETURN line in the top call-stack frame");

    fs::remove_all(temp_root, ignored);
}

void test_assignment_rhs_uses_heap_backed_expression_checkpoints() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_assignment_rhs";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path depth_path = temp_root / "assignment_depth_limit.prg";
    write_text(
        depth_path,
        "result = recurse(1)\n"
        "RETURN\n"
        "FUNCTION recurse\n"
        "LPARAMETERS depth\n"
        "nested = 1 + recurse(depth + 1)\n"
        "RETURN nested\n");

    auto depth_options = make_runtime_session_options(depth_path.string(), temp_root.string(), false);
    depth_options.max_call_depth = 2048U;
    auto depth_session = copperfin::runtime::PrgRuntimeSession::create(depth_options);
    const auto depth_state = depth_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(depth_state.reason == copperfin::runtime::DebugPauseReason::error,
           "deep recursive assignment RHS should stop at the runtime guardrail");
    expect(depth_state.message.find("maximum call depth") != std::string::npos,
           "deep recursive assignment RHS should report the configured call-depth diagnostic");

    const fs::path semantics_path = temp_root / "assignment_checkpoint_semantics.prg";
    write_text(
        semantics_path,
        "SET UDFPARMS TO REFERENCE\n"
        "seed = 7\n"
        "sequence = ''\n"
        "beforeCalls = 0\n"
        "innerCalls = 0\n"
        "outerCalls = 0\n"
        "afterCalls = 0\n"
        "referenceCalls = 0\n"
        "arrayReferenceCalls = 0\n"
        "arrayTargetCalls = 0\n"
        "fieldTargetCalls = 0\n"
        "objectTargetCalls = 0\n"
        "faultCalls = 0\n"
        "caughtCalls = 0\n"
        "finallyCalls = 0\n"
        "referenceValue = 3\n"
        "DIMENSION values[2]\n"
        "values[1] = 10\n"
        "values[2] = 20\n"
        "CREATE CURSOR assignmentFields (stored N(3,0))\n"
        "APPEND BLANK\n"
        "stored = 12\n"
        "oTarget = CREATEOBJECT('AssignmentTarget')\n"
        "memoryTarget = seed + ABS(-2) + before() + outer(inner()) + after()\n"
        "referenceTarget = explicitref(@referenceValue) + barearray(values)\n"
        "values[2] = arrayrhs()\n"
        "stored = fieldrhs()\n"
        "oTarget.FieldValue = objectrhs()\n"
        "faultTarget = 91\n"
        "TRY\n"
        "    faultTarget = faultvalue() + 1 / 0\n"
        "CATCH\n"
        "    caughtCalls = caughtCalls + 1\n"
        "ENDTRY\n"
        "TRY\n"
        "    finallyTarget = finalvalue()\n"
        "FINALLY\n"
        "    finallyCalls = finallyCalls + 1\n"
        "ENDTRY\n"
        "faultAfter = faultTarget\n"
        "objectAfter = oTarget.FieldValue\n"
        "referenceAfter = referenceValue\n"
        "arrayReferenceAfter = values[1]\n"
        "arrayTargetAfter = values[2]\n"
        "fieldAfter = stored\n"
        "seedAfter = seed\n"
        "RETURN\n"
        "FUNCTION before\n"
        "beforeCalls = beforeCalls + 1\n"
        "sequence = sequence + 'B'\n"
        "seed = 50\n"
        "RETURN 10\n"
        "FUNCTION inner\n"
        "innerCalls = innerCalls + 1\n"
        "sequence = sequence + 'I'\n"
        "RETURN 3\n"
        "FUNCTION outer\n"
        "LPARAMETERS value\n"
        "outerCalls = outerCalls + 1\n"
        "sequence = sequence + 'O'\n"
        "RETURN value * 10\n"
        "FUNCTION after\n"
        "afterCalls = afterCalls + 1\n"
        "sequence = sequence + 'A'\n"
        "RETURN seed\n"
        "FUNCTION explicitref\n"
        "LPARAMETERS target\n"
        "referenceCalls = referenceCalls + 1\n"
        "target = target + 2\n"
        "RETURN target\n"
        "FUNCTION barearray\n"
        "LPARAMETERS target\n"
        "arrayReferenceCalls = arrayReferenceCalls + 1\n"
        "target[1] = target[1] + 4\n"
        "RETURN target[1]\n"
        "FUNCTION arrayrhs\n"
        "arrayTargetCalls = arrayTargetCalls + 1\n"
        "RETURN 44\n"
        "FUNCTION objectrhs\n"
        "objectTargetCalls = objectTargetCalls + 1\n"
        "RETURN 55\n"
        "FUNCTION fieldrhs\n"
        "fieldTargetCalls = fieldTargetCalls + 1\n"
        "RETURN 77\n"
        "FUNCTION faultvalue\n"
        "faultCalls = faultCalls + 1\n"
        "RETURN 5\n"
        "FUNCTION finalvalue\n"
        "RETURN 66\n"
        "DEFINE CLASS AssignmentTarget AS Custom\n"
        "    FieldValue = 0\n"
        "ENDDEFINE\n");

    auto semantics_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(semantics_path.string(), temp_root.string(), false));
    const auto semantics_state = semantics_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(semantics_state.completed,
           "assignment checkpoint semantics script should complete: " + semantics_state.message);
    const auto expect_global = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = semantics_state.globals.find(name);
        expect(found != semantics_state.globals.end(), name + " should remain visible");
        if (found != semantics_state.globals.end()) {
            const std::string actual = copperfin::runtime::format_value(found->second);
            expect(actual == expected,
                   message + " (expected '" + expected + "', got '" + actual + "')");
        }
    };
    expect_global("memorytarget", "99", "assignment checkpoints should preserve prior reads and left-to-right values");
    expect_global("sequence", "BIOA", "nested assignment RHS calls should preserve left-to-right order");
    expect_global("referencetarget", "19", "explicit and bare-array references should preserve returned values");
    expect_global("referenceafter", "5", "explicit assignment RHS references should update caller storage once");
    expect_global("arrayreferenceafter", "14", "bare-array assignment RHS references should update caller storage once");
    expect_global("arraytargetafter", "44", "array element targets should receive the completed RHS once");
    expect_global("fieldafter", "77", "field targets should receive the completed RHS once");
    expect_global("objectafter", "55", "object member targets should receive the completed RHS once");
    expect_global("faultafter", "91", "a fault after RHS resumption should leave the assignment target untouched");
    expect_global("finallytarget", "66", "a successful suspended assignment should write before FINALLY completes");
    expect_global("seedafter", "50", "RHS side effects should remain visible after assignment completion");
    expect_global("beforecalls", "1", "an earlier assignment RHS routine should run once");
    expect_global("innercalls", "1", "a nested assignment RHS routine should run once");
    expect_global("outercalls", "1", "an outer assignment RHS routine should run once");
    expect_global("aftercalls", "1", "a later assignment RHS routine should run once");
    expect_global("referencecalls", "1", "an explicit-reference assignment RHS routine should run once");
    expect_global("arrayreferencecalls", "1", "a bare-array assignment RHS routine should run once");
    expect_global("arraytargetcalls", "1", "an array-target RHS routine should run once");
    expect_global("fieldtargetcalls", "1", "a field-target RHS routine should run once");
    expect_global("objecttargetcalls", "1", "an object-target RHS routine should run once");
    expect_global("faultcalls", "1", "a resumed faulting assignment RHS routine should run once");
    expect_global("caughtcalls", "1", "a resumed assignment fault should enter CATCH once");
    expect_global("finallycalls", "1", "a suspended assignment should run FINALLY once");
    expect(std::count_if(
               semantics_state.events.begin(),
               semantics_state.events.end(),
               [](const copperfin::runtime::RuntimeEvent &event) {
                   return event.category == "ole.set" &&
                       event.detail == "AssignmentTarget.FieldValue = 55";
               }) == 1,
           "a suspended object target should emit exactly one completed write event");

    const fs::path on_error_path = temp_root / "assignment_on_error.prg";
    write_text(
        on_error_path,
        "PUBLIC handlerCount, handlerLine, handlerRows, handlerStatement\n"
        "handlerCount = 0\n"
        "target = 73\n"
        "ON ERROR DO handleerr\n"
        "target = childvalue() + 1 / 0\n"
        "targetAfter = target\n"
        "afterError = 1\n"
        "RETURN\n"
        "FUNCTION childvalue\n"
        "RETURN 5\n"
        "PROCEDURE handleerr\n"
        "handlerCount = handlerCount + 1\n"
        "handlerLine = LINENO()\n"
        "handlerRows = AERROR(handlerError)\n"
        "handlerStatement = handlerError[1,7]\n"
        "RETURN\n");
    auto on_error_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(on_error_path.string(), temp_root.string(), false));
    const auto on_error_state = on_error_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(on_error_state.completed,
           "ON ERROR should handle a fault raised after assignment RHS resumption: " + on_error_state.message);
    const auto expect_on_error_global = [&](const std::string &name, const std::string &expected) {
        const auto found = on_error_state.globals.find(name);
        expect(found != on_error_state.globals.end(), name + " should be captured by the assignment handler");
        if (found != on_error_state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " should equal '" + expected + "'");
        }
    };
    expect_on_error_global("handlercount", "1");
    expect_on_error_global("handlerline", "5");
    expect_on_error_global("handlerrows", "1");
    expect_on_error_global("handlerstatement", "target = childvalue() + 1 / 0");
    expect_on_error_global("targetafter", "73");
    expect_on_error_global("aftererror", "1");

    const fs::path debugger_path = temp_root / "assignment_debugger.prg";
    write_text(
        debugger_path,
        "result = child() + 1\n"
        "RETURN\n"
        "FUNCTION child\n"
        "RETURN 2\n");

    auto exact_budget_options = make_runtime_session_options(debugger_path.string(), temp_root.string(), false);
    exact_budget_options.max_executed_statements = 3U;
    auto exact_budget_session = copperfin::runtime::PrgRuntimeSession::create(exact_budget_options);
    const auto exact_budget_state = exact_budget_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(exact_budget_state.completed && exact_budget_state.executed_statement_count == 3U,
           "assignment RHS resumption should not consume an extra statement-budget slot");

    auto exhausted_budget_options = exact_budget_options;
    exhausted_budget_options.max_executed_statements = 2U;
    auto exhausted_budget_session = copperfin::runtime::PrgRuntimeSession::create(exhausted_budget_options);
    const auto exhausted_budget_state = exhausted_budget_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(exhausted_budget_state.reason == copperfin::runtime::DebugPauseReason::error &&
               exhausted_budget_state.executed_statement_count == 2U &&
               exhausted_budget_state.location.line == 2U,
           "assignment budget exhaustion should stop after the write without double-counting its statement");

    auto breakpoint_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(debugger_path.string(), temp_root.string(), false));
    breakpoint_session.add_breakpoint({.file_path = debugger_path.string(), .line = 1U});
    const auto breakpoint_state = breakpoint_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(breakpoint_state.reason == copperfin::runtime::DebugPauseReason::breakpoint,
           "an assignment should honor its breakpoint before initial RHS evaluation");
    const auto breakpoint_completed_state = breakpoint_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(breakpoint_completed_state.completed,
           "a resumed assignment should not hit its already-consumed breakpoint again");
    const auto execute_count_for_line = [&](std::size_t line) {
        return std::count_if(
            breakpoint_completed_state.events.begin(),
            breakpoint_completed_state.events.end(),
            [&](const copperfin::runtime::RuntimeEvent &event) {
                return event.category == "execute" && event.location.line == line;
            });
    };
    expect(execute_count_for_line(1U) == 1 && execute_count_for_line(2U) == 1 &&
               execute_count_for_line(4U) == 1,
           "assignment debugger events should record each physical statement once");

    const auto make_debug_session = [&]() {
        return copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(debugger_path.string(), temp_root.string(), true));
    };
    auto debug_session = make_debug_session();
    const auto entry_state = debug_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    const auto child_state = debug_session.run(copperfin::runtime::DebugResumeAction::step_into);
    const auto resumed_state = debug_session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(entry_state.reason == copperfin::runtime::DebugPauseReason::entry,
           "assignment debugger test should stop on entry");
    expect(child_state.reason == copperfin::runtime::DebugPauseReason::step && child_state.location.line == 4U,
           "step-into should pause at the assignment RHS child RETURN");
    expect(resumed_state.reason == copperfin::runtime::DebugPauseReason::step &&
               resumed_state.location.line == 1U &&
               resumed_state.statement_text == "result = child() + 1",
           "a suspended assignment should retain its source metadata while paused");
    const auto completed_state = debug_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(completed_state.completed, "continuing a debugger-paused assignment should complete");

    auto step_over_session = make_debug_session();
    (void)step_over_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    const auto step_over_state =
        step_over_session.run(copperfin::runtime::DebugResumeAction::step_over);
    expect(step_over_state.reason == copperfin::runtime::DebugPauseReason::step &&
               step_over_state.location.line == 2U,
           "step-over should complete a suspended assignment before pausing at the next statement");
    const auto step_over_result = step_over_state.globals.find("result");
    expect(step_over_result != step_over_state.globals.end() &&
               copperfin::runtime::format_value(step_over_result->second) == "3",
           "step-over should apply a suspended assignment target before pausing");

    auto step_out_session = make_debug_session();
    (void)step_out_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    const auto step_out_child =
        step_out_session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(step_out_child.location.line == 4U,
           "step-out setup should enter the assignment RHS child");
    const auto step_out_state =
        step_out_session.run(copperfin::runtime::DebugResumeAction::step_out);
    expect(step_out_state.reason == copperfin::runtime::DebugPauseReason::step &&
               step_out_state.location.line == 2U,
           "step-out should finish the caller assignment before pausing at its next statement");
    const auto step_out_result = step_out_state.globals.find("result");
    expect(step_out_result != step_out_state.globals.end() &&
               copperfin::runtime::format_value(step_out_result->second) == "3",
           "step-out should apply the suspended caller target exactly once");

    auto cancel_session = make_debug_session();
    (void)cancel_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    (void)cancel_session.run(copperfin::runtime::DebugResumeAction::step_into);
    const auto cancel_resumed_state = cancel_session.run(copperfin::runtime::DebugResumeAction::step_into);
    expect(cancel_resumed_state.location.line == 1U,
           "cancellation setup should pause on the suspended assignment");
    cancel_session.request_cancel();
    const auto cancelled_state = cancel_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(cancelled_state.reason == copperfin::runtime::DebugPauseReason::error &&
               cancelled_state.location.line == 1U &&
               cancelled_state.statement_text == "result = child() + 1",
           "cancelling a suspended assignment should retain its source metadata");
    expect(cancelled_state.globals.find("result") == cancelled_state.globals.end(),
           "cancelling a suspended assignment should not apply its pending target write");

    const auto expect_direct_control_transfer_abandons_assignment =
        [&](const std::string &name, const std::string &command, const std::string &event_category) {
            const fs::path control_path = temp_root / (name + ".prg");
            write_text(
                control_path,
                "target = 88\n"
                "target = controlrhs()\n"
                "afterControl = 1\n"
                "RETURN\n"
                "FUNCTION controlrhs\n" +
                    command + "\n" +
                "RETURN 5\n");
            auto control_session = copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(control_path.string(), temp_root.string(), false));
            const auto control_state =
                control_session.run(copperfin::runtime::DebugResumeAction::continue_run);
            expect(control_state.completed,
                   command + " in an assignment RHS should terminate cleanly");
            const auto target = control_state.globals.find("target");
            expect(target != control_state.globals.end() &&
                       copperfin::runtime::format_value(target->second) == "88",
                   command + " should abandon the pending assignment target write");
            expect(control_state.globals.find("aftercontrol") == control_state.globals.end(),
                   command + " should prevent execution after the abandoned assignment");
            expect(std::count_if(
                       control_state.events.begin(),
                       control_state.events.end(),
                       [&](const copperfin::runtime::RuntimeEvent &event) {
                           return event.category == event_category;
                       }) == 1,
                   command + " should preserve its runtime event while abandoning the assignment");
        };
    expect_direct_control_transfer_abandons_assignment(
        "assignment_cancel_rhs", "CANCEL", "runtime.cancel");
    expect_direct_control_transfer_abandons_assignment(
        "assignment_quit_rhs", "QUIT", "runtime.quit");

    fs::remove_all(temp_root, ignored);
}


}  // namespace cf_test_prg_engine_control_flow
