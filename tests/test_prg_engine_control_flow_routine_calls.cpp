// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {
void test_do_with_parameters_binds_arguments_in_called_routine() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_do_with_parameters";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "do_with_parameters.prg";
    write_text(
        main_path,
        "DO addvals WITH 4, 5\n"
        "RETURN\n"
        "PROCEDURE addvals\n"
        "LPARAMETERS a, b\n"
        "sum_result = a + b\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DO WITH LPARAMETERS script should complete");

    const auto sum_result = state.globals.find("sum_result");
    expect(sum_result != state.globals.end(), "called routine should assign sum_result");
    if (sum_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_result->second) == "9", "DO WITH arguments should bind to LPARAMETERS");
    }

    fs::remove_all(temp_root, ignored);
}

void test_call_with_parameters_binds_arguments_in_called_routine() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_call_with_parameters";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "call_with_parameters.prg";
    write_text(
        main_path,
        "CALL addvals WITH 7, 8\n"
        "RETURN\n"
        "PROCEDURE addvals\n"
        "LPARAMETERS a, b\n"
        "sum_result = a + b\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CALL WITH LPARAMETERS script should complete");

    const auto sum_result = state.globals.find("sum_result");
    expect(sum_result != state.globals.end(), "CALL routine should assign sum_result");
    if (sum_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_result->second) == "15", "CALL WITH arguments should bind to LPARAMETERS");
    }

    fs::remove_all(temp_root, ignored);
}

void test_unsupplied_parameters_initialize_to_logical_false() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_unsupplied_parameters";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "unsupplied_parameters.prg";
    write_text(
        main_path,
        "DO inspectlocal WITH 'supplied'\n"
        "cPrivateType = inspectprivate('supplied')\n"
        "cInlineType = inspectinline('supplied')\n"
        "oProbe = CREATEOBJECT('ParameterProbe')\n"
        "cMethodType = oProbe.Inspect('supplied')\n"
        "RETURN\n"
        "PROCEDURE inspectlocal\n"
        "LPARAMETERS supplied, omitted, explicitDefault = 17\n"
        "cLocalType = VARTYPE(omitted)\n"
        "lLocalValue = omitted\n"
        "nLocalPCount = PCOUNT()\n"
        "nExplicitDefault = explicitDefault\n"
        "RETURN\n"
        "FUNCTION inspectprivate\n"
        "PARAMETERS supplied, omitted\n"
        "RETURN VARTYPE(omitted)\n"
        "ENDFUNC\n"
        "FUNCTION inspectinline(supplied, omitted)\n"
        "RETURN VARTYPE(omitted)\n"
        "ENDFUNC\n"
        "DEFINE CLASS ParameterProbe AS Custom\n"
        "    FUNCTION Inspect\n"
        "        LPARAMETERS supplied, omitted\n"
        "        RETURN VARTYPE(omitted)\n"
        "    ENDFUNC\n"
        "ENDDEFINE\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "unsupplied parameter script should complete: " + state.message);

    const auto expect_formatted = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should remain visible");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };

    expect_formatted("clocaltype", "L", "unsupplied LPARAMETERS formal should have logical type");
    const auto local_value = state.globals.find("llocalvalue");
    expect(local_value != state.globals.end(), "unsupplied LPARAMETERS value should remain visible");
    if (local_value != state.globals.end()) {
        expect(local_value->second.kind == copperfin::runtime::PrgValueKind::boolean,
               "unsupplied LPARAMETERS value should be logical");
        expect(!local_value->second.boolean_value, "unsupplied LPARAMETERS value should be false");
    }
    expect_formatted("nlocalpcount", "1", "PCOUNT() should retain the actual caller argument count");
    expect_formatted("nexplicitdefault", "17", "explicit parameter defaults should remain unchanged");
    expect_formatted("cprivatetype", "L", "unsupplied PARAMETERS formal should have logical type");
    expect_formatted("cinlinetype", "L", "unsupplied inline formal should have logical type");
    expect_formatted("cmethodtype", "L", "unsupplied object-method formal should have logical type");

    fs::remove_all(temp_root, ignored);
}

void test_call_external_target_with_by_reference_updates_caller_variable() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_call_external_byref";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path helper_path = temp_root / "helper.prg";
    write_text(
        helper_path,
        "LPARAMETERS pcount\n"
        "pcount = pcount + 5\n"
        "RETURN\n");

    const fs::path main_path = temp_root / "call_external_byref.prg";
    write_text(
        main_path,
        "counter = 3\n"
        "CALL helper WITH @counter\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CALL external WITH @var script should complete");

    const auto counter = state.globals.find("counter");
    expect(counter != state.globals.end(), "caller variable should exist after CALL external BYREF");
    if (counter != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(counter->second) == "8",
            "CALL external target should resolve .prg path and write BYREF updates back to caller");
    }

    fs::remove_all(temp_root, ignored);
}

void test_do_with_by_reference_updates_caller_variable() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_do_with_byref";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "do_with_byref.prg";
    write_text(
        main_path,
        "counter = 1\n"
        "DO bump WITH @counter\n"
        "RETURN\n"
        "PROCEDURE bump\n"
        "LPARAMETERS pcount\n"
        "pcount = pcount + 1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DO WITH @var script should complete");

    const auto counter = state.globals.find("counter");
    expect(counter != state.globals.end(), "caller variable should still exist after BYREF call");
    if (counter != state.globals.end()) {
        expect(
            copperfin::runtime::format_value(counter->second) == "2",
            "BYREF argument binding should write callee updates back to the caller");
    }

    fs::remove_all(temp_root, ignored);
}

void test_expression_level_function_call_assigns_return_value() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_expr_function_assign";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "expr_function_assign.prg";
    write_text(
        main_path,
        "result = addvals(4, 5)\n"
        "RETURN\n"
        "FUNCTION addvals\n"
        "LPARAMETERS a, b\n"
        "RETURN a + b\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "expression-level FUNCTION assignment script should complete");

    const auto result = state.globals.find("result");
    expect(result != state.globals.end(), "expression-level FUNCTION call should assign result");
    if (result != state.globals.end()) {
        expect(copperfin::runtime::format_value(result->second) == "9",
               "expression-level FUNCTION call should return its evaluated RETURN expression");
    }

    fs::remove_all(temp_root, ignored);
}

void test_expression_level_procedure_call_assigns_return_value() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_expr_procedure_assign";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "expr_procedure_assign.prg";
    write_text(
        main_path,
        "result = addvalsproc(4, 5)\n"
        "RETURN\n"
        "PROCEDURE addvalsproc\n"
        "LPARAMETERS a, b\n"
        "RETURN a + b\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "expression-level PROCEDURE assignment script should complete");

    const auto result = state.globals.find("result");
    expect(result != state.globals.end(), "expression-level PROCEDURE call should assign result");
    if (result != state.globals.end()) {
        expect(copperfin::runtime::format_value(result->second) == "9",
               "expression-level PROCEDURE call should return its evaluated RETURN expression");
    }

    fs::remove_all(temp_root, ignored);
}

void test_expression_level_function_call_supports_by_reference_arguments() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_expr_function_byref";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "expr_function_byref.prg";
    write_text(
        main_path,
        "counter = 2\n"
        "result = bump(@counter)\n"
        "RETURN\n"
        "FUNCTION bump\n"
        "LPARAMETERS pcount\n"
        "pcount = pcount + 3\n"
        "RETURN pcount * 2\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "expression-level FUNCTION BYREF script should complete");

    const auto result = state.globals.find("result");
    expect(result != state.globals.end(), "expression-level FUNCTION BYREF should assign result");
    if (result != state.globals.end()) {
        expect(copperfin::runtime::format_value(result->second) == "10",
               "expression-level FUNCTION BYREF should return the callee RETURN value");
    }

    const auto counter = state.globals.find("counter");
    expect(counter != state.globals.end(), "expression-level FUNCTION BYREF should preserve caller variable");
    if (counter != state.globals.end()) {
        expect(copperfin::runtime::format_value(counter->second) == "5",
               "expression-level FUNCTION BYREF should write callee updates back to the caller");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_udfparms_controls_expression_routine_parameter_aliasing() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_udfparms";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "set_udfparms.prg";
    write_text(
        main_path,
        "DIMENSION aValue[2], aReference[2], aExplicit[2]\n"
        "aValue[1] = 10\n"
        "aValue[2] = 20\n"
        "aReference[1] = 30\n"
        "aReference[2] = 40\n"
        "aExplicit[1] = 50\n"
        "aExplicit[2] = 60\n"
        "scalarValue = 5\n"
        "defaultMode = SET('UDFPARMS')\n"
        "defaultResult = bump(scalarValue)\n"
        "defaultScalar = scalarValue\n"
        "defaultArrayResult = bump(m.aValue)\n"
        "defaultArrayOne = aValue[1]\n"
        "defaultArrayTwo = aValue[2]\n"
        "SET UDFPARMS TO REFERENCE\n"
        "referenceMode = SET('UDFP')\n"
        "referenceResult = bump(scalarValue)\n"
        "referenceScalar = scalarValue\n"
        "parenthesizedResult = bump((scalarValue))\n"
        "parenthesizedScalar = scalarValue\n"
        "expressionResult = bump(scalarValue + 10)\n"
        "expressionScalar = scalarValue\n"
        "literalResult = bump(90)\n"
        "referenceArrayResult = mutatearray(aReference)\n"
        "referenceArrayOne = aReference[1]\n"
        "referenceArrayTwo = aReference[2]\n"
        "sameAliasValue = 1\n"
        "sameAliasResult = mutatetwo(sameAliasValue, sameAliasValue)\n"
        "sameAliasAfter = sameAliasValue\n"
        "nestedAliasValue = 2\n"
        "nestedAliasResult = forwardaliases(nestedAliasValue, nestedAliasValue)\n"
        "nestedAliasAfter = nestedAliasValue\n"
        "indirectAliasValue = 8\n"
        "indirectAliasResult = readindirectalias(indirectAliasValue)\n"
        "indirectAliasAfter = indirectAliasValue\n"
        "TRY\n"
        "    ignoredFaultResult = faulting(scalarValue)\n"
        "CATCH\n"
        "    modeAfterFault = SET('UDFPARMS')\n"
        "ENDTRY\n"
        "afterFaultResult = bump(scalarValue)\n"
        "afterFaultScalar = scalarValue\n"
        "SET UDFPARMS TO VALUE\n"
        "valueMode = SET('UDFPARMS')\n"
        "explicitResult = bump(@scalarValue)\n"
        "explicitScalar = scalarValue\n"
        "qualifiedValue = 70\n"
        "qualifiedResult = bump(@m.qualifiedValue)\n"
        "qualifiedScalar = qualifiedValue\n"
        "explicitArrayResult = mutatearraydirect(@m.aExplicit)\n"
        "explicitArrayOne = aExplicit[1]\n"
        "explicitArrayTwo = aExplicit[2]\n"
        "DO bumpprocedure WITH m.scalarValue\n"
        "doScalar = scalarValue\n"
        "callValue = 30\n"
        "CALL bumpprocedure WITH m.callValue\n"
        "callScalar = callValue\n"
        "RETURN\n"
        "FUNCTION bump\n"
        "LPARAMETERS value\n"
        "value = value + 1\n"
        "RETURN value\n"
        "FUNCTION mutatearray\n"
        "LPARAMETERS values\n"
        "values[1] = values[1] + 1\n"
        "RETURN forwardarray(values)\n"
        "FUNCTION forwardarray\n"
        "LPARAMETERS forwarded\n"
        "forwarded[2] = forwarded[2] + 2\n"
        "RETURN ALEN(forwarded)\n"
        "FUNCTION mutatearraydirect\n"
        "LPARAMETERS values\n"
        "values[1] = values[1] + 3\n"
        "values[2] = values[2] + 4\n"
        "RETURN ALEN(values)\n"
        "FUNCTION mutatetwo\n"
        "LPARAMETERS firstValue, secondValue\n"
        "firstValue = firstValue + 1\n"
        "secondValue = secondValue + 2\n"
        "RETURN firstValue * 10 + secondValue\n"
        "FUNCTION forwardaliases\n"
        "LPARAMETERS outerFirst, outerSecond\n"
        "RETURN mutatetwo(outerFirst, outerSecond)\n"
        "FUNCTION readindirectalias\n"
        "LPARAMETERS aliased\n"
        "DO mutateindirectglobal\n"
        "RETURN aliased\n"
        "FUNCTION faulting\n"
        "LPARAMETERS value\n"
        "value = value + 10\n"
        "DO missing_udfparms_fault_target\n"
        "RETURN value\n"
        "PROCEDURE bumpprocedure\n"
        "LPARAMETERS value\n"
        "value = value + 5\n"
        "RETURN\n"
        "PROCEDURE mutateindirectglobal\n"
        "indirectAliasValue = indirectAliasValue + 2\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SET UDFPARMS compatibility script should complete: " + state.message);

    const auto expect_global = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should remain visible");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };

    expect_global("defaultmode", "VALUE", "new sessions should default SET UDFPARMS to VALUE");
    expect_global("defaultresult", "6", "VALUE mode should still pass the scalar value into the UDF");
    expect_global("defaultscalar", "5", "VALUE mode should not alias a bare scalar");
    expect_global("defaultarrayresult", "11", "VALUE mode should pass m.-qualified bare array element 1 by value");
    expect_global("defaultarrayone", "10", "VALUE mode should not alias a bare array");
    expect_global("defaultarraytwo", "20", "VALUE mode should not expose the rest of a bare array");
    expect_global("referencemode", "REFERENCE", "SET('UDFP') should query REFERENCE mode");
    expect_global("referenceresult", "6", "REFERENCE mode should return the mutated scalar");
    expect_global("referencescalar", "6", "REFERENCE mode should alias an eligible bare scalar");
    expect_global("parenthesizedresult", "7", "parentheses should force value passing in REFERENCE mode");
    expect_global("parenthesizedscalar", "6", "parenthesized variables should not alias caller storage");
    expect_global("expressionresult", "17", "expressions should remain values in REFERENCE mode");
    expect_global("expressionscalar", "6", "expression arguments should not alias their input variables");
    expect_global("literalresult", "91", "literal arguments should remain values in REFERENCE mode");
    expect_global("referencearrayresult", "2", "REFERENCE mode should expose the full bare array");
    expect_global("referencearrayone", "31", "REFERENCE mode should alias direct array element writes");
    expect_global("referencearraytwo", "42", "REFERENCE mode should preserve nested bare-array forwarding");
    expect_global("samealiasresult", "44", "multiple formals bound to one scalar should observe one aliased value");
    expect_global("samealiasafter", "4", "multiple scalar aliases should update one caller binding immediately");
    expect_global("nestedaliasresult", "55", "nested forwarding should preserve one coherent scalar alias");
    expect_global("nestedaliasafter", "5", "nested scalar aliases should update the original caller binding");
    expect_global("indirectaliasresult", "10", "scalar aliases should observe direct writes to their caller storage");
    expect_global("indirectaliasafter", "10", "direct caller-storage writes should not be overwritten by stale alias caches");
    expect_global("modeafterfault", "REFERENCE", "caught UDF faults should preserve SET UDFPARMS state");
    expect_global("afterfaultresult", "17", "UDF calls should continue using REFERENCE mode after a caught fault");
    expect_global("afterfaultscalar", "17", "post-fault reference updates should reach caller storage");
    expect_global("valuemode", "VALUE", "SET UDFPARMS TO VALUE should restore value mode in the same session");
    expect_global("explicitresult", "18", "explicit @ should force scalar reference passing in VALUE mode");
    expect_global("explicitscalar", "18", "explicit @ should alias scalar caller storage in VALUE mode");
    expect_global("qualifiedresult", "71", "explicit @ should accept an m.-qualified scalar in VALUE mode");
    expect_global("qualifiedscalar", "71", "m.-qualified references should alias caller storage");
    expect_global("explicitarrayresult", "2", "explicit @ should expose an m.-qualified full array in VALUE mode");
    expect_global("explicitarrayone", "53", "explicit @ should alias the first array element in VALUE mode");
    expect_global("explicitarraytwo", "64", "explicit @ should alias the second array element in VALUE mode");
    expect_global("doscalar", "23", "DO WITH should keep m.-qualified variables reference-default independently of SET UDFPARMS");
    expect_global("callscalar", "35", "CALL WITH should keep m.-qualified variables reference-default independently of SET UDFPARMS");

    fs::remove_all(temp_root, ignored);
}

void test_set_udfparms_state_is_isolated_between_data_and_runtime_sessions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_udfparms_isolation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path first_path = temp_root / "first_session.prg";
    write_text(
        first_path,
        "SET UDFPARMS TO REFERENCE\n"
        "sessionOneMode = SET('UDFPARMS')\n"
        "SET DATASESSION TO 2\n"
        "dataSessionTwoInherited = SET('UDFPARMS')\n"
        "SET UDFPARMS TO VALUE\n"
        "dataSessionTwoChanged = SET('UDFPARMS')\n"
        "SET DATASESSION TO 1\n"
        "dataSessionOneSeesChange = SET('UDFPARMS')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession first_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(first_path.string(), temp_root.string(), false));
    const auto first_state = first_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(first_state.completed, "first SET UDFPARMS isolation session should complete: " + first_state.message);

    const auto second_path = temp_root / "second_session.prg";
    write_text(
        second_path,
        "freshRuntimeMode = SET('UDFPARMS')\n"
        "value = 2\n"
        "result = bump(value)\n"
        "valueAfter = value\n"
        "RETURN\n"
        "FUNCTION bump\n"
        "LPARAMETERS value\n"
        "value = value + 1\n"
        "RETURN value\n");

    copperfin::runtime::PrgRuntimeSession second_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(second_path.string(), temp_root.string(), false));
    const auto second_state = second_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(second_state.completed, "second SET UDFPARMS isolation session should complete: " + second_state.message);

    const auto expect_global = [](const auto &state, const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should remain visible");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };

    expect_global(first_state, "sessiononemode", "REFERENCE", "the first data session should retain its selected mode");
    expect_global(first_state, "datasessiontwoinherited", "REFERENCE", "UDFPARMS should remain runtime-session-wide across data sessions");
    expect_global(first_state, "datasessiontwochanged", "VALUE", "a data-session change should update the runtime-session-wide mode");
    expect_global(first_state, "datasessiononeseeschange", "VALUE", "returning to a data session should keep the session-wide mode");
    expect_global(second_state, "freshruntimemode", "VALUE", "a new runtime session should not inherit UDFPARMS state");
    expect_global(second_state, "result", "3", "a fresh runtime session should evaluate bare scalar UDF arguments by value");
    expect_global(second_state, "valueafter", "2", "a fresh runtime session should not alias bare scalar UDF arguments");

    fs::remove_all(temp_root, ignored);
}

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

void test_array_parameters_alias_caller_storage_across_nested_function_calls() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_expr_array_byref";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "expr_array_byref.prg";
    write_text(
        main_path,
        "DIMENSION aData[2], aOther[2], aBare[2]\n"
        "aData[1] = 1\n"
        "aData[2] = 2\n"
        "aOther[1] = 100\n"
        "aOther[2] = 200\n"
        "aBare[1] = 1000\n"
        "aBare[2] = 2000\n"
        "result_data = mutatearray(@aData, 10)\n"
        "result_other = mutatearray(@aOther, 100)\n"
        "bare_return = mutatevalue(aBare)\n"
        "bare_one = aBare[1]\n"
        "bare_two = aBare[2]\n"
        "data_one = aData[1]\n"
        "data_two = aData[2]\n"
        "data_three = aData[3]\n"
        "other_one = aOther[1]\n"
        "other_two = aOther[2]\n"
        "other_three = aOther[3]\n"
        "phantom_size = ALEN(aItems)\n"
        "RETURN\n"
        "FUNCTION mutatearray\n"
        "LPARAMETERS aItems, tnOffset\n"
        "aItems[1] = aItems[1] + tnOffset\n"
        "= ASIZE(aItems, 3)\n"
        "aItems[3] = tnOffset * 3\n"
        "RETURN forwardarray(@aItems, tnOffset)\n"
        "FUNCTION forwardarray\n"
        "LPARAMETERS aForwarded, tnOffset\n"
        "aForwarded[2] = aForwarded[2] + tnOffset\n"
        "RETURN ALEN(aForwarded)\n"
        "FUNCTION mutatevalue\n"
        "LPARAMETERS tnFirstElement\n"
        "tnFirstElement = tnFirstElement + 5\n"
        "RETURN tnFirstElement\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "array FUNCTION parameter script should complete");

    const auto expect_global = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should remain visible");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };
    expect_global("result_data", "3", "nested array alias should preserve resized caller dimensions");
    expect_global("result_other", "3", "each caller array should retain its own resized dimensions");
    expect_global("data_one", "11", "callee element writes should update the first caller array");
    expect_global("data_two", "12", "nested callee writes should update the first caller array");
    expect_global("data_three", "30", "callee resize and append should update the first caller array");
    expect_global("other_one", "200", "second invocation should update only the second caller array");
    expect_global("other_two", "300", "nested second invocation should update only the second caller array");
    expect_global("other_three", "300", "second caller should retain its independent appended value");
    expect_global("bare_return", "1005", "bare UDF arrays should pass their first element by value");
    expect_global("bare_one", "1000", "bare UDF array arguments must not mutate caller storage");
    expect_global("bare_two", "2000", "bare UDF array arguments must not expose the entire caller array");
    expect_global("phantom_size", "0", "callee parameter names must not create persistent phantom arrays");

    fs::remove_all(temp_root, ignored);
}

void test_do_and_call_array_parameters_alias_caller_storage() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_command_array_byref";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_text(
        temp_root / "external_bump.prg",
        "LPARAMETERS aItem, tnAmount\n"
        "aItem[1] = aItem[1] + tnAmount\n"
        "RETURN\n");

    const fs::path main_path = temp_root / "command_array_byref.prg";
    write_text(
        main_path,
        "DIMENSION aDo[1], aCall[1], aExternal[1]\n"
        "aDo[1] = 4\n"
        "aCall[1] = 8\n"
        "aExternal[1] = 20\n"
        "DO bumpitem WITH aDo, 3\n"
        "CALL bumpitem WITH @aCall, 5\n"
        "DO external_bump WITH aExternal, 11\n"
        "do_result = aDo[1]\n"
        "call_result = aCall[1]\n"
        "external_result = aExternal[1]\n"
        "scalar_value = 10\n"
        "DO bumpitem WITH scalar_value, 7\n"
        "scalar_result = scalar_value\n"
        "RETURN\n"
        "PROCEDURE bumpitem\n"
        "LPARAMETERS aItem, tnAmount\n"
        "IF ALEN(aItem) > 0\n"
        "    aItem[1] = aItem[1] + tnAmount\n"
        "ELSE\n"
        "    aItem = aItem + tnAmount\n"
        "ENDIF\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DO/CALL array parameter script should complete");

    const auto do_result = state.globals.find("do_result");
    expect(do_result != state.globals.end(), "DO array result should remain visible");
    if (do_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(do_result->second) == "7", "bare DO array parameters should alias caller storage");
    }
    const auto call_result = state.globals.find("call_result");
    expect(call_result != state.globals.end(), "CALL array result should remain visible");
    if (call_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(call_result->second) == "13", "explicit CALL array parameters should alias caller storage");
    }
    const auto external_result = state.globals.find("external_result");
    expect(external_result != state.globals.end(), "external DO array result should remain visible");
    if (external_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(external_result->second) == "31", "external DO array parameters should alias caller storage");
    }
    const auto scalar_result = state.globals.find("scalar_result");
    expect(scalar_result != state.globals.end(), "scalar by-reference result should remain visible");
    if (scalar_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(scalar_result->second) == "17", "bare procedure parameters should use default by-reference semantics");
    }

    fs::remove_all(temp_root, ignored);
}

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
        expect(copperfin::runtime::format_value(result->second) == "9",
               "expression-level FUNCTION calls should chain through nested user-defined routines");
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
        "SET PROCEDURE TO helpers\n"
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

}  // namespace cf_test_prg_engine_control_flow
