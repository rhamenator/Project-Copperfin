// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_runtime_pipeline_support.h"

namespace cf_test_runtime_pipeline {
void test_generated_launcher_forwards_manifest_and_debug_flag() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_launcher_contract";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg", "RETURN\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "launcher_contract.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "LauncherContract";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "LauncherContract";
    workspace.build_plan.output_path = (output_dir / "LauncherContract.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        true);

    expect(plan.ok, "launcher contract plan should be created");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "launcher contract package should materialize");
    if (result.ok) {
        const std::string launcher_source = read_text(result.plan.launcher_source_path);
        const std::string launcher_project = read_text(result.plan.launcher_project_path);
        expect(
            launcher_source.find("var forwarded = new List<string> { \"--manifest\", Quote(manifest) };") != std::string::npos,
            "generated launcher should forward the manifest path to the runtime host");
        expect(
            launcher_source.find("string.Equals(arg, \"--debug\", StringComparison.OrdinalIgnoreCase)") != std::string::npos &&
            launcher_source.find("string.Equals(arg, \"/debug\", StringComparison.OrdinalIgnoreCase)") != std::string::npos,
            "generated launcher should preserve debug command-line forwarding");
        expect(
            launcher_source.find("string.Equals(arg, \"/locale\", StringComparison.OrdinalIgnoreCase)") != std::string::npos &&
            launcher_source.find("forwarded.Add(\"--locale\");") != std::string::npos,
            "generated launcher should normalize /locale before forwarding arguments to the runtime host");
        expect(
            launcher_source.find("string.Equals(args[index], \"--locale\", StringComparison.OrdinalIgnoreCase)") != std::string::npos &&
            launcher_source.find("Environment.GetEnvironmentVariable(\"COPPERFIN_LOCALE\")") != std::string::npos,
            "generated launcher should resolve locale from forwarded arguments and COPPERFIN_LOCALE");
        expect(
            launcher_source.find("forwarded.Add(Quote(arg));") != std::string::npos,
            "generated launcher should preserve ordinary application arguments instead of dropping them");
        expect(
            launcher_source.find("Runtime.Package.Launcher.Error.RuntimeHostMissing") != std::string::npos &&
            launcher_source.find("Runtime.Package.Launcher.Error.ManifestMissing") != std::string::npos &&
            launcher_source.find("Runtime.Package.Launcher.Error.RuntimeHostStartFailed") != std::string::npos,
            "generated launcher should route launcher failure text through localization keys");
        expect(
            launcher_source.find("[\"qps-ploc\"] = new(StringComparer.OrdinalIgnoreCase)") != std::string::npos,
            "generated launcher should embed a qps-ploc locale bucket for pseudo-localized runtime use");
        expect(
            launcher_source.find("WorkingDirectory = baseDir") != std::string::npos,
            "generated launcher should run the runtime host from the package directory");
        expect(
            launcher_project.find("<AssemblyName>LauncherContract</AssemblyName>") != std::string::npos,
            "generated launcher project should preserve the sanitized assembly name contract");
    }

    fs::remove_all(temp_root, ignored);
}

void test_dotnet_launcher_request_falls_back_to_native_host_when_unavailable() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_dotnet_fallback";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg", "RETURN\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "dotnet_fallback.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "DotNetFallback";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "DotNetFallback";
    workspace.build_plan.output_path = (output_dir / "DotNetFallback.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"}
    };

    auto extensibility_profile = copperfin::platform::default_extensibility_profile();
    extensibility_profile.dotnet_output.available = false;

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        extensibility_profile,
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        true);

    expect(plan.ok, "dotnet-fallback plan should be created");
    expect(plan.requested_dotnet_launcher, "dotnet-fallback plan should record the requested .NET launcher");
    expect(!plan.emit_dotnet_launcher, "dotnet-fallback plan should disable .NET launcher emission when unavailable");
    expect(plan.launcher_mode == "native_runtime_host", "dotnet-fallback plan should resolve to native runtime host mode");
    expect(plan.launcher_fallback == "dotnet_output_unavailable", "dotnet-fallback plan should record the fallback reason");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        extensibility_profile,
        runtime_host.string());

    expect(result.ok, "dotnet-fallback package should materialize");
    if (result.ok) {
        expect(!fs::exists(result.plan.launcher_project_path),
               "dotnet-fallback package should not emit a launcher project when .NET output is unavailable");
        expect(!fs::exists(result.plan.launcher_source_path),
               "dotnet-fallback package should not emit launcher source when .NET output is unavailable");
        expect(fs::exists(result.plan.launcher_output_path),
               "dotnet-fallback package should materialize a project-named native entrypoint");
        expect(read_text(result.plan.launcher_output_path) == "runtime-host",
               "dotnet-fallback native entrypoint should package the runtime host payload bytes");
        expect(
            std::any_of(
                result.plan.extension_payload_digests.begin(),
                result.plan.extension_payload_digests.end(),
                [&](const copperfin::runtime::RuntimeArtifactDigest& digest) {
                    return digest.path == result.plan.launcher_output_path;
                }),
            "dotnet-fallback package should record the native entrypoint in extension payload digests");

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        expect(runtime_manifest.find("launcher_mode=native_runtime_host") != std::string::npos,
               "dotnet-fallback manifest should record the native runtime host mode");
        expect(runtime_manifest.find("launcher_fallback=dotnet_output_unavailable") != std::string::npos,
               "dotnet-fallback manifest should record the .NET-unavailable fallback reason");
        expect(runtime_manifest.find("feature_flag=launcher.dotnet.requested|true|rollout") != std::string::npos,
               "dotnet-fallback manifest should preserve the requested .NET launcher feature flag");
        expect(runtime_manifest.find("feature_flag=launcher.dotnet.active|false|host_compatibility") != std::string::npos,
               "dotnet-fallback manifest should record the inactive .NET launcher feature flag");
        expect(debug_manifest.find("launcher_mode=native_runtime_host") != std::string::npos,
               "dotnet-fallback debug manifest should record the native runtime host mode");
        expect(debug_manifest.find("launcher_fallback=dotnet_output_unavailable") != std::string::npos,
               "dotnet-fallback debug manifest should record the fallback reason");
        expect(runtime_manifest.find("extension_payload=" + quote_manifest_value(result.plan.launcher_output_path) + "|") != std::string::npos,
               "dotnet-fallback manifest should include the native entrypoint payload digest");
    }

    fs::remove_all(temp_root, ignored);
}

void test_security_enabled_runtime_host_name_validation() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_security_tests";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path canonical_runtime_host = runtime_host_fixture_path(temp_root);
    const fs::path non_canonical_runtime_host = temp_root / "runtime_host_custom.exe";

    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg", "RETURN\n");
    write_text(canonical_runtime_host, "runtime-host");
    write_text(non_canonical_runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "secure_demo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "SecureDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "SecureDemo";
    workspace.build_plan.output_path = (output_dir / "SecureDemo.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"}
    };

    const auto secure_plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        true,
        false);

    expect(secure_plan.ok, "security-enabled plan should be created");

    const auto rejected_result = copperfin::runtime::materialize_runtime_package(
        secure_plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        non_canonical_runtime_host.string());

    expect(!rejected_result.ok, "security-enabled packaging should reject non-standard runtime host names");

    const auto accepted_result = copperfin::runtime::materialize_runtime_package(
        secure_plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        canonical_runtime_host.string());

    expect(accepted_result.ok, "security-enabled packaging should accept canonical runtime host name");

    fs::remove_all(temp_root, ignored);
}

void test_runtime_security_role_environment_fidelity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_security_role_fidelity";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);

    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    write_text(project_dir / "main.prg", "RETURN\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "security_role_fidelity.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "SecurityRoleFidelity";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "SecurityRoleFidelity";
    workspace.build_plan.output_path = (output_dir / "SecurityRoleFidelity.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"}
    };

    {
        ScopedEnvironmentVariable valid_role("COPPERFIN_SECURITY_ROLE", "security-admin");
        const auto plan = copperfin::runtime::create_runtime_package_plan(
            document,
            workspace,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile(),
            output_dir.string(),
            copperfin::runtime::BuildConfiguration::debug,
            true,
            true);

        expect(plan.security_role == "security-admin", "security role should accept explicit valid role from environment");
        expect(std::find(plan.warnings.begin(), plan.warnings.end(), plan.security_role) == plan.warnings.end() &&
               std::none_of(plan.warnings.begin(), plan.warnings.end(), [](const std::string& warning) {
                   return warning.find("Unknown security role requested") != std::string::npos;
               }),
               "security plan should not emit unknown-role warning for valid role");
    }

    {
        ScopedEnvironmentVariable invalid_role("COPPERFIN_SECURITY_ROLE", "not-a-real-role");
        const auto invalid_plan = copperfin::runtime::create_runtime_package_plan(
            document,
            workspace,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile(),
            output_dir.string(),
            copperfin::runtime::BuildConfiguration::debug,
            true,
            true);
        expect(invalid_plan.security_role == "developer", "invalid security role should fallback to default developer role");
        expect(std::any_of(invalid_plan.warnings.begin(), invalid_plan.warnings.end(), [](const std::string& warning) {
                   return warning.find("Unknown security role requested") != std::string::npos;
               }),
               "invalid security role should emit explicit unknown-role warning");

        const auto materialize_invalid = copperfin::runtime::materialize_runtime_package(
            invalid_plan,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile(),
            runtime_host.string());
        expect(materialize_invalid.ok, "package materialization should proceed with defaulted security role");
        if (materialize_invalid.ok) {
            const std::string runtime_manifest = read_text(materialize_invalid.plan.manifest_path);
            expect(runtime_manifest.find("security_role=developer") != std::string::npos,
                   "runtime manifest should record fallback security role after invalid role request");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_dotnet_launcher_finalization_rewrites_manifest_after_publish_output_materializes() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_dotnet_finalize";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg", "RETURN\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "dotnet_finalize.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "DotNetFinalize";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "DotNetFinalize";
    workspace.build_plan.output_path = (output_dir / "DotNetFinalize.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        true);
    expect(plan.ok, "dotnet-finalize plan should be created");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());
    expect(result.ok, "dotnet-finalize package should materialize");
    if (result.ok) {
        const std::string pre_runtime_manifest = read_text(result.plan.manifest_path);
        const std::string pre_debug_manifest = read_text(result.plan.debug_manifest_path);
        expect(!result.plan.primary_output_materialized,
               "dotnet-finalize plan should remain non-materialized before publish output exists");
        expect(pre_runtime_manifest.find("primary_output_materialized=false") != std::string::npos,
               "dotnet-finalize runtime manifest should start non-materialized");
        expect(pre_debug_manifest.find("primary_output_materialized=false") != std::string::npos,
               "dotnet-finalize debug manifest should start non-materialized");
        expect(pre_runtime_manifest.find("extension_payload=" + quote_manifest_value(result.plan.launcher_output_path) + "|") == std::string::npos,
               "dotnet-finalize runtime manifest should not claim a launcher payload before publish");
        expect(pre_debug_manifest.find("extension_payload=" + quote_manifest_value(result.plan.launcher_output_path) + "|") == std::string::npos,
               "dotnet-finalize debug manifest should not claim a launcher payload before publish");

        write_text(result.plan.launcher_output_path, "published-launcher");
        const auto finalize_result = copperfin::runtime::finalize_runtime_package_primary_output(
            result.plan,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile());
        expect(finalize_result.ok,
               "dotnet-finalize helper should succeed once the launcher output exists");
        if (finalize_result.ok) {
            expect(finalize_result.plan.primary_output_materialized,
                   "dotnet-finalize helper should mark the primary output as materialized");
            const std::string runtime_manifest = read_text(finalize_result.plan.manifest_path);
            const std::string debug_manifest = read_text(finalize_result.plan.debug_manifest_path);
            expect(runtime_manifest.find("primary_output_materialized=true") != std::string::npos,
                   "dotnet-finalize runtime manifest should report the materialized launcher output");
            expect(debug_manifest.find("primary_output_materialized=true") != std::string::npos,
                   "dotnet-finalize debug manifest should report the materialized launcher output");
            expect(runtime_manifest.find("extension_payload=" + quote_manifest_value(finalize_result.plan.launcher_output_path) + "|") != std::string::npos,
                   "dotnet-finalize runtime manifest should record the published launcher payload");
            expect(debug_manifest.find("extension_payload=" + quote_manifest_value(finalize_result.plan.launcher_output_path) + "|") != std::string::npos,
                   "dotnet-finalize debug manifest should record the published launcher payload");
            expect(lines_with_prefix(runtime_manifest, "extension_payload=") ==
                       lines_with_prefix(debug_manifest, "extension_payload="),
                   "dotnet-finalize runtime and debug manifests should stay synchronized");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_package_diagnostics_resolve_through_localization_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::vector<std::string_view> keys{
        "Runtime.Package.Launcher.Error.ManifestMissing",
        "Runtime.Package.Launcher.Error.RuntimeHostMissing",
        "Runtime.Package.Launcher.Error.RuntimeHostStartFailed",
        "Runtime.Package.Transpilation.Error.ManualPortRequiredForXAssetMethod",
        "Runtime.Package.Transpilation.Error.UnsupportedFoxProStatement",
        "Runtime.Package.Error.CopyFileFailed",
        "Runtime.Package.Error.CreateContentRootFailed",
        "Runtime.Package.Error.CreateDirectoryFailed",
        "Runtime.Package.Error.CreateFileFailed",
        "Runtime.Package.Error.CreateLauncherDirectoryFailed",
        "Runtime.Package.Error.CreateNativeWrapperBuildDirectoryFailed",
        "Runtime.Package.Error.CreateNativeWrapperDirectoryFailed",
        "Runtime.Package.Error.CreatePackageRootFailed",
        "Runtime.Package.Error.NativeWrapperCMakeMissing",
        "Runtime.Package.Error.NativeWrapperPrimaryOutputBuildFailed",
        "Runtime.Package.Error.NativeWrapperPrimaryOutputConfigureFailed",
        "Runtime.Package.Error.NativeWrapperPrimaryOutputMissing",
        "Runtime.Package.Error.OpenFileFailed",
        "Runtime.Package.Error.PlanInvalid",
        "Runtime.Package.Error.PrimaryOutputMissing",
        "Runtime.Package.Error.PrimaryOutputRequiresLibraryOutput",
        "Runtime.Package.Error.RuntimeHostSourcePathEmpty",
        "Runtime.Package.Error.RuntimeHostSourcePathNotRegularFile",
        "Runtime.Package.Error.RuntimeHostSourcePathResolveFailed",
        "Runtime.Package.Error.SecurityRequiresAbsoluteRuntimeHostPath",
        "Runtime.Package.Error.SecurityRequiresCanonicalRuntimeHostName",
        "Runtime.Package.Error.SourceFileMissing",
        "Runtime.Package.Error.WriteFileFailed"};

    expect(
        english_catalog.translate("Runtime.Package.Launcher.Error.RuntimeHostMissing") ==
            "Copperfin runtime host was not found beside the launcher.",
        "#2725: generated launcher runtime-host-missing diagnostics should resolve through the en-US catalog");
    expect(
        spanish_catalog.translate("Runtime.Package.Launcher.Error.ManifestMissing") ==
            "No se encontro el manifiesto de Copperfin junto al iniciador.",
        "#2725: generated launcher manifest-missing diagnostics should resolve through the es-419 catalog");
    expect(
        portuguese_catalog.translate("Runtime.Package.Launcher.Error.RuntimeHostStartFailed") ==
            "Nao foi possivel iniciar o host de runtime do Copperfin.",
        "#2725: generated launcher runtime-host-start diagnostics should resolve through the pt-BR catalog");
    expect(
        english_catalog.translate(
            "Runtime.Package.Transpilation.Error.UnsupportedFoxProStatement",
            {{"statementText", "READ EVENTS"}}) ==
            "Unsupported FoxPro statement: READ EVENTS",
        "#2726: transpiled unsupported-statement diagnostics should resolve through the en-US catalog");
    expect(
        spanish_catalog.translate(
            "Runtime.Package.Transpilation.Error.ManualPortRequiredForXAssetMethod",
            {{"methodIdentity", "custWidget.txtName.Valid"}}) ==
            "Se requiere port manual para el metodo xAsset de FoxPro: custWidget.txtName.Valid",
        "#2726: xAsset manual-port diagnostics should resolve through the es-419 catalog");
    expect(
        portuguese_catalog.translate(
            "Runtime.Package.Transpilation.Error.UnsupportedFoxProStatement",
            {{"statementText", "READ EVENTS"}}) ==
            "Instrucao FoxPro sem suporte: READ EVENTS",
        "#2726: transpiled unsupported-statement diagnostics should resolve through the pt-BR catalog");
    expect(
        pseudo_catalog.translate("Runtime.Package.Launcher.Error.RuntimeHostMissing") ==
            copperfin::localization::pseudo_localize("Copperfin runtime host was not found beside the launcher."),
        "#2725: generated launcher qps-ploc runtime-host-missing diagnostics should route through the pseudo-localization transform");
    expect(
        pseudo_catalog.translate(
            "Runtime.Package.Transpilation.Error.ManualPortRequiredForXAssetMethod",
            {{"methodIdentity", "custWidget.txtName.Valid"}}) ==
            "[!! Måñüål pørţ rëqüïrëd før FøxPrø xÅssëţ mëţhød: custWidget.txtName.Valid !!]",
        "#2726: xAsset manual-port qps-ploc diagnostics should route through the pseudo-localization transform");
    expect(
        english_catalog.translate("Runtime.Package.Error.PlanInvalid") == "Package plan is not valid.",
        "#2390: runtime package invalid-plan diagnostics should resolve through the en-US catalog");
    expect(
        pseudo_catalog.translate("Runtime.Package.Launcher.Error.RuntimeHostMissing") !=
            english_catalog.translate("Runtime.Package.Launcher.Error.RuntimeHostMissing"),
        "#2725: generated launcher diagnostics should be pseudo-localizable");
    expect(
        pseudo_catalog.translate(
            "Runtime.Package.Transpilation.Error.UnsupportedFoxProStatement",
            {{"statementText", "READ EVENTS"}}) !=
            english_catalog.translate(
                "Runtime.Package.Transpilation.Error.UnsupportedFoxProStatement",
                {{"statementText", "READ EVENTS"}}),
        "#2726: transpiled unsupported-statement diagnostics should be pseudo-localizable");
    expect(
        english_catalog.translate("Runtime.Package.Error.PrimaryOutputRequiresLibraryOutput") ==
            "Primary-output builds are only supported for library-output packages.",
        "#2390: primary-output diagnostics should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Runtime.Package.Error.SourceFileMissing", {{"path", "missing.prg"}}) ==
            "Source file does not exist: missing.prg",
        "#2390: runtime package diagnostics should preserve path placeholders");
    expect(
        spanish_catalog.translate("Runtime.Package.Error.PlanInvalid") ==
            "El plan del paquete no es valido.",
        "#2606: runtime package invalid-plan diagnostics should resolve through the es-419 catalog");
    expect(
        portuguese_catalog.translate("Runtime.Package.Error.PrimaryOutputRequiresLibraryOutput") ==
            "Builds de saida primaria sao suportados apenas para pacotes de saida de biblioteca.",
        "#2606: runtime package primary-output diagnostics should resolve through the pt-BR catalog");
    expect(
        portuguese_catalog.translate("Runtime.Package.Error.SourceFileMissing", {{"path", "missing.prg"}}) ==
            "O arquivo de origem nao existe: missing.prg",
        "#2606: runtime package source-file diagnostics should preserve placeholders through the pt-BR catalog");
    expect(
        pseudo_catalog.translate("Runtime.Package.Error.PlanInvalid") !=
            english_catalog.translate("Runtime.Package.Error.PlanInvalid"),
        "#2390: runtime package diagnostics should be pseudo-localizable");
    expect(
        pseudo_catalog.translate("Runtime.Package.Error.PlanInvalid") ==
            copperfin::localization::pseudo_localize("Package plan is not valid."),
        "#2606: runtime package qps-ploc diagnostics should route through the pseudo-localization transform");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", keys) == 0U,
        "#2606: es-419 should define every remaining Runtime.Package.Error localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", keys) == 0U,
        "#2606: pt-BR should define every remaining Runtime.Package.Error localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", keys) == 0U,
        "#2606: qps-ploc should define every remaining Runtime.Package.Error localization key");

    ScopedEnvironmentVariable scoped_locale("COPPERFIN_LOCALE", "en-US");

    const copperfin::runtime::RuntimePackagePlan invalid_plan{};
    const auto materialize_result = copperfin::runtime::materialize_runtime_package(
        invalid_plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        "runtime-host-fixture");
    expect(!materialize_result.ok, "invalid runtime package plan should not materialize");
    expect(
        materialize_result.error == "Package plan is not valid.",
        "#2390: materialize_runtime_package should preserve the default localized invalid-plan diagnostic");

    set_env_variable("COPPERFIN_LOCALE", "es-419", true);
    const auto spanish_materialize_result = copperfin::runtime::materialize_runtime_package(
        invalid_plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        "runtime-host-fixture");
    expect(
        !spanish_materialize_result.ok &&
            spanish_materialize_result.error == "El plan del paquete no es valido.",
        "#2606: runtime package invalid-plan diagnostics should refresh to es-419 when the runtime locale changes in-process");

    copperfin::runtime::RuntimePackagePlan executable_plan{};
    executable_plan.ok = true;
    executable_plan.output_kind = copperfin::runtime::BuildOutputKind::executable;
    set_env_variable("COPPERFIN_LOCALE", "en-US", true);
    const auto build_result = copperfin::runtime::build_runtime_package_primary_output(
        executable_plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile());
    expect(!build_result.ok, "primary-output build should reject executable package plans");
    expect(
        build_result.error == "Primary-output builds are only supported for library-output packages.",
        "#2390: primary-output build should preserve the default localized unsupported-kind diagnostic");

    set_env_variable("COPPERFIN_LOCALE", "pt-BR", true);
    const auto portuguese_build_result = copperfin::runtime::build_runtime_package_primary_output(
        executable_plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile());
    expect(
        !portuguese_build_result.ok &&
            portuguese_build_result.error ==
                "Builds de saida primaria sao suportados apenas para pacotes de saida de biblioteca.",
        "#2606: runtime package primary-output diagnostics should refresh to pt-BR when the runtime locale changes in-process");

    set_env_variable("COPPERFIN_LOCALE", "qps-ploc", true);
    const auto pseudo_materialize_result = copperfin::runtime::materialize_runtime_package(
        invalid_plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        "runtime-host-fixture");
    expect(
        !pseudo_materialize_result.ok &&
            pseudo_materialize_result.error ==
                copperfin::localization::pseudo_localize("Package plan is not valid."),
        "#2606: runtime package invalid-plan diagnostics should refresh to qps-ploc when the runtime locale changes in-process");
}

}  // namespace cf_test_runtime_pipeline
