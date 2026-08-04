// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
#include "prg_engine_test_support.h"
#include "test_prg_engine_table_mutation_tests.h"

#include <cstdlib>
#include <iostream>

int main()
{
    using namespace copperfin::table_mutation_tests;
    using namespace copperfin::test_support;
    test_local_table_mutation_and_scan_flow();
    test_delete_all_and_recall_all_affect_whole_local_table();
    test_replace_for_updates_all_matching_records();
    test_replace_scope_clauses_bound_physical_record_ranges();
    test_delete_and_recall_scope_clauses_bound_physical_record_ranges();
    test_replace_additive_appends_only_memo_assignments();
    test_replace_matches_local_field_names_case_insensitively();
    test_undo_restores_scoped_additive_replace_bytes();
    test_multi_field_replace_uses_original_values_for_later_expressions();
    test_pack_compacts_deleted_local_records();
    test_pack_is_reverted_by_undo();
    test_zap_truncates_local_table_records();
    test_zap_is_reverted_by_undo();
    test_replace_character_field_truncates_to_field_width();
    test_character_field_at_maximum_width_round_trips();
    test_memo_field_replace_with_empty_string();
    test_set_exclusive_controls_table_maintenance_guards();
    test_lock_functions_and_unlock_command_track_session_locks();
    test_record_lock_argument_conversion_is_bounded();
    test_replacing_a_used_work_area_releases_prior_table_locks();
    test_reprocess_contention_retries_and_mutation_lock_timeouts();
    test_reprocess_table_lock_timeouts_are_localized();
    test_lock_retry_blocking_is_rejected_inside_critical_section();
    test_rlock_retry_blocking_is_rejected_inside_critical_section();
    test_flock_retry_blocking_is_rejected_inside_critical_section();
    test_insert_into_and_delete_from_local_table();
    test_insert_into_select_materializes_filtered_ordered_rows();
    test_insert_into_select_rolls_back_the_whole_failed_batch();
    test_insert_into_rolls_back_failed_local_append();
    test_indexed_table_mutation_succeeds_for_structural_indexes();
    test_append_blank_supports_opaque_field_layouts_at_runtime();
    test_cancel_rolls_back_active_transaction();
    test_update_command_sets_scoped_records();
    test_update_and_delete_accept_in_subquery_predicates();
    test_update_and_delete_accept_not_in_subquery_predicates();
    test_sql_style_for_clauses_accept_macro_expressions();
    test_undo_reverts_latest_append_blank();
    test_undo_reverts_latest_delete_command();
    test_undo_reverts_latest_update_command();
    test_undo_reverts_latest_insert_into_command();
    test_undo_reverts_latest_create_table_command();
    test_undo_reverts_latest_alter_table_command();
    test_append_from_array_rolls_back_failed_multi_row_write();
    test_undo_reverts_latest_append_from_array();
    test_undo_reverts_latest_replacement_command();
    test_command_undo_query_reports_available_label_after_bulk_operation();
    test_undo_all_reverts_multiple_latest_commands();
    test_undo_without_history_fails_deterministically();
    test_rollback_transaction_replays_local_dbf_changes();
    test_rollback_transaction_replays_append_from_array();
    test_rollback_transaction_prunes_stale_alter_table_field_rules();
    test_rollback_transaction_removes_created_table_cursor();
    test_transaction_rollback_leaves_table_unchanged();
    test_startup_replays_pending_transaction_journal();
    test_transaction_journal_serializes_grouped_levels_invariantly();
    test_startup_rejects_malformed_transaction_journal_scalars();

    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
