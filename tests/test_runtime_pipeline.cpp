#include "copperfin/platform/extensibility_model.h"
#include "copperfin/runtime/runtime_pipeline.h"
#include "copperfin/security/security_model.h"
#include "copperfin/studio/project_workspace.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void write_text(const std::filesystem::path& path, const std::string& contents) {
    std::ofstream output(path, std::ios::binary);
    output << contents;
}

std::filesystem::path runtime_host_fixture_path(const std::filesystem::path& root) {
#if defined(_WIN32)
    return root / "copperfin_runtime_host.exe";
#else
    return root / "copperfin_runtime_host";
#endif
}

std::string read_text(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    std::ostringstream stream;
    stream << input.rdbuf();
    return stream.str();
}

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
        expect(runtime_manifest.find("dotnet_policy_allowlist=") != std::string::npos, "runtime manifest should include .NET policy allowlist metadata");
        expect(runtime_manifest.find("dotnet_policy_denylist=") != std::string::npos, "runtime manifest should include .NET policy denylist metadata");
        expect(runtime_manifest.find("dotnet_parity_matrix_entries=") != std::string::npos, "runtime manifest should include .NET parity matrix metadata");
        expect(runtime_manifest.find("dotnet_gateway_task_primitives=") != std::string::npos, "runtime manifest should include .NET gateway allow decision diagnostics");
        expect(runtime_manifest.find("dotnet_gateway_unsafe_reflection=") != std::string::npos, "runtime manifest should include .NET gateway deny decision diagnostics");
    }

    fs::remove_all(temp_root, ignored);
}

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
            launcher_source.find("WorkingDirectory = baseDir") != std::string::npos,
            "generated launcher should run the runtime host from the package directory");
        expect(
            launcher_project.find("<AssemblyName>LauncherContract</AssemblyName>") != std::string::npos,
            "generated launcher project should preserve the sanitized assembly name contract");
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
            return warning.find("No startup source asset could be resolved.") != std::string::npos;
        });
    const bool has_debug_startup_warning = std::any_of(
        plan.warnings.begin(),
        plan.warnings.end(),
        [](const std::string& warning) {
            return warning.find("No source-side startup asset could be resolved for debugging.") != std::string::npos;
        });
    expect(has_runtime_startup_warning, "missing startup record should emit runtime startup resolution warning");
    expect(has_debug_startup_warning, "missing startup record should emit debug startup resolution warning");

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

    expect(result.ok, "manifest-asset-copy-state package should materialize");
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

    const std::string debug_manifest = copperfin::runtime::build_debug_manifest_text(plan);
    const std::string expected_roots_line = "source_roots=" + project_dir.lexically_normal().string();
    expect(debug_manifest.find(expected_roots_line) != std::string::npos,
           "debug manifest should emit a single normalized source_roots entry");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_materialize_runtime_package();
    test_generated_launcher_forwards_manifest_and_debug_flag();
    test_materialize_excluded_xasset_startup_package();
    test_security_enabled_runtime_host_name_validation();
    test_materialize_fails_before_asset_staging_when_runtime_host_source_is_invalid();
    test_startup_prg_extension_matching_is_case_insensitive();
    test_startup_asset_is_staged_even_when_marked_excluded();
    test_missing_startup_record_surfaces_plan_warnings_and_disables_debug_startup_support();
    test_manifest_asset_lines_include_copy_state_contract();
    test_debug_source_roots_are_unique_when_source_and_content_paths_match();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
