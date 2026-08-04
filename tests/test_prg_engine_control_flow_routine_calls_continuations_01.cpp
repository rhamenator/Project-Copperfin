// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {

void test_deep_scalar_reference_forwarding_uses_heap_backed_frame_walk() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_deep_scalar_reference";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    constexpr std::size_t forwarding_depth = 768U;
    std::ostringstream script;
    script << "value = 1\n"
           << "DO forward1 WITH value\n"
           << "deepResult = value\n"
           << "RETURN\n";
    for (std::size_t depth = 1U; depth <= forwarding_depth; ++depth) {
        script << "PROCEDURE forward" << depth << "\n"
               << "LPARAMETERS forwarded\n";
        if (depth == forwarding_depth) {
            script << "forwarded = forwarded + 1\n";
        } else {
            script << "DO forward" << (depth + 1U) << " WITH forwarded\n";
        }
        script << "RETURN\n";
    }

    const fs::path main_path = temp_root / "deep_scalar_reference.prg";
    write_text(main_path, script.str());

    auto options = make_runtime_session_options(main_path.string(), temp_root.string(), false);
    options.max_call_depth = forwarding_depth + 8U;
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(options);

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "deep scalar reference forwarding should complete without host-stack propagation: " + state.message);

    const auto result = state.globals.find("deepresult");
    expect(result != state.globals.end(), "deep scalar reference forwarding should leave the caller result visible");
    if (result != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(result->second) == "2",
            "the deepest scalar alias should update the original caller storage exactly once");
    }

    fs::remove_all(temp_root, ignored);
}

void test_direct_recursive_return_uses_heap_backed_frame_continuations() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_direct_recursive_return";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path depth_path = temp_root / "depth_limit.prg";
    write_text(
        depth_path,
        "result = recurse(1)\n"
        "RETURN\n"
        "FUNCTION recurse\n"
        "LPARAMETERS depth\n"
        "RETURN recurse(depth + 1)\n");

    auto depth_options = make_runtime_session_options(depth_path.string(), temp_root.string(), false);
    depth_options.max_call_depth = 2048U;
    copperfin::runtime::PrgRuntimeSession depth_session =
        copperfin::runtime::PrgRuntimeSession::create(depth_options);
    const auto depth_state = depth_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(
        depth_state.reason == copperfin::runtime::DebugPauseReason::error,
        "deep direct recursive RETURN should stop at the runtime guardrail");
    expect(
        depth_state.message.find("maximum call depth") != std::string::npos,
        "deep direct recursive RETURN should report the configured call-depth diagnostic");

    const fs::path semantics_path = temp_root / "continuation_semantics.prg";
    write_text(
        semantics_path,
        "PUBLIC capturedfirst, capturedsecond\n"
        "SET UDFPARMS TO REFERENCE\n"
        "scalarValue = 2\n"
        "argumentCalls = 0\n"
        "finallyCalls = 0\n"
        "caughtCalls = 0\n"
        "DIMENSION values[2]\n"
        "values[1] = 10\n"
        "values[2] = 20\n"
        "DIMENSION namedLikeRoutine[1]\n"
        "namedLikeRoutine[1] = 6\n"
        "result = outer(@scalarValue, @values)\n"
        "builtinResult = returnbuiltin()\n"
        "arrayResult = returnarray()\n"
        "caughtResult = catchchildfault()\n"
        "scalarAfter = scalarValue\n"
        "arrayOneAfter = values[1]\n"
        "arrayTwoAfter = values[2]\n"
        "RETURN\n"
        "FUNCTION outer\n"
        "LPARAMETERS forwardedScalar, forwardedValues\n"
        "forwardedScalar = forwardedScalar + 3\n"
        "forwardedValues[1] = forwardedValues[1] + 4\n"
        "TRY\n"
        "RETURN inner(countargument(forwardedScalar), forwardedValues)\n"
        "FINALLY\n"
        "finallyCalls = finallyCalls + 1\n"
        "ENDTRY\n"
        "FUNCTION countargument\n"
        "LPARAMETERS value\n"
        "argumentCalls = argumentCalls + 1\n"
        "RETURN value\n"
        "FUNCTION inner\n"
        "LPARAMETERS receivedScalar, receivedValues\n"
        "receivedValues[2] = receivedValues[2] + 5\n"
        "RETURN receivedScalar + receivedValues[1] + receivedValues[2]\n"
        "FUNCTION returnbuiltin\n"
        "RETURN ABS(-7)\n"
        "FUNCTION abs\n"
        "RETURN 99\n"
        "FUNCTION returnarray\n"
        "RETURN namedLikeRoutine(1)\n"
        "FUNCTION namedLikeRoutine\n"
        "RETURN 99\n"
        "FUNCTION catchchildfault\n"
        "TRY\n"
        "RETURN throwfromchild()\n"
        "CATCH\n"
        "caughtCalls = caughtCalls + 1\n"
        "ENDTRY\n"
        "RETURN 42\n"
        "FUNCTION throwfromchild\n"
        "THROW 'child fault'\n");

    copperfin::runtime::PrgRuntimeSession semantics_session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(semantics_path.string(), temp_root.string(), false));
    const auto semantics_state = semantics_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(
        semantics_state.completed,
        "direct-return continuation semantics script should complete: " + semantics_state.message);

    const auto expect_global = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = semantics_state.globals.find(name);
        expect(found != semantics_state.globals.end(), name + " should remain visible");
        if (found != semantics_state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };
    expect_global("result", "44", "the continued RETURN should preserve the nested routine result");
    expect_global("builtinresult", "7", "built-ins should retain precedence over same-named user routines");
    expect_global("arrayresult", "6", "array access should retain precedence over same-named user routines");
    expect_global("caughtresult", "42", "a caller CATCH should cancel its aborted direct-return continuation");
    expect_global("scalarafter", "5", "the explicit scalar reference should reach caller storage");
    expect_global("arrayoneafter", "14", "the direct caller array mutation should remain visible");
    expect_global("arraytwoafter", "25", "SET UDFPARMS reference forwarding should retain the array alias");
    expect_global("argumentcalls", "1", "a suspended direct-return argument should be evaluated exactly once");
    expect_global("finallycalls", "1", "the suspended direct return should run FINALLY exactly once");
    expect_global("caughtcalls", "1", "a deferred child fault should enter the caller CATCH exactly once");

    fs::remove_all(temp_root, ignored);
}

void test_standalone_expression_uses_heap_backed_frame_continuations() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_standalone_expression";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path depth_path = temp_root / "expression_depth_limit.prg";
    write_text(
        depth_path,
        "? recurse(1)\n"
        "RETURN\n"
        "FUNCTION recurse\n"
        "LPARAMETERS depth\n"
        "? recurse(depth + 1)\n");

    auto depth_options = make_runtime_session_options(depth_path.string(), temp_root.string(), false);
    depth_options.max_call_depth = 2048U;
    copperfin::runtime::PrgRuntimeSession depth_session =
        copperfin::runtime::PrgRuntimeSession::create(depth_options);
    const auto depth_state = depth_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(
        depth_state.reason == copperfin::runtime::DebugPauseReason::error,
        "deep standalone print recursion should stop at the runtime guardrail");
    expect(
        depth_state.message.find("maximum call depth") != std::string::npos,
        "deep standalone print recursion should report the configured call-depth diagnostic");

    const fs::path semantics_path = temp_root / "expression_semantics.prg";
    write_text(
        semantics_path,
        "calls = 0\n"
        "? report()\n"
        "report()\n"
        "after = calls\n"
        "WAIT WINDOW prompt()\n"
        "RETURN\n"
        "FUNCTION report\n"
        "calls = calls + 1\n"
        "RETURN 42\n"
        "FUNCTION prompt\n"
        "calls = calls + 1\n"
        "RETURN 'hello'\n");

    copperfin::runtime::PrgRuntimeSession semantics_session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(semantics_path.string(), temp_root.string(), false));
    const auto semantics_state = semantics_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(semantics_state.completed, "standalone expression semantics script should complete: " + semantics_state.message);

    const auto after = semantics_state.globals.find("after");
    expect(after != semantics_state.globals.end(), "standalone expression calls should leave the post-call count visible");
    if (after != semantics_state.globals.end()) {
        expect(
            copperfin::runtime::format_value(after->second) == "2",
            "a print command and a bare expression statement should each invoke their UDF once");
    }

    const std::size_t print_count = static_cast<std::size_t>(std::count_if(
        semantics_state.events.begin(),
        semantics_state.events.end(),
        [](const copperfin::runtime::RuntimeEvent &event) {
            return event.category == "runtime.print" && event.detail == "42";
        }));
    expect(print_count == 1U, "deferred print expression evaluation should emit exactly one runtime.print event");

    const std::size_t wait_window_count = static_cast<std::size_t>(std::count_if(
        semantics_state.events.begin(),
        semantics_state.events.end(),
        [](const copperfin::runtime::RuntimeEvent &event) {
            return event.category == "runtime.wait" &&
                   event.detail.find("mode=WINDOW") != std::string::npos &&
                   event.detail.find("prompt=hello") != std::string::npos;
        }));
    expect(wait_window_count == 1U, "deferred WAIT WINDOW evaluation should emit exactly one runtime.wait event");

    fs::remove_all(temp_root, ignored);
}

void test_store_expression_uses_heap_backed_frame_continuations() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_store_expression";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path depth_path = temp_root / "store_depth_limit.prg";
    write_text(
        depth_path,
        "STORE recurse(1) TO result\n"
        "RETURN\n"
        "FUNCTION recurse\n"
        "LPARAMETERS depth\n"
        "STORE recurse(depth + 1) TO result\n");

    auto depth_options = make_runtime_session_options(depth_path.string(), temp_root.string(), false);
    depth_options.max_call_depth = 2048U;
    copperfin::runtime::PrgRuntimeSession depth_session =
        copperfin::runtime::PrgRuntimeSession::create(depth_options);
    const auto depth_state = depth_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(
        depth_state.reason == copperfin::runtime::DebugPauseReason::error,
        "deep STORE expression recursion should stop at the runtime guardrail");
    expect(
        depth_state.message.find("maximum call depth") != std::string::npos,
        "deep STORE expression recursion should report the configured call-depth diagnostic");

    const fs::path semantics_path = temp_root / "store_expression_semantics.prg";
    write_text(
        semantics_path,
        "calls = 0\n"
        "STORE value() TO first, second\n"
        "after = calls\n"
        "RETURN\n"
        "FUNCTION value\n"
        "calls = calls + 1\n"
        "RETURN 42\n");

    copperfin::runtime::PrgRuntimeSession semantics_session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(semantics_path.string(), temp_root.string(), false));
    const auto semantics_state = semantics_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(semantics_state.completed, "STORE expression semantics script should complete: " + semantics_state.message);

    const auto after = semantics_state.globals.find("after");
    expect(after != semantics_state.globals.end(), "STORE expression UDF calls should leave the post-call count visible");
    if (after != semantics_state.globals.end()) {
        expect(
            copperfin::runtime::format_value(after->second) == "1",
            "STORE should evaluate its expression exactly once before writing all targets");
    }

    for (const char *name : {"first", "second"})
    {
        const auto value = semantics_state.globals.find(name);
        expect(value != semantics_state.globals.end(), std::string{"STORE should assign its multi-target "} + name + " binding");
        if (value != semantics_state.globals.end()) {
            expect(
                copperfin::runtime::format_value(value->second) == "42",
                "STORE should write the completed UDF value to every target");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_do_with_arguments_use_heap_backed_frame_continuations() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_do_argument_continuation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path depth_path = temp_root / "do_argument_depth_limit.prg";
    write_text(
        depth_path,
        "DO capture WITH recurse_argument(1)\n"
        "RETURN\n"
        "FUNCTION recurse_argument\n"
        "LPARAMETERS nDepth\n"
        "RETURN recurse_argument(nDepth + 1)\n"
        "PROCEDURE capture\n"
        "LPARAMETERS value\n"
        "RETURN\n");

    auto depth_options = make_runtime_session_options(depth_path.string(), temp_root.string(), false);
    depth_options.max_call_depth = 2048U;
    copperfin::runtime::PrgRuntimeSession depth_session =
        copperfin::runtime::PrgRuntimeSession::create(depth_options);
    const auto depth_state = depth_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(
        depth_state.reason == copperfin::runtime::DebugPauseReason::error,
        "deep DO argument recursion should stop at the runtime guardrail");
    expect(
        depth_state.message.find("maximum call depth") != std::string::npos,
        "deep DO argument recursion should report the configured call-depth diagnostic");

    const fs::path semantics_path = temp_root / "do_argument_semantics.prg";
    write_text(
        semantics_path,
        "PUBLIC capturedfirst, capturedsecond\n"
        "SET UDFPARMS TO REFERENCE\n"
        "first = 2\n"
        "second = 3\n"
        "calls = 0\n"
        "order = ''\n"
        "DO capture WITH record_call(first), @first, record_call(second)\n"
        "afterFirst = first\n"
        "afterCalls = calls\n"
        "afterOrder = order\n"
        "RETURN\n"
        "FUNCTION record_call\n"
        "LPARAMETERS value\n"
        "calls = calls + 1\n"
        "IF value = 2\n"
        "order = order + 'A'\n"
        "ELSE\n"
        "order = order + 'B'\n"
        "ENDIF\n"
        "RETURN value + 10\n"
        "PROCEDURE capture\n"
        "LPARAMETERS firstValue, forwarded, secondValue\n"
        "capturedFirst = firstValue\n"
        "forwarded = forwarded + 5\n"
        "capturedSecond = secondValue\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession semantics_session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(semantics_path.string(), temp_root.string(), false));
    const auto semantics_state = semantics_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(semantics_state.completed, "DO argument continuation semantics script should complete: " + semantics_state.message);

    const auto expect_global = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = semantics_state.globals.find(name);
        expect(found != semantics_state.globals.end(), name + " should remain visible");
        if (found != semantics_state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };
    expect_global("afterfirst", "7", "explicit @ reference should update the caller variable after all DO arguments are evaluated");
    expect_global("aftercalls", "2", "DO UDF arguments should each execute exactly once");
    expect_global("afterorder", "AB", "DO UDF arguments should preserve left-to-right evaluation order");
    expect_global("capturedfirst", "12", "the first completed DO argument should bind its returned value");
    expect_global("capturedsecond", "13", "the later completed DO argument should bind its returned value");

    fs::remove_all(temp_root, ignored);
}

void test_call_with_arguments_use_heap_backed_frame_continuations() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_call_argument_continuation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path depth_path = temp_root / "call_argument_depth_limit.prg";
    write_text(
        depth_path,
        "CALL capture WITH recurse_argument(1)\n"
        "RETURN\n"
        "FUNCTION recurse_argument\n"
        "LPARAMETERS nDepth\n"
        "RETURN recurse_argument(nDepth + 1)\n"
        "PROCEDURE capture\n"
        "LPARAMETERS value\n"
        "RETURN\n");

    auto depth_options = make_runtime_session_options(depth_path.string(), temp_root.string(), false);
    depth_options.max_call_depth = 2048U;
    copperfin::runtime::PrgRuntimeSession depth_session =
        copperfin::runtime::PrgRuntimeSession::create(depth_options);
    const auto depth_state = depth_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(
        depth_state.reason == copperfin::runtime::DebugPauseReason::error,
        "deep CALL argument recursion should stop at the runtime guardrail");
    expect(
        depth_state.message.find("maximum call depth") != std::string::npos,
        "deep CALL argument recursion should report the configured call-depth diagnostic");

    const fs::path semantics_path = temp_root / "call_argument_semantics.prg";
    write_text(
        semantics_path,
        "PUBLIC capturedfirst, capturedsecond\n"
        "SET UDFPARMS TO REFERENCE\n"
        "first = 2\n"
        "second = 3\n"
        "calls = 0\n"
        "order = ''\n"
        "CALL capture WITH record_call(first), @first, record_call(second)\n"
        "afterFirst = first\n"
        "afterCalls = calls\n"
        "afterOrder = order\n"
        "RETURN\n"
        "FUNCTION record_call\n"
        "LPARAMETERS value\n"
        "calls = calls + 1\n"
        "IF value = 2\n"
        "order = order + 'A'\n"
        "ELSE\n"
        "order = order + 'B'\n"
        "ENDIF\n"
        "RETURN value + 10\n"
        "PROCEDURE capture\n"
        "LPARAMETERS firstValue, forwarded, secondValue\n"
        "capturedFirst = firstValue\n"
        "forwarded = forwarded + 5\n"
        "capturedSecond = secondValue\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession semantics_session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(semantics_path.string(), temp_root.string(), false));
    const auto semantics_state = semantics_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(semantics_state.completed, "CALL argument continuation semantics script should complete: " + semantics_state.message);

    const auto expect_global = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = semantics_state.globals.find(name);
        expect(found != semantics_state.globals.end(), name + " should remain visible");
        if (found != semantics_state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };
    expect_global("afterfirst", "7", "CALL explicit @ reference should update the caller variable");
    expect_global("aftercalls", "2", "CALL UDF arguments should each execute exactly once");
    expect_global("afterorder", "AB", "CALL UDF arguments should preserve left-to-right order");
    expect_global("capturedfirst", "12", "CALL should bind the first completed argument value");
    expect_global("capturedsecond", "13", "CALL should bind the later completed argument value");

    fs::remove_all(temp_root, ignored);
}


}  // namespace cf_test_prg_engine_control_flow
