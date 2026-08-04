// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

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
    workspace.project_title = "Launcher Project Title";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = workspace.project_title;
    workspace.build_plan.output_path =
        (output_dir / "Configured $(Configuration) @(Items); 100% & Launcher.exe").string();
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

    expect_materialization(result, "launcher contract package should materialize");
    if (result.ok) {
        const std::string launcher_source = read_text(result.plan.launcher_source_path);
        const std::string launcher_project = read_text(result.plan.launcher_project_path);
        expect(
            launcher_source.find("var runtimeHost = Path.Combine(baseDir, \"" + runtime_host.filename().string() + "\");") != std::string::npos,
            "generated launcher should use the platform-correct runtime host filename");
        expect(
            launcher_source.find("var debugManifest = Path.Combine(baseDir, \"app.cfdebug\");") != std::string::npos &&
            launcher_source.find("var selectedManifest = debugRequested && File.Exists(debugManifest) ? debugManifest : manifest;") != std::string::npos &&
            launcher_source.find("forwarded.Insert(0, selectedManifest);") != std::string::npos,
            "generated launcher should prefer the debug manifest for implicit debug launches");
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
            launcher_source.find("var dotSuffix = value.IndexOf('.')") != std::string::npos &&
            launcher_source.find("var modifierSuffix = value.IndexOf('@')") != std::string::npos &&
            launcher_source.find("value = value.Substring(0, suffix).Trim()") != std::string::npos,
            "generated launcher should strip POSIX encoding and modifier suffixes before locale fallback");
        expect(
            launcher_source.find("forwarded.Add(arg);") != std::string::npos &&
            launcher_source.find("startInfo.ArgumentList.Add(argument);") != std::string::npos &&
            launcher_source.find("Arguments = string.Join") == std::string::npos,
            "generated launcher should preserve ordinary application arguments instead of dropping them");
        expect(
            launcher_source.find("Runtime.Package.Launcher.Error.RuntimeHostMissing") != std::string::npos &&
            launcher_source.find("Runtime.Package.Launcher.Error.ManifestMissing") != std::string::npos &&
            launcher_source.find("Runtime.Package.Launcher.Error.RuntimeHostStartFailed") != std::string::npos &&
            launcher_source.find("catch (Exception)") != std::string::npos,
            "generated launcher should route launcher failure text through localization keys");
        expect(
            launcher_source.find("[\"qps-ploc\"] = new(StringComparer.OrdinalIgnoreCase)") != std::string::npos,
            "generated launcher should embed a qps-ploc locale bucket for pseudo-localized runtime use");
        expect(
            launcher_source.find("Console.OutputEncoding = Encoding.UTF8;") != std::string::npos,
            "generated launcher should emit localized console diagnostics as UTF-8");
        expect(
            launcher_source.find("WorkingDirectory = baseDir") != std::string::npos,
            "generated launcher should run the runtime host from the package directory");
        expect(
            fs::path(result.plan.launcher_output_path).filename() ==
                "Configured $(Configuration) @(Items); 100% & Launcher.exe" &&
            launcher_project.find(
                "<AssemblyName>Copperfin.GeneratedLauncher</AssemblyName>") != std::string::npos,
            "generated launcher project should keep a stable internal assembly name beside the configured output contract");
        expect(
            launcher_project.find("<EnableDefaultCompileItems>false</EnableDefaultCompileItems>") != std::string::npos &&
            launcher_project.find("<Compile Include=\"Program.cs\" />") != std::string::npos,
            "generated launcher project should compile Program.cs explicitly when publishing into its package root");
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

    expect_materialization(result, "dotnet-fallback package should materialize");
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
        expect(runtime_manifest.find("launcher_mode=") == std::string::npos,
               "dotnet-fallback runtime manifest should omit launcher mode from the execution contract");
        expect(runtime_manifest.find("launcher_fallback=") == std::string::npos,
               "dotnet-fallback runtime manifest should omit launcher fallback from the execution contract");
        expect(lines_with_prefix(runtime_manifest, "feature_flag=").empty(),
               "dotnet-fallback runtime manifest should omit feature-flag inventory from the execution contract");
        expect(debug_manifest.find("launcher_mode=native_runtime_host") != std::string::npos,
               "dotnet-fallback debug manifest should record the native runtime host mode");
        expect(debug_manifest.find("launcher_fallback=dotnet_output_unavailable") != std::string::npos,
               "dotnet-fallback debug manifest should record the fallback reason");
        expect(debug_manifest.find("feature_flag=launcher.dotnet.requested|true|rollout") != std::string::npos,
               "dotnet-fallback debug manifest should preserve the requested .NET launcher feature flag");
        expect(debug_manifest.find("feature_flag=launcher.dotnet.active|false|host_compatibility") != std::string::npos,
               "dotnet-fallback debug manifest should record the inactive .NET launcher feature flag");
        expect(runtime_manifest.find("extension_payload=" + quote_manifest_value(result.plan.launcher_output_path) + "|") != std::string::npos,
               "dotnet-fallback manifest should include the native entrypoint payload digest");
    }

    fs::remove_all(temp_root, ignored);
}

void test_security_enabled_runtime_host_name_validation() {
    namespace fs = std::filesystem;
    const fs::path locale_root = runtime_pipeline_locale_root();
    expect(fs::is_directory(locale_root),
           "security-enabled runtime-host path test should resolve the source localization catalogs");
    ScopedEnvironmentVariable locale_dir(
        "COPPERFIN_LOCALE_DIR",
        locale_root.string());
    ScopedEnvironmentVariable locale("COPPERFIN_LOCALE", "en-US");
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

    const fs::path original_working_directory = fs::current_path();
    std::error_code current_path_error;
    fs::current_path(temp_root, current_path_error);
    expect(!current_path_error, "security-enabled runtime-host path test should enter its fixture directory");
    const std::string relative_runtime_host = canonical_runtime_host.filename().string();
    expect(fs::exists(relative_runtime_host),
           "security-enabled runtime-host path test should expose an existing relative fixture path");

    const auto relative_rejected_result = copperfin::runtime::materialize_runtime_package(
        secure_plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        relative_runtime_host);
    expect(!relative_rejected_result.ok,
           "security-enabled packaging should reject an existing caller-relative runtime host path");
    expect(relative_rejected_result.error ==
               "Security-enabled packaging requires an absolute runtime host source path.",
           "security-enabled relative runtime-host rejection should preserve the localized English diagnostic; observed: " +
               relative_rejected_result.error);

    {
        ScopedEnvironmentVariable portuguese_locale("COPPERFIN_LOCALE", "pt-BR");
        const auto localized_relative_rejection = copperfin::runtime::materialize_runtime_package(
            secure_plan,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile(),
            relative_runtime_host);
        expect(!localized_relative_rejection.ok &&
                   localized_relative_rejection.error ==
                       "Pacotes com seguranca exigem um caminho absoluto do runtime host.",
               "security-enabled relative runtime-host rejection should localize without weakening the path rule; observed: " +
                   localized_relative_rejection.error);
    }

    std::error_code restore_path_error;
    fs::current_path(original_working_directory, restore_path_error);
    expect(!restore_path_error, "security-enabled runtime-host path test should restore its working directory");

    const fs::path safely_noncanonical_runtime_host =
        canonical_runtime_host.parent_path() / "." / canonical_runtime_host.filename();
    expect(safely_noncanonical_runtime_host.is_absolute(),
           "security-enabled runtime-host noncanonical fixture should remain caller-absolute");
    const auto noncanonical_accepted_result = copperfin::runtime::materialize_runtime_package(
        secure_plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        safely_noncanonical_runtime_host.string());
    expect(noncanonical_accepted_result.ok,
           "security-enabled packaging should accept an absolute runtime-host path with safe dot normalization");

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
    const std::string unicode_launcher_name = "caf\xC3\xA9-launcher.exe";
    workspace.build_plan.output_path = copperfin::platform::path_to_utf8_string(
        output_dir / copperfin::platform::path_from_utf8_string(unicode_launcher_name));
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

    auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());
    expect_materialization(result, "dotnet-finalize package should materialize");
    if (result.ok) {
        const std::string pre_runtime_manifest = read_text(result.plan.manifest_path);
        const std::string pre_debug_manifest = read_text(result.plan.debug_manifest_path);
        expect(!result.plan.primary_output_materialized,
               "dotnet-finalize plan should remain non-materialized before publish output exists");
        expect(pre_runtime_manifest.find("primary_output_path=") == std::string::npos,
               "dotnet-finalize runtime manifest should omit the primary output path before publish");
        expect(pre_debug_manifest.find("primary_output_path=" + quote_manifest_value(result.plan.launcher_output_path)) != std::string::npos,
               "dotnet-finalize debug manifest should preserve the primary output path before publish");
        expect(pre_runtime_manifest.find("primary_output_materialized=") == std::string::npos,
               "dotnet-finalize runtime manifest should omit the primary-output materialization state before publish");
        expect(pre_debug_manifest.find("primary_output_materialized=false") != std::string::npos,
               "dotnet-finalize debug manifest should start non-materialized");
        expect(pre_runtime_manifest.find("extension_payload=" + quote_manifest_value(result.plan.launcher_output_path) + "|") == std::string::npos,
               "dotnet-finalize runtime manifest should not claim a launcher payload before publish");
        expect(pre_debug_manifest.find("extension_payload=" + quote_manifest_value(result.plan.launcher_output_path) + "|") == std::string::npos,
               "dotnet-finalize debug manifest should not claim a launcher payload before publish");

        const fs::path package_root = copperfin::platform::path_from_utf8_string(
            result.plan.package_root);
        const fs::path launcher_output = copperfin::platform::path_from_utf8_string(
            result.plan.launcher_output_path);
        const fs::path launcher_dll = package_root / "Copperfin.GeneratedLauncher.dll";
        const fs::path launcher_deps = package_root / "Copperfin.GeneratedLauncher.deps.json";
        const fs::path launcher_pdb = package_root / "Copperfin.GeneratedLauncher.pdb";

        expect(
            fs::is_regular_file(copperfin::platform::path_from_utf8_string(
                result.plan.ast_manifest_path)) &&
                copperfin::platform::path_to_utf8_string(
                    copperfin::platform::path_from_utf8_string(
                        result.plan.ast_manifest_path).filename()) ==
                    unicode_launcher_name + ".ast.json",
            "#3873: a Unicode public OUTFILE should preserve its UTF-8 identity in compiler artifacts");

        const auto missing_sidecars = copperfin::runtime::finalize_runtime_package_primary_output(
            result.plan,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile());
        expect(!missing_sidecars.ok,
               "#4052: dotnet finalization should fail when required internal sidecars are missing");

        const auto retry_materialized = copperfin::runtime::materialize_runtime_package(
            plan,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile(),
            runtime_host.string());
        expect(retry_materialized.ok,
               "#4053: a failed first finalization should permit a clean package retry");
        if (!retry_materialized.ok) {
            return;
        }
        result.plan = retry_materialized.plan;
        const fs::path retry_package_root = copperfin::platform::path_from_utf8_string(
            result.plan.package_root);
        const fs::path retry_launcher_output = copperfin::platform::path_from_utf8_string(
            result.plan.launcher_output_path);
        const fs::path retry_launcher_dll = retry_package_root / "Copperfin.GeneratedLauncher.dll";
        const fs::path retry_launcher_deps = retry_package_root / "Copperfin.GeneratedLauncher.deps.json";
        const fs::path retry_launcher_runtimeconfig =
            retry_package_root / "Copperfin.GeneratedLauncher.runtimeconfig.json";
        write_text(retry_launcher_output, "published-launcher");

#if defined(_WIN32)
        write_text(retry_package_root / "Copperfin.GeneratedLauncher.apphost.exe", "launcher-apphost");
#endif
        write_text(retry_launcher_dll, "launcher-dll");
        write_text(retry_launcher_deps, "launcher-deps");
        write_text(retry_launcher_runtimeconfig, "launcher-runtimeconfig");
        const auto finalize_result = copperfin::runtime::finalize_runtime_package_primary_output(
            result.plan,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile());
        expect(finalize_result.ok,
               "#4052: dotnet finalization should succeed once the apphost and required sidecars exist: " +
                   finalize_result.error);
        if (finalize_result.ok) {
            expect(finalize_result.plan.primary_output_materialized,
                   "dotnet-finalize helper should mark the primary output as materialized");
            const std::string runtime_manifest = read_text(finalize_result.plan.manifest_path);
            const std::string debug_manifest = read_text(finalize_result.plan.debug_manifest_path);
            expect(runtime_manifest.find("primary_output_path=") == std::string::npos,
                   "dotnet-finalize runtime manifest should omit the primary output path after publish");
            expect(debug_manifest.find("primary_output_path=" + quote_manifest_value(finalize_result.plan.launcher_output_path)) != std::string::npos,
                   "dotnet-finalize debug manifest should preserve the primary output path after publish");
            expect(runtime_manifest.find("primary_output_materialized=") == std::string::npos,
                   "dotnet-finalize runtime manifest should omit the materialized launcher output state after publish");
            expect(debug_manifest.find("primary_output_materialized=true") != std::string::npos,
                   "dotnet-finalize debug manifest should report the materialized launcher output");
            expect(runtime_manifest.find("extension_payload=" + quote_manifest_value(finalize_result.plan.launcher_output_path) + "|") == std::string::npos,
                   "#4052: the post-launch runtime manifest must not claim to protect the launcher apphost");
            expect(debug_manifest.find("extension_payload=" + quote_manifest_value(finalize_result.plan.launcher_output_path) + "|") == std::string::npos,
                   "#4052: generated-launcher provenance should not use the runtime extension-payload surface");

            const std::vector<std::string> launcher_artifacts =
                lines_with_prefix(debug_manifest, "launcher_artifact=");
            const std::size_t expected_required_launcher_artifacts =
#if defined(_WIN32)
                5U;
#else
                4U;
#endif
            expect(finalize_result.plan.launcher_artifacts.size() == expected_required_launcher_artifacts,
                   "#4052: finalized package plans should retain the exact admitted launcher inventory");
            if (finalize_result.plan.launcher_artifacts.size() == expected_required_launcher_artifacts) {
                expect(finalize_result.plan.launcher_artifacts[0].package_relative_path ==
                           copperfin::platform::path_to_utf8_string(launcher_output.filename()) &&
                           finalize_result.plan.launcher_artifacts[0].role ==
                               copperfin::runtime::RuntimeLauncherArtifactRole::public_apphost,
                       "#4052: the first plan artifact should preserve the configured public apphost identity");
                expect(std::all_of(
                           finalize_result.plan.launcher_artifacts.begin() + 1,
                           finalize_result.plan.launcher_artifacts.end(),
                           [](const copperfin::runtime::RuntimeLauncherArtifact& artifact) {
                               return artifact.role ==
                                   copperfin::runtime::RuntimeLauncherArtifactRole::runtime_required;
                           }),
                       "#4052: the remaining required plan artifacts should use the runtime-required role");
            }
            expect(lines_with_prefix(runtime_manifest, "launcher_artifact=") == launcher_artifacts,
                   "#4052: runtime and debug manifests should expose the same provenance-only launcher inventory");
            expect(launcher_artifacts.size() == expected_required_launcher_artifacts,
                   "#4052: debug provenance should inventory one apphost and three required internal sidecars");
            expect(debug_manifest.find(
                       "launcher_artifact=" + quote_manifest_value(
                           copperfin::platform::path_to_utf8_string(launcher_output.filename())) +
                       "|public_apphost|") != std::string::npos,
                   "#3873: debug provenance should preserve and classify the Unicode public apphost");
            for (const auto& required_name : {
                     std::string("Copperfin.GeneratedLauncher.dll"),
                     std::string("Copperfin.GeneratedLauncher.deps.json"),
                     std::string("Copperfin.GeneratedLauncher.runtimeconfig.json")}) {
                expect(debug_manifest.find(
                           "launcher_artifact=" + quote_manifest_value(required_name) +
                           "|runtime_required|") != std::string::npos,
                       "#4052: debug provenance should classify required internal sidecar " + required_name);
            }
            for (const auto& artifact_line : launcher_artifacts) {
                expect(artifact_line.find(package_root.string()) == std::string::npos &&
                           artifact_line.find(project_dir.string()) == std::string::npos,
                       "#4052: launcher provenance should contain package-relative paths without source roots");
            }

            write_text(launcher_pdb, "launcher-pdb");
            auto repeated_plan = finalize_result.plan;
            const fs::path stale_identity = package_root / "Copperfin.GeneratedLauncher.retired";
            repeated_plan.extension_payload_digests.push_back({
                .path = stale_identity.string(),
                .sha256 = "stale-digest"
            });
            const auto with_optional_debug = copperfin::runtime::finalize_runtime_package_primary_output(
                repeated_plan,
                copperfin::security::default_native_security_profile(),
                copperfin::platform::default_extensibility_profile());
            expect(with_optional_debug.ok,
                   "#4052: repeated finalization should admit an optional launcher PDB");
            if (with_optional_debug.ok) {
                const std::string optional_debug_manifest = read_text(with_optional_debug.plan.debug_manifest_path);
                expect(lines_with_prefix(optional_debug_manifest, "launcher_artifact=").size() ==
#if defined(_WIN32)
                           6U &&
#else
                           5U &&
#endif
                           optional_debug_manifest.find(
                               "launcher_artifact=Copperfin.GeneratedLauncher.pdb|debug_optional|") != std::string::npos,
                       "#4052: optional debug artifacts should be classified separately and exactly once");
                expect(optional_debug_manifest.find(stale_identity.string()) == std::string::npos,
                       "#4052: repeated finalization should remove stale launcher identities from provenance");

                fs::remove(launcher_pdb, ignored);
                const auto without_optional_debug = copperfin::runtime::finalize_runtime_package_primary_output(
                    with_optional_debug.plan,
                    copperfin::security::default_native_security_profile(),
                    copperfin::platform::default_extensibility_profile());
                expect(without_optional_debug.ok,
                       "#4052: repeated finalization should succeed after an optional PDB is removed");
                if (without_optional_debug.ok) {
                    const std::string no_optional_debug_manifest =
                        read_text(without_optional_debug.plan.debug_manifest_path);
                    expect(lines_with_prefix(no_optional_debug_manifest, "launcher_artifact=").size() ==
#if defined(_WIN32)
                               5U &&
#else
                               4U &&
#endif
                           no_optional_debug_manifest.find("Copperfin.GeneratedLauncher.pdb") == std::string::npos,
                           "#4052: repeated finalization should not retain stale optional-debug inventory");
                }
            }

            const fs::path unexpected_sidecar = package_root / "Copperfin.GeneratedLauncher.stale.json";
            write_text(unexpected_sidecar, "unexpected");
            const auto unexpected_result = copperfin::runtime::finalize_runtime_package_primary_output(
                finalize_result.plan,
                copperfin::security::default_native_security_profile(),
                copperfin::platform::default_extensibility_profile());
            expect(!unexpected_result.ok,
                   "#4052: unexpected stale internal launcher sidecars should fail exact inventory admission");
            fs::remove(unexpected_sidecar, ignored);

            fs::remove(launcher_deps, ignored);
            const auto missing_required = copperfin::runtime::finalize_runtime_package_primary_output(
                finalize_result.plan,
                copperfin::security::default_native_security_profile(),
                copperfin::platform::default_extensibility_profile());
            expect(!missing_required.ok,
                   "#4052: removing a required sidecar should fail repeated finalization");
            write_text(launcher_deps, "launcher-deps-restored");

            fs::remove(launcher_dll, ignored);
            fs::create_directory(launcher_dll, ignored);
            const auto directory_sidecar = copperfin::runtime::finalize_runtime_package_primary_output(
                finalize_result.plan,
                copperfin::security::default_native_security_profile(),
                copperfin::platform::default_extensibility_profile());
            expect(!directory_sidecar.ok,
                   "#4052: a required sidecar directory should not pass direct-regular-file admission");
            fs::remove(launcher_dll, ignored);
            write_text(launcher_dll, "launcher-dll-restored");

            const fs::path external_sidecar = temp_root / "external-launcher-sidecar";
            write_text(external_sidecar, "external-sidecar");
            fs::remove(launcher_dll, ignored);
            fs::create_symlink(external_sidecar, launcher_dll, ignored);
            if (!ignored) {
                const auto redirected_sidecar = copperfin::runtime::finalize_runtime_package_primary_output(
                    finalize_result.plan,
                    copperfin::security::default_native_security_profile(),
                    copperfin::platform::default_extensibility_profile());
                expect(!redirected_sidecar.ok,
                       "#4052: a redirected required sidecar should fail direct-file admission");
                fs::remove(launcher_dll, ignored);
            } else {
                ignored.clear();
                fs::remove(launcher_dll, ignored);
            }
            write_text(launcher_dll, "launcher-dll-restored-again");

            const fs::path ambiguous_dll = package_root / "COPPERFIN.GENERATEDLAUNCHER.DLL";
            write_text(ambiguous_dll, "ambiguous-launcher-dll");
            std::error_code equivalent_error;
            const bool distinct_case_entries =
                !fs::equivalent(launcher_dll, ambiguous_dll, equivalent_error) && !equivalent_error;
            if (distinct_case_entries) {
                const auto ambiguous_sidecar = copperfin::runtime::finalize_runtime_package_primary_output(
                    finalize_result.plan,
                    copperfin::security::default_native_security_profile(),
                    copperfin::platform::default_extensibility_profile());
                expect(!ambiguous_sidecar.ok,
                       "#4052: case-fold duplicate internal sidecars should fail deterministic inventory admission");
                fs::remove(ambiguous_dll, ignored);
            }

            const fs::path external_apphost = temp_root / "external-launcher-apphost";
            write_text(external_apphost, "external-apphost");
            fs::remove(launcher_output, ignored);
            fs::create_symlink(external_apphost, launcher_output, ignored);
            if (!ignored) {
                const auto redirected_apphost = copperfin::runtime::finalize_runtime_package_primary_output(
                    finalize_result.plan,
                    copperfin::security::default_native_security_profile(),
                    copperfin::platform::default_extensibility_profile());
                expect(!redirected_apphost.ok,
                       "#4052: a redirected public apphost should fail direct-file admission");
            }

            const auto rematerialized = copperfin::runtime::materialize_runtime_package(
                finalize_result.plan,
                copperfin::security::default_native_security_profile(),
                copperfin::platform::default_extensibility_profile(),
                runtime_host.string());
            expect(rematerialized.ok,
                   "#4052: a previously finalized launcher plan should rematerialize into a fresh pre-publish package");
            if (rematerialized.ok) {
                expect(!rematerialized.plan.primary_output_materialized &&
                           rematerialized.plan.launcher_artifacts.empty(),
                       "#4052: rematerialization should clear launcher materialization state and inventory");
                expect(!fs::exists(copperfin::platform::path_from_utf8_string(
                               rematerialized.plan.launcher_output_path)) &&
                           !fs::exists(copperfin::platform::path_from_utf8_string(
                               rematerialized.plan.package_root) /
                                       "Copperfin.GeneratedLauncher.dll") &&
                           !fs::exists(copperfin::platform::path_from_utf8_string(
                               rematerialized.plan.package_root) /
                                       "Copperfin.GeneratedLauncher.deps.json") &&
                           !fs::exists(copperfin::platform::path_from_utf8_string(
                               rematerialized.plan.package_root) /
                                       "Copperfin.GeneratedLauncher.runtimeconfig.json"),
                       "#4052: rematerialization should remove the previous launcher publish set");
                const std::string rematerialized_debug =
                    read_text(rematerialized.plan.debug_manifest_path);
                expect(lines_with_prefix(rematerialized_debug, "launcher_artifact=").empty() &&
                           rematerialized_debug.find("primary_output_materialized=false") != std::string::npos,
                       "#4052: fresh pre-publish debug metadata should not retain stale launcher provenance");
            }
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_deferred_package_transaction_rolls_back_failed_second_build() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_runtime_pipeline_deferred_transaction";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    write_text(project_dir / "main.prg", "RETURN\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "deferred_transaction.pjx").string();
    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "DeferredTransaction";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = workspace.project_title;
    workspace.build_plan.output_path =
        (output_dir / "DeferredTransaction.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U,
         .name = "main.prg",
         .relative_path = "main.prg",
         .type_title = "Program"}
    };

    const auto security_profile = copperfin::security::default_native_security_profile();
    const auto extensibility_profile = copperfin::platform::default_extensibility_profile();
    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        security_profile,
        extensibility_profile,
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        true);
    expect(plan.ok, "#4053: deferred-transaction plan should be created");
    if (!plan.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const auto materialize_and_publish = [&](const copperfin::runtime::RuntimePackagePlan& input,
                                             const std::string& suffix) {
        auto materialized = copperfin::runtime::materialize_runtime_package(
            input,
            security_profile,
            extensibility_profile,
            runtime_host.string());
        expect(materialized.ok, "#4053: generated-launcher build should materialize");
        if (!materialized.ok) {
            return materialized;
        }
        const fs::path package_root(materialized.plan.package_root);
        write_text(materialized.plan.launcher_output_path, "published-" + suffix);
#if defined(_WIN32)
        write_text(
            package_root / "Copperfin.GeneratedLauncher.apphost.exe",
            "launcher-apphost-" + suffix);
#endif
        write_text(package_root / "Copperfin.GeneratedLauncher.dll", "launcher-dll-" + suffix);
        write_text(package_root / "Copperfin.GeneratedLauncher.deps.json", "launcher-deps-" + suffix);
        write_text(
            package_root / "Copperfin.GeneratedLauncher.runtimeconfig.json",
            "launcher-runtimeconfig-" + suffix);
        return materialized;
    };

    const auto first_materialized = materialize_and_publish(plan, "first");
    if (!first_materialized.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }
    const auto first_finalized = copperfin::runtime::finalize_runtime_package_primary_output(
        first_materialized.plan,
        security_profile,
        extensibility_profile);
    expect(first_finalized.ok, "#4053: initial generated-launcher build should finalize");
    if (!first_finalized.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }
    const std::string known_good_runtime = read_text(first_finalized.plan.manifest_path);
    const std::string known_good_debug = read_text(first_finalized.plan.debug_manifest_path);

    const auto second_materialized = copperfin::runtime::materialize_runtime_package(
        plan,
        security_profile,
        extensibility_profile,
        runtime_host.string());
    expect(second_materialized.ok, "#4053: second generated-launcher build should materialize");
    if (second_materialized.ok) {
        const auto failed_finalize = copperfin::runtime::finalize_runtime_package_primary_output(
            second_materialized.plan,
            security_profile,
            extensibility_profile);
        expect(!failed_finalize.ok,
               "#4053: second-build finalization should fail without published launcher artifacts");
        expect(read_text(second_materialized.plan.manifest_path) == known_good_runtime &&
                   read_text(second_materialized.plan.debug_manifest_path) == known_good_debug,
               "#4053: failed finalization should restore the last known-good manifest pair");
        expect(!fs::exists(second_materialized.plan.package_root + ".copperfin-previous") &&
                   !fs::exists(second_materialized.plan.package_root + ".copperfin-materializing"),
               "#4053: failed finalization should remove package transaction artifacts");
    }

    const auto third_materialized = copperfin::runtime::materialize_runtime_package(
        plan,
        security_profile,
        extensibility_profile,
        runtime_host.string());
    expect(third_materialized.ok, "#4053: package should be replaceable after finalization rollback");
    if (third_materialized.ok) {
        const auto aborted = copperfin::runtime::abort_runtime_package_transaction(
            third_materialized.plan);
        expect(aborted.ok, "#4053: publish failure abort should restore the previous package");
        expect(read_text(third_materialized.plan.manifest_path) == known_good_runtime &&
                   read_text(third_materialized.plan.debug_manifest_path) == known_good_debug,
               "#4053: publish abort should restore the last known-good manifest pair");
        expect(!fs::exists(third_materialized.plan.package_root + ".copperfin-previous") &&
                   !fs::exists(third_materialized.plan.package_root + ".copperfin-materializing"),
               "#4053: publish abort should remove package transaction artifacts");
    }

    const auto final_materialized = materialize_and_publish(plan, "final");
    if (final_materialized.ok) {
        const auto final_result = copperfin::runtime::finalize_runtime_package_primary_output(
            final_materialized.plan,
            security_profile,
            extensibility_profile);
        expect(final_result.ok,
               "#4053: package should finalize successfully after rollback and abort");
    }

    fs::remove_all(temp_root, ignored);
}

void test_primary_output_status_errors_are_reported_as_missing() {
#if !defined(_WIN32)
    namespace fs = std::filesystem;
    const ScopedEnvironmentVariable scoped_locale("COPPERFIN_LOCALE", "en-US");
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_primary_output_status";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path primary_output = temp_root / "primary.fxp";
    fs::create_symlink(primary_output, primary_output, ignored);
    if (!ignored) {
        copperfin::runtime::RuntimePackagePlan plan;
        plan.ok = true;
        plan.package_root = temp_root.string();
        plan.launcher_output_path = primary_output.string();
        plan.output_kind = copperfin::runtime::BuildOutputKind::fxp;
        plan.emit_dotnet_launcher = false;

        const auto result = copperfin::runtime::finalize_runtime_package_primary_output(
            plan,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile());
        expect(!result.ok,
               "#4400: primary-output status errors should fail package finalization");
        expect(result.error == runtime_pipeline_english_catalog().translate(
                                  "Runtime.Package.Error.PrimaryOutputMissing"),
               "#4400: primary-output status errors should preserve the localized missing-output diagnostic");
    }

    fs::remove_all(temp_root, ignored);
#endif
}

void test_package_output_names_reject_reserved_artifacts() {
    namespace fs = std::filesystem;
    const ScopedEnvironmentVariable scoped_locale("COPPERFIN_LOCALE", "en-US");
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_reserved_output_name";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    write_text(project_dir / "main.prg", "RETURN\n");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "reserved_output_name.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "ReservedOutputName";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = workspace.project_title;
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"}
    };

    auto extensibility_profile = copperfin::platform::default_extensibility_profile();
    extensibility_profile.dotnet_output.available = true;
    const auto make_plan = [&](const std::string& output_name,
                               const std::string& output_kind,
                               const bool emit_dotnet_launcher) {
        workspace.build_plan.output_path = (output_dir / output_name).string();
        workspace.build_plan.output_kind = output_kind;
        return copperfin::runtime::create_runtime_package_plan(
            document,
            workspace,
            copperfin::security::default_native_security_profile(),
            extensibility_profile,
            output_dir.string(),
            copperfin::runtime::BuildConfiguration::debug,
            false,
            emit_dotnet_launcher);
    };

#if defined(_WIN32)
    const std::string runtime_host_name = "copperfin_runtime_host.exe";
    const std::string alternate_platform_host_name = "copperfin_runtime_host";
#else
    const std::string runtime_host_name = "copperfin_runtime_host";
    const std::string alternate_platform_host_name = "copperfin_runtime_host.exe";
#endif
    const auto assert_rejected = [&](const std::string& output_name,
                                     const std::string& reserved_name,
                                     const bool emit_dotnet_launcher) {
        const auto plan = make_plan(output_name, "executable", emit_dotnet_launcher);
        const std::string expected_error =
            "Package output name conflicts with a reserved Copperfin artifact: " + output_name +
            " conflicts with " + reserved_name;
        expect(!plan.ok,
               "#4054: package planning should reject reserved public output names");
        expect(plan.warnings.size() == 1U && plan.warnings.front() == expected_error,
               "#4054: planning should report the localized reserved-output diagnostic");

        const auto materialize_result = copperfin::runtime::materialize_runtime_package(
            plan,
            copperfin::security::default_native_security_profile(),
            extensibility_profile,
            "unused-runtime-host");
        expect(!materialize_result.ok && materialize_result.error == expected_error,
               "#4054: materialization should reject reserved output names before changing the package root");
        expect(!fs::exists(plan.package_root),
               "#4054: rejected package planning must not create a package root");

        const auto finalize_result = copperfin::runtime::finalize_runtime_package_primary_output(
            plan,
            copperfin::security::default_native_security_profile(),
            extensibility_profile);
        expect(!finalize_result.ok && finalize_result.error == expected_error,
               "#4054: primary-output publication should reject reserved output names");

        const auto build_result = copperfin::runtime::build_runtime_package_primary_output(
            plan,
            copperfin::security::default_native_security_profile(),
            extensibility_profile);
        expect(!build_result.ok && build_result.error == expected_error,
               "#4054: native wrapper publication should reject reserved output names");
    };

    assert_rejected(runtime_host_name, runtime_host_name, false);
    assert_rejected("COPPERFIN_RUNTIME_HOST" + std::string(runtime_host_name.ends_with(".exe") ? ".EXE" : ""),
                    runtime_host_name,
                    false);
    assert_rejected("Copperfin.GeneratedLauncher.dll", "Copperfin.GeneratedLauncher.dll", true);
    assert_rejected("COPPERFIN.GENERATEDLAUNCHER.DEPS.JSON",
                    "Copperfin.GeneratedLauncher.deps.json",
                    true);

    expect(make_plan("CustomerApp.exe", "executable", true).ok,
           "#4054: noncolliding custom executable output names should remain valid");
    expect(make_plan("Copperfin.GeneratedLauncher.exe", "executable", true).ok,
           "#4054: the configured public apphost identity should remain valid");
    expect(make_plan(alternate_platform_host_name, "executable", false).ok,
           "#4054: the other platform runtime-host filename should remain valid");
    expect(make_plan(runtime_host_name, "app", false).ok,
           "#4054: output modes that do not bundle a runtime host should not reserve its filename");

    fs::remove_all(temp_root, ignored);
}

void test_runtime_package_diagnostics_resolve_through_localization_catalog() {
    const auto catalog_root = runtime_pipeline_locale_root();
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
        "Runtime.Package.Error.AmbiguousProjectAssetPath",
        "Runtime.Package.Error.AmbiguousCompanionPath",
        "Runtime.Package.Error.ContentDestinationRejected",
        "Runtime.Package.Error.ContentRootRejected",
        "Runtime.Package.Error.CopyFileFailed",
        "Runtime.Package.Error.CreateContentRootFailed",
        "Runtime.Package.Error.CreateDirectoryFailed",
        "Runtime.Package.Error.CreateFileFailed",
        "Runtime.Package.Error.CreateLauncherDirectoryFailed",
        "Runtime.Package.Error.CreateNativeWrapperBuildDirectoryFailed",
        "Runtime.Package.Error.CreateNativeWrapperDirectoryFailed",
        "Runtime.Package.Error.CreatePackageRootFailed",
        "Runtime.Package.Error.LauncherArtifactAmbiguous",
        "Runtime.Package.Error.LauncherArtifactMissing",
        "Runtime.Package.Error.LauncherArtifactNotDirectRegularFile",
        "Runtime.Package.Error.LauncherArtifactUnexpected",
        "Runtime.Package.Error.OutputNameReserved",
        "Runtime.Package.Error.ManifestPairPathRejected",
        "Runtime.Package.Error.ManifestPairPublishFailed",
        "Runtime.Package.Error.ManifestPairRollbackFailed",
        "Runtime.Package.Error.ManifestPairStageFailed",
        "Runtime.Package.Error.ManifestPairTransactionCollision",
        "Runtime.Package.Error.NativeWrapperCMakeMissing",
        "Runtime.Package.Error.NativeWrapperPrimaryOutputBuildFailed",
        "Runtime.Package.Error.NativeWrapperPrimaryOutputConfigureFailed",
        "Runtime.Package.Error.NativeWrapperPrimaryOutputMissing",
        "Runtime.Package.Error.OpenFileFailed",
        "Runtime.Package.Error.PackageRollbackFailed",
        "Runtime.Package.Error.PackageTransactionStartFailed",
        "Runtime.Package.Error.PlanInvalid",
        "Runtime.Package.Error.PrimaryOutputMissing",
        "Runtime.Package.Error.PrimaryOutputRequiresLibraryOutput",
        "Runtime.Package.Error.RuntimeHostSourcePathEmpty",
        "Runtime.Package.Error.RuntimeHostSourcePathNotRegularFile",
        "Runtime.Package.Error.RuntimeHostSourcePathResolveFailed",
        "Runtime.Package.Error.SecurityRequiresAbsoluteRuntimeHostPath",
        "Runtime.Package.Error.SecurityRequiresCanonicalRuntimeHostName",
        "Runtime.Package.Error.SourceFileMissing",
        "Runtime.Package.Error.WriteFileFailed",
        "Runtime.Package.Warning.ManifestPairRewriteFailed"};

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
        english_catalog.translate(
            "Runtime.Package.Error.LauncherArtifactMissing",
            {{"path", "Copperfin.GeneratedLauncher.dll"}}) ==
            "Required generated-launcher artifact is missing: Copperfin.GeneratedLauncher.dll",
        "#4052: generated-launcher admission diagnostics should preserve package-relative path placeholders");
    expect(
        english_catalog.translate(
            "Runtime.Package.Error.OutputNameReserved",
            {{"outputName", "COPPERFIN.GENERATEDLAUNCHER.DLL"},
             {"reservedName", "Copperfin.GeneratedLauncher.dll"}}) ==
            "Package output name conflicts with a reserved Copperfin artifact: "
            "COPPERFIN.GENERATEDLAUNCHER.DLL conflicts with Copperfin.GeneratedLauncher.dll",
        "#4054: reserved-output diagnostics should preserve the public and reserved artifact identities");
    expect(
        pseudo_catalog.translate(
            "Runtime.Package.Error.OutputNameReserved",
            {{"outputName", "COPPERFIN.GENERATEDLAUNCHER.DLL"},
             {"reservedName", "Copperfin.GeneratedLauncher.dll"}}) ==
            copperfin::localization::format_named_placeholders(
                copperfin::localization::pseudo_localize(
                    "Package output name conflicts with a reserved Copperfin artifact: "
                    "{outputName} conflicts with {reservedName}"),
                {{"outputName", "COPPERFIN.GENERATEDLAUNCHER.DLL"},
                 {"reservedName", "Copperfin.GeneratedLauncher.dll"}}),
        "#4054: reserved-output diagnostics should route through pseudo-localization");
    expect(
        english_catalog.translate(
            "Runtime.Package.Error.ContentDestinationRejected",
            {{"path", "content/forms/customer.scx"}}) ==
            "Package asset destination must be a direct path inside the content root: content/forms/customer.scx",
        "#4065: package-content rejection diagnostics should preserve path placeholders");
    expect(
        pseudo_catalog.translate(
            "Runtime.Package.Error.ContentDestinationRejected",
            {{"path", "content/forms/customer.scx"}}) ==
            copperfin::localization::format_named_placeholders(
                copperfin::localization::pseudo_localize(
                    "Package asset destination must be a direct path inside the content root: {path}"),
                {{"path", "content/forms/customer.scx"}}),
        "#4065: package-content rejection diagnostics should route through pseudo-localization");
    expect(
        spanish_catalog.translate(
            "Runtime.Package.Error.ContentRootRejected",
            {{"path", "outside/content"}}) ==
            "La raiz de contenido del paquete debe ser el directorio de contenido directo dentro de la raiz del paquete: outside/content",
        "#4065: content-root rejection diagnostics should localize without changing path placeholders");
    expect(
        spanish_catalog.translate(
            "Runtime.Package.Error.ManifestPairPublishFailed",
            {{"path", "app.cfdebug"}}) ==
            "No se pudo publicar atomicamente el par de manifiestos del paquete: app.cfdebug",
        "#4056: manifest-pair publication diagnostics should localize without changing path placeholders");
    expect(
        pseudo_catalog.translate(
            "Runtime.Package.Error.ManifestPairRollbackFailed",
            {{"path", "package-root"}}) ==
            copperfin::localization::format_named_placeholders(
                copperfin::localization::pseudo_localize(
                    "Unable to restore the previous package manifest pair: {path}"),
                {{"path", "package-root"}}),
        "#4056: manifest-pair rollback diagnostics should route through pseudo-localization");
    expect(
        english_catalog.translate(
            "Runtime.Package.Error.ManifestPairPathRejected",
            {{"path", "app.cfmanifest"}}) ==
            "Runtime and debug manifest destinations must be direct regular files in the package root: app.cfmanifest",
        "#4056: manifest-pair path diagnostics should preserve the rejected path placeholder");
    expect(
        portuguese_catalog.translate(
            "Runtime.Package.Error.ManifestPairStageFailed",
            {{"path", "app.cfmanifest.next"}}) ==
            "Nao foi possivel preparar o par de manifestos do pacote: app.cfmanifest.next",
        "#4056: manifest-pair staging diagnostics should localize without changing path placeholders");
    expect(
        pseudo_catalog.translate(
            "Runtime.Package.Error.ManifestPairTransactionCollision",
            {{"path", "reserved-entry"}}) ==
            copperfin::localization::format_named_placeholders(
                copperfin::localization::pseudo_localize(
                    "A reserved manifest-transaction path is not owned by Copperfin: {path}"),
                {{"path", "reserved-entry"}}),
        "#4056: manifest-pair collision diagnostics should route through pseudo-localization");
    expect(
        english_catalog.translate(
            "Runtime.Package.Warning.ManifestPairRewriteFailed",
            {{"path", "package-root"}}) ==
            "The new package is live, but its runtime and debug manifests could not be refreshed: package-root",
        "#4096: post-commit manifest rewrite warnings should preserve their explicit warning contract");
    expect(
        pseudo_catalog.translate(
            "Runtime.Package.Warning.ManifestPairRewriteFailed",
            {{"path", "package-root"}}) ==
            copperfin::localization::format_named_placeholders(
                copperfin::localization::pseudo_localize(
                    "The new package is live, but its runtime and debug manifests could not be refreshed: {path}"),
                {{"path", "package-root"}}),
        "#4096: post-commit manifest rewrite warnings should route through pseudo-localization");
    expect(
        english_catalog.translate("Runtime.Package.Error.SourceFileMissing", {{"path", "missing.prg"}}) ==
            "Source file does not exist: missing.prg",
        "#2390: runtime package diagnostics should preserve path placeholders");
    expect(
        english_catalog.translate(
            "Runtime.Package.Error.AmbiguousProjectAssetPath",
            {{"path", "forms/customer.scx"}}) ==
            "Multiple case-insensitive project asset paths match: forms/customer.scx",
        "#3953: ambiguous project-asset diagnostics should preserve path placeholders");
    expect(
        english_catalog.translate(
            "Runtime.Package.Error.AmbiguousCompanionPath",
            {{"path", "forms/customer.sct"}}) ==
            "Multiple case-insensitive companion files match: forms/customer.sct",
        "#3905: ambiguous companion diagnostics should preserve path placeholders");
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
        pseudo_catalog.translate(
            "Runtime.Package.Error.AmbiguousProjectAssetPath",
            {{"path", "forms/customer.scx"}}) !=
            english_catalog.translate(
                "Runtime.Package.Error.AmbiguousProjectAssetPath",
                {{"path", "forms/customer.scx"}}),
        "#3953: ambiguous project-asset diagnostics should be pseudo-localizable");
    expect(
        pseudo_catalog.translate(
            "Runtime.Package.Error.AmbiguousCompanionPath",
            {{"path", "forms/customer.sct"}}) !=
            english_catalog.translate(
                "Runtime.Package.Error.AmbiguousCompanionPath",
                {{"path", "forms/customer.sct"}}),
        "#3905: ambiguous companion diagnostics should be pseudo-localizable");
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

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
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
    set_env_value("COPPERFIN_LOCALE", "en-US", true);
    const auto build_result = copperfin::runtime::build_runtime_package_primary_output(
        executable_plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile());
    expect(!build_result.ok, "primary-output build should reject executable package plans");
    expect(
        build_result.error == "Primary-output builds are only supported for library-output packages.",
        "#2390: primary-output build should preserve the default localized unsupported-kind diagnostic");

    set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
    const auto portuguese_build_result = copperfin::runtime::build_runtime_package_primary_output(
        executable_plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile());
    expect(
        !portuguese_build_result.ok &&
            portuguese_build_result.error ==
                "Builds de saida primaria sao suportados apenas para pacotes de saida de biblioteca.",
        "#2606: runtime package primary-output diagnostics should refresh to pt-BR when the runtime locale changes in-process");

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
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
