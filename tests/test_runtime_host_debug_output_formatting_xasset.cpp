// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_runtime_host_debug_output_support.h"

namespace {

void write_synthetic_faulting_xasset(
    const std::filesystem::path& table_path,
    copperfin::studio::StudioAssetKind kind) {
    if (kind == copperfin::studio::StudioAssetKind::menu) {
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "OBJTYPE", .type = 'N', .length = 3U},
            {.name = "NAME", .type = 'M', .length = 4U},
            {.name = "PROMPT", .type = 'M', .length = 4U},
            {.name = "COMMAND", .type = 'M', .length = 4U},
            {.name = "LEVELNAME", .type = 'C', .length = 24U},
            {.name = "ITEMNUM", .type = 'C', .length = 8U}
        };
        const auto create_result = copperfin::vfp::create_dbf_table_file(
            table_path.string(),
            fields,
            {
                {"2", "MainMenu", "", "", "", ""},
                {"3", "FaultItem", "Fault", "before_action = \"kept\"\nfault_value = LOG(-1)\nafter_action = \"continued\"", "MainMenu", "1"}
            });
        expect(create_result.ok, "synthetic MNX fault fixture should be created");
        return;
    }

    const std::string object_name = kind == copperfin::studio::StudioAssetKind::class_library
        ? "clsFault"
        : "frmFault";
    const std::string base_class = kind == copperfin::studio::StudioAssetKind::class_library
        ? "custom"
        : "form";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "PLATFORM", .type = 'C', .length = 16U},
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::string methods =
        "PROCEDURE Load\n"
        "startup_value = \"ready\"\n"
        "ENDPROC\n"
        "PROCEDURE HandleFault\n"
        "before_action = \"kept\"\n"
        "fault_value = LOG(-1)\n"
        "after_action = \"continued\"\n"
        "ENDPROC\n";
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        fields,
        {{"WINDOWS", object_name, "", base_class, methods}});
    expect(create_result.ok, "synthetic SCX/VCX fault fixture should be created");
}

void write_synthetic_faulting_layout(const std::filesystem::path& table_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "PLATFORM", .type = 'C', .length = 16U},
        {.name = "OBJTYPE", .type = 'N', .length = 3U},
        {.name = "OBJCODE", .type = 'N', .length = 3U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::string methods =
        "PROCEDURE HandleFault\n"
        "before_action = \"kept\"\n"
        "fault_value = LOG(-1)\n"
        "after_action = \"continued\"\n"
        "ENDPROC\n";
    const std::vector<std::vector<std::string>> records{
        {"WINDOWS", "1", "0", "ENVIRONMENT = 1", methods},
        {"WINDOWS", "9", "4", "", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        fields,
        records);
    expect(create_result.ok, "synthetic FRX/LBX fault fixture should be created");
}

}  // namespace

void test_runtime_host_compatibility_launcher_note_reflects_xasset_fallback(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_compatibility_note_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);
    ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());

    const fs::path table_path = temp_root / "broken.scx";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_text(table_path, "not-a-dbf");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=BrokenXAsset\n"
        "startup_item=broken.scx\n"
        "startup_source=" + table_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {"--manifest", manifest_path.string()},
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "compatibility launcher stdout:\n" << process.stdout_text << "\n";
        std::cerr << "compatibility launcher stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0,
           "#3736: compatibility-launcher fallback should still exit successfully for non-PRG startup assets");
    expect(process.stdout_text.find("runtime.mode: compatibility-launcher") != std::string::npos,
           "#3736: compatibility-launcher fallback should report compatibility-launcher mode");
    expect(process.stdout_text.find(
               "launch.note: Startup asset is not a PRG file and could not be materialized for xAsset bootstrap. This launch is falling back to compatibility-launcher mode.") != std::string::npos,
           "#3736: compatibility-launcher fallback should describe xAsset bootstrap fallback instead of claiming non-PRG startup is only a later runtime slice");
    expect(process.stdout_text.find("later runtime slice") == std::string::npos,
           "#3736: compatibility-launcher fallback should not emit the stale later-runtime-slice wording");
    expect(process.stdout_text.find("launch.note: ") != std::string::npos,
           "#3736: compatibility-launcher fallback should continue surfacing a second detailed launch note");
    expect(process.stdout_text.find("debug.breakpoint_support: false") != std::string::npos,
           "#3736: compatibility-launcher fallback should keep breakpoint support disabled");
    expect(process.stdout_text.find("debug.step_support: false") != std::string::npos,
           "#3736: compatibility-launcher fallback should keep step-debug support disabled");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_supports_breakpoint_management_commands(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_breakpoint_command_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path startup_path = temp_root / "main.prg";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_text(
        startup_path,
        "LOCAL nValue\n"
        "nValue = 1\n"
        "nValue = 2\n"
        "RETURN\n");
    const std::string expected_startup_path = startup_path.string();
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=BreakpointDemo\n"
        "startup_item=main.prg\n"
        "startup_source=" + quote_manifest_value(startup_path.string()) + "\n"
        "working_directory=" + quote_manifest_value(temp_root.string()) + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--debug",
            "--debug-command", "break:add:2",
            "--debug-command", "break:list",
            "--debug-command", "break:clear",
            "--debug-command", "break:list",
            "--debug-command", "break:add:3",
            "--debug-command", "continue"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "breakpoint command stdout:\n" << process.stdout_text << "\n";
        std::cerr << "breakpoint command stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0, "runtime host breakpoint-command smoke should exit successfully");
    expect(process.stdout_text.find("debug.command[0]: break:add:2") != std::string::npos,
           "runtime host should report breakpoint add commands");
    expect(process.stdout_text.find("debug.command[1]: break:list") != std::string::npos,
           "runtime host should report breakpoint list commands");
    expect(process.stdout_text.find("debug.command[2]: break:clear") != std::string::npos,
           "runtime host should report breakpoint clear commands");
    expect(process.stdout_text.find("debug.breakpoint.count: 1") != std::string::npos,
           "runtime host should report one active breakpoint after add");
    expect(process.stdout_text.find("debug.breakpoint[0]: " + expected_startup_path + ":2") != std::string::npos,
           "runtime host should list the added breakpoint against the startup source");
    expect(process.stdout_text.find("debug.breakpoint.count: 0") != std::string::npos,
           "runtime host should report an empty breakpoint inventory after clear");
    expect(process.stdout_text.find("debug.command[5]: continue") != std::string::npos,
           "runtime host should continue after breakpoint management commands");
    expect(process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
           "runtime host should still pause on the live managed breakpoint");
    expect(process.stdout_text.find("debug.location: " + expected_startup_path + ":3") != std::string::npos,
           "runtime host should break on the breakpoint added after clear");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_supports_single_breakpoint_removal(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_breakpoint_remove_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path startup_path = temp_root / "main.prg";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_text(
        startup_path,
        "LOCAL nValue\n"
        "nValue = 1\n"
        "nValue = 2\n"
        "RETURN\n");
    const std::string expected_startup_path = startup_path.string();
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=BreakpointRemoveDemo\n"
        "startup_item=main.prg\n"
        "startup_source=" + quote_manifest_value(startup_path.string()) + "\n"
        "working_directory=" + quote_manifest_value(temp_root.string()) + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--debug",
            "--debug-command", "break:add:2",
            "--debug-command", "break:add:3",
            "--debug-command", "break:remove:2",
            "--debug-command", "break:list",
            "--debug-command", "continue"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "breakpoint remove stdout:\n" << process.stdout_text << "\n";
        std::cerr << "breakpoint remove stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0, "runtime host single-breakpoint removal smoke should exit successfully");
    expect(process.stdout_text.find("debug.command[2]: break:remove:2") != std::string::npos,
           "runtime host should report breakpoint remove commands");
    expect(process.stdout_text.find("debug.command[3]: break:list") != std::string::npos,
           "runtime host should report breakpoint list after removal");
    expect(process.stdout_text.find("debug.breakpoint[0]: " + expected_startup_path + ":2") != std::string::npos,
           "runtime host should initially register the first breakpoint before removal");
    expect(process.stdout_text.find("debug.breakpoint[1]: " + expected_startup_path + ":3") != std::string::npos,
           "runtime host should initially register the second breakpoint before removal");
    expect(process.stdout_text.find("debug.breakpoint[0]: " + expected_startup_path + ":3") != std::string::npos,
           "runtime host should retain the unrelated breakpoint after single removal");
    expect(process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
           "runtime host should still pause on the remaining breakpoint");
    expect(process.stdout_text.find("debug.location: " + expected_startup_path + ":3") != std::string::npos,
           "runtime host should pause on the surviving breakpoint after removing the earlier line");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_prefers_debug_manifest_for_implicit_debug_launches(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_implicit_debug_manifest_tests";
    const fs::path package_root = temp_root / "package";
    const fs::path content_root = package_root / "content";
    const fs::path source_root = temp_root / "source";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(content_root);
    fs::create_directories(source_root);

    const fs::path deployed_runtime_host = package_root / fs::path(runtime_host_path).filename();
    fs::copy_file(runtime_host_path, deployed_runtime_host, fs::copy_options::overwrite_existing);
#if defined(__unix__) || defined(__APPLE__)
    fs::permissions(
        deployed_runtime_host,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add,
        ignored);
#endif

    const fs::path release_startup_path = content_root / "release_main.prg";
    const fs::path debug_startup_path = source_root / "debug_main.prg";
    const fs::path manifest_path = package_root / "app.cfmanifest";
    const fs::path debug_manifest_path = package_root / "app.cfdebug";
    write_text(
        release_startup_path,
        "LOCAL cMode\n"
        "cMode = 'release'\n"
        "RETURN\n");
    write_text(
        debug_startup_path,
        "LOCAL cMode\n"
        "cMode = 'debug'\n"
        "RETURN\n");
    expect(
        quote_manifest_value("C:\\fixture\\release_main.prg") == "C:\\\\fixture\\\\release_main.prg",
        "#3669: manual manifest fixtures should escape Windows path separators before release filenames");
    const std::string serialized_package_root = quote_manifest_value(package_root.string());
    const std::string serialized_content_root = quote_manifest_value(content_root.string());
    const std::string serialized_source_root = quote_manifest_value(source_root.string());
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=ImplicitReleaseManifest\n"
        "package_root=" + serialized_package_root + "\n"
        "content_root=" + serialized_content_root + "\n"
        "startup_item=release_main.prg\n"
        "startup_source=" + quote_manifest_value(release_startup_path.string()) + "\n"
        "working_directory=" + serialized_content_root + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");
    write_text(
        debug_manifest_path,
        "debug_manifest_version=2\n"
        "project_title=ImplicitDebugManifest\n"
        "package_root=" + serialized_package_root + "\n"
        "content_root=" + serialized_content_root + "\n"
        "startup_item=debug_main.prg\n"
        "startup_source=" + quote_manifest_value(debug_startup_path.string()) + "\n"
        "working_directory=" + serialized_source_root + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    const auto debug_process = run_process_capture(
        deployed_runtime_host.string(),
        {
            "--debug",
            "--debug-command", "break:add:2",
            "--debug-command", "continue"
        },
        package_root);

    if (debug_process.exit_code != 0) {
        std::cerr << "implicit debug manifest stdout:\n" << debug_process.stdout_text << "\n";
        std::cerr << "implicit debug manifest stderr:\n" << debug_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(debug_process.exit_code == 0,
           "#3669: implicit debug launches should succeed when app.cfdebug is present");
    expect(debug_process.stdout_text.find("debug.location: " + debug_startup_path.string() + ":2") != std::string::npos,
           "#3669: implicit debug launches should prefer app.cfdebug over app.cfmanifest");

    write_text(debug_manifest_path, "manifest_version=1\nproject_title=BrokenDebugManifest\n");

    const auto non_debug_process = run_process_capture(
        deployed_runtime_host.string(),
        {},
        package_root);

    if (non_debug_process.exit_code != 0) {
        std::cerr << "implicit non-debug manifest stdout:\n" << non_debug_process.stdout_text << "\n";
        std::cerr << "implicit non-debug manifest stderr:\n" << non_debug_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(non_debug_process.exit_code == 0,
           "#3669: implicit non-debug launches should continue to use app.cfmanifest");
    expect(non_debug_process.stdout_text.find("project.title: ImplicitReleaseManifest") != std::string::npos,
           "#3669: implicit non-debug launches should report the app.cfmanifest project identity");
    expect(non_debug_process.stdout_text.find("startup.item: release_main.prg") != std::string::npos,
           "#3669: implicit non-debug launches should report the app.cfmanifest startup identity");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_reports_xasset_pause_identity(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_debug_output_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "demo.scx";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_synthetic_form_asset(table_path);

    copperfin::studio::StudioOpenRequest request{};
    request.path = table_path.string();
    request.read_only = true;
    request.load_full_table = true;
    const auto open_result = copperfin::studio::open_document(request);
    expect(open_result.ok, "runtime-host debugger fixture should reopen as a full SCX document");
    if (!open_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const auto model = copperfin::runtime::build_xasset_executable_model(open_result.document);
    expect(model.ok, "runtime-host debugger fixture should yield an xAsset executable model");
    const auto page_activate = find_action(model, "frmdemo.pgfmain.page2.activate");
    expect(page_activate.has_value(), "synthetic form fixture should expose the nested page action");
    if (!model.ok || !page_activate.has_value()) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const std::string bootstrap =
        copperfin::runtime::build_xasset_bootstrap_source(model, true, {}, true);
    const std::size_t breakpoint_line = find_breakpoint_line_for_routine_statement(
        bootstrap,
        page_activate->routine_name,
        "THISFORM.Refresh");
    expect(breakpoint_line != 0U, "synthetic xAsset bootstrap should contain a breakpointable nested page statement");
    if (breakpoint_line == 0U) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=DemoForm\n"
        "startup_item=demo.scx\n"
        "startup_source=" + table_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--debug",
            "--breakpoint", std::to_string(breakpoint_line),
            "--debug-command", "continue",
            "--debug-command", "select:frmdemo.pgfmain.page2.activate"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "runtime host stdout:\n" << process.stdout_text << "\n";
        std::cerr << "runtime host stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0, "runtime host xAsset debugger smoke should exit successfully");
    expect(process.stdout_text.find("runtime.mode: xasset-bootstrap") != std::string::npos,
           "runtime host should report xasset-bootstrap mode");
    expect(process.stdout_text.find(".detail: __cf_xasset_instance.__cf_frmDemo_Load()") != std::string::npos,
           "runtime host should invoke packaged form lifecycle methods on the generated form instance");
    expect(process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
           "runtime host should pause on the nested xAsset breakpoint");
    expect(process.stdout_text.find("debug.command[1]: select:frmdemo.pgfmain.page2.activate") != std::string::npos,
           "runtime host should report the dispatched xAsset debug command");
    expect(process.stdout_text.find("debug.xasset.action_id: " + page_activate->action_id) != std::string::npos,
           "runtime host pause output should report the originating xAsset action id");
    expect(process.stdout_text.find("debug.xasset.record_index: " + std::to_string(page_activate->record_index)) != std::string::npos,
           "runtime host pause output should report the originating xAsset record index");
    expect(process.stdout_text.find("debug.xasset.kind: " + page_activate->kind) != std::string::npos,
           "runtime host pause output should report the xAsset action kind");
    expect(process.stdout_text.find("debug.xasset.title: " + page_activate->title) != std::string::npos,
           "runtime host pause output should report the xAsset action title");
    expect(process.stdout_text.find("debug.frame[0]: " + page_activate->routine_name + "@") != std::string::npos,
           "runtime host pause stack should still identify the generated action routine");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_supports_xasset_action_breakpoint_commands(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_xasset_breakpoint_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "demo.scx";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_synthetic_form_asset(table_path);

    copperfin::studio::StudioOpenRequest request{};
    request.path = table_path.string();
    request.read_only = true;
    request.load_full_table = true;
    const auto open_result = copperfin::studio::open_document(request);
    expect(open_result.ok, "xAsset action-breakpoint fixture should reopen as a full SCX document");
    if (!open_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const auto model = copperfin::runtime::build_xasset_executable_model(open_result.document);
    expect(model.ok, "xAsset action-breakpoint fixture should yield an executable model");
    const auto page_activate = find_action(model, "frmdemo.pgfmain.page2.activate");
    const auto root_activate = find_action(model, "frmdemo.activate");
    expect(page_activate.has_value(), "xAsset action-breakpoint fixture should expose the nested page action");
    expect(root_activate.has_value(), "xAsset action-breakpoint fixture should expose the root form activate action");
    if (!model.ok || !page_activate.has_value() || !root_activate.has_value()) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const std::string bootstrap =
        copperfin::runtime::build_xasset_bootstrap_source(model, true, {}, true);
    const std::size_t first_breakpoint_line =
        find_first_breakpoint_line_for_routine(bootstrap, page_activate->routine_name);
    expect(first_breakpoint_line != 0U, "xAsset action-breakpoint fixture should resolve the first executable line");
    if (first_breakpoint_line == 0U) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=DemoFormBreakpoint\n"
        "startup_item=demo.scx\n"
        "startup_source=" + table_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    const auto add_process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--debug",
            "--debug-command", "continue",
            "--debug-command", "break:add-action:frmdemo.pgfmain.page2.activate",
            "--debug-command", "break:list",
            "--debug-command", "select:frmdemo.pgfmain.page2.activate"
        },
        temp_root);

    if (add_process.exit_code != 0) {
        std::cerr << "xasset add-action stdout:\n" << add_process.stdout_text << "\n";
        std::cerr << "xasset add-action stderr:\n" << add_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(add_process.exit_code == 0, "runtime host xAsset add-action breakpoint smoke should exit successfully");
    expect(add_process.stdout_text.find("debug.command[1]: break:add-action:frmdemo.pgfmain.page2.activate") != std::string::npos,
           "runtime host should report xAsset add-action commands");
    expect(add_process.stdout_text.find("debug.breakpoint.count: 1") != std::string::npos,
           "runtime host should report one active xAsset action breakpoint");
    expect(add_process.stdout_text.find("_copperfin_host_bootstrap_") != std::string::npos &&
               add_process.stdout_text.find(".prg:" + std::to_string(first_breakpoint_line)) != std::string::npos,
           "runtime host should list the resolved bootstrap breakpoint for the xAsset action");
    expect(add_process.stdout_text.find("debug.breakpoint[0].xasset.action_id: " + page_activate->action_id) != std::string::npos,
           "runtime host should surface xAsset action ids in breakpoint inventory");
    expect(add_process.stdout_text.find("debug.breakpoint[0].xasset.title: " + page_activate->title) != std::string::npos,
           "runtime host should surface xAsset action titles in breakpoint inventory");
    expect(add_process.stdout_text.find("debug.command[3]: select:frmdemo.pgfmain.page2.activate") != std::string::npos,
           "runtime host should report the dispatched xAsset action after add-action");
    expect(add_process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
           "runtime host should pause on the xAsset action breakpoint added by action id");

    const auto remove_process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--debug",
            "--debug-command", "continue",
            "--debug-command", "break:add-action:frmdemo.activate",
            "--debug-command", "break:remove-action:frmdemo.activate",
            "--debug-command", "break:list",
            "--debug-command", "select:frmdemo.activate"
        },
        temp_root);

    if (remove_process.exit_code != 0) {
        std::cerr << "xasset remove-action stdout:\n" << remove_process.stdout_text << "\n";
        std::cerr << "xasset remove-action stderr:\n" << remove_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(remove_process.exit_code == 0, "runtime host xAsset remove-action breakpoint smoke should exit successfully");
    expect(remove_process.stdout_text.find("debug.command[2]: break:remove-action:frmdemo.activate") != std::string::npos,
           "runtime host should report xAsset remove-action commands");
    expect(remove_process.stdout_text.find("debug.breakpoint.count: 0") != std::string::npos,
           "runtime host should report an empty inventory after removing the xAsset action breakpoint");
    expect(remove_process.stdout_text.find("debug.command[4]: select:frmdemo.activate") != std::string::npos,
           "runtime host should still dispatch the xAsset action after remove-action");
    expect(remove_process.stdout_text.find("debug.reason: event_loop") != std::string::npos,
           "runtime host should return to the event loop instead of breaking after removing the xAsset action breakpoint");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_surfaces_xasset_breakpoint_metadata_in_pause_output(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_xasset_pause_breakpoint_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "demo.scx";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_synthetic_form_asset(table_path);

    copperfin::studio::StudioOpenRequest request{};
    request.path = table_path.string();
    request.read_only = true;
    request.load_full_table = true;
    const auto open_result = copperfin::studio::open_document(request);
    expect(open_result.ok, "xAsset pause-breakpoint fixture should reopen as a full SCX document");
    if (!open_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const auto model = copperfin::runtime::build_xasset_executable_model(open_result.document);
    expect(model.ok, "xAsset pause-breakpoint fixture should yield an executable model");
    const auto page_activate = find_action(model, "frmdemo.pgfmain.page2.activate");
    expect(page_activate.has_value(), "xAsset pause-breakpoint fixture should expose the nested page action");
    if (!model.ok || !page_activate.has_value()) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const std::string bootstrap =
        copperfin::runtime::build_xasset_bootstrap_source(model, true, {}, true);
    const std::size_t first_breakpoint_line =
        find_first_breakpoint_line_for_routine(bootstrap, page_activate->routine_name);
    expect(first_breakpoint_line != 0U, "xAsset pause-breakpoint fixture should resolve the first executable line");
    if (first_breakpoint_line == 0U) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=DemoFormPauseBreakpoint\n"
        "startup_item=demo.scx\n"
        "startup_source=" + table_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--debug",
            "--debug-command", "continue",
            "--debug-command", "break:add-action:frmdemo.pgfmain.page2.activate",
            "--debug-command", "select:frmdemo.pgfmain.page2.activate"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "xasset pause-breakpoint stdout:\n" << process.stdout_text << "\n";
        std::cerr << "xasset pause-breakpoint stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0, "runtime host xAsset pause-breakpoint smoke should exit successfully");
    expect(process.stdout_text.find("break:list") == std::string::npos,
           "pause-breakpoint smoke should not rely on explicit breakpoint inventory commands");
    expect(process.stdout_text.find("debug.command[1]: break:add-action:frmdemo.pgfmain.page2.activate") != std::string::npos,
           "runtime host should report xAsset add-action commands in pause-breakpoint smoke");
    expect(process.stdout_text.find("debug.command[2]: select:frmdemo.pgfmain.page2.activate") != std::string::npos,
           "runtime host should report the dispatched xAsset action in pause-breakpoint smoke");
    expect(process.stdout_text.find("debug.breakpoint.count: 1") != std::string::npos,
           "runtime host pause output should report one active xAsset action breakpoint");
    expect(process.stdout_text.find("_copperfin_host_bootstrap_") != std::string::npos &&
               process.stdout_text.find(".prg:" + std::to_string(first_breakpoint_line)) != std::string::npos,
           "runtime host pause output should still report the resolved bootstrap breakpoint");
    expect(process.stdout_text.find("debug.breakpoint[0].xasset.action_id: " + page_activate->action_id) != std::string::npos,
           "runtime host pause output should surface xAsset action ids for active breakpoints");
    expect(process.stdout_text.find("debug.breakpoint[0].xasset.title: " + page_activate->title) != std::string::npos,
           "runtime host pause output should surface xAsset action titles for active breakpoints");
    expect(process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
           "runtime host should pause on the xAsset action breakpoint in pause-breakpoint smoke");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_removes_xasset_bootstrap_after_execution(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_xasset_bootstrap_cleanup_tests";
    const fs::path table_path = temp_root / "demo.scx";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    write_synthetic_form_asset(table_path);

    copperfin::studio::StudioOpenRequest request{};
    request.path = table_path.string();
    request.read_only = true;
    request.load_full_table = true;
    const auto open_result = copperfin::studio::open_document(request);
    expect(open_result.ok, "xAsset bootstrap cleanup fixture should reopen as a full SCX document");
    if (!open_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const auto model = copperfin::runtime::build_xasset_executable_model(open_result.document);
    expect(model.ok, "xAsset bootstrap cleanup fixture should yield an executable xAsset model");
    const auto page_activate = find_action(model, "frmdemo.pgfmain.page2.activate");
    expect(page_activate.has_value(), "xAsset bootstrap cleanup fixture should expose the nested page action");
    if (!model.ok || !page_activate.has_value()) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=DemoFormCleanup\n"
        "startup_item=demo.scx\n"
        "startup_source=" + table_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    const auto run_debug_inventory = [&](const std::string& label) {
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "break:add-action:frmdemo.pgfmain.page2.activate",
                "--debug-command", "break:list"
            },
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << label << " stdout:\n" << process.stdout_text << "\n";
            std::cerr << label << " stderr:\n" << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0, "runtime host xAsset bootstrap cleanup smoke should exit successfully");
        const std::string breakpoint_entry = output_line_value(process.stdout_text, "debug.breakpoint[0]: ");
        expect(!breakpoint_entry.empty(), "runtime host should report the generated xAsset bootstrap breakpoint entry");
        const std::string bootstrap_path = breakpoint_entry_path(breakpoint_entry);
        expect(!bootstrap_path.empty(), "runtime host should report a parseable xAsset bootstrap breakpoint path");
        if (!bootstrap_path.empty()) {
            expect(bootstrap_path.find("demo_copperfin_host_bootstrap_") != std::string::npos,
                   "runtime host should use a per-run xAsset bootstrap file name");
            expect(!fs::exists(bootstrap_path),
                   "runtime host should remove the xAsset bootstrap file after execution completes");
        }
        return breakpoint_entry;
    };

    const std::string first_breakpoint_entry = run_debug_inventory("xasset-bootstrap-cleanup first run");
    const std::string second_breakpoint_entry = run_debug_inventory("xasset-bootstrap-cleanup second run");
    if (!first_breakpoint_entry.empty() && !second_breakpoint_entry.empty()) {
        expect(first_breakpoint_entry != second_breakpoint_entry,
               "runtime host should not reuse one deterministic xAsset bootstrap path across repeated launches");
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_cleans_failed_xasset_bootstrap_write(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin_runtime_host_xasset_bootstrap_write_failure_tests";
    const fs::path table_path = temp_root / "demo.scx";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    write_synthetic_form_asset(table_path);
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=DemoFormWriteFailure\n"
        "startup_item=demo.scx\n"
        "startup_source=" + table_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    {
        ScopedEnvironmentValue tmpdir("TMPDIR", temp_root.string());
        ScopedEnvironmentValue temp("TEMP", temp_root.string());
        ScopedEnvironmentValue tmp("TMP", temp_root.string());
        ScopedEnvironmentValue fail_marker(
            "COPPERFIN_TEST_FAIL_WRITE_PATH_CONTAINS",
            "demo_copperfin_host_bootstrap_");
        ScopedEnvironmentValue fail_stage("COPPERFIN_TEST_FAIL_WRITE_STAGE", "xasset-bootstrap");

        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string()},
            temp_root);
        if (process.exit_code != 0) {
            std::cerr << "xAsset write-failure stdout:\n" << process.stdout_text << "\n";
            std::cerr << "xAsset write-failure stderr:\n" << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "failed xAsset bootstrap writes should use the compatibility-launcher fallback");
        expect(process.stdout_text.find("runtime.mode: compatibility-launcher") != std::string::npos,
               "failed xAsset bootstrap writes should report compatibility-launcher mode");
    }

    bool bootstrap_leaked = false;
    for (fs::directory_iterator it(temp_root, ignored), end; !ignored && it != end; it.increment(ignored)) {
        const std::string filename = it->path().filename().string();
        if (filename.find("_copperfin_host_bootstrap_") != std::string::npos &&
            it->path().extension() == ".prg") {
            bootstrap_leaked = true;
            break;
        }
    }
    expect(!bootstrap_leaked,
           "failed xAsset bootstrap writes should remove the partial temporary source");
    fs::remove_all(temp_root, ignored);
}

void test_runtime_host_contains_executable_xasset_action_faults(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    struct FaultingAssetCase {
        const char* filename;
        copperfin::studio::StudioAssetKind kind;
        const char* action_id;
    };
    const FaultingAssetCase cases[]{
        {"fault.scx", copperfin::studio::StudioAssetKind::form, "frmfault.handlefault"},
        {"fault.vcx", copperfin::studio::StudioAssetKind::class_library, "clsfault.handlefault"},
        {"fault.mnx", copperfin::studio::StudioAssetKind::menu, "faultitem"}
    };

    for (const auto& asset_case : cases) {
        const fs::path temp_root = fs::temp_directory_path() /
            (std::string("copperfin_runtime_host_xasset_action_fault_") + asset_case.filename);
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path table_path = temp_root / asset_case.filename;
        const fs::path manifest_path = temp_root / "app.cfmanifest";
        const fs::path locale_root = temp_root / "locales";
        write_synthetic_faulting_xasset(table_path, asset_case.kind);
        write_runtime_host_usage_catalogs(locale_root);
        write_text(
            manifest_path,
            "manifest_version=1\n"
            "project_title=XAssetFaultRecovery\n"
            "startup_item=" + std::string(asset_case.filename) + "\n"
            "startup_source=" + table_path.string() + "\n"
            "working_directory=" + temp_root.string() + "\n"
            "security_enabled=false\n"
            "security_role=\n"
            "security_mode=native\n"
            "dotnet_story=none\n");

        ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "en-US");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "continue",
                "--debug-command", std::string("invoke:") + asset_case.action_id,
                "--debug-command", "watch:before_action",
                "--debug-command", "continue",
                "--debug-command", "watch:after_action"
            },
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << asset_case.filename << " xAsset fault stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << asset_case.filename << " xAsset fault stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }
        const std::string prefix = std::string("#4626 ") + asset_case.filename + ": ";
        expect(process.exit_code == 0, prefix + "xAsset action fault should recover in the same host process");
        expect(process.stdout_text.find("runtime.mode: xasset-bootstrap") != std::string::npos,
               prefix + "xAsset action fault should use the executable xAsset bootstrap");
        expect(process.stdout_text.find("debug.command[1]: invoke:" + std::string(asset_case.action_id)) != std::string::npos,
               prefix + "xAsset action invocation should preserve its command identity");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               prefix + "xAsset action fault should emit structured error status");
        expect(process.stdout_text.find("debug.reason: error") != std::string::npos,
               prefix + "xAsset action fault should preserve the error pause state");
        expect(process.stdout_text.find("debug.watch.value: kept") != std::string::npos,
               prefix + "xAsset action fault should preserve pre-fault watch state");
        expect(process.stdout_text.find("debug.reason: event_loop") != std::string::npos,
               prefix + "xAsset action fault should return to the event loop after continue");
        expect(process.stdout_text.find("after_action = \"continued\"") != std::string::npos,
               prefix + "xAsset action fault should execute post-fault code after continue");
        expect(process.stdout_text.find("terminate called") == std::string::npos,
               prefix + "xAsset action fault should not terminate the runtime host");

        if (failures == 0) {
            fs::remove_all(temp_root, ignored);
        }
    }
}

void test_runtime_host_contains_report_label_action_faults(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const char* filenames[]{"fault.frx", "fault.lbx"};
    for (const char* filename : filenames) {
        const fs::path temp_root = fs::temp_directory_path() /
            (std::string("copperfin_runtime_host_report_label_action_fault_") + filename);
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path table_path = temp_root / filename;
        const fs::path manifest_path = temp_root / "app.cfmanifest";
        const fs::path locale_root = temp_root / "locales";
        write_synthetic_faulting_layout(table_path);
        write_runtime_host_usage_catalogs(locale_root);
        write_text(
            manifest_path,
            "manifest_version=1\n"
            "project_title=ReportLabelFaultRecovery\n"
            "startup_item=" + std::string(filename) + "\n"
            "startup_source=" + table_path.string() + "\n"
            "working_directory=" + temp_root.string() + "\n"
            "security_enabled=false\n"
            "security_role=\n"
            "security_mode=native\n"
            "dotnet_story=none\n");

        ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "en-US");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "continue",
                "--debug-command", "invoke:handlefault",
                "--debug-command", "watch:before_action",
                "--debug-command", "continue",
                "--debug-command", "watch:after_action"
            },
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << filename << " report/label fault stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << filename << " report/label fault stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        const std::string prefix = std::string("#4627 ") + filename + ": ";
        expect(process.exit_code == 0, prefix + "report/label action fault should recover in the same host process");
        expect(process.stdout_text.find("runtime.mode: xasset-bootstrap") != std::string::npos,
               prefix + "report/label action fault should use the executable xAsset bootstrap");
        expect(process.stdout_text.find("debug.command[1]: invoke:handlefault") != std::string::npos,
               prefix + "report/label action invocation should preserve its command identity");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               prefix + "report/label action fault should emit structured error status");
        expect(process.stdout_text.find("debug.reason: error") != std::string::npos,
               prefix + "report/label action fault should preserve the error pause state");
        expect(process.stdout_text.find("debug.watch.value: kept") != std::string::npos,
               prefix + "report/label action fault should preserve pre-fault watch state");
        expect(process.stdout_text.find("debug.reason: event_loop") != std::string::npos,
               prefix + "report/label action fault should return to the preview event loop after continue");
        expect(process.stdout_text.find("after_action = \"continued\"") != std::string::npos,
               prefix + "report/label action fault should execute post-fault code after continue");
        expect(process.stdout_text.find("terminate called") == std::string::npos,
               prefix + "report/label action fault should not terminate the runtime host");

        if (failures == 0) {
            fs::remove_all(temp_root, ignored);
        }
    }
}
