// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "runtime_pipeline_support.h"

namespace copperfin::runtime {
namespace {

constexpr int kRuntimeManifestVersion = 2;
constexpr int kDebugManifestVersion = 2;

}  // namespace

const char* build_configuration_name(BuildConfiguration configuration) {
    switch (configuration) {
        case BuildConfiguration::debug:
            return "debug";
        case BuildConfiguration::release:
            return "release";
    }
    return "debug";
}

BuildConfiguration parse_build_configuration(const std::string& value) {
    return trim_copy(value) == "release"
        ? BuildConfiguration::release
        : BuildConfiguration::debug;
}

const char* build_output_kind_name(BuildOutputKind output_kind) {
    switch (output_kind) {
        case BuildOutputKind::executable:
            return "executable";
        case BuildOutputKind::app:
            return "app";
        case BuildOutputKind::dll:
            return "dll";
        case BuildOutputKind::fll:
            return "fll";
        case BuildOutputKind::fxp:
            return "fxp";
        case BuildOutputKind::ocx:
            return "ocx";
        case BuildOutputKind::unknown:
            return "unknown";
    }
    return "unknown";
}

RuntimePackagePlan create_runtime_package_plan(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectWorkspace& workspace,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile,
    const std::string& output_root,
    BuildConfiguration configuration,
    bool enable_security,
    bool emit_dotnet_launcher) {
    RuntimePackagePlan plan;
    plan.project_path = document.path;
    plan.project_title = workspace.project_title.empty()
        ? std::filesystem::path(document.path).stem().string()
        : workspace.project_title;
    plan.configuration = configuration;
    plan.security_enabled = enable_security;
    plan.output_kind = parse_build_output_kind(workspace.build_plan.output_kind);
    if (plan.output_kind == BuildOutputKind::unknown) {
        plan.output_kind = infer_build_output_kind_from_output_path(workspace.build_plan.output_path);
    }
    plan.requested_dotnet_launcher = emit_dotnet_launcher;
    plan.emit_dotnet_launcher =
        is_native_host_output_kind(plan.output_kind) &&
        emit_dotnet_launcher &&
        extensibility_profile.dotnet_output.available;
    if (is_library_output_kind(plan.output_kind)) {
        plan.launcher_mode = "foxpro_library_definition";
        plan.launcher_fallback = "library_binary_generation_pending";
    } else if (plan.output_kind == BuildOutputKind::app) {
        plan.launcher_mode = "foxpro_application_archive_contract";
        plan.launcher_fallback = "foxpro_app_binary_generation_pending";
    } else if (plan.output_kind == BuildOutputKind::fxp) {
        plan.launcher_mode = "foxpro_tokenized_contract";
        plan.launcher_fallback = "foxpro_fxp_binary_generation_pending";
    } else {
        plan.launcher_mode = plan.emit_dotnet_launcher ? "dotnet_launcher" : "native_runtime_host";
        plan.launcher_fallback =
            (plan.requested_dotnet_launcher && !plan.emit_dotnet_launcher)
                ? "dotnet_output_unavailable"
                : "none";
    }

    if (!workspace.available) {
        plan.warnings.push_back(runtime_text("Runtime.Package.Warning.ProjectWorkspaceUnavailable"));
        return plan;
    }

    const std::filesystem::path root(output_root);
    const std::filesystem::path package_root = root / sanitize_file_name(plan.project_title);
    const std::filesystem::path content_root = package_root / "content";
    plan.package_root = package_root.string();
    plan.content_root = content_root.string();
    plan.manifest_path = (package_root / "app.cfmanifest").string();
    plan.debug_manifest_path = (package_root / "app.cfdebug").string();
    plan.launcher_project_path = (package_root / "launcher" / "Copperfin.GeneratedLauncher.csproj").string();
    plan.launcher_source_path = (package_root / "launcher" / "Program.cs").string();
    const std::filesystem::path output_file_name(resolve_output_file_name(workspace, plan.project_title));
    plan.ast_manifest_path = (package_root / (output_file_name.string() + ".ast.json")).string();
    plan.ir_manifest_path = (package_root / (output_file_name.string() + ".ir.json")).string();
    plan.transpiled_csharp_path = (package_root / (output_file_name.string() + ".transpiled.cs")).string();
    std::filesystem::path module_definition_file_name = output_file_name;
    module_definition_file_name.replace_extension(".def");
    plan.launcher_output_path = (package_root / output_file_name).string();
    plan.module_definition_path = (package_root / module_definition_file_name).string();
    if (is_library_output_kind(plan.output_kind)) {
        const std::filesystem::path wrapper_root = package_root / "wrapper";
        const std::string output_stem = output_file_name.stem().string();
        plan.native_wrapper_source_path = (wrapper_root / (output_stem + "_wrapper.cpp")).string();
        plan.native_wrapper_cmake_path = (wrapper_root / "CMakeLists.txt").string();
        plan.native_wrapper_build_script_path = (wrapper_root / "build_wrapper.sh").string();
        plan.native_wrapper_build_powershell_path = (wrapper_root / "build_wrapper.ps1").string();
    }
    if (plan.output_kind == BuildOutputKind::dll || plan.output_kind == BuildOutputKind::ocx) {
        std::filesystem::path library_api_manifest_file_name = output_file_name;
        library_api_manifest_file_name += ".api";
        plan.library_api_manifest_path = (package_root / library_api_manifest_file_name).string();
    }
    if (plan.output_kind == BuildOutputKind::fll) {
        std::filesystem::path fll_api_manifest_file_name = output_file_name;
        fll_api_manifest_file_name += ".api";
        plan.fll_api_manifest_path = (package_root / fll_api_manifest_file_name).string();
    }
    if (plan.output_kind == BuildOutputKind::fxp) {
        std::filesystem::path fxp_token_manifest_file_name = output_file_name;
        fxp_token_manifest_file_name += ".tokens";
        plan.fxp_token_manifest_path = (package_root / fxp_token_manifest_file_name).string();
    }
    if (plan.output_kind == BuildOutputKind::app) {
        std::filesystem::path app_archive_manifest_file_name = output_file_name;
        app_archive_manifest_file_name += ".contents";
        plan.app_archive_manifest_path = (package_root / app_archive_manifest_file_name).string();
    }
    plan.runtime_host_destination_path = (package_root / "copperfin_runtime_host.exe").string();
    plan.working_directory = content_root.lexically_normal().string();
    plan.startup_item = workspace.build_plan.startup_item;
    plan.security_role = resolve_security_role(enable_security);
    if (enable_security && !is_recognized_security_role(security_profile, plan.security_role)) {
        const std::string requested_role = plan.security_role;
        plan.security_role = "developer";
        if (!requested_role.empty()) {
            plan.warnings.push_back(runtime_text(
                "Runtime.Package.Warning.UnknownSecurityRoleRequested",
                {
                    {"requestedRole", requested_role},
                    {"defaultRole", plan.security_role}
                }));
        }
    }
    plan.audit_log_path = (package_root / "security_audit.log").string();
    const std::string source_working_directory = resolve_working_directory(document, workspace);

    for (const auto& entry : workspace.entries) {
        RuntimePackageAsset asset;
        asset.record_index = entry.record_index;
        asset.relative_path = relative_asset_path(entry);
        asset.source_path = resolve_project_item_source(document, entry);
        asset.staged_path = (content_root / asset.relative_path).lexically_normal().string();
        asset.type_title = entry.type_title;
        asset.excluded = entry.excluded;
        asset.exists = !asset.source_path.empty() && std::filesystem::exists(asset.source_path);
        if (entry.record_index == workspace.build_plan.startup_record_index) {
            asset.required_for_runtime = true;
            plan.startup_source_path = asset.staged_path;
            plan.debug_plan.startup_source_path = asset.source_path;
        }
        if (!asset.exists && !entry.excluded && entry.group_id != "project") {
            plan.warnings.push_back(runtime_text(
                "Runtime.Package.Warning.MissingProjectAsset",
                {{"path", asset.source_path}}));
        }
        plan.assets.push_back(std::move(asset));
    }

    if (plan.startup_source_path.empty()) {
        plan.warnings.push_back(runtime_text("Runtime.Package.Warning.StartupSourceUnresolved"));
    }
    if (plan.debug_plan.startup_source_path.empty()) {
        plan.warnings.push_back(runtime_text("Runtime.Package.Warning.DebugStartupSourceUnresolved"));
    }

    plan.debug_plan.manifest_path = plan.debug_manifest_path;
    plan.debug_plan.startup_item = plan.startup_item;
    plan.debug_plan.working_directory = source_working_directory;
    plan.debug_plan.source_roots = unique_non_empty_paths_preserve_order({
        source_working_directory,
        plan.content_root
    });
    plan.debug_plan.supports_breakpoints =
        is_prg_path(plan.debug_plan.startup_source_path) ||
        is_xasset_path(plan.debug_plan.startup_source_path);
    plan.debug_plan.supports_step_debugging = plan.debug_plan.supports_breakpoints;

    if (enable_security && !security_profile.available) {
        plan.warnings.push_back(runtime_text("Runtime.Package.Warning.SecurityProfileUnavailable"));
    }
    if (emit_dotnet_launcher && !extensibility_profile.dotnet_output.available) {
        plan.warnings.push_back(runtime_text("Runtime.Package.Warning.DotNetOutputProfileUnavailable"));
    }
    if (is_library_output_kind(plan.output_kind)) {
        plan.exported_symbols = collect_library_exported_symbols(plan);
        if (plan.exported_symbols.empty()) {
            plan.warnings.push_back(runtime_text("Runtime.Package.Warning.LibraryExportsUnresolved"));
        }
    } else if (plan.output_kind == BuildOutputKind::fxp) {
        const bool has_prg_asset = std::any_of(plan.assets.begin(), plan.assets.end(), [](const RuntimePackageAsset& asset) {
            return is_prg_path(asset.source_path);
        });
        if (!has_prg_asset) {
            plan.warnings.push_back(runtime_text("Runtime.Package.Warning.FxpSourcesUnresolved"));
        }
    }

    plan.ok = true;
    return plan;
}

std::string build_runtime_manifest_text(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile) {
    std::ostringstream stream;
    stream << "manifest_version=" << kRuntimeManifestVersion << "\n";
    stream << "project_title=" << quote_manifest_value(plan.project_title) << "\n";
    stream << "package_root=" << quote_manifest_value(plan.package_root) << "\n";
    stream << "content_root=" << quote_manifest_value(plan.content_root) << "\n";
    stream << "working_directory=" << quote_manifest_value(plan.working_directory) << "\n";
    stream << "startup_item=" << quote_manifest_value(plan.startup_item) << "\n";
    stream << "startup_source=" << quote_manifest_value(plan.startup_source_path) << "\n";
    stream << "configuration=" << build_configuration_name(plan.configuration) << "\n";
    stream << "output_kind=" << quote_manifest_value(build_output_kind_name(plan.output_kind)) << "\n";
    stream << "primary_output_path=" << quote_manifest_value(plan.launcher_output_path) << "\n";
    stream << "primary_output_materialized=" << (plan.primary_output_materialized ? "true" : "false") << "\n";
    stream << "module_definition_path=" << quote_manifest_value(plan.module_definition_path) << "\n";
    stream << "library_api_manifest_path=" << quote_manifest_value(plan.library_api_manifest_path) << "\n";
    stream << "fll_api_manifest_path=" << quote_manifest_value(plan.fll_api_manifest_path) << "\n";
    stream << "fll_loader_entrypoint="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllLoaderEntrypoint) : std::string()) << "\n";
    stream << "fll_registration_symbol="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllRegistrationSymbol) : std::string()) << "\n";
    stream << "fll_callable_signature="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllCallableSignature) : std::string()) << "\n";
    stream << "fll_default_return_helper="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllDefaultReturnHelper) : std::string()) << "\n";
    stream << "library_callable_convention="
           << quote_manifest_value((plan.output_kind == BuildOutputKind::dll || plan.output_kind == BuildOutputKind::ocx)
                                       ? std::string(kVfpLibraryCallableConvention)
                                       : std::string()) << "\n";
    stream << "fxp_token_manifest_path=" << quote_manifest_value(plan.fxp_token_manifest_path) << "\n";
    stream << "app_archive_manifest_path=" << quote_manifest_value(plan.app_archive_manifest_path) << "\n";
    stream << "security_enabled=" << (plan.security_enabled ? "true" : "false") << "\n";
    stream << "security_role=" << quote_manifest_value(plan.security_role) << "\n";
    stream << "security_mode=" << quote_manifest_value(security_profile.mode) << "\n";
    stream << "audit_log_path=" << quote_manifest_value(plan.audit_log_path) << "\n";
    stream << "runtime_host_sha256=" << quote_manifest_value(plan.runtime_host_sha256) << "\n";
    stream << "license_state=" << quote_manifest_value(plan.license_state) << "\n";
    stream << "license_type=" << quote_manifest_value(plan.license_type) << "\n";
    stream << "license_id=" << quote_manifest_value(plan.license_id) << "\n";
    stream << "license_licensee=" << quote_manifest_value(plan.license_licensee) << "\n";
    stream << "license_seats=" << plan.license_seats << "\n";
    stream << "license_subscription_expires=" << quote_manifest_value(plan.license_subscription_expires) << "\n";
    stream << "license_perpetual_max_major_version=" << plan.license_perpetual_max_major_version << "\n";
    stream << "security_roles=" << security_profile.roles.size() << "\n";
    stream << "launcher_mode=" << quote_manifest_value(plan.launcher_mode) << "\n";
    stream << "launcher_fallback=" << quote_manifest_value(plan.launcher_fallback) << "\n";
    stream << "dotnet_enabled=" << (extensibility_profile.dotnet_output.available ? "true" : "false") << "\n";
    stream << "dotnet_story=" << quote_manifest_value(extensibility_profile.dotnet_output.primary_story) << "\n";
    stream << "dotnet_policy_allowlist=" << extensibility_profile.dotnet_output.policy.allowlist.size() << "\n";
    stream << "dotnet_policy_denylist=" << extensibility_profile.dotnet_output.policy.denylist.size() << "\n";
    stream << "dotnet_parity_matrix_entries=" << extensibility_profile.dotnet_output.parity_matrix.size() << "\n";
    stream << "dotnet_policy_allowlist_items=" << extensibility_profile.dotnet_output.policy.allowlist.size() << "\n";
    for (const auto& capability_id : extensibility_profile.dotnet_output.policy.allowlist) {
        stream << "dotnet_policy_allowlist_item=" << quote_manifest_value(capability_id) << "\n";
    }
    stream << "dotnet_policy_denylist_items=" << extensibility_profile.dotnet_output.policy.denylist.size() << "\n";
    for (const auto& capability_id : extensibility_profile.dotnet_output.policy.denylist) {
        stream << "dotnet_policy_denylist_item=" << quote_manifest_value(capability_id) << "\n";
    }
    stream << "dotnet_parity_matrix_count=" << extensibility_profile.dotnet_output.parity_matrix.size() << "\n";
    for (const auto& capability : extensibility_profile.dotnet_output.parity_matrix) {
        stream << "dotnet_parity_matrix_item="
               << quote_manifest_value(capability.id) << "|"
               << quote_manifest_value(capability.title) << "|"
               << dotnet_parity_tier_name(capability.tier) << "|"
               << quote_manifest_value(capability.rationale) << "|"
               << quote_manifest_value(capability.verification_reference) << "\n";
    }

    stream << "language_integration_count=" << extensibility_profile.languages.size() << "\n";
    for (const auto& language : extensibility_profile.languages) {
        stream << "language_integration="
               << quote_manifest_value(language.id) << "|"
               << quote_manifest_value(language.title) << "|"
               << quote_manifest_value(language.integration_mode) << "|"
               << quote_manifest_value(language.trust_boundary) << "|"
               << quote_manifest_value(language.output_story) << "|"
               << (language.enabled_by_default ? "true" : "false") << "\n";
    }
    stream << "ai_feature_count=" << extensibility_profile.ai_features.size() << "\n";
    for (const auto& feature : extensibility_profile.ai_features) {
        stream << "ai_feature="
               << quote_manifest_value(feature.id) << "|"
               << quote_manifest_value(feature.title) << "|"
               << quote_manifest_value(feature.description) << "|"
               << quote_manifest_value(feature.trust_boundary) << "|"
               << (feature.enabled_by_default ? "true" : "false") << "\n";
    }
    stream << "extensibility_guardrail_count=" << extensibility_profile.guardrails.size() << "\n";
    for (const auto& guardrail : extensibility_profile.guardrails) {
        stream << "extensibility_guardrail=" << quote_manifest_value(guardrail) << "\n";
    }

    const platform::DotNetInteropCallDecision launcher_decision = platform::evaluate_dotnet_interop_call(
        extensibility_profile,
        platform::DotNetInteropCallRequest{
            .capability_id = "task-primitives",
            .estimated_latency_ms = 10U,
            .requires_reflection = false,
            .untrusted_input = false,
            .security_sensitive = false});
    stream << "dotnet_gateway_task_primitives=" << quote_manifest_value(launcher_decision.execution_path + ":" + launcher_decision.reason) << "\n";

    const platform::DotNetInteropCallDecision denied_decision = platform::evaluate_dotnet_interop_call(
        extensibility_profile,
        platform::DotNetInteropCallRequest{
            .capability_id = "unsafe-reflection-load",
            .estimated_latency_ms = 2U,
            .requires_reflection = true,
            .untrusted_input = true,
            .security_sensitive = true});
    stream << "dotnet_gateway_unsafe_reflection=" << quote_manifest_value(denied_decision.execution_path + ":" + denied_decision.reason) << "\n";

    stream << "language_integrations=" << extensibility_profile.languages.size() << "\n";
    stream << "ai_features=" << extensibility_profile.ai_features.size() << "\n";
    append_runtime_feature_flag_manifest_lines(stream, plan, security_profile);

    append_runtime_asset_manifest_lines(stream, plan);

    for (const auto& digest : plan.extension_payload_digests) {
        stream << "extension_payload="
               << quote_manifest_value(digest.path) << "|"
               << quote_manifest_value(digest.sha256) << "\n";
    }

    for (const auto& digest : plan.compiler_contract_digests) {
        stream << "compiler_contract="
               << quote_manifest_value(digest.path) << "|"
               << quote_manifest_value(digest.sha256) << "\n";
    }

    for (const auto& symbol : plan.exported_symbols) {
        stream << "export_symbol=" << quote_manifest_value(symbol) << "\n";
    }

    append_library_function_manifest_lines(stream, plan);

    append_warning_manifest_lines(stream, plan);

    return stream.str();
}

std::string build_debug_manifest_text(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile) {
    std::ostringstream stream;
    stream << "debug_manifest_version=" << kDebugManifestVersion << "\n";
    stream << "project_title=" << quote_manifest_value(plan.project_title) << "\n";
    stream << "project_path=" << quote_manifest_value(plan.project_path) << "\n";
    stream << "package_root=" << quote_manifest_value(plan.package_root) << "\n";
    stream << "content_root=" << quote_manifest_value(plan.content_root) << "\n";
    stream << "ast_manifest_path=" << quote_manifest_value(plan.ast_manifest_path) << "\n";
    stream << "ir_manifest_path=" << quote_manifest_value(plan.ir_manifest_path) << "\n";
    stream << "transpiled_csharp_path=" << quote_manifest_value(plan.transpiled_csharp_path) << "\n";
    stream << "configuration=" << build_configuration_name(plan.configuration) << "\n";
    stream << "security_enabled=" << (plan.security_enabled ? "true" : "false") << "\n";
    stream << "security_role=" << quote_manifest_value(plan.security_role) << "\n";
    stream << "security_mode=" << quote_manifest_value(security_profile.mode) << "\n";
    stream << "audit_log_path=" << quote_manifest_value(plan.audit_log_path) << "\n";
    stream << "runtime_host_sha256=" << quote_manifest_value(plan.runtime_host_sha256) << "\n";
    stream << "license_state=" << quote_manifest_value(plan.license_state) << "\n";
    stream << "license_type=" << quote_manifest_value(plan.license_type) << "\n";
    stream << "license_id=" << quote_manifest_value(plan.license_id) << "\n";
    stream << "license_licensee=" << quote_manifest_value(plan.license_licensee) << "\n";
    stream << "license_seats=" << plan.license_seats << "\n";
    stream << "license_subscription_expires=" << quote_manifest_value(plan.license_subscription_expires) << "\n";
    stream << "license_perpetual_max_major_version=" << plan.license_perpetual_max_major_version << "\n";
    stream << "security_roles=" << security_profile.roles.size() << "\n";
    stream << "startup_item=" << quote_manifest_value(plan.debug_plan.startup_item) << "\n";
    stream << "startup_source=" << quote_manifest_value(plan.debug_plan.startup_source_path) << "\n";
    stream << "working_directory=" << quote_manifest_value(plan.debug_plan.working_directory) << "\n";
    stream << "supports_breakpoints=" << (plan.debug_plan.supports_breakpoints ? "true" : "false") << "\n";
    stream << "supports_step_debugging=" << (plan.debug_plan.supports_step_debugging ? "true" : "false") << "\n";
    stream << "output_kind=" << quote_manifest_value(build_output_kind_name(plan.output_kind)) << "\n";
    stream << "primary_output_path=" << quote_manifest_value(plan.launcher_output_path) << "\n";
    stream << "primary_output_materialized=" << (plan.primary_output_materialized ? "true" : "false") << "\n";
    stream << "module_definition_path=" << quote_manifest_value(plan.module_definition_path) << "\n";
    stream << "native_wrapper_source_path=" << quote_manifest_value(plan.native_wrapper_source_path) << "\n";
    stream << "native_wrapper_cmake_path=" << quote_manifest_value(plan.native_wrapper_cmake_path) << "\n";
    stream << "native_wrapper_build_script_path=" << quote_manifest_value(plan.native_wrapper_build_script_path) << "\n";
    stream << "native_wrapper_build_powershell_path=" << quote_manifest_value(plan.native_wrapper_build_powershell_path) << "\n";
    stream << "library_api_manifest_path=" << quote_manifest_value(plan.library_api_manifest_path) << "\n";
    stream << "fll_api_manifest_path=" << quote_manifest_value(plan.fll_api_manifest_path) << "\n";
    stream << "fll_loader_entrypoint="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllLoaderEntrypoint) : std::string()) << "\n";
    stream << "fll_registration_symbol="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllRegistrationSymbol) : std::string()) << "\n";
    stream << "fll_callable_signature="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllCallableSignature) : std::string()) << "\n";
    stream << "fll_default_return_helper="
           << quote_manifest_value(plan.output_kind == BuildOutputKind::fll ? std::string(kFllDefaultReturnHelper) : std::string()) << "\n";
    stream << "library_callable_convention="
           << quote_manifest_value((plan.output_kind == BuildOutputKind::dll || plan.output_kind == BuildOutputKind::ocx)
                                       ? std::string(kVfpLibraryCallableConvention)
                                       : std::string()) << "\n";
    stream << "launcher_mode=" << quote_manifest_value(plan.launcher_mode) << "\n";
    stream << "launcher_fallback=" << quote_manifest_value(plan.launcher_fallback) << "\n";
    stream << "dotnet_enabled=" << (extensibility_profile.dotnet_output.available ? "true" : "false") << "\n";
    stream << "dotnet_story=" << quote_manifest_value(extensibility_profile.dotnet_output.primary_story) << "\n";
    stream << "dotnet_policy_allowlist=" << extensibility_profile.dotnet_output.policy.allowlist.size() << "\n";
    stream << "dotnet_policy_denylist=" << extensibility_profile.dotnet_output.policy.denylist.size() << "\n";
    stream << "dotnet_parity_matrix_entries=" << extensibility_profile.dotnet_output.parity_matrix.size() << "\n";
    stream << "dotnet_policy_allowlist_items=" << extensibility_profile.dotnet_output.policy.allowlist.size() << "\n";
    for (const auto& capability_id : extensibility_profile.dotnet_output.policy.allowlist) {
        stream << "dotnet_policy_allowlist_item=" << quote_manifest_value(capability_id) << "\n";
    }
    stream << "dotnet_policy_denylist_items=" << extensibility_profile.dotnet_output.policy.denylist.size() << "\n";
    for (const auto& capability_id : extensibility_profile.dotnet_output.policy.denylist) {
        stream << "dotnet_policy_denylist_item=" << quote_manifest_value(capability_id) << "\n";
    }
    stream << "dotnet_parity_matrix_count=" << extensibility_profile.dotnet_output.parity_matrix.size() << "\n";
    for (const auto& capability : extensibility_profile.dotnet_output.parity_matrix) {
        stream << "dotnet_parity_matrix_item="
               << quote_manifest_value(capability.id) << "|"
               << quote_manifest_value(capability.title) << "|"
               << dotnet_parity_tier_name(capability.tier) << "|"
               << quote_manifest_value(capability.rationale) << "|"
               << quote_manifest_value(capability.verification_reference) << "\n";
    }
    const platform::DotNetInteropCallDecision launcher_decision = platform::evaluate_dotnet_interop_call(
        extensibility_profile,
        platform::DotNetInteropCallRequest{
            .capability_id = "task-primitives",
            .estimated_latency_ms = 10U,
            .requires_reflection = false,
            .untrusted_input = false,
            .security_sensitive = false});
    stream << "dotnet_gateway_task_primitives=" << quote_manifest_value(launcher_decision.execution_path + ":" + launcher_decision.reason) << "\n";

    const platform::DotNetInteropCallDecision denied_decision = platform::evaluate_dotnet_interop_call(
        extensibility_profile,
        platform::DotNetInteropCallRequest{
            .capability_id = "unsafe-reflection-load",
            .estimated_latency_ms = 2U,
            .requires_reflection = true,
            .untrusted_input = true,
            .security_sensitive = true});
    stream << "dotnet_gateway_unsafe_reflection=" << quote_manifest_value(denied_decision.execution_path + ":" + denied_decision.reason) << "\n";
    stream << "language_integration_count=" << extensibility_profile.languages.size() << "\n";
    for (const auto& language : extensibility_profile.languages) {
        stream << "language_integration="
               << quote_manifest_value(language.id) << "|"
               << quote_manifest_value(language.title) << "|"
               << quote_manifest_value(language.integration_mode) << "|"
               << quote_manifest_value(language.trust_boundary) << "|"
               << quote_manifest_value(language.output_story) << "|"
               << (language.enabled_by_default ? "true" : "false") << "\n";
    }
    stream << "ai_feature_count=" << extensibility_profile.ai_features.size() << "\n";
    for (const auto& feature : extensibility_profile.ai_features) {
        stream << "ai_feature="
               << quote_manifest_value(feature.id) << "|"
               << quote_manifest_value(feature.title) << "|"
               << quote_manifest_value(feature.description) << "|"
               << quote_manifest_value(feature.trust_boundary) << "|"
               << (feature.enabled_by_default ? "true" : "false") << "\n";
    }
    stream << "extensibility_guardrail_count=" << extensibility_profile.guardrails.size() << "\n";
    for (const auto& guardrail : extensibility_profile.guardrails) {
        stream << "extensibility_guardrail=" << quote_manifest_value(guardrail) << "\n";
    }
    stream << "language_integrations=" << extensibility_profile.languages.size() << "\n";
    stream << "ai_features=" << extensibility_profile.ai_features.size() << "\n";
    stream << "source_roots=" << quote_manifest_value(join_strings(plan.debug_plan.source_roots)) << "\n";
    append_runtime_feature_flag_manifest_lines(stream, plan, security_profile);
    for (const auto& digest : plan.compiler_contract_digests) {
        stream << "compiler_contract="
               << quote_manifest_value(digest.path) << "|"
               << quote_manifest_value(digest.sha256) << "\n";
    }
    for (const auto& digest : plan.extension_payload_digests) {
        stream << "extension_payload="
               << quote_manifest_value(digest.path) << "|"
               << quote_manifest_value(digest.sha256) << "\n";
    }
    for (const auto& symbol : plan.exported_symbols) {
        stream << "export_symbol=" << quote_manifest_value(symbol) << "\n";
    }
    append_runtime_asset_manifest_lines(stream, plan);
    append_warning_manifest_lines(stream, plan);
    append_library_function_manifest_lines(stream, plan);
    return stream.str();
}

RuntimeMaterializeResult materialize_runtime_package(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile,
    const std::string& runtime_host_source_path) {
    if (!plan.ok) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.PlanInvalid")};
    }

    std::error_code directory_error;
    std::filesystem::create_directories(plan.package_root, directory_error);
    if (directory_error) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.CreatePackageRootFailed")};
    }
    std::filesystem::create_directories(plan.content_root, directory_error);
    if (directory_error) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.CreateContentRootFailed")};
    }
    if (plan.emit_dotnet_launcher) {
        std::filesystem::create_directories(std::filesystem::path(plan.launcher_project_path).parent_path(), directory_error);
        if (directory_error) {
            return {.ok = false, .error = runtime_text("Runtime.Package.Error.CreateLauncherDirectoryFailed")};
        }
    }

    RuntimePackagePlan materialized_plan = plan;
    std::string error;
    if (is_native_host_output_kind(plan.output_kind) &&
        !validate_runtime_host_source_path(plan, runtime_host_source_path, error)) {
        return {.ok = false, .error = error};
    }
    for (auto& asset : materialized_plan.assets) {
        if (!should_stage_asset(asset)) {
            continue;
        }

        const std::filesystem::path destination = std::filesystem::path(plan.content_root) / asset.relative_path;
        if (!copy_file_if_exists(asset.source_path, destination, error)) {
            materialized_plan.warnings.push_back(error);
            continue;
        }
        copy_companion_files_if_present(asset, materialized_plan.warnings);
        asset.copied = true;

        const auto digest = security::sha256_hex_for_file(destination.string());
        if (!digest.ok) {
            return {.ok = false, .error = digest.error};
        }
        asset.sha256 = digest.hex_digest;

        if (is_extension_payload_path(destination)) {
            materialized_plan.extension_payload_digests.push_back({
                .path = destination.string(),
                .sha256 = digest.hex_digest
            });
        }
    }

    if (is_library_output_kind(plan.output_kind)) {
        std::filesystem::create_directories(std::filesystem::path(plan.native_wrapper_source_path).parent_path(), directory_error);
        if (directory_error) {
            return {.ok = false, .error = runtime_text("Runtime.Package.Error.CreateNativeWrapperDirectoryFailed")};
        }
        if (!write_text_file(plan.module_definition_path, build_module_definition_source(materialized_plan), error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.module_definition_path, error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(plan.native_wrapper_source_path, build_native_wrapper_source(materialized_plan), error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.native_wrapper_source_path, error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(plan.native_wrapper_cmake_path, build_native_wrapper_cmake_source(materialized_plan), error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.native_wrapper_cmake_path, error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(plan.native_wrapper_build_script_path, build_native_wrapper_shell_script_source(), error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.native_wrapper_build_script_path, error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(plan.native_wrapper_build_powershell_path, build_native_wrapper_powershell_script_source(), error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.native_wrapper_build_powershell_path, error)) {
            return {.ok = false, .error = error};
        }
        if (plan.output_kind == BuildOutputKind::dll || plan.output_kind == BuildOutputKind::ocx) {
            if (!write_text_file(plan.library_api_manifest_path, build_library_api_manifest_source(materialized_plan), error)) {
                return {.ok = false, .error = error};
            }
            if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.library_api_manifest_path, error)) {
                return {.ok = false, .error = error};
            }
        }
        if (plan.output_kind == BuildOutputKind::fll) {
            if (!write_text_file(plan.fll_api_manifest_path, build_fll_api_manifest_source(materialized_plan), error)) {
                return {.ok = false, .error = error};
            }
            if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.fll_api_manifest_path, error)) {
                return {.ok = false, .error = error};
            }
        }
    } else if (plan.output_kind == BuildOutputKind::fxp) {
        const std::string fxp_token_manifest = build_fxp_token_manifest_source(materialized_plan);
        if (!write_text_file(plan.fxp_token_manifest_path, fxp_token_manifest, error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.fxp_token_manifest_path, error)) {
            return {.ok = false, .error = error};
        }
        if (!write_fxp_primary_output_contract(materialized_plan, fxp_token_manifest, error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.extension_payload_digests, plan.launcher_output_path, error)) {
            return {.ok = false, .error = error};
        }
        materialized_plan.primary_output_materialized = true;
    } else if (plan.output_kind == BuildOutputKind::app) {
        if (!write_text_file(plan.app_archive_manifest_path, build_app_archive_manifest_source(materialized_plan), error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.app_archive_manifest_path, error)) {
            return {.ok = false, .error = error};
        }
        if (!write_app_archive_primary_output(materialized_plan, error)) {
            return {.ok = false, .error = error};
        }
        if (!append_runtime_artifact_digest(materialized_plan.extension_payload_digests, plan.launcher_output_path, error)) {
            return {.ok = false, .error = error};
        }
        materialized_plan.primary_output_materialized = true;
    } else {
        if (!copy_file_if_exists(runtime_host_source_path, plan.runtime_host_destination_path, error)) {
            return {.ok = false, .error = error};
        }

        const auto runtime_host_digest = security::sha256_hex_for_file(plan.runtime_host_destination_path);
        if (!runtime_host_digest.ok) {
            return {.ok = false, .error = runtime_host_digest.error};
        }
        materialized_plan.runtime_host_sha256 = runtime_host_digest.hex_digest;
        materialized_plan.extension_payload_digests.push_back({
            .path = plan.runtime_host_destination_path,
            .sha256 = runtime_host_digest.hex_digest
        });

        if (!plan.emit_dotnet_launcher) {
            if (!copy_file_if_exists(plan.runtime_host_destination_path, plan.launcher_output_path, error)) {
                return {.ok = false, .error = error};
            }

            const auto native_entrypoint_digest = security::sha256_hex_for_file(plan.launcher_output_path);
            if (!native_entrypoint_digest.ok) {
                return {.ok = false, .error = native_entrypoint_digest.error};
            }
            materialized_plan.extension_payload_digests.push_back({
                .path = plan.launcher_output_path,
                .sha256 = native_entrypoint_digest.hex_digest
            });
            materialized_plan.primary_output_materialized = true;
        }
    }

    if (plan.emit_dotnet_launcher) {
        if (!write_text_file(plan.launcher_project_path, build_launcher_project_source(plan), error)) {
            return {.ok = false, .error = error};
        }
        if (!write_text_file(plan.launcher_source_path, build_launcher_program_source(plan), error)) {
            return {.ok = false, .error = error};
        }
    }

    if (!write_text_file(plan.ast_manifest_path, build_ast_manifest_source(materialized_plan), error)) {
        return {.ok = false, .error = error};
    }
    if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.ast_manifest_path, error)) {
        return {.ok = false, .error = error};
    }
    if (!write_text_file(plan.ir_manifest_path, build_ir_manifest_source(materialized_plan), error)) {
        return {.ok = false, .error = error};
    }
    if (!append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.ir_manifest_path, error)) {
        return {.ok = false, .error = error};
    }
    if (plan.requested_dotnet_launcher &&
        !write_text_file(plan.transpiled_csharp_path, build_csharp_transpilation_source(materialized_plan), error)) {
        return {.ok = false, .error = error};
    }
    if (plan.requested_dotnet_launcher &&
        !append_runtime_artifact_digest(materialized_plan.compiler_contract_digests, plan.transpiled_csharp_path, error)) {
        return {.ok = false, .error = error};
    }
    if (!write_text_file(plan.manifest_path, build_runtime_manifest_text(materialized_plan, security_profile, extensibility_profile), error)) {
        return {.ok = false, .error = error};
    }
    if (!write_text_file(plan.debug_manifest_path, build_debug_manifest_text(materialized_plan, security_profile, extensibility_profile), error)) {
        return {.ok = false, .error = error};
    }

    return {.ok = true, .plan = std::move(materialized_plan), .error = {}};
}

RuntimeBuildResult finalize_runtime_package_primary_output(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile) {
    if (!plan.ok) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.PlanInvalid")};
    }
    if (!std::filesystem::exists(plan.launcher_output_path)) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.PrimaryOutputMissing")};
    }

    RuntimePackagePlan finalized_plan = plan;
    finalized_plan.primary_output_materialized = true;
    std::erase_if(
        finalized_plan.extension_payload_digests,
        [&](const RuntimeArtifactDigest& digest) {
            return digest.path == plan.launcher_output_path;
        });

    std::string error;
    if (!append_runtime_artifact_digest(finalized_plan.extension_payload_digests, plan.launcher_output_path, error)) {
        return {.ok = false, .error = error};
    }
    if (!write_text_file(plan.manifest_path, build_runtime_manifest_text(finalized_plan, security_profile, extensibility_profile), error)) {
        return {.ok = false, .error = error};
    }
    if (!write_text_file(plan.debug_manifest_path, build_debug_manifest_text(finalized_plan, security_profile, extensibility_profile), error)) {
        return {.ok = false, .error = error};
    }

    return {.ok = true, .plan = std::move(finalized_plan), .error = {}};
}

RuntimeBuildResult build_runtime_package_primary_output(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile) {
    if (!plan.ok) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.PlanInvalid")};
    }
    if (!is_library_output_kind(plan.output_kind)) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.PrimaryOutputRequiresLibraryOutput")};
    }
    if (!std::filesystem::exists(plan.native_wrapper_cmake_path)) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.NativeWrapperCMakeMissing")};
    }

    RuntimePackagePlan built_plan = plan;
    std::string error;
    const std::filesystem::path source_root = std::filesystem::path(plan.native_wrapper_cmake_path).parent_path();
    const std::filesystem::path build_root = source_root / "cmake_pipeline_build";
    const std::filesystem::path configure_log_path = build_root / "cmake-configure.log";
    const std::filesystem::path build_log_path = build_root / "cmake-build.log";
    std::error_code ignored;
    std::filesystem::remove_all(build_root, ignored);
    std::filesystem::remove(plan.launcher_output_path, ignored);
    std::filesystem::create_directories(build_root, ignored);
    if (ignored) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.CreateNativeWrapperBuildDirectoryFailed")};
    }

    const std::string configure_command =
        "cmake -S \"" + source_root.string() + "\" -B \"" + build_root.string() + "\" > \"" +
        configure_log_path.string() + "\" 2>&1";
    if (std::system(configure_command.c_str()) != 0) {
        error = runtime_text("Runtime.Package.Error.NativeWrapperPrimaryOutputConfigureFailed");
        if (std::filesystem::exists(configure_log_path)) {
            error += ":\n" + read_text_file(configure_log_path);
        }
        return {.ok = false, .error = error};
    }

    const std::string build_command =
        "cmake --build \"" + build_root.string() + "\" > \"" + build_log_path.string() + "\" 2>&1";
    if (std::system(build_command.c_str()) != 0) {
        error = runtime_text("Runtime.Package.Error.NativeWrapperPrimaryOutputBuildFailed");
        if (std::filesystem::exists(build_log_path)) {
            error += ":\n" + read_text_file(build_log_path);
        }
        return {.ok = false, .error = error};
    }

    if (!std::filesystem::exists(plan.launcher_output_path)) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.NativeWrapperPrimaryOutputMissing")};
    }

    return finalize_runtime_package_primary_output(
        built_plan,
        security_profile,
        extensibility_profile);
}

}  // namespace copperfin::runtime
