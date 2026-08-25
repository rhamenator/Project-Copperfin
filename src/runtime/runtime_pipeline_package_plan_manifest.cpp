// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "runtime_pipeline_support.h"

namespace copperfin::runtime {

namespace {

constexpr int kRuntimeManifestVersion = 3;
constexpr int kDebugManifestVersion = 3;

}  // namespace

RuntimePackagePlan create_runtime_package_plan(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectWorkspace& workspace,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile,
    const std::string& output_root,
    BuildConfiguration configuration,
    bool enable_security,
    bool emit_dotnet_launcher) {
    return create_runtime_package_plan(
        document,
        workspace,
        security_profile,
        extensibility_profile,
        output_root,
        configuration,
        enable_security,
        emit_dotnet_launcher,
        {});
}

RuntimePackagePlan create_runtime_package_plan(
    const studio::StudioDocumentModel& document,
    const studio::StudioProjectWorkspace& workspace,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile,
    const std::string& output_root,
    BuildConfiguration configuration,
    bool enable_security,
    bool emit_dotnet_launcher,
    const std::vector<std::string>& external_include_roots) {
    RuntimePackagePlan plan;
    plan.project_path = document.path;
    plan.project_title = workspace.project_title.empty()
        ? copperfin::platform::path_to_utf8_string(
              copperfin::platform::path_from_utf8_string(document.path).stem())
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

    const std::filesystem::path root = copperfin::platform::path_from_utf8_string(output_root);
    const std::filesystem::path package_root = root / sanitize_file_name(plan.project_title);
    const std::filesystem::path content_root = package_root / "content";
    plan.package_root = copperfin::platform::path_to_utf8_string(package_root);
    plan.content_root = copperfin::platform::path_to_utf8_string(content_root);
    plan.manifest_path = copperfin::platform::path_to_utf8_string(package_root / "app.cfmanifest");
    plan.debug_manifest_path = copperfin::platform::path_to_utf8_string(package_root / "app.cfdebug");
    plan.launcher_project_path = copperfin::platform::path_to_utf8_string(package_root / "launcher" / "Copperfin.GeneratedLauncher.csproj");
    plan.launcher_source_path = copperfin::platform::path_to_utf8_string(package_root / "launcher" / "Program.cs");
    const std::filesystem::path output_file_name =
        copperfin::platform::path_from_utf8_string(
            resolve_output_file_name(workspace, plan.project_title));
    std::filesystem::path ast_manifest_file_name = output_file_name;
    ast_manifest_file_name += ".ast.json";
    std::filesystem::path ir_manifest_file_name = output_file_name;
    ir_manifest_file_name += ".ir.json";
    std::filesystem::path transpiled_csharp_file_name = output_file_name;
    transpiled_csharp_file_name += ".transpiled.cs";
    plan.ast_manifest_path = copperfin::platform::path_to_utf8_string(package_root / ast_manifest_file_name);
    plan.ir_manifest_path = copperfin::platform::path_to_utf8_string(package_root / ir_manifest_file_name);
    plan.transpiled_csharp_path = copperfin::platform::path_to_utf8_string(package_root / transpiled_csharp_file_name);
    std::filesystem::path module_definition_file_name = output_file_name;
    module_definition_file_name.replace_extension(".def");
    plan.launcher_output_path = copperfin::platform::path_to_utf8_string(package_root / output_file_name);
    plan.module_definition_path = copperfin::platform::path_to_utf8_string(package_root / module_definition_file_name);
    if (is_library_output_kind(plan.output_kind)) {
        const std::filesystem::path wrapper_root = package_root / "wrapper";
        const std::string output_stem = copperfin::platform::path_to_utf8_string(output_file_name.stem());
        plan.native_wrapper_source_path = copperfin::platform::path_to_utf8_string(wrapper_root / (output_stem + "_wrapper.cpp"));
        plan.native_wrapper_cmake_path = copperfin::platform::path_to_utf8_string(wrapper_root / "CMakeLists.txt");
        plan.native_wrapper_build_script_path = copperfin::platform::path_to_utf8_string(wrapper_root / "build_wrapper.sh");
        plan.native_wrapper_build_powershell_path = copperfin::platform::path_to_utf8_string(wrapper_root / "build_wrapper.ps1");
    }
    if (plan.output_kind == BuildOutputKind::dll || plan.output_kind == BuildOutputKind::ocx) {
        std::filesystem::path library_api_manifest_file_name = output_file_name;
        library_api_manifest_file_name += ".api";
        plan.library_api_manifest_path = copperfin::platform::path_to_utf8_string(package_root / library_api_manifest_file_name);
    }
    if (plan.output_kind == BuildOutputKind::fll) {
        std::filesystem::path fll_api_manifest_file_name = output_file_name;
        fll_api_manifest_file_name += ".api";
        plan.fll_api_manifest_path = copperfin::platform::path_to_utf8_string(package_root / fll_api_manifest_file_name);
    }
    if (plan.output_kind == BuildOutputKind::fxp) {
        std::filesystem::path fxp_token_manifest_file_name = output_file_name;
        fxp_token_manifest_file_name += ".tokens";
        plan.fxp_token_manifest_path = copperfin::platform::path_to_utf8_string(package_root / fxp_token_manifest_file_name);
    }
    if (plan.output_kind == BuildOutputKind::app) {
        std::filesystem::path app_archive_manifest_file_name = output_file_name;
        app_archive_manifest_file_name += ".contents";
        plan.app_archive_manifest_path = copperfin::platform::path_to_utf8_string(package_root / app_archive_manifest_file_name);
    }
    plan.runtime_host_destination_path = copperfin::platform::path_to_utf8_string(package_root / runtime_host_file_name());
    std::string output_name_error;
    if (!validate_public_output_artifact_name(plan, output_name_error)) {
        plan.warnings.push_back(std::move(output_name_error));
        return plan;
    }
    plan.working_directory = copperfin::platform::path_to_utf8_string(content_root.lexically_normal());
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
    plan.audit_log_path = copperfin::platform::path_to_utf8_string(package_root / "security_audit.log");
    const std::string source_working_directory = resolve_working_directory(document, workspace);

    for (const auto& entry : workspace.entries) {
        RuntimePackageAsset asset;
        asset.record_index = entry.record_index;
        asset.required_for_runtime =
            entry.record_index == workspace.build_plan.startup_record_index;
        asset.source_path = resolve_project_item_source(
            document,
            entry,
            asset.required_for_runtime,
            asset.source_resolution_error);
        asset.exists =
            asset.source_resolution_error.empty() &&
            !asset.source_path.empty() &&
            source_path_exists_on_host(asset.source_path);
        asset.relative_path = relative_asset_path(
            document,
            entry,
            asset.source_path,
            asset.required_for_runtime && asset.exists);
        asset.staged_path = copperfin::platform::path_to_utf8_string(
            (content_root / copperfin::platform::path_from_utf8_string(asset.relative_path)).lexically_normal());
        asset.type_title = entry.type_title;
        asset.excluded = entry.excluded;
        if (asset.required_for_runtime) {
            plan.startup_source_path = asset.staged_path;
            plan.debug_plan.startup_source_path = asset.source_path;
        }
        asset.package_writable =
            !asset.required_for_runtime && is_writable_package_data_path(asset.source_path);
        if (!asset.exists && !entry.excluded && entry.group_id != "project") {
            if (!asset.source_resolution_error.empty()) {
                plan.warnings.push_back(asset.source_resolution_error);
            } else {
                plan.warnings.push_back(runtime_text(
                    "Runtime.Package.Warning.MissingProjectAsset",
                    {{"path", asset.source_path}}));
            }
        }
        plan.assets.push_back(std::move(asset));
    }

    std::unordered_set<std::string> known_asset_sources;
    std::vector<std::filesystem::path> dependency_scan_sources;
    const auto is_class_library_source = [](const std::string& source) {
        return lowercase_copy(trim_copy(
            copperfin::platform::path_to_utf8_string(
                copperfin::platform::path_from_utf8_string(source).extension()))) == ".vcx";
    };
    for (const auto& asset : plan.assets) {
        known_asset_sources.insert(lowercase_copy(asset.source_path));
        if (asset.exists && (is_prg_path(asset.source_path) || is_class_library_source(asset.source_path))) {
            dependency_scan_sources.push_back(
                copperfin::platform::path_from_utf8_string(asset.source_path));
        }
    }
    const std::filesystem::path project_root = normalize_existing_path_spelling(
        copperfin::platform::path_from_utf8_string(document.path).parent_path());
    std::vector<std::filesystem::path> admitted_external_roots;
    for (const auto& root_value : external_include_roots) {
        if (root_value.empty()) {
            continue;
        }
        const std::filesystem::path external_root = normalize_existing_path_spelling(
            copperfin::platform::path_from_utf8_string(root_value).lexically_normal());
        std::error_code root_error;
        if (!std::filesystem::is_directory(external_root, root_error) || root_error) {
            plan.warnings.push_back(runtime_text(
                "Runtime.Package.Warning.ExternalIncludeRootUnavailable",
                {{"path", root_value}}));
            continue;
        }
        admitted_external_roots.push_back(external_root);
    }
    const auto external_relative_path = [&](const std::filesystem::path& source_path)
        -> std::optional<std::filesystem::path> {
        const std::filesystem::path normalized_source =
            normalize_existing_path_spelling(source_path);
        for (const auto& admitted_root : admitted_external_roots) {
            const std::filesystem::path normalized_root =
                normalize_existing_path_spelling(admitted_root);
            const auto relative = normalized_source.lexically_relative(normalized_root);
            bool contained = !relative.empty() && !relative.is_absolute();
            for (const auto& component : relative) {
                if (component == "..") {
                    contained = false;
                    break;
                }
            }
            if (contained) {
                const std::string root_name = sanitize_file_name(
                    copperfin::platform::path_to_utf8_string(admitted_root.filename()));
                return std::filesystem::path("external") / root_name / relative;
            }
        }
        return std::nullopt;
    };
    const auto enqueue_dependency = [&](const std::filesystem::path& candidate,
                                        const char* type_title) {
        const std::filesystem::path source_path = normalize_existing_path_spelling(
            candidate.lexically_normal());
        std::filesystem::path relative = source_path.lexically_relative(project_root);
        bool escapes_project = relative.empty() || relative.is_absolute();
        for (const auto& component : relative) {
            if (component == "..") {
                escapes_project = true;
                break;
            }
        }
        if (escapes_project) {
            const auto external_relative = external_relative_path(source_path);
            if (!external_relative.has_value()) {
                return;
            }
            relative = *external_relative;
        }

        const std::string source = copperfin::platform::path_to_utf8_string(source_path);
        if (source.empty()) {
            return;
        }

        const std::string source_identity = lowercase_copy(source);
        if (!known_asset_sources.insert(source_identity).second) {
            // A project may mark a compile-time header excluded even though a
            // staged PRG or generated VCX bridge requires it. Dependency
            // discovery is an explicit admission for that contained file.
            for (auto& existing : plan.assets) {
                if (lowercase_copy(existing.source_path) != source_identity || !existing.exists) {
                    continue;
                }
                if (existing.excluded) {
                    existing.excluded = false;
                    existing.type_title = type_title;
                }
                return;
            }
            return;
        }

        RuntimePackageAsset dependency;
        dependency.record_index = plan.assets.size();
        dependency.source_path = source;
        dependency.relative_path = copperfin::platform::path_to_utf8_string(relative);
        std::replace(dependency.relative_path.begin(), dependency.relative_path.end(), '\\', '/');
        if (dependency.relative_path.empty()) {
            return;
        }
        dependency.staged_path = copperfin::platform::path_to_utf8_string(
            (content_root / copperfin::platform::path_from_utf8_string(dependency.relative_path)).lexically_normal());
        dependency.type_title = type_title;
        dependency.exists = true;
        plan.assets.push_back(std::move(dependency));
        if (is_prg_path(source) || is_class_library_source(source)) {
            dependency_scan_sources.push_back(source_path);
        }
    };

    for (std::size_t source_index = 0U;
         source_index < dependency_scan_sources.size();
         ++source_index) {
        const std::filesystem::path source = dependency_scan_sources[source_index];
        if (is_prg_path(copperfin::platform::path_to_utf8_string(source))) {
            for (const auto& include_source : discover_prg_include_source_paths(
                     source,
                     admitted_external_roots,
                     plan.warnings)) {
                enqueue_dependency(include_source, "PRG Include");
            }
            for (const auto& library_source : discover_prg_literal_library_source_paths(source)) {
                enqueue_dependency(library_source, "PRG Runtime Dependency");
            }
            for (const auto& do_source : discover_prg_literal_do_source_paths(source)) {
                enqueue_dependency(do_source, "PRG DO Dependency");
            }
        } else if (is_class_library_source(copperfin::platform::path_to_utf8_string(source))) {
            for (const auto& library_source : discover_vcx_literal_library_source_paths(source)) {
                enqueue_dependency(library_source, "xAsset Runtime Dependency");
            }
        }
    }

    std::unordered_map<std::string, std::string> staged_asset_paths;
    const auto duplicate_staged_path = [&](const std::filesystem::path& relative_path)
        -> std::optional<std::string> {
        const std::string relative = copperfin::platform::path_to_utf8_string(
            relative_path.lexically_normal());
        const std::string identity = canonical_casefolded_path_identity(relative_path);
        const auto [existing, inserted] = staged_asset_paths.emplace(identity, relative);
        return inserted ? std::nullopt : std::optional<std::string>(existing->second);
    };
    const auto reject_duplicate_staged_path = [&](const std::string& path) {
        plan.ok = false;
        plan.warnings.push_back(runtime_text(
            "Runtime.Package.Error.DuplicateStagedAssetPath",
            {{"path", path}}));
        plan.planning_warning_count = plan.warnings.size();
        plan.planning_warnings_captured = true;
    };
    for (const auto& asset : plan.assets) {
        if (!should_stage_asset(asset)) {
            continue;
        }
        if (const auto duplicate = duplicate_staged_path(
                copperfin::platform::path_from_utf8_string(asset.relative_path));
            duplicate.has_value()) {
            reject_duplicate_staged_path(*duplicate);
            return plan;
        }

        const std::filesystem::path staged_relative =
            copperfin::platform::path_from_utf8_string(asset.relative_path);
        for (const auto& companion_source : infer_companion_source_paths(
                 copperfin::platform::path_from_utf8_string(asset.source_path))) {
            bool ambiguous = false;
            const auto resolved_companion_source = resolve_existing_path_casefold(
                companion_source, ambiguous);
            if (!resolved_companion_source.has_value()) {
                continue;
            }
            if (const auto duplicate = duplicate_staged_path(
                    staged_relative.parent_path() / resolved_companion_source->filename());
                duplicate.has_value()) {
                reject_duplicate_staged_path(*duplicate);
                return plan;
            }
        }
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
    const auto startup_asset = std::find_if(
        plan.assets.begin(),
        plan.assets.end(),
        [](const RuntimePackageAsset& asset) {
            return asset.required_for_runtime;
        });
    plan.debug_plan.supports_breakpoints =
        startup_asset != plan.assets.end() &&
        startup_asset->exists &&
        (is_prg_path(plan.debug_plan.startup_source_path) ||
         is_xasset_path(plan.debug_plan.startup_source_path));
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

    plan.planning_warning_count = plan.warnings.size();
    plan.planning_warnings_captured = true;
    plan.ok = true;
    return plan;
}

std::string build_runtime_manifest_text(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << "manifest_version=" << kRuntimeManifestVersion << "\n";
    stream << "manifest_value_encoding=backslash-v1\n";
    stream << "project_title=" << quote_manifest_value(plan.project_title) << "\n";
    stream << "package_root=" << quote_manifest_value(plan.package_root) << "\n";
    stream << "content_root=" << quote_manifest_value(plan.content_root) << "\n";
    stream << "working_directory=" << quote_manifest_value(plan.working_directory) << "\n";
    stream << "startup_item=" << quote_manifest_value(plan.startup_item) << "\n";
    stream << "startup_source=" << quote_manifest_value(plan.startup_source_path) << "\n";
    stream << "configuration=" << build_configuration_name(plan.configuration) << "\n";
    stream << "output_kind=" << quote_manifest_value(build_output_kind_name(plan.output_kind)) << "\n";
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
    stream << "security_enabled=" << (plan.security_enabled ? "true" : "false") << "\n";
    stream << "security_role=" << quote_manifest_value(plan.security_role) << "\n";
    stream << "security_mode=" << quote_manifest_value(security_profile.mode) << "\n";
    stream << "audit_log_path=" << quote_manifest_value(plan.audit_log_path) << "\n";
    stream << "runtime_host_sha256=" << quote_manifest_value(plan.runtime_host_sha256) << "\n";
    stream << "dotnet_story=" << quote_manifest_value(extensibility_profile.dotnet_output.primary_story) << "\n";
    stream << "data_policy=package_writable\n";

    append_runtime_asset_manifest_lines(stream, plan);
    append_writable_data_manifest_lines(stream, plan);

    for (const auto& digest : plan.extension_payload_digests) {
        stream << "extension_payload="
               << quote_manifest_value(digest.path) << "|"
               << quote_manifest_value(digest.sha256) << "\n";
    }
    // Provenance only: app.cfmanifest is consumed after these launcher files execute.
    for (const auto& artifact : plan.launcher_artifacts) {
        stream << "launcher_artifact="
               << quote_manifest_value(artifact.package_relative_path) << "|"
               << launcher_artifact_role_name(artifact.role) << "|"
               << quote_manifest_value(artifact.sha256) << "\n";
    }
    append_warning_manifest_lines(stream, plan);

    return stream.str();
}

std::string build_debug_manifest_text(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << "debug_manifest_version=" << kDebugManifestVersion << "\n";
    stream << "manifest_value_encoding=backslash-v1\n";
    stream << "project_title=" << quote_manifest_value(plan.project_title) << "\n";
    stream << "project_path=" << quote_manifest_value(plan.project_path) << "\n";
    stream << "package_root=" << quote_manifest_value(plan.package_root) << "\n";
    stream << "content_root=" << quote_manifest_value(plan.content_root) << "\n";
    stream << "data_policy=package_writable\n";
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
    stream << "fxp_token_manifest_path=" << quote_manifest_value(plan.fxp_token_manifest_path) << "\n";
    stream << "app_archive_manifest_path=" << quote_manifest_value(plan.app_archive_manifest_path) << "\n";
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
            .security_sensitive = false,
            .actor_id = "build-manifest",
            .granted_capabilities = {"task-primitives"},
            .policy_context_verified = true,
            .audit_sink_available = true});
    stream << "dotnet_gateway_task_primitives=" << quote_manifest_value(launcher_decision.execution_path + ":" + launcher_decision.reason) << "\n";

    const platform::DotNetInteropCallDecision denied_decision = platform::evaluate_dotnet_interop_call(
        extensibility_profile,
        platform::DotNetInteropCallRequest{
            .capability_id = "unsafe-reflection-load",
            .estimated_latency_ms = 2U,
            .requires_reflection = true,
            .untrusted_input = true,
            .security_sensitive = true,
            .actor_id = "build-manifest",
            .granted_capabilities = {"unsafe-reflection-load"},
            .policy_context_verified = true,
            .audit_sink_available = true});
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
    for (const auto& artifact : plan.launcher_artifacts) {
        stream << "launcher_artifact="
               << quote_manifest_value(artifact.package_relative_path) << "|"
               << launcher_artifact_role_name(artifact.role) << "|"
               << quote_manifest_value(artifact.sha256) << "\n";
    }
    for (const auto& symbol : plan.exported_symbols) {
        stream << "export_symbol=" << quote_manifest_value(symbol) << "\n";
    }
    append_runtime_asset_manifest_lines(stream, plan);
    append_writable_data_manifest_lines(stream, plan);
    append_warning_manifest_lines(stream, plan);
    append_library_function_manifest_lines(stream, plan, true);
    return stream.str();
}

}  // namespace copperfin::runtime
