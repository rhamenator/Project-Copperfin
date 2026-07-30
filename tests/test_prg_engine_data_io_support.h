// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#ifndef COPPERFIN_TEST_PRG_ENGINE_DATA_IO_SUPPORT_H
#define COPPERFIN_TEST_PRG_ENGINE_DATA_IO_SUPPORT_H

#include "copperfin/localization/localization.h"
#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "test_environment_support.h"
#include "prg_engine_test_support.h"
#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#if defined(_WIN32)
#include <process.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#define _getpid getpid
#endif
#include <sstream>
#include <system_error>
#include <vector>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif


namespace cf_test_prg_engine_data_io {

using namespace copperfin::test_support;

// ==== Shared test helpers and fixtures ====
using copperfin::test_support::set_env_value;
void test_runtime_array_mutator_functions();
void test_set_filter_dimension_sleep_runtime_errors_localize();
void test_aerror_content_for_sql_passthrough_fault();
using ScopedEnvironmentValue = copperfin::test_support::ScopedEnvironmentValue;

// ==== SCATTER/GATHER tests (memvar/array/name targets, field filters) ====
void test_scatter_memvar_from_current_record();
void test_scatter_gather_memvar_fields_blank_and_for_semantics();
void test_gather_memvar_preserves_fields_without_matching_variables();
void test_scatter_gather_memvar_single_name_field_filter_semantics();
void test_scatter_to_array_and_gather_from_array_round_trip();
void test_scatter_and_gather_array_preserve_explicit_fields_order();
void test_scatter_gather_array_like_and_except_field_filters();
void test_scatter_gather_memvar_preserves_date_and_datetime_like_values();
void test_scatter_gather_array_preserves_date_and_datetime_like_values();
void test_scatter_gather_name_object_round_trip();
void test_scatter_name_additive_merges_existing_object_properties();
void test_scatter_gather_name_single_name_field_filter_semantics();
void test_scatter_gather_name_like_and_except_field_filters();
void test_scatter_gather_memvar_like_and_except_field_filters();
void test_scatter_gather_name_supports_macro_object_variable_names();
void test_scatter_gather_name_supports_nested_object_targets();
void test_scatter_gather_name_supports_macro_expanded_nested_property_segments();
void test_scatter_gather_name_creates_missing_nested_object_targets();
void test_scatter_name_without_additive_replaces_existing_nested_target_object();
void test_scatter_gather_predeclared_2d_array_row_one_semantics();
void test_scatter_gather_two_column_name_value_array_semantics();
void test_scatter_memo_clause_controls_memo_field_inclusion();
void test_gather_from_array_skips_memo_fields_by_default();
void test_gather_memvar_skips_memo_fields_by_default();
void test_scatter_memvar_blank_on_empty_table_succeeds();
void test_scatter_gather_runtime_errors_localize();
void test_gather_memvar_round_trips_field_values();
void test_gather_from_array_is_reverted_by_undo();

// ==== Table import/export tests (APPEND FROM/COPY TO/COPY STRUCTURE TO) ====
void test_copy_to_runtime_errors_localize();
void test_append_from_array_runtime_errors_localize();
void test_append_from_runtime_errors_localize();
void test_append_from_type_runtime_errors_localize();
void test_copy_to_emits_event();
void test_copy_to_creates_destination_dbf();
void test_copy_structure_to_creates_empty_schema();
void test_copy_structure_extended_emits_vfp_metadata_rows();
void test_copy_to_from_empty_table_produces_valid_empty_dbf();
void test_append_from_copies_records_into_current_table();
void test_append_from_honors_open_source_cursor_filter();
void test_append_from_skips_extra_source_fields();
void test_append_from_rolls_back_matched_field_write_failure();
void test_append_from_is_reverted_by_undo();
void test_append_from_transaction_rollback_restores_destination();
void test_copy_to_type_sdf_writes_fixed_width_text_rows();
void test_append_from_type_sdf_imports_fixed_width_text_rows();
void test_copy_to_type_csv_and_delimited_text_rows();
void test_append_from_type_csv_imports_delimited_rows();
void test_append_from_type_sdf_and_delimited_preserve_explicit_fields_order();
void test_copy_to_type_tab_and_append_from_type_tab_round_trip();
void test_copy_to_type_xls_and_append_from_type_xls_round_trip();
void test_copy_to_type_dif_and_append_from_type_dif_round_trip();
void test_copy_to_type_sylk_and_append_from_type_sylk_round_trip();
void test_copy_to_type_json_and_append_from_type_json_round_trip();
void test_copy_to_array_fills_2d_runtime_array();
void test_copy_to_array_fields_clause_allows_keyword_named_field();
void test_append_from_array_writes_records_from_2d_array();
void test_append_from_array_fields_clause_allows_keyword_named_field();
void test_copy_append_array_preserves_explicit_fields_order();
void test_copy_append_array_like_and_except_field_filters();
void test_copy_append_dbf_like_and_except_field_filters();
void test_append_from_array_macro_source_preserves_date_and_datetime_fields();

// ==== SAVE TO/RESTORE FROM variable persistence tests ====
void test_save_to_writes_variables_to_file();
void test_restore_from_loads_variables_from_file();
void test_save_restore_runtime_errors_localize();
void test_restore_from_additive_merges_variables();
void test_save_to_like_pattern_filters_variables();
void test_save_to_except_pattern_filters_variables();
void test_save_restore_auto_mem_extension_without_explicit_extension();
void test_save_restore_round_trips_escaped_string_and_types();
void test_restore_from_rejects_numeric_trailing_garbage();
void test_restore_from_parses_numeric_values_invariantly();
void test_restore_from_without_additive_clears_prior_globals();
void test_restore_from_honors_current_frame_local_bindings();
void test_save_restore_round_trips_arrays();
void test_restore_from_rejects_invalid_array_dimensions();
void test_save_restore_round_trips_public_scope();
void test_save_to_shadowed_public_name_does_not_persist_public_scope_marker();
void test_restore_from_without_additive_clears_stale_arrays();
void test_restore_from_without_additive_clears_private_shadow_state();

// ==== DISPLAY/LIST/BROWSE tests ====
void test_browse_emits_effective_cursor_view_metadata();
void test_browse_like_and_except_field_filters_surface_event_metadata();
void test_browse_nowait_remains_a_clause_boundary();
void test_display_structure_emits_runtime_display_event();
void test_display_status_surfaces_session_metadata();
void test_display_memory_surfaces_visible_variable_and_array_metadata();
void test_display_memory_hides_internal_application_surfaces();
void test_display_records_surfaces_effective_cursor_view_metadata();
void test_display_and_list_records_surface_resolved_in_target_detail();
void test_display_and_list_structure_surface_target_detail();
void test_list_status_emits_runtime_list_event();
void test_list_memory_surfaces_visible_variable_and_array_metadata();
void test_display_list_memory_like_except_filter_applies_to_output();
void test_list_structure_surfaces_selected_cursor_schema();
void test_list_records_surfaces_effective_cursor_view_metadata();

// ==== Interactive I/O command tests (ACCEPT/INPUT/WAIT/KEYBOARD/dialogs) ====
void test_m_dot_namespace_shares_bare_memory_variable_binding();
void test_edit_command_emits_runtime_edit_event();
void test_change_command_emits_runtime_change_event();
void test_input_command_emits_runtime_input_event_with_prompt();
void test_accept_command_emits_runtime_accept_event_with_prompt();
void test_input_accept_commands_surface_macro_prompt_and_target_detail();
void test_input_accept_to_local_targets_stay_local_in_routine_scope();
void test_getfile_command_emits_runtime_getfile_event_with_clause_details();
void test_putfile_command_emits_runtime_putfile_event_with_clause_details();
void test_getdir_command_emits_runtime_getdir_event_with_clause_details();
void test_inputbox_command_emits_runtime_inputbox_event_with_clause_details();
void test_dialog_commands_parenthesized_forms_assign_targets_and_extract_positional_details();
void test_wait_window_command_emits_runtime_wait_event();
void test_wait_clear_command_emits_runtime_wait_clear_event();
void test_keyboard_command_emits_runtime_keyboard_event();

}  // namespace cf_test_prg_engine_data_io

#endif
