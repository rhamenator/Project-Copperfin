#include "test_prg_engine_seek_index_support.h"
#include "test_prg_engine_seek_index_tests.h"

int main()
{
    using namespace copperfin::seek_index_tests;
    test_set_order_and_seek_for_local_tables();
    test_seek_search_key_uses_heap_backed_frame_continuations();
    test_set_collate_guides_plain_string_seek_comparisons();
    test_seek_uses_grounded_order_normalization_hints();
    test_seek_supports_composite_tag_expressions();
    test_seek_supports_left_function_tag_expressions();
    test_seek_supports_right_function_tag_expressions();
    test_seek_supports_substr_function_tag_expressions();
    test_seek_supports_padl_function_tag_expressions();
    test_seek_supports_padr_function_tag_expressions();
    test_seek_supports_padl_default_padding_tag_expressions();
    test_seek_supports_padr_default_padding_tag_expressions();
    test_seek_supports_str_function_tag_expressions();
    test_seek_supports_str_default_width_tag_expressions();
    test_seek_supports_str_decimal_tag_expressions();
    test_set_near_changes_seek_failure_position();
    test_set_order_descending_changes_seek_ordering();
    test_seek_command_accepts_tag_override_without_set_order();
    test_seek_command_accepts_descending_tag_override_without_set_order();
    test_seek_related_index_functions();
    test_key_and_tagcount_functions();
    test_tagno_function();
    test_key_and_tag_ordinal_order_across_companion_naming_styles();
    test_seek_function_accepts_direction_suffix_in_order_designator();
    test_local_table_temporary_order_expression_parity();
    test_order_and_tag_preserve_index_file_identity();
    test_local_command_seek_in_target_with_temporary_order_expression();
    test_local_descending_temporary_order_expression_in_target_preserves_selection();
    test_local_plain_temporary_order_in_target_honors_collate_and_preserves_selection();
    test_local_temporary_order_expression_indexseek_parity();
    test_seek_respects_grounded_order_for_expression_hints();
    test_seek_respects_set_deleted_visibility();
    test_seek_respects_numeric_order_for_expression_hints();
    test_seek_respects_string_order_for_expression_hints();
    test_ndx_numeric_domain_guides_seek_near_ordering();
    test_local_numeric_temporary_order_domain_guides_seek_near_ordering();
    test_set_near_is_scoped_by_data_session();
    test_foxtools_registration_and_call_bridge();
    test_foxtools_registration_is_scoped_by_data_session();
    test_declared_dll_string_byref_argument_writeback();
    test_declared_dll_explicit_relative_child_path();
    test_declared_dll_double_arguments_follow_x64_abi();
    test_declared_dll_long_uses_vfp_32_bit_width();
    test_declared_dll_single_uses_vfp_32_bit_float_width();
    test_declared_dll_win32_uses_typed_stdcall_slots();
    test_declared_dll_win32_resolves_no_underscore_stdcall_export();
    test_declared_dll_short_return_uses_signed_16_bit_width();
    test_declared_dll_short_parameter_is_rejected_and_localized();
    test_declared_dll_win32api_search_and_ansi_fallback();
    test_declared_dll_win32api_missing_symbol_localizes();
    test_declared_dll_module_ownership_and_failed_redeclare_rollback();
    test_declared_dll_explicit_ansi_fallback_and_exact_precedence();
    test_declared_dll_argument_count_is_validated_before_native_entry();
    test_declared_dll_numeric_byref_requires_callsite_reference();
    test_declare_dll_runtime_errors_localize();
    test_set_exact_affects_comparisons_and_seek();
    test_use_again_and_alias_collision_semantics();
    test_use_again_without_in_allocates_new_area_and_preserves_alias_selection();
    test_use_in_selected_alias_replacement_clears_old_alias_and_order_state();
    test_select_missing_alias_is_an_error();
    test_use_in_missing_alias_is_an_error();
    test_runtime_fault_containment();
    test_set_filter_scopes_local_cursor_visibility();
    test_set_filter_defers_local_cursor_evaluation_until_navigation();
    test_seek_respects_active_filter_visibility();
    test_set_filter_in_targets_nonselected_alias();

    if (copperfin::test_support::test_failures() != 0)
    {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
