#include "test_prg_engine_runtime_surface_functions_support.h"
#include "test_prg_engine_runtime_surface_functions_tests.h"

int main()
{
    using namespace copperfin::runtime_surface_tests;
    using namespace copperfin::test_support;
    test_curval_evaluates_on_disk_record();
    test_curval_uses_verified_post_commit_session_image();
    test_lupdate_reads_table_header_date_and_designator_variants();
    test_lupdate_respects_set_date_format();
    test_lupdate_designator_edge_cases();
    test_curval_verified_commit_overlay_survives_cursor_reopen();
    test_curval_verified_admission_patch_tracks_second_cursor_memo_and_rollback();
    test_curval_verified_rollback_preserves_case_distinct_posix_admissions();
    test_curval_verified_command_undo_restores_admission_bytes();
    test_curval_verified_failed_command_undo_keeps_admission_bytes();
    test_curval_verified_missing_command_undo_backup_keeps_admission_bytes();
    test_curval_verified_force_commit_merges_other_alias_admission();
    test_curval_verified_rollback_deduplicates_windows_case_aliases();
    test_getfldstate_tracks_buffered_mutation_state();
    test_setfldstate_assigns_buffered_mutation_state();
    test_oldval_evaluates_buffered_original_record();
    test_local_optimistic_table_buffering();
    test_local_optimistic_table_buffering_append_lifecycle();
    test_table_buffer_appends_use_negative_recno_identity();
    test_local_optimistic_table_buffering_delete_recall();
    test_local_pessimistic_table_buffering();
    test_local_optimistic_buffer_conflicts();
    test_local_optimistic_row_buffering();
    test_local_pessimistic_row_buffering();

    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
