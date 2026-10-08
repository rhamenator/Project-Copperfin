// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#include "test_prg_engine_runtime_surface_functions_support.h"
#include "test_prg_engine_runtime_surface_functions_tests.h"
int main() {
    copperfin::test_support::ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
    copperfin::test_support::set_env_value("COPPERFIN_LOCALE","en-US",true);
    using namespace copperfin::runtime_surface_tests;
    test_navigation_commands_reject_reentrant_cursor_replacement();
    test_seek_rejects_cursor_closed_by_own_filter_during_indexed_scan();
    test_table_buffer_appends_use_negative_recno_identity();
    test_table_buffer_append_identity_survives_partial_update_failure();
    return copperfin::test_support::test_failures()==0?0:1;
}
