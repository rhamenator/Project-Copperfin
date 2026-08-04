// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_runtime_pipeline_output_packaging_support.h"

namespace cf_test_runtime_pipeline {

void test_runtime_package_emits_ast_manifest_for_prg_sources() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_ast_contract";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg",
               "LOCAL nValue\n"
               "nValue = 1\n"
               "DO worker\n"
               "RETURN\n"
               "PROCEDURE worker\n"
               "WAIT WINDOW 'ast\x1f" "control'\n"
               "RETURN\n"
               "ENDPROC\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "astdemo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "AstDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "AstDemo";
    workspace.build_plan.output_path = (output_dir / "AstDemo.exe").string();
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
        false);

    expect(plan.ok, "ast-output plan should be created");
    expect(fs::path(plan.ast_manifest_path).filename() == "AstDemo.exe.ast.json",
           "ast-output plan should derive a target-specific AST manifest filename");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect_materialization(result, "ast-output package should materialize");
    if (result.ok) {
        expect(fs::exists(result.plan.ast_manifest_path),
               "ast-output package should emit an AST manifest");

        const std::string ast_manifest = read_text(result.plan.ast_manifest_path);
        expect(ast_manifest.find("\"schema_version\": 1") != std::string::npos,
               "ast manifest should declare the schema version");
        expect(ast_manifest.find("\"project_title\": \"AstDemo\"") != std::string::npos,
               "ast manifest should record the project title");
        expect(ast_manifest.find("\"output_kind\": \"executable\"") != std::string::npos,
               "ast manifest should record the selected output kind");
        expect(ast_manifest.find("\"relative_path\": \"main.prg\"") != std::string::npos,
               "ast manifest should record the source-relative program path");
        expect(ast_manifest.find("\"name\": \"MAIN\"") != std::string::npos,
               "ast manifest should emit the MAIN routine");
        expect(ast_manifest.find("\"text\": \"DO worker\"") != std::string::npos,
               "ast manifest should preserve main-scope statement text");
        expect(ast_manifest.find("\"name\": \"worker\"") != std::string::npos,
               "ast manifest should emit named routines");
        expect(ast_manifest.find("\"text\": \"WAIT WINDOW 'ast\\u001fcontrol'\"") != std::string::npos,
               "ast manifest should canonically escape source control bytes");
        expect(ast_manifest.find('\x1f') == std::string::npos,
               "ast manifest should not contain a raw source control byte");

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        expect(debug_manifest.find("ast_manifest_path=" + quote_manifest_value(result.plan.ast_manifest_path)) != std::string::npos,
               "debug manifest should record the AST-manifest path");
        expect(runtime_manifest.find("ast_manifest_path=") == std::string::npos,
               "runtime manifest should omit the AST-manifest path");
        expect(lines_with_prefix(runtime_manifest, "feature_flag=").empty(),
               "runtime manifest should omit feature-flag inventory while trimming AST build metadata");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_package_emits_ir_manifest_with_instruction_mapping() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_ir_contract";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg",
               "LOCAL nValue\n"
               "nValue = 1\n"
               "DO worker\n"
               "DEFINE MENU MainMenu\n"
               "RETURN\n"
               "PROCEDURE worker\n"
               "WAIT WINDOW 'ir\x1f" "control'\n"
               "RETURN\n"
               "ENDPROC\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "irdemo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "IrDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "IrDemo";
    workspace.build_plan.output_path = (output_dir / "IrDemo.exe").string();
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
        false);

    expect(plan.ok, "ir-output plan should be created");
    expect(fs::path(plan.ir_manifest_path).filename() == "IrDemo.exe.ir.json",
           "ir-output plan should derive a target-specific IR manifest filename");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect_materialization(result, "ir-output package should materialize");
    if (result.ok) {
        expect(fs::exists(result.plan.ir_manifest_path),
               "ir-output package should emit an IR manifest");

        const std::string ir_manifest = read_text(result.plan.ir_manifest_path);
        expect(ir_manifest.find("\"schema_version\": 1") != std::string::npos,
               "ir manifest should declare the schema version");
        expect(ir_manifest.find("\"project_title\": \"IrDemo\"") != std::string::npos,
               "ir manifest should record the project title");
        expect(ir_manifest.find("\"output_kind\": \"executable\"") != std::string::npos,
               "ir manifest should record the selected output kind");
        expect(ir_manifest.find("\"relative_path\": \"main.prg\"") != std::string::npos,
               "ir manifest should record the source-relative program path");
        expect(ir_manifest.find("\"name\": \"MAIN\"") != std::string::npos,
               "ir manifest should emit the MAIN routine");
        expect(ir_manifest.find("\"opcode\": \"local_declaration\"") != std::string::npos,
               "ir manifest should map LOCAL statements to a stable opcode");
        expect(ir_manifest.find("\"opcode\": \"assignment\"") != std::string::npos,
               "ir manifest should map assignments to a stable opcode");
        expect(ir_manifest.find("\"opcode\": \"do_command\"") != std::string::npos,
               "ir manifest should map DO statements to a stable opcode");
        expect(ir_manifest.find("\"opcode\": \"define_menu_command\"") != std::string::npos,
               "ir manifest should map DEFINE MENU statements to a stable opcode");
        expect(ir_manifest.find("\"opcode\": \"wait_command\"") != std::string::npos,
               "ir manifest should map WAIT WINDOW statements to a stable opcode");
        expect(ir_manifest.find("\"text\": \"WAIT WINDOW 'ir\\u001fcontrol'\"") != std::string::npos,
               "ir manifest should canonically escape source control bytes");
        expect(ir_manifest.find('\x1f') == std::string::npos,
               "ir manifest should not contain a raw source control byte");
        expect(ir_manifest.find("\"name\": \"worker\"") != std::string::npos,
               "ir manifest should emit named routines");

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        expect(debug_manifest.find("ir_manifest_path=" + quote_manifest_value(result.plan.ir_manifest_path)) != std::string::npos,
               "debug manifest should record the IR-manifest path");
        expect(runtime_manifest.find("ir_manifest_path=") == std::string::npos,
               "runtime manifest should omit the IR-manifest path");
        expect(lines_with_prefix(runtime_manifest, "feature_flag=").empty(),
               "runtime manifest should omit feature-flag inventory while trimming IR build metadata");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_package_emits_csharp_transpilation_for_procedural_prg_code() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_csharp_contract";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg",
               "LOCAL nValue\n"
               "nValue = 1\n"
               "DO worker\n"
               "READ EVENTS\n"
               "RETURN\n"
               "PROCEDURE worker\n"
               "WAIT WINDOW 'csharp\x1f" "control'\n"
               "RETURN\n"
               "ENDPROC\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "csharpdemo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "CSharpDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "CSharpDemo";
    workspace.build_plan.output_path = (output_dir / "CSharpDemo.exe").string();
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

    expect(plan.ok, "csharp-output plan should be created");
    expect(fs::path(plan.transpiled_csharp_path).filename() == "CSharpDemo.exe.transpiled.cs",
           "csharp-output plan should derive a target-specific transpilation filename");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect_materialization(result, "csharp-output package should materialize");
    if (result.ok) {
        expect(fs::exists(result.plan.transpiled_csharp_path),
               "csharp-output package should emit a C# transpilation artifact");

        const std::string transpiled = read_text(result.plan.transpiled_csharp_path);
        expect(transpiled.find("public static class TranspiledProgram") != std::string::npos,
               "csharp transpilation should emit the generated container type");
        expect(transpiled.find("public static void MainRoutine()") != std::string::npos,
               "csharp transpilation should emit a main routine");
        expect(transpiled.find("dynamic nValue = null;") != std::string::npos,
               "csharp transpilation should map LOCAL declarations to dynamic locals");
        expect(transpiled.find("nValue = 1;") != std::string::npos,
               "csharp transpilation should preserve simple assignments");
        expect(transpiled.find("Worker();") != std::string::npos,
               "csharp transpilation should map DO worker to a routine call");
        expect(transpiled.find("public static void worker()") != std::string::npos ||
               transpiled.find("public static void Worker()") != std::string::npos,
               "csharp transpilation should emit the called FoxPro routine");
        expect(transpiled.find("Console.WriteLine(\"csharp\\u001fcontrol\");") != std::string::npos,
               "csharp transpilation should canonically escape source control bytes");
        expect(transpiled.find('\x1f') == std::string::npos,
               "csharp transpilation should not contain a raw source control byte");
        expect(
            transpiled.find("GeneratedLocalization.Translate(\"Runtime.Package.Transpilation.Error.UnsupportedFoxProStatement\"") != std::string::npos &&
                transpiled.find("[\"statementText\"] = \"READ EVENTS\"") != std::string::npos,
            "csharp transpilation should route unsupported-statement exceptions through localization while preserving statement text");
        expect(
            transpiled.find("[\"qps-ploc\"] = new(StringComparer.OrdinalIgnoreCase)") != std::string::npos,
            "csharp transpilation should embed a qps-ploc locale bucket for runtime exception localization");
        if (dotnet_is_available()) {
            std::string compile_error;
            const bool compiled = compile_csharp_artifact(result.plan.transpiled_csharp_path, compile_error);
            if (!compiled && !compile_error.empty()) {
                std::cerr << "FAIL: " << compile_error << "\n";
            }
            expect(compiled,
                   "csharp transpilation should compile under dotnet");
        }

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        expect(debug_manifest.find("transpiled_csharp_path=" + quote_manifest_value(result.plan.transpiled_csharp_path)) != std::string::npos,
               "debug manifest should record the transpiled C# artifact path");
        expect(runtime_manifest.find("transpiled_csharp_path=") == std::string::npos,
               "runtime manifest should omit the transpiled C# artifact path");
        expect(lines_with_prefix(runtime_manifest, "feature_flag=").empty(),
               "runtime manifest should omit feature-flag inventory while trimming C# transpilation metadata");
    }

    fs::remove_all(temp_root, ignored);
}



}  // namespace cf_test_runtime_pipeline
