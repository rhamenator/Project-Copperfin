// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_runtime_pipeline_support.h"

using namespace cf_test_runtime_pipeline;

int main() {
    test_materialize_runtime_package();
    test_generated_launcher_forwards_manifest_and_debug_flag();
    test_materialize_excluded_xasset_startup_package();
    test_vfp_style_parent_relative_assets_resolve_and_stage_under_content_root();
    test_vfp_source_layout_parent_relative_assets_resolve_by_tail_match();
    test_dotnet_launcher_request_falls_back_to_native_host_when_unavailable();
    test_library_output_package_emits_module_definition_from_prg_routines();
    test_fll_output_package_emits_api_manifest_from_prg_routines();
    test_library_output_warning_lines_are_mirrored_into_debug_manifest();
    test_fxp_output_package_emits_token_manifest_from_prg_statements();
    test_app_output_package_emits_archive_manifest_for_staged_assets();
    test_runtime_package_emits_ast_manifest_for_prg_sources();
    test_runtime_package_emits_ir_manifest_with_instruction_mapping();
    test_runtime_package_emits_csharp_transpilation_for_procedural_prg_code();
    test_runtime_package_emits_csharp_transpilation_for_class_library_objects();
    test_runtime_manifest_records_generated_compiler_contract_digests();
    test_startup_dbf_companion_assets_are_staged();
    test_runtime_package_license_fields_bump_manifest_schema_versions();
    test_security_enabled_runtime_host_name_validation();
    test_runtime_security_role_environment_fidelity();
    test_materialize_fails_before_asset_staging_when_runtime_host_source_is_invalid();
    test_startup_prg_extension_matching_is_case_insensitive();
    test_startup_asset_is_staged_even_when_marked_excluded();
    test_missing_startup_record_surfaces_plan_warnings_and_disables_debug_startup_support();
    test_manifest_asset_lines_include_copy_state_contract();
    test_debug_source_roots_are_unique_when_source_and_content_paths_match();
    test_debug_source_roots_preserve_source_first_and_content_second_order();
    test_runtime_package_diagnostics_resolve_through_localization_catalog();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
