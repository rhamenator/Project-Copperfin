// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_runtime_pipeline_output_packaging_support.h"

namespace cf_test_runtime_pipeline {

void test_runtime_package_emits_csharp_transpilation_for_class_library_objects() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_csharp_xasset_contract";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    const fs::path class_library_path = project_dir / "widget.vcx";
    write_synthetic_class_library_asset(class_library_path);
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "widgetdemo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "WidgetDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "WidgetDemo";
    workspace.build_plan.output_path = (output_dir / "WidgetDemo.exe").string();
    workspace.build_plan.output_kind = "executable";
    workspace.build_plan.build_target = "x64 Windows executable";
    workspace.build_plan.startup_item = "widget.vcx";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "widget.vcx", .relative_path = "widget.vcx", .type_title = "Class Library"}
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

    expect(plan.ok, "class-library csharp-output plan should be created");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect_materialization(result, "class-library csharp-output package should materialize");
    if (result.ok) {
        expect(fs::exists(result.plan.transpiled_csharp_path),
               "class-library csharp-output package should emit a C# transpilation artifact");

        const std::string transpiled = read_text(result.plan.transpiled_csharp_path);
        expect(transpiled.find("public sealed class CustWidget") != std::string::npos,
               "class-library transpilation should emit a concrete C# type for the root object");
        expect(transpiled.find("public void Load()") != std::string::npos,
               "class-library transpilation should surface the root Load lifecycle method");
        expect(transpiled.find("public void Init()") != std::string::npos,
               "class-library transpilation should surface the root Init lifecycle method");
        expect(transpiled.find("public void Destroy()") != std::string::npos,
               "class-library transpilation should surface the root Destroy lifecycle method");
        expect(transpiled.find("public void TxtName_Valid()") != std::string::npos,
               "class-library transpilation should surface nested object methods");
        expect(transpiled.find("public void RunStartup()") != std::string::npos,
               "class-library transpilation should emit an ordered startup wrapper");
        expect(transpiled.find("Load();") != std::string::npos &&
               transpiled.find("Init();") != std::string::npos,
               "class-library transpilation should preserve root startup ordering");
        expect(transpiled.find("public void RunShutdown()") != std::string::npos,
               "class-library transpilation should emit an ordered shutdown wrapper");
        expect(transpiled.find("Destroy();") != std::string::npos,
               "class-library transpilation should preserve root shutdown ordering");
        expect(
            transpiled.find("GeneratedLocalization.Translate(\"Runtime.Package.Transpilation.Error.ManualPortRequiredForXAssetMethod\"") != std::string::npos &&
                transpiled.find("[\"methodIdentity\"] = \"custWidget.txtName.Valid\"") != std::string::npos,
            "class-library transpilation should route xAsset manual-port exceptions through localization while preserving method identity");

        if (dotnet_is_available()) {
            std::string compile_error;
            const bool compiled = compile_csharp_artifact(result.plan.transpiled_csharp_path, compile_error);
            if (!compiled && !compile_error.empty()) {
                std::cerr << "FAIL: " << compile_error << "\n";
            }
            expect(compiled,
                   "class-library csharp transpilation should compile under dotnet");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_manifest_records_generated_compiler_contract_digests() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_compiler_contract_digests";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg",
               "LOCAL nValue\n"
               "nValue = 1\n"
               "RETURN\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "contractdigests.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "ContractDigests";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "ContractDigests";
    workspace.build_plan.output_path = (output_dir / "ContractDigests.exe").string();
    workspace.build_plan.output_kind = "executable";
    workspace.build_plan.build_target = "x64 Windows executable";
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

    expect(plan.ok, "compiler-contract-digest plan should be created");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect_materialization(result, "compiler-contract-digest package should materialize");
    if (result.ok) {
        const auto has_digest = [&](const std::string& path) {
            return std::find_if(
                       result.plan.compiler_contract_digests.begin(),
                       result.plan.compiler_contract_digests.end(),
                       [&](const copperfin::runtime::RuntimeArtifactDigest& digest) {
                           return digest.path == path && !digest.sha256.empty();
                       }) != result.plan.compiler_contract_digests.end();
        };

        expect(has_digest(result.plan.ast_manifest_path),
               "compiler-contract digests should include the AST artifact");
        expect(has_digest(result.plan.ir_manifest_path),
               "compiler-contract digests should include the IR artifact");
        expect(has_digest(result.plan.transpiled_csharp_path),
               "compiler-contract digests should include the transpiled C# artifact");

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        for (const auto& digest : result.plan.compiler_contract_digests) {
            expect(runtime_manifest.find("compiler_contract=" + quote_manifest_value(digest.path) + "|" + quote_manifest_value(digest.sha256)) == std::string::npos,
                   "runtime manifest should omit compiler-contract digests from the execution contract");
            expect(debug_manifest.find("compiler_contract=" + quote_manifest_value(digest.path) + "|" + quote_manifest_value(digest.sha256)) != std::string::npos,
                   "debug manifest should record each generated compiler-contract digest");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_manifest_asset_lines_include_copy_state_contract() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_manifest_asset_copy_state";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg", "RETURN\n");
    write_text(project_dir / "excluded.txt", "do not stage");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "manifest_contract.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "ManifestAssetContract";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "ManifestAssetContract";
    workspace.build_plan.output_path = (output_dir / "ManifestAssetContract.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"},
        {.record_index = 2U, .name = "excluded.txt", .relative_path = "excluded.txt", .type_title = "Text", .excluded = true}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        false);

    expect(plan.ok, "manifest-asset-copy-state plan should be created");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect_materialization(result, "manifest-asset-copy-state package should materialize");
    if (result.ok) {
        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string startup_line_marker = "asset=1|main.prg|";
        const std::string excluded_line_marker = "asset=2|excluded.txt|";
        const std::size_t startup_line_pos = runtime_manifest.find(startup_line_marker);
        const std::size_t excluded_line_pos = runtime_manifest.find(excluded_line_marker);
        expect(startup_line_pos != std::string::npos,
               "runtime manifest should include startup asset line");
        expect(excluded_line_pos != std::string::npos,
               "runtime manifest should include excluded asset line");

        const bool startup_copied = startup_line_pos != std::string::npos &&
            runtime_manifest.find("|true\n", startup_line_pos) != std::string::npos;
        const bool excluded_not_copied = excluded_line_pos != std::string::npos &&
            runtime_manifest.find("|false\n", excluded_line_pos) != std::string::npos;
        expect(startup_copied, "startup asset line should report copied=true in manifest contract");
        expect(excluded_not_copied, "excluded non-runtime asset line should report copied=false in manifest contract");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_package_stages_recursive_prg_include_dependencies() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_prg_include_dependencies";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir / "include");

    write_text(project_dir / "main.prg", "#include \"include\\shared.h\"\nRETURN\n");
    // VFP projects sometimes retain an include directory in the directive
    // even when the shipped header is beside the owning PRG.
    write_text(project_dir / "shared.h", "#include \"nested.h\"\n#DEFINE SHARED_VALUE 42\n");
    write_text(project_dir / "nested.h", "#DEFINE NESTED_VALUE 7\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "include_dependencies.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "IncludeDependencies";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "IncludeDependencies";
    workspace.build_plan.output_path = (output_dir / "IncludeDependencies.exe").string();
    workspace.build_plan.output_kind = "executable";
    workspace.build_plan.build_target = "x64 Windows executable";
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

    expect(plan.ok, "PRG include dependency plan should be created");
    const auto has_asset = [&](const std::string& relative_path) {
        return std::find_if(
                   plan.assets.begin(),
                   plan.assets.end(),
                   [&](const copperfin::runtime::RuntimePackageAsset& asset) {
                       return asset.relative_path == relative_path && asset.type_title == "PRG Include";
                   }) != plan.assets.end();
    };
    expect(has_asset("shared.h"), "PRG include basename fallback should stage the direct header");
    expect(has_asset("nested.h"), "PRG include discovery should recurse through headers");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect_materialization(result, "PRG include dependency package should materialize");
    if (result.ok) {
        expect(fs::exists(fs::path(result.plan.content_root) / "shared.h"),
               "direct PRG include dependency should be staged under the content root");
        expect(fs::exists(fs::path(result.plan.content_root) / "nested.h"),
               "recursive PRG include dependency should be staged under the content root");
        const std::string manifest = read_text(result.plan.manifest_path);
        expect(manifest.find("|shared.h|") != std::string::npos &&
                   manifest.find("|nested.h|") != std::string::npos,
               "runtime manifest should identify staged PRG include dependencies");
    }

    fs::remove_all(temp_root, ignored);
}



}  // namespace cf_test_runtime_pipeline
