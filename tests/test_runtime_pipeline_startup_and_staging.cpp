#include "test_runtime_pipeline_support.h"

namespace cf_test_runtime_pipeline {
void test_materialize_runtime_package() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_tests";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg", "DO FORM customer\n");
    write_text(project_dir / "customer.scx", "synthetic form");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "demo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "DemoApp";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "DemoApp";
    workspace.build_plan.output_path = (output_dir / "DemoApp.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"},
        {.record_index = 2U, .name = "customer.scx", .relative_path = "customer.scx", .type_title = "Form"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        true,
        true);

    expect(plan.ok, "runtime package plan should be created");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "runtime package should materialize");
    if (result.ok) {
        expect(fs::exists(result.plan.manifest_path), "runtime package should emit a manifest");
        expect(fs::exists(result.plan.debug_manifest_path), "runtime package should emit a debug manifest");
        expect(fs::exists(result.plan.runtime_host_destination_path), "runtime package should bundle the runtime host");
        expect(fs::exists(fs::path(result.plan.content_root) / "main.prg"), "runtime package should stage the startup source");
        expect(fs::exists(fs::path(result.plan.content_root) / "customer.scx"), "runtime package should stage project assets");
        expect(fs::exists(result.plan.launcher_project_path), "runtime package should emit a generated launcher project");
        expect(fs::exists(result.plan.launcher_source_path), "runtime package should emit a generated launcher source file");

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        expect(
            result.plan.startup_source_path == (fs::path(result.plan.content_root) / "main.prg").string(),
            "runtime plan should point startup to staged package content");
        expect(
            result.plan.debug_plan.startup_source_path == (project_dir / "main.prg").string(),
            "debug plan should point startup to source content");
        expect(result.plan.debug_plan.supports_breakpoints, "debug plan should enable breakpoints for PRG startup");
        expect(result.plan.debug_plan.supports_step_debugging, "debug plan should enable step debugging for PRG startup");
        expect(runtime_manifest.find("startup_source=") != std::string::npos, "runtime manifest should include a startup source field");
        expect(debug_manifest.find("startup_source=") != std::string::npos, "debug manifest should include a startup source field");
        expect(runtime_manifest.find("runtime_host_sha256=") != std::string::npos, "runtime manifest should include a runtime host SHA-256 digest");
        expect(runtime_manifest.find("security_role=") != std::string::npos, "runtime manifest should include the effective security role");
        expect(runtime_manifest.find("audit_log_path=") != std::string::npos, "runtime manifest should include the audit log path");
        expect(runtime_manifest.find("launcher_mode=dotnet_launcher") != std::string::npos, "runtime manifest should record the effective .NET launcher mode");
        expect(runtime_manifest.find("launcher_fallback=none") != std::string::npos, "runtime manifest should record the absence of launcher fallback");
        expect(runtime_manifest.find("dotnet_policy_allowlist=") != std::string::npos, "runtime manifest should include .NET policy allowlist metadata");
        expect(runtime_manifest.find("dotnet_policy_denylist=") != std::string::npos, "runtime manifest should include .NET policy denylist metadata");
        expect(runtime_manifest.find("dotnet_parity_matrix_entries=") != std::string::npos, "runtime manifest should include .NET parity matrix metadata");
        expect(runtime_manifest.find("dotnet_policy_allowlist_items=") != std::string::npos, "runtime manifest should include .NET policy allowlist item count");
        expect(runtime_manifest.find("dotnet_policy_denylist_items=") != std::string::npos, "runtime manifest should include .NET policy denylist item count");
        expect(runtime_manifest.find("dotnet_policy_allowlist_item=task-primitives") != std::string::npos,
               "runtime manifest should emit task-primitives allowlist entry");
        expect(runtime_manifest.find("dotnet_policy_denylist_item=unsafe-reflection-load") != std::string::npos,
               "runtime manifest should emit unsafe-reflection-load denylist entry");
        expect(runtime_manifest.find("dotnet_parity_matrix_count=") != std::string::npos, "runtime manifest should include .NET parity matrix count");
        expect(runtime_manifest.find("dotnet_parity_matrix_item=task-primitives") != std::string::npos,
               "runtime manifest should emit task-primitives parity matrix entry");
        expect(runtime_manifest.find("dotnet_parity_matrix_item=unsafe-reflection-load") != std::string::npos,
               "runtime manifest should emit unsafe-reflection-load parity matrix entry");
        expect(runtime_manifest.find("language_integration_count=") != std::string::npos, "runtime manifest should include language integration count");
        expect(runtime_manifest.find("language_integration=python|") != std::string::npos,
               "runtime manifest should emit python sidecar language integration");
        expect(runtime_manifest.find("language_integration=r|") != std::string::npos,
               "runtime manifest should emit R sidecar language integration");
        expect(runtime_manifest.find("ai_feature_count=") != std::string::npos, "runtime manifest should include AI feature count");
        expect(runtime_manifest.find("ai_feature=mcp-host|") != std::string::npos,
               "runtime manifest should emit MCP host AI feature metadata");
        expect(runtime_manifest.find("ai_feature=ai-assist|") != std::string::npos,
               "runtime manifest should emit AI-assisted developer workflow metadata");
        expect(runtime_manifest.find("extensibility_guardrail_count=") != std::string::npos,
               "runtime manifest should include extensibility guardrail count");
        expect(runtime_manifest.find("The trusted execution core stays native-first and security-first.") != std::string::npos,
               "runtime manifest should include explicit extensibility guardrails");
        expect(runtime_manifest.find("dotnet_gateway_task_primitives=") != std::string::npos, "runtime manifest should include .NET gateway allow decision diagnostics");
        expect(runtime_manifest.find("dotnet_gateway_unsafe_reflection=") != std::string::npos, "runtime manifest should include .NET gateway deny decision diagnostics");
        expect(runtime_manifest.find("feature_flag=launcher.dotnet.requested|true|rollout") != std::string::npos,
               "runtime manifest should expose the requested .NET launcher feature flag");
        expect(runtime_manifest.find("feature_flag=launcher.dotnet.active|true|host_compatibility") != std::string::npos,
               "runtime manifest should expose the active .NET launcher feature flag");
        expect(debug_manifest.find("launcher_mode=dotnet_launcher") != std::string::npos,
               "debug manifest should record the effective launcher mode");
        expect(debug_manifest.find("launcher_fallback=none") != std::string::npos,
               "debug manifest should record the launcher fallback state");
    }

    fs::remove_all(temp_root, ignored);
}

void test_materialize_excluded_xasset_startup_package() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_xasset_tests";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "startup.scx", "synthetic form table");
    write_text(project_dir / "startup.sct", "synthetic form memo");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "demo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "DemoXAsset";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "DemoXAsset";
    workspace.build_plan.output_path = (output_dir / "DemoXAsset.exe").string();
    workspace.build_plan.startup_item = "startup.scx";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "startup.scx", .relative_path = "startup.scx", .type_title = "Form", .excluded = true}
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

    expect(plan.ok, "xasset runtime package plan should be created");
    expect(plan.debug_plan.supports_breakpoints, "xasset startup should advertise breakpoint support");
    expect(plan.debug_plan.supports_step_debugging, "xasset startup should advertise step debugging");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "xasset runtime package should materialize");
    if (result.ok) {
        expect(fs::exists(fs::path(result.plan.content_root) / "startup.scx"), "packaged xasset startup should be staged even if excluded");
        expect(fs::exists(fs::path(result.plan.content_root) / "startup.sct"), "packaged xasset memo sidecar should be staged");
    }

    fs::remove_all(temp_root, ignored);
}

void test_vfp_style_parent_relative_assets_resolve_and_stage_under_content_root() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_vfp_parent_relative";
    const fs::path source_root = temp_root / "VFPSource";
    const fs::path project_dir = source_root / "addlabel";
    const fs::path shared_dir = source_root / "wzcommon";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    fs::create_directories(shared_dir);

    write_text(project_dir / "main.prg", "DO FORM registry\n");
    write_text(shared_dir / "registry.vcx", "synthetic shared class library");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "addlabel.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "AddLabel";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "AddLabel";
    workspace.build_plan.output_path = (output_dir / "AddLabel.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"},
        {.record_index = 2U, .name = R"(..\wzcommon\registry.vcx)", .relative_path = R"(..\wzcommon\registry.vcx)", .type_title = "Class Library"}
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

    expect(plan.ok, "VFP-style parent-relative plan should be created");

    const auto shared_asset = std::find_if(plan.assets.begin(), plan.assets.end(), [](const auto& asset) {
        return asset.record_index == 2U;
    });
    expect(shared_asset != plan.assets.end(), "VFP-style parent-relative asset should be present in the plan");
    if (shared_asset != plan.assets.end()) {
        expect(shared_asset->source_path == (shared_dir / "registry.vcx").lexically_normal().string(),
               "VFP-style parent-relative asset should resolve to its sibling source file");
        expect(shared_asset->relative_path == "wzcommon/registry.vcx",
               "VFP-style parent-relative asset should stage under a safe package-relative path");
        expect(shared_asset->staged_path == (fs::path(plan.content_root) / "wzcommon" / "registry.vcx").lexically_normal().string(),
               "VFP-style parent-relative asset should stage beneath the content root");
    }

    const bool has_missing_asset_warning = std::any_of(
        plan.warnings.begin(),
        plan.warnings.end(),
        [](const std::string& warning) {
            return warning.find(runtime_pipeline_english_catalog().translate(
                "Runtime.Package.Warning.MissingProjectAsset")) != std::string::npos;
        });
    expect(!has_missing_asset_warning,
           "resolved VFP-style parent-relative assets should not emit missing-asset warnings");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "VFP-style parent-relative package should materialize");
    if (result.ok) {
        const fs::path content_root(result.plan.content_root);
        expect(fs::exists(content_root / "wzcommon" / "registry.vcx"),
               "VFP-style parent-relative asset should be copied under content/");
        expect(!fs::exists(fs::path(result.plan.package_root) / "wzcommon" / "registry.vcx"),
               "VFP-style parent-relative asset should not escape the content root");

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        expect(runtime_manifest.find("asset=2|wzcommon/registry.vcx|") != std::string::npos,
               "runtime manifest should record the sanitized package-relative path");
    }

    fs::remove_all(temp_root, ignored);
}

void test_vfp_source_layout_parent_relative_assets_resolve_by_tail_match() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_vfp_source_tail_match";
    const fs::path source_root = temp_root / "VFPSource";
    const fs::path project_dir = source_root / "addlabel";
    const fs::path shared_dir = source_root / "Wizards" / "wzcommon";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    fs::create_directories(shared_dir);

    write_text(project_dir / "main.prg", "DO FORM registry\n");
    write_text(shared_dir / "REGISTRY.VCX", "synthetic shared class library");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "addlabel.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "AddLabel";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "AddLabel";
    workspace.build_plan.output_path = (output_dir / "AddLabel.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"},
        {.record_index = 2U, .name = R"(..\wzcommon\registry.vcx)", .relative_path = R"(..\wzcommon\registry.vcx)", .type_title = "Class Library"}
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

    expect(plan.ok, "VFPSource-style tail-match plan should be created");

    const auto shared_asset = std::find_if(plan.assets.begin(), plan.assets.end(), [](const auto& asset) {
        return asset.record_index == 2U;
    });
    expect(shared_asset != plan.assets.end(), "VFPSource-style tail-match asset should be present in the plan");
    if (shared_asset != plan.assets.end()) {
        expect(shared_asset->source_path == (shared_dir / "REGISTRY.VCX").lexically_normal().string(),
               "VFPSource-style tail-match asset should resolve to the case-insensitive shared source file");
        expect(shared_asset->relative_path == "wzcommon/registry.vcx",
               "VFPSource-style tail-match asset should preserve the sanitized package-relative path");
    }

    const bool has_missing_asset_warning = std::any_of(
        plan.warnings.begin(),
        plan.warnings.end(),
        [](const std::string& warning) {
            return warning.find(runtime_pipeline_english_catalog().translate(
                "Runtime.Package.Warning.MissingProjectAsset")) != std::string::npos;
        });
    expect(!has_missing_asset_warning,
           "VFPSource-style tail-match assets should not emit missing-asset warnings");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "VFPSource-style tail-match package should materialize");
    if (result.ok) {
        const fs::path content_root(result.plan.content_root);
        expect(fs::exists(content_root / "wzcommon" / "registry.vcx"),
               "VFPSource-style tail-match asset should be copied under content/");
    }

    fs::remove_all(temp_root, ignored);
}

void test_startup_dbf_companion_assets_are_staged() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_dbf_companions";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "startup.dbf", "synthetic dbf");
    write_text(project_dir / "startup.fpt", "synthetic memo");
    write_text(project_dir / "startup.cdx", "synthetic index");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "companion_demo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "DbfCompanionDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "DbfCompanionDemo";
    workspace.build_plan.output_path = (output_dir / "DbfCompanionDemo.exe").string();
    workspace.build_plan.startup_item = "startup.dbf";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "startup.dbf", .relative_path = "startup.dbf", .type_title = "Table", .excluded = true}
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

    expect(plan.ok, "dbf companion runtime package plan should be created");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "dbf companion runtime package should materialize");
    if (result.ok) {
        const std::filesystem::path content_root(result.plan.content_root);
        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        expect(fs::exists(content_root / "startup.dbf"), "startup DBF should be staged even when marked excluded");
        expect(fs::exists(content_root / "startup.fpt"), "startup DBF memo companion should be staged");
        expect(fs::exists(content_root / "startup.cdx"), "startup DBF index companion should be staged");
        expect(
            runtime_manifest.find("asset=1|startup.dbf|") != std::string::npos &&
            runtime_manifest.find("asset=1|startup.dbf|") < runtime_manifest.find("|true|true|") &&
            runtime_manifest.find("|true|true|", runtime_manifest.find("asset=1|startup.dbf|")) != std::string::npos,
            "runtime manifest should report the startup DBF asset as copied");
    }

    fs::remove_all(temp_root, ignored);
}

void test_materialize_fails_before_asset_staging_when_runtime_host_source_is_invalid() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_failfast_invalid_host";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path invalid_runtime_host = temp_root / "missing_runtime_host.exe";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg", "RETURN\n");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "failfast_host.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "FailFastHost";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "FailFastHost";
    workspace.build_plan.output_path = (output_dir / "FailFastHost.exe").string();
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

    expect(plan.ok, "fail-fast invalid-host plan should be created");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        invalid_runtime_host.string());

    expect(!result.ok, "invalid runtime host source should fail materialization");
    expect(!fs::exists(fs::path(plan.content_root) / "main.prg"),
           "invalid runtime host source should fail before staging startup assets");

    fs::remove_all(temp_root, ignored);
}

void test_startup_prg_extension_matching_is_case_insensitive() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_case_insensitive_startup";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "MAIN.PRG", "RETURN\n");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "case_demo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "CaseDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "CaseDemo";
    workspace.build_plan.output_path = (output_dir / "CaseDemo.exe").string();
    workspace.build_plan.startup_item = "MAIN.PRG";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "MAIN.PRG", .relative_path = "MAIN.PRG", .type_title = "Program"}
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

    expect(plan.ok, "runtime package plan should be created for uppercase PRG startup");
    expect(plan.debug_plan.supports_breakpoints,
           "uppercase .PRG startup should enable breakpoint support");
    expect(plan.debug_plan.supports_step_debugging,
           "uppercase .PRG startup should enable step-debug support");

    fs::remove_all(temp_root, ignored);
}

void test_startup_asset_is_staged_even_when_marked_excluded() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_startup_excluded_stage";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "MAIN.PRG", "RETURN\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "startup_excluded.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "StartupExcluded";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "StartupExcluded";
    workspace.build_plan.output_path = (output_dir / "StartupExcluded.exe").string();
    workspace.build_plan.startup_item = "MAIN.PRG";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "MAIN.PRG", .relative_path = "MAIN.PRG", .type_title = "Program", .excluded = true}
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

    expect(plan.ok, "runtime package plan should be created when startup asset is excluded");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "runtime package should materialize when startup asset is excluded");
    if (result.ok) {
        expect(fs::exists(fs::path(result.plan.content_root) / "MAIN.PRG"),
               "startup program should still be staged even when entry is marked excluded");
    }

    fs::remove_all(temp_root, ignored);
}

void test_missing_startup_record_surfaces_plan_warnings_and_disables_debug_startup_support() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_missing_startup_record";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg", "RETURN\n");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "missing_startup.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "MissingStartup";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "MissingStartup";
    workspace.build_plan.output_path = (output_dir / "MissingStartup.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 42U;
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

    expect(plan.ok, "runtime package plan should still be creatable when startup record is unresolved");
    expect(!plan.debug_plan.supports_breakpoints,
           "missing startup record should disable debug startup breakpoint support");
    expect(!plan.debug_plan.supports_step_debugging,
           "missing startup record should disable debug startup step-debug support");
    const bool has_runtime_startup_warning = std::any_of(
        plan.warnings.begin(),
        plan.warnings.end(),
        [](const std::string& warning) {
            return warning.find(runtime_pipeline_english_catalog().translate(
                "Runtime.Package.Warning.StartupSourceUnresolved")) != std::string::npos;
        });
    const bool has_debug_startup_warning = std::any_of(
        plan.warnings.begin(),
        plan.warnings.end(),
        [](const std::string& warning) {
            return warning.find(runtime_pipeline_english_catalog().translate(
                "Runtime.Package.Warning.DebugStartupSourceUnresolved")) != std::string::npos;
        });
    expect(has_runtime_startup_warning, "missing startup record should emit runtime startup resolution warning");
    expect(has_debug_startup_warning, "missing startup record should emit debug startup resolution warning");

    fs::remove_all(temp_root, ignored);
}

void test_debug_source_roots_are_unique_when_source_and_content_paths_match() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_debug_roots_unique";
    const fs::path output_dir = temp_root / "output";
    const std::string project_title = "SourceRootParity";
    const fs::path project_dir = output_dir / project_title / "content";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg", "RETURN\n");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "source_root_parity.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = project_title;
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = project_title;
    workspace.build_plan.output_path = (output_dir / "SourceRootParity.exe").string();
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

    expect(plan.ok, "debug source-root uniqueness plan should be created");
    expect(plan.debug_plan.source_roots.size() == 1U,
           "debug source roots should collapse to one unique path when source and content roots match");

    const std::string debug_manifest = copperfin::runtime::build_debug_manifest_text(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile());
    const std::string expected_roots_line = "source_roots=" + project_dir.lexically_normal().string();
    expect(debug_manifest.find(expected_roots_line) != std::string::npos,
           "debug manifest should emit a single normalized source_roots entry");

    fs::remove_all(temp_root, ignored);
}

void test_debug_source_roots_preserve_source_first_and_content_second_order() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_debug_roots_order";
    const fs::path source_root = temp_root / "source";
    const fs::path output_dir = temp_root / "output";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_root);

    write_text(source_root / "main.prg", "RETURN\n");

    copperfin::studio::StudioDocumentModel document;
    document.path = (source_root / "debug_roots_order.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "DebugRootsOrder";
    workspace.home_directory = source_root.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "DebugRootsOrder";
    workspace.build_plan.output_path = (output_dir / "DebugRootsOrder.exe").string();
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

    expect(plan.ok, "ordered debug source-root plan should be created");
    expect(plan.debug_plan.source_roots.size() == 2U,
           "ordered debug source-root plan should preserve both source and content roots");
    if (plan.debug_plan.source_roots.size() == 2U) {
        expect(plan.debug_plan.source_roots.front() == source_root.lexically_normal().string(),
               "debug source roots should keep the source-side working directory first");
        expect(plan.debug_plan.source_roots.back() == (output_dir / "DebugRootsOrder" / "content").lexically_normal().string(),
               "debug source roots should keep the packaged content root second");
    }

    const std::string debug_manifest = copperfin::runtime::build_debug_manifest_text(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile());
    const std::string expected_roots_line =
        "source_roots=" + source_root.lexically_normal().string() + ";" +
        (output_dir / "DebugRootsOrder" / "content").lexically_normal().string();
    expect(debug_manifest.find(expected_roots_line) != std::string::npos,
           "debug manifest should preserve source-first source_roots ordering");

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_runtime_pipeline
