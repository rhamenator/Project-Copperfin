// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#ifndef COPPERFIN_TEST_PRG_ENGINE_CONTROL_FLOW_SUPPORT_H
#define COPPERFIN_TEST_PRG_ENGINE_CONTROL_FLOW_SUPPORT_H

#include "copperfin/localization/localization.h"
#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "../src/runtime/prg_engine_command_helpers.h"
#include "test_environment_support.h"
#include "prg_engine_test_support.h"
#include <algorithm>
#include <chrono>
#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <cstdint>
#include <future>
#include <filesystem>
#include <fstream>
#include <iostream>
#if defined(_WIN32)
#include <process.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#define _getpid getpid
#endif
#include <sstream>
#include <system_error>
#include <thread>
#include <vector>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif



using namespace copperfin::test_support;

namespace cf_test_prg_engine_control_flow {

// ==== Shared test helpers and fixtures ====
using copperfin::test_support::set_env_value;
std::size_t count_missing_locale_keys(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view locale,
    const std::vector<std::string_view>& keys);
std::string build_nested_do_chain_script(std::size_t nested_routine_count);
using ScopedEnvironmentValue = copperfin::test_support::ScopedEnvironmentValue;

// ==== Control-flow construct tests (DO WHILE/CASE, FOR EACH, SCAN, WITH, TEXT/ENDTEXT, command scanning) ====
void test_command_keyword_scanner_ignores_nested_text();
void test_do_while_and_loop_control_flow();
void test_logical_operators_drive_control_flow();
void test_do_case_control_flow();
void test_push_pop_key_menu_popup_stack_commands();
void test_text_endtext_literal_blocks();
void test_text_endtext_honors_set_textmerge_state_and_delimiters();
void test_text_endtext_textmerge_keeps_interpolated_delimiters_literal();
void test_scan_on_empty_table_does_not_execute_body();
void test_locate_continue_advances_to_later_matches();
void test_locate_on_empty_table_sets_eof();
void test_go_top_bottom_on_empty_table_does_not_crash();
void test_go_top_bottom_with_no_visible_records_sets_bof_and_eof();
void test_static_diagnostic_flags_likely_infinite_do_while_loop();
void test_elseif_control_flow_executes_matching_branch();
void test_block_terminators_ignore_trailing_annotations();
void test_with_endwith_resolves_leading_dot_member_access();
void test_with_endwith_preserves_reserved_dotted_logical_tokens();
void test_with_target_uses_heap_backed_expression_continuations();
void test_loop_and_exit_unwind_with_bindings_before_jump();
void test_loop_and_exit_unwind_case_contexts_before_jump();
void test_print_command_emits_event();
void test_erase_copy_rename_file_commands();
void test_erase_copy_file_strict_verified_bytes();
void test_rename_file_command_rejects_existing_destination();
void test_for_each_iterates_array_elements();
void test_for_each_single_element_expression();
void test_for_each_iterates_native_collection_direct_and_member_path();
void test_for_each_foxobject_qualifier_tolerates_direct_and_member_path_collections();

// ==== Variable scope and lifetime tests (PRIVATE/PUBLIC/RELEASE/CLEAR MEMORY/STORE) ====
void test_private_declaration_masks_caller_variable();
void test_private_all_hides_matching_caller_variables_and_arrays();
void test_scoped_array_declarations_follow_vfp_lifetime_rules();
void test_whole_array_assignment_copies_scoped_storage();
void test_double_parentheses_force_array_value_copy();
void test_local_and_private_array_by_reference_bindings();
void test_implicit_routine_assignments_are_frame_private();
void test_private_variable_visible_to_called_routines();
void test_release_private_restores_saved_binding_immediately();
void test_release_local_restores_visible_outer_global();
void test_macro_assignment_target_updates_private_binding_and_release_restores_outer_value();
void test_macro_assignment_target_preserves_public_binding_across_release_all();
void test_store_command_assigns_multiple_variables();
void test_release_vars_erases_named_globals();
void test_release_all_clears_all_globals();
void test_release_all_clears_current_frame_locals_without_global_leak();
void test_release_all_local_shadow_preserves_outer_global();
void test_release_all_private_shadow_restores_outer_global();
void test_release_all_like_pattern();
void test_release_all_like_pattern_reaches_arrays();
void test_release_all_except_pattern();
void test_release_all_except_pattern_reaches_arrays();
void test_release_all_preserves_public_bindings();
void test_clear_memory_erases_all_globals();
void test_clear_memory_prevents_private_bindings_from_restoring();
void test_clear_memory_clears_current_frame_locals_without_global_leak();

// ==== Aggregate and TOTAL command tests (SUM/COUNT/CALCULATE/TOTAL) ====
void test_aggregate_functions_respect_visibility();
void test_calculate_command_aggregates();
void test_command_level_aggregate_commands();
void test_aggregate_command_errors_use_default_locale_messages();
void test_aggregate_command_errors_localize_without_changing_runtime_behavior();
void test_aggregate_commands_on_empty_table_return_zero();
void test_aggregate_commands_support_macro_targets_and_calculate_while();
void test_command_level_aggregate_scope_and_while_semantics();
void test_aggregate_helpers_tolerate_non_numeric_field_text();
void test_total_command_for_local_tables();
void test_total_numeric_formatting_ignores_global_locale();
void test_total_command_tolerates_non_numeric_field_text();
void test_total_command_errors_use_default_locale_messages();
void test_total_command_supports_currency_and_integer_fields();
void test_total_command_for_sql_result_cursors();
void test_select_query_into_array_commands();
void test_create_table_free_dynamic_target();

// ==== Concurrency tests (SPAWN/AWAIT/YIELD/critical sections/DOEVENTS) ====
void test_doevents_pumps_event_queue();
void test_doevents_in_responsive_loop();
void test_sleep_command_emits_runtime_sleep_event();
void test_sleep_duration_uses_heap_backed_frame_continuations();
void test_spawn_and_await_command_runs_task_to_completion();
void test_spawn_task_supervision_observes_status_result_and_output_without_consuming_task();
void test_spawn_task_supervision_requests_cooperative_cancellation();
void test_spawn_arguments_use_heap_backed_frame_continuations();
void test_spawn_cancellation_propagates_to_sibling_tasks();
void test_request_cancel_rolls_back_active_transaction_and_resets_txnlevel();
void test_spawn_critical_section_serializes_workers();
void test_critical_section_order_policy_rejects_descending_nested_acquire();
void test_critical_section_exit_order_is_enforced();
void test_critical_section_reentrant_enter_same_section_is_allowed();
void test_critical_section_blocking_policy_rejects_await_inside_section();
void test_critical_section_blocking_policy_rejects_sleep_inside_section();
void test_yield_is_explicit_policy_exception_in_enter_critical();
void test_yield_allowed_in_enter_critical_regression_minimal();
void test_yield_in_enter_critical_has_no_blocking_violation();
void test_yield_in_enter_critical_is_explicit_policy_exception_regression();
void test_enter_critical_allows_yield_without_blocking_violation_event();
void test_critical_sections_release_on_task_fault_without_deadlock();
void test_yield_command_emits_runtime_yield_event_and_preserves_state();
void test_yield_is_allowed_while_holding_critical_section();
void test_yield_in_critical_section_keeps_section_semantics();
void test_yield_is_allowed_in_default_critical_section_is_policy_exception();
void test_yield_is_allowed_in_critical_section_is_policy_exception();
void test_yield_inside_critical_section_is_allowed();
void test_yield_allowed_in_enter_critical_is_small_regression();
void test_yield_allowed_in_reentrant_enter_critical_section();
void test_yield_in_enter_critical_is_explicit_policy_exception_small();
void test_yield_preserves_fault_metadata_when_followed_by_error();

// ==== Error-handling and fault tests (ON ERROR/TRY-CATCH/AERROR/RETRY/RESUME) ====
void test_cursor_use_and_seek_errors_use_default_locale_messages();
void test_sql_runtime_errors_localize_without_changing_runtime_behavior();
void test_on_error_do_handler_dispatches_routine();
void test_bare_on_error_restores_default_error_handling();
void test_bare_null_expression_preserves_null_error_sentinels();
void test_on_error_do_with_handler_receives_error_metadata();
void test_aerror_populates_structured_runtime_error_array();
void test_aerror_exposes_sql_and_ole_specific_rows();
void test_on_error_handler_preserves_original_fault_metadata_across_caught_inner_faults();
void test_on_error_handler_catch_to_uses_inner_fault_metadata();
void test_ole_property_fault_dispatches_on_error_and_preserves_object_state();
void test_ole_method_fault_is_catchable_and_preserves_object_state();
void test_thrown_expression_fault_preserves_pause_statement_and_recovery();
void test_repeated_thrown_faults_refresh_pause_metadata_each_time();
void test_nested_routine_faults_report_faulting_stack_frame_line();
void test_repeated_nested_faults_refresh_stack_frame_and_statement_metadata();
void test_try_catch_finally_handles_runtime_errors();
void test_try_catch_unwinds_leaked_with_binding_before_catch();
void test_outer_try_does_not_catch_fault_from_unrelated_expression_invoked_routine();
void test_error_handler_still_fires_after_fault_inside_expression_invoked_routine();
void test_catch_to_binds_exception_object_with_error_metadata();
void test_throw_is_catchable_and_preserves_exception_uservalue();
void test_bare_throw_rethrows_active_exception_object();
void test_bare_throw_without_active_exception_creates_user_thrown_default();
void test_throw_exception_object_chains_outer_uservalue_reference();
void test_catch_when_false_falls_through_to_later_clause();
void test_catch_to_when_false_resets_variable_and_falls_to_outer_handler();
void test_catch_when_false_with_finally_reaches_outer_catch_with_original_metadata();
void test_catch_when_false_with_finally_reaches_on_error_with_original_metadata();
void test_catch_fault_runs_pending_finally_before_propagation();
void test_try_finally_runs_without_catch_on_success();
void test_return_inside_try_runs_finally_before_return();
void test_return_inside_catch_runs_all_enclosing_finally_before_return();
void test_file_operation_runtime_errors_localize();
void test_residual_dispatch_runtime_errors_localize();
void test_dispatch_array_and_object_target_runtime_errors_use_default_locale_messages();
void test_dispatch_array_and_object_target_runtime_errors_localize();
void test_ole_property_assignment_runtime_errors_localize();
void test_ole_invocation_and_property_read_runtime_errors_localize();
void test_on_error_resume_restores_fault_session_and_cursor_state();
void test_fault_continue_cycle_preserves_open_cursor_and_record_position();
void test_fault_continue_cycle_preserves_selected_alias_across_data_session_scope();
void test_pause_stack_frame_contains_accurate_intermediate_frame_lines();
void test_repeated_fault_pauses_refresh_intermediate_stack_frame_lines();
void test_thrown_expression_fault_aerror_columns_match_error_message_functions();
void test_repeated_on_error_faults_refresh_normalized_diagnostics();
void test_division_by_zero_dispatches_runtime_error();
void test_numeric_field_overflow_is_diagnosed_not_silently_truncated();
void test_retry_reexecutes_faulting_statement();
void test_resume_next_continues_after_fault();
void test_retry_with_no_fault_checkpoint_is_noop();
void test_runtime_faults_preserve_state_and_allow_retry();
void test_aerror_line_number_is_innermost_faulting_line_not_catch_site();

// ==== Runtime guardrail and lifecycle tests (QUIT/shutdown/CONFIG FPW/call-depth and loop limits) ====
void test_runtime_guardrail_limits_call_depth_without_crashing_host();
void test_runtime_guardrail_exactly_at_call_depth_limit_succeeds();
void test_runtime_guardrail_one_over_call_depth_limit_fails();
void test_runtime_guardrail_limits_statement_budget_without_crashing_host();
void test_runtime_guardrail_limits_loop_iterations_without_crashing_host();
void test_runtime_guardrail_errors_localize_without_changing_behavior();
void test_config_fpw_overrides_runtime_limits();
void test_config_fpw_rejects_grouped_integer_tokens();
void test_config_fpw_custom_limit_is_enforced_at_boundary();
void test_config_fpw_overrides_temp_directory_default();
void test_close_command_closes_all_work_areas();
void test_close_all_releases_runtime_handles();
void test_cancel_halts_execution();
void test_quit_emits_event();
void test_quit_cancelled_by_callback();
void test_shutdown_handler_quit_exits_event_loop_without_clear_events();
void test_shutdown_handler_cleanup_code_remains_harmless();
void test_on_shutdown_clear_events_runs_and_quit_completes();
void test_on_shutdown_do_cleanup_can_call_quit_without_recursing();
void test_on_shutdown_inline_close_databases_all_runs_before_quit();
void test_quit_closes_open_database_and_runtime_handles();

// ==== Routine call and parameter-passing tests (DO/CALL with parameters, by-reference) ====
void test_do_with_parameters_binds_arguments_in_called_routine();
void test_parenthesized_dynamic_do_targets_use_heap_backed_frames();
void test_proc_abbreviation_registers_same_file_do_routine();
void test_store_expands_defined_indirect_target_without_changing_array_targets();
void test_call_with_parameters_binds_arguments_in_called_routine();
void test_unsupplied_parameters_initialize_to_logical_false();
void test_parameter_defaults_use_heap_backed_expression_continuations();
void test_call_external_target_with_by_reference_updates_caller_variable();
void test_do_with_by_reference_updates_caller_variable();
void test_expression_level_function_call_assigns_return_value();
void test_expression_level_procedure_call_assigns_return_value();
void test_expression_level_function_call_supports_by_reference_arguments();
void test_missing_argument_commas_raise_expression_errors();
void test_set_udfparms_controls_expression_routine_parameter_aliasing();
void test_set_udfparms_state_is_isolated_between_data_and_runtime_sessions();
void test_deep_scalar_reference_forwarding_uses_heap_backed_frame_walk();
void test_direct_recursive_return_uses_heap_backed_frame_continuations();
void test_standalone_expression_uses_heap_backed_frame_continuations();
void test_store_expression_uses_heap_backed_frame_continuations();
void test_do_with_arguments_use_heap_backed_frame_continuations();
void test_call_with_arguments_use_heap_backed_frame_continuations();
void test_compound_return_uses_heap_backed_expression_checkpoints();
void test_assignment_rhs_uses_heap_backed_expression_checkpoints();
void test_array_parameters_alias_caller_storage_across_nested_function_calls();
void test_do_and_call_array_parameters_alias_caller_storage();
void test_expression_level_function_call_works_in_if_predicates();
void test_if_elseif_predicates_use_heap_backed_expression_checkpoints();
void test_case_predicates_use_heap_backed_expression_checkpoints();
void test_loop_predicates_and_bounds_use_heap_backed_expression_checkpoints();
void test_scan_predicate_preserves_rest_scope_and_exhaustion_state();
void test_elseif_predicate_resumption_review_gaps();
void test_expression_level_function_call_can_chain_nested_user_routines();
void test_set_procedure_registers_external_function_for_expression_calls();
void test_set_procedure_registers_external_procedure_for_do_calls();
void test_set_procedure_macro_off_clears_saved_procedure_state();
void test_set_procedure_additive_uses_first_opened_precedence_and_replace_resets_lookup();
void test_set_procedure_registers_external_event_handler();
void test_set_procedure_registers_external_error_handler();

}  // namespace cf_test_prg_engine_control_flow

#endif
