// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#include "test_prg_engine_runtime_surface_functions_support.h"
#include "test_prg_engine_runtime_surface_functions_tests.h"
int main() {
 using namespace copperfin::runtime_surface_tests;
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 test_native_defined_menu_lifecycle();
 test_native_list_controls_popup_rowsource_materializes_static_bars();
 return test_failures()==0?0:1;
}
