// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#ifndef COPPERFIN_TEST_RUNTIME_PIPELINE_SUPPORT_H
#define COPPERFIN_TEST_RUNTIME_PIPELINE_SUPPORT_H

#include "test_environment_support.h"
#include "copperfin/localization/localization.h"
#include "copperfin/platform/extensibility_model.h"
#include "copperfin/runtime/runtime_pipeline.h"
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
void write_text(const std::filesystem::path& path, const std::string& contents);
std::filesystem::path runtime_host_fixture_path(const std::filesystem::path& root);
std::string read_text(const std::filesystem::path& path);
std::string hex_decode_bytes(const std::string& encoded);
std::unordered_map<std::string, std::string> parse_app_archive_payloads(const std::string& archive_text);
std::string trim_copy(std::string value);
std::string quote_manifest_value(const std::string& value);
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
bool shell_is_available();
bool compile_native_wrapper_scaffold(
    const std::filesystem::path& source_path,
    std::filesystem::path& output_path,
    std::string& error);
bool build_native_wrapper_with_cmake(
    const std::filesystem::path& cmake_lists_path,
    const std::filesystem::path& expected_output_path,
    std::filesystem::path& output_path,
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
void test_runtime_package_license_fields_stay_debug_only();

// ==== Runtime package materialization and asset-staging tests ====
void test_materialize_runtime_package();
void test_materialize_excluded_xasset_startup_package();
void test_uppercase_xasset_companion_assets_are_staged();
void test_menu_startup_assets_are_staged();
void test_report_startup_assets_are_staged();
void test_label_startup_assets_are_staged();
void test_vfp_style_parent_relative_assets_resolve_and_stage_under_content_root();
void test_vfp_source_layout_parent_relative_assets_resolve_by_tail_match();
void test_startup_dbf_companion_assets_are_staged();
void test_uppercase_dbf_companion_assets_are_staged();
void test_materialize_fails_before_asset_staging_when_runtime_host_source_is_invalid();
void test_startup_prg_extension_matching_is_case_insensitive();
void test_xasset_startup_extension_matching_is_case_insensitive();
void test_startup_asset_is_staged_even_when_marked_excluded();
void test_missing_startup_record_surfaces_plan_warnings_and_disables_debug_startup_support();
void test_debug_source_roots_are_unique_when_source_and_content_paths_match();
void test_debug_source_roots_preserve_source_first_and_content_second_order();

// ==== Generated-launcher, security, and localization diagnostics tests ====
void test_generated_launcher_forwards_manifest_and_debug_flag();
void test_dotnet_launcher_request_falls_back_to_native_host_when_unavailable();
void test_dotnet_launcher_finalization_rewrites_manifest_after_publish_output_materializes();
void test_security_enabled_runtime_host_name_validation();
void test_runtime_security_role_environment_fidelity();
void test_runtime_package_diagnostics_resolve_through_localization_catalog();

}  // namespace cf_test_runtime_pipeline

#endif
