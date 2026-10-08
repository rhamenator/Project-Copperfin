// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#include "test_prg_engine_control_flow_support.h"
int main() {
    // RQ-CF-PRG-ERROR-COMMAND-NUMERIC-001: run existing ERROR regressions
    // without the unrelated task/concurrency portions of full control_flow.
    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE","en-US",true);
    using namespace cf_test_prg_engine_control_flow;
    test_error_command_numeric_form_with_parameter_is_catchable();
    test_error_command_numeric_form_without_parameter_is_catchable();
    test_error_command_string_form_raises_user_error_1098();
    test_error_command_routes_through_on_error_handler();
    test_error_command_rejects_too_many_operands();
    test_error_command_rejects_bare_keyword_with_no_operand();
    test_error_command_rejects_non_numeric_non_character_operand();
    return test_failures()==0?0:1;
}
