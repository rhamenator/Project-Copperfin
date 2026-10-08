// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#include "test_prg_engine_runtime_surface_functions_support.h"
#include "test_prg_engine_runtime_surface_functions_tests.h"
int main() {
 using namespace copperfin::runtime_surface_tests;
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 test_native_popup_bar_selection_dispatches_registered_callback();
 test_native_on_selection_bar_executes_static_action_command();
 test_native_on_bar_activates_static_popup_submenu();
 return test_failures()==0?0:1;
}
