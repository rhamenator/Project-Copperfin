// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {

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


}  // namespace cf_test_prg_engine_control_flow
