// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#ifndef COPPERFIN_TEST_RUNTIME_PIPELINE_SUPPORT_H
#define COPPERFIN_TEST_RUNTIME_PIPELINE_SUPPORT_H

#include "test_environment_support.h"
#include "copperfin/localization/localization.h"
#include "copperfin/platform/path.h"
#include "copperfin/platform/extensibility_model.h"
#include "copperfin/runtime/runtime_pipeline.h"
#include "copperfin/security/sha256.h"
#include "copperfin/security/security_model.h"
#include "copperfin/studio/project_workspace.h"
#include "copperfin/vfp/dbf_table.h"

#include <algorithm>
#include <cerrno>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>

#if defined(_WIN32)
#include <process.h>
#else
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#endif


namespace cf_test_runtime_pipeline {

// ==== Shared test helpers and fixtures ====
void expect(bool condition, const std::string& message);
void expect_materialization(
    const copperfin::runtime::RuntimeMaterializeResult& result,
    const std::string& message);
void write_text(const std::filesystem::path& path, const std::string& contents);
std::filesystem::path runtime_host_fixture_path(const std::filesystem::path& root);
std::string read_text(const std::filesystem::path& path);
bool paths_refer_to_same_filesystem_entry(
    const std::filesystem::path& actual,
    const std::filesystem::path& expected);
bool paths_refer_to_same_filesystem_entry(
    const std::string& actual_utf8,
    const std::filesystem::path& expected);
std::string decode_manifest_value(const std::string& value);

class ScopedRuntimePipelineFixtureNamespace {
public:
    ScopedRuntimePipelineFixtureNamespace();
    ScopedRuntimePipelineFixtureNamespace(const ScopedRuntimePipelineFixtureNamespace&) = delete;
    ScopedRuntimePipelineFixtureNamespace& operator=(const ScopedRuntimePipelineFixtureNamespace&) = delete;
    ~ScopedRuntimePipelineFixtureNamespace();

    [[nodiscard]] const std::filesystem::path& root() const;

private:
    std::filesystem::path root_;
    copperfin::test_support::ScopedEnvironmentValue tmpdir_;
    copperfin::test_support::ScopedEnvironmentValue temp_;
    copperfin::test_support::ScopedEnvironmentValue tmp_;
};

int run_runtime_pipeline_fixture_isolation_probe(
    const std::string& probe_id,
    const std::filesystem::path& ready_path,
    const std::filesystem::path& go_path,
    const std::filesystem::path& result_path);
void test_runtime_pipeline_fixtures_are_process_isolated(
    const std::filesystem::path& executable_path);
void test_concurrent_materialization_is_serialized_per_package_root(
    const std::filesystem::path& executable_path);
void test_fd_backed_binary_reads_accept_direct_descriptor_paths();
void test_package_transaction_rejects_rebound_output_parent();
void test_package_content_root_remains_pinned_during_asset_writes();
void test_windows_nested_package_parent_rebind_fails_closed();
int run_materialization_lock_probe_process(
    const std::filesystem::path& executable_path,
    const std::filesystem::path& config_path,
    const std::filesystem::path& ready_path,
    const std::filesystem::path& go_path,
    const std::filesystem::path& result_path);

#if defined(_WIN32)
bool create_windows_junction(
    const std::filesystem::path& link,
    const std::filesystem::path& target);
bool create_windows_drive_mapping(
    const std::filesystem::path& target,
    std::filesystem::path& drive_root);
bool remove_windows_drive_mapping(
    const std::filesystem::path& target,
    const std::filesystem::path& drive_root);
#endif
std::string hex_decode_bytes(const std::string& encoded);
std::unordered_map<std::string, std::string> parse_app_archive_payloads(const std::string& archive_text);
std::string trim_copy(std::string value);
std::string quote_manifest_value(const std::string& value);
std::filesystem::path runtime_pipeline_locale_root();
const copperfin::localization::LocalizedCatalog& runtime_pipeline_english_catalog();
std::size_t count_missing_locale_keys(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view locale,
    const std::vector<std::string_view>& keys);
std::vector<std::string> lines_with_prefix(const std::string& text, const std::string& prefix);
std::string manifest_value_for_key(const std::string& text, const std::string& key);
using copperfin::test_support::getenv_value;
using copperfin::test_support::set_env_value;
bool dotnet_is_available();
std::string native_cxx_command();
bool native_cxx_is_available();
bool native_symbol_dump_is_available();
bool cmake_is_available();
bool ninja_multi_config_is_available();
bool shell_is_available();
bool compile_native_wrapper_scaffold(
    const std::filesystem::path& source_path,
    std::filesystem::path& output_path,
    std::string& error);
void test_generated_posix_bridge_environment_launch(const std::filesystem::path& wrapper_path);
void test_generated_bridge_runtime_host_verification(const std::filesystem::path& wrapper_path);
bool build_native_wrapper_with_cmake(
    const std::filesystem::path& cmake_lists_path,
    const std::filesystem::path& expected_output_path,
    std::filesystem::path& output_path,
    std::string& error);
bool build_native_wrapper_with_ninja_multi_config(
    const std::filesystem::path& cmake_lists_path,
    const std::filesystem::path& expected_output_path,
    std::string& error);
bool build_native_wrapper_with_script(
    const std::filesystem::path& script_path,
    const std::filesystem::path& expected_output_path,
    std::string& error);
bool runtime_pipeline_primary_output_build_supported();
std::set<std::string> read_native_exported_symbols(const std::filesystem::path& binary_path, std::string& error);
std::set<std::string> read_module_definition_exports(const std::filesystem::path& path);
std::set<std::string> read_fll_api_declared_symbols(const std::filesystem::path& path);
std::set<std::string> read_library_api_declared_symbols(const std::filesystem::path& path);
bool compile_csharp_artifact(const std::filesystem::path& source_path, std::string& error);
void write_synthetic_class_library_asset(const std::filesystem::path& table_path);
void run_library_output_warning_debug_manifest_smoke(const std::string& output_kind, const std::string& extension);

extern int failures;
struct ScopedEnvironmentVariable {
    copperfin::test_support::ScopedEnvironmentValue scoped_;

    explicit ScopedEnvironmentVariable(const std::string& var_name, const std::string& value)
        : scoped_(var_name) {
        scoped_.set(value);
    }

    explicit ScopedEnvironmentVariable(const std::string& var_name)
        : scoped_(var_name) {
    }
};

// ==== Output-package manifest tests (app/library/FLL/FXP, AST/IR manifests, C# transpilation, compiler-contract digests) ====
void test_library_output_package_emits_module_definition_from_prg_routines();
void test_native_wrapper_primary_output_handles_literal_shell_paths();
void test_library_manifest_source_location_escaping();
void test_library_api_manifest_arities_ignore_grouping_locale();
void test_library_native_wrapper_numeric_literals_ignore_grouping_locale();
void test_fll_output_package_emits_api_manifest_from_prg_routines();
void test_fxp_output_package_emits_token_manifest_from_prg_statements();
void test_library_output_warning_lines_are_mirrored_into_debug_manifest();
void test_app_output_package_emits_archive_manifest_for_staged_assets();
void test_runtime_package_emits_ast_manifest_for_prg_sources();
void test_runtime_package_emits_ir_manifest_with_instruction_mapping();
void test_runtime_package_emits_csharp_transpilation_for_procedural_prg_code();
void test_runtime_package_emits_csharp_transpilation_for_class_library_objects();
void test_runtime_manifest_records_generated_compiler_contract_digests();
void test_manifest_asset_lines_include_copy_state_contract();
void test_runtime_package_stages_recursive_prg_include_dependencies();
void test_runtime_package_admits_trusted_external_include_roots();
void test_runtime_package_stages_literal_do_dependencies();
void test_runtime_package_stages_unicode_prg_include_dependencies();
void test_runtime_package_stages_literal_newobject_library_dependencies();
void test_runtime_package_stages_nested_vcx_newobject_dependencies();
void test_runtime_package_scans_dependencies_after_include_enqueue();
void test_runtime_package_license_fields_stay_debug_only();

// ==== Runtime package materialization and asset-staging tests ====
void test_materialize_runtime_package();
void test_staged_asset_destination_collisions_are_rejected();
void test_casefold_startup_paths_preserve_actual_spelling_for_all_mvp_families();
void test_exact_startup_path_wins_over_casefold_siblings();
void test_ambiguous_casefold_startup_path_fails_closed();
void test_startup_resolution_preserves_parent_tail_and_name_fallbacks();
void test_missing_startup_primary_fails_for_all_mvp_families();
void test_missing_required_startup_sidecar_fails_for_all_xasset_families();
void test_unicode_runtime_package_paths_preserve_source_and_manifest_contracts();
void test_materialize_excluded_xasset_startup_package();
void test_uppercase_xasset_companion_assets_are_staged();
void test_ambiguous_casefold_xasset_companions_fail_closed();
void test_form_startup_assets_are_staged();
void test_class_library_startup_assets_are_staged();
void test_menu_startup_assets_are_staged();
void test_report_startup_assets_are_staged();
void test_label_startup_assets_are_staged();
void test_unicode_report_and_label_startup_assets_are_staged();
void test_vfp_style_parent_relative_assets_resolve_and_stage_under_content_root();
void test_vfp_source_layout_parent_relative_assets_resolve_by_tail_match();
void test_startup_dbf_companion_assets_are_staged();
void test_uppercase_dbf_companion_assets_are_staged();
void test_writable_dbf_assets_use_data_manifest_surface_and_dbc_stays_immutable();
void test_materialize_fails_before_asset_staging_when_runtime_host_source_is_invalid();
void test_startup_prg_extension_matching_is_case_insensitive();
void test_xasset_startup_extension_matching_is_case_insensitive();
void test_startup_asset_is_staged_even_when_marked_excluded();
void test_missing_startup_record_surfaces_plan_warnings_and_disables_debug_startup_support();
void test_debug_source_roots_are_unique_when_source_and_content_paths_match();
void test_debug_source_roots_preserve_source_first_and_content_second_order();
void test_repeated_materialization_replaces_generated_package_transactionally();

// ==== Generated-launcher, security, and localization diagnostics tests ====
void test_generated_launcher_forwards_manifest_and_debug_flag();
void test_dotnet_launcher_request_falls_back_to_native_host_when_unavailable();
void test_dotnet_launcher_finalization_rewrites_manifest_after_publish_output_materializes();
void test_primary_output_status_errors_are_reported_as_missing();
void test_deferred_package_transaction_rolls_back_failed_second_build();
void test_package_output_names_reject_reserved_artifacts();
void test_manifest_pair_finalization_rejects_redirected_destinations();
void test_manifest_pair_directory_stays_pinned_and_never_overwrites();
void test_manifest_pair_finalization_rolls_back_failed_promotions();
void test_manifest_pair_finalization_recovers_stale_transactions();
void test_materialize_cleanup_warning_rewrites_manifest_pair_atomically();
void test_write_text_file_reports_close_failure();
void test_drive_relative_asset_paths_use_contained_package_identity();
void test_materialization_rejects_external_asset_destinations();
void test_optional_rejected_asset_identity_stays_out_of_manifests();
void test_materialization_rejects_external_content_root();
void test_package_content_copy_rejects_indirect_parent();
void test_package_content_copy_rejects_hard_link_destination();
void test_relative_output_root_preserves_plan_path_contract();
void test_security_enabled_runtime_host_name_validation();
void test_runtime_security_role_environment_fidelity();
void test_runtime_package_diagnostics_resolve_through_localization_catalog();
void test_launcher_artifact_admission_rejects_rename_during_read();

}  // namespace cf_test_runtime_pipeline

#endif
