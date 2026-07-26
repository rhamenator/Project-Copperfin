// Copyright 2026 Richard M. Hamilton. All rights reserved.
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
        "PUBLIC sum_result\n"
        "cTarget = resolve_target()\n"
        "DO &cTarget WITH 4, 5\n"
        "RETURN\n"
        "PROCEDURE addvals\n"
        "LPARAMETERS a, b\n"
        "sum_result = a + b\n"
        "RETURN\n"
        "FUNCTION resolve_target\n"
        "RETURN 'addvals'\n"
        "ENDFUNC\n");

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

void test_parenthesized_dynamic_do_targets_use_heap_backed_frames() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_parenthesized_dynamic_do";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path external_path = temp_root / "dynamic_target.prg";
    write_text(
        external_path,
        "LPARAMETERS a, b\n"
        "PUBLIC external_result\n"
        "external_result = a + b\n"
        "RETURN\n");

    const fs::path main_path = temp_root / "parenthesized_dynamic_do.prg";
    write_text(
        main_path,
        std::string("PUBLIC direct_result, nested_result\n") +
            "cTarget = 'add_direct'\n"
            "DO (cTarget) WITH 4, 5\n"
            "cTargetHolder = 'add_nested'\n"
            "DO (&cTargetHolder) WITH 6, 7\n"
            "cExternalTarget = '" + external_path.string() + "'\n"
            "DO (cExternalTarget) WITH 8, 9\n"
            "RETURN\n"
            "PROCEDURE add_direct\n"
            "LPARAMETERS a, b\n"
            "direct_result = a + b\n"
            "RETURN\n"
            "PROCEDURE add_nested\n"
            "LPARAMETERS a, b\n"
            "nested_result = a + b\n"
            "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "parenthesized dynamic DO targets should complete: " + state.message);

    const auto direct_result = state.globals.find("direct_result");
    expect(direct_result != state.globals.end() &&
               copperfin::runtime::format_value(direct_result->second) == "9",
           "DO (cTarget) should dispatch a same-file routine with arguments");
    const auto nested_result = state.globals.find("nested_result");
    expect(nested_result != state.globals.end() &&
               copperfin::runtime::format_value(nested_result->second) == "13",
           "DO (&cTargetHolder) should evaluate a nested macro target");
    const auto external_result = state.globals.find("external_result");
    expect(external_result != state.globals.end() &&
               copperfin::runtime::format_value(external_result->second) == "17",
           "DO (cExternalTarget) should dispatch a project-local PRG target");

    const fs::path missing_path = temp_root / "parenthesized_dynamic_do_missing.prg";
    write_text(
        missing_path,
        "DO ('missing_dynamic_target')\n"
        "RETURN\n");
    copperfin::runtime::PrgRuntimeSession missing_session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(missing_path.string(), temp_root.string(), false));
    const auto missing_state = missing_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(missing_state.reason == copperfin::runtime::DebugPauseReason::error &&
               missing_state.message == "Unable to resolve DO target: missing_dynamic_target",
           "parenthesized dynamic DO missing targets should preserve the deterministic diagnostic");

    fs::remove_all(temp_root, ignored);
}

void test_proc_abbreviation_registers_same_file_do_routine() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_proc_abbreviation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "proc_abbreviation.prg";
    write_text(
        main_path,
        "PUBLIC cleanup_result\n"
        "DO cleanup WITH 6\n"
        "RETURN\n"
        "PROC cleanup\n"
        "LPARAMETERS value\n"
        "cleanup_result = value + 1\n"
        "RETURN\n"
        "ENDPROC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PROC abbreviation same-file DO script should complete: " + state.message);

    const auto result = state.globals.find("cleanup_result");
    expect(result != state.globals.end(), "PROC routine should assign the caller-visible result");
    if (result != state.globals.end()) {
        expect(copperfin::runtime::format_value(result->second) == "7",
               "same-file DO should dispatch to a PROC abbreviation routine");
    }

    fs::remove_all(temp_root, ignored);
}

void test_store_expands_defined_indirect_target_without_changing_array_targets() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_store_indirect_macro_target";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "store_indirect_macro_target.prg";
    write_text(
        main_path,
        "#DEFINE target_name oItems\n"
        "#DEFINE collection_class \"Collection\"\n"
        "PUBLIC oItems, nCount, nArrayValue\n"
        "DIMENSION aValues[1]\n"
        "STORE CREATEOBJECT(collection_class) TO ([target_name])\n"
        "STORE 7 TO aValues[1]\n"
        "nCount = oItems.Count\n"
        "nArrayValue = aValues[1]\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "indirect STORE macro target script should complete: " + state.message);

    const auto expect_value = [&](const std::string& name, const std::string& expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should remain visible");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " should equal " + expected + ", got " +
                       copperfin::runtime::format_value(found->second));
        }
    };

    expect_value("ncount", "0");
    expect_value("narrayvalue", "7");

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
        "PUBLIC sum_result\n"
        "cTarget = resolve_target()\n"
        "CALL &cTarget WITH 7, 8\n"
        "RETURN\n"
        "PROCEDURE addvals\n"
        "LPARAMETERS a, b\n"
        "sum_result = a + b\n"
        "RETURN\n"
        "FUNCTION resolve_target\n"
        "RETURN 'addvals'\n"
        "ENDFUNC\n");

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
        "PUBLIC clocaltype, llocalvalue, nlocalpcount, nexplicitdefault\n"
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

void test_parameter_defaults_use_heap_backed_expression_continuations() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_parameter_default_continuation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "parameter_default_continuation.prg";
    write_text(
        main_path,
        "PUBLIC nfirstdefault, nseconddefault\n"
        "nDefaultCalls = 0\n"
        "DO inspectdefaults WITH 5\n"
        "RETURN\n"
        "PROCEDURE inspectdefaults\n"
        "LPARAMETERS supplied, first = make_default(supplied), second = make_default(first)\n"
        "nFirstDefault = first\n"
        "nSecondDefault = second\n"
        "RETURN\n"
        "FUNCTION make_default\n"
        "LPARAMETERS value\n"
        "nDefaultCalls = nDefaultCalls + 1\n"
        "RETURN value + 1\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "parameter default continuation script should complete: " + state.message);

    const auto expect_value = [&](const char *name, const char *expected) {
        const auto value = state.globals.find(name);
        expect(value != state.globals.end(), std::string("parameter default continuation should assign ") + name);
        if (value != state.globals.end()) {
            expect(copperfin::runtime::format_value(value->second) == expected,
                   std::string("parameter default continuation value mismatch for ") + name);
        }
    };
    expect_value("nfirstdefault", "6");
    expect_value("nseconddefault", "7");
    expect_value("ndefaultcalls", "2");

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

void test_missing_argument_commas_raise_expression_errors() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_missing_argument_commas";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_error_script = [&](const std::string& file_stem, const std::string& script) {
        const fs::path main_path = temp_root / (file_stem + ".prg");
        write_text(main_path, script);
        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string(), false));
        return session.run(copperfin::runtime::DebugResumeAction::continue_run);
    };

    const auto missing_regular_comma = run_error_script(
        "missing_regular_comma",
        "result = addvals(1 2)\n"
        "RETURN\n"
        "FUNCTION addvals\n"
        "LPARAMETERS a, b\n"
        "RETURN a + b\n");
    expect(
        missing_regular_comma.reason == copperfin::runtime::DebugPauseReason::error &&
            missing_regular_comma.message.find("Expected function argument") != std::string::npos,
        "a missing comma in an ordinary function call should raise the localized argument error");

    const auto missing_reference_comma = run_error_script(
        "missing_reference_comma",
        "counter = 1\n"
        "result = bump(@counter @counter)\n"
        "RETURN\n"
        "FUNCTION bump\n"
        "LPARAMETERS value\n"
        "RETURN value\n");
    expect(
        missing_reference_comma.reason == copperfin::runtime::DebugPauseReason::error &&
            missing_reference_comma.message.find("Expected function argument") != std::string::npos,
        "a missing comma between by-reference arguments should raise the localized argument error");

    const auto missing_method_comma = run_error_script(
        "missing_method_comma",
        "oProbe = CREATEOBJECT('MissingCommaProbe')\n"
        "result = oProbe.Inspect(1 2)\n"
        "RETURN\n"
        "DEFINE CLASS MissingCommaProbe AS Custom\n"
        "    FUNCTION Inspect\n"
        "        LPARAMETERS a, b\n"
        "        RETURN a + b\n"
        "    ENDFUNC\n"
        "ENDDEFINE\n");
    expect(
        missing_method_comma.reason == copperfin::runtime::DebugPauseReason::error &&
            missing_method_comma.message.find("Expected function argument") != std::string::npos,
        "a missing comma in an object method call should raise the localized argument error");

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


}  // namespace cf_test_prg_engine_control_flow
