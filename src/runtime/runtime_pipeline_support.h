// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#ifndef COPPERFIN_RUNTIME_PIPELINE_SUPPORT_H
#define COPPERFIN_RUNTIME_PIPELINE_SUPPORT_H

#include "copperfin/runtime/runtime_pipeline.h"
#include "copperfin/platform/json.h"
#include "copperfin/platform/path.h"
#include "localized_text.h"
#include "copperfin/localization/localization.h"
#include "copperfin/runtime/xasset_methods.h"
#include "prg_engine_internal.h"
#include "copperfin/security/sha256.h"

#include <cctype>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <optional>
#include <sstream>
#include <unordered_set>



namespace copperfin::runtime {

namespace runtime_pipeline_detail {

constexpr std::string_view kFllLoaderEntrypoint = "FoxInfo";
constexpr std::string_view kFllRegistrationSymbol = "_FoxTable";
constexpr std::string_view kFllCallableSignature = "ParamBlk*";
constexpr std::string_view kFllDefaultReturnHelper = "_RetInt";
constexpr std::string_view kVfpLibraryCallableConvention = "vfp_declare_default";

// ==== Text/identifier sanitization helpers ====
std::string sanitize_file_name(const std::string& value);
std::string trim_copy(std::string value);
std::string lowercase_copy(std::string value);
std::string canonical_casefolded_path_identity(const std::filesystem::path& path);
std::string quote_manifest_value(const std::string& value);
std::vector<std::string> unique_non_empty_paths_preserve_order(std::initializer_list<std::string> values);
std::string normalize_export_symbol(std::string value);
std::string json_escape(std::string_view value);
std::string extract_declared_parameter_name(const std::string& raw_name);
std::string sanitize_cpp_identifier(const std::string& value, const std::size_t fallback_index);
std::string sanitize_csharp_identifier(std::string value, const std::string& fallback);
std::string sanitize_csharp_routine_identifier(std::string value, const std::string& fallback);
std::string unquote_literal(std::string value);
std::string sanitize_csharp_compound_identifier(std::string value, const std::string& fallback);
std::string join_strings(const std::vector<std::string>& values);

// ==== File I/O, path classification, and security-role helpers ====
BuildOutputKind parse_build_output_kind(const std::string& value);
std::string dotnet_parity_tier_name(copperfin::platform::DotNetParityTier tier);
bool write_text_file(const std::filesystem::path& path, const std::string& contents, std::string& error);
bool write_text_file(const std::string& utf8_path, const std::string& contents, std::string& error);
bool write_runtime_manifest_pair_atomically(
    const RuntimePackagePlan& plan,
    const std::string& runtime_contents,
    const std::string& debug_contents,
    std::string& error);
std::string read_text_file(const std::filesystem::path& path);
std::string read_binary_file(const std::filesystem::path& path, std::string& error);
std::string hex_encode_bytes(const std::string& bytes);
bool append_runtime_artifact_digest(
    std::vector<RuntimeArtifactDigest>& digests,
    const std::string& path,
    std::string& error);
std::string_view launcher_artifact_role_name(RuntimeLauncherArtifactRole role);
bool inventory_generated_launcher_artifacts(
    const RuntimePackagePlan& plan,
    std::vector<RuntimeLauncherArtifact>& inventory,
    std::string& error);
bool validate_public_output_artifact_name(
    const RuntimePackagePlan& plan,
    std::string& error);
bool is_launcher_owned_digest(
    const RuntimeArtifactDigest& digest,
    const RuntimePackagePlan& plan);
bool is_library_output_kind(const BuildOutputKind output_kind);
bool is_native_host_output_kind(const BuildOutputKind output_kind);
std::string runtime_host_file_name();
std::string resolve_output_file_name(const studio::StudioProjectWorkspace& workspace, const std::string& project_title);
BuildOutputKind infer_build_output_kind_from_output_path(const std::string& output_path);
bool copy_file_if_exists(
    const std::filesystem::path& source,
    const std::filesystem::path& destination,
    std::string& error);
bool copy_file_if_exists(
    const std::filesystem::path& source,
    const std::string& utf8_destination,
    std::string& error);
#if !defined(_WIN32)
bool is_fd_backed_runtime_path(const std::filesystem::path& path);
bool try_copy_file_if_exists_fd_backed(
    const std::filesystem::path& source,
    const std::filesystem::path& destination,
    bool& handled,
    std::string& error);
bool try_read_file_fd_backed(
    const std::filesystem::path& source,
    bool& handled,
    std::string& contents);
bool try_collect_fd_backed_regular_files(
    const std::filesystem::path& root,
    bool& handled,
    std::vector<std::filesystem::path>& relative_files);
bool try_write_text_file_fd_backed(
    const std::filesystem::path& destination,
    const std::string& contents,
    bool& handled,
    std::string& error);
#endif
bool prepare_package_content_root(
    const std::filesystem::path& package_root,
    const std::filesystem::path& content_root,
    std::string& error,
    int* content_descriptor = nullptr);
bool copy_file_to_package_content(
    const std::filesystem::path& source,
    const std::filesystem::path& package_root,
    const std::filesystem::path& content_root,
    const std::filesystem::path& relative_path,
    std::filesystem::path& destination,
    std::string& error);
bool validate_runtime_host_source_path(
    const RuntimePackagePlan& plan,
    const std::string& runtime_host_source_path,
    std::string& error);
std::string resolve_project_item_source(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectEntry& entry,
    bool require_unique_casefold,
    std::string& error);
std::vector<std::filesystem::path> discover_prg_include_source_paths(
    const std::filesystem::path& source,
    const std::vector<std::filesystem::path>& external_include_roots,
    std::vector<std::string>& warnings);
std::vector<std::filesystem::path> discover_prg_literal_library_source_paths(
    const std::filesystem::path& source);
std::vector<std::filesystem::path> discover_prg_literal_do_source_paths(
    const std::filesystem::path& source);
std::vector<std::filesystem::path> discover_vcx_literal_library_source_paths(
    const std::filesystem::path& source);
bool source_path_exists_on_host(const std::string& value);
std::string relative_asset_path(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectEntry& entry,
    const std::string& resolved_source_path,
    bool preserve_resolved_spelling);
std::string resolve_working_directory(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectWorkspace& workspace);
std::string resolve_security_role(bool security_enabled);
bool is_extension_payload_path(const std::filesystem::path& path);
bool is_recognized_security_role(
    const security::NativeSecurityProfile& profile,
    const std::string& role_id);
bool is_prg_path(const std::string& value);
bool is_xasset_path(const std::string& value);
bool is_writable_package_data_path(const std::string& value);
bool should_stage_asset(const RuntimePackageAsset& asset);
std::optional<std::filesystem::path> resolve_existing_path_casefold(
    const std::filesystem::path& candidate,
    bool& ambiguous);
std::filesystem::path normalize_existing_path_spelling(
    const std::filesystem::path& candidate);
std::vector<std::filesystem::path> infer_companion_source_paths(const std::filesystem::path& source);
struct RuntimeCompanionCopyResult {
    bool ok = true;
    std::vector<std::filesystem::path> copied_paths;
    std::string error;
};
RuntimeCompanionCopyResult copy_companion_files_if_present(
    const RuntimePackageAsset& asset,
    const std::filesystem::path& package_root,
    const std::filesystem::path& content_root,
    std::vector<std::string>& warnings);

// ==== DLL/FLL native-wrapper and library-export manifest generation ====
struct NativeWrapperProcessResult {
    bool started = false;
    int exit_code = -1;
};
NativeWrapperProcessResult run_native_wrapper_process(
    const std::string& executable,
    const std::vector<std::string>& arguments,
    const std::filesystem::path& output_log_path);
std::vector<std::string> collect_library_exported_symbols(const RuntimePackagePlan& plan);
std::map<std::string, std::size_t> collect_library_export_parameter_counts(const RuntimePackagePlan& plan);
std::string build_routine_kind_name(const RoutineKind kind);
std::map<std::string, std::vector<std::string>> collect_library_export_parameter_names(const RuntimePackagePlan& plan);
std::map<std::string, std::string> collect_library_export_parameter_declaration_kinds(const RuntimePackagePlan& plan);
std::map<std::string, std::string> collect_library_export_routine_kinds(const RuntimePackagePlan& plan);
std::map<std::string, SourceLocation> collect_library_export_routine_locations(const RuntimePackagePlan& plan);
std::string build_placeholder_int_parameter_list(const std::vector<std::string>& parameter_names);
std::string build_manifest_parameter_names(const std::vector<std::string>& parameter_names);
std::string build_manifest_source_location(const SourceLocation& location);
void append_native_wrapper_compilation_preamble(std::ostringstream& stream);
void append_native_wrapper_host_authentication_source(std::ostringstream& stream);
void append_native_wrapper_bridge_model_source(std::ostringstream& stream);
void append_native_wrapper_request_serialization_source(std::ostringstream& stream);
void append_native_wrapper_process_launch_source(std::ostringstream& stream);
void append_native_wrapper_response_handling_source(std::ostringstream& stream);
void append_native_wrapper_library_entrypoint_source(std::ostringstream& stream, const RuntimePackagePlan& plan);
std::string build_module_definition_source(const RuntimePackagePlan& plan);
std::string build_native_wrapper_source(const RuntimePackagePlan& plan);
std::string build_native_wrapper_cmake_source(const RuntimePackagePlan& plan);
std::string build_native_wrapper_shell_script_source();
std::string build_native_wrapper_powershell_script_source();
std::string build_fll_api_manifest_source(const RuntimePackagePlan& plan);
std::string build_library_api_manifest_source(const RuntimePackagePlan& plan);

// ==== FXP token/app-archive manifest generation and primary-output writing ====
void append_fxp_statement_lines(
    std::ostringstream& stream,
    const std::string& scope_name,
    const std::vector<Statement>& statements);
std::string build_fxp_token_manifest_source(const RuntimePackagePlan& plan);
bool write_fxp_primary_output_contract(
    const RuntimePackagePlan& plan,
    const std::string& token_manifest_text,
    const std::string& output_path,
    std::string& error);
std::string build_app_archive_manifest_source(const RuntimePackagePlan& plan);
bool write_app_archive_primary_output(
    const RuntimePackagePlan& plan,
    const RuntimePackagePlan& filesystem_plan,
    std::string& error);
void append_library_function_manifest_lines(
    std::ostringstream& stream,
    const RuntimePackagePlan& plan,
    bool include_source_provenance);
void append_runtime_asset_manifest_lines(std::ostringstream& stream, const RuntimePackagePlan& plan);
void append_writable_data_manifest_lines(std::ostringstream& stream, const RuntimePackagePlan& plan);
void append_warning_manifest_lines(std::ostringstream& stream, const RuntimePackagePlan& plan);
void append_runtime_feature_flag_manifest_lines(
    std::ostringstream& stream,
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile);
void append_feature_flag_line(
    std::ostringstream& stream,
    std::string_view name,
    bool enabled,
    std::string_view category);

// ==== AST/IR debug manifest generation ====
const char* statement_kind_name(const StatementKind kind);
void append_ast_routine_json(
    std::ostringstream& stream,
    const std::string& routine_name,
    const std::vector<Statement>& statements);
std::string build_ast_manifest_source(const RuntimePackagePlan& plan);
void append_ir_routine_json(
    std::ostringstream& stream,
    const std::string& routine_name,
    const std::vector<Statement>& statements);
std::string build_ir_manifest_source(const RuntimePackagePlan& plan);

// ==== C# transpilation and .NET launcher source generation ====
std::map<std::string, std::map<std::string, std::string>> build_generated_launcher_localized_messages();
void append_generated_launcher_localization_helpers(std::ostringstream& stream);
std::map<std::string, std::map<std::string, std::string>> build_generated_csharp_localized_messages();
void append_generated_csharp_localization_helpers(std::ostringstream& stream);
std::string transpile_statement_to_csharp(
    const Statement& statement,
    const std::map<std::string, std::string>& routine_name_map);
std::string build_xasset_csharp_method_identifier(
    const XAssetExecutableModel& model,
    const XAssetMethod& method);
void append_xasset_csharp_type(
    std::ostringstream& stream,
    const studio::StudioDocumentModel& document);
std::string build_csharp_transpilation_source(const RuntimePackagePlan& plan);
std::string build_launcher_program_source(const RuntimePackagePlan&);
std::string build_launcher_project_source(const RuntimePackagePlan& plan);

}  // namespace runtime_pipeline_detail
using namespace runtime_pipeline_detail;

// ==== Public package-planning and build/materialize entry points ====
const char* build_configuration_name(BuildConfiguration configuration);
BuildConfiguration parse_build_configuration(const std::string& value);
const char* build_output_kind_name(BuildOutputKind output_kind);
RuntimePackagePlan create_runtime_package_plan(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectWorkspace& workspace,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile,
    const std::string& output_root,
    BuildConfiguration configuration,
    bool enable_security,
    bool emit_dotnet_launcher);
RuntimePackagePlan create_runtime_package_plan(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectWorkspace& workspace,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile,
    const std::string& output_root,
    BuildConfiguration configuration,
    bool enable_security,
    bool emit_dotnet_launcher,
    const std::vector<std::string>& external_include_roots);
std::string build_runtime_manifest_text(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile);
std::string build_debug_manifest_text(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile);
RuntimeMaterializeResult materialize_runtime_package(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile,
    const std::string& runtime_host_source_path);
RuntimeBuildResult build_runtime_package_primary_output(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile);

}  // namespace copperfin::runtime

#endif
