// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_runtime_pipeline_output_packaging_support.h"

namespace cf_test_runtime_pipeline {

namespace {

class compiler_manifest_grouped_numpunct final : public std::numpunct<char> {
protected:
    char do_decimal_point() const override { return ','; }
    char do_thousands_sep() const override { return '.'; }
    std::string do_grouping() const override { return "\1"; }
};

class compiler_manifest_global_locale_guard final {
public:
    explicit compiler_manifest_global_locale_guard(const std::locale& replacement)
        : previous_(std::locale::global(replacement)) {}

    ~compiler_manifest_global_locale_guard() { std::locale::global(previous_); }

    compiler_manifest_global_locale_guard(const compiler_manifest_global_locale_guard&) = delete;
    compiler_manifest_global_locale_guard& operator=(const compiler_manifest_global_locale_guard&) = delete;

private:
    std::locale previous_;
};

}  // namespace

void test_fxp_output_package_emits_token_manifest_from_prg_statements() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_fxp_contract";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg",
               std::string(1233U, '\n') +
               "LOCAL nValue\n"
               "nValue = 1\n"
               "DO worker\n"
               "RETURN\n"
               "PROCEDURE worker\n"
               "WAIT WINDOW 'hello'\n"
               "RETURN\n"
               "ENDPROC\n");
    write_text(project_dir / "excluded_helper.prg",
               "WAIT WINDOW 'excluded-leak-marker'\n"
               "RETURN\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "compiledemo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "CompileDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "CompileDemo";
    workspace.build_plan.output_path = (output_dir / "CompileDemo.fxp").string();
    workspace.build_plan.output_kind = "fxp";
    workspace.build_plan.build_target = "x64 Visual FoxPro tokenized program";
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1234U;
    workspace.entries = {
        {.record_index = 1234U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program", .excluded = true},
        {.record_index = 1235U, .name = "excluded_helper.prg", .relative_path = "excluded_helper.prg", .type_title = "Program", .excluded = true}
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

    expect(plan.ok, "fxp-output plan should be created");
    expect(plan.output_kind == copperfin::runtime::BuildOutputKind::fxp,
           "fxp-output plan should preserve FXP output kind");
    expect(!plan.emit_dotnet_launcher,
           "fxp-output plan should not route through .NET launcher emission");
    expect(plan.launcher_mode == "foxpro_tokenized_contract",
           "fxp-output plan should switch to the tokenized-contract packaging mode");
    expect(plan.launcher_fallback == "foxpro_fxp_binary_generation_pending",
           "fxp-output plan should record the honest non-binary fallback state");
    expect(fs::path(plan.launcher_output_path).filename() == "CompileDemo.fxp",
           "fxp-output plan should preserve the requested output filename");
    expect(fs::path(plan.fxp_token_manifest_path).filename() == "CompileDemo.fxp.tokens",
           "fxp-output plan should derive a matching token-manifest filename");

    const std::locale grouping_locale(std::locale::classic(), new compiler_manifest_grouped_numpunct());
    const compiler_manifest_global_locale_guard locale_guard(grouping_locale);
    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect_materialization(result, "fxp-output package should materialize");
    if (result.ok) {
        expect(fs::exists(result.plan.fxp_token_manifest_path),
               "fxp-output package should emit a token manifest");
        expect(fs::exists(result.plan.launcher_output_path),
               "fxp-output package should materialize an honest FXP contract file");
        expect(!fs::exists(result.plan.runtime_host_destination_path),
               "fxp-output package should not bundle an executable runtime host into the FXP output slot");
        expect(result.plan.primary_output_materialized,
               "fxp-output package should report that the FXP contract file is materialized");
        const auto main_asset = std::find_if(result.plan.assets.begin(), result.plan.assets.end(), [](const auto& asset) {
            return asset.relative_path == "main.prg";
        });
        const auto excluded_asset = std::find_if(result.plan.assets.begin(), result.plan.assets.end(), [](const auto& asset) {
            return asset.relative_path == "excluded_helper.prg";
        });
        expect(main_asset != result.plan.assets.end() && main_asset->excluded &&
                   main_asset->required_for_runtime && main_asset->copied,
               "#3881: excluded startup PRG should remain admitted and staged");
        expect(excluded_asset != result.plan.assets.end() && excluded_asset->excluded &&
                   !excluded_asset->required_for_runtime && !excluded_asset->copied,
               "#3881: excluded non-runtime PRG should remain outside staged content");
        expect(fs::exists(fs::path(result.plan.content_root) / "main.prg") &&
                   !fs::exists(fs::path(result.plan.content_root) / "excluded_helper.prg"),
               "#3881: staged content should follow the shared package admission rule");

        const std::string token_manifest = read_text(result.plan.fxp_token_manifest_path);
        expect(token_manifest.find("output_kind=fxp") != std::string::npos,
               "fxp-output token manifest should declare the FXP output kind");
        expect(token_manifest.find("token_contract=logical_statements") != std::string::npos,
               "fxp-output token manifest should declare the token-contract mode");
        expect(token_manifest.find("primary_output=CompileDemo.fxp") != std::string::npos,
               "fxp-output token manifest should name the requested FXP file");
        expect(token_manifest.find("program=main.prg") != std::string::npos,
               "fxp-output token manifest should list the source program");
        expect(token_manifest.find("statement=MAIN|") != std::string::npos,
               "fxp-output token manifest should include main-scope statements");
        expect(token_manifest.find("statement=MAIN|1234|LOCAL nValue") != std::string::npos,
               "fxp-output token manifest should keep source-line coordinates locale invariant");
        expect(token_manifest.find("DO worker") != std::string::npos,
               "fxp-output token manifest should preserve logical statement text");
        expect(token_manifest.find("statement=worker|") != std::string::npos,
               "fxp-output token manifest should include routine-scope statements");
        expect(token_manifest.find("WAIT WINDOW 'hello'") != std::string::npos,
               "fxp-output token manifest should preserve routine statement text");
        expect(token_manifest.find("excluded_helper.prg") == std::string::npos &&
                   token_manifest.find("excluded-leak-marker") == std::string::npos,
               "#3881: FXP token manifest should omit excluded non-runtime PRG source");

        const std::string ast_manifest = read_text(result.plan.ast_manifest_path);
        const std::string ir_manifest = read_text(result.plan.ir_manifest_path);
        const std::string transpiled_csharp = read_text(result.plan.transpiled_csharp_path);
        expect(ast_manifest.find("\"relative_path\": \"main.prg\"") != std::string::npos &&
                   ast_manifest.find("WAIT WINDOW 'hello'") != std::string::npos,
               "#3881: AST manifest should retain the required excluded startup PRG");
        expect(ast_manifest.find("\"line\": 1234, \"text\": \"LOCAL nValue\"") != std::string::npos,
               "AST manifest should keep source-line coordinates locale invariant");
        expect(ir_manifest.find("\"relative_path\": \"main.prg\"") != std::string::npos &&
                   ir_manifest.find("WAIT WINDOW 'hello'") != std::string::npos,
               "#3881: IR manifest should retain the required excluded startup PRG");
        expect(ir_manifest.find("\"line\": 1234, \"opcode\": \"local_declaration\"") != std::string::npos,
               "IR manifest should keep source-line coordinates locale invariant");
        expect(transpiled_csharp.find("Console.WriteLine(\"hello\");") != std::string::npos,
               "#3881: generated C# should retain the required excluded startup PRG");
        expect(ast_manifest.find("excluded_helper.prg") == std::string::npos &&
                   ast_manifest.find("excluded-leak-marker") == std::string::npos,
               "#3881: AST manifest should omit excluded non-runtime PRG source");
        expect(ir_manifest.find("excluded_helper.prg") == std::string::npos &&
                   ir_manifest.find("excluded-leak-marker") == std::string::npos,
               "#3881: IR manifest should omit excluded non-runtime PRG source");
        expect(transpiled_csharp.find("excluded-leak-marker") == std::string::npos,
               "#3881: generated C# should omit excluded non-runtime PRG source");

        const std::string fxp_contract = read_text(result.plan.launcher_output_path);
        expect(fxp_contract.find("copperfin_fxp_contract_version=1") != std::string::npos,
               "fxp-output primary output should identify the Copperfin FXP contract format");
        expect(fxp_contract.find("token_contract=copperfin_logical_statement_contract_v1") != std::string::npos,
               "fxp-output primary output should declare the Copperfin FXP contract");
        expect(fxp_contract.find("token_manifest=" + quote_manifest_value(result.plan.fxp_token_manifest_path)) != std::string::npos,
               "fxp-output primary output should point back to the token manifest");
        expect(fxp_contract.find("output_kind=fxp") != std::string::npos,
               "fxp-output primary output should embed the FXP token-manifest content");
        expect(fxp_contract.find("statement=MAIN|") != std::string::npos,
               "fxp-output primary output should preserve main-scope logical statements");
        expect(fxp_contract.find("statement=MAIN|1234|LOCAL nValue") != std::string::npos,
               "fxp-output primary output should preserve locale-invariant source-line coordinates");
        expect(fxp_contract.find("statement=worker|") != std::string::npos,
               "fxp-output primary output should preserve routine-scope logical statements");
        expect(fxp_contract.find("excluded_helper.prg") == std::string::npos &&
                   fxp_contract.find("excluded-leak-marker") == std::string::npos,
               "#3881: embedded FXP contract should omit excluded non-runtime PRG source");

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        expect(runtime_manifest.find("asset=1234|main.prg|") != std::string::npos,
               "runtime manifest should preserve invariant asset record index 1234 under grouped punctuation");
        expect(debug_manifest.find("asset=1234|main.prg|") != std::string::npos,
               "debug manifest should preserve the same invariant asset record index under grouped punctuation");
        expect(runtime_manifest.find("asset=1.234|main.prg|") == std::string::npos &&
                   debug_manifest.find("asset=1.234|main.prg|") == std::string::npos,
               "runtime and debug manifests should reject grouped asset record indices");
        expect(runtime_manifest.find("output_kind=fxp") != std::string::npos,
               "fxp-output manifest should record FXP output kind");
        expect(runtime_manifest.find("primary_output_path=") == std::string::npos,
               "fxp-output runtime manifest should omit the primary output path from the execution contract");
        expect(runtime_manifest.find("primary_output_materialized=") == std::string::npos,
               "fxp-output runtime manifest should omit the materialized primary output state from the execution contract");
        expect(runtime_manifest.find("extension_payload=" + quote_manifest_value(result.plan.launcher_output_path) + "|") != std::string::npos,
               "fxp-output manifest should record the emitted FXP contract as an extension payload");
        expect(lines_with_prefix(runtime_manifest, "feature_flag=").empty(),
               "fxp-output runtime manifest should omit feature-flag inventory from the execution contract");
        expect(debug_manifest.find("feature_flag=build.output.fxp_token_contract|true|build_output") != std::string::npos,
               "fxp-output debug manifest should preserve the FXP token-contract feature flag");
        expect(debug_manifest.find("output_kind=fxp") != std::string::npos,
               "fxp-output debug manifest should record FXP output kind");
        expect(debug_manifest.find("fxp_token_manifest_path=" + quote_manifest_value(result.plan.fxp_token_manifest_path)) != std::string::npos,
               "fxp-output debug manifest should record the emitted token-manifest path");
        expect(debug_manifest.find("launcher_mode=foxpro_tokenized_contract") != std::string::npos,
               "fxp-output debug manifest should record the tokenized-contract mode");
    }

    fs::remove_all(temp_root, ignored);
}

void test_library_output_warning_lines_are_mirrored_into_debug_manifest() {
    run_library_output_warning_debug_manifest_smoke("dll", "dll");
    run_library_output_warning_debug_manifest_smoke("fll", "fll");
}

void test_app_output_package_emits_archive_manifest_for_staged_assets() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_app_contract";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    const std::string large_text(1234U, 'L');
    const std::string binary_payload("\x00\x0f\x10\xff", 4U);
    write_text(project_dir / "main.prg", "DO helper\nRETURN\n");
    write_text(project_dir / "helper.prg", "WAIT WINDOW 'archived'\nRETURN\n");
    write_text(project_dir / "config.txt", "mode=demo");
    write_text(project_dir / "large.txt", large_text);
    write_text(project_dir / "binary.bin", binary_payload);
    write_text(project_dir / "sample.scx", "screen-bytes");
    write_text(project_dir / "sample.sct", "screen-sidecar-bytes");
    write_text(project_dir / "sample.frx", "report-bytes");
    write_text(project_dir / "sample.frt", "report-sidecar-bytes");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "archivedemo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "ArchiveDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "ArchiveDemo";
    workspace.build_plan.output_path = (output_dir / "ArchiveDemo.app").string();
    workspace.build_plan.output_kind = "app";
    workspace.build_plan.build_target = "x64 Visual FoxPro application archive";
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"},
        {.record_index = 2U, .name = "helper.prg", .relative_path = "helper.prg", .type_title = "Program"},
        {.record_index = 3U, .name = "config.txt", .relative_path = "config.txt", .type_title = "Text"},
        {.record_index = 4U, .name = "large.txt", .relative_path = "large.txt", .type_title = "Text"},
        {.record_index = 5U, .name = "binary.bin", .relative_path = "binary.bin", .type_title = "Binary"},
        {.record_index = 6U, .name = "sample.scx", .relative_path = "sample.scx", .type_title = "Form"},
        {.record_index = 7U, .name = "sample.frx", .relative_path = "sample.frx", .type_title = "Report"}
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

    expect(plan.ok, "app-output plan should be created");
    expect(plan.output_kind == copperfin::runtime::BuildOutputKind::app,
           "app-output plan should preserve APP output kind");
    expect(!plan.emit_dotnet_launcher,
           "app-output plan should not route through .NET launcher emission");
    expect(plan.launcher_mode == "foxpro_application_archive_contract",
           "app-output plan should switch to the archive-contract packaging mode");
    expect(plan.launcher_fallback == "foxpro_app_binary_generation_pending",
           "app-output plan should record the honest non-binary fallback state");
    expect(fs::path(plan.launcher_output_path).filename() == "ArchiveDemo.app",
           "app-output plan should preserve the requested output filename");
    expect(fs::path(plan.app_archive_manifest_path).filename() == "ArchiveDemo.app.contents",
           "app-output plan should derive a matching archive-manifest filename");

    const std::locale grouping_locale(std::locale::classic(), new compiler_manifest_grouped_numpunct());
    const compiler_manifest_global_locale_guard locale_guard(grouping_locale);
    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect_materialization(result, "app-output package should materialize");
    if (result.ok) {
        expect(fs::exists(result.plan.app_archive_manifest_path),
               "app-output package should emit an archive manifest");
        expect(fs::exists(result.plan.launcher_output_path),
               "app-output package should materialize an honest APP archive contract");
        expect(!fs::exists(result.plan.runtime_host_destination_path),
               "app-output package should not bundle an executable runtime host into the APP output slot");
        expect(result.plan.primary_output_materialized,
               "app-output package should report that the APP archive contract is materialized");
        expect(fs::exists(fs::path(result.plan.content_root) / "main.prg"),
               "app-output package should still stage the startup program");
        expect(fs::exists(fs::path(result.plan.content_root) / "helper.prg"),
               "app-output package should still stage supporting program assets");
        expect(fs::exists(fs::path(result.plan.content_root) / "config.txt"),
               "app-output package should still stage non-program assets");
        expect(fs::exists(fs::path(result.plan.content_root) / "large.txt"),
               "app-output package should still stage a large declared asset");
        expect(fs::exists(fs::path(result.plan.content_root) / "binary.bin"),
               "app-output package should still stage a binary declared asset");
        expect(fs::exists(fs::path(result.plan.content_root) / "sample.scx"),
               "app-output package should still stage declared xAsset files");
        expect(fs::exists(fs::path(result.plan.content_root) / "sample.sct"),
               "app-output package should stage inferred form sidecars");
        expect(fs::exists(fs::path(result.plan.content_root) / "sample.frx"),
               "app-output package should still stage declared report assets");
        expect(fs::exists(fs::path(result.plan.content_root) / "sample.frt"),
               "app-output package should stage inferred report sidecars");

        const std::string archive_manifest = read_text(result.plan.app_archive_manifest_path);
        expect(archive_manifest.find("output_kind=app") != std::string::npos,
               "app-output archive manifest should declare the APP output kind");
        expect(archive_manifest.find("archive_contract=staged_content_manifest") != std::string::npos,
               "app-output archive manifest should declare the archive-contract mode");
        expect(archive_manifest.find("primary_output=ArchiveDemo.app") != std::string::npos,
               "app-output archive manifest should name the requested APP file");
        expect(archive_manifest.find("startup_item=main.prg") != std::string::npos,
               "app-output archive manifest should record the startup item");
        expect(archive_manifest.find("asset=main.prg|Program|true|true") != std::string::npos,
               "app-output archive manifest should record the staged startup program asset");
        expect(archive_manifest.find("asset=helper.prg|Program|false|true") != std::string::npos,
               "app-output archive manifest should record staged supporting program assets");
        expect(archive_manifest.find("asset=config.txt|Text|false|true") != std::string::npos,
               "app-output archive manifest should record staged non-program assets");
        expect(archive_manifest.find("asset=binary.bin|Binary|false|true") != std::string::npos,
               "app-output archive manifest should record staged binary assets");
        expect(archive_manifest.find("asset=sample.scx|Form|false|true") != std::string::npos,
               "app-output archive manifest should record declared xAsset files");
        expect(archive_manifest.find("asset=sample.frx|Report|false|true") != std::string::npos,
               "app-output archive manifest should record declared report assets");
        expect(archive_manifest.find("content_file=sample.scx|declared_asset") != std::string::npos,
               "app-output archive manifest should list staged declared xAsset files");
        expect(archive_manifest.find("content_file=sample.sct|companion") != std::string::npos,
               "app-output archive manifest should list inferred form sidecars");
        expect(archive_manifest.find("content_file=sample.frx|declared_asset") != std::string::npos,
               "app-output archive manifest should list staged declared report assets");
        expect(archive_manifest.find("content_file=sample.frt|companion") != std::string::npos,
               "app-output archive manifest should list inferred report sidecars");

        const std::string app_archive = read_text(result.plan.launcher_output_path);
        expect(app_archive.find("copperfin_app_archive_version=1") != std::string::npos,
               "app-output primary output should identify the Copperfin APP archive format");
        expect(app_archive.find("archive_contract=copperfin_content_archive_v1") != std::string::npos,
               "app-output primary output should declare the APP archive contract");
        expect(app_archive.find("content_manifest=" + quote_manifest_value(result.plan.app_archive_manifest_path)) != std::string::npos,
               "app-output primary output should point back to the staged-content manifest");
        const auto archive_payloads = parse_app_archive_payloads(app_archive);
        expect(archive_payloads.contains("main.prg"),
               "app-output primary archive should carry the startup program payload");
        expect(archive_payloads.contains("helper.prg"),
               "app-output primary archive should carry supporting program payloads");
        expect(archive_payloads.contains("config.txt"),
               "app-output primary archive should carry non-program payloads");
        expect(archive_payloads.contains("large.txt"),
               "app-output primary archive should carry the large declared payload");
        expect(archive_payloads.contains("binary.bin"),
               "app-output primary archive should carry binary declared payloads");
        expect(archive_payloads.contains("sample.scx"),
               "app-output primary archive should carry declared xAsset payloads");
        expect(archive_payloads.contains("sample.sct"),
               "app-output primary archive should carry inferred form sidecar payloads");
        expect(archive_payloads.contains("sample.frx"),
               "app-output primary archive should carry declared report payloads");
        expect(archive_payloads.contains("sample.frt"),
               "app-output primary archive should carry inferred report sidecar payloads");
        if (archive_payloads.contains("main.prg")) {
            expect(archive_payloads.at("main.prg") == "DO helper\nRETURN\n",
                   "app-output primary archive should preserve startup program bytes");
        }
        if (archive_payloads.contains("helper.prg")) {
            expect(archive_payloads.at("helper.prg") == "WAIT WINDOW 'archived'\nRETURN\n",
                   "app-output primary archive should preserve supporting program bytes");
        }
        if (archive_payloads.contains("config.txt")) {
            expect(archive_payloads.at("config.txt") == "mode=demo",
                   "app-output primary archive should preserve non-program asset bytes");
        }
        if (archive_payloads.contains("large.txt")) {
            expect(archive_payloads.at("large.txt") == large_text,
                   "app-output primary archive should preserve the large asset bytes");
        }
        if (archive_payloads.contains("binary.bin")) {
            expect(archive_payloads.at("binary.bin") == binary_payload,
                   "app-output primary archive should preserve every binary asset byte");
        }
        expect(app_archive.find("payload=binary.bin|000f10ff\n") != std::string::npos,
               "app-output archive should emit canonical two-digit lowercase hex under every-digit grouping");
        expect(app_archive.find("payload=binary.bin|0.0.0.f.1.0.f.f\n") == std::string::npos,
               "app-output archive should reject grouped punctuation in binary payload hex");
        const auto large_digest = copperfin::security::sha256_hex_for_text(large_text);
        expect(large_digest.ok,
               "app-output large asset fixture should have a deterministic digest");
        if (large_digest.ok) {
            const std::string expected_content_line =
                "content=large.txt|DeclaredAsset|true|1234|" + large_digest.hex_digest + "\n";
            expect(app_archive.find(expected_content_line) != std::string::npos,
                   "app-output archive should preserve the exact invariant byte count and digest for the large asset");
            expect(app_archive.find("content=large.txt|DeclaredAsset|true|1.234|") == std::string::npos,
                   "app-output archive should reject grouped punctuation in the large asset byte count");
        }
        if (archive_payloads.contains("sample.scx")) {
            expect(archive_payloads.at("sample.scx") == "screen-bytes",
                   "app-output primary archive should preserve declared xAsset bytes");
        }
        if (archive_payloads.contains("sample.sct")) {
            expect(archive_payloads.at("sample.sct") == "screen-sidecar-bytes",
                   "app-output primary archive should preserve inferred form sidecar bytes");
        }
        if (archive_payloads.contains("sample.frx")) {
            expect(archive_payloads.at("sample.frx") == "report-bytes",
                   "app-output primary archive should preserve declared report bytes");
        }
        if (archive_payloads.contains("sample.frt")) {
            expect(archive_payloads.at("sample.frt") == "report-sidecar-bytes",
                   "app-output primary archive should preserve inferred report sidecar bytes");
        }

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        expect(runtime_manifest.find("output_kind=app") != std::string::npos,
               "app-output manifest should record APP output kind");
        expect(runtime_manifest.find("primary_output_path=") == std::string::npos,
               "app-output runtime manifest should omit the primary output path from the execution contract");
        expect(runtime_manifest.find("primary_output_materialized=") == std::string::npos,
               "app-output runtime manifest should omit the materialized primary archive state from the execution contract");
        expect(runtime_manifest.find("extension_payload=" + quote_manifest_value(result.plan.launcher_output_path) + "|") != std::string::npos,
               "app-output manifest should record the emitted APP archive as an extension payload");
        expect(lines_with_prefix(runtime_manifest, "feature_flag=").empty(),
               "app-output runtime manifest should omit feature-flag inventory from the execution contract");
        expect(debug_manifest.find("feature_flag=build.output.app_archive_contract|true|build_output") != std::string::npos,
               "app-output debug manifest should preserve the APP archive-contract feature flag");
        expect(debug_manifest.find("output_kind=app") != std::string::npos,
               "app-output debug manifest should record APP output kind");
        expect(debug_manifest.find("app_archive_manifest_path=" + quote_manifest_value(result.plan.app_archive_manifest_path)) != std::string::npos,
               "app-output debug manifest should record the emitted archive-manifest path");
        expect(debug_manifest.find("launcher_mode=foxpro_application_archive_contract") != std::string::npos,
               "app-output debug manifest should record the archive-contract mode");
    }

    fs::remove_all(temp_root, ignored);
}



}  // namespace cf_test_runtime_pipeline
