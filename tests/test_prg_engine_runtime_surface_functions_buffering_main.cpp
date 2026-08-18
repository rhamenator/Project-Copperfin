#include "test_prg_engine_runtime_surface_functions_support.h"
#include "test_prg_engine_runtime_surface_functions_tests.h"

int main()
{
    using namespace copperfin::runtime_surface_tests;
    using namespace copperfin::test_support;
    test_curval_evaluates_on_disk_record();
    test_curval_uses_verified_post_commit_session_image();
    test_curval_verified_commit_overlay_does_not_survive_cursor_reopen();
    test_getfldstate_tracks_buffered_mutation_state();
    test_setfldstate_assigns_buffered_mutation_state();
    test_oldval_evaluates_buffered_original_record();
    test_local_optimistic_table_buffering();
    test_local_optimistic_table_buffering_append_lifecycle();
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
