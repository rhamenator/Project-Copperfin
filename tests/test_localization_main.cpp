// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_localization_support.h"

void test_catalog_loading_and_fallback();
void test_catalog_file_accepts_one_leading_utf8_bom();
void test_bcp47_script_locale_normalization_and_catalog_fallback();
void test_posix_locale_suffixes_normalize_before_catalog_fallback();
void test_placeholders_pseudo_locale_and_unicode();
void test_catalog_json_unicode_escapes_support_surrogate_pairs();
void test_catalog_json_rejects_literal_string_control_characters();
void test_machine_contract_fields_remain_invariant();
void test_product_locale_catalogs_have_key_parity();
void test_catalog_root_auto_discovery_skips_invalid_candidates();
void test_catalog_root_auto_discovery_skips_malformed_default_catalog();
void test_catalog_root_auto_discovery_skips_unreadable_default_catalog_when_enforced();
void test_catalog_root_auto_discovery_preserves_executable_candidate_precedence();
void test_catalog_root_explicit_override_remains_authoritative();
void test_catalog_root_resolution_searches_parent_directories();
void test_catalog_root_resolution_finds_repo_build_output_layout_from_executable_path();
void test_catalog_root_resolution_finds_repo_build_output_layout_from_path_launched_basename();
void test_parser_behavior_remains_locale_invariant();
void test_runtime_session_diagnostics_route_through_catalog();
void test_runtime_cursor_diagnostics_route_through_catalog();
void test_runtime_total_diagnostics_route_through_catalog();
void test_runtime_report_output_messages_route_through_catalog();
void test_runtime_report_output_errors_localize_without_changing_runtime_behavior();
void test_runtime_aggregate_errors_route_through_catalog();
void test_runtime_sql_errors_route_through_catalog();
void test_build_host_catalog_entries_cover_placeholder_locales();
void test_inspect_catalog_entries_cover_placeholder_locales();
void test_shared_core_catalog_entries_cover_placeholder_locales();
void test_runtime_host_manifest_verification_errors_route_through_catalog();
void test_runtime_host_quit_prompt_routes_through_catalog();
void test_runtime_host_security_policy_denial_routes_through_catalog();
void test_platform_federation_ai_planner_fallback_routes_through_catalog();
void test_runtime_numeric_domain_errors_route_through_catalog();
void test_runtime_expression_errors_route_through_catalog();
void test_runtime_record_precondition_errors_route_through_catalog();
void test_runtime_dll_errors_route_through_catalog();
void test_runtime_core_errors_route_through_catalog();
void test_runtime_pause_and_session_messages_route_through_catalog();
void test_runtime_watch_errors_route_through_catalog();
void test_runtime_dispatch_errors_route_through_catalog();
void test_runtime_surface_errors_route_through_catalog();
void test_runtime_save_restore_errors_route_through_catalog();
void test_runtime_file_operation_errors_route_through_catalog();
void test_runtime_copy_to_errors_route_through_catalog();
void test_runtime_append_from_array_errors_route_through_catalog();
void test_runtime_append_from_errors_route_through_catalog();
void test_runtime_append_from_type_errors_route_through_catalog();
void test_runtime_scatter_gather_errors_route_through_catalog();
void test_runtime_dispatch_array_and_object_target_errors_route_through_catalog();
void test_runtime_table_structure_errors_route_through_catalog();
void test_runtime_set_filter_dimension_sleep_errors_route_through_catalog();
void test_runtime_declare_dispatch_errors_route_through_catalog();
void test_runtime_residual_command_dispatch_errors_route_through_catalog();
void test_runtime_object_helper_dispatch_errors_route_through_catalog();
void test_runtime_ole_invocation_and_property_read_errors_route_through_catalog();
void test_inspect_usage_routes_through_localization(const std::string& inspect_path);
void test_inspect_accepts_posix_locale_suffixes(const std::string& inspect_path);
void test_inspect_explicit_locale_routes_dbf_version_display(const std::string& inspect_path);
void test_inspect_license_status_preserves_machine_contracts(const std::string& inspect_path);
void test_runtime_package_warnings_pseudo_localize();
void test_inspect_error_prefix_routes_through_localization(const std::string& inspect_path);

int main(int argc, char** argv) {
    test_catalog_loading_and_fallback();
    test_catalog_file_accepts_one_leading_utf8_bom();
    test_bcp47_script_locale_normalization_and_catalog_fallback();
    test_posix_locale_suffixes_normalize_before_catalog_fallback();
    test_placeholders_pseudo_locale_and_unicode();
    test_catalog_json_unicode_escapes_support_surrogate_pairs();
    test_catalog_json_rejects_literal_string_control_characters();
    test_machine_contract_fields_remain_invariant();
    test_product_locale_catalogs_have_key_parity();
    test_catalog_root_auto_discovery_skips_invalid_candidates();
    test_catalog_root_auto_discovery_skips_malformed_default_catalog();
#if !defined(_WIN32)
    test_catalog_root_auto_discovery_skips_unreadable_default_catalog_when_enforced();
#endif
    test_catalog_root_auto_discovery_preserves_executable_candidate_precedence();
    test_catalog_root_explicit_override_remains_authoritative();
    test_catalog_root_resolution_searches_parent_directories();
    test_catalog_root_resolution_finds_repo_build_output_layout_from_executable_path();
    test_catalog_root_resolution_finds_repo_build_output_layout_from_path_launched_basename();
    test_parser_behavior_remains_locale_invariant();
    test_runtime_session_diagnostics_route_through_catalog();
    test_runtime_cursor_diagnostics_route_through_catalog();
    test_runtime_total_diagnostics_route_through_catalog();
    test_runtime_report_output_messages_route_through_catalog();
    test_runtime_report_output_errors_localize_without_changing_runtime_behavior();
    test_runtime_aggregate_errors_route_through_catalog();
    test_runtime_sql_errors_route_through_catalog();
    test_build_host_catalog_entries_cover_placeholder_locales();
    test_inspect_catalog_entries_cover_placeholder_locales();
    test_shared_core_catalog_entries_cover_placeholder_locales();
    test_runtime_host_manifest_verification_errors_route_through_catalog();
    test_runtime_host_quit_prompt_routes_through_catalog();
    test_runtime_host_security_policy_denial_routes_through_catalog();
    test_platform_federation_ai_planner_fallback_routes_through_catalog();
    test_runtime_numeric_domain_errors_route_through_catalog();
    test_runtime_expression_errors_route_through_catalog();
    test_runtime_record_precondition_errors_route_through_catalog();
    test_runtime_dll_errors_route_through_catalog();
    test_runtime_core_errors_route_through_catalog();
    test_runtime_pause_and_session_messages_route_through_catalog();
    test_runtime_watch_errors_route_through_catalog();
    test_runtime_dispatch_errors_route_through_catalog();
    test_runtime_surface_errors_route_through_catalog();
    test_runtime_save_restore_errors_route_through_catalog();
    test_runtime_file_operation_errors_route_through_catalog();
    test_runtime_copy_to_errors_route_through_catalog();
    test_runtime_append_from_array_errors_route_through_catalog();
    test_runtime_append_from_errors_route_through_catalog();
    test_runtime_append_from_type_errors_route_through_catalog();
    test_runtime_scatter_gather_errors_route_through_catalog();
    test_runtime_dispatch_array_and_object_target_errors_route_through_catalog();
    test_runtime_table_structure_errors_route_through_catalog();
    test_runtime_set_filter_dimension_sleep_errors_route_through_catalog();
    test_runtime_declare_dispatch_errors_route_through_catalog();
    test_runtime_residual_command_dispatch_errors_route_through_catalog();
    test_runtime_object_helper_dispatch_errors_route_through_catalog();
    test_runtime_ole_invocation_and_property_read_errors_route_through_catalog();
    test_runtime_package_warnings_pseudo_localize();
    if (argc > 1) {
        test_inspect_usage_routes_through_localization(argv[1]);
        test_inspect_accepts_posix_locale_suffixes(argv[1]);
        test_inspect_explicit_locale_routes_dbf_version_display(argv[1]);
        test_inspect_license_status_preserves_machine_contracts(argv[1]);
        test_inspect_error_prefix_routes_through_localization(argv[1]);
    } else {
        expect(false, "#1779: test_localization requires the copperfin_inspect executable path");
    }

    return test_failures() == 0 ? 0 : 1;
}
