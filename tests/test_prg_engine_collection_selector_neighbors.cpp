// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
#include "test_prg_engine_runtime_surface_functions_support.h"
#include "test_prg_engine_runtime_surface_functions_tests.h"
int main() {
 using namespace copperfin::runtime_surface_tests;
 ScopedEnvironmentValue locale("COPPERFIN_LOCALE");set_env_value("COPPERFIN_LOCALE","en-US",true);
 test_native_collection_default_item_invocation_routes_bare_and_member_path_calls();
 test_native_collection_default_item_calls_preserve_member_chains();
 test_native_collection_duplicate_key_raises_without_mutation();
 test_native_collection_subscript_access_routes_to_items_and_preserves_member_chains();
 test_native_objects_child_collection_reflects_count_item_and_foreach_without_leaking_hidden_runtime_surfaces();
 test_native_controls_child_collection_reflects_count_item_and_foreach_without_leaking_hidden_runtime_surfaces();
 test_native_pageframe_pages_child_collection_reflects_count_item_and_foreach_without_leaking_hidden_runtime_surfaces();
 test_runtime_application_forms_aliases_track_representative_window_collection();
 return test_failures()==0?0:1;
}
