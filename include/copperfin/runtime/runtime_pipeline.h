// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/platform/extensibility_model.h"
#include "copperfin/security/security_model.h"
#include "copperfin/studio/document_model.h"
#include "copperfin/studio/project_workspace.h"

#include <cstddef>
#include <string>
#include <vector>

namespace copperfin::runtime {

enum class BuildConfiguration {
    debug,
    release
};

enum class BuildOutputKind {
    executable,
    app,
    dll,
    fll,
    fxp,
    ocx,
    unknown
};

struct RuntimePackageAsset {
    std::size_t record_index = 0;
    std::string source_path;
    std::string staged_path;
    std::string relative_path;
    std::string type_title;
    bool excluded = false;
    bool exists = false;
    bool required_for_runtime = false;
    bool package_writable = false;
    bool copied = false;
    std::string sha256;
    std::string source_resolution_error;
};

struct RuntimeArtifactDigest {
    std::string path;
    std::string sha256;
};

enum class RuntimeLauncherArtifactRole {
    public_apphost,
    runtime_required,
    debug_optional
};

struct RuntimeLauncherArtifact {
    std::string package_relative_path;
    RuntimeLauncherArtifactRole role = RuntimeLauncherArtifactRole::runtime_required;
    std::string sha256;
};

struct RuntimeDebugLaunchPlan {
    std::string manifest_path;
    std::string startup_item;
    std::string startup_source_path;
    std::string working_directory;
    std::vector<std::string> source_roots;
    bool supports_breakpoints = false;
    bool supports_step_debugging = false;
};

struct RuntimePackagePlan {
    bool ok = false;
    std::string project_path;
    std::string project_title;
    std::string package_root;
    std::string content_root;
    std::string manifest_path;
    std::string debug_manifest_path;
    std::string ast_manifest_path;
    std::string ir_manifest_path;
    std::string transpiled_csharp_path;
    std::string launcher_project_path;
    std::string launcher_source_path;
    std::string launcher_output_path;
    std::string module_definition_path;
    std::string native_wrapper_source_path;
    std::string native_wrapper_cmake_path;
    std::string native_wrapper_build_script_path;
    std::string native_wrapper_build_powershell_path;
    std::string library_api_manifest_path;
    std::string fll_api_manifest_path;
    std::string fxp_token_manifest_path;
    std::string app_archive_manifest_path;
    std::string runtime_host_destination_path;
    std::string startup_item;
    std::string startup_source_path;
    std::string working_directory;
    std::string security_role;
    std::string audit_log_path;
    std::string runtime_host_sha256;
    std::string license_state;
    std::string license_type;
    std::string license_id;
    std::string license_licensee;
    int license_seats = 0;
    std::string license_subscription_expires;
    int license_perpetual_max_major_version = 0;
    std::string license_source_path;
    BuildConfiguration configuration = BuildConfiguration::debug;
    BuildOutputKind output_kind = BuildOutputKind::executable;
    bool security_enabled = false;
    bool requested_dotnet_launcher = false;
    bool emit_dotnet_launcher = true;
    bool primary_output_materialized = false;
    std::string launcher_mode;
    std::string launcher_fallback;
    std::vector<RuntimePackageAsset> assets;
    std::vector<std::string> exported_symbols;
    std::vector<RuntimeArtifactDigest> compiler_contract_digests;
    std::vector<RuntimeArtifactDigest> extension_payload_digests;
    std::vector<RuntimeArtifactDigest> writable_data_payload_digests;
    std::vector<RuntimeLauncherArtifact> launcher_artifacts;
    RuntimeDebugLaunchPlan debug_plan{};
    std::vector<std::string> warnings;
    std::size_t planning_warning_count = 0;
    bool planning_warnings_captured = false;
};

struct RuntimeMaterializeResult {
    bool ok = false;
    RuntimePackagePlan plan{};
    std::string error;
};

struct RuntimeBuildResult {
    bool ok = false;
    RuntimePackagePlan plan{};
    std::string error;
};

[[nodiscard]] const char* build_configuration_name(BuildConfiguration configuration);
[[nodiscard]] BuildConfiguration parse_build_configuration(const std::string& value);
[[nodiscard]] const char* build_output_kind_name(BuildOutputKind output_kind);

[[nodiscard]] RuntimePackagePlan create_runtime_package_plan(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectWorkspace& workspace,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile,
    const std::string& output_root,
    BuildConfiguration configuration,
    bool enable_security,
    bool emit_dotnet_launcher);

[[nodiscard]] RuntimePackagePlan create_runtime_package_plan(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectWorkspace& workspace,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile,
    const std::string& output_root,
    BuildConfiguration configuration,
    bool enable_security,
    bool emit_dotnet_launcher,
    const std::vector<std::string>& external_include_roots);

[[nodiscard]] std::string build_runtime_manifest_text(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile);

[[nodiscard]] std::string build_debug_manifest_text(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile);

RuntimeMaterializeResult materialize_runtime_package(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile,
    const std::string& runtime_host_source_path);

RuntimeBuildResult finalize_runtime_package_primary_output(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile);

// Aborts a deferred generated-launcher or native-library package transaction
// after an external publication step fails, restoring the last complete
// package when one exists.
RuntimeBuildResult abort_runtime_package_transaction(
    const RuntimePackagePlan& plan);

RuntimeBuildResult build_runtime_package_primary_output(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile);

}  // namespace copperfin::runtime
