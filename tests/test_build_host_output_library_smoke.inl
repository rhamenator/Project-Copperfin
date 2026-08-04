// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

// Kept separate from the driver because this cohesive DLL/FLL scenario is intentionally large.
void run_direct_library_bridge_shell_safety_smoke(
    const std::filesystem::path& package_directory,
    const std::filesystem::path& library_path) {
    namespace fs = std::filesystem;

    const fs::path sentinel_path = fs::current_path() / "copperfin_generated_bridge_shell_sentinel";
    std::error_code ignored;
    fs::remove(sentinel_path, ignored);
    const fs::path injected_package_directory =
        package_directory.parent_path() / "LibraryDemo_$(touch${IFS}copperfin_generated_bridge_shell_sentinel)";
    fs::remove_all(injected_package_directory, ignored);
    std::error_code move_error;
    fs::rename(
        package_directory,
        injected_package_directory,
        move_error);
    expect(!move_error, "native library bridge shell-safety fixture should move the complete package");
    if (move_error) {
        return;
    }

    const fs::path injected_library_path = injected_package_directory / library_path.filename();
#if defined(_WIN32)
#if defined(_M_IX86)
    using LibraryFunction = int (__stdcall *)(int);
#else
    using LibraryFunction = int (*)(int);
#endif
    HMODULE module = LoadLibraryW(injected_library_path.wstring().c_str());
    expect(module != nullptr, "native library bridge shell-safety fixture should load the copied DLL");
    if (module != nullptr) {
        const auto function = reinterpret_cast<LibraryFunction>(GetProcAddress(module, "InitLibrary"));
        expect(function != nullptr, "native library bridge shell-safety fixture should resolve InitLibrary");
        if (function != nullptr) {
            (void)function(0);
        }
        FreeLibrary(module);
    }
#else
    void* module = dlopen(injected_library_path.c_str(), RTLD_NOW | RTLD_LOCAL);
    expect(module != nullptr, "native library bridge shell-safety fixture should load the copied shared library");
    if (module != nullptr) {
        using LibraryFunction = int (*)(int);
        const auto function = reinterpret_cast<LibraryFunction>(dlsym(module, "InitLibrary"));
        expect(function != nullptr, "native library bridge shell-safety fixture should resolve InitLibrary");
        if (function != nullptr) {
            (void)function(0);
        }
        dlclose(module);
    }
    expect(!fs::exists(sentinel_path),
           "native library bridge should not evaluate shell syntax from a package path");
#endif

    fs::remove_all(injected_package_directory, ignored);
    fs::remove(sentinel_path, ignored);
}

void run_library_build_host_smoke(
    const std::string& build_host_path,
    const std::string& extension) {
    namespace fs = std::filesystem;

    expect(fs::exists(build_host_path), "build host executable should exist before running the smoke test");
    const fs::path temp_root = fs::temp_directory_path() / ("copperfin_build_host_" + extension + "_smoke");
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path project_path = project_dir / "librarydemo.pjx";
    const fs::path expected_output = output_dir / "LibraryDemo" / ("LibraryDemo." + extension);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    fs::create_directories(output_dir);

    write_text(project_dir / "librarymain.prg", "PROCEDURE InitLibrary\nLPARAMETERS tcMode\nRETURN\nENDPROC\n");
    write_text(project_dir / "helper.prg", "FUNCTION AddNumbers\nPARAMETERS tnLeft, tnRight\nRETURN 1\nENDFUNC\n");
    write_synthetic_project(project_path, project_dir, output_dir / ("LibraryDemo." + extension));

    const auto process = run_process_capture(
        build_host_path,
        {"build", "--project", project_path.string(), "--output-dir", output_dir.string()},
        temp_root);

    expect_process_success(process, "build host should succeed for " + extension + " library outputs");
    expect(process.stdout_text.find("status: ok") != std::string::npos,
           "build host should report success for " + extension + " outputs");
    expect(process.stdout_text.find("output.kind: " + extension) != std::string::npos,
           "build host should report the correct output kind for " + extension + " outputs");
    expect(process.stdout_text.find("primary.output.materialized: true") != std::string::npos,
           "build host should report a materialized primary output for " + extension + " outputs");
    expect(fs::exists(expected_output),
           "build host should materialize the requested primary output for " + extension + " outputs");

    const fs::path manifest_path = value_for_key(process.stdout_text, "manifest.path");
    const fs::path debug_manifest_path = value_for_key(process.stdout_text, "debug.manifest.path");
    const fs::path expected_ast_manifest = output_dir / "LibraryDemo" / ("LibraryDemo." + extension + ".ast.json");
    const fs::path expected_ir_manifest = output_dir / "LibraryDemo" / ("LibraryDemo." + extension + ".ir.json");
    const fs::path expected_transpiled_csharp = output_dir / "LibraryDemo" / ("LibraryDemo." + extension + ".transpiled.cs");
    const fs::path expected_audit_log = output_dir / "LibraryDemo" / "security_audit.log";
    expect(!manifest_path.empty(), "build host should report a manifest path for " + extension + " outputs");
    expect(!debug_manifest_path.empty(), "build host should report a debug-manifest path for " + extension + " outputs");
    const std::string init_library_source = (project_dir / "librarymain.prg").string();
    const std::string add_numbers_source = (project_dir / "helper.prg").string();
    const std::string manifest_text = manifest_path.empty() ? std::string{} : read_text(manifest_path);
    const std::string debug_manifest_text = debug_manifest_path.empty() ? std::string{} : read_text(debug_manifest_path);
    const std::vector<std::string> manifest_asset_lines = lines_with_prefix(manifest_text, "asset=");
    if (!manifest_path.empty()) {
        expect(manifest_text.find("primary_output_materialized=") == std::string::npos,
               "build host runtime manifest should omit the materialized primary output state for " + extension + " outputs");
        expect(manifest_text.find("project_title=LibraryDemo") != std::string::npos,
               "build host manifest should record the project title for " + extension + " outputs");
        expect(manifest_text.find("project_path=" + project_path.string()) == std::string::npos,
               "build host runtime manifest should omit the project path for " + extension + " outputs");
        expect(manifest_text.find("package_root=" + quote_manifest_value((output_dir / "LibraryDemo").string())) != std::string::npos,
               "build host manifest should record the package root for " + extension + " outputs");
        expect(manifest_text.find("content_root=" + quote_manifest_value((output_dir / "LibraryDemo" / "content").string())) != std::string::npos,
               "build host manifest should record the content root for " + extension + " outputs");
        expect(manifest_text.find("ast_manifest_path=" + expected_ast_manifest.string()) == std::string::npos,
               "build host runtime manifest should omit the AST manifest path for " + extension + " outputs");
        expect(manifest_text.find("ir_manifest_path=" + expected_ir_manifest.string()) == std::string::npos,
               "build host runtime manifest should omit the IR manifest path for " + extension + " outputs");
        expect(manifest_text.find("transpiled_csharp_path=" + expected_transpiled_csharp.string()) == std::string::npos,
               "build host runtime manifest should omit the transpiled C# path for " + extension + " outputs");
        expect(manifest_text.find("primary_output_path=") == std::string::npos,
               "build host runtime manifest should omit the primary output path for " + extension + " outputs");
        expect(manifest_text.find("module_definition_path=") == std::string::npos,
               "build host runtime manifest should omit the module-definition path for " + extension + " outputs");
        expect(manifest_text.find("library_api_manifest_path=") == std::string::npos,
               "build host runtime manifest should omit the DLL API-manifest path for " + extension + " outputs");
        expect(manifest_text.find("fll_api_manifest_path=") == std::string::npos,
               "build host runtime manifest should omit the FLL API-manifest path for " + extension + " outputs");
        expect(manifest_text.find("fxp_token_manifest_path=") == std::string::npos,
               "build host runtime manifest should omit the FXP token-manifest path for " + extension + " outputs");
        expect(manifest_text.find("app_archive_manifest_path=") == std::string::npos,
               "build host runtime manifest should omit the APP archive-manifest path for " + extension + " outputs");
        expect(manifest_text.find("configuration=debug") != std::string::npos,
               "build host manifest should record the debug build configuration for " + extension + " outputs");
        expect(manifest_text.find("security_enabled=false") != std::string::npos,
               "build host manifest should record the disabled security state for " + extension + " outputs");
        expect(manifest_text.find("audit_log_path=" + quote_manifest_value(expected_audit_log.string())) != std::string::npos,
               "build host manifest should record the audit log path for " + extension + " outputs");
        expect(manifest_text.find("runtime_host_sha256=") != std::string::npos,
               "build host manifest should record the runtime host SHA-256 digest for " + extension + " outputs");
        expect(manifest_text.find("security_roles=") == std::string::npos,
               "build host runtime manifest should omit the security-role count for " + extension + " outputs");
        expect(manifest_text.find("extension_payload=" + quote_manifest_value(expected_output.string()) + "|") != std::string::npos,
               "build host manifest should record the built primary output as an extension payload for " + extension + " outputs");
        if (extension == "dll") {
            expect(manifest_text.find("library_callable_convention=vfp_declare_default") != std::string::npos,
                   "build host manifest should record the VFP DLL calling convention contract");
            expect(lines_with_prefix(manifest_text, "library_function_").empty(),
                   "build host runtime manifest should omit DLL library-function inventory");
        }
    }
    if (!debug_manifest_path.empty()) {
        expect(debug_manifest_text.find("primary_output_path=" + quote_manifest_value(expected_output.string())) != std::string::npos,
               "build host debug manifest should record the materialized primary output path for " + extension + " outputs");
        expect(debug_manifest_text.find("project_title=LibraryDemo") != std::string::npos,
               "build host debug manifest should record the project title for " + extension + " outputs");
        expect(debug_manifest_text.find("project_path=" + quote_manifest_value(project_path.string())) != std::string::npos,
               "build host debug manifest should record the project path for " + extension + " outputs");
        expect(debug_manifest_text.find("package_root=" + quote_manifest_value((output_dir / "LibraryDemo").string())) != std::string::npos,
               "build host debug manifest should record the package root for " + extension + " outputs");
        expect(debug_manifest_text.find("content_root=" + quote_manifest_value((output_dir / "LibraryDemo" / "content").string())) != std::string::npos,
               "build host debug manifest should record the content root for " + extension + " outputs");
        expect(debug_manifest_text.find("ast_manifest_path=" + quote_manifest_value(expected_ast_manifest.string())) != std::string::npos,
               "build host debug manifest should record the AST manifest path for " + extension + " outputs");
        expect(debug_manifest_text.find("ir_manifest_path=" + quote_manifest_value(expected_ir_manifest.string())) != std::string::npos,
               "build host debug manifest should record the IR manifest path for " + extension + " outputs");
        expect(debug_manifest_text.find("transpiled_csharp_path=" + quote_manifest_value(expected_transpiled_csharp.string())) != std::string::npos,
               "build host debug manifest should record the transpiled C# path for " + extension + " outputs");
        expect(debug_manifest_text.find("configuration=debug") != std::string::npos,
               "build host debug manifest should record the debug build configuration for " + extension + " outputs");
        expect(debug_manifest_text.find("security_enabled=false") != std::string::npos,
               "build host debug manifest should record the disabled security state for " + extension + " outputs");
        expect(debug_manifest_text.find("audit_log_path=" + quote_manifest_value(expected_audit_log.string())) != std::string::npos,
               "build host debug manifest should record the audit log path for " + extension + " outputs");
        const std::string security_role = manifest_value_for_key(manifest_text, "security_role");
        const std::string security_mode = manifest_value_for_key(manifest_text, "security_mode");
        const std::string runtime_host_sha256 = manifest_value_for_key(manifest_text, "runtime_host_sha256");
        const bool debug_manifest_has_security_roles = debug_manifest_text.find("security_roles=") != std::string::npos;
        const std::string dotnet_story = manifest_value_for_key(manifest_text, "dotnet_story");
        expect(debug_manifest_text.find("security_role=" + security_role) != std::string::npos,
               "build host debug manifest should mirror the effective security role for " + extension + " outputs");
        expect(debug_manifest_text.find("security_mode=" + security_mode) != std::string::npos,
               "build host debug manifest should mirror the security mode for " + extension + " outputs");
        expect(debug_manifest_text.find("runtime_host_sha256=" + runtime_host_sha256) != std::string::npos,
               "build host debug manifest should mirror the runtime host SHA-256 digest for " + extension + " outputs");
        expect(debug_manifest_has_security_roles,
               "build host debug manifest should mirror the security-role count for " + extension + " outputs");
        expect(!dotnet_story.empty(),
               "build host runtime manifest should preserve the .NET story for " + extension + " outputs");
        expect(debug_manifest_text.find("dotnet_story=" + dotnet_story) != std::string::npos,
               "build host debug manifest should mirror the .NET story for " + extension + " outputs");
        expect(manifest_value_for_key(manifest_text, "dotnet_enabled").empty(),
               "build host runtime manifest should omit the .NET availability flag for " + extension + " outputs");
        expect(manifest_value_for_key(manifest_text, "dotnet_policy_allowlist").empty(),
               "build host runtime manifest should omit the .NET allowlist summary for " + extension + " outputs");
        expect(manifest_value_for_key(manifest_text, "dotnet_policy_denylist").empty(),
               "build host runtime manifest should omit the .NET denylist summary for " + extension + " outputs");
        expect(manifest_value_for_key(manifest_text, "dotnet_parity_matrix_entries").empty(),
               "build host runtime manifest should omit the .NET parity summary for " + extension + " outputs");
        expect(manifest_value_for_key(manifest_text, "dotnet_policy_allowlist_items").empty(),
               "build host runtime manifest should omit the .NET allowlist item count for " + extension + " outputs");
        expect(manifest_value_for_key(manifest_text, "dotnet_policy_denylist_items").empty(),
               "build host runtime manifest should omit the .NET denylist item count for " + extension + " outputs");
        expect(manifest_value_for_key(manifest_text, "dotnet_parity_matrix_count").empty(),
               "build host runtime manifest should omit the .NET parity item count for " + extension + " outputs");
        expect(manifest_value_for_key(manifest_text, "dotnet_gateway_task_primitives").empty(),
               "build host runtime manifest should omit the .NET gateway allow decision for " + extension + " outputs");
        expect(manifest_value_for_key(manifest_text, "dotnet_gateway_unsafe_reflection").empty(),
               "build host runtime manifest should omit the .NET gateway deny decision for " + extension + " outputs");
        expect(!manifest_value_for_key(debug_manifest_text, "dotnet_enabled").empty(),
               "build host debug manifest should preserve the .NET availability flag for " + extension + " outputs");
        expect(!manifest_value_for_key(debug_manifest_text, "dotnet_policy_allowlist").empty(),
               "build host debug manifest should preserve the .NET allowlist summary for " + extension + " outputs");
        expect(!manifest_value_for_key(debug_manifest_text, "dotnet_policy_denylist").empty(),
               "build host debug manifest should preserve the .NET denylist summary for " + extension + " outputs");
        expect(!manifest_value_for_key(debug_manifest_text, "dotnet_parity_matrix_entries").empty(),
               "build host debug manifest should preserve the .NET parity summary for " + extension + " outputs");
        expect(!manifest_value_for_key(debug_manifest_text, "dotnet_policy_allowlist_items").empty(),
               "build host debug manifest should preserve the .NET allowlist item count for " + extension + " outputs");
        expect(!manifest_value_for_key(debug_manifest_text, "dotnet_policy_denylist_items").empty(),
               "build host debug manifest should preserve the .NET denylist item count for " + extension + " outputs");
        expect(!manifest_value_for_key(debug_manifest_text, "dotnet_parity_matrix_count").empty(),
               "build host debug manifest should preserve the .NET parity item count for " + extension + " outputs");
        expect(!manifest_value_for_key(debug_manifest_text, "dotnet_gateway_task_primitives").empty(),
               "build host debug manifest should preserve the .NET gateway allow decision for " + extension + " outputs");
        expect(!manifest_value_for_key(debug_manifest_text, "dotnet_gateway_unsafe_reflection").empty(),
               "build host debug manifest should preserve the .NET gateway deny decision for " + extension + " outputs");
        const std::vector<std::string> runtime_allowlist_items = lines_with_prefix(manifest_text, "dotnet_policy_allowlist_item=");
        const std::vector<std::string> debug_allowlist_items = lines_with_prefix(debug_manifest_text, "dotnet_policy_allowlist_item=");
        expect(runtime_allowlist_items.empty(),
               "build host runtime manifest should omit the .NET allowlist items for " + extension + " outputs");
        expect(!debug_allowlist_items.empty(),
               "build host debug manifest should preserve the .NET allowlist items for " + extension + " outputs");
        const std::vector<std::string> runtime_denylist_items = lines_with_prefix(manifest_text, "dotnet_policy_denylist_item=");
        const std::vector<std::string> debug_denylist_items = lines_with_prefix(debug_manifest_text, "dotnet_policy_denylist_item=");
        expect(runtime_denylist_items.empty(),
               "build host runtime manifest should omit the .NET denylist items for " + extension + " outputs");
        expect(!debug_denylist_items.empty(),
               "build host debug manifest should preserve the .NET denylist items for " + extension + " outputs");
        const std::vector<std::string> runtime_parity_items = lines_with_prefix(manifest_text, "dotnet_parity_matrix_item=");
        const std::vector<std::string> debug_parity_items = lines_with_prefix(debug_manifest_text, "dotnet_parity_matrix_item=");
        expect(runtime_parity_items.empty(),
               "build host runtime manifest should omit the .NET parity entries for " + extension + " outputs");
        expect(!debug_parity_items.empty(),
               "build host debug manifest should preserve the .NET parity entries for " + extension + " outputs");
        const std::vector<std::string> extensibility_summary_keys{
            "language_integration_count",
            "ai_feature_count",
            "extensibility_guardrail_count",
            "language_integrations",
            "ai_features"};
        for (const auto& key : extensibility_summary_keys) {
            expect(manifest_value_for_key(manifest_text, key).empty(),
                   "build host runtime manifest should omit " + key + " for " + extension + " outputs");
            expect(!manifest_value_for_key(debug_manifest_text, key).empty(),
                   "build host debug manifest should preserve " + key + " for " + extension + " outputs");
        }
        const std::vector<std::string> runtime_language_integrations = lines_with_prefix(manifest_text, "language_integration=");
        const std::vector<std::string> debug_language_integrations = lines_with_prefix(debug_manifest_text, "language_integration=");
        expect(runtime_language_integrations.empty(),
               "build host runtime manifest should omit language integration entries for " + extension + " outputs");
        expect(!debug_language_integrations.empty(),
               "build host debug manifest should preserve language integration entries for " + extension + " outputs");
        const std::vector<std::string> runtime_ai_features = lines_with_prefix(manifest_text, "ai_feature=");
        const std::vector<std::string> debug_ai_features = lines_with_prefix(debug_manifest_text, "ai_feature=");
        expect(runtime_ai_features.empty(),
               "build host runtime manifest should omit AI feature entries for " + extension + " outputs");
        expect(!debug_ai_features.empty(),
               "build host debug manifest should preserve AI feature entries for " + extension + " outputs");
        const std::vector<std::string> runtime_guardrails = lines_with_prefix(manifest_text, "extensibility_guardrail=");
        const std::vector<std::string> debug_guardrails = lines_with_prefix(debug_manifest_text, "extensibility_guardrail=");
        expect(runtime_guardrails.empty(),
               "build host runtime manifest should omit extensibility guardrails for " + extension + " outputs");
        expect(!debug_guardrails.empty(),
               "build host debug manifest should preserve extensibility guardrails for " + extension + " outputs");
        const std::vector<std::string> runtime_feature_flags = lines_with_prefix(manifest_text, "feature_flag=");
        const std::vector<std::string> debug_feature_flags = lines_with_prefix(debug_manifest_text, "feature_flag=");
        expect(runtime_feature_flags.empty(),
               "build host runtime manifest should omit feature-flag inventory for " + extension + " outputs");
        expect(!debug_feature_flags.empty(),
               "build host debug manifest should preserve feature-flag inventory for " + extension + " outputs");
        expect(debug_manifest_text.find("primary_output_materialized=true") != std::string::npos,
               "build host debug manifest should record a materialized primary output for " + extension + " outputs");
        expect(debug_manifest_text.find("extension_payload=" + quote_manifest_value(expected_output.string()) + "|") != std::string::npos,
               "build host debug manifest should record the built primary output as an extension payload for " + extension + " outputs");
        expect(!manifest_asset_lines.empty(),
               "build host manifest should record staged asset inventory for " + extension + " outputs");
        for (const auto& asset_line : manifest_asset_lines) {
            expect(debug_manifest_text.find(asset_line) != std::string::npos,
                   "build host debug manifest should mirror each staged asset line for " + extension + " outputs");
        }
        if (extension == "dll") {
            expect(debug_manifest_text.find("module_definition_path=") != std::string::npos,
                   "build host DLL debug manifest should record the module-definition path");
            expect(debug_manifest_text.find("library_api_manifest_path=") != std::string::npos,
                   "build host DLL debug manifest should record the dedicated API-manifest path");
            const fs::path module_definition_path = value_for_key(process.stdout_text, "module.definition");
            const fs::path library_api_manifest_path = value_for_key(process.stdout_text, "library.api.manifest");
            expect(debug_manifest_text.find("compiler_contract=" + quote_manifest_value(module_definition_path.string()) + "|") != std::string::npos,
                   "build host DLL debug manifest should record the module-definition compiler-contract digest");
            expect(debug_manifest_text.find("compiler_contract=" + quote_manifest_value(library_api_manifest_path.string()) + "|") != std::string::npos,
                   "build host DLL debug manifest should record the API-manifest compiler-contract digest");
            expect(debug_manifest_text.find("feature_flag=build.output.library_contract|true|build_output") != std::string::npos,
                   "build host DLL debug manifest should expose the library-contract feature flag");
            expect(debug_manifest_text.find("feature_flag=build.output.native_library_wrapper|true|build_output") != std::string::npos,
                   "build host DLL debug manifest should expose the native-wrapper feature flag");
            expect(debug_manifest_text.find("export_symbol=InitLibrary") != std::string::npos,
                   "build host DLL debug manifest should record discovered export symbols");
            expect(debug_manifest_text.find("export_symbol=AddNumbers") != std::string::npos,
                   "build host DLL debug manifest should record all export symbols");
            expect(debug_manifest_text.find("library_function_arity=InitLibrary|1") != std::string::npos,
                   "build host DLL debug manifest should record InitLibrary arity");
            expect(debug_manifest_text.find("library_function_arity=AddNumbers|2") != std::string::npos,
                   "build host DLL debug manifest should record AddNumbers arity");
            expect(debug_manifest_text.find("library_function_kind=InitLibrary|procedure") != std::string::npos,
                   "build host DLL debug manifest should record InitLibrary routine kind");
            expect(debug_manifest_text.find("library_function_kind=AddNumbers|function") != std::string::npos,
                   "build host DLL debug manifest should record AddNumbers routine kind");
            expect(manifest_source_location_matches(
                       debug_manifest_text,
                       "library_function_source",
                       "InitLibrary",
                       init_library_source,
                       1U),
                   "build host DLL debug manifest should record InitLibrary source provenance");
            expect(manifest_source_location_matches(
                       debug_manifest_text,
                       "library_function_source",
                       "AddNumbers",
                       add_numbers_source,
                       1U),
                   "build host DLL debug manifest should record AddNumbers source provenance");
            expect(debug_manifest_text.find("library_function_parameters=InitLibrary|tcMode") != std::string::npos,
                   "build host DLL debug manifest should record InitLibrary parameter names");
            expect(debug_manifest_text.find("library_function_parameters=AddNumbers|tnLeft|tnRight") != std::string::npos,
                   "build host DLL debug manifest should record AddNumbers parameter names");
            expect(debug_manifest_text.find("library_function_parameter_declaration=InitLibrary|lparameters") != std::string::npos,
                   "build host DLL debug manifest should record InitLibrary parameter declaration style");
            expect(debug_manifest_text.find("library_function_parameter_declaration=AddNumbers|parameters") != std::string::npos,
                   "build host DLL debug manifest should record AddNumbers parameter declaration style");
            expect(debug_manifest_text.find("library_function_call_surface=InitLibrary|vfp_declare_default|int tcMode") != std::string::npos,
                   "build host DLL debug manifest should record InitLibrary call surface");
            expect(debug_manifest_text.find("library_function_call_surface=AddNumbers|vfp_declare_default|int tnLeft, int tnRight") != std::string::npos,
                   "build host DLL debug manifest should record AddNumbers call surface");
        }
        if (extension == "fll") {
            expect(debug_manifest_text.find("module_definition_path=") != std::string::npos,
                   "build host FLL debug manifest should record the module-definition path");
            expect(debug_manifest_text.find("fll_api_manifest_path=") != std::string::npos,
                   "build host FLL debug manifest should record the dedicated API-manifest path");
            const fs::path module_definition_path = value_for_key(process.stdout_text, "module.definition");
            const fs::path fll_api_manifest_path = value_for_key(process.stdout_text, "fll.api.manifest");
            expect(debug_manifest_text.find("compiler_contract=" + quote_manifest_value(module_definition_path.string()) + "|") != std::string::npos,
                   "build host FLL debug manifest should record the module-definition compiler-contract digest");
            expect(debug_manifest_text.find("compiler_contract=" + quote_manifest_value(fll_api_manifest_path.string()) + "|") != std::string::npos,
                   "build host FLL debug manifest should record the API-manifest compiler-contract digest");
            expect(debug_manifest_text.find("feature_flag=build.output.library_contract|true|build_output") != std::string::npos,
                   "build host FLL debug manifest should expose the library-contract feature flag");
            expect(debug_manifest_text.find("feature_flag=build.output.native_library_wrapper|true|build_output") != std::string::npos,
                   "build host FLL debug manifest should expose the native-wrapper feature flag");
            expect(debug_manifest_text.find("feature_flag=build.output.fll_api_contract|true|build_output") != std::string::npos,
                   "build host FLL debug manifest should expose the FLL API-contract feature flag");
            expect(debug_manifest_text.find("export_symbol=InitLibrary") != std::string::npos,
                   "build host FLL debug manifest should record discovered routine export symbols");
            expect(debug_manifest_text.find("export_symbol=AddNumbers") != std::string::npos,
                   "build host FLL debug manifest should record all routine export symbols");
            expect(debug_manifest_text.find("library_function_kind=InitLibrary|procedure") != std::string::npos,
                   "build host FLL debug manifest should record InitLibrary routine kind");
            expect(debug_manifest_text.find("library_function_kind=AddNumbers|function") != std::string::npos,
                   "build host FLL debug manifest should record AddNumbers routine kind");
            expect(manifest_source_location_matches(
                       debug_manifest_text,
                       "library_function_source",
                       "InitLibrary",
                       init_library_source,
                       1U),
                   "build host FLL debug manifest should record InitLibrary source provenance");
            expect(manifest_source_location_matches(
                       debug_manifest_text,
                       "library_function_source",
                       "AddNumbers",
                       add_numbers_source,
                       1U),
                   "build host FLL debug manifest should record AddNumbers source provenance");
            expect(debug_manifest_text.find("library_function_parameters=InitLibrary|tcMode") != std::string::npos,
                   "build host FLL debug manifest should record InitLibrary parameter names");
            expect(debug_manifest_text.find("library_function_parameters=AddNumbers|tnLeft|tnRight") != std::string::npos,
                   "build host FLL debug manifest should record AddNumbers parameter names");
            expect(debug_manifest_text.find("library_function_parameter_declaration=InitLibrary|lparameters") != std::string::npos,
                   "build host FLL debug manifest should record InitLibrary parameter declaration style");
            expect(debug_manifest_text.find("library_function_parameter_declaration=AddNumbers|parameters") != std::string::npos,
                   "build host FLL debug manifest should record AddNumbers parameter declaration style");
            expect(debug_manifest_text.find("library_function_call_surface=InitLibrary|ParamBlk*|_RetInt") != std::string::npos,
                   "build host FLL debug manifest should record InitLibrary callable surface");
            expect(debug_manifest_text.find("library_function_call_surface=AddNumbers|ParamBlk*|_RetInt") != std::string::npos,
                   "build host FLL debug manifest should record AddNumbers callable surface");
        }
    }

    if (fs::exists(expected_output) && native_symbol_dump_is_available()) {
        std::string symbol_error;
        const std::set<std::string> exported_symbols = read_native_exported_symbols(expected_output, symbol_error);
        if (exported_symbols.empty() && !symbol_error.empty()) {
            std::cerr << "FAIL: " << symbol_error << "\n";
        }

        const fs::path module_definition_path = value_for_key(process.stdout_text, "module.definition");
        const std::set<std::string> declared_module_symbols = read_module_definition_exports(module_definition_path);
        expect(exported_symbols == declared_module_symbols,
               "build host should preserve the module-definition export contract for " + extension + " outputs");

        if (extension == "dll") {
            const fs::path library_api_manifest_path = value_for_key(process.stdout_text, "library.api.manifest");
            const std::set<std::string> declared_api_symbols = read_library_api_declared_symbols(library_api_manifest_path);
            expect(exported_symbols == declared_api_symbols,
                   "build host should preserve the dedicated DLL API-manifest export contract");
            const std::string api_manifest = read_text(library_api_manifest_path);
            expect(manifest_value_for_key(manifest_text, "native_wrapper_source_path").empty(),
                   "build host DLL runtime manifest should omit the native-wrapper source path");
            expect(manifest_value_for_key(manifest_text, "native_wrapper_cmake_path").empty(),
                   "build host DLL runtime manifest should omit the native-wrapper CMake path");
            const fs::path wrapper_source_path = manifest_path_for_key(debug_manifest_text, "native_wrapper_source_path");
            const std::string wrapper_source = wrapper_source_path.empty() ? std::string{} : read_text(wrapper_source_path);
            const fs::path wrapper_cmake_path = manifest_path_for_key(debug_manifest_text, "native_wrapper_cmake_path");
            const std::string wrapper_cmake = wrapper_cmake_path.empty() ? std::string{} : read_text(wrapper_cmake_path);
            expect(api_manifest.find("output_kind=dll") != std::string::npos,
                   "build host DLL API manifest should declare the DLL output kind");
            expect(api_manifest.find("callable_convention=vfp_declare_default") != std::string::npos,
                   "build host DLL API manifest should declare the VFP DLL calling convention");
            expect(wrapper_source.find("static std::filesystem::path copperfin_wrapper_module_path(void* symbol_address)") != std::string::npos,
                   "build host DLL wrapper should derive its loaded module path");
            expect(wrapper_source.find("static std::filesystem::path copperfin_runtime_manifest_path(void* symbol_address)") != std::string::npos,
                   "build host DLL wrapper should derive a sibling manifest path");
            expect(wrapper_source.find("static std::filesystem::path copperfin_runtime_host_path(void* symbol_address)") != std::string::npos,
                   "build host DLL wrapper should derive a sibling runtime-host path");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeDescriptor") != std::string::npos,
                   "build host DLL wrapper should declare a shared bridge-descriptor surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeDescriptor copperfin_build_runtime_bridge_descriptor(") != std::string::npos,
                   "build host DLL wrapper should declare a bridge-descriptor helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeInvocation") != std::string::npos,
                   "build host DLL wrapper should declare a shared bridge-invocation surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeInvocation copperfin_build_runtime_bridge_invocation(") != std::string::npos,
                   "build host DLL wrapper should declare a bridge-invocation helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_manifest_flag()") != std::string::npos,
                   "build host DLL wrapper should declare manifest flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_manifest_flag()") != std::string::npos,
                   "build host DLL wrapper should route manifest flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_library_export_flag()") != std::string::npos,
                   "build host DLL wrapper should declare library-export flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_library_export_flag()") != std::string::npos,
                   "build host DLL wrapper should route library-export flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_routine_kind_flag()") != std::string::npos,
                   "build host DLL wrapper should declare routine-kind flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_routine_kind_flag()") != std::string::npos,
                   "build host DLL wrapper should route routine-kind flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_source_path_flag()") != std::string::npos,
                   "build host DLL wrapper should declare source-path flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_source_path_flag()") != std::string::npos,
                   "build host DLL wrapper should route source-path flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_source_line_flag()") != std::string::npos,
                   "build host DLL wrapper should declare source-line flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_source_line_flag()") != std::string::npos,
                   "build host DLL wrapper should route source-line flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_parameter_declaration_flag()") != std::string::npos,
                   "build host DLL wrapper should declare parameter-declaration flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_parameter_declaration_flag()") != std::string::npos,
                   "build host DLL wrapper should route parameter-declaration flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_parameter_names_flag()") != std::string::npos,
                   "build host DLL wrapper should declare parameter-names flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_parameter_names_flag()") != std::string::npos,
                   "build host DLL wrapper should route parameter-names flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_parameter_count_flag()") != std::string::npos,
                   "build host DLL wrapper should declare parameter-count flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_parameter_count_flag()") != std::string::npos,
                   "build host DLL wrapper should route parameter-count flag through helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeParameter") != std::string::npos,
                   "build host DLL wrapper should declare a bridge-parameter surface");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeCall") != std::string::npos,
                   "build host DLL wrapper should declare a bridge-call surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeCall copperfin_build_runtime_bridge_call(") != std::string::npos,
                   "build host DLL wrapper should declare a bridge-call helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeReturn") != std::string::npos,
                   "build host DLL wrapper should declare a return-binding surface");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeResult") != std::string::npos,
                   "build host DLL wrapper should declare a bridge-result surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResult copperfin_build_runtime_bridge_result(") != std::string::npos,
                   "build host DLL wrapper should declare a bridge-result helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturn copperfin_build_runtime_bridge_placeholder_return_binding(") != std::string::npos,
                   "build host DLL wrapper should declare a shared placeholder return-binding helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeEnvironmentVariable") != std::string::npos,
                   "build host DLL wrapper should declare a launch-environment surface");
            expect(wrapper_source.find("std::vector<CopperfinRuntimeBridgeEnvironmentVariable> environment;") != std::string::npos,
                   "build host DLL wrapper should carry launch environment entries through dispatch and launch surfaces.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeLaunchPlan") != std::string::npos,
                   "build host DLL wrapper should declare a launch-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeLaunchPlan copperfin_build_runtime_bridge_launch_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a launch-plan helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_library_export_env_var()") != std::string::npos,
                   "build host DLL wrapper should declare library-export env-var helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_library_export_env_var()") != std::string::npos,
                   "build host DLL wrapper should route library-export env-var through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_routine_kind_env_var()") != std::string::npos,
                   "build host DLL wrapper should declare routine-kind env-var helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_routine_kind_env_var()") != std::string::npos,
                   "build host DLL wrapper should route routine-kind env-var through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_source_path_env_var()") != std::string::npos,
                   "build host DLL wrapper should declare source-path env-var helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_source_path_env_var()") != std::string::npos,
                   "build host DLL wrapper should route source-path env-var through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_parameter_count_env_var()") != std::string::npos,
                   "build host DLL wrapper should declare parameter-count env-var helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_parameter_count_env_var()") != std::string::npos,
                   "build host DLL wrapper should route parameter-count env-var through helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeObservationPlan") != std::string::npos,
                   "build host DLL wrapper should declare an observation-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeObservationPlan copperfin_build_runtime_bridge_observation_plan(") != std::string::npos,
                   "build host DLL wrapper should declare an observation-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeExecutionPlan") != std::string::npos,
                   "build host DLL wrapper should declare an execution-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeExecutionPlan copperfin_build_runtime_bridge_execution_plan(") != std::string::npos,
                   "build host DLL wrapper should declare an execution-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeTransportPlan") != std::string::npos,
                   "build host DLL wrapper should declare a transport-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeTransportPlan copperfin_build_runtime_bridge_transport_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a transport-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeSerializationPlan") != std::string::npos,
                   "build host DLL wrapper should declare a serialization-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeSerializationPlan copperfin_build_runtime_bridge_serialization_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a serialization-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeDispatchPlan") != std::string::npos,
                   "build host DLL wrapper should declare a dispatch-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeDispatchPlan copperfin_build_runtime_bridge_dispatch_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a dispatch-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeDispatchExecution copperfin_runtime_bridge_execute_dispatch(") != std::string::npos,
                   "build host DLL wrapper should declare a shared dispatch-execution helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeProcessLaunch copperfin_runtime_bridge_launch_process(") != std::string::npos,
                   "build host DLL wrapper should declare a shared process-launch helper.");
            expect(wrapper_source.find("#include <windows.h>") != std::string::npos &&
                       wrapper_source.find("#include <unistd.h>") != std::string::npos,
                   "build host DLL wrapper should include native process-launch support.");
            expect(wrapper_source.find("static std::vector<std::wstring> copperfin_runtime_bridge_windows_environment(") != std::string::npos &&
                       wrapper_source.find("static std::vector<std::string> copperfin_runtime_bridge_posix_environment(") != std::string::npos,
                   "build host DLL wrapper should build native environment blocks for both supported process APIs.");
            expect(wrapper_source.find("launch_plan.environment") != std::string::npos,
                   "build host DLL wrapper should carry launch environment entries into dispatch execution.");
            expect(wrapper_source.find("const auto environment_entries = copperfin_runtime_bridge_windows_environment(dispatch_execution.environment);") != std::string::npos &&
                       wrapper_source.find("const auto environment_values = copperfin_runtime_bridge_posix_environment(dispatch_execution.environment);") != std::string::npos,
                   "build host DLL wrapper should apply launch environment entries through native process APIs.");
            expect(wrapper_source.find("CreateProcessW(") != std::string::npos &&
                       wrapper_source.find("execve(") != std::string::npos,
                   "build host DLL wrapper should launch the runtime host without a shell.");
            expect(wrapper_source.find("std::system(") == std::string::npos &&
                       wrapper_source.find("copperfin_runtime_bridge_build_process_command(") == std::string::npos,
                   "build host DLL wrapper should not execute a generated shell command.");
            expect(wrapper_source.find("const bool launch_succeeded = launch_attempted && process_created && exit_code == dispatch_execution.expected_exit_code;") != std::string::npos,
                   "build host DLL wrapper should compare runtime-host exit code with the expected exit code.");
            expect(wrapper_source.find("        false,\n        false,\n        dispatch_execution.expected_exit_code,\n        dispatch_execution.expected_exit_code") == std::string::npos,
                   "build host DLL wrapper should not keep the deterministic process-launch failure placeholder.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeHostFailureEvaluation copperfin_runtime_bridge_evaluate_host_failure(") != std::string::npos,
                   "build host DLL wrapper should declare a shared host-failure evaluation helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeMissingResponseEvaluation copperfin_runtime_bridge_evaluate_missing_response(") != std::string::npos,
                   "build host DLL wrapper should declare a shared missing-response evaluation helper.");
            expect(wrapper_source.find("const CopperfinRuntimeBridgeResponseReadPlan& response_read_plan,\n    const std::string& response_document) {") != std::string::npos,
                   "build host DLL wrapper should pass response documents into missing-response evaluation.");
            expect(wrapper_source.find("const bool response_missing = response_read_plan.require_existing_response && response_document.empty();") != std::string::npos,
                   "build host DLL wrapper should detect missing responses from the read response document.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseValidationEvaluation copperfin_runtime_bridge_evaluate_response_validation(") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-validation evaluation helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_response_document_has_required_fields(") != std::string::npos,
                   "build host DLL wrapper should declare a required response-field validation helper.");
            expect(wrapper_source.find("response_validation_plan.required_response_fields") != std::string::npos,
                   "build host DLL wrapper should validate the declared required response fields.");
            expect(wrapper_source.find("!required_response_fields_present") != std::string::npos,
                   "build host DLL wrapper should fail response validation when required response fields are absent.");
            expect(wrapper_source.find("copperfin_runtime_bridge_extract_json_field(\n        response_document,\n        copperfin_build_runtime_bridge_response_media_type_field_name())") != std::string::npos,
                   "build host DLL wrapper should read response media type during response validation.");
            expect(wrapper_source.find("response_media_type == response_validation_plan.expected_response_media_type") != std::string::npos,
                   "build host DLL wrapper should compare response media type with the expected response media type.");
            expect(wrapper_source.find("!response_media_type_matches") != std::string::npos,
                   "build host DLL wrapper should fail response validation when response media type mismatches.");
            expect(wrapper_source.find("copperfin_runtime_bridge_extract_json_field(\n        response_document,\n        copperfin_build_runtime_bridge_schema_version_field_name())") != std::string::npos,
                   "build host DLL wrapper should read response schema version during response validation.");
            expect(wrapper_source.find("response_schema_version == response_validation_plan.expected_schema_version") != std::string::npos,
                   "build host DLL wrapper should compare response schema version with the expected schema version.");
            expect(wrapper_source.find("!response_schema_version_matches") != std::string::npos,
                   "build host DLL wrapper should fail response validation when response schema version mismatches.");
            expect(wrapper_source.find("bool response_document_available = false;") != std::string::npos,
                   "build host DLL wrapper should track response-document availability in response-validation evaluation.");
            expect(wrapper_source.find("const std::string& response_document) {") != std::string::npos,
                   "build host DLL wrapper should pass response documents into response-validation evaluation.");
            expect(wrapper_source.find("const bool response_document_available = !response_document.empty();") != std::string::npos,
                   "build host DLL wrapper should derive response-document availability during response-validation evaluation.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgePayloadPlan") != std::string::npos,
                   "build host DLL wrapper should declare a payload-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgePayloadPlan copperfin_build_runtime_bridge_payload_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a payload-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeInterpretationPlan") != std::string::npos,
                   "build host DLL wrapper should declare an interpretation-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeInterpretationPlan copperfin_build_runtime_bridge_interpretation_plan(") != std::string::npos,
                   "build host DLL wrapper should declare an interpretation-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeFailurePolicyPlan") != std::string::npos,
                   "build host DLL wrapper should declare a failure-policy surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeFailurePolicyPlan copperfin_build_runtime_bridge_failure_policy_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a failure-policy helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_status_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-status field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_return_value_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-value field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_diagnostics_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-diagnostics field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_payload_shape_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared request payload-shape helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_payload_shape_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response payload-shape helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_export_name_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared export-name field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_routine_kind_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared routine-kind field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_source_path_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared source-path field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_source_line_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared source-line field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_declaration_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared parameter-declaration field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_names_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared parameter-names field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_count_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared parameter-count field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_schema_version_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared schema-version field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameters_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared parameters field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_media_type_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared request-media-type field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_fields_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared request-fields contract helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_expected_response_media_type_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared expected-response media-type helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_fields_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-fields contract helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_media_type_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-media-type field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_payload_shape_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared payload-shape field helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_payload_shape_field_name()") != std::string::npos,
                   "build host DLL wrapper should route payload-shape field through helper in request document");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_name_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared parameter-name field helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_name_field_name()") != std::string::npos,
                   "build host DLL wrapper should route parameter-name field through helper in request document");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_value_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared parameter-value field helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_value_field_name()") != std::string::npos,
                   "build host DLL wrapper should route parameter-value field through helper in request document");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_surface_field_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared parameter-surface field helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_surface_field_name()") != std::string::npos,
                   "build host DLL wrapper should route parameter-surface field through helper in request document");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_failure_diagnostics_value()") != std::string::npos,
                   "build host DLL wrapper should declare a shared failure-diagnostics token helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_success_status_value()") != std::string::npos,
                   "build host DLL wrapper should declare a shared success-status token helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeResponseValidationPlan") != std::string::npos,
                   "build host DLL wrapper should declare a response-validation surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseValidationPlan copperfin_build_runtime_bridge_response_validation_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a response-validation helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeRequestArtifact") != std::string::npos,
                   "build host DLL wrapper should declare a request-artifact surface");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_document(") != std::string::npos,
                   "build host DLL wrapper should declare a request-document helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeRequestArtifact copperfin_build_runtime_bridge_request_artifact(") != std::string::npos,
                   "build host DLL wrapper should declare a request-artifact helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeRequestWritePlan") != std::string::npos,
                   "build host DLL wrapper should declare a request-write-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeRequestWritePlan copperfin_build_runtime_bridge_request_write_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a request-write-plan helper");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_execute_write_request(") != std::string::npos,
                   "build host DLL wrapper should declare a shared request-write execution helper.");
            expect(wrapper_source.find("out << plan.request_artifact.request_document;") != std::string::npos,
                   "build host DLL wrapper should stage request-document writes through the shared request-write execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeResponseReadPlan") != std::string::npos,
                   "build host DLL wrapper should declare a response-read-plan surface");
            expect(wrapper_source.find("bool request_write_succeeded = false;") != std::string::npos,
                   "build host DLL wrapper should carry request-write success on the response-read plan.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseReadPlan copperfin_build_runtime_bridge_response_read_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a response-read-plan helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_execute_read_response(") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-read execution helper.");
            expect(wrapper_source.find("if (!plan.request_write_succeeded)") != std::string::npos,
                   "build host DLL wrapper should fall back when request writing failed before reading a response.");
            expect(wrapper_source.find("response_document << input.rdbuf();") != std::string::npos,
                   "build host DLL wrapper should stage response-document reads through the shared response-read execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeResponseArtifact") != std::string::npos,
                   "build host DLL wrapper should declare a response-artifact surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseArtifact copperfin_build_runtime_bridge_response_artifact(") != std::string::npos,
                   "build host DLL wrapper should declare a response-artifact helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeResponseParsePlan") != std::string::npos,
                   "build host DLL wrapper should declare a response-parse-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseParsePlan copperfin_build_runtime_bridge_response_parse_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a response-parse-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseParseAdmission copperfin_runtime_bridge_admit_response_parse(") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-parse admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeParsedResponse copperfin_runtime_bridge_execute_parse_response(") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-parse execution helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_extract_json_field(response_document, plan.status_field)") != std::string::npos,
                   "build host DLL wrapper should stage response field extraction through the shared response-parse execution helper.");
            expect(wrapper_source.find("object_depth == 1U && array_depth == 0U") != std::string::npos,
                   "build host DLL wrapper should validate required response fields as top-level object fields");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_find_json_field_value_start(") != std::string::npos,
                   "build host DLL wrapper should declare a shared response field-value scanner");
            expect(wrapper_source.find("copperfin_runtime_bridge_find_json_field_value_start(response_document, field_name, value_start)") != std::string::npos,
                   "build host DLL wrapper should extract response field values through the shared scanner");
            expect(wrapper_source.find("response_document.compare(index, field_token.size(), field_token) == 0") != std::string::npos,
                   "build host DLL wrapper should compare required response fields through the scanner token match");
            expect(wrapper_source.find("return response_document.find(field_token) != std::string::npos;") == std::string::npos,
                   "build host DLL wrapper should not validate required response fields with raw token search");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeInterpretedResultPlan") != std::string::npos,
                   "build host DLL wrapper should declare an interpreted-result-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeInterpretedResultPlan copperfin_build_runtime_bridge_interpreted_result_plan(") != std::string::npos,
                   "build host DLL wrapper should declare an interpreted-result-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeInterpretedResultAdmission copperfin_runtime_bridge_admit_interpreted_result(") != std::string::npos,
                   "build host DLL wrapper should declare a shared interpreted-result admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeInterpretedResult copperfin_runtime_bridge_execute_interpreted_result(") != std::string::npos,
                   "build host DLL wrapper should declare a shared interpreted-result execution helper.");
            expect(wrapper_source.find("parsed_response.status_value == plan.success_status_value") != std::string::npos,
                   "build host DLL wrapper should stage interpreted-result selection through the shared execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeNativeReturnPlan") != std::string::npos,
                   "build host DLL wrapper should declare a native-return-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeNativeReturnPlan copperfin_build_runtime_bridge_native_return_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a native-return-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeNativeReturnAdmission copperfin_runtime_bridge_admit_native_return(") != std::string::npos,
                   "build host DLL wrapper should declare a shared native-return admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeNativeReturn copperfin_runtime_bridge_execute_native_return(") != std::string::npos,
                   "build host DLL wrapper should declare a shared native-return execution helper.");
            expect(wrapper_source.find("interpreted_result.matched_success_status") != std::string::npos,
                   "build host DLL wrapper should stage native-return selection through the shared execution helper.");
            expect(wrapper_source.find("static int copperfin_parse_runtime_bridge_int_value_representation(") != std::string::npos,
                   "build host DLL wrapper should declare an integer return-representation parser");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_parse_json_string_at(") != std::string::npos,
                   "build host DLL wrapper should declare a JSON string escape decoder for response parsing");
            expect(wrapper_source.find("copperfin_runtime_bridge_parse_json_string_at(response_document, value_start, string_end, decoded_value)") != std::string::npos,
                   "build host DLL wrapper should decode escaped response string fields before interpreting returns");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_default_int_value()") != std::string::npos,
                   "build host DLL wrapper should declare a shared parsed-int default sentinel helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_default_int_value()") != std::string::npos,
                   "build host DLL wrapper should route the parsed-int default sentinel through the shared helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeOutcomeSelectionPlan") != std::string::npos,
                   "build host DLL wrapper should declare an outcome-selection-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeOutcomeSelectionPlan copperfin_build_runtime_bridge_outcome_selection_plan(") != std::string::npos,
                   "build host DLL wrapper should declare an outcome-selection-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeOutcomeSelectionAdmission copperfin_runtime_bridge_admit_outcome_selection(") != std::string::npos,
                   "build host DLL wrapper should declare a shared outcome-selection admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeOutcomeSelection copperfin_runtime_bridge_execute_outcome_selection(") != std::string::npos,
                   "build host DLL wrapper should declare a shared outcome-selection execution helper.");
            expect(wrapper_source.find("native_return.matched_success_status") != std::string::npos,
                   "build host DLL wrapper should stage outcome selection through the shared execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeReturnMaterializationPlan") != std::string::npos,
                   "build host DLL wrapper should declare a return-materialization-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnMaterializationPlan copperfin_build_runtime_bridge_return_materialization_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a return-materialization-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnMaterializationAdmission copperfin_runtime_bridge_admit_return_materialization(") != std::string::npos,
                   "build host DLL wrapper should declare a shared return-materialization admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnMaterialization copperfin_runtime_bridge_execute_return_materialization(") != std::string::npos,
                   "build host DLL wrapper should declare a shared return-materialization execution helper.");
            expect(wrapper_source.find("const auto& outcome_selection = plan.outcome_selection") != std::string::npos,
                   "build host DLL wrapper should consume explicit outcome selection while materializing returns.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_native_int_return_surface()") != std::string::npos,
                   "build host DLL wrapper should declare a shared native-int return-surface helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_native_int_return_surface()") != std::string::npos,
                   "build host DLL wrapper should route native-int return-surface comparisons through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_native_int_placeholder_signature_token()") != std::string::npos,
                   "build host DLL wrapper should declare a shared native-int placeholder-signature helper.");
            expect(wrapper_source.find("find(copperfin_build_runtime_bridge_native_int_placeholder_signature_token())") != std::string::npos,
                   "build host DLL wrapper should route native-int placeholder-signature matching through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_return_statement_from_expression(") != std::string::npos,
                   "build host DLL wrapper should declare a shared native return-statement framing helper.");
            expect(wrapper_source.find("return copperfin_build_runtime_bridge_return_statement_from_expression(") != std::string::npos,
                   "build host DLL wrapper should route native return-statement framing through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_typed_native_return_expression(") != std::string::npos,
                   "build host DLL wrapper should declare a shared typed native return-expression helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_typed_native_return_expression(") != std::string::npos,
                   "build host DLL wrapper should route typed native return-expression construction through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_stdout_log_suffix()") != std::string::npos,
                   "build host DLL wrapper should declare a shared stdout log-file suffix helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_stdout_log_suffix()") != std::string::npos,
                   "build host DLL wrapper should route stdout log-file suffix through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_stderr_log_suffix()") != std::string::npos,
                   "build host DLL wrapper should declare a shared stderr log-file suffix helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_stderr_log_suffix()") != std::string::npos,
                   "build host DLL wrapper should route stderr log-file suffix through the shared helper.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_expected_exit_code()") != std::string::npos,
                   "build host DLL wrapper should declare a shared expected-exit-code helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_expected_exit_code()") != std::string::npos,
                   "build host DLL wrapper should route expected-exit-code through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_request_artifact_suffix()") != std::string::npos,
                   "build host DLL wrapper should declare a shared request artifact suffix helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_request_artifact_suffix()") != std::string::npos,
                   "build host DLL wrapper should route request artifact suffix through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_response_artifact_suffix()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response artifact suffix helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_response_artifact_suffix()") != std::string::npos,
                   "build host DLL wrapper should route response artifact suffix through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_activates_adopted_return_policy()") != std::string::npos,
                   "build host DLL wrapper should declare a shared activates-adopted-return policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_activates_adopted_return_policy()") != std::string::npos,
                   "build host DLL wrapper should route activates-adopted-return policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_capture_stdout_policy()") != std::string::npos,
                   "build host DLL wrapper should declare a shared capture-stdout policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_capture_stdout_policy()") != std::string::npos,
                   "build host DLL wrapper should route capture-stdout policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_capture_stderr_policy()") != std::string::npos,
                   "build host DLL wrapper should declare a shared capture-stderr policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_capture_stderr_policy()") != std::string::npos,
                   "build host DLL wrapper should route capture-stderr policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_fail_on_nonzero_exit_policy()") != std::string::npos,
                   "build host DLL wrapper should declare a shared fail-on-nonzero-exit policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_fail_on_nonzero_exit_policy()") != std::string::npos,
                   "build host DLL wrapper should route fail-on-nonzero-exit policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_fail_on_missing_response_policy()") != std::string::npos,
                   "build host DLL wrapper should declare a shared fail-on-missing-response policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_fail_on_missing_response_policy()") != std::string::npos,
                   "build host DLL wrapper should route fail-on-missing-response policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_ensure_parent_directory_policy()") != std::string::npos,
                   "build host DLL wrapper should declare a shared ensure-parent-directory policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_ensure_parent_directory_policy()") != std::string::npos,
                   "build host DLL wrapper should route ensure-parent-directory policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_require_existing_response_policy()") != std::string::npos,
                   "build host DLL wrapper should declare a shared require-existing-response policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_require_existing_response_policy()") != std::string::npos,
                   "build host DLL wrapper should route require-existing-response policy through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_replace_placeholder_return_mode()") != std::string::npos,
                   "build host DLL wrapper should declare a shared replace-placeholder-return adoption-mode helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_replace_placeholder_return_mode()") != std::string::npos,
                   "build host DLL wrapper should route replace-placeholder-return mode token through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_planned_activation_pending_mode()") != std::string::npos,
                   "build host DLL wrapper should declare a shared planned-activation-pending activation-mode helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_planned_activation_pending_mode()") != std::string::npos,
                   "build host DLL wrapper should route planned-activation-pending mode token through the shared helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeReturnEmissionPlan") != std::string::npos,
                   "build host DLL wrapper should declare a return-emission-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnEmissionPlan copperfin_build_runtime_bridge_return_emission_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a return-emission-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnEmissionAdmission copperfin_runtime_bridge_admit_return_emission(") != std::string::npos,
                   "build host DLL wrapper should declare a shared return-emission admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnEmission copperfin_runtime_bridge_execute_return_emission(") != std::string::npos,
                   "build host DLL wrapper should declare a shared return-emission execution helper.");
            expect(wrapper_source.find("const auto& return_materialization = plan.return_materialization") != std::string::npos,
                   "build host DLL wrapper should consume explicit materialized return while emitting returns.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeFinalReturnAdoptionPlan") != std::string::npos,
                   "build host DLL wrapper should declare a final-return-adoption-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeFinalReturnAdoptionPlan copperfin_build_runtime_bridge_final_return_adoption_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a final-return-adoption-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeFinalReturnAdoptionAdmission copperfin_runtime_bridge_admit_final_return_adoption(") != std::string::npos,
                   "build host DLL wrapper should declare a shared final-return-adoption admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeFinalReturnAdoption copperfin_runtime_bridge_execute_final_return_adoption(") != std::string::npos,
                   "build host DLL wrapper should declare a shared final-return-adoption execution helper.");
            expect(wrapper_source.find("const auto& return_emission = plan.return_emission") != std::string::npos,
                   "build host DLL wrapper should consume explicit return emission while adopting final returns.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_placeholder_return_statement(") != std::string::npos,
                   "build host DLL wrapper should declare a shared placeholder return-statement helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeReturnActivationPlan") != std::string::npos,
                   "build host DLL wrapper should declare a return-activation-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnActivationPlan copperfin_build_runtime_bridge_return_activation_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a return-activation-plan helper");
            expect(wrapper_source.find("CopperfinRuntimeBridgeStubEmissionWrapper stub_emission_wrapper;") != std::string::npos,
                   "build host DLL wrapper should carry the stub-emission wrapper contract through the descriptor plan.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnActivationAdmission copperfin_runtime_bridge_admit_return_activation(") != std::string::npos,
                   "build host DLL wrapper should declare a shared return-activation admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnActivation copperfin_runtime_bridge_execute_return_activation(") != std::string::npos,
                   "build host DLL wrapper should declare a shared return-activation execution helper.");
            expect(wrapper_source.find("const auto& final_return_adoption = plan.final_return_adoption") != std::string::npos,
                   "build host DLL wrapper should consume explicit final-return adoption while activating returns.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeStubReturnPlan") != std::string::npos,
                   "build host DLL wrapper should declare a stub-return-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubReturnPlan copperfin_build_runtime_bridge_stub_return_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a stub-return-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubReturnAdmission copperfin_runtime_bridge_admit_stub_return(") != std::string::npos,
                   "build host DLL wrapper should declare a shared stub-return admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubReturn copperfin_runtime_bridge_execute_stub_return(") != std::string::npos,
                   "build host DLL wrapper should declare a shared stub-return execution helper.");
            expect(wrapper_source.find("const auto& return_activation = plan.return_activation") != std::string::npos,
                   "build host DLL wrapper should consume explicit return activation while routing stub returns.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgePlaceholderReturnValuePlan") != std::string::npos,
                   "build host DLL wrapper should declare a placeholder-return-value-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgePlaceholderReturnValuePlan copperfin_build_runtime_bridge_placeholder_return_value_plan(") != std::string::npos,
                   "build host DLL wrapper should declare a placeholder-return-value-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgePlaceholderReturnValueAdmission copperfin_runtime_bridge_admit_placeholder_return_value(") != std::string::npos,
                   "build host DLL wrapper should declare a shared placeholder-return-value admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgePlaceholderReturnValue copperfin_runtime_bridge_execute_placeholder_return_value(") != std::string::npos,
                   "build host DLL wrapper should declare a shared placeholder-return-value execution helper.");
            expect(wrapper_source.find("const auto& stub_return = plan.stub_return") != std::string::npos,
                   "build host DLL wrapper should consume explicit stub return while planning placeholder return values.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgePlaceholderReturnIntAdmission copperfin_runtime_bridge_admit_placeholder_return_int(") != std::string::npos,
                   "build host DLL wrapper should declare a shared placeholder-return-int admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubEmission copperfin_runtime_bridge_execute_stub_emission(") != std::string::npos,
                   "build host DLL wrapper should declare a shared stub-emission execution helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubEmissionReturnSurface copperfin_runtime_bridge_build_stub_emission_return_surface(") != std::string::npos,
                   "build host DLL wrapper should declare a shared stub-emission return-surface helper.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_apply_stub_emission_output(") != std::string::npos,
                   "build host DLL wrapper should declare a shared stub-emission output-application helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeStubEmissionWrapper") != std::string::npos,
                   "build host DLL wrapper should declare a shared stub-emission wrapper surface.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubEmissionWrapper copperfin_runtime_bridge_build_stub_emission_wrapper(") != std::string::npos,
                   "build host DLL wrapper should declare a shared stub-emission wrapper helper.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_emit_stub_return_shared(") == std::string::npos,
                   "build host DLL wrapper should apply stub-emission output at generated call-sites instead of a shared emitter helper.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_execute_placeholder_return_int(") != std::string::npos,
                   "build host DLL wrapper should declare a shared placeholder-return-int execution helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_execute_placeholder_return_int(placeholder_return_value)};") == std::string::npos,
                   "build host DLL wrapper should not hide placeholder-return-int execution inside the shared stub-emission helper.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_return_native_int(int value)") != std::string::npos,
                   "build host DLL wrapper should declare the DLL native-int return adapter for shared output application.");
            expect(wrapper_source.find("const auto stub_emission_wrapper =\n        copperfin_runtime_bridge_build_stub_emission_wrapper(") != std::string::npos,
                   "build host DLL wrapper should build a shared stub-emission wrapper before building the descriptor plan.");
            expect(wrapper_source.find("return copperfin_runtime_bridge_apply_stub_emission_output(\n        stub_emission_return_surface,") != std::string::npos,
                   "build host DLL wrapper should route DLL stub emission through the generated output-application call-site.");
            expect(wrapper_source.find("final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.native_return_surface") != std::string::npos,
                   "build host DLL wrapper should read the stub-emission return surface through the descriptor plan.");
            expect(wrapper_source.find("final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.return_adapter") != std::string::npos,
                   "build host DLL wrapper should read the stub-emission return adapter through the descriptor plan.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_native_int_return_surface(),") != std::string::npos,
                   "build host DLL wrapper should pass the DLL native int return-surface contract into the shared wrapper helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_return_native_int);") != std::string::npos,
                   "build host DLL wrapper should pass the DLL native-int return adapter into the shared wrapper helper.");
            expect(wrapper_source.find(", stub_emission_wrapper);") != std::string::npos,
                   "build host DLL wrapper should pass the built wrapper into the descriptor-plan builder.");
            expect(wrapper_source.find("const auto failure_policy = copperfin_build_runtime_bridge_failure_policy_plan(\n        interpretation_plan,\n        placeholder_return_binding.value_representation);") != std::string::npos,
                   "build host DLL wrapper should build the failure-policy plan from the enriched interpretation plan.");
            expect(wrapper_source.find("const auto response_validation = copperfin_build_runtime_bridge_response_validation_plan(\n        failure_policy);") != std::string::npos,
                   "build host DLL wrapper should build the response-validation plan from the enriched failure-policy plan.");
            expect(wrapper_source.find("const auto request_artifact = copperfin_build_runtime_bridge_request_artifact(\n        response_validation);") != std::string::npos,
                   "build host DLL wrapper should build the request artifact directly from the response-validation plan once the wrapper contract is upstream.");
            expect(wrapper_source.find("const auto request_write_plan = copperfin_build_runtime_bridge_request_write_plan(\n        request_artifact);") != std::string::npos,
                   "build host DLL wrapper should build the request-write plan directly from the request artifact once the wrapper contract is upstream.");
            expect(wrapper_source.find("const auto response_read_plan = copperfin_build_runtime_bridge_response_read_plan(\n        request_write_plan,\n        request_write_execution);") != std::string::npos,
                   "build host DLL wrapper should build the response-read plan from the request-write plan and executed write result.");
            expect(wrapper_source.find("const auto response_document =\n        copperfin_runtime_bridge_execute_read_response(response_read_plan);") != std::string::npos,
                   "build host DLL wrapper should execute the response-read plan before building the response artifact.");
            expect(wrapper_source.find("const auto response_artifact = copperfin_build_runtime_bridge_response_artifact(\n        response_read_plan,\n        response_document);") != std::string::npos,
                   "build host DLL wrapper should build the response artifact from the response-read plan and executed response document.");
            expect(wrapper_source.find("const auto response_parse_plan = copperfin_build_runtime_bridge_response_parse_plan(\n        response_artifact);") != std::string::npos,
                   "build host DLL wrapper should build the response-parse plan directly from the response artifact once the wrapper contract is upstream.");
            expect(wrapper_source.find("const auto parsed_response =\n        copperfin_runtime_bridge_execute_parse_response(response_parse_plan);") != std::string::npos,
                   "build host DLL wrapper should execute the response-parse plan before building the interpreted-result plan.");
            expect(wrapper_source.find("const auto interpreted_result_plan = copperfin_build_runtime_bridge_interpreted_result_plan(\n        response_parse_plan,\n        parsed_response);") != std::string::npos,
                   "build host DLL wrapper should build the interpreted-result plan from the response-parse plan and parsed response.");
            expect(wrapper_source.find("const auto interpreted_result =\n        copperfin_runtime_bridge_execute_interpreted_result(interpreted_result_plan);") != std::string::npos,
                   "build host DLL wrapper should execute the interpreted-result plan before building the native-return plan.");
            expect(wrapper_source.find("const auto native_return_plan = copperfin_build_runtime_bridge_native_return_plan(\n        result,\n        interpreted_result_plan,\n        interpreted_result);") != std::string::npos,
                   "build host DLL wrapper should build the native-return plan from the interpreted-result plan and interpreted result.");
            expect(wrapper_source.find("const auto native_return =\n        copperfin_runtime_bridge_execute_native_return(native_return_plan);") != std::string::npos,
                   "build host DLL wrapper should execute the native-return plan before building the outcome-selection plan.");
            expect(wrapper_source.find("const auto outcome_selection_plan = copperfin_build_runtime_bridge_outcome_selection_plan(\n        native_return_plan,\n        native_return);") != std::string::npos,
                   "build host DLL wrapper should build the outcome-selection plan from the native-return plan and native return.");
            expect(wrapper_source.find("const auto outcome_selection =\n        copperfin_runtime_bridge_execute_outcome_selection(outcome_selection_plan);") != std::string::npos,
                   "build host DLL wrapper should execute the outcome-selection plan before building the return-materialization plan.");
            expect(wrapper_source.find("const auto return_materialization_plan = copperfin_build_runtime_bridge_return_materialization_plan(\n        outcome_selection_plan,\n        outcome_selection);") != std::string::npos,
                   "build host DLL wrapper should build the return-materialization plan from the outcome-selection plan and outcome selection.");
            expect(wrapper_source.find("const auto return_materialization =\n        copperfin_runtime_bridge_execute_return_materialization(return_materialization_plan);") != std::string::npos,
                   "build host DLL wrapper should execute the return-materialization plan before building the return-emission plan.");
            expect(wrapper_source.find("const auto return_emission_plan = copperfin_build_runtime_bridge_return_emission_plan(\n        return_materialization_plan,\n        return_materialization);") != std::string::npos,
                   "build host DLL wrapper should build the return-emission plan from the return-materialization plan and materialized return.");
            expect(wrapper_source.find("const auto return_emission =\n        copperfin_runtime_bridge_execute_return_emission(return_emission_plan);") != std::string::npos,
                   "build host DLL wrapper should execute the return-emission plan before building the final-return-adoption plan.");
            expect(wrapper_source.find("const auto final_return_adoption_plan = copperfin_build_runtime_bridge_final_return_adoption_plan(\n        return_emission_plan,\n        return_emission,\n        copperfin_build_runtime_bridge_placeholder_return_statement(placeholder_return_binding));") != std::string::npos,
                   "build host DLL wrapper should build the final-return-adoption plan from the return-emission plan and emitted return.");
            expect(wrapper_source.find("const auto final_return_adoption =\n        copperfin_runtime_bridge_execute_final_return_adoption(final_return_adoption_plan);") != std::string::npos,
                   "build host DLL wrapper should execute the final-return-adoption plan before building the return-activation plan.");
            expect(wrapper_source.find("const auto return_activation_plan = copperfin_build_runtime_bridge_return_activation_plan(\n        final_return_adoption_plan,\n        final_return_adoption);") != std::string::npos,
                   "build host DLL wrapper should build the return-activation plan from the final-return-adoption plan and adopted return.");
            expect(wrapper_source.find("const auto return_activation =\n        copperfin_runtime_bridge_execute_return_activation(return_activation_plan);") != std::string::npos,
                   "build host DLL wrapper should execute the return-activation plan before building the stub-return plan.");
            expect(wrapper_source.find("const auto stub_return_plan = copperfin_build_runtime_bridge_stub_return_plan(\n        return_activation_plan,\n        return_activation);") != std::string::npos,
                   "build host DLL wrapper should build the stub-return plan from the return-activation plan and activated return.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_emit_stub_return(\n") == std::string::npos,
                   "build host DLL wrapper should no longer declare an output-specific stub-return wrapper helper once the plan carries the wrapper contract.");
            expect(wrapper_source.find("const auto descriptor = copperfin_build_runtime_bridge_descriptor(\"InitLibrary\"") != std::string::npos,
                   "build host DLL wrapper should build a bridge descriptor for InitLibrary");
            expect(wrapper_source.find("const auto descriptor = copperfin_build_runtime_bridge_descriptor(\"AddNumbers\"") != std::string::npos,
                   "build host DLL wrapper should build a bridge descriptor for AddNumbers");
            expect(wrapper_source.find("const auto invocation = copperfin_build_runtime_bridge_invocation(\n        descriptor);") != std::string::npos,
                   "build host DLL wrapper should build a bridge invocation from the descriptor");
            expect(wrapper_source.find("const auto call = copperfin_build_runtime_bridge_call(") != std::string::npos,
                   "build host DLL wrapper should build a bridge call from the invocation");
            expect(wrapper_source.find("const auto result = copperfin_build_runtime_bridge_result(\n        call,\n        placeholder_return_binding);") != std::string::npos,
                   "build host DLL wrapper should build a bridge result from the enriched call");
            expect(wrapper_source.find("const auto placeholder_return_binding =") != std::string::npos,
                   "build host DLL wrapper should build a shared placeholder return binding before building the result");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_placeholder_return_binding(\"int\")") != std::string::npos,
                   "build host DLL wrapper should build the DLL placeholder return binding through the shared helper");
            expect(wrapper_source.find("const auto launch_plan = copperfin_build_runtime_bridge_launch_plan(\n        result);") != std::string::npos,
                   "build host DLL wrapper should build a launch plan from the result");
            expect(wrapper_source.find("const auto observation_plan = copperfin_build_runtime_bridge_observation_plan(\n        launch_plan);") != std::string::npos,
                   "build host DLL wrapper should build an observation plan from the launch plan");
            expect(wrapper_source.find("const auto execution_plan = copperfin_build_runtime_bridge_execution_plan(\n        observation_plan);") != std::string::npos,
                   "build host DLL wrapper should build an execution plan from the observation plan");
            expect(wrapper_source.find("const auto transport_plan = copperfin_build_runtime_bridge_transport_plan(\n        execution_plan);") != std::string::npos,
                   "build host DLL wrapper should build a transport plan from the execution plan");
            expect(wrapper_source.find("const auto serialization_plan = copperfin_build_runtime_bridge_serialization_plan(\n        transport_plan);") != std::string::npos,
                   "build host DLL wrapper should build a serialization plan from the transport plan");
            expect(wrapper_source.find("const auto dispatch_plan = copperfin_build_runtime_bridge_dispatch_plan(\n        serialization_plan);") != std::string::npos,
                   "build host DLL wrapper should build a dispatch plan from the serialization plan");
            expect(wrapper_source.find("const auto dispatch_execution = copperfin_runtime_bridge_execute_dispatch(dispatch_plan);") != std::string::npos,
                   "build host DLL wrapper should route the dispatch plan through the shared dispatch-execution helper.");
            expect(wrapper_source.find("(void)dispatch_execution;") == std::string::npos,
                   "build host DLL wrapper should consume dispatch execution when launching the process.");
            expect(wrapper_source.find("const auto process_launch = request_write_execution\n        ? copperfin_runtime_bridge_launch_process(dispatch_execution)") != std::string::npos,
                   "build host DLL wrapper should route dispatch execution through the shared process-launch helper.");
            expect(wrapper_source.find("(void)process_launch;") == std::string::npos,
                   "build host DLL wrapper should consume process launch when evaluating host failure.");
            expect(wrapper_source.find("const auto payload_plan = copperfin_build_runtime_bridge_payload_plan(\n        dispatch_plan);") != std::string::npos,
                   "build host DLL wrapper should build a payload plan from the dispatch plan");
            expect(wrapper_source.find("const auto interpretation_plan = copperfin_build_runtime_bridge_interpretation_plan(") != std::string::npos,
                   "build host DLL wrapper should build an interpretation plan from the payload plan");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_native_int_return_surface());") != std::string::npos,
                   "build host DLL wrapper stub should route wrapper-return-surface through native-int return-surface helper");
            expect(wrapper_source.find("const auto failure_policy = copperfin_build_runtime_bridge_failure_policy_plan(") != std::string::npos,
                   "build host DLL wrapper should build a failure policy from the interpretation plan");
            expect(wrapper_source.find("const auto host_failure =") != std::string::npos,
                   "build host DLL wrapper should evaluate staged host failure from the process-launch helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_evaluate_host_failure(process_launch, failure_policy);") != std::string::npos,
                   "build host DLL wrapper should route process-launch output through the shared host-failure evaluation helper.");
            expect(wrapper_source.find("(void)host_failure;") == std::string::npos,
                   "build host DLL wrapper should consume host-failure evaluation when evaluating missing response.");
            expect(wrapper_source.find("const auto response_validation = copperfin_build_runtime_bridge_response_validation_plan(") != std::string::npos,
                   "build host DLL wrapper should build a response-validation plan from the failure policy");
            expect(wrapper_source.find("const auto request_artifact = copperfin_build_runtime_bridge_request_artifact(") != std::string::npos,
                   "build host DLL wrapper should build a request artifact from the response validation plan");
            expect(wrapper_source.find("const auto request_write_plan = copperfin_build_runtime_bridge_request_write_plan(") != std::string::npos,
                   "build host DLL wrapper should build a request write plan from the request artifact");
            expect(wrapper_source.find("const auto request_write_execution =\n        copperfin_runtime_bridge_execute_write_request(request_write_plan);") != std::string::npos,
                   "build host DLL wrapper should execute the request-write plan through the shared helper.");
            expect(wrapper_source.find("(void)request_write_execution;") == std::string::npos,
                   "build host DLL wrapper should consume request-write execution when planning response reads.");
            expect(wrapper_source.find("const auto response_read_plan = copperfin_build_runtime_bridge_response_read_plan(") != std::string::npos,
                   "build host DLL wrapper should build a response read plan from the request write plan");
            expect(wrapper_source.find("const auto missing_response =") != std::string::npos,
                   "build host DLL wrapper should evaluate staged missing-response policy from the host-failure and response-read helpers.");
            expect(wrapper_source.find("copperfin_runtime_bridge_evaluate_missing_response(\n            host_failure,\n            response_read_plan,\n            response_document);") != std::string::npos,
                   "build host DLL wrapper should route host-failure output and the response document through the shared missing-response evaluation helper.");
            expect(wrapper_source.find("(void)missing_response;") == std::string::npos,
                   "build host DLL wrapper should consume missing-response evaluation when evaluating response validation.");
            expect(wrapper_source.find("const auto response_validation_evaluation =") != std::string::npos,
                   "build host DLL wrapper should evaluate staged response-validation policy from the missing-response, validation, and response-document helpers.");
            expect(wrapper_source.find("copperfin_runtime_bridge_evaluate_response_validation(\n            missing_response,\n            response_validation,\n            response_document);") != std::string::npos,
                   "build host DLL wrapper should route missing-response output and the response document through the shared response-validation evaluation helper.");
            expect(wrapper_source.find("(void)response_validation_evaluation;") == std::string::npos,
                   "build host DLL wrapper should consume response-validation evaluation when admitting response parsing.");
            expect(wrapper_source.find("const auto response_artifact = copperfin_build_runtime_bridge_response_artifact(") != std::string::npos,
                   "build host DLL wrapper should build a response artifact from the response read plan");
            expect(wrapper_source.find("const auto response_parse_plan = copperfin_build_runtime_bridge_response_parse_plan(") != std::string::npos,
                   "build host DLL wrapper should build a response parse plan from the response artifact");
            expect(wrapper_source.find("const auto response_parse_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged response parsing from the response-validation evaluation and parse plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_response_parse(response_validation_evaluation, response_parse_plan);") != std::string::npos,
                   "build host DLL wrapper should route response-validation evaluation through the shared response-parse admission helper.");
            expect(wrapper_source.find("(void)response_parse_admission;") == std::string::npos,
                   "build host DLL wrapper should consume response-parse admission when admitting interpreted result.");
            expect(wrapper_source.find("const auto interpreted_result_plan = copperfin_build_runtime_bridge_interpreted_result_plan(") != std::string::npos,
                   "build host DLL wrapper should build an interpreted result plan from the response parse plan");
            expect(wrapper_source.find("const auto interpreted_result_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged interpreted-result selection from the response-parse admission and interpreted-result plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_interpreted_result(response_parse_admission, interpreted_result_plan);") != std::string::npos,
                   "build host DLL wrapper should route response-parse admission through the shared interpreted-result admission helper.");
            expect(wrapper_source.find("(void)interpreted_result_admission;") == std::string::npos,
                   "build host DLL wrapper should consume interpreted-result admission when admitting native return.");
            expect(wrapper_source.find("const auto native_return_plan = copperfin_build_runtime_bridge_native_return_plan(") != std::string::npos,
                   "build host DLL wrapper should build a native return plan from the interpreted result plan");
            expect(wrapper_source.find("const auto native_return_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged native-return selection from the interpreted-result admission and native-return plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_native_return(interpreted_result_admission, native_return_plan);") != std::string::npos,
                   "build host DLL wrapper should route interpreted-result admission through the shared native-return admission helper.");
            expect(wrapper_source.find("(void)native_return_admission;") == std::string::npos,
                   "build host DLL wrapper should consume native-return admission when admitting outcome selection.");
            expect(wrapper_source.find("const auto success_value_representation = interpreted_result.selected_return_value_representation;") != std::string::npos,
                   "build host DLL wrapper should route interpreted response return values into native-return planning");
            expect(wrapper_source.find("const int success_int_value = copperfin_parse_runtime_bridge_int_value_representation(\n        success_value_representation);") != std::string::npos,
                   "build host DLL wrapper should parse the typed success integer value from the interpreted response representation");
            expect(wrapper_source.find("const int fallback_int_value = copperfin_parse_runtime_bridge_int_value_representation(") != std::string::npos,
                   "build host DLL wrapper should parse the typed fallback integer value from the fallback representation");
            expect(wrapper_source.find("const auto int_value_representation = std::to_string(int_value);") != std::string::npos,
                   "build host DLL wrapper should build typed return statements from parsed integer values");
            expect(wrapper_source.find("native_return_plan.success_int_value,") != std::string::npos,
                   "build host DLL wrapper should materialize success returns from the parsed success integer value");
            expect(wrapper_source.find("native_return_plan.fallback_int_value,") != std::string::npos,
                   "build host DLL wrapper should materialize fallback returns from the parsed fallback integer value");
            expect(wrapper_source.find("\"else { \" + return_materialization.fallback_return_statement + \" }\";") != std::string::npos,
                   "build host DLL wrapper should record an explicit fallback else-branch statement");
            expect(wrapper_source.find("success_branch_statement + \" \" + fallback_branch_statement;") != std::string::npos,
                   "build host DLL wrapper should compose the emitted return block from the explicit branch statements");
            expect(wrapper_source.find("const auto active_return_block = final_return_adoption.adopted_return_block;") != std::string::npos,
                   "build host DLL wrapper should seed the inactive active-return block from the adopted return block");
            expect(wrapper_source.find(": return_activation.active_return_block;") != std::string::npos,
                   "build host DLL wrapper should route the deferred stub-return block through the activation metadata");
            expect(wrapper_source.find("int placeholder_fallback_int_value = -1;") != std::string::npos,
                   "build host DLL wrapper should record placeholder fallback integers in the stub-return plan");
            expect(wrapper_source.find("std::string placeholder_fallback_value_representation;") != std::string::npos,
                   "build host DLL wrapper should record placeholder fallback representations in the stub-return plan");
            expect(wrapper_source.find("bool emits_placeholder_return = true;") != std::string::npos,
                   "build host DLL wrapper should record placeholder-emission flags in the placeholder-return-value plan");
            expect(wrapper_source.find("std::string emitted_return_statement;") != std::string::npos,
                   "build host DLL wrapper should record placeholder emitted-return statements in the placeholder-return-value plan");
            expect(wrapper_source.find("std::string deferred_return_block;") != std::string::npos,
                   "build host DLL wrapper should record deferred return blocks in the placeholder-return-value plan");
            expect(wrapper_source.find("stub_return.emits_placeholder_return,") != std::string::npos,
                   "build host DLL wrapper should feed placeholder-emission flags from stub-return metadata");
            expect(wrapper_source.find("stub_return.emitted_return_statement,") != std::string::npos,
                   "build host DLL wrapper should feed emitted placeholder-return statements from stub-return metadata");
            expect(wrapper_source.find("stub_return.deferred_return_block,") != std::string::npos,
                   "build host DLL wrapper should feed deferred return blocks from stub-return metadata");
            expect(wrapper_source.find("stub_return.activation_mode,") != std::string::npos,
                   "build host DLL wrapper should feed activation modes from stub-return metadata");
            expect(wrapper_source.find("stub_return.adoption_mode,") != std::string::npos,
                   "build host DLL wrapper should feed adoption modes from stub-return metadata");
            expect(wrapper_source.find("stub_return.keeps_placeholder_return_active,") != std::string::npos,
                   "build host DLL wrapper should feed placeholder-helper active-policy booleans from stub-return metadata");
            expect(wrapper_source.find("stub_return.adopts_placeholder_replacement,") != std::string::npos,
                   "build host DLL wrapper should feed placeholder-helper replacement-policy booleans from stub-return metadata");
            expect(wrapper_source.find("stub_return.placeholder_fallback_int_value,") != std::string::npos,
                   "build host DLL wrapper should feed placeholder fallback integers from stub-return metadata");
            expect(wrapper_source.find("stub_return.placeholder_fallback_value_representation};") != std::string::npos,
                   "build host DLL wrapper should feed placeholder fallback representations from stub-return metadata");
            expect(wrapper_source.find("const bool keeps_placeholder_return_active =") != std::string::npos,
                   "build host DLL wrapper should derive placeholder-helper active-policy booleans upstream");
            expect(wrapper_source.find("const bool adopts_placeholder_replacement =") != std::string::npos,
                   "build host DLL wrapper should derive placeholder-helper replacement-policy booleans upstream");
            expect(wrapper_source.find("(void)placeholder_return_value.emitted_return_statement;") == std::string::npos,
                   "build host DLL wrapper should not have the int helper consume placeholder-value return-statement contracts");
            expect(wrapper_source.find("(void)placeholder_return_value.deferred_return_block;") == std::string::npos,
                   "build host DLL wrapper should not have the int helper consume placeholder-value deferred return-block contracts");
            expect(wrapper_source.find("placeholder_return_int_admission.should_return_int") != std::string::npos,
                   "build host DLL wrapper should have the int helper consume the admitted int-return policy boolean");
            expect(wrapper_source.find("placeholder_return_int_admission.selected_int_value") != std::string::npos,
                   "build host DLL wrapper should have the int helper consume the admitted selected integer value");
            expect(wrapper_source.find("const auto outcome_selection_plan = copperfin_build_runtime_bridge_outcome_selection_plan(") != std::string::npos,
                   "build host DLL wrapper should build an outcome selection plan from the native return plan");
            expect(wrapper_source.find("const auto outcome_selection_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged outcome selection from the native-return admission and outcome-selection plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_outcome_selection(native_return_admission, outcome_selection_plan);") != std::string::npos,
                   "build host DLL wrapper should route native-return admission through the shared outcome-selection admission helper.");
            expect(wrapper_source.find("(void)outcome_selection_admission;") == std::string::npos,
                   "build host DLL wrapper should consume outcome-selection admission when admitting return materialization.");
            expect(wrapper_source.find("const auto return_materialization_plan = copperfin_build_runtime_bridge_return_materialization_plan(") != std::string::npos,
                   "build host DLL wrapper should build a return materialization plan from the outcome selection plan");
            expect(wrapper_source.find("const auto return_materialization_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged return materialization from the outcome-selection admission and return-materialization plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_return_materialization(outcome_selection_admission, return_materialization_plan);") != std::string::npos,
                   "build host DLL wrapper should route outcome-selection admission through the shared return-materialization admission helper.");
            expect(wrapper_source.find("(void)return_materialization_admission;") == std::string::npos,
                   "build host DLL wrapper should consume return-materialization admission when admitting return emission.");
            expect(wrapper_source.find("const auto return_emission_plan = copperfin_build_runtime_bridge_return_emission_plan(") != std::string::npos,
                   "build host DLL wrapper should build a return emission plan from the return materialization plan");
            expect(wrapper_source.find("const auto return_emission_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged return emission from the return-materialization admission and return-emission plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_return_emission(return_materialization_admission, return_emission_plan);") != std::string::npos,
                   "build host DLL wrapper should route return-materialization admission through the shared return-emission admission helper.");
            expect(wrapper_source.find("(void)return_emission_admission;") == std::string::npos,
                   "build host DLL wrapper should consume return-emission admission when admitting final-return adoption.");
            expect(wrapper_source.find("const auto final_return_adoption_plan = copperfin_build_runtime_bridge_final_return_adoption_plan(") != std::string::npos,
                   "build host DLL wrapper should build a final return adoption plan from the return emission plan");
            expect(wrapper_source.find("const auto final_return_adoption_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged final return adoption from the return-emission admission and final-return-adoption plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_final_return_adoption(return_emission_admission, final_return_adoption_plan);") != std::string::npos,
                   "build host DLL wrapper should route return-emission admission through the shared final-return-adoption admission helper.");
            expect(wrapper_source.find("(void)final_return_adoption_admission;") == std::string::npos,
                   "build host DLL wrapper should consume final-return-adoption admission when admitting return activation.");
            expect(wrapper_source.find("const auto return_activation_plan = copperfin_build_runtime_bridge_return_activation_plan(") != std::string::npos,
                   "build host DLL wrapper should build a return activation plan from the final return adoption plan");
            expect(wrapper_source.find("const auto return_activation_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged return activation from the final-return-adoption admission and return-activation plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_return_activation(final_return_adoption_admission, return_activation_plan);") != std::string::npos,
                   "build host DLL wrapper should route final-return-adoption admission through the shared return-activation admission helper.");
            expect(wrapper_source.find("(void)return_activation_admission;") == std::string::npos,
                   "build host DLL wrapper should consume return-activation admission when admitting stub-return routing.");
            expect(wrapper_source.find("const auto stub_return_plan = copperfin_build_runtime_bridge_stub_return_plan(") != std::string::npos,
                   "build host DLL wrapper should build a stub return plan from the return activation plan");
            expect(wrapper_source.find("const auto stub_return_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged stub-return routing from the return-activation admission and stub-return plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_stub_return(return_activation_admission, stub_return_plan);") != std::string::npos,
                   "build host DLL wrapper should route return-activation admission through the shared stub-return admission helper.");
            expect(wrapper_source.find("(void)stub_return_admission;") == std::string::npos,
                   "build host DLL wrapper should consume stub-return admission when admitting placeholder-return-value routing.");
            expect(wrapper_source.find("const auto placeholder_return_value_plan = copperfin_build_runtime_bridge_placeholder_return_value_plan(") != std::string::npos,
                   "build host DLL wrapper should build a placeholder-return-value plan from the stub return plan");
            expect(wrapper_source.find("const auto stub_return =\n        copperfin_runtime_bridge_execute_stub_return(stub_return_plan);") != std::string::npos,
                   "build host DLL wrapper should execute the stub-return plan before building the placeholder-return-value plan.");
            expect(wrapper_source.find("const auto placeholder_return_value_plan = copperfin_build_runtime_bridge_placeholder_return_value_plan(\n        stub_return_plan,\n        stub_return);") != std::string::npos,
                   "build host DLL wrapper should build the placeholder-return-value plan from the stub-return plan and stub return.");
            expect(wrapper_source.find("const auto placeholder_return_value_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged placeholder-return-value routing from the stub-return admission and placeholder-return-value plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_placeholder_return_value(stub_return_admission, placeholder_return_value_plan);") != std::string::npos,
                   "build host DLL wrapper should route stub-return admission through the shared placeholder-return-value admission helper.");
            expect(wrapper_source.find("(void)placeholder_return_value_admission;") == std::string::npos,
                   "build host DLL wrapper should consume placeholder-return-value admission when admitting placeholder-return-int routing.");
            expect(wrapper_source.find("const auto placeholder_return_value =\n        copperfin_runtime_bridge_execute_placeholder_return_value(placeholder_return_value_plan);") != std::string::npos,
                   "build host DLL wrapper should execute the placeholder-return-value plan before shared stub emission.");
            expect(wrapper_source.find("const auto placeholder_return_int_admission =") != std::string::npos,
                   "build host DLL wrapper should admit staged placeholder-return-int routing from the placeholder-return-value admission and placeholder-return-value plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_placeholder_return_int(placeholder_return_value_admission, placeholder_return_value_plan);") != std::string::npos,
                   "build host DLL wrapper should route placeholder-return-value admission through the shared placeholder-return-int admission helper.");
            expect(wrapper_source.find("(void)placeholder_return_int_admission;") == std::string::npos,
                   "build host DLL wrapper should consume placeholder-return-int admission when executing placeholder-return-int output.");
            expect(wrapper_source.find("const auto placeholder_return_int =\n        copperfin_runtime_bridge_execute_placeholder_return_int(placeholder_return_int_admission);") != std::string::npos,
                   "build host DLL wrapper should execute placeholder-return-int routing from the admitted surface before stub emission.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_stub_emission(placeholder_return_int_admission, placeholder_return_int);") != std::string::npos,
                   "build host DLL wrapper should route explicit placeholder-return-int output through the shared stub-emission admission helper.");
            expect(wrapper_source.find("(void)stub_emission_admission;") == std::string::npos,
                   "build host DLL wrapper should consume stub-emission admission when executing stub emission.");
            expect(wrapper_source.find("const auto stub_emission =\n        copperfin_runtime_bridge_execute_stub_emission(stub_emission_admission);") != std::string::npos,
                   "build host DLL wrapper should execute stub emission from explicit admission output.");
            expect(wrapper_source.find("const auto stub_emission_return_surface =\n        copperfin_runtime_bridge_build_stub_emission_return_surface(") != std::string::npos,
                   "build host DLL wrapper should build the stub-emission return surface before direct output application.");
            expect(wrapper_source.find("native_return_plan.fallback_int_value") != std::string::npos,
                   "build host DLL wrapper should propagate the typed native fallback integer value downstream");
            expect(wrapper_source.find("return copperfin_runtime_bridge_apply_stub_emission_output(\n        stub_emission_return_surface,\n        placeholder_return_value_plan.stub_return_plan.return_activation_plan.final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.return_adapter);") != std::string::npos,
                   "build host DLL wrapper should route the placeholder return through the generated stub-emission output-application call-site.");
            expect(wrapper_source.find("\"--library-export\"") != std::string::npos,
                   "build host DLL wrapper should encode the export name into the bridge invocation plan");
            expect(wrapper_source.find("(void)tcMode;") == std::string::npos,
                   "build host DLL wrapper should consume DLL arguments through bridge call bindings.");
            expect(wrapper_source.find("{\"tcMode\", std::to_string(tcMode), \"int\"}") != std::string::npos,
                   "build host DLL wrapper should preserve the DLL placeholder argument binding");
            expect(wrapper_source.find(", stub_emission_wrapper);") != std::string::npos,
                   "build host DLL wrapper should feed the bridge result from the enriched descriptor and shared placeholder return binding");
            expect(wrapper_source.find("{copperfin_runtime_bridge_library_export_env_var(), result.call.invocation.descriptor.export_name}") != std::string::npos,
                   "build host DLL wrapper should preserve launch environment export metadata");
            expect(wrapper_source.find("std::string(export_name) + copperfin_runtime_bridge_stdout_log_suffix()") != std::string::npos,
                   "build host DLL wrapper should derive stdout observation paths");
            expect(wrapper_source.find("std::string(export_name) + copperfin_runtime_bridge_stderr_log_suffix()") != std::string::npos,
                   "build host DLL wrapper should derive stderr observation paths");
            expect(wrapper_source.find("observation_plan.launch_plan.result.call.invocation.descriptor.runtime_host_path") != std::string::npos,
                   "build host DLL wrapper should preserve the runtime-host executable path in the execution plan");
            expect(wrapper_source.find("observation_plan.launch_plan.result.call.invocation.arguments") != std::string::npos,
                   "build host DLL wrapper should preserve the bridge invocation arguments in the execution plan");
            expect(wrapper_source.find("artifact_stem + copperfin_runtime_bridge_request_artifact_suffix()") != std::string::npos,
                   "build host DLL wrapper should derive request transport paths");
            expect(wrapper_source.find("artifact_stem + copperfin_runtime_bridge_response_artifact_suffix()") != std::string::npos,
                   "build host DLL wrapper should derive response transport paths");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_media_type_value()") != std::string::npos,
                   "build host DLL wrapper should declare a shared request serialization media-type helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_media_type_value()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response serialization media-type helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_schema_version_value()") != std::string::npos,
                   "build host DLL wrapper should declare a shared serialization schema-version helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_path_argument_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared request-path dispatch helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_path_argument_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-path dispatch helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_media_type_argument_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared request-media-type dispatch helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_media_type_argument_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response-media-type dispatch helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_schema_version_argument_name()") != std::string::npos,
                   "build host DLL wrapper should declare a shared schema-version dispatch helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_media_type_value()") != std::string::npos,
                   "build host DLL wrapper should route the request serialization media type through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_media_type_value()") != std::string::npos,
                   "build host DLL wrapper should route the response serialization media type through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_schema_version_value()") != std::string::npos,
                   "build host DLL wrapper should route the serialization schema version through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_path_argument_name()") != std::string::npos,
                   "build host DLL wrapper should route the request-path dispatch argument through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_path_argument_name()") != std::string::npos,
                   "build host DLL wrapper should route the response-path dispatch argument through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_media_type_argument_name()") != std::string::npos,
                   "build host DLL wrapper should route the request-media-type dispatch argument through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_media_type_argument_name()") != std::string::npos,
                   "build host DLL wrapper should route the response-media-type dispatch argument through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_schema_version_argument_name()") != std::string::npos,
                   "build host DLL wrapper should route the schema-version dispatch argument through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_payload_shape_name()") != std::string::npos,
                   "build host DLL wrapper should route the request payload shape through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_payload_shape_name()") != std::string::npos,
                   "build host DLL wrapper should route the response payload shape through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_export_name_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the export-name field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_routine_kind_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the routine-kind field through the shared helper");
            expect(wrapper_source.find("copperfin_escape_runtime_bridge_json_string(call.invocation.descriptor.routine_kind)") != std::string::npos,
                   "build host DLL wrapper should serialize routine-kind metadata into the request document");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_source_path_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the source-path field through the shared helper");
            expect(wrapper_source.find("copperfin_escape_runtime_bridge_json_string(call.invocation.descriptor.source_path)") != std::string::npos,
                   "build host DLL wrapper should serialize source-path metadata into the request document");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_source_line_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the source-line field through the shared helper");
            expect(wrapper_source.find("call.invocation.descriptor.source_line") != std::string::npos,
                   "build host DLL wrapper should serialize source-line metadata into the request document");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_declaration_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the parameter-declaration field through the shared helper");
            expect(wrapper_source.find("copperfin_escape_runtime_bridge_json_string(call.invocation.descriptor.parameter_declaration_kind)") != std::string::npos,
                   "build host DLL wrapper should serialize parameter-declaration metadata into the request document");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_names_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the parameter-names field through the shared helper");
            expect(wrapper_source.find("copperfin_escape_runtime_bridge_json_string(call.invocation.descriptor.parameter_names)") != std::string::npos,
                   "build host DLL wrapper should serialize parameter-name metadata into the request document");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_count_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the parameter-count field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_schema_version_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the schema-version field through the shared helper");
            expect(wrapper_source.find("payload_plan.dispatch_plan.serialization_plan.schema_version") != std::string::npos,
                   "build host DLL wrapper should serialize schema-version metadata into the request document");
            expect(wrapper_source.find("{copperfin_build_runtime_bridge_export_name_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_routine_kind_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_source_path_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_source_line_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_parameter_declaration_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_parameter_names_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_parameter_count_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_schema_version_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_parameters_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_request_media_type_field_name()}") != std::string::npos,
                   "build host DLL wrapper should declare descriptor metadata in the request-field contract");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameters_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the parameters field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_media_type_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the request-media-type field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_fields_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the request-fields contract through the shared helper");
            expect(wrapper_source.find("payload_plan.request_fields.size()") != std::string::npos,
                   "build host DLL wrapper should serialize the request-field contract list");
            expect(wrapper_source.find("payload_plan.request_fields[index]") != std::string::npos,
                   "build host DLL wrapper should serialize each request-field contract item");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_expected_response_media_type_field_name()") != std::string::npos,
                   "build host DLL wrapper should route expected response media-type through the shared helper");
            expect(wrapper_source.find("payload_plan.dispatch_plan.serialization_plan.response_media_type") != std::string::npos,
                   "build host DLL wrapper should serialize expected response media type into the request document");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_fields_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the response-fields contract through the shared helper");
            expect(wrapper_source.find("payload_plan.response_fields.size()") != std::string::npos,
                   "build host DLL wrapper should serialize the response-field contract list");
            expect(wrapper_source.find("payload_plan.response_fields[index]") != std::string::npos,
                   "build host DLL wrapper should serialize each response-field contract item");
            expect(wrapper_source.find("{copperfin_build_runtime_bridge_status_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_return_value_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_response_media_type_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_schema_version_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_diagnostics_field_name()}") != std::string::npos,
                   "build host DLL wrapper should declare schema version in the response-field contract");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_return_value_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the response value field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_media_type_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the response-media-type field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_status_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the response status field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_diagnostics_field_name()") != std::string::npos,
                   "build host DLL wrapper should route the response diagnostics field through the shared helper");
            expect(wrapper_source.find("        copperfin_build_runtime_bridge_native_int_return_surface());") != std::string::npos,
                   "build host DLL wrapper should preserve the DLL wrapper return surface");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_failure_diagnostics_value()") != std::string::npos,
                   "build host DLL wrapper should declare the diagnostics fallback policy through the shared token helper");
            expect(wrapper_source.find("placeholder_return_binding.value_representation);") != std::string::npos,
                   "build host DLL wrapper should declare the fallback return value policy through the shared binding");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_placeholder_return_statement(placeholder_return_binding)") != std::string::npos,
                   "build host DLL wrapper should derive the placeholder return statement from the shared binding helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_success_status_value()") != std::string::npos,
                   "build host DLL wrapper should declare the success-status expectation through the shared token helper");
            expect(wrapper_source.find("std::string request_document;") != std::string::npos,
                   "build host DLL wrapper should record the request document payload.");
            expect(wrapper_source.find("std::filesystem::path target_path;") != std::string::npos,
                   "build host DLL wrapper should record the request write target path.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_write_mode()") != std::string::npos,
                   "build host DLL wrapper should declare a shared request write-mode helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_write_mode()") != std::string::npos,
                   "build host DLL wrapper should route the request write mode through the shared helper.");
            expect(wrapper_source.find("std::filesystem::path source_path;") != std::string::npos,
                   "build host DLL wrapper should record the response read source path.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_read_mode()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response read-mode helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_read_mode()") != std::string::npos,
                   "build host DLL wrapper should route the response read mode through the shared helper.");
            expect(wrapper_source.find("std::string response_document;") != std::string::npos,
                   "build host DLL wrapper should record the response document payload.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_empty_response_document()") != std::string::npos,
                   "build host DLL wrapper should declare a shared empty response-document helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_empty_response_document()") != std::string::npos,
                   "build host DLL wrapper should route the empty response-document token through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_parse_kind()") != std::string::npos,
                   "build host DLL wrapper should declare a shared response parse-kind helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_parse_kind()") != std::string::npos,
                   "build host DLL wrapper should route the response parse kind through the shared helper.");
            expect(wrapper_source.find("std::string wrapper_return_surface;") != std::string::npos,
                   "build host DLL wrapper should record the wrapper return surface.");
            expect(wrapper_source.find("std::string native_return_surface;") != std::string::npos,
                   "build host DLL wrapper should record the native return surface.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_success_comparator_token()") != std::string::npos,
                   "build host DLL wrapper should declare a shared success-comparator helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_fallback_comparator_token()") != std::string::npos,
                   "build host DLL wrapper should declare a shared fallback-comparator helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_success_comparator_token()") != std::string::npos,
                   "build host DLL wrapper should route the success comparator through the shared helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_fallback_comparator_token()") != std::string::npos,
                   "build host DLL wrapper should route the fallback comparator through the shared helper.");
            expect(wrapper_source.find("std::string success_condition;") != std::string::npos,
                   "build host DLL wrapper should record the outcome success condition.");
            expect(wrapper_source.find("std::string success_return_statement;") != std::string::npos,
                   "build host DLL wrapper should record the success return statement.");
            expect(wrapper_source.find("std::string emitted_return_block;") != std::string::npos,
                   "build host DLL wrapper should record the emitted return block.");
            expect(wrapper_source.find("std::string placeholder_return_statement;") != std::string::npos,
                   "build host DLL wrapper should record the placeholder return statement.");
            expect(wrapper_source.find("bool activates_adopted_return = false;") != std::string::npos,
                   "build host DLL wrapper should record the inactive return-activation flag.");
            expect(wrapper_source.find("bool emits_placeholder_return = true;") != std::string::npos,
                   "build host DLL wrapper should record the placeholder-emission flag.");
            expect(wrapper_source.find("int fallback_int_value = -1;") != std::string::npos,
                   "build host DLL wrapper should record the placeholder fallback integer value.");
            expect(wrapper_source.find("int success_int_value = -1;") != std::string::npos,
                   "build host DLL wrapper should record the typed native success integer value.");
            expect(wrapper_cmake.find("target_link_libraries(LibraryDemo PRIVATE dl)") != std::string::npos,
                   "build host DLL wrapper CMake should link dl on supported Unix hosts");
        }

        if (extension == "fll") {
            expect(manifest_text.find("fll_loader_entrypoint=FoxInfo") != std::string::npos,
                   "build host manifest should record the FLL loader entrypoint");
            expect(manifest_text.find("fll_registration_symbol=_FoxTable") != std::string::npos,
                   "build host manifest should record the FLL registration symbol");
            expect(manifest_text.find("fll_callable_signature=ParamBlk*") != std::string::npos,
                   "build host manifest should record the FLL callable signature");
            expect(manifest_text.find("fll_default_return_helper=_RetInt") != std::string::npos,
                   "build host manifest should record the FLL default return helper");
            expect(lines_with_prefix(manifest_text, "library_function_").empty(),
                   "build host runtime manifest should omit FLL library-function inventory");
        }
    }

    if (fs::exists(expected_output) && native_symbol_dump_is_available()) {
        std::string symbol_error;
        const std::set<std::string> exported_symbols = read_native_exported_symbols(expected_output, symbol_error);
        if (exported_symbols.empty() && !symbol_error.empty()) {
            std::cerr << "FAIL: " << symbol_error << "\n";
        }

        const fs::path module_definition_path = value_for_key(process.stdout_text, "module.definition");
        const std::set<std::string> declared_module_symbols = read_module_definition_exports(module_definition_path);
        expect(exported_symbols == declared_module_symbols,
               "build host should preserve the module-definition export contract for " + extension + " outputs");

        if (extension == "dll") {
            const fs::path library_api_manifest_path = value_for_key(process.stdout_text, "library.api.manifest");
            const std::set<std::string> declared_api_symbols = read_library_api_declared_symbols(library_api_manifest_path);
            expect(exported_symbols == declared_api_symbols,
                   "build host should preserve the dedicated DLL API-manifest export contract");
            const std::string api_manifest = read_text(library_api_manifest_path);
            expect(api_manifest.find("output_kind=dll") != std::string::npos,
                   "build host DLL API manifest should declare the DLL output kind");
            expect(api_manifest.find("callable_convention=vfp_declare_default") != std::string::npos,
                   "build host DLL API manifest should declare the VFP DLL calling convention");
        }

        if (extension == "fll") {
            const fs::path fll_api_manifest_path = value_for_key(process.stdout_text, "fll.api.manifest");
            const std::set<std::string> declared_api_symbols = read_fll_api_declared_symbols(fll_api_manifest_path);
            expect(exported_symbols == declared_api_symbols,
                   "build host should preserve the API-manifest export contract for fll outputs");
            const std::string api_manifest = read_text(fll_api_manifest_path);
            expect(manifest_value_for_key(manifest_text, "native_wrapper_source_path").empty(),
                   "build host FLL runtime manifest should omit the native-wrapper source path");
            expect(manifest_value_for_key(manifest_text, "native_wrapper_cmake_path").empty(),
                   "build host FLL runtime manifest should omit the native-wrapper CMake path");
            const fs::path wrapper_source_path = manifest_path_for_key(debug_manifest_text, "native_wrapper_source_path");
            const std::string wrapper_source = wrapper_source_path.empty() ? std::string{} : read_text(wrapper_source_path);
            const fs::path wrapper_cmake_path = manifest_path_for_key(debug_manifest_text, "native_wrapper_cmake_path");
            const std::string wrapper_cmake = wrapper_cmake_path.empty() ? std::string{} : read_text(wrapper_cmake_path);
            expect(api_manifest.find("registration_symbol=_FoxTable") != std::string::npos,
                   "build host FLL manifest should declare the FoxTable registration symbol");
            expect(api_manifest.find("callable_signature=ParamBlk*") != std::string::npos,
                   "build host FLL manifest should declare the ParamBlk callable signature");
            expect(api_manifest.find("default_return_helper=_RetInt") != std::string::npos,
                   "build host FLL manifest should declare the default return helper");
            expect(wrapper_source.find("static std::filesystem::path copperfin_wrapper_module_path(void* symbol_address)") != std::string::npos,
                   "build host FLL wrapper should derive its loaded module path");
            expect(wrapper_source.find("static std::filesystem::path copperfin_runtime_manifest_path(void* symbol_address)") != std::string::npos,
                   "build host FLL wrapper should derive a sibling manifest path");
            expect(wrapper_source.find("static std::filesystem::path copperfin_runtime_host_path(void* symbol_address)") != std::string::npos,
                   "build host FLL wrapper should derive a sibling runtime-host path");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeDescriptor") != std::string::npos,
                   "build host FLL wrapper should declare a shared bridge-descriptor surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeDescriptor copperfin_build_runtime_bridge_descriptor(") != std::string::npos,
                   "build host FLL wrapper should declare a bridge-descriptor helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeInvocation") != std::string::npos,
                   "build host FLL wrapper should declare a shared bridge-invocation surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeInvocation copperfin_build_runtime_bridge_invocation(") != std::string::npos,
                   "build host FLL wrapper should declare a bridge-invocation helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_manifest_flag()") != std::string::npos,
                   "build host FLL wrapper should declare manifest flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_manifest_flag()") != std::string::npos,
                   "build host FLL wrapper should route manifest flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_library_export_flag()") != std::string::npos,
                   "build host FLL wrapper should declare library-export flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_library_export_flag()") != std::string::npos,
                   "build host FLL wrapper should route library-export flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_routine_kind_flag()") != std::string::npos,
                   "build host FLL wrapper should declare routine-kind flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_routine_kind_flag()") != std::string::npos,
                   "build host FLL wrapper should route routine-kind flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_source_path_flag()") != std::string::npos,
                   "build host FLL wrapper should declare source-path flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_source_path_flag()") != std::string::npos,
                   "build host FLL wrapper should route source-path flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_source_line_flag()") != std::string::npos,
                   "build host FLL wrapper should declare source-line flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_source_line_flag()") != std::string::npos,
                   "build host FLL wrapper should route source-line flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_parameter_declaration_flag()") != std::string::npos,
                   "build host FLL wrapper should declare parameter-declaration flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_parameter_declaration_flag()") != std::string::npos,
                   "build host FLL wrapper should route parameter-declaration flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_parameter_names_flag()") != std::string::npos,
                   "build host FLL wrapper should declare parameter-names flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_parameter_names_flag()") != std::string::npos,
                   "build host FLL wrapper should route parameter-names flag through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_parameter_count_flag()") != std::string::npos,
                   "build host FLL wrapper should declare parameter-count flag helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_parameter_count_flag()") != std::string::npos,
                   "build host FLL wrapper should route parameter-count flag through helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeParameter") != std::string::npos,
                   "build host FLL wrapper should declare a bridge-parameter surface");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeCall") != std::string::npos,
                   "build host FLL wrapper should declare a bridge-call surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeCall copperfin_build_runtime_bridge_call(") != std::string::npos,
                   "build host FLL wrapper should declare a bridge-call helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeReturn") != std::string::npos,
                   "build host FLL wrapper should declare a return-binding surface");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeResult") != std::string::npos,
                   "build host FLL wrapper should declare a bridge-result surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResult copperfin_build_runtime_bridge_result(") != std::string::npos,
                   "build host FLL wrapper should declare a bridge-result helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturn copperfin_build_runtime_bridge_placeholder_return_binding(") != std::string::npos,
                   "build host FLL wrapper should declare a shared placeholder return-binding helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeEnvironmentVariable") != std::string::npos,
                   "build host FLL wrapper should declare a launch-environment surface");
            expect(wrapper_source.find("std::vector<CopperfinRuntimeBridgeEnvironmentVariable> environment;") != std::string::npos,
                   "build host FLL wrapper should carry launch environment entries through dispatch and launch surfaces.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeLaunchPlan") != std::string::npos,
                   "build host FLL wrapper should declare a launch-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeLaunchPlan copperfin_build_runtime_bridge_launch_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a launch-plan helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_library_export_env_var()") != std::string::npos,
                   "build host FLL wrapper should declare library-export env-var helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_library_export_env_var()") != std::string::npos,
                   "build host FLL wrapper should route library-export env-var through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_routine_kind_env_var()") != std::string::npos,
                   "build host FLL wrapper should declare routine-kind env-var helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_routine_kind_env_var()") != std::string::npos,
                   "build host FLL wrapper should route routine-kind env-var through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_source_path_env_var()") != std::string::npos,
                   "build host FLL wrapper should declare source-path env-var helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_source_path_env_var()") != std::string::npos,
                   "build host FLL wrapper should route source-path env-var through helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_parameter_count_env_var()") != std::string::npos,
                   "build host FLL wrapper should declare parameter-count env-var helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_parameter_count_env_var()") != std::string::npos,
                   "build host FLL wrapper should route parameter-count env-var through helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeObservationPlan") != std::string::npos,
                   "build host FLL wrapper should declare an observation-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeObservationPlan copperfin_build_runtime_bridge_observation_plan(") != std::string::npos,
                   "build host FLL wrapper should declare an observation-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeExecutionPlan") != std::string::npos,
                   "build host FLL wrapper should declare an execution-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeExecutionPlan copperfin_build_runtime_bridge_execution_plan(") != std::string::npos,
                   "build host FLL wrapper should declare an execution-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeTransportPlan") != std::string::npos,
                   "build host FLL wrapper should declare a transport-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeTransportPlan copperfin_build_runtime_bridge_transport_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a transport-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeSerializationPlan") != std::string::npos,
                   "build host FLL wrapper should declare a serialization-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeSerializationPlan copperfin_build_runtime_bridge_serialization_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a serialization-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeDispatchPlan") != std::string::npos,
                   "build host FLL wrapper should declare a dispatch-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeDispatchPlan copperfin_build_runtime_bridge_dispatch_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a dispatch-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeDispatchExecution copperfin_runtime_bridge_execute_dispatch(") != std::string::npos,
                   "build host FLL wrapper should declare a shared dispatch-execution helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeProcessLaunch copperfin_runtime_bridge_launch_process(") != std::string::npos,
                   "build host FLL wrapper should declare a shared process-launch helper.");
            expect(wrapper_source.find("#include <windows.h>") != std::string::npos &&
                       wrapper_source.find("#include <unistd.h>") != std::string::npos,
                   "build host FLL wrapper should include native process-launch support.");
            expect(wrapper_source.find("static std::vector<std::wstring> copperfin_runtime_bridge_windows_environment(") != std::string::npos &&
                       wrapper_source.find("static std::vector<std::string> copperfin_runtime_bridge_posix_environment(") != std::string::npos,
                   "build host FLL wrapper should build native environment blocks for both supported process APIs.");
            expect(wrapper_source.find("launch_plan.environment") != std::string::npos,
                   "build host FLL wrapper should carry launch environment entries into dispatch execution.");
            expect(wrapper_source.find("const auto environment_entries = copperfin_runtime_bridge_windows_environment(dispatch_execution.environment);") != std::string::npos &&
                       wrapper_source.find("const auto environment_values = copperfin_runtime_bridge_posix_environment(dispatch_execution.environment);") != std::string::npos,
                   "build host FLL wrapper should apply launch environment entries through native process APIs.");
            expect(wrapper_source.find("CreateProcessW(") != std::string::npos &&
                       wrapper_source.find("execve(") != std::string::npos,
                   "build host FLL wrapper should launch the runtime host without a shell.");
            expect(wrapper_source.find("std::system(") == std::string::npos &&
                       wrapper_source.find("copperfin_runtime_bridge_build_process_command(") == std::string::npos,
                   "build host FLL wrapper should not execute a generated shell command.");
            expect(wrapper_source.find("const bool launch_succeeded = launch_attempted && process_created && exit_code == dispatch_execution.expected_exit_code;") != std::string::npos,
                   "build host FLL wrapper should compare runtime-host exit code with the expected exit code.");
            expect(wrapper_source.find("        false,\n        false,\n        dispatch_execution.expected_exit_code,\n        dispatch_execution.expected_exit_code") == std::string::npos,
                   "build host FLL wrapper should not keep the deterministic process-launch failure placeholder.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeHostFailureEvaluation copperfin_runtime_bridge_evaluate_host_failure(") != std::string::npos,
                   "build host FLL wrapper should declare a shared host-failure evaluation helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeMissingResponseEvaluation copperfin_runtime_bridge_evaluate_missing_response(") != std::string::npos,
                   "build host FLL wrapper should declare a shared missing-response evaluation helper.");
            expect(wrapper_source.find("const CopperfinRuntimeBridgeResponseReadPlan& response_read_plan,\n    const std::string& response_document) {") != std::string::npos,
                   "build host FLL wrapper should pass response documents into missing-response evaluation.");
            expect(wrapper_source.find("const bool response_missing = response_read_plan.require_existing_response && response_document.empty();") != std::string::npos,
                   "build host FLL wrapper should detect missing responses from the read response document.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseValidationEvaluation copperfin_runtime_bridge_evaluate_response_validation(") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-validation evaluation helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_response_document_has_required_fields(") != std::string::npos,
                   "build host FLL wrapper should declare a required response-field validation helper.");
            expect(wrapper_source.find("response_validation_plan.required_response_fields") != std::string::npos,
                   "build host FLL wrapper should validate the declared required response fields.");
            expect(wrapper_source.find("!required_response_fields_present") != std::string::npos,
                   "build host FLL wrapper should fail response validation when required response fields are absent.");
            expect(wrapper_source.find("copperfin_runtime_bridge_extract_json_field(\n        response_document,\n        copperfin_build_runtime_bridge_response_media_type_field_name())") != std::string::npos,
                   "build host FLL wrapper should read response media type during response validation.");
            expect(wrapper_source.find("response_media_type == response_validation_plan.expected_response_media_type") != std::string::npos,
                   "build host FLL wrapper should compare response media type with the expected response media type.");
            expect(wrapper_source.find("!response_media_type_matches") != std::string::npos,
                   "build host FLL wrapper should fail response validation when response media type mismatches.");
            expect(wrapper_source.find("copperfin_runtime_bridge_extract_json_field(\n        response_document,\n        copperfin_build_runtime_bridge_schema_version_field_name())") != std::string::npos,
                   "build host FLL wrapper should read response schema version during response validation.");
            expect(wrapper_source.find("response_schema_version == response_validation_plan.expected_schema_version") != std::string::npos,
                   "build host FLL wrapper should compare response schema version with the expected schema version.");
            expect(wrapper_source.find("!response_schema_version_matches") != std::string::npos,
                   "build host FLL wrapper should fail response validation when response schema version mismatches.");
            expect(wrapper_source.find("bool response_document_available = false;") != std::string::npos,
                   "build host FLL wrapper should track response-document availability in response-validation evaluation.");
            expect(wrapper_source.find("const std::string& response_document) {") != std::string::npos,
                   "build host FLL wrapper should pass response documents into response-validation evaluation.");
            expect(wrapper_source.find("const bool response_document_available = !response_document.empty();") != std::string::npos,
                   "build host FLL wrapper should derive response-document availability during response-validation evaluation.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgePayloadPlan") != std::string::npos,
                   "build host FLL wrapper should declare a payload-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgePayloadPlan copperfin_build_runtime_bridge_payload_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a payload-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeInterpretationPlan") != std::string::npos,
                   "build host FLL wrapper should declare an interpretation-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeInterpretationPlan copperfin_build_runtime_bridge_interpretation_plan(") != std::string::npos,
                   "build host FLL wrapper should declare an interpretation-plan helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeFailurePolicyPlan") != std::string::npos,
                   "build host FLL wrapper should declare a failure-policy surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeFailurePolicyPlan copperfin_build_runtime_bridge_failure_policy_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a failure-policy helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_status_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-status field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_return_value_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-value field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_diagnostics_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-diagnostics field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_payload_shape_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared request payload-shape helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_payload_shape_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response payload-shape helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_export_name_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared export-name field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_routine_kind_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared routine-kind field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_source_path_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared source-path field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_source_line_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared source-line field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_declaration_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared parameter-declaration field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_names_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared parameter-names field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_count_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared parameter-count field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_schema_version_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared schema-version field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameters_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared parameters field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_media_type_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared request-media-type field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_fields_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared request-fields contract helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_expected_response_media_type_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared expected-response media-type helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_fields_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-fields contract helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_media_type_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-media-type field helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_payload_shape_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared payload-shape field helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_payload_shape_field_name()") != std::string::npos,
                   "build host FLL wrapper should route payload-shape field through helper in request document");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_name_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared parameter-name field helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_name_field_name()") != std::string::npos,
                   "build host FLL wrapper should route parameter-name field through helper in request document");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_value_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared parameter-value field helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_value_field_name()") != std::string::npos,
                   "build host FLL wrapper should route parameter-value field through helper in request document");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_surface_field_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared parameter-surface field helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_surface_field_name()") != std::string::npos,
                   "build host FLL wrapper should route parameter-surface field through helper in request document");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_failure_diagnostics_value()") != std::string::npos,
                   "build host FLL wrapper should declare a shared failure-diagnostics token helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_success_status_value()") != std::string::npos,
                   "build host FLL wrapper should declare a shared success-status token helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeResponseValidationPlan") != std::string::npos,
                   "build host FLL wrapper should declare a response-validation surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseValidationPlan copperfin_build_runtime_bridge_response_validation_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a response-validation helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeRequestArtifact") != std::string::npos,
                   "build host FLL wrapper should declare a request-artifact surface");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_document(") != std::string::npos,
                   "build host FLL wrapper should declare a request-document helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeRequestArtifact copperfin_build_runtime_bridge_request_artifact(") != std::string::npos,
                   "build host FLL wrapper should declare a request-artifact helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeRequestWritePlan") != std::string::npos,
                   "build host FLL wrapper should declare a request-write-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeRequestWritePlan copperfin_build_runtime_bridge_request_write_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a request-write-plan helper");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_execute_write_request(") != std::string::npos,
                   "build host FLL wrapper should declare a shared request-write execution helper.");
            expect(wrapper_source.find("out << plan.request_artifact.request_document;") != std::string::npos,
                   "build host FLL wrapper should stage request-document writes through the shared request-write execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeResponseReadPlan") != std::string::npos,
                   "build host FLL wrapper should declare a response-read-plan surface");
            expect(wrapper_source.find("bool request_write_succeeded = false;") != std::string::npos,
                   "build host FLL wrapper should carry request-write success on the response-read plan.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseReadPlan copperfin_build_runtime_bridge_response_read_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a response-read-plan helper");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_execute_read_response(") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-read execution helper.");
            expect(wrapper_source.find("if (!plan.request_write_succeeded)") != std::string::npos,
                   "build host FLL wrapper should fall back when request writing failed before reading a response.");
            expect(wrapper_source.find("response_document << input.rdbuf();") != std::string::npos,
                   "build host FLL wrapper should stage response-document reads through the shared response-read execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeResponseArtifact") != std::string::npos,
                   "build host FLL wrapper should declare a response-artifact surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseArtifact copperfin_build_runtime_bridge_response_artifact(") != std::string::npos,
                   "build host FLL wrapper should declare a response-artifact helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeResponseParsePlan") != std::string::npos,
                   "build host FLL wrapper should declare a response-parse-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseParsePlan copperfin_build_runtime_bridge_response_parse_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a response-parse-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseParseAdmission copperfin_runtime_bridge_admit_response_parse(") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-parse admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeParsedResponse copperfin_runtime_bridge_execute_parse_response(") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-parse execution helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_extract_json_field(response_document, plan.status_field)") != std::string::npos,
                   "build host FLL wrapper should stage response field extraction through the shared response-parse execution helper.");
            expect(wrapper_source.find("object_depth == 1U && array_depth == 0U") != std::string::npos,
                   "build host FLL wrapper should validate required response fields as top-level object fields");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_find_json_field_value_start(") != std::string::npos,
                   "build host FLL wrapper should declare a shared response field-value scanner");
            expect(wrapper_source.find("copperfin_runtime_bridge_find_json_field_value_start(response_document, field_name, value_start)") != std::string::npos,
                   "build host FLL wrapper should extract response field values through the shared scanner");
            expect(wrapper_source.find("response_document.compare(index, field_token.size(), field_token) == 0") != std::string::npos,
                   "build host FLL wrapper should compare required response fields through the scanner token match");
            expect(wrapper_source.find("return response_document.find(field_token) != std::string::npos;") == std::string::npos,
                   "build host FLL wrapper should not validate required response fields with raw token search");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeInterpretedResultPlan") != std::string::npos,
                   "build host FLL wrapper should declare an interpreted-result-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeInterpretedResultPlan copperfin_build_runtime_bridge_interpreted_result_plan(") != std::string::npos,
                   "build host FLL wrapper should declare an interpreted-result-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeInterpretedResultAdmission copperfin_runtime_bridge_admit_interpreted_result(") != std::string::npos,
                   "build host FLL wrapper should declare a shared interpreted-result admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeInterpretedResult copperfin_runtime_bridge_execute_interpreted_result(") != std::string::npos,
                   "build host FLL wrapper should declare a shared interpreted-result execution helper.");
            expect(wrapper_source.find("parsed_response.status_value == plan.success_status_value") != std::string::npos,
                   "build host FLL wrapper should stage interpreted-result selection through the shared execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeNativeReturnPlan") != std::string::npos,
                   "build host FLL wrapper should declare a native-return-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeNativeReturnPlan copperfin_build_runtime_bridge_native_return_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a native-return-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeNativeReturnAdmission copperfin_runtime_bridge_admit_native_return(") != std::string::npos,
                   "build host FLL wrapper should declare a shared native-return admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeNativeReturn copperfin_runtime_bridge_execute_native_return(") != std::string::npos,
                   "build host FLL wrapper should declare a shared native-return execution helper.");
            expect(wrapper_source.find("interpreted_result.matched_success_status") != std::string::npos,
                   "build host FLL wrapper should stage native-return selection through the shared execution helper.");
            expect(wrapper_source.find("static int copperfin_parse_runtime_bridge_int_value_representation(") != std::string::npos,
                   "build host FLL wrapper should declare an integer return-representation parser");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_parse_json_string_at(") != std::string::npos,
                   "build host FLL wrapper should declare a JSON string escape decoder for response parsing");
            expect(wrapper_source.find("copperfin_runtime_bridge_parse_json_string_at(response_document, value_start, string_end, decoded_value)") != std::string::npos,
                   "build host FLL wrapper should decode escaped response string fields before interpreting returns");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_default_int_value()") != std::string::npos,
                   "build host FLL wrapper should declare a shared parsed-int default sentinel helper");
            expect(wrapper_source.find("copperfin_runtime_bridge_default_int_value()") != std::string::npos,
                   "build host FLL wrapper should route the parsed-int default sentinel through the shared helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeOutcomeSelectionPlan") != std::string::npos,
                   "build host FLL wrapper should declare an outcome-selection-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeOutcomeSelectionPlan copperfin_build_runtime_bridge_outcome_selection_plan(") != std::string::npos,
                   "build host FLL wrapper should declare an outcome-selection-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeOutcomeSelectionAdmission copperfin_runtime_bridge_admit_outcome_selection(") != std::string::npos,
                   "build host FLL wrapper should declare a shared outcome-selection admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeOutcomeSelection copperfin_runtime_bridge_execute_outcome_selection(") != std::string::npos,
                   "build host FLL wrapper should declare a shared outcome-selection execution helper.");
            expect(wrapper_source.find("native_return.matched_success_status") != std::string::npos,
                   "build host FLL wrapper should stage outcome selection through the shared execution helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeReturnMaterializationPlan") != std::string::npos,
                   "build host FLL wrapper should declare a return-materialization-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnMaterializationPlan copperfin_build_runtime_bridge_return_materialization_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a return-materialization-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnMaterializationAdmission copperfin_runtime_bridge_admit_return_materialization(") != std::string::npos,
                   "build host FLL wrapper should declare a shared return-materialization admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnMaterialization copperfin_runtime_bridge_execute_return_materialization(") != std::string::npos,
                   "build host FLL wrapper should declare a shared return-materialization execution helper.");
            expect(wrapper_source.find("const auto& outcome_selection = plan.outcome_selection") != std::string::npos,
                   "build host FLL wrapper should consume explicit outcome selection while materializing returns.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_native_int_return_surface()") != std::string::npos,
                   "build host FLL wrapper should declare a shared native-int return-surface helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_native_int_return_surface()") != std::string::npos,
                   "build host FLL wrapper should route native-int return-surface comparisons through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_native_int_placeholder_signature_token()") != std::string::npos,
                   "build host FLL wrapper should declare a shared native-int placeholder-signature helper.");
            expect(wrapper_source.find("find(copperfin_build_runtime_bridge_native_int_placeholder_signature_token())") != std::string::npos,
                   "build host FLL wrapper should route native-int placeholder-signature matching through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_return_statement_from_expression(") != std::string::npos,
                   "build host FLL wrapper should declare a shared native return-statement framing helper.");
            expect(wrapper_source.find("return copperfin_build_runtime_bridge_return_statement_from_expression(") != std::string::npos,
                   "build host FLL wrapper should route native return-statement framing through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_typed_native_return_expression(") != std::string::npos,
                   "build host FLL wrapper should declare a shared typed native return-expression helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_typed_native_return_expression(") != std::string::npos,
                   "build host FLL wrapper should route typed native return-expression construction through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_stdout_log_suffix()") != std::string::npos,
                   "build host FLL wrapper should declare a shared stdout log-file suffix helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_stdout_log_suffix()") != std::string::npos,
                   "build host FLL wrapper should route stdout log-file suffix through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_stderr_log_suffix()") != std::string::npos,
                   "build host FLL wrapper should declare a shared stderr log-file suffix helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_stderr_log_suffix()") != std::string::npos,
                   "build host FLL wrapper should route stderr log-file suffix through the shared helper.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_expected_exit_code()") != std::string::npos,
                   "build host FLL wrapper should declare a shared expected-exit-code helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_expected_exit_code()") != std::string::npos,
                   "build host FLL wrapper should route expected-exit-code through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_request_artifact_suffix()") != std::string::npos,
                   "build host FLL wrapper should declare a shared request artifact suffix helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_request_artifact_suffix()") != std::string::npos,
                   "build host FLL wrapper should route request artifact suffix through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_runtime_bridge_response_artifact_suffix()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response artifact suffix helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_response_artifact_suffix()") != std::string::npos,
                   "build host FLL wrapper should route response artifact suffix through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_activates_adopted_return_policy()") != std::string::npos,
                   "build host FLL wrapper should declare a shared activates-adopted-return policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_activates_adopted_return_policy()") != std::string::npos,
                   "build host FLL wrapper should route activates-adopted-return policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_capture_stdout_policy()") != std::string::npos,
                   "build host FLL wrapper should declare a shared capture-stdout policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_capture_stdout_policy()") != std::string::npos,
                   "build host FLL wrapper should route capture-stdout policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_capture_stderr_policy()") != std::string::npos,
                   "build host FLL wrapper should declare a shared capture-stderr policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_capture_stderr_policy()") != std::string::npos,
                   "build host FLL wrapper should route capture-stderr policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_fail_on_nonzero_exit_policy()") != std::string::npos,
                   "build host FLL wrapper should declare a shared fail-on-nonzero-exit policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_fail_on_nonzero_exit_policy()") != std::string::npos,
                   "build host FLL wrapper should route fail-on-nonzero-exit policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_fail_on_missing_response_policy()") != std::string::npos,
                   "build host FLL wrapper should declare a shared fail-on-missing-response policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_fail_on_missing_response_policy()") != std::string::npos,
                   "build host FLL wrapper should route fail-on-missing-response policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_ensure_parent_directory_policy()") != std::string::npos,
                   "build host FLL wrapper should declare a shared ensure-parent-directory policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_ensure_parent_directory_policy()") != std::string::npos,
                   "build host FLL wrapper should route ensure-parent-directory policy through the shared helper.");
            expect(wrapper_source.find("static bool copperfin_runtime_bridge_require_existing_response_policy()") != std::string::npos,
                   "build host FLL wrapper should declare a shared require-existing-response policy helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_require_existing_response_policy()") != std::string::npos,
                   "build host FLL wrapper should route require-existing-response policy through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_replace_placeholder_return_mode()") != std::string::npos,
                   "build host FLL wrapper should declare a shared replace-placeholder-return adoption-mode helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_replace_placeholder_return_mode()") != std::string::npos,
                   "build host FLL wrapper should route replace-placeholder-return mode token through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_planned_activation_pending_mode()") != std::string::npos,
                   "build host FLL wrapper should declare a shared planned-activation-pending activation-mode helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_planned_activation_pending_mode()") != std::string::npos,
                   "build host FLL wrapper should route planned-activation-pending mode token through the shared helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeReturnEmissionPlan") != std::string::npos,
                   "build host FLL wrapper should declare a return-emission-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnEmissionPlan copperfin_build_runtime_bridge_return_emission_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a return-emission-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnEmissionAdmission copperfin_runtime_bridge_admit_return_emission(") != std::string::npos,
                   "build host FLL wrapper should declare a shared return-emission admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnEmission copperfin_runtime_bridge_execute_return_emission(") != std::string::npos,
                   "build host FLL wrapper should declare a shared return-emission execution helper.");
            expect(wrapper_source.find("const auto& return_materialization = plan.return_materialization") != std::string::npos,
                   "build host FLL wrapper should consume explicit materialized return while emitting returns.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeFinalReturnAdoptionPlan") != std::string::npos,
                   "build host FLL wrapper should declare a final-return-adoption-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeFinalReturnAdoptionPlan copperfin_build_runtime_bridge_final_return_adoption_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a final-return-adoption-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeFinalReturnAdoptionAdmission copperfin_runtime_bridge_admit_final_return_adoption(") != std::string::npos,
                   "build host FLL wrapper should declare a shared final-return-adoption admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeFinalReturnAdoption copperfin_runtime_bridge_execute_final_return_adoption(") != std::string::npos,
                   "build host FLL wrapper should declare a shared final-return-adoption execution helper.");
            expect(wrapper_source.find("const auto& return_emission = plan.return_emission") != std::string::npos,
                   "build host FLL wrapper should consume explicit return emission while adopting final returns.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_placeholder_return_statement(") != std::string::npos,
                   "build host FLL wrapper should declare a shared placeholder return-statement helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeReturnActivationPlan") != std::string::npos,
                   "build host FLL wrapper should declare a return-activation-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnActivationPlan copperfin_build_runtime_bridge_return_activation_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a return-activation-plan helper");
            expect(wrapper_source.find("CopperfinRuntimeBridgeStubEmissionWrapper stub_emission_wrapper;") != std::string::npos,
                   "build host FLL wrapper should carry the stub-emission wrapper contract through the descriptor plan.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnActivationAdmission copperfin_runtime_bridge_admit_return_activation(") != std::string::npos,
                   "build host FLL wrapper should declare a shared return-activation admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnActivation copperfin_runtime_bridge_execute_return_activation(") != std::string::npos,
                   "build host FLL wrapper should declare a shared return-activation execution helper.");
            expect(wrapper_source.find("const auto& final_return_adoption = plan.final_return_adoption") != std::string::npos,
                   "build host FLL wrapper should consume explicit final-return adoption while activating returns.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeStubReturnPlan") != std::string::npos,
                   "build host FLL wrapper should declare a stub-return-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubReturnPlan copperfin_build_runtime_bridge_stub_return_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a stub-return-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubReturnAdmission copperfin_runtime_bridge_admit_stub_return(") != std::string::npos,
                   "build host FLL wrapper should declare a shared stub-return admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubReturn copperfin_runtime_bridge_execute_stub_return(") != std::string::npos,
                   "build host FLL wrapper should declare a shared stub-return execution helper.");
            expect(wrapper_source.find("const auto& return_activation = plan.return_activation") != std::string::npos,
                   "build host FLL wrapper should consume explicit return activation while routing stub returns.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgePlaceholderReturnValuePlan") != std::string::npos,
                   "build host FLL wrapper should declare a placeholder-return-value-plan surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgePlaceholderReturnValuePlan copperfin_build_runtime_bridge_placeholder_return_value_plan(") != std::string::npos,
                   "build host FLL wrapper should declare a placeholder-return-value-plan helper");
            expect(wrapper_source.find("static CopperfinRuntimeBridgePlaceholderReturnValueAdmission copperfin_runtime_bridge_admit_placeholder_return_value(") != std::string::npos,
                   "build host FLL wrapper should declare a shared placeholder-return-value admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgePlaceholderReturnValue copperfin_runtime_bridge_execute_placeholder_return_value(") != std::string::npos,
                   "build host FLL wrapper should declare a shared placeholder-return-value execution helper.");
            expect(wrapper_source.find("const auto& stub_return = plan.stub_return") != std::string::npos,
                   "build host FLL wrapper should consume explicit stub return while planning placeholder return values.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgePlaceholderReturnIntAdmission copperfin_runtime_bridge_admit_placeholder_return_int(") != std::string::npos,
                   "build host FLL wrapper should declare a shared placeholder-return-int admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubEmissionAdmission copperfin_runtime_bridge_admit_stub_emission(") != std::string::npos,
                   "build host FLL wrapper should declare a shared stub-emission admission helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubEmission copperfin_runtime_bridge_execute_stub_emission(") != std::string::npos,
                   "build host FLL wrapper should declare a shared stub-emission execution helper.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubEmissionReturnSurface copperfin_runtime_bridge_build_stub_emission_return_surface(") != std::string::npos,
                   "build host FLL wrapper should declare a shared stub-emission return-surface helper.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_apply_stub_emission_output(") != std::string::npos,
                   "build host FLL wrapper should declare a shared stub-emission output-application helper.");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeStubEmissionWrapper") != std::string::npos,
                   "build host FLL wrapper should declare a shared stub-emission wrapper surface.");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeStubEmissionWrapper copperfin_runtime_bridge_build_stub_emission_wrapper(") != std::string::npos,
                   "build host FLL wrapper should declare a shared stub-emission wrapper helper.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_emit_stub_return_shared(") == std::string::npos,
                   "build host FLL wrapper should apply stub-emission output at generated call-sites instead of a shared emitter helper.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_execute_placeholder_return_int(") != std::string::npos,
                   "build host FLL wrapper should declare a shared placeholder-return-int execution helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_execute_placeholder_return_int(placeholder_return_value)};") == std::string::npos,
                   "build host FLL wrapper should not hide placeholder-return-int execution inside the shared stub-emission helper.");
            expect(wrapper_source.find("const auto stub_emission_wrapper =\n        copperfin_runtime_bridge_build_stub_emission_wrapper(") != std::string::npos,
                   "build host FLL wrapper should build a shared stub-emission wrapper before building the descriptor plan.");
            expect(wrapper_source.find("return copperfin_runtime_bridge_apply_stub_emission_output(\n        stub_emission_return_surface,") != std::string::npos,
                   "build host FLL wrapper should route FLL stub emission through the generated output-application call-site.");
            expect(wrapper_source.find("final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.native_return_surface") != std::string::npos,
                   "build host FLL wrapper should read the stub-emission return surface through the descriptor plan.");
            expect(wrapper_source.find("final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.return_adapter") != std::string::npos,
                   "build host FLL wrapper should read the stub-emission return adapter through the descriptor plan.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_fll_int_return_surface(),") != std::string::npos,
                   "build host FLL wrapper should pass the FLL int return-surface contract into the shared wrapper helper.");
            expect(wrapper_source.find("_RetInt);") != std::string::npos,
                   "build host FLL wrapper should pass the `_RetInt` adapter into the shared wrapper helper.");
            expect(wrapper_source.find(", stub_emission_wrapper);") != std::string::npos,
                   "build host FLL wrapper should pass the built wrapper into the descriptor-plan builder.");
            expect(wrapper_source.find("const auto failure_policy = copperfin_build_runtime_bridge_failure_policy_plan(\n        interpretation_plan,\n        placeholder_return_binding.value_representation);") != std::string::npos,
                   "build host FLL wrapper should build the failure-policy plan from the enriched interpretation plan.");
            expect(wrapper_source.find("const auto response_validation = copperfin_build_runtime_bridge_response_validation_plan(\n        failure_policy);") != std::string::npos,
                   "build host FLL wrapper should build the response-validation plan from the enriched failure-policy plan.");
            expect(wrapper_source.find("const auto request_artifact = copperfin_build_runtime_bridge_request_artifact(\n        response_validation);") != std::string::npos,
                   "build host FLL wrapper should build the request artifact directly from the response-validation plan once the wrapper contract is upstream.");
            expect(wrapper_source.find("const auto request_write_plan = copperfin_build_runtime_bridge_request_write_plan(\n        request_artifact);") != std::string::npos,
                   "build host FLL wrapper should build the request-write plan directly from the request artifact once the wrapper contract is upstream.");
            expect(wrapper_source.find("const auto response_read_plan = copperfin_build_runtime_bridge_response_read_plan(\n        request_write_plan,\n        request_write_execution);") != std::string::npos,
                   "build host FLL wrapper should build the response-read plan from the request-write plan and executed write result.");
            expect(wrapper_source.find("const auto response_document =\n        copperfin_runtime_bridge_execute_read_response(response_read_plan);") != std::string::npos,
                   "build host FLL wrapper should execute the response-read plan before building the response artifact.");
            expect(wrapper_source.find("const auto response_artifact = copperfin_build_runtime_bridge_response_artifact(\n        response_read_plan,\n        response_document);") != std::string::npos,
                   "build host FLL wrapper should build the response artifact from the response-read plan and executed response document.");
            expect(wrapper_source.find("const auto response_parse_plan = copperfin_build_runtime_bridge_response_parse_plan(\n        response_artifact);") != std::string::npos,
                   "build host FLL wrapper should build the response-parse plan directly from the response artifact once the wrapper contract is upstream.");
            expect(wrapper_source.find("const auto parsed_response =\n        copperfin_runtime_bridge_execute_parse_response(response_parse_plan);") != std::string::npos,
                   "build host FLL wrapper should execute the response-parse plan before building the interpreted-result plan.");
            expect(wrapper_source.find("const auto interpreted_result_plan = copperfin_build_runtime_bridge_interpreted_result_plan(\n        response_parse_plan,\n        parsed_response);") != std::string::npos,
                   "build host FLL wrapper should build the interpreted-result plan from the response-parse plan and parsed response.");
            expect(wrapper_source.find("const auto interpreted_result =\n        copperfin_runtime_bridge_execute_interpreted_result(interpreted_result_plan);") != std::string::npos,
                   "build host FLL wrapper should execute the interpreted-result plan before building the native-return plan.");
            expect(wrapper_source.find("const auto native_return_plan = copperfin_build_runtime_bridge_native_return_plan(\n        result,\n        interpreted_result_plan,\n        interpreted_result);") != std::string::npos,
                   "build host FLL wrapper should build the native-return plan from the interpreted-result plan and interpreted result.");
            expect(wrapper_source.find("const auto native_return =\n        copperfin_runtime_bridge_execute_native_return(native_return_plan);") != std::string::npos,
                   "build host FLL wrapper should execute the native-return plan before building the outcome-selection plan.");
            expect(wrapper_source.find("const auto outcome_selection_plan = copperfin_build_runtime_bridge_outcome_selection_plan(\n        native_return_plan,\n        native_return);") != std::string::npos,
                   "build host FLL wrapper should build the outcome-selection plan from the native-return plan and native return.");
            expect(wrapper_source.find("const auto outcome_selection =\n        copperfin_runtime_bridge_execute_outcome_selection(outcome_selection_plan);") != std::string::npos,
                   "build host FLL wrapper should execute the outcome-selection plan before building the return-materialization plan.");
            expect(wrapper_source.find("const auto return_materialization_plan = copperfin_build_runtime_bridge_return_materialization_plan(\n        outcome_selection_plan,\n        outcome_selection);") != std::string::npos,
                   "build host FLL wrapper should build the return-materialization plan from the outcome-selection plan and outcome selection.");
            expect(wrapper_source.find("const auto return_materialization =\n        copperfin_runtime_bridge_execute_return_materialization(return_materialization_plan);") != std::string::npos,
                   "build host FLL wrapper should execute the return-materialization plan before building the return-emission plan.");
            expect(wrapper_source.find("const auto return_emission_plan = copperfin_build_runtime_bridge_return_emission_plan(\n        return_materialization_plan,\n        return_materialization);") != std::string::npos,
                   "build host FLL wrapper should build the return-emission plan from the return-materialization plan and materialized return.");
            expect(wrapper_source.find("const auto return_emission =\n        copperfin_runtime_bridge_execute_return_emission(return_emission_plan);") != std::string::npos,
                   "build host FLL wrapper should execute the return-emission plan before building the final-return-adoption plan.");
            expect(wrapper_source.find("const auto final_return_adoption_plan = copperfin_build_runtime_bridge_final_return_adoption_plan(\n        return_emission_plan,\n        return_emission,\n        copperfin_build_runtime_bridge_placeholder_return_statement(placeholder_return_binding));") != std::string::npos,
                   "build host FLL wrapper should build the final-return-adoption plan from the return-emission plan and emitted return.");
            expect(wrapper_source.find("const auto final_return_adoption =\n        copperfin_runtime_bridge_execute_final_return_adoption(final_return_adoption_plan);") != std::string::npos,
                   "build host FLL wrapper should execute the final-return-adoption plan before building the return-activation plan.");
            expect(wrapper_source.find("const auto return_activation_plan = copperfin_build_runtime_bridge_return_activation_plan(\n        final_return_adoption_plan,\n        final_return_adoption);") != std::string::npos,
                   "build host FLL wrapper should build the return-activation plan from the final-return-adoption plan and adopted return.");
            expect(wrapper_source.find("const auto return_activation =\n        copperfin_runtime_bridge_execute_return_activation(return_activation_plan);") != std::string::npos,
                   "build host FLL wrapper should execute the return-activation plan before building the stub-return plan.");
            expect(wrapper_source.find("const auto stub_return_plan = copperfin_build_runtime_bridge_stub_return_plan(\n        return_activation_plan,\n        return_activation);") != std::string::npos,
                   "build host FLL wrapper should build the stub-return plan from the return-activation plan and activated return.");
            expect(wrapper_source.find("static int copperfin_runtime_bridge_emit_stub_return(\n") == std::string::npos,
                   "build host FLL wrapper should no longer declare an output-specific stub-return wrapper helper once the plan carries the wrapper contract.");
            expect(wrapper_source.find("const char* routine_kind;") != std::string::npos,
                   "build host FLL wrapper should record routine kind fields in the FoxInfo table");
            expect(wrapper_source.find("const char* source_path;") != std::string::npos,
                   "build host FLL wrapper should record source-path fields in the FoxInfo table");
            expect(wrapper_source.find("unsigned int source_line;") != std::string::npos,
                   "build host FLL wrapper should record source-line fields in the FoxInfo table");
            expect(wrapper_source.find("const char* parameter_declaration_kind;") != std::string::npos,
                   "build host FLL wrapper should record parameter-declaration fields in the FoxInfo table");
            expect(wrapper_source.find("const char* parameter_names;") != std::string::npos,
                   "build host FLL wrapper should record parameter-name fields in the FoxInfo table");
            expect(wrapper_source.find("{\"InitLibrary\", &InitLibrary, \"procedure\", \"" + quote_manifest_value(init_library_source) + "\", 1U, \"lparameters\", \"tcMode\", 1U}") != std::string::npos,
                   "build host FLL wrapper should record InitLibrary metadata in the FoxInfo table");
            expect(wrapper_source.find("{\"AddNumbers\", &AddNumbers, \"function\", \"" + quote_manifest_value(add_numbers_source) + "\", 1U, \"parameters\", \"tnLeft|tnRight\", 2U}") != std::string::npos,
                   "build host FLL wrapper should record AddNumbers metadata in the FoxInfo table");
            expect(wrapper_source.find("const auto descriptor = copperfin_build_runtime_bridge_descriptor(\"InitLibrary\"") != std::string::npos,
                   "build host FLL wrapper should build a bridge descriptor for InitLibrary");
            expect(wrapper_source.find("const auto descriptor = copperfin_build_runtime_bridge_descriptor(\"AddNumbers\"") != std::string::npos,
                   "build host FLL wrapper should build a bridge descriptor for AddNumbers");
            expect(wrapper_source.find("const auto invocation = copperfin_build_runtime_bridge_invocation(\n        descriptor);") != std::string::npos,
                   "build host FLL wrapper should build a bridge invocation from the descriptor");
            expect(wrapper_source.find("const auto call = copperfin_build_runtime_bridge_call(") != std::string::npos,
                   "build host FLL wrapper should build a bridge call from the invocation");
            expect(wrapper_source.find("const auto result = copperfin_build_runtime_bridge_result(\n        call,\n        placeholder_return_binding);") != std::string::npos,
                   "build host FLL wrapper should build a bridge result from the enriched call");
            expect(wrapper_source.find("const auto placeholder_return_binding =") != std::string::npos,
                   "build host FLL wrapper should build a shared placeholder return binding before building the result");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_fll_int_return_surface()") != std::string::npos,
                   "build host FLL wrapper should declare a shared FLL int return-surface helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_fll_int_return_surface()") != std::string::npos,
                   "build host FLL wrapper should route FLL return surface through helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_placeholder_return_binding(\n            copperfin_build_runtime_bridge_fll_int_return_surface())") != std::string::npos,
                   "build host FLL wrapper should build the FLL placeholder return binding through the shared helper");
            expect(wrapper_source.find("const auto launch_plan = copperfin_build_runtime_bridge_launch_plan(\n        result);") != std::string::npos,
                   "build host FLL wrapper should build a launch plan from the result");
            expect(wrapper_source.find("const auto observation_plan = copperfin_build_runtime_bridge_observation_plan(\n        launch_plan);") != std::string::npos,
                   "build host FLL wrapper should build an observation plan from the launch plan");
            expect(wrapper_source.find("const auto execution_plan = copperfin_build_runtime_bridge_execution_plan(\n        observation_plan);") != std::string::npos,
                   "build host FLL wrapper should build an execution plan from the observation plan");
            expect(wrapper_source.find("const auto transport_plan = copperfin_build_runtime_bridge_transport_plan(\n        execution_plan);") != std::string::npos,
                   "build host FLL wrapper should build a transport plan from the execution plan");
            expect(wrapper_source.find("const auto serialization_plan = copperfin_build_runtime_bridge_serialization_plan(\n        transport_plan);") != std::string::npos,
                   "build host FLL wrapper should build a serialization plan from the transport plan");
            expect(wrapper_source.find("const auto dispatch_plan = copperfin_build_runtime_bridge_dispatch_plan(\n        serialization_plan);") != std::string::npos,
                   "build host FLL wrapper should build a dispatch plan from the serialization plan");
            expect(wrapper_source.find("const auto dispatch_execution = copperfin_runtime_bridge_execute_dispatch(dispatch_plan);") != std::string::npos,
                   "build host FLL wrapper should route the dispatch plan through the shared dispatch-execution helper.");
            expect(wrapper_source.find("(void)dispatch_execution;") == std::string::npos,
                   "build host FLL wrapper should consume dispatch execution when launching the process.");
            expect(wrapper_source.find("const auto process_launch = request_write_execution\n        ? copperfin_runtime_bridge_launch_process(dispatch_execution)") != std::string::npos,
                   "build host FLL wrapper should route dispatch execution through the shared process-launch helper.");
            expect(wrapper_source.find("(void)process_launch;") == std::string::npos,
                   "build host FLL wrapper should consume process launch when evaluating host failure.");
            expect(wrapper_source.find("const auto payload_plan = copperfin_build_runtime_bridge_payload_plan(\n        dispatch_plan);") != std::string::npos,
                   "build host FLL wrapper should build a payload plan from the dispatch plan");
            expect(wrapper_source.find("const auto interpretation_plan = copperfin_build_runtime_bridge_interpretation_plan(") != std::string::npos,
                   "build host FLL wrapper should build an interpretation plan from the payload plan");
            expect(wrapper_source.find("const auto failure_policy = copperfin_build_runtime_bridge_failure_policy_plan(") != std::string::npos,
                   "build host FLL wrapper should build a failure policy from the interpretation plan");
            expect(wrapper_source.find("const auto host_failure =") != std::string::npos,
                   "build host FLL wrapper should evaluate staged host failure from the process-launch helper.");
            expect(wrapper_source.find("copperfin_runtime_bridge_evaluate_host_failure(process_launch, failure_policy);") != std::string::npos,
                   "build host FLL wrapper should route process-launch output through the shared host-failure evaluation helper.");
            expect(wrapper_source.find("(void)host_failure;") == std::string::npos,
                   "build host FLL wrapper should consume host-failure evaluation when evaluating missing response.");
            expect(wrapper_source.find("const auto response_validation = copperfin_build_runtime_bridge_response_validation_plan(") != std::string::npos,
                   "build host FLL wrapper should build a response-validation plan from the failure policy");
            expect(wrapper_source.find("const auto request_artifact = copperfin_build_runtime_bridge_request_artifact(") != std::string::npos,
                   "build host FLL wrapper should build a request artifact from the response validation plan");
            expect(wrapper_source.find("const auto request_write_plan = copperfin_build_runtime_bridge_request_write_plan(") != std::string::npos,
                   "build host FLL wrapper should build a request write plan from the request artifact");
            expect(wrapper_source.find("const auto request_write_execution =\n        copperfin_runtime_bridge_execute_write_request(request_write_plan);") != std::string::npos,
                   "build host FLL wrapper should execute the request-write plan through the shared helper.");
            expect(wrapper_source.find("(void)request_write_execution;") == std::string::npos,
                   "build host FLL wrapper should consume request-write execution when planning response reads.");
            expect(wrapper_source.find("const auto response_read_plan = copperfin_build_runtime_bridge_response_read_plan(") != std::string::npos,
                   "build host FLL wrapper should build a response read plan from the request write plan");
            expect(wrapper_source.find("const auto missing_response =") != std::string::npos,
                   "build host FLL wrapper should evaluate staged missing-response policy from the host-failure and response-read helpers.");
            expect(wrapper_source.find("copperfin_runtime_bridge_evaluate_missing_response(\n            host_failure,\n            response_read_plan,\n            response_document);") != std::string::npos,
                   "build host FLL wrapper should route host-failure output and the response document through the shared missing-response evaluation helper.");
            expect(wrapper_source.find("(void)missing_response;") == std::string::npos,
                   "build host FLL wrapper should consume missing-response evaluation when evaluating response validation.");
            expect(wrapper_source.find("const auto response_validation_evaluation =") != std::string::npos,
                   "build host FLL wrapper should evaluate staged response-validation policy from the missing-response, validation, and response-document helpers.");
            expect(wrapper_source.find("copperfin_runtime_bridge_evaluate_response_validation(\n            missing_response,\n            response_validation,\n            response_document);") != std::string::npos,
                   "build host FLL wrapper should route missing-response output and the response document through the shared response-validation evaluation helper.");
            expect(wrapper_source.find("(void)response_validation_evaluation;") == std::string::npos,
                   "build host FLL wrapper should consume response-validation evaluation when admitting response parsing.");
            expect(wrapper_source.find("const auto response_artifact = copperfin_build_runtime_bridge_response_artifact(") != std::string::npos,
                   "build host FLL wrapper should build a response artifact from the response read plan");
            expect(wrapper_source.find("const auto response_parse_plan = copperfin_build_runtime_bridge_response_parse_plan(") != std::string::npos,
                   "build host FLL wrapper should build a response parse plan from the response artifact");
            expect(wrapper_source.find("const auto response_parse_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged response parsing from the response-validation evaluation and parse plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_response_parse(response_validation_evaluation, response_parse_plan);") != std::string::npos,
                   "build host FLL wrapper should route response-validation evaluation through the shared response-parse admission helper.");
            expect(wrapper_source.find("(void)response_parse_admission;") == std::string::npos,
                   "build host FLL wrapper should consume response-parse admission when admitting interpreted result.");
            expect(wrapper_source.find("const auto interpreted_result_plan = copperfin_build_runtime_bridge_interpreted_result_plan(") != std::string::npos,
                   "build host FLL wrapper should build an interpreted result plan from the response parse plan");
            expect(wrapper_source.find("const auto interpreted_result_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged interpreted-result selection from the response-parse admission and interpreted-result plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_interpreted_result(response_parse_admission, interpreted_result_plan);") != std::string::npos,
                   "build host FLL wrapper should route response-parse admission through the shared interpreted-result admission helper.");
            expect(wrapper_source.find("(void)interpreted_result_admission;") == std::string::npos,
                   "build host FLL wrapper should consume interpreted-result admission when admitting native return.");
            expect(wrapper_source.find("const auto native_return_plan = copperfin_build_runtime_bridge_native_return_plan(") != std::string::npos,
                   "build host FLL wrapper should build a native return plan from the interpreted result plan");
            expect(wrapper_source.find("const auto native_return_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged native-return selection from the interpreted-result admission and native-return plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_native_return(interpreted_result_admission, native_return_plan);") != std::string::npos,
                   "build host FLL wrapper should route interpreted-result admission through the shared native-return admission helper.");
            expect(wrapper_source.find("(void)native_return_admission;") == std::string::npos,
                   "build host FLL wrapper should consume native-return admission when admitting outcome selection.");
            expect(wrapper_source.find("const auto success_value_representation = interpreted_result.selected_return_value_representation;") != std::string::npos,
                   "build host FLL wrapper should route interpreted response return values into native-return planning");
            expect(wrapper_source.find("const int success_int_value = copperfin_parse_runtime_bridge_int_value_representation(\n        success_value_representation);") != std::string::npos,
                   "build host FLL wrapper should parse the typed success integer value from the interpreted response representation");
            expect(wrapper_source.find("const int fallback_int_value = copperfin_parse_runtime_bridge_int_value_representation(") != std::string::npos,
                   "build host FLL wrapper should parse the typed fallback integer value from the fallback representation");
            expect(wrapper_source.find("const auto int_value_representation = std::to_string(int_value);") != std::string::npos,
                   "build host FLL wrapper should build typed return statements from parsed integer values");
            expect(wrapper_source.find("native_return_plan.success_int_value,") != std::string::npos,
                   "build host FLL wrapper should materialize success returns from the parsed success integer value");
            expect(wrapper_source.find("native_return_plan.fallback_int_value,") != std::string::npos,
                   "build host FLL wrapper should materialize fallback returns from the parsed fallback integer value");
            expect(wrapper_source.find("\"else { \" + return_materialization.fallback_return_statement + \" }\";") != std::string::npos,
                   "build host FLL wrapper should record an explicit fallback else-branch statement");
            expect(wrapper_source.find("success_branch_statement + \" \" + fallback_branch_statement;") != std::string::npos,
                   "build host FLL wrapper should compose the emitted return block from the explicit branch statements");
            expect(wrapper_source.find("const auto active_return_block = final_return_adoption.adopted_return_block;") != std::string::npos,
                   "build host FLL wrapper should seed the inactive active-return block from the adopted return block");
            expect(wrapper_source.find(": return_activation.active_return_block;") != std::string::npos,
                   "build host FLL wrapper should route the deferred stub-return block through the activation metadata");
            expect(wrapper_source.find("int placeholder_fallback_int_value = -1;") != std::string::npos,
                   "build host FLL wrapper should record placeholder fallback integers in the stub-return plan");
            expect(wrapper_source.find("std::string placeholder_fallback_value_representation;") != std::string::npos,
                   "build host FLL wrapper should record placeholder fallback representations in the stub-return plan");
            expect(wrapper_source.find("bool emits_placeholder_return = true;") != std::string::npos,
                   "build host FLL wrapper should record placeholder-emission flags in the placeholder-return-value plan");
            expect(wrapper_source.find("std::string emitted_return_statement;") != std::string::npos,
                   "build host FLL wrapper should record placeholder emitted-return statements in the placeholder-return-value plan");
            expect(wrapper_source.find("std::string deferred_return_block;") != std::string::npos,
                   "build host FLL wrapper should record deferred return blocks in the placeholder-return-value plan");
            expect(wrapper_source.find("stub_return.emits_placeholder_return,") != std::string::npos,
                   "build host FLL wrapper should feed placeholder-emission flags from stub-return metadata");
            expect(wrapper_source.find("stub_return.emitted_return_statement,") != std::string::npos,
                   "build host FLL wrapper should feed emitted placeholder-return statements from stub-return metadata");
            expect(wrapper_source.find("stub_return.deferred_return_block,") != std::string::npos,
                   "build host FLL wrapper should feed deferred return blocks from stub-return metadata");
            expect(wrapper_source.find("stub_return.activation_mode,") != std::string::npos,
                   "build host FLL wrapper should feed activation modes from stub-return metadata");
            expect(wrapper_source.find("stub_return.adoption_mode,") != std::string::npos,
                   "build host FLL wrapper should feed adoption modes from stub-return metadata");
            expect(wrapper_source.find("stub_return.keeps_placeholder_return_active,") != std::string::npos,
                   "build host FLL wrapper should feed placeholder-helper active-policy booleans from stub-return metadata");
            expect(wrapper_source.find("stub_return.adopts_placeholder_replacement,") != std::string::npos,
                   "build host FLL wrapper should feed placeholder-helper replacement-policy booleans from stub-return metadata");
            expect(wrapper_source.find("stub_return.placeholder_fallback_int_value,") != std::string::npos,
                   "build host FLL wrapper should feed placeholder fallback integers from stub-return metadata");
            expect(wrapper_source.find("stub_return.placeholder_fallback_value_representation};") != std::string::npos,
                   "build host FLL wrapper should feed placeholder fallback representations from stub-return metadata");
            expect(wrapper_source.find("const bool keeps_placeholder_return_active =") != std::string::npos,
                   "build host FLL wrapper should derive placeholder-helper active-policy booleans upstream");
            expect(wrapper_source.find("const bool adopts_placeholder_replacement =") != std::string::npos,
                   "build host FLL wrapper should derive placeholder-helper replacement-policy booleans upstream");
            expect(wrapper_source.find("(void)placeholder_return_value.emitted_return_statement;") == std::string::npos,
                   "build host FLL wrapper should not have the int helper consume placeholder-value return-statement contracts");
            expect(wrapper_source.find("(void)placeholder_return_value.deferred_return_block;") == std::string::npos,
                   "build host FLL wrapper should not have the int helper consume placeholder-value deferred return-block contracts");
            expect(wrapper_source.find("placeholder_return_int_admission.should_return_int") != std::string::npos,
                   "build host FLL wrapper should have the int helper consume the admitted int-return policy boolean");
            expect(wrapper_source.find("placeholder_return_int_admission.selected_int_value") != std::string::npos,
                   "build host FLL wrapper should have the int helper consume the admitted selected integer value");
            expect(wrapper_source.find("const auto outcome_selection_plan = copperfin_build_runtime_bridge_outcome_selection_plan(") != std::string::npos,
                   "build host FLL wrapper should build an outcome selection plan from the native return plan");
            expect(wrapper_source.find("const auto outcome_selection_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged outcome selection from the native-return admission and outcome-selection plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_outcome_selection(native_return_admission, outcome_selection_plan);") != std::string::npos,
                   "build host FLL wrapper should route native-return admission through the shared outcome-selection admission helper.");
            expect(wrapper_source.find("(void)outcome_selection_admission;") == std::string::npos,
                   "build host FLL wrapper should consume outcome-selection admission when admitting return materialization.");
            expect(wrapper_source.find("const auto return_materialization_plan = copperfin_build_runtime_bridge_return_materialization_plan(") != std::string::npos,
                   "build host FLL wrapper should build a return materialization plan from the outcome selection plan");
            expect(wrapper_source.find("const auto return_materialization_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged return materialization from the outcome-selection admission and return-materialization plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_return_materialization(outcome_selection_admission, return_materialization_plan);") != std::string::npos,
                   "build host FLL wrapper should route outcome-selection admission through the shared return-materialization admission helper.");
            expect(wrapper_source.find("(void)return_materialization_admission;") == std::string::npos,
                   "build host FLL wrapper should consume return-materialization admission when admitting return emission.");
            expect(wrapper_source.find("const auto return_emission_plan = copperfin_build_runtime_bridge_return_emission_plan(") != std::string::npos,
                   "build host FLL wrapper should build a return emission plan from the return materialization plan");
            expect(wrapper_source.find("const auto return_emission_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged return emission from the return-materialization admission and return-emission plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_return_emission(return_materialization_admission, return_emission_plan);") != std::string::npos,
                   "build host FLL wrapper should route return-materialization admission through the shared return-emission admission helper.");
            expect(wrapper_source.find("(void)return_emission_admission;") == std::string::npos,
                   "build host FLL wrapper should consume return-emission admission when admitting final-return adoption.");
            expect(wrapper_source.find("const auto final_return_adoption_plan = copperfin_build_runtime_bridge_final_return_adoption_plan(") != std::string::npos,
                   "build host FLL wrapper should build a final return adoption plan from the return emission plan");
            expect(wrapper_source.find("const auto final_return_adoption_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged final return adoption from the return-emission admission and final-return-adoption plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_final_return_adoption(return_emission_admission, final_return_adoption_plan);") != std::string::npos,
                   "build host FLL wrapper should route return-emission admission through the shared final-return-adoption admission helper.");
            expect(wrapper_source.find("(void)final_return_adoption_admission;") == std::string::npos,
                   "build host FLL wrapper should consume final-return-adoption admission when admitting return activation.");
            expect(wrapper_source.find("const auto return_activation_plan = copperfin_build_runtime_bridge_return_activation_plan(") != std::string::npos,
                   "build host FLL wrapper should build a return activation plan from the final return adoption plan");
            expect(wrapper_source.find("const auto return_activation_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged return activation from the final-return-adoption admission and return-activation plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_return_activation(final_return_adoption_admission, return_activation_plan);") != std::string::npos,
                   "build host FLL wrapper should route final-return-adoption admission through the shared return-activation admission helper.");
            expect(wrapper_source.find("(void)return_activation_admission;") == std::string::npos,
                   "build host FLL wrapper should consume return-activation admission when admitting stub-return routing.");
            expect(wrapper_source.find("const auto stub_return_plan = copperfin_build_runtime_bridge_stub_return_plan(") != std::string::npos,
                   "build host FLL wrapper should build a stub return plan from the return activation plan");
            expect(wrapper_source.find("const auto stub_return_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged stub-return routing from the return-activation admission and stub-return plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_stub_return(return_activation_admission, stub_return_plan);") != std::string::npos,
                   "build host FLL wrapper should route return-activation admission through the shared stub-return admission helper.");
            expect(wrapper_source.find("(void)stub_return_admission;") == std::string::npos,
                   "build host FLL wrapper should consume stub-return admission when admitting placeholder-return-value routing.");
            expect(wrapper_source.find("const auto placeholder_return_value_plan = copperfin_build_runtime_bridge_placeholder_return_value_plan(") != std::string::npos,
                   "build host FLL wrapper should build a placeholder-return-value plan from the stub return plan");
            expect(wrapper_source.find("const auto stub_return =\n        copperfin_runtime_bridge_execute_stub_return(stub_return_plan);") != std::string::npos,
                   "build host FLL wrapper should execute the stub-return plan before building the placeholder-return-value plan.");
            expect(wrapper_source.find("const auto placeholder_return_value_plan = copperfin_build_runtime_bridge_placeholder_return_value_plan(\n        stub_return_plan,\n        stub_return);") != std::string::npos,
                   "build host FLL wrapper should build the placeholder-return-value plan from the stub-return plan and stub return.");
            expect(wrapper_source.find("const auto placeholder_return_value_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged placeholder-return-value routing from the stub-return admission and placeholder-return-value plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_placeholder_return_value(stub_return_admission, placeholder_return_value_plan);") != std::string::npos,
                   "build host FLL wrapper should route stub-return admission through the shared placeholder-return-value admission helper.");
            expect(wrapper_source.find("(void)placeholder_return_value_admission;") == std::string::npos,
                   "build host FLL wrapper should consume placeholder-return-value admission when admitting placeholder-return-int routing.");
            expect(wrapper_source.find("const auto placeholder_return_value =\n        copperfin_runtime_bridge_execute_placeholder_return_value(placeholder_return_value_plan);") != std::string::npos,
                   "build host FLL wrapper should execute the placeholder-return-value plan before shared stub emission.");
            expect(wrapper_source.find("const auto placeholder_return_int_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged placeholder-return-int routing from the placeholder-return-value admission and placeholder-return-value plan.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_placeholder_return_int(placeholder_return_value_admission, placeholder_return_value_plan);") != std::string::npos,
                   "build host FLL wrapper should route placeholder-return-value admission through the shared placeholder-return-int admission helper.");
            expect(wrapper_source.find("(void)placeholder_return_int_admission;") == std::string::npos,
                   "build host FLL wrapper should consume placeholder-return-int admission when executing placeholder-return-int output.");
            expect(wrapper_source.find("const auto placeholder_return_int =\n        copperfin_runtime_bridge_execute_placeholder_return_int(placeholder_return_int_admission);") != std::string::npos,
                   "build host FLL wrapper should execute placeholder-return-int routing from the admitted surface before stub emission.");
            expect(wrapper_source.find("const auto stub_emission_admission =") != std::string::npos,
                   "build host FLL wrapper should admit staged stub emission from the placeholder-return-int admission.");
            expect(wrapper_source.find("copperfin_runtime_bridge_admit_stub_emission(placeholder_return_int_admission, placeholder_return_int);") != std::string::npos,
                   "build host FLL wrapper should route explicit placeholder-return-int output through the shared stub-emission admission helper.");
            expect(wrapper_source.find("(void)stub_emission_admission;") == std::string::npos,
                   "build host FLL wrapper should consume stub-emission admission when executing stub emission.");
            expect(wrapper_source.find("const auto stub_emission =\n        copperfin_runtime_bridge_execute_stub_emission(stub_emission_admission);") != std::string::npos,
                   "build host FLL wrapper should execute stub emission from explicit admission output.");
            expect(wrapper_source.find("const auto stub_emission_return_surface =\n        copperfin_runtime_bridge_build_stub_emission_return_surface(") != std::string::npos,
                   "build host FLL wrapper should build the stub-emission return surface before direct output application.");
            expect(wrapper_source.find("native_return_plan.fallback_int_value") != std::string::npos,
                   "build host FLL wrapper should propagate the typed native fallback integer value downstream");
            expect(wrapper_source.find("return copperfin_runtime_bridge_apply_stub_emission_output(\n        stub_emission_return_surface,\n        placeholder_return_value_plan.stub_return_plan.return_activation_plan.final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.return_adapter);") != std::string::npos,
                   "build host FLL wrapper should route the placeholder return through the generated stub-emission output-application call-site.");
            expect(wrapper_source.find("\"--library-export\"") != std::string::npos,
                   "build host FLL wrapper should encode the export name into the bridge invocation plan");
            expect(wrapper_source.find("(void)parm;") == std::string::npos,
                   "build host FLL wrapper should consume ParamBlk through bridge call bindings.");
            expect(wrapper_source.find("{{\"parm\", std::to_string(static_cast<unsigned long long>(reinterpret_cast<std::uintptr_t>(parm))), \"ParamBlk*\"}}") != std::string::npos,
                   "build host FLL wrapper should preserve the ParamBlk call-surface binding");
            expect(wrapper_source.find(", stub_emission_wrapper);") != std::string::npos,
                   "build host FLL wrapper should feed the bridge result from the enriched descriptor and shared placeholder return binding");
            expect(wrapper_source.find("{copperfin_runtime_bridge_library_export_env_var(), result.call.invocation.descriptor.export_name}") != std::string::npos,
                   "build host FLL wrapper should preserve launch environment export metadata");
            expect(wrapper_source.find("std::string(export_name) + copperfin_runtime_bridge_stdout_log_suffix()") != std::string::npos,
                   "build host FLL wrapper should derive stdout observation paths");
            expect(wrapper_source.find("std::string(export_name) + copperfin_runtime_bridge_stderr_log_suffix()") != std::string::npos,
                   "build host FLL wrapper should derive stderr observation paths");
            expect(wrapper_source.find("observation_plan.launch_plan.result.call.invocation.descriptor.runtime_host_path") != std::string::npos,
                   "build host FLL wrapper should preserve the runtime-host executable path in the execution plan");
            expect(wrapper_source.find("observation_plan.launch_plan.result.call.invocation.arguments") != std::string::npos,
                   "build host FLL wrapper should preserve the bridge invocation arguments in the execution plan");
            expect(wrapper_source.find("artifact_stem + copperfin_runtime_bridge_request_artifact_suffix()") != std::string::npos,
                   "build host FLL wrapper should derive request transport paths");
            expect(wrapper_source.find("artifact_stem + copperfin_runtime_bridge_response_artifact_suffix()") != std::string::npos,
                   "build host FLL wrapper should derive response transport paths");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_media_type_value()") != std::string::npos,
                   "build host FLL wrapper should declare a shared request serialization media-type helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_media_type_value()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response serialization media-type helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_schema_version_value()") != std::string::npos,
                   "build host FLL wrapper should declare a shared serialization schema-version helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_path_argument_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared request-path dispatch helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_path_argument_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-path dispatch helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_media_type_argument_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared request-media-type dispatch helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_media_type_argument_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response-media-type dispatch helper");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_schema_version_argument_name()") != std::string::npos,
                   "build host FLL wrapper should declare a shared schema-version dispatch helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_media_type_value()") != std::string::npos,
                   "build host FLL wrapper should route the request serialization media type through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_media_type_value()") != std::string::npos,
                   "build host FLL wrapper should route the response serialization media type through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_schema_version_value()") != std::string::npos,
                   "build host FLL wrapper should route the serialization schema version through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_path_argument_name()") != std::string::npos,
                   "build host FLL wrapper should route the request-path dispatch argument through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_path_argument_name()") != std::string::npos,
                   "build host FLL wrapper should route the response-path dispatch argument through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_media_type_argument_name()") != std::string::npos,
                   "build host FLL wrapper should route the request-media-type dispatch argument through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_media_type_argument_name()") != std::string::npos,
                   "build host FLL wrapper should route the response-media-type dispatch argument through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_schema_version_argument_name()") != std::string::npos,
                   "build host FLL wrapper should route the schema-version dispatch argument through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_payload_shape_name()") != std::string::npos,
                   "build host FLL wrapper should route the request payload shape through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_payload_shape_name()") != std::string::npos,
                   "build host FLL wrapper should route the response payload shape through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_export_name_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the export-name field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_routine_kind_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the routine-kind field through the shared helper");
            expect(wrapper_source.find("copperfin_escape_runtime_bridge_json_string(call.invocation.descriptor.routine_kind)") != std::string::npos,
                   "build host FLL wrapper should serialize routine-kind metadata into the request document");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_source_path_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the source-path field through the shared helper");
            expect(wrapper_source.find("copperfin_escape_runtime_bridge_json_string(call.invocation.descriptor.source_path)") != std::string::npos,
                   "build host FLL wrapper should serialize source-path metadata into the request document");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_source_line_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the source-line field through the shared helper");
            expect(wrapper_source.find("call.invocation.descriptor.source_line") != std::string::npos,
                   "build host FLL wrapper should serialize source-line metadata into the request document");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_declaration_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the parameter-declaration field through the shared helper");
            expect(wrapper_source.find("copperfin_escape_runtime_bridge_json_string(call.invocation.descriptor.parameter_declaration_kind)") != std::string::npos,
                   "build host FLL wrapper should serialize parameter-declaration metadata into the request document");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_names_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the parameter-names field through the shared helper");
            expect(wrapper_source.find("copperfin_escape_runtime_bridge_json_string(call.invocation.descriptor.parameter_names)") != std::string::npos,
                   "build host FLL wrapper should serialize parameter-name metadata into the request document");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_count_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the parameter-count field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_schema_version_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the schema-version field through the shared helper");
            expect(wrapper_source.find("payload_plan.dispatch_plan.serialization_plan.schema_version") != std::string::npos,
                   "build host FLL wrapper should serialize schema-version metadata into the request document");
            expect(wrapper_source.find("{copperfin_build_runtime_bridge_export_name_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_routine_kind_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_source_path_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_source_line_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_parameter_declaration_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_parameter_names_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_parameter_count_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_schema_version_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_parameters_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_request_media_type_field_name()}") != std::string::npos,
                   "build host FLL wrapper should declare descriptor metadata in the request-field contract");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_parameters_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the parameters field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_media_type_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the request-media-type field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_fields_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the request-fields contract through the shared helper");
            expect(wrapper_source.find("payload_plan.request_fields.size()") != std::string::npos,
                   "build host FLL wrapper should serialize the request-field contract list");
            expect(wrapper_source.find("payload_plan.request_fields[index]") != std::string::npos,
                   "build host FLL wrapper should serialize each request-field contract item");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_expected_response_media_type_field_name()") != std::string::npos,
                   "build host FLL wrapper should route expected response media-type through the shared helper");
            expect(wrapper_source.find("payload_plan.dispatch_plan.serialization_plan.response_media_type") != std::string::npos,
                   "build host FLL wrapper should serialize expected response media type into the request document");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_fields_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the response-fields contract through the shared helper");
            expect(wrapper_source.find("payload_plan.response_fields.size()") != std::string::npos,
                   "build host FLL wrapper should serialize the response-field contract list");
            expect(wrapper_source.find("payload_plan.response_fields[index]") != std::string::npos,
                   "build host FLL wrapper should serialize each response-field contract item");
            expect(wrapper_source.find("{copperfin_build_runtime_bridge_status_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_return_value_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_response_media_type_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_schema_version_field_name(),\n"
                                       "         copperfin_build_runtime_bridge_diagnostics_field_name()}") != std::string::npos,
                   "build host FLL wrapper should declare schema version in the response-field contract");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_return_value_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the response value field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_media_type_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the response-media-type field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_status_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the response status field through the shared helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_diagnostics_field_name()") != std::string::npos,
                   "build host FLL wrapper should route the response diagnostics field through the shared helper");
            expect(wrapper_source.find("        copperfin_build_runtime_bridge_fll_int_return_surface());") != std::string::npos,
                   "build host FLL wrapper should preserve the FLL wrapper return surface");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_failure_diagnostics_value()") != std::string::npos,
                   "build host FLL wrapper should declare the diagnostics fallback policy through the shared token helper");
            expect(wrapper_source.find("placeholder_return_binding.value_representation);") != std::string::npos,
                   "build host FLL wrapper should declare the fallback return value policy through the shared binding");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_placeholder_return_statement(placeholder_return_binding)") != std::string::npos,
                   "build host FLL wrapper should derive the placeholder return statement from the shared binding helper");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_success_status_value()") != std::string::npos,
                   "build host FLL wrapper should declare the success-status expectation through the shared token helper");
            expect(wrapper_source.find("std::string request_document;") != std::string::npos,
                   "build host FLL wrapper should record the request document payload.");
            expect(wrapper_source.find("std::filesystem::path target_path;") != std::string::npos,
                   "build host FLL wrapper should record the request write target path.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_write_mode()") != std::string::npos,
                   "build host FLL wrapper should declare a shared request write-mode helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_request_write_mode()") != std::string::npos,
                   "build host FLL wrapper should route the request write mode through the shared helper.");
            expect(wrapper_source.find("std::filesystem::path source_path;") != std::string::npos,
                   "build host FLL wrapper should record the response read source path.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_read_mode()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response read-mode helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_read_mode()") != std::string::npos,
                   "build host FLL wrapper should route the response read mode through the shared helper.");
            expect(wrapper_source.find("std::string response_document;") != std::string::npos,
                   "build host FLL wrapper should record the response document payload.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_empty_response_document()") != std::string::npos,
                   "build host FLL wrapper should declare a shared empty response-document helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_empty_response_document()") != std::string::npos,
                   "build host FLL wrapper should route the empty response-document token through the shared helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_parse_kind()") != std::string::npos,
                   "build host FLL wrapper should declare a shared response parse-kind helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_response_parse_kind()") != std::string::npos,
                   "build host FLL wrapper should route the response parse kind through the shared helper.");
            expect(wrapper_source.find("std::string wrapper_return_surface;") != std::string::npos,
                   "build host FLL wrapper should record the wrapper return surface.");
            expect(wrapper_source.find("std::string native_return_surface;") != std::string::npos,
                   "build host FLL wrapper should record the native return surface.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_success_comparator_token()") != std::string::npos,
                   "build host FLL wrapper should declare a shared success-comparator helper.");
            expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_fallback_comparator_token()") != std::string::npos,
                   "build host FLL wrapper should declare a shared fallback-comparator helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_success_comparator_token()") != std::string::npos,
                   "build host FLL wrapper should route the success comparator through the shared helper.");
            expect(wrapper_source.find("copperfin_build_runtime_bridge_fallback_comparator_token()") != std::string::npos,
                   "build host FLL wrapper should route the fallback comparator through the shared helper.");
            expect(wrapper_source.find("std::string success_condition;") != std::string::npos,
                   "build host FLL wrapper should record the outcome success condition.");
            expect(wrapper_source.find("std::string success_return_statement;") != std::string::npos,
                   "build host FLL wrapper should record the success return statement.");
            expect(wrapper_source.find("std::string emitted_return_block;") != std::string::npos,
                   "build host FLL wrapper should record the emitted return block.");
            expect(wrapper_source.find("std::string placeholder_return_statement;") != std::string::npos,
                   "build host FLL wrapper should record the placeholder return statement.");
            expect(wrapper_source.find("bool activates_adopted_return = false;") != std::string::npos,
                   "build host FLL wrapper should record the inactive return-activation flag.");
            expect(wrapper_source.find("bool emits_placeholder_return = true;") != std::string::npos,
                   "build host FLL wrapper should record the placeholder-emission flag.");
            expect(wrapper_source.find("int fallback_int_value = -1;") != std::string::npos,
                   "build host FLL wrapper should record the placeholder fallback integer value.");
            expect(wrapper_source.find("int success_int_value = -1;") != std::string::npos,
                   "build host FLL wrapper should record the typed native success integer value.");
            expect(wrapper_cmake.find("target_link_libraries(LibraryDemo PRIVATE dl)") != std::string::npos,
                   "build host FLL wrapper CMake should link dl on supported Unix hosts");
            expect(api_manifest.find("function_arity=InitLibrary|1") != std::string::npos,
                   "build host FLL manifest should declare InitLibrary arity");
            expect(api_manifest.find("function_arity=AddNumbers|2") != std::string::npos,
                   "build host FLL manifest should declare AddNumbers arity");
            expect(api_manifest.find("function_kind=InitLibrary|procedure") != std::string::npos,
                   "build host FLL manifest should declare InitLibrary routine kind");
            expect(api_manifest.find("function_kind=AddNumbers|function") != std::string::npos,
                   "build host FLL manifest should declare AddNumbers routine kind");
            expect(manifest_source_location_matches(
                       api_manifest,
                       "function_source",
                       "InitLibrary",
                       init_library_source,
                       1U),
                   "build host FLL manifest should declare InitLibrary source provenance");
            expect(manifest_source_location_matches(
                       api_manifest,
                       "function_source",
                       "AddNumbers",
                       add_numbers_source,
                       1U),
                   "build host FLL manifest should declare AddNumbers source provenance");
            expect(api_manifest.find("function_parameters=InitLibrary|tcMode") != std::string::npos,
                   "build host FLL manifest should declare InitLibrary parameter names");
            expect(api_manifest.find("function_parameters=AddNumbers|tnLeft|tnRight") != std::string::npos,
                   "build host FLL manifest should declare AddNumbers parameter names");
            expect(api_manifest.find("function_parameter_declaration=InitLibrary|lparameters") != std::string::npos,
                   "build host FLL manifest should declare InitLibrary parameter declaration style");
            expect(api_manifest.find("function_parameter_declaration=AddNumbers|parameters") != std::string::npos,
                   "build host FLL manifest should declare AddNumbers parameter declaration style");
            expect(api_manifest.find("function_call_surface=InitLibrary|ParamBlk*|_RetInt") != std::string::npos,
                   "build host FLL manifest should declare InitLibrary callable surface");
            expect(api_manifest.find("function_call_surface=AddNumbers|ParamBlk*|_RetInt") != std::string::npos,
                   "build host FLL manifest should declare AddNumbers callable surface");
        }
    }

    if (extension == "dll" && fs::exists(expected_output)) {
        run_direct_library_bridge_shell_safety_smoke(expected_output.parent_path(), expected_output);
    }

    fs::remove_all(temp_root, ignored);
}
