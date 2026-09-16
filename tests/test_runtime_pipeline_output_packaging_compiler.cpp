// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

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
               "ON KEY LABEL F1 DO key_handler\n"
               "ON ESCAPE DO escape_handler\n"
               "ON PAGE AT LINE 42 DO page_handler\n"
               "EJECT PAGE\n"
               "EXPORT DATABASE 'catalog.dbc' TO 'catalog.json' TYPE JSON\n"
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
        expect(ir_manifest.find("\"opcode\": \"on_key_command\"") != std::string::npos,
               "ir manifest should map ON KEY statements to a stable opcode");
        expect(ir_manifest.find("\"opcode\": \"on_escape\"") != std::string::npos,
               "ir manifest should map ON ESCAPE statements to a stable opcode");
        expect(ir_manifest.find("\"opcode\": \"on_page\"") != std::string::npos,
               "ir manifest should map ON PAGE statements to a stable opcode");
        expect(ir_manifest.find("\"opcode\": \"eject_page_command\"") != std::string::npos,
               "ir manifest should map EJECT PAGE statements to a stable opcode");
        expect(ir_manifest.find("\"opcode\": \"export_database_command\"") != std::string::npos,
               "ir manifest should map EXPORT DATABASE statements to a stable opcode");
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

    const auto expect_keyword_at = [](const std::string& sql,
                                      const std::string_view keyword,
                                      const std::size_t expected,
                                      const std::string& scenario) {
        const std::size_t actual =
            copperfin::runtime::runtime_pipeline_detail::find_linq_top_level_keyword(
                sql,
                0U,
                keyword);
        expect(actual == expected,
               "RQ-CF-MODERNIZATION-012/#6173: LINQ keyword scanner should " + scenario);
    };
    for (const auto& [keyword, identifier_keyword] :
         std::vector<std::pair<std::string_view, std::string_view>>{
             {"from", "from"}, {"where", "where"}, {"as", "as"},
             {"into", "into"}, {"union", "union"}, {"having", "having"},
             {"group by", "group_by"}, {"order by", "order_by"}}) {
        for (const std::string& embedded : {
                 std::string(identifier_keyword) + "_code",
                 "code_" + std::string(identifier_keyword) + "_value",
                 "code_" + std::string(identifier_keyword)}) {
            const std::string sql = "select " + embedded + " from source";
            const std::size_t clause_from = sql.rfind(" from ") + 1U;
            expect_keyword_at(
                sql,
                keyword,
                keyword == "from" ? clause_from : std::string::npos,
                "keep start/middle/end keyword text inside underscore-containing identifiers");
        }
    }
    {
        const std::string sql = "select source.from, source->where from source";
        expect_keyword_at(sql, "from", sql.rfind(" from ") + 1U,
                          "keep clause text after dot and arrow qualifiers inside identifiers");
        expect_keyword_at(sql, "where", std::string::npos,
                          "keep clause text after an arrow qualifier inside an identifier");
    }
    {
        const std::string sql =
            "select 'it''s from', \"a\"\"where\", [a]]group by], field /* from */ from source where active = .t.";
        expect_keyword_at(sql, "from", sql.rfind(" from ") + 1U,
                          "ignore clause text inside quoted, bracketed, and comment content");
        expect_keyword_at(sql, "where", sql.rfind(" where ") + 1U,
                          "find the real WHERE after quoted and bracketed keyword text");
    }
    {
        const std::string sql = "select field && from inside comment\nfrom source";
        expect_keyword_at(sql, "from", sql.rfind("from source"),
                          "ignore clause text inside a VFP line comment");
    }
    {
        const std::string sql = "select caf\xE9" "from_code from/* where */source where active = .t.";
        expect_keyword_at(sql, "from", sql.find(" from/*") + 1U,
                          "keep clause text adjacent to a legacy identifier byte inside that identifier");
        expect_keyword_at(sql, "where", sql.rfind(" where ") + 1U,
                          "treat a block comment as token-separating trivia beside a real clause");
    }

    write_text(project_dir / "main.prg",
               "LOCAL nValue\n"
               "nValue = 1\n"
               "SELECT id, name AS customer_name, COUNT(*) AS total FROM customer WHERE active = .T. GROUP BY id, name\n"
               "SELECT customer_from_code,;\n"
               "  order_where_status AS status;\n"
               "  FROM customer WHERE active = .T.\n"
               "SELECT from_code, where_code, as_code, into_code, union_code, having_code, group_by_code, order_by_code, [from,where] AS quoted_name FROM source_from_code WHERE filter_into_code = .T. GROUP BY group_union_code ORDER BY order_having_code\n"
               "SELECT field FROM source WHERE name = 'unterminated\n"
               "SELECT field] FROM source\n"
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
        expect(transpiled.find("using System.Linq;") != std::string::npos,
               "#57: C# transpilation should expose the LINQ query catalog surface");
        expect(transpiled.find("public sealed class LinqQueryDescriptor") != std::string::npos &&
                   transpiled.find("public static IQueryable<LinqQueryDescriptor> AsQueryable()") != std::string::npos,
               "#57: C# transpilation should emit a queryable descriptor catalog without binding a database provider");
        expect(transpiled.find("LinqQueryCatalog.Record(new LinqQueryDescriptor(\"SELECT id, name AS customer_name, COUNT(*) AS total FROM customer WHERE active = .T. GROUP BY id, name\"") != std::string::npos,
               "#57: C# transpilation should preserve the complete FoxPro SELECT source text");
        expect(transpiled.find("new LinqProjectionDescriptor(\"id\", \"\")") != std::string::npos &&
                   transpiled.find("new LinqProjectionDescriptor(\"name\", \"customer_name\")") != std::string::npos &&
                   transpiled.find("new LinqProjectionDescriptor(\"COUNT(*)\", \"total\")") != std::string::npos,
               "#57: C# transpilation should preserve projection expressions and explicit aliases structurally");
        expect(transpiled.find("\"active = .T.\", \"id, name\", new[] {\"COUNT(*)\"}") != std::string::npos,
               "#57: C# transpilation should preserve filter, grouping, and aggregate structure without executing the query");
        expect(transpiled.find("LinqQueryCatalog.Record(new LinqQueryDescriptor(\"SELECT customer_from_code, order_where_status AS status FROM customer WHERE active = .T.\"") != std::string::npos,
               "RQ-CF-MODERNIZATION-012/#6173: C# transpilation should preserve the complete keyword-bearing identifier query");
        expect(transpiled.find("new LinqProjectionDescriptor(\"customer_from_code\", \"\")") != std::string::npos &&
                   transpiled.find("new LinqProjectionDescriptor(\"order_where_status\", \"status\")") != std::string::npos &&
                   transpiled.find("\"active = .T.\", \"\", Array.Empty<string>()") != std::string::npos,
               "RQ-CF-MODERNIZATION-012/#6173: LINQ descriptors should not split underscore-containing identifiers at embedded SQL keywords");
        for (const std::string_view identifier : {
                 "from_code", "where_code", "as_code", "into_code", "union_code",
                 "having_code", "group_by_code", "order_by_code"}) {
            expect(transpiled.find("new LinqProjectionDescriptor(\"" + std::string(identifier) + "\", \"\")") != std::string::npos,
                   "RQ-CF-MODERNIZATION-012/#6173: every recognized keyword should remain part of an underscore-containing projection identifier");
        }
        expect(transpiled.find("new LinqProjectionDescriptor(\"[from,where]\", \"quoted_name\")") != std::string::npos,
               "RQ-CF-MODERNIZATION-012/#6173: commas and keywords inside bracket-delimited projection text should not split the descriptor");
        expect(transpiled.find("\"filter_into_code = .T.\", \"group_union_code\", Array.Empty<string>()") != std::string::npos,
               "RQ-CF-MODERNIZATION-012/#6173: keyword-bearing identifiers should remain intact in filters and grouping before later clauses");
        expect(transpiled.find("LinqQueryCatalog.Record(new LinqQueryDescriptor(\"SELECT field FROM source WHERE name = 'unterminated\"") == std::string::npos &&
                   transpiled.find("[\"statementText\"] = \"SELECT field FROM source WHERE name = 'unterminated\"") != std::string::npos,
               "RQ-CF-MODERNIZATION-012/#6173: an unclosed quote should fail explicitly without publishing partial descriptor metadata");
        expect(transpiled.find("LinqQueryCatalog.Record(new LinqQueryDescriptor(\"SELECT field] FROM source\"") == std::string::npos &&
                   transpiled.find("[\"statementText\"] = \"SELECT field] FROM source\"") != std::string::npos,
               "RQ-CF-MODERNIZATION-012/#6173: an unmatched closing bracket should fail explicitly without publishing partial descriptor metadata");
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

void test_runtime_package_csharp_transpilation_rejects_return_to_targeted_forms() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_csharp_return_to_contract";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    // #6441 review fix: RETURN TO MASTER/ProcedureName are nonlocal,
    // multi-frame transfers with no direct C# equivalent -- the
    // transpiler must reject them explicitly rather than emitting a
    // plain `return;` that would silently drop the unwind.
    write_text(project_dir / "main.prg",
               "DO worker\n"
               "RETURN\n"
               "PROCEDURE worker\n"
               "RETURN TO MASTER\n"
               "ENDPROC\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "returntomasterdemo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "ReturnToMasterDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "ReturnToMasterDemo";
    workspace.build_plan.output_path = (output_dir / "ReturnToMasterDemo.exe").string();
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

    expect(plan.ok, "#6441: csharp-output plan should be created for a RETURN TO MASTER source");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect_materialization(result, "#6441: csharp-output package should materialize for a RETURN TO MASTER source");
    if (result.ok) {
        const std::string transpiled = read_text(result.plan.transpiled_csharp_path);
        expect(
            transpiled.find("GeneratedLocalization.Translate(\"Runtime.Package.Transpilation.Error.UnsupportedFoxProStatement\"") != std::string::npos &&
                transpiled.find("[\"statementText\"] = \"RETURN TO MASTER\"") != std::string::npos,
            "#6441: csharp transpilation should reject RETURN TO MASTER instead of silently lowering it to a plain return");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_runtime_pipeline
