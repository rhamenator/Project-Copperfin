// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_runtime_pipeline_support.h"

#include <charconv>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <string_view>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace cf_test_runtime_pipeline {
void test_absolute_project_item_paths_never_rebind_to_project_decoys();
void test_file_valued_home_directory_falls_back_to_project_directory();
void test_relative_home_directory_resolves_from_project_directory();
}

using namespace cf_test_runtime_pipeline;

namespace {
#if defined(_WIN32)
template <typename T>
bool parse_unsigned_probe_field(std::string_view input, std::size_t& offset, T& value) {
    const std::size_t delimiter = input.find(',', offset);
    const std::size_t end = delimiter == std::string_view::npos ? input.size() : delimiter;
    if (end == offset) {
        return false;
    }

    const auto parsed = std::from_chars(input.data() + offset, input.data() + end, value);
    if (parsed.ec != std::errc{} || parsed.ptr != input.data() + end) {
        return false;
    }

    offset = delimiter == std::string_view::npos ? end : delimiter + 1U;
    return true;
}

int run_inherited_handle_probe() {
    char value[256]{};
    const DWORD length = ::GetEnvironmentVariableA(
        "COPPERFIN_INHERITED_HANDLE_PROBE",
        value,
        static_cast<DWORD>(sizeof(value)));
    unsigned long long raw_handle = 0U;
    unsigned long volume = 0U;
    unsigned long index_high = 0U;
    unsigned long index_low = 0U;
    const std::string_view input(value, length);
    std::size_t offset = 0U;
    const bool parsed = length > 0U &&
        parse_unsigned_probe_field(input, offset, raw_handle) &&
        parse_unsigned_probe_field(input, offset, volume) &&
        parse_unsigned_probe_field(input, offset, index_high) &&
        parse_unsigned_probe_field(input, offset, index_low) &&
        offset == input.size();
    BY_HANDLE_FILE_INFORMATION information{};
    const bool matches = parsed &&
        ::GetFileInformationByHandle(
            reinterpret_cast<HANDLE>(static_cast<std::uintptr_t>(raw_handle)),
            &information) != 0 &&
        information.dwVolumeSerialNumber == volume &&
        information.nFileIndexHigh == index_high &&
        information.nFileIndexLow == index_low;
    std::cout << (matches ? "inherited\n" : "not-inherited\n");
    return matches ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif

int run_runtime_pipeline_tests(const std::filesystem::path& executable_path) {
    const ScopedRuntimePipelineFixtureNamespace fixture_namespace;
    const ScopedEnvironmentVariable locale_root(
        "COPPERFIN_LOCALE_DIR",
        copperfin::test_support::path_to_utf8_string(runtime_pipeline_locale_root()));

    test_runtime_pipeline_fixtures_are_process_isolated(executable_path);
    test_materialize_runtime_package();
    test_concurrent_materialization_is_serialized_per_package_root(executable_path);
    test_package_transaction_rejects_rebound_output_parent();
    test_package_content_root_remains_pinned_during_asset_writes();
    test_windows_nested_package_parent_rebind_fails_closed();
    test_casefold_startup_paths_preserve_actual_spelling_for_all_mvp_families();
    test_exact_startup_path_wins_over_casefold_siblings();
    test_ambiguous_casefold_startup_path_fails_closed();
    test_startup_resolution_preserves_parent_tail_and_name_fallbacks();
    test_absolute_project_item_paths_never_rebind_to_project_decoys();
    test_file_valued_home_directory_falls_back_to_project_directory();
    test_relative_home_directory_resolves_from_project_directory();
    test_missing_startup_primary_fails_for_all_mvp_families();
    test_missing_required_startup_sidecar_fails_for_all_xasset_families();
    test_unicode_runtime_package_paths_preserve_source_and_manifest_contracts();
    test_generated_launcher_forwards_manifest_and_debug_flag();
    test_materialize_excluded_xasset_startup_package();
    test_uppercase_xasset_companion_assets_are_staged();
    test_ambiguous_casefold_xasset_companions_fail_closed();
    test_form_startup_assets_are_staged();
    test_class_library_startup_assets_are_staged();
    test_menu_startup_assets_are_staged();
    test_report_startup_assets_are_staged();
    test_label_startup_assets_are_staged();
    test_unicode_report_and_label_startup_assets_are_staged();
    test_vfp_style_parent_relative_assets_resolve_and_stage_under_content_root();
    test_vfp_source_layout_parent_relative_assets_resolve_by_tail_match();
    test_dotnet_launcher_request_falls_back_to_native_host_when_unavailable();
    test_dotnet_launcher_finalization_rewrites_manifest_after_publish_output_materializes();
    test_primary_output_status_errors_are_reported_as_missing();
    test_deferred_package_transaction_rolls_back_failed_second_build();
    test_package_output_names_reject_reserved_artifacts();
    test_manifest_pair_finalization_rejects_redirected_destinations();
    test_manifest_pair_directory_stays_pinned_and_never_overwrites();
    test_manifest_pair_finalization_rolls_back_failed_promotions();
    test_manifest_pair_finalization_recovers_stale_transactions();
    test_materialize_cleanup_warning_rewrites_manifest_pair_atomically();
    test_write_text_file_reports_close_failure();
    test_drive_relative_asset_paths_use_contained_package_identity();
    test_materialization_rejects_external_asset_destinations();
    test_optional_rejected_asset_identity_stays_out_of_manifests();
    test_materialization_rejects_external_content_root();
    test_package_content_copy_rejects_indirect_parent();
    test_package_content_copy_rejects_hard_link_destination();
    test_relative_output_root_preserves_plan_path_contract();
    test_library_manifest_source_location_escaping();
    test_library_output_package_emits_module_definition_from_prg_routines();
    test_native_wrapper_primary_output_handles_literal_shell_paths();
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
    test_uppercase_dbf_companion_assets_are_staged();
    test_writable_dbf_assets_use_data_manifest_surface_and_dbc_stays_immutable();
    test_runtime_package_license_fields_stay_debug_only();
    test_staged_asset_destination_collisions_are_rejected();
    test_security_enabled_runtime_host_name_validation();
    test_runtime_security_role_environment_fidelity();
    test_materialize_fails_before_asset_staging_when_runtime_host_source_is_invalid();
    test_startup_prg_extension_matching_is_case_insensitive();
    test_xasset_startup_extension_matching_is_case_insensitive();
    test_startup_asset_is_staged_even_when_marked_excluded();
    test_missing_startup_record_surfaces_plan_warnings_and_disables_debug_startup_support();
    test_manifest_asset_lines_include_copy_state_contract();
    test_runtime_package_stages_recursive_prg_include_dependencies();
    test_runtime_package_stages_unicode_prg_include_dependencies();
    test_runtime_package_stages_literal_newobject_library_dependencies();
    test_runtime_package_stages_nested_vcx_newobject_dependencies();
    test_runtime_package_scans_dependencies_after_include_enqueue();
    test_debug_source_roots_are_unique_when_source_and_content_paths_match();
    test_debug_source_roots_preserve_source_first_and_content_second_order();
    test_fd_backed_binary_reads_accept_direct_descriptor_paths();
    test_repeated_materialization_replaces_generated_package_transactionally();
    test_runtime_package_diagnostics_resolve_through_localization_catalog();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
}  // namespace

#if defined(_WIN32)
int wmain(int argc, wchar_t* argv[]) {
    if (argc == 2 && std::wstring(argv[1]) == L"--copperfin-inherited-handle-probe") {
        return run_inherited_handle_probe();
    }
    if (argc == 6 && std::wstring(argv[1]) == L"--fixture-isolation-probe") {
        std::string probe_id;
        for (const wchar_t character : std::wstring(argv[2])) {
            probe_id.push_back(static_cast<char>(character));
        }
        return run_runtime_pipeline_fixture_isolation_probe(
            probe_id,
            std::filesystem::path(argv[3]),
            std::filesystem::path(argv[4]),
            std::filesystem::path(argv[5]));
    }
    return run_runtime_pipeline_tests(std::filesystem::absolute(std::filesystem::path(argv[0])));
}
#else
int main(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) == "--copperfin-inherited-handle-probe") {
        return EXIT_FAILURE;
    }
    if (argc == 6 && std::string(argv[1]) == "--fixture-isolation-probe") {
        return run_runtime_pipeline_fixture_isolation_probe(
            argv[2],
            argv[3],
            argv[4],
            argv[5]);
    }
    return run_runtime_pipeline_tests(std::filesystem::absolute(argv[0]));
}
#endif
