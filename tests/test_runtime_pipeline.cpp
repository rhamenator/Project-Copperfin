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

std::string quote_manifest_value(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (const char ch : value) {
        if (ch == '\\') {
            escaped += "\\\\";
        } else if (ch == '\n') {
            escaped += "\\n";
        } else if (ch == '\r') {
            escaped += "\\r";
        } else {
            escaped.push_back(ch);
        }
    }
    return escaped;
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
        expect(runtime_manifest.find("launcher_mode=dotnet_launcher") != std::string::npos, "runtime manifest should record the effective .NET launcher mode");
        expect(runtime_manifest.find("launcher_fallback=none") != std::string::npos, "runtime manifest should record the absence of launcher fallback");
        expect(runtime_manifest.find("dotnet_policy_allowlist=") != std::string::npos, "runtime manifest should include .NET policy allowlist metadata");
        expect(runtime_manifest.find("dotnet_policy_denylist=") != std::string::npos, "runtime manifest should include .NET policy denylist metadata");
        expect(runtime_manifest.find("dotnet_parity_matrix_entries=") != std::string::npos, "runtime manifest should include .NET parity matrix metadata");
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
            launcher_source.find("forwarded.Add(Quote(arg));") != std::string::npos,
            "generated launcher should preserve ordinary application arguments instead of dropping them");
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

void test_library_output_package_emits_module_definition_from_prg_routines() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_library_contract";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "librarymain.prg",
               "PROCEDURE InitLibrary\nRETURN\nENDPROC\n");
    write_text(project_dir / "helper.prg",
               "FUNCTION AddNumbers\nRETURN 1\nENDFUNC\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "librarydemo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "LibraryDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "LibraryDemo";
    workspace.build_plan.output_path = (output_dir / "LibraryDemo.dll").string();
    workspace.build_plan.output_kind = "dll";
    workspace.build_plan.build_target = "x64 Windows dynamic-link library";
    workspace.build_plan.startup_item = "librarymain.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "librarymain.prg", .relative_path = "librarymain.prg", .type_title = "Program"},
        {.record_index = 2U, .name = "helper.prg", .relative_path = "helper.prg", .type_title = "Program"}
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

    expect(plan.ok, "library-output plan should be created");
    expect(plan.output_kind == copperfin::runtime::BuildOutputKind::dll,
           "library-output plan should preserve DLL output kind");
    expect(!plan.emit_dotnet_launcher,
           "library-output plan should not route through .NET launcher emission");
    expect(plan.launcher_mode == "foxpro_library_definition",
           "library-output plan should switch to the library-definition packaging mode");
    expect(plan.launcher_fallback == "library_binary_generation_pending",
           "library-output plan should record the honest non-binary fallback state");
    expect(fs::path(plan.launcher_output_path).filename() == "LibraryDemo.dll",
           "library-output plan should preserve the requested output filename");
    expect(fs::path(plan.module_definition_path).filename() == "LibraryDemo.def",
           "library-output plan should derive a matching module-definition filename");
    expect(plan.exported_symbols.size() == 2U,
           "library-output plan should discover routine exports from PRG assets");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "library-output package should materialize");
    if (result.ok) {
        expect(fs::exists(result.plan.module_definition_path),
               "library-output package should emit a module-definition file");
        expect(!fs::exists(result.plan.launcher_output_path),
               "library-output package should not fake a DLL binary");
        expect(!fs::exists(result.plan.runtime_host_destination_path),
               "library-output package should not bundle an executable runtime host into the DLL output slot");
        expect(!result.plan.primary_output_materialized,
               "library-output package should report that the primary DLL binary is not yet materialized");

        const std::string module_definition = read_text(result.plan.module_definition_path);
        expect(module_definition.find("LIBRARY LibraryDemo") != std::string::npos,
               "module-definition file should declare the library name");
        expect(module_definition.find("EXPORTS") != std::string::npos,
               "module-definition file should include an EXPORTS section");
        expect(module_definition.find("InitLibrary") != std::string::npos,
               "module-definition file should export discovered procedure names");
        expect(module_definition.find("AddNumbers") != std::string::npos,
               "module-definition file should export discovered function names");

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        expect(runtime_manifest.find("output_kind=dll") != std::string::npos,
               "library-output manifest should record DLL output kind");
        expect(runtime_manifest.find("module_definition_path=" + quote_manifest_value(result.plan.module_definition_path)) != std::string::npos,
               "library-output manifest should record the emitted module-definition path");
        expect(runtime_manifest.find("export_symbol=InitLibrary") != std::string::npos,
               "library-output manifest should record discovered export symbols");
        expect(runtime_manifest.find("export_symbol=AddNumbers") != std::string::npos,
               "library-output manifest should record all discovered export symbols");
        expect(runtime_manifest.find("primary_output_materialized=false") != std::string::npos,
               "library-output manifest should record the honest non-materialized DLL state");
        expect(runtime_manifest.find("feature_flag=build.output.library_contract|true|build_output") != std::string::npos,
               "library-output manifest should expose the library-contract feature flag");
        expect(debug_manifest.find("output_kind=dll") != std::string::npos,
               "library-output debug manifest should record DLL output kind");
    }

    fs::remove_all(temp_root, ignored);
}

void test_fll_output_package_emits_api_manifest_from_prg_routines() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_fll_contract";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "librarymain.prg",
               "PROCEDURE InitLibrary\nRETURN\nENDPROC\n");
    write_text(project_dir / "helper.prg",
               "FUNCTION AddNumbers\nRETURN 1\nENDFUNC\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "librarydemo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "LibraryDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "LibraryDemo";
    workspace.build_plan.output_path = (output_dir / "LibraryDemo.fll").string();
    workspace.build_plan.output_kind = "fll";
    workspace.build_plan.build_target = "x64 Visual FoxPro library";
    workspace.build_plan.startup_item = "librarymain.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "librarymain.prg", .relative_path = "librarymain.prg", .type_title = "Program"},
        {.record_index = 2U, .name = "helper.prg", .relative_path = "helper.prg", .type_title = "Program"}
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

    expect(plan.ok, "fll-output plan should be created");
    expect(plan.output_kind == copperfin::runtime::BuildOutputKind::fll,
           "fll-output plan should preserve FLL output kind");
    expect(!plan.emit_dotnet_launcher,
           "fll-output plan should not route through .NET launcher emission");
    expect(plan.launcher_mode == "foxpro_library_definition",
           "fll-output plan should switch to the library-definition packaging mode");
    expect(plan.launcher_fallback == "library_binary_generation_pending",
           "fll-output plan should record the honest non-binary fallback state");
    expect(fs::path(plan.launcher_output_path).filename() == "LibraryDemo.fll",
           "fll-output plan should preserve the requested output filename");
    expect(fs::path(plan.module_definition_path).filename() == "LibraryDemo.def",
           "fll-output plan should derive a matching module-definition filename");
    expect(fs::path(plan.fll_api_manifest_path).filename() == "LibraryDemo.fll.api",
           "fll-output plan should derive a matching API-manifest filename");
    expect(plan.exported_symbols.size() == 2U,
           "fll-output plan should discover routine exports from PRG assets");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "fll-output package should materialize");
    if (result.ok) {
        expect(fs::exists(result.plan.module_definition_path),
               "fll-output package should emit a module-definition file");
        expect(fs::exists(result.plan.fll_api_manifest_path),
               "fll-output package should emit an API manifest");
        expect(!fs::exists(result.plan.launcher_output_path),
               "fll-output package should not fake an FLL binary");
        expect(!fs::exists(result.plan.runtime_host_destination_path),
               "fll-output package should not bundle an executable runtime host into the FLL output slot");
        expect(!result.plan.primary_output_materialized,
               "fll-output package should report that the primary FLL binary is not yet materialized");

        const std::string module_definition = read_text(result.plan.module_definition_path);
        expect(module_definition.find("LIBRARY LibraryDemo") != std::string::npos,
               "fll-output module-definition file should declare the library name");
        expect(module_definition.find("InitLibrary") != std::string::npos,
               "fll-output module-definition file should export discovered procedure names");
        expect(module_definition.find("AddNumbers") != std::string::npos,
               "fll-output module-definition file should export discovered function names");

        const std::string api_manifest = read_text(result.plan.fll_api_manifest_path);
        expect(api_manifest.find("output_kind=fll") != std::string::npos,
               "fll-output API manifest should declare the FLL output kind");
        expect(api_manifest.find("library_file=LibraryDemo.fll") != std::string::npos,
               "fll-output API manifest should name the requested FLL file");
        expect(api_manifest.find("registration_command=SET LIBRARY TO") != std::string::npos,
               "fll-output API manifest should declare the registration command");
        expect(api_manifest.find("release_command=RELEASE LIBRARY") != std::string::npos,
               "fll-output API manifest should declare the release command");
        expect(api_manifest.find("additive_supported=true") != std::string::npos,
               "fll-output API manifest should declare additive loading support");
        expect(api_manifest.find("function=InitLibrary") != std::string::npos,
               "fll-output API manifest should list discovered procedure names");
        expect(api_manifest.find("function=AddNumbers") != std::string::npos,
               "fll-output API manifest should list discovered function names");

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        expect(runtime_manifest.find("output_kind=fll") != std::string::npos,
               "fll-output manifest should record FLL output kind");
        expect(runtime_manifest.find("fll_api_manifest_path=" + quote_manifest_value(result.plan.fll_api_manifest_path)) != std::string::npos,
               "fll-output manifest should record the emitted API-manifest path");
        expect(runtime_manifest.find("feature_flag=build.output.library_contract|true|build_output") != std::string::npos,
               "fll-output manifest should expose the library-contract feature flag");
        expect(runtime_manifest.find("feature_flag=build.output.fll_api_contract|true|build_output") != std::string::npos,
               "fll-output manifest should expose the FLL API-contract feature flag");
        expect(debug_manifest.find("output_kind=fll") != std::string::npos,
               "fll-output debug manifest should record FLL output kind");
    }

    fs::remove_all(temp_root, ignored);
}

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
               "LOCAL nValue\n"
               "nValue = 1\n"
               "DO worker\n"
               "RETURN\n"
               "PROCEDURE worker\n"
               "WAIT WINDOW 'hello'\n"
               "RETURN\n"
               "ENDPROC\n");
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

    expect(plan.ok, "fxp-output plan should be created");
    expect(plan.output_kind == copperfin::runtime::BuildOutputKind::fxp,
           "fxp-output plan should preserve FXP output kind");
    expect(!plan.emit_dotnet_launcher,
           "fxp-output plan should not route through .NET launcher emission");
    expect(plan.launcher_mode == "foxpro_tokenized_contract",
           "fxp-output plan should switch to the tokenized-contract packaging mode");
    expect(plan.launcher_fallback == "fxp_binary_generation_pending",
           "fxp-output plan should record the honest non-binary fallback state");
    expect(fs::path(plan.launcher_output_path).filename() == "CompileDemo.fxp",
           "fxp-output plan should preserve the requested output filename");
    expect(fs::path(plan.fxp_token_manifest_path).filename() == "CompileDemo.fxp.tokens",
           "fxp-output plan should derive a matching token-manifest filename");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "fxp-output package should materialize");
    if (result.ok) {
        expect(fs::exists(result.plan.fxp_token_manifest_path),
               "fxp-output package should emit a token manifest");
        expect(!fs::exists(result.plan.launcher_output_path),
               "fxp-output package should not fake an FXP binary");
        expect(!fs::exists(result.plan.runtime_host_destination_path),
               "fxp-output package should not bundle an executable runtime host into the FXP output slot");
        expect(!result.plan.primary_output_materialized,
               "fxp-output package should report that the primary FXP binary is not yet materialized");

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
        expect(token_manifest.find("DO worker") != std::string::npos,
               "fxp-output token manifest should preserve logical statement text");
        expect(token_manifest.find("statement=worker|") != std::string::npos,
               "fxp-output token manifest should include routine-scope statements");
        expect(token_manifest.find("WAIT WINDOW 'hello'") != std::string::npos,
               "fxp-output token manifest should preserve routine statement text");

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        expect(runtime_manifest.find("output_kind=fxp") != std::string::npos,
               "fxp-output manifest should record FXP output kind");
        expect(runtime_manifest.find("fxp_token_manifest_path=" + quote_manifest_value(result.plan.fxp_token_manifest_path)) != std::string::npos,
               "fxp-output manifest should record the emitted token-manifest path");
        expect(runtime_manifest.find("feature_flag=build.output.fxp_token_contract|true|build_output") != std::string::npos,
               "fxp-output manifest should expose the FXP token-contract feature flag");
        expect(debug_manifest.find("output_kind=fxp") != std::string::npos,
               "fxp-output debug manifest should record FXP output kind");
        expect(debug_manifest.find("launcher_mode=foxpro_tokenized_contract") != std::string::npos,
               "fxp-output debug manifest should record the tokenized-contract mode");
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

    const std::string debug_manifest = copperfin::runtime::build_debug_manifest_text(plan);
    const std::string expected_roots_line =
        "source_roots=" + source_root.lexically_normal().string() + ";" +
        (output_dir / "DebugRootsOrder" / "content").lexically_normal().string();
    expect(debug_manifest.find(expected_roots_line) != std::string::npos,
           "debug manifest should preserve source-first source_roots ordering");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_materialize_runtime_package();
    test_generated_launcher_forwards_manifest_and_debug_flag();
    test_materialize_excluded_xasset_startup_package();
    test_dotnet_launcher_request_falls_back_to_native_host_when_unavailable();
    test_library_output_package_emits_module_definition_from_prg_routines();
    test_fll_output_package_emits_api_manifest_from_prg_routines();
    test_fxp_output_package_emits_token_manifest_from_prg_statements();
    test_startup_dbf_companion_assets_are_staged();
    test_security_enabled_runtime_host_name_validation();
    test_materialize_fails_before_asset_staging_when_runtime_host_source_is_invalid();
    test_startup_prg_extension_matching_is_case_insensitive();
    test_startup_asset_is_staged_even_when_marked_excluded();
    test_missing_startup_record_surfaces_plan_warnings_and_disables_debug_startup_support();
    test_manifest_asset_lines_include_copy_state_contract();
    test_debug_source_roots_are_unique_when_source_and_content_paths_match();
    test_debug_source_roots_preserve_source_first_and_content_second_order();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
