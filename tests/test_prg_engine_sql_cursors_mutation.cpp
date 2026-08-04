// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_sql_cursors_mutation_tests.h"
#include "prg_engine_test_support.h"

#include <cstdlib>
#include <iostream>

int main()
{
    using namespace copperfin::sql_cursor_mutation_tests;
    test_sql_result_cursor_backward_navigation_in_target_parity();
    test_cursor_identity_functions_for_sql_result_cursors();
    test_copy_to_exports_selected_sql_result_cursor_rows();
    test_copy_to_type_json_exports_selected_sql_result_cursor_and_preserves_selection();
    test_append_from_dbf_mutates_selected_sql_result_cursor();
    test_copy_structure_to_exports_sql_metadata_cursor_schema();
    test_append_from_dbf_for_filters_selected_sql_result_cursor();
    test_sql_result_cursor_mutation_commands();
    test_delete_all_and_recall_all_affect_whole_selected_sql_result_cursor();
    test_targeted_sql_result_cursor_mutations_preserve_selected_alias_and_pointer();
    test_sql_result_cursors_are_isolated_by_data_session();
    test_sql_result_cursor_auto_allocation_tracks_session_selection_flow();
    test_sql_result_cursors_and_ole_actions();
    test_sql_result_cursor_mutation_parity();
    test_sql_result_cursor_multi_field_replace_uses_original_values_for_later_expressions();
    test_sql_result_cursor_sql_style_mutation_parity();
    test_sql_result_cursor_mutation_in_target_parity();
    test_append_from_json_mutates_selected_sql_result_cursor();
    test_append_from_csv_mutates_selected_sql_result_cursor();
    test_append_from_delimited_fields_clause_preserves_typed_order_for_selected_sql_result_cursor();
    test_append_from_selected_sql_result_cursor_runtime_errors_localize();
    test_sql_plain_temporary_order_in_target_honors_collate_and_preserves_selection();
    test_append_from_json_for_filters_selected_sql_result_cursor();

    if (copperfin::test_support::test_failures() != 0) {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
