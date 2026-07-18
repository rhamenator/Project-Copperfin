// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_runtime_host_debug_output_support.h"

namespace {
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

    const std::string bootstrap = copperfin::runtime::build_xasset_bootstrap_source(model, true);
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

    const std::string bootstrap = copperfin::runtime::build_xasset_bootstrap_source(model, true);
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

    const std::string bootstrap = copperfin::runtime::build_xasset_bootstrap_source(model, true);
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

void test_runtime_host_writes_bridge_response_artifact(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_response_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path startup_path = temp_root / "content" / "startup.prg";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "nested" / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeResponse\n"
        "startup_item=startup.prg\n"
        "startup_source=") + startup_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(startup_path, "RETURN 7\n");
    write_text(
        source_path,
        "PROCEDURE AddNumbers\n"
        "RETURN 42\n"
        "ENDPROC\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + json_escape_string(source_path.string()) + "\",\n"
        "  \"source_line\": 7,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"\",\n"
        "  \"parameter_count\": 0,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": []\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "7",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "",
            "--parameter-count", "0",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "bridge-response stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-response stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0,
           "runtime host should accept wrapper-emitted bridge descriptor and response arguments");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should report bridge invocation mode");
    expect(process.stdout_text.find("bridge.library_export: AddNumbers") != std::string::npos,
           "runtime host should preserve bridge export metadata in diagnostics");
    expect(process.stdout_text.find("bridge.return_value: 42") != std::string::npos,
           "runtime host should report the PRG return value in bridge diagnostics");
    expect(process.stdout_text.find("bridge.routine_bootstrap: true") != std::string::npos,
           "runtime host bridge mode should invoke exported routines through a bootstrap");
    expect(fs::exists(response_path),
           "runtime host should write the requested bridge response artifact");

    const std::string response_document = read_text(response_path);
    expect(response_document.find("\"status\": \"ok\"") != std::string::npos,
           "runtime host bridge response should include ok status");
    expect(response_document.find("\"return_value\": \"42\"") != std::string::npos,
           "runtime host bridge response should include the evaluated PRG return value");
    expect(response_document.find("\"response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\"") != std::string::npos,
           "runtime host bridge response should echo the expected response media type");
    expect(response_document.find("\"schema_version\": \"v1\"") != std::string::npos,
           "runtime host bridge response should echo the requested schema version");
    expect(response_document.find("\"diagnostics\": \"bridge_response_written\"") != std::string::npos,
           "runtime host bridge response should include diagnostics");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_security_enabled_bridge_source_stays_inside_verified_package(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_secure_bridge_source";
    const fs::path content_root = temp_root / "content";
    const fs::path startup_path = content_root / "startup.prg";
    const fs::path source_path = content_root / "exports" / "exports.prg";
    const fs::path include_path = content_root / "shared" / "bridge_value.h";
    const fs::path outside_path = temp_root.parent_path() / "copperfin_external_bridge_source.prg";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path request_path = temp_root / "GetAnswer.request.json";
    const fs::path response_path = temp_root / "GetAnswer.response.json";
    const fs::path locale_root = temp_root / "locales";
    const fs::path deployed_runtime_host = deployed_runtime_host_path(temp_root, runtime_host_path);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::remove(outside_path, ignored);
    fs::create_directories(content_root);
    fs::create_directories(source_path.parent_path());
    fs::create_directories(include_path.parent_path());
    write_runtime_host_usage_catalogs(locale_root);
    write_text(startup_path, "RETURN\n");
    write_text(
        source_path,
        "#INCLUDE '../shared/BRIDGE_VALUE.H'\n"
        "PROCEDURE GetAnswer\n"
        "RETURN BRIDGE_VALUE\n"
        "ENDPROC\n");
    write_text(include_path, "#DEFINE BRIDGE_VALUE 42\n");
    write_text(outside_path, "PROCEDURE GetAnswer\nRETURN 99\nENDPROC\n");
    fs::copy_file(runtime_host_path, deployed_runtime_host, fs::copy_options::overwrite_existing);
#if defined(__unix__) || defined(__APPLE__)
    fs::permissions(
        deployed_runtime_host,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add,
        ignored);
#endif

    const auto runtime_host_hash =
        copperfin::security::sha256_hex_for_file(deployed_runtime_host.string());
    const auto startup_hash = copperfin::security::sha256_hex_for_file(startup_path.string());
    const auto source_hash = copperfin::security::sha256_hex_for_file(source_path.string());
    const auto include_hash = copperfin::security::sha256_hex_for_file(include_path.string());
    expect(runtime_host_hash.ok && startup_hash.ok && source_hash.ok && include_hash.ok,
           "secure bridge fixture should hash host, startup, export source, and include");
    if (!runtime_host_hash.ok || !startup_hash.ok || !source_hash.ok || !include_hash.ok) {
        fs::remove_all(temp_root, ignored);
        fs::remove(outside_path, ignored);
        return;
    }

    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=SecureBridgeSource\n"
        "package_root=" + temp_root.string() + "\n"
        "content_root=" + content_root.string() + "\n"
        "working_directory=" + content_root.string() + "\n"
        "startup_item=startup.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "security_enabled=true\n"
        "security_role=runtime-operator\n"
        "security_mode=native\n"
        "runtime_host_sha256=" + runtime_host_hash.hex_digest + "\n"
        "asset=1|startup.prg|" + startup_path.string() +
            "|Program|false|true|" + startup_hash.hex_digest + "|true\n"
        "asset=2|exports.prg|" + source_path.string() +
            "|Program|false|true|" + source_hash.hex_digest + "|true\n"
        "extension_payload=" + include_path.string() + "|" + include_hash.hex_digest + "\n"
        "dotnet_story=none\n");

    const auto write_request = [&](const fs::path& requested_source) {
        const std::string escaped_source_path = json_escape_string(requested_source.string());
        write_text(
            request_path,
            std::string("{\n"
            "  \"export_name\": \"GetAnswer\",\n"
            "  \"routine_kind\": \"procedure\",\n"
            "  \"source_path\": \"") + escaped_source_path + "\",\n"
            "  \"source_line\": 1,\n"
            "  \"parameter_declaration\": \"LPARAMETERS\",\n"
            "  \"parameter_names\": \"\",\n"
            "  \"parameter_count\": 0,\n"
            "  \"schema_version\": \"v1\",\n"
            "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
            "  \"parameters\": []\n"
            "}\n");
    };
    const auto invoke = [&](const fs::path& requested_source) {
        write_request(requested_source);
        fs::remove(response_path, ignored);
        return run_process_capture(
            deployed_runtime_host.string(),
            {
                "--manifest", manifest_path.string(),
                "--library-export", "GetAnswer",
                "--routine-kind", "procedure",
                "--source-path", requested_source.string(),
                "--source-line", "1",
                "--parameter-declaration", "LPARAMETERS",
                "--parameter-names", "",
                "--parameter-count", "0",
                "--request-path", request_path.string(),
                "--response-path", response_path.string(),
                "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
                "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
                "--schema-version", "v1"
            },
            temp_root);
    };

    ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
    const auto packaged_process = invoke(source_path);
    expect(packaged_process.exit_code == 0,
           "security-enabled bridge invocation should execute its verified packaged source bytes");
    expect(packaged_process.stdout_text.find("bridge.return_value: 42") != std::string::npos,
           "security-enabled bridge invocation should resolve verified includes case-insensitively from memory");

    const auto external_process = invoke(outside_path);
    expect(external_process.exit_code == 4,
           "security-enabled bridge invocation should reject an external source path");
    expect(external_process.stdout_text.find(
               "error: Bridge routine source is missing from the package: copperfin_external_bridge_source.prg") !=
               std::string::npos,
           "external bridge-source rejection should use the localized package-boundary diagnostic");
    expect(external_process.stdout_text.find("bridge.return_value: 99") == std::string::npos,
           "security-enabled bridge invocation must not execute external source content");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
        fs::remove(outside_path, ignored);
    }
}

void test_runtime_host_invokes_zero_argument_bridge_export(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_zero_arg_export_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path startup_path = temp_root / "content" / "startup.prg";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "GetAnswer.request.json";
    const fs::path response_path = temp_root / "nested" / "GetAnswer.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeZeroArgExport\n"
        "startup_item=startup.prg\n"
        "startup_source=") + startup_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(startup_path, "RETURN 7\n");
    write_text(
        source_path,
        "PROCEDURE GetAnswer\n"
        "RETURN 42\n"
        "ENDPROC\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"GetAnswer\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + json_escape_string(source_path.string()) + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"\",\n"
        "  \"parameter_count\": 0,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": []\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "GetAnswer",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "",
            "--parameter-count", "0",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "bridge-zero-arg-export stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-zero-arg-export stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0,
           "runtime host should invoke zero-argument bridge exports through a bootstrap PRG");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should report bridge invocation mode for zero-argument exports");
    expect(process.stdout_text.find("bridge.library_export: GetAnswer") != std::string::npos,
           "runtime host should preserve zero-argument export metadata in diagnostics");
    expect(process.stdout_text.find("bridge.routine_bootstrap: true") != std::string::npos,
           "runtime host should report routine bootstrap execution for zero-argument exports");
    expect(process.stdout_text.find("bridge.return_value: 42") != std::string::npos,
           "runtime host should report the zero-argument export return value in diagnostics");
    expect(fs::exists(response_path),
           "runtime host should write the bridge response for zero-argument exports");

    const std::string response_document = read_text(response_path);
    expect(response_document.find("\"status\": \"ok\"") != std::string::npos,
           "zero-argument bridge export response should include ok status");
    expect(response_document.find("\"return_value\": \"42\"") != std::string::npos,
           "zero-argument bridge export response should include the exported routine return value");
    expect(response_document.find("\"schema_version\": \"v1\"") != std::string::npos,
           "zero-argument bridge export response should echo the requested schema version");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_removes_bridge_routine_bootstrap_after_execution(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_bootstrap_cleanup_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path startup_path = temp_root / "content" / "startup.prg";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "GetAnswer.request.json";
    const fs::path response_path = temp_root / "nested" / "GetAnswer.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeBootstrapCleanup\n"
        "startup_item=startup.prg\n"
        "startup_source=") + startup_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(startup_path, "RETURN 7\n");
    write_text(
        source_path,
        "PROCEDURE GetAnswer\n"
        "RETURN 42\n"
        "ENDPROC\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"GetAnswer\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + json_escape_string(source_path.string()) + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"\",\n"
        "  \"parameter_count\": 0,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": []\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "GetAnswer",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "",
            "--parameter-count", "0",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "bridge-bootstrap-cleanup stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-bootstrap-cleanup stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0,
           "runtime host should complete bridge routine invocation before bootstrap cleanup assertion");
    expect(process.stdout_text.find("bridge.routine_bootstrap: true") != std::string::npos,
           "runtime host should report routine bootstrap execution for cleanup coverage");
    const std::string execution_source = output_line_value(process.stdout_text, "bridge.execution_source: ");
    expect(!execution_source.empty(),
           "runtime host should report the materialized bootstrap execution source");
    if (!execution_source.empty()) {
        expect(execution_source.find("copperfin_bridge_GetAnswer_") != std::string::npos,
               "runtime host should report the generated bridge bootstrap path");
        expect(!fs::exists(execution_source),
               "runtime host should remove the generated bridge bootstrap after execution");
    }
    expect(fs::exists(response_path),
           "runtime host should still write the bridge response after bootstrap cleanup");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_unescapes_bridge_descriptor_string_fields(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_escaped_descriptor_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path startup_path = temp_root / "content" / "startup.prg";
    const fs::path source_path = temp_root / "content" / "exports\\escaped.prg";
    const fs::path request_path = temp_root / "GetAnswer.request.json";
    const fs::path response_path = temp_root / "nested" / "GetAnswer.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeEscapedDescriptor\n"
        "startup_item=startup.prg\n"
        "startup_source=") + startup_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(startup_path, "RETURN 7\n");
    write_text(
        source_path,
        "PROCEDURE GetAnswer\n"
        "RETURN 42\n"
        "ENDPROC\n");

    const std::string escaped_source_path = json_escape_string(source_path.string());
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"GetAnswer\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + escaped_source_path + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"\",\n"
        "  \"parameter_count\": 0,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": []\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "GetAnswer",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "",
            "--parameter-count", "0",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "bridge-escaped-descriptor stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-escaped-descriptor stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0,
           "runtime host should decode escaped descriptor strings before bridge validation");
    expect(process.stdout_text.find("bridge.routine_bootstrap: true") != std::string::npos,
           "runtime host should reach routine bootstrap execution after escaped descriptor validation");
    expect(process.stdout_text.find("bridge.return_value: 42") != std::string::npos,
           "runtime host should execute the escaped-path descriptor source");
    const std::string response_document = read_text(response_path);
    expect(response_document.find("\"return_value\": \"42\"") != std::string::npos,
           "escaped descriptor bridge response should include the exported routine return value");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_passes_bridge_request_parameters_to_export(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_parameter_export_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path startup_path = temp_root / "content" / "startup.prg";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "nested" / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeParameterExport\n"
        "startup_item=startup.prg\n"
        "startup_source=") + startup_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(startup_path, "RETURN 7\n");
    write_text(
        source_path,
        "PROCEDURE AddNumbers\n"
        "LPARAMETERS tnLeft, tnRight\n"
        "RETURN tnLeft + tnRight\n"
        "ENDPROC\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + json_escape_string(source_path.string()) + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"tnLeft|tnRight\",\n"
        "  \"parameter_count\": 2,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": [\n"
        "    {\"name\": \"tnLeft\", \"value\": \"40\", \"surface\": \"dll-int\"},\n"
        "    {\"name\": \"tnRight\", \"value\": \"2\", \"surface\": \"dll-int\"}\n"
        "  ]\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "tnLeft|tnRight",
            "--parameter-count", "2",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "bridge-parameter-export stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-parameter-export stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0,
           "runtime host should pass bridge request parameter values to exported routines");
    expect(process.stdout_text.find("bridge.routine_bootstrap: true") != std::string::npos,
           "runtime host should report routine bootstrap execution for parameterized exports");
    expect(process.stdout_text.find("bridge.parameter_count: 2") != std::string::npos,
           "runtime host should preserve the parameter count in diagnostics");
    expect(process.stdout_text.find("bridge.return_value: 42") != std::string::npos,
           "runtime host should report the parameterized export return value in diagnostics");
    expect(fs::exists(response_path),
           "runtime host should write the bridge response for parameterized exports");

    const std::string response_document = read_text(response_path);
    expect(response_document.find("\"status\": \"ok\"") != std::string::npos,
           "parameterized bridge export response should include ok status");
    expect(response_document.find("\"return_value\": \"42\"") != std::string::npos,
           "parameterized bridge export response should include the exported routine return value");
    expect(response_document.find("\"response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\"") != std::string::npos,
           "parameterized bridge export response should echo the expected response media type");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_bridge_parameter_count_mismatch(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_parameter_mismatch_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "nested" / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeParameterMismatch\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(
        source_path,
        "PROCEDURE AddNumbers\n"
        "LPARAMETERS tnLeft, tnRight\n"
        "RETURN tnLeft + tnRight\n"
        "ENDPROC\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + json_escape_string(source_path.string()) + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"tnLeft|tnRight|tnExtra\",\n"
        "  \"parameter_count\": 3,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": [\n"
        "    {\"name\": \"tnLeft\", \"value\": \"40\", \"surface\": \"dll-int\"},\n"
        "    {\"name\": \"tnRight\", \"value\": \"2\", \"surface\": \"dll-int\"}\n"
        "  ]\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "tnLeft|tnRight|tnExtra",
            "--parameter-count", "3",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-parameter-mismatch stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-parameter-mismatch stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject bridge parameter count mismatches before execution");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on parameter count mismatches");
    expect(process.stdout_text.find("error: Bridge request parameter count mismatch.") != std::string::npos,
           "runtime host should report bridge parameter count mismatches");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when bridge parameter counts mismatch");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_nested_bridge_parameter_array_for_nonzero_arity(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_nested_parameter_array_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeNestedParameters\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(
        source_path,
        "PROCEDURE AddNumbers\n"
        "LPARAMETERS tnLeft, tnRight\n"
        "RETURN tnLeft + tnRight\n"
        "ENDPROC\n"
        "RETURN 7\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + json_escape_string(source_path.string()) + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"tnLeft|tnRight\",\n"
        "  \"parameter_count\": 2,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameter_shadow\": {\n"
        "    \"parameters\": [\n"
        "      {\"name\": \"tnLeft\", \"value\": \"40\", \"surface\": \"int\"},\n"
        "      {\"name\": \"tnRight\", \"value\": \"2\", \"surface\": \"int\"}\n"
        "    ]\n"
        "  }\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "tnLeft|tnRight",
            "--parameter-count", "2",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-nested-parameter-array stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-nested-parameter-array stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject nested bridge parameter arrays for nonzero arity");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on nested parameter-array errors");
    expect(process.stdout_text.find("error: Bridge request parameter count mismatch.") != std::string::npos,
           "runtime host should report a parameter count mismatch when no top-level parameter payload exists");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when nonzero bridge parameters are nested");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_nested_bridge_parameter_values_for_nonzero_arity(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_nested_parameter_value_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeNestedParameterValues\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(
        source_path,
        "PROCEDURE AddNumbers\n"
        "LPARAMETERS tnLeft, tnRight\n"
        "RETURN tnLeft + tnRight\n"
        "ENDPROC\n"
        "RETURN 7\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + json_escape_string(source_path.string()) + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"tnLeft|tnRight\",\n"
        "  \"parameter_count\": 2,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": [\n"
        "    {\"name\": \"tnLeft\", \"value_shadow\": {\"value\": \"40\"}, \"surface\": \"int\"},\n"
        "    {\"name\": \"tnRight\", \"value_shadow\": {\"value\": \"2\"}, \"surface\": \"int\"}\n"
        "  ]\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "tnLeft|tnRight",
            "--parameter-count", "2",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-nested-parameter-values stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-nested-parameter-values stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject nested bridge parameter values for nonzero arity");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on nested parameter-value errors");
    expect(process.stdout_text.find("error: Bridge request parameter count mismatch.") != std::string::npos,
           "runtime host should report a parameter count mismatch when parameter values are nested");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when nonzero bridge parameter values are nested");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_bridge_parameter_name_mismatch(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_parameter_name_mismatch_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "nested" / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeParameterNameMismatch\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(
        source_path,
        "PROCEDURE AddNumbers\n"
        "LPARAMETERS tnLeft, tnRight\n"
        "RETURN tnLeft + tnRight\n"
        "ENDPROC\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + json_escape_string(source_path.string()) + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"tnLeft|tnRight\",\n"
        "  \"parameter_count\": 2,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": [\n"
        "    {\"name\": \"tnRight\", \"value\": \"40\", \"surface\": \"dll-int\"},\n"
        "    {\"name\": \"tnLeft\", \"value\": \"2\", \"surface\": \"dll-int\"}\n"
        "  ]\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "tnLeft|tnRight",
            "--parameter-count", "2",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-parameter-name-mismatch stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-parameter-name-mismatch stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject bridge parameter name mismatches before execution");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on parameter name mismatches");
    expect(process.stdout_text.find("error: Bridge request parameter name mismatch.") != std::string::npos,
           "runtime host should report bridge parameter name mismatches");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when bridge parameter names mismatch");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_bridge_request_contract_mismatch(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_request_contract_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeRequestContract\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(source_path, "RETURN 42\n");
    write_text(
        request_path,
        "{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.bad-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": []\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "left,right",
            "--parameter-count", "2",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-request-contract stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-request-contract stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject bridge request media-type mismatches before execution");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on request contract errors");
    expect(process.stdout_text.find("error: Bridge request media type mismatch.") != std::string::npos,
           "runtime host should report the request media-type mismatch");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when the request contract mismatches");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_nested_bridge_descriptor_fields(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_nested_descriptor_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeNestedDescriptor\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(source_path, "RETURN 42\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"descriptor_shadow\": {\n"
        "    \"export_name\": \"AddNumbers\",\n"
        "    \"routine_kind\": \"procedure\",\n"
        "    \"source_path\": \"") + json_escape_string(source_path.string()) + "\",\n"
        "    \"parameter_count\": 0,\n"
        "    \"schema_version\": \"v1\",\n"
        "    \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\"\n"
        "  },\n"
        "  \"parameters\": []\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "",
            "--parameter-count", "0",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-nested-descriptor stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-nested-descriptor stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject nested bridge descriptor fields before execution");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on nested descriptor errors");
    expect(process.stdout_text.find("error: Bridge request media type mismatch.") != std::string::npos,
           "runtime host should not accept nested request-media fields as top-level contract fields");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when bridge descriptor fields are nested");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_bridge_descriptor_identity_mismatch(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_descriptor_contract_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeDescriptorContract\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(source_path, "RETURN 42\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"WrongExport\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + json_escape_string(source_path.string()) + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"left,right\",\n"
        "  \"parameter_count\": 2,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": []\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "left,right",
            "--parameter-count", "2",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-descriptor-contract stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-descriptor-contract stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject bridge descriptor identity mismatches before execution");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on descriptor contract errors");
    expect(process.stdout_text.find("error: Bridge request descriptor mismatch.") != std::string::npos,
           "runtime host should report descriptor identity mismatches");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when descriptor identity mismatches");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_bridge_descriptor_metadata_mismatch(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_descriptor_metadata_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeDescriptorMetadata\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(source_path, "RETURN 42\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + json_escape_string(source_path.string()) + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"PARAMETERS\",\n"
        "  \"parameter_names\": \"tnLeft|tnRight\",\n"
        "  \"parameter_count\": 2,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": [\n"
        "    {\"name\": \"tnLeft\", \"value\": \"40\", \"surface\": \"int\"},\n"
        "    {\"name\": \"tnRight\", \"value\": \"2\", \"surface\": \"int\"}\n"
        "  ]\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "tnLeft|tnRight",
            "--parameter-count", "2",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-descriptor-metadata stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-descriptor-metadata stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject bridge descriptor metadata mismatches before execution");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on descriptor metadata errors");
    expect(process.stdout_text.find("error: Bridge request descriptor mismatch.") != std::string::npos,
           "runtime host should report descriptor metadata mismatches");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when descriptor metadata mismatches");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_usage_text_localizes_without_changing_cli_tokens(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_usage_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    std::cerr << "USAGE: fixture root ready\n";
#if defined(_WIN32)
    const fs::path locale_root = temp_root / fs::path(L"locales_\u0416_\u6F22");
#else
    const fs::path locale_root = temp_root / "locales_\xD0\x96_\xE6\xBC\xA2";
#endif
    write_runtime_host_usage_catalogs(locale_root);
    std::cerr << "USAGE: catalogs ready\n";

    {
        std::cerr << "USAGE: BEGIN en-US\n";
        ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
        const auto process = run_process_capture(runtime_host_path, {}, temp_root);
        expect(process.exit_code == 2,
               "#2349: runtime host without manifest should keep the usage exit code");
        expect(process.stdout_text.find("Usage: copperfin_runtime_host --manifest <path> [--debug]") != std::string::npos,
               "#2349: runtime host en-US usage should remain stable");
        expect(process.stdout_text.find("--federation-backend <sqlite|postgresql|sqlserver|oracle>") != std::string::npos,
               "#2349: runtime host en-US usage should preserve federation CLI tokens");

        const auto invalid_federation_bool = run_process_capture(
            runtime_host_path,
            {
                "--federation-backend", "sqlite",
                "--federation-query", "SELECT * FROM customer",
                "--federation-planning-enable", "maybe"
            },
            temp_root);
        expect(invalid_federation_bool.exit_code == 2,
               "#3791: runtime host should reject invalid federation planning booleans");
        expect(invalid_federation_bool.stdout_text.find("status: error") != std::string::npos,
               "#3791: invalid federation planning booleans should preserve machine-readable status");
        expect(invalid_federation_bool.stdout_text.find(
                   "error: The --federation-planning-enable value must be true or false.") != std::string::npos,
               "#3791: invalid federation planning booleans should localize the en-US parse error");
        expect(invalid_federation_bool.stdout_text.find("--federation-planning-enable") != std::string::npos &&
                   invalid_federation_bool.stdout_text.find("<true|false>") != std::string::npos,
               "#3791: invalid federation planning booleans should preserve invariant CLI tokens in usage output");
        std::cerr << "USAGE: END en-US\n";
    }

    {
        std::cerr << "USAGE: BEGIN es-419\n";
        ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(runtime_host_path, {}, temp_root);
        expect(process.exit_code == 2,
               "#2585: es-419 runtime host usage should keep the usage exit code");
        expect(process.stdout_text.find("Uso: copperfin_runtime_host --manifest <path> [--debug]") != std::string::npos,
               "#2585: es-419 runtime host usage should localize manifest usage prose");
        expect(process.stdout_text.find("   o: copperfin_runtime_host") != std::string::npos &&
                   process.stdout_text.find("--federation-backend") != std::string::npos &&
                   process.stdout_text.find("--federation-query") != std::string::npos,
               "#2585: es-419 runtime host usage should localize alternate usage prose while preserving CLI tokens");
        expect(process.stdout_text.find("Usage: copperfin_runtime_host --manifest <path> [--debug]") == std::string::npos,
               "#2585: es-419 runtime host usage should not fall back to raw English prose");

        const auto slash_locale_process = run_process_capture(runtime_host_path, {"/locale", "es-419"}, temp_root);
        expect(slash_locale_process.exit_code == 2,
               "#3752: /locale should keep the normal usage exit code when no manifest is available");
        expect(slash_locale_process.stdout_text.find("Uso: copperfin_runtime_host --manifest <path> [--debug]") != std::string::npos,
               "#3752: /locale should select the same localized catalog as --locale");
        expect(slash_locale_process.stdout_text.find("Usage: copperfin_runtime_host --manifest <path> [--debug]") == std::string::npos,
               "#3752: /locale should not fall back to raw English prose");

        const auto invalid_federation_bool = run_process_capture(
            runtime_host_path,
            {
                "--federation-backend", "sqlite",
                "--federation-query", "SELECT * FROM customer",
                "--federation-planning-require", "quizas"
            },
            temp_root);
        expect(invalid_federation_bool.exit_code == 2,
               "#3791: es-419 invalid federation planning booleans should keep the usage exit code");
        expect(invalid_federation_bool.stdout_text.find(
                   "error: El valor de --federation-planning-require debe ser true o false.") != std::string::npos,
               "#3791: es-419 invalid federation planning booleans should localize parse errors while preserving option tokens");
        expect(invalid_federation_bool.stdout_text.find(
                   "error: The --federation-planning-require value must be true or false.") == std::string::npos,
               "#3791: es-419 invalid federation planning booleans should not fall back to raw English prose");
        std::cerr << "USAGE: END es-419\n";
    }

    {
        std::cerr << "USAGE: BEGIN qps-ploc\n";
        ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        const auto process = run_process_capture(runtime_host_path, {}, temp_root);
        expect(process.exit_code == 2,
               "#2349: pseudo-localized runtime host usage should keep the usage exit code");
        expect(process.stdout_text.find("[!! ") != std::string::npos,
               "#2349: pseudo-localized runtime host usage should decorate prose");
        expect(process.stdout_text.find("copperfin_runtime_host") != std::string::npos &&
                   process.stdout_text.find("--manifest") != std::string::npos &&
                   process.stdout_text.find("--debug-command") != std::string::npos &&
                   process.stdout_text.find("<continue|step|next|out|watch:<expr>|select:<action-id>|invoke:<action-id>|break:add:<file:line>|break:remove:<file:line>|break:add-action:<action-id>|break:remove-action:<action-id>|break:clear|break:list>") != std::string::npos,
               "#2349: pseudo-localized runtime host usage should preserve CLI and debug-command tokens");

        const auto slash_debug = run_process_capture(runtime_host_path, {"/debug"}, temp_root);
        expect(slash_debug.exit_code == 2,
               "#3752: /debug should keep the normal usage exit code when no manifest is available");
        expect(slash_debug.stdout_text.find("status: error") == std::string::npos,
               "#3752: /debug should be accepted as a host alias instead of surfacing an unknown-argument contract");
        expect(slash_debug.stdout_text.find("[!! ") != std::string::npos,
               "#3752: /debug acceptance should still honor the selected pseudo-localized catalog");
        expect(slash_debug.stdout_text.find("--debug-command") != std::string::npos,
               "#3752: /debug acceptance should preserve ordinary usage/debug token output");

        const auto unknown_argument = run_process_capture(runtime_host_path, {"--unknown-option"}, temp_root);
        expect(unknown_argument.exit_code == 2,
               "#2351: pseudo-localized runtime host unknown arguments should keep the usage exit code");
        expect(unknown_argument.stdout_text.find("status: error") != std::string::npos,
               "#2351: pseudo-localized runtime host errors should preserve machine-readable status");
        expect(unknown_argument.stdout_text.find("[!! ") != std::string::npos,
               "#2351: pseudo-localized runtime host unknown arguments should decorate prose");
        expect(unknown_argument.stdout_text.find("--unknown-option") != std::string::npos,
               "#2351: pseudo-localized runtime host unknown arguments should preserve CLI tokens");

        const auto missing_federation_argument = run_process_capture(
            runtime_host_path,
            {"--federation-backend", "sqlite"},
            temp_root);
        expect(missing_federation_argument.exit_code == 2,
               "#2351: pseudo-localized federation validation should keep the usage exit code");
        expect(missing_federation_argument.stdout_text.find("status: error") != std::string::npos,
               "#2351: pseudo-localized federation validation should preserve machine-readable status");
        expect(missing_federation_argument.stdout_text.find("[!! ") != std::string::npos,
               "#2351: pseudo-localized federation validation should decorate prose");
        expect(missing_federation_argument.stdout_text.find("--federation-backend") != std::string::npos &&
                   missing_federation_argument.stdout_text.find("--federation-query") != std::string::npos,
               "#2351: pseudo-localized federation validation should preserve CLI tokens");

        const auto invalid_federation_bool = run_process_capture(
            runtime_host_path,
            {
                "--federation-backend", "sqlite",
                "--federation-query", "SELECT * FROM customer",
                "--federation-planning-audit", "maybe"
            },
            temp_root);
        expect(invalid_federation_bool.exit_code == 2,
               "#3791: pseudo-localized invalid federation planning booleans should keep the usage exit code");
        expect(invalid_federation_bool.stdout_text.find("status: error") != std::string::npos,
               "#3791: pseudo-localized invalid federation planning booleans should preserve machine-readable status");
        expect(invalid_federation_bool.stdout_text.find("[!! ") != std::string::npos,
               "#3791: pseudo-localized invalid federation planning booleans should decorate prose");
        expect(invalid_federation_bool.stdout_text.find("--federation-planning-audit") != std::string::npos &&
                   invalid_federation_bool.stdout_text.find("true") != std::string::npos &&
                   invalid_federation_bool.stdout_text.find("false") != std::string::npos,
               "#3791: pseudo-localized invalid federation planning booleans should preserve invariant boolean tokens");

        const fs::path bridge_manifest_path = temp_root / "bridge.cfmanifest";
        const fs::path bridge_source_path = temp_root / "bridge.prg";
        const fs::path bridge_response_path = temp_root / "bridge.response.json";
        write_text(
            bridge_manifest_path,
            std::string("manifest_version=1\n"
            "project_title=BridgeLocalization\n"
            "startup_item=bridge.prg\n"
            "startup_source=") + bridge_source_path.string() + "\n"
            "security_enabled=false\n"
            "dotnet_story=none\n");
        write_text(bridge_source_path, "RETURN 1\n");

        const auto bridge_error = run_process_capture(
            runtime_host_path,
            {
                "--manifest", bridge_manifest_path.string(),
                "--request-path", (temp_root / "missing.request.json").string(),
                "--response-path", bridge_response_path.string(),
                "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
                "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
                "--schema-version", "v1"
            },
            temp_root);
        expect(bridge_error.exit_code == 6,
               "#2352: pseudo-localized bridge errors should keep the bridge validation exit code");
        expect(bridge_error.stdout_text.find("status: error") != std::string::npos,
               "#2352: pseudo-localized bridge errors should preserve machine-readable status");
        expect(bridge_error.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
               "#2352: pseudo-localized bridge errors should preserve bridge runtime mode");
        expect(bridge_error.stdout_text.find("[!! ") != std::string::npos,
               "#2352: pseudo-localized bridge errors should decorate prose");
        std::cerr << "USAGE: END qps-ploc\n";
    }

    {
        std::cerr << "USAGE: BEGIN es-419 bridge\n";
        ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");

        const fs::path bridge_manifest_path = temp_root / "bridge_es.cfmanifest";
        const fs::path bridge_source_path = temp_root / "bridge_es.prg";
        const fs::path bridge_response_path = temp_root / "bridge_es.response.json";
        write_text(
            bridge_manifest_path,
            std::string("manifest_version=1\n"
            "project_title=BridgeLocalizationSpanish\n"
            "startup_item=bridge_es.prg\n"
            "startup_source=") + bridge_source_path.string() + "\n"
            "security_enabled=false\n"
            "dotnet_story=none\n");
        write_text(bridge_source_path, "RETURN 1\n");

        const auto bridge_error = run_process_capture(
            runtime_host_path,
            {
                "--manifest", bridge_manifest_path.string(),
                "--request-path", (temp_root / "missing.request.es.json").string(),
                "--response-path", bridge_response_path.string(),
                "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
                "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
                "--schema-version", "v1"
            },
            temp_root);
        expect(bridge_error.exit_code == 6,
               "#2587: es-419 bridge request-artifact errors should keep the bridge validation exit code");
        expect(bridge_error.stdout_text.find("status: error") != std::string::npos,
               "#2587: es-419 bridge request-artifact errors should preserve machine-readable status");
        expect(bridge_error.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
               "#2587: es-419 bridge request-artifact errors should preserve bridge runtime mode");
        expect(bridge_error.stdout_text.find("error: No se encontro el artefacto de solicitud bridge.") != std::string::npos,
               "#2587: es-419 bridge request-artifact errors should localize prose");
        expect(bridge_error.stdout_text.find("error: Bridge request artifact not found.") == std::string::npos,
               "#2587: es-419 bridge request-artifact errors should not fall back to raw English prose");
        std::cerr << "USAGE: END es-419 bridge\n";
    }

    {
        std::cerr << "USAGE: BEGIN pt-BR\n";
        ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");

        const auto unknown_argument = run_process_capture(runtime_host_path, {"--unknown-option"}, temp_root);
        expect(unknown_argument.exit_code == 2,
               "#2585: pt-BR runtime host unknown arguments should keep the usage exit code");
        expect(unknown_argument.stdout_text.find("status: error") != std::string::npos,
               "#2585: pt-BR runtime host unknown arguments should preserve machine-readable status");
        expect(unknown_argument.stdout_text.find("erro: Argumento desconhecido: --unknown-option") != std::string::npos,
               "#2585: pt-BR runtime host unknown arguments should localize prefixed error prose");
        expect(unknown_argument.stdout_text.find("error: Unknown argument: --unknown-option") == std::string::npos,
               "#2585: pt-BR runtime host unknown arguments should not fall back to raw English prose");

        const auto missing_federation_argument = run_process_capture(
            runtime_host_path,
            {"--federation-backend", "sqlite"},
            temp_root);
        expect(missing_federation_argument.exit_code == 2,
               "#2585: pt-BR federation validation should keep the usage exit code");
        expect(missing_federation_argument.stdout_text.find("erro: --federation-backend e --federation-query sao obrigatorios no modo de federacao.") != std::string::npos,
               "#2585: pt-BR federation validation should localize required-option prose");
        expect(missing_federation_argument.stdout_text.find("--federation-backend") != std::string::npos &&
                   missing_federation_argument.stdout_text.find("--federation-query") != std::string::npos,
               "#2585: pt-BR federation validation should preserve CLI tokens");
        expect(missing_federation_argument.stdout_text.find("error: --federation-backend and --federation-query are both required in federation mode.") == std::string::npos,
               "#2585: pt-BR federation validation should not fall back to raw English prose");

        const fs::path bridge_manifest_path = temp_root / "bridge_pt.cfmanifest";
        const fs::path bridge_source_path = temp_root / "bridge_pt.prg";
        const fs::path bridge_response_path = temp_root / "bridge_pt.response.json";
        write_text(
            bridge_manifest_path,
            std::string("manifest_version=1\n"
            "project_title=BridgeLocalizationPortuguese\n"
            "startup_item=bridge_pt.prg\n"
            "startup_source=") + bridge_source_path.string() + "\n"
            "security_enabled=false\n"
            "dotnet_story=none\n");
        write_text(bridge_source_path, "RETURN 1\n");

        const auto bridge_error = run_process_capture(
            runtime_host_path,
            {
                "--manifest", bridge_manifest_path.string(),
                "--request-path", (temp_root / "missing.request.pt.json").string(),
                "--response-path", bridge_response_path.string(),
                "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
                "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
                "--schema-version", "v1"
            },
            temp_root);
        expect(bridge_error.exit_code == 6,
               "#2587: pt-BR bridge request-artifact errors should keep the bridge validation exit code");
        expect(bridge_error.stdout_text.find("status: error") != std::string::npos,
               "#2587: pt-BR bridge request-artifact errors should preserve machine-readable status");
        expect(bridge_error.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
               "#2587: pt-BR bridge request-artifact errors should preserve bridge runtime mode");
        expect(bridge_error.stdout_text.find("erro: Artefato de solicitacao bridge nao encontrado.") != std::string::npos,
               "#2587: pt-BR bridge request-artifact errors should localize prose");
        expect(bridge_error.stdout_text.find("error: Bridge request artifact not found.") == std::string::npos,
               "#2587: pt-BR bridge request-artifact errors should not fall back to raw English prose");
        std::cerr << "USAGE: END pt-BR\n";
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_host_debug_errors_localize_without_changing_command_tokens(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_debug_error_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);

    const fs::path startup_path = temp_root / "main.prg";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_text(
        startup_path,
        "LOCAL nValue\n"
        "nValue = 1\n"
        "RETURN\n");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=DebugErrorLocalization\n"
        "startup_item=main.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "break:add:not-a-breakpoint"
            },
            temp_root);
        expect(process.exit_code == 5,
               "#2391: en-US invalid breakpoint diagnostics should keep the debug error exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2391: en-US invalid breakpoint diagnostics should preserve machine-readable status");
        expect(
            process.stdout_text.find("error: Invalid breakpoint command: break:add:not-a-breakpoint") !=
                std::string::npos,
            "#2391: en-US invalid breakpoint diagnostics should remain stable");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "break:remove:2"
            },
            temp_root);
        expect(process.exit_code == 5,
               "#2586: es-419 unknown breakpoint diagnostics should keep the debug error exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2586: es-419 unknown breakpoint diagnostics should preserve machine-readable status");
        expect(
            process.stdout_text.find("Breakpoint desconocido: " + startup_path.string() + ":2") != std::string::npos,
            "#2586: es-419 unknown breakpoint diagnostics should localize the error body while preserving path and line");
        expect(process.stdout_text.find("Unknown breakpoint: " + startup_path.string() + ":2") == std::string::npos,
               "#2586: es-419 unknown breakpoint diagnostics should not fall back to the raw English error");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "break:add:not-a-breakpoint"
            },
            temp_root);
        expect(process.exit_code == 5,
               "#2566: pt-BR invalid breakpoint diagnostics should keep the debug error exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2566: pt-BR invalid breakpoint diagnostics should preserve machine-readable status");
        expect(process.stdout_text.find("erro: ") != std::string::npos,
               "#2566: pt-BR invalid breakpoint diagnostics should localize the error prefix");
        expect(process.stdout_text.find("Comando de breakpoint invalido: break:add:not-a-breakpoint") != std::string::npos,
               "#2566: pt-BR invalid breakpoint diagnostics should localize the error body");
        expect(process.stdout_text.find("error: Invalid breakpoint command") == std::string::npos,
               "#2566: pt-BR invalid breakpoint diagnostics should not fall back to the raw English prefixed error");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "watch:nValue"
            },
            temp_root);
        expect(process.exit_code == 5,
               "#2586: pt-BR watch diagnostics should keep the debug error exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2586: pt-BR watch diagnostics should preserve machine-readable status");
        expect(process.stdout_text.find("erro: ") != std::string::npos,
               "#2586: pt-BR watch diagnostics should localize the error prefix");
        expect(process.stdout_text.find("A avaliacao de watch requer um estado pausado ativo.") != std::string::npos,
               "#2586: pt-BR watch diagnostics should localize the paused-state error");
        expect(process.stdout_text.find("Watch evaluation requires an active paused state.") == std::string::npos,
               "#2586: pt-BR watch diagnostics should not fall back to the raw English error");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "break:add:not-a-breakpoint"
            },
            temp_root);
        expect(process.exit_code == 5,
               "#2391: pseudo-localized invalid breakpoint diagnostics should keep the debug error exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2391: pseudo-localized invalid breakpoint diagnostics should preserve machine-readable status");
        const std::string pseudo_error_prefix =
            copperfin::localization::load_catalogs(locale_root, "qps-ploc").translate("RuntimeHost.Prefix.Error");
        expect(process.stdout_text.find(pseudo_error_prefix) != std::string::npos,
               "#2566: pseudo-localized invalid breakpoint diagnostics should route the error prefix through qps-ploc");
        expect(process.stdout_text.find("[!! ") != std::string::npos,
               "#2391: pseudo-localized invalid breakpoint diagnostics should decorate prose");
        expect(process.stdout_text.find("break:add:not-a-breakpoint") != std::string::npos,
               "#2391: pseudo-localized invalid breakpoint diagnostics should preserve debug command tokens");
        expect(process.stdout_text.find("error: Invalid breakpoint command: break:add:not-a-breakpoint") == std::string::npos,
               "#2566: pseudo-localized invalid breakpoint diagnostics should not fall back to the raw English prefixed error");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_host_pause_messages_localize_without_changing_pause_reasons(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_pause_message_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);

    const fs::path startup_path = temp_root / "main.prg";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=PauseMessageLocalization\n"
        "startup_item=main.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    {
        write_text(startup_path, "RETURN\n");
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "continue"
            },
            temp_root);
        expect(process.exit_code == 0,
               "#2589: en-US completed pause messages should keep the runtime-host success exit code");
        expect(process.stdout_text.find("status: ok") != std::string::npos,
               "#2589: en-US completed pause messages should preserve machine-readable ok status");
        expect(process.stdout_text.find("debug.reason: completed") != std::string::npos,
               "#2589: en-US completed pause messages should preserve the completed pause reason");
        expect(process.stdout_text.find("debug.message: Execution completed.") != std::string::npos,
               "#2589: en-US completed pause messages should remain stable");
    }

    {
        write_text(
            startup_path,
            "LOCAL nValue\n"
            "nValue = 1\n"
            "RETURN\n");
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "step"
            },
            temp_root);
        expect(process.exit_code == 0,
               "#2589: es-419 step pause messages should keep the runtime-host success exit code");
        expect(process.stdout_text.find("debug.reason: step") != std::string::npos,
               "#2589: es-419 step pause messages should preserve the step pause reason");
        expect(process.stdout_text.find("debug.message: El paso se completo.") != std::string::npos,
               "#2589: es-419 step pause messages should localize the step-completed prose");
        expect(process.stdout_text.find("debug.message: Step completed.") == std::string::npos,
               "#2589: es-419 step pause messages should not fall back to the raw English step-completed prose");
    }

    {
        write_text(
            startup_path,
            "READ EVENTS\n"
            "RETURN\n");
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "continue"
            },
            temp_root);
        expect(process.exit_code == 0,
               "#2589: pt-BR READ EVENTS pause messages should keep the runtime-host success exit code");
        expect(process.stdout_text.find("debug.reason: event_loop") != std::string::npos,
               "#2589: pt-BR READ EVENTS pause messages should preserve the event-loop pause reason");
        expect(process.stdout_text.find("debug.message: O runtime esta aguardando em READ EVENTS.") !=
                   std::string::npos,
               "#2589: pt-BR READ EVENTS pause messages should localize prose while preserving the READ EVENTS token");
        expect(process.stdout_text.find("The runtime is waiting in READ EVENTS.") == std::string::npos,
               "#2589: pt-BR READ EVENTS pause messages should not fall back to the raw English prose");
    }

    {
        write_text(
            startup_path,
            "LOCAL nValue\n"
            "nValue = 1\n"
            "RETURN\n");
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--breakpoint", startup_path.string() + ":2",
                "--debug-command", "continue"
            },
            temp_root);
        expect(process.exit_code == 0,
               "#2589: qps-ploc breakpoint pause messages should keep the runtime-host success exit code");
        expect(process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
               "#2589: qps-ploc breakpoint pause messages should preserve the breakpoint pause reason");
        expect(process.stdout_text.find("debug.message: [!! ") != std::string::npos,
               "#2589: qps-ploc breakpoint pause messages should pseudo-localize the debug message");
        expect(process.stdout_text.find("Breakpoint hit.") == std::string::npos,
               "#2589: qps-ploc breakpoint pause messages should not fall back to the raw English breakpoint prose");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_host_watch_errors_localize_without_changing_watch_fields(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_watch_error_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);

    const fs::path startup_path = temp_root / "main.prg";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_text(
        startup_path,
        "LOCAL nValue\n"
        "nValue = 1\n"
        "RETURN\n");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=WatchErrorLocalization\n"
        "startup_item=main.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--breakpoint", startup_path.string() + ":2",
                "--debug-command", "continue",
                "--debug-command", "watch:"
            },
            temp_root);
        expect(process.exit_code == 0,
               "#2590: pt-BR watch errors should keep the runtime-host success exit code");
        expect(process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
               "#2590: pt-BR watch errors should preserve the breakpoint pause reason");
        expect(process.stdout_text.find("debug.watch.expression: ") != std::string::npos,
               "#2590: pt-BR watch errors should preserve the debug.watch.expression field");
        expect(process.stdout_text.find("debug.watch.ok: false") != std::string::npos,
               "#2590: pt-BR watch errors should preserve the debug.watch.ok field");
        expect(process.stdout_text.find("debug.watch.error: A expressao de watch esta vazia.") !=
                   std::string::npos,
               "#2590: pt-BR watch errors should localize the watch error prose");
        expect(process.stdout_text.find("debug.watch.error: Watch expression is empty.") ==
                   std::string::npos,
               "#2590: pt-BR watch errors should not fall back to the raw English watch error");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--breakpoint", startup_path.string() + ":2",
                "--debug-command", "continue",
                "--debug-command", "watch:"
            },
            temp_root);
        expect(process.exit_code == 0,
               "#2590: qps-ploc watch errors should keep the runtime-host success exit code");
        expect(process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
               "#2590: qps-ploc watch errors should preserve the breakpoint pause reason");
        expect(process.stdout_text.find("debug.watch.ok: false") != std::string::npos,
               "#2590: qps-ploc watch errors should preserve the debug.watch.ok field");
        expect(process.stdout_text.find("debug.watch.error: [!! ") != std::string::npos,
               "#2590: qps-ploc watch errors should pseudo-localize the watch error prose");
        expect(process.stdout_text.find("debug.watch.error: Watch expression is empty.") ==
                   std::string::npos,
               "#2590: qps-ploc watch errors should not fall back to the raw English watch error");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_host_quit_prompt_localizes_without_changing_confirmation_tokens(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_quit_prompt_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);

    const fs::path startup_path = temp_root / "quit_prompt.prg";
    const fs::path manifest_path = temp_root / "quit_prompt.cfmanifest";
    write_text(
        startup_path,
        "LOCAL nValue\n"
        "QUIT\n"
        "nValue = 1\n"
        "RETURN\n");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=QuitPromptLocalization\n"
        "startup_item=quit_prompt.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string()},
            temp_root,
            std::string("n\n"));
        expect(process.exit_code == 0,
               "#2591: runtime-host quit prompt should keep the normal success exit code when quit is cancelled");
        expect(process.stderr_text.find("Do you want to quit this application? [y/N]: ") != std::string::npos,
               "#2591: runtime-host quit prompt should preserve the en-US confirmation prompt");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string()},
            temp_root,
            std::string("n\n"));
        expect(process.exit_code == 0,
               "#2591: es-419 runtime-host quit prompt should keep the normal success exit code when quit is cancelled");
        expect(process.stderr_text.find("Desea salir de esta aplicacion? [y/N]: ") != std::string::npos,
               "#2591: es-419 runtime-host quit prompt should localize the prompt prose");
        expect(process.stderr_text.find("Do you want to quit this application?") == std::string::npos,
               "#2591: es-419 runtime-host quit prompt should not fall back to raw English prose");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string()},
            temp_root,
            std::string("n\n"));
        expect(process.exit_code == 0,
               "#2591: pt-BR runtime-host quit prompt should keep the normal success exit code when quit is cancelled");
        expect(process.stderr_text.find("Deseja sair deste aplicativo? [y/N]: ") != std::string::npos,
               "#2591: pt-BR runtime-host quit prompt should localize the prompt prose");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string()},
            temp_root,
            std::string("n\n"));
        expect(process.exit_code == 0,
               "#2591: qps-ploc runtime-host quit prompt should keep the normal success exit code when quit is cancelled");
        expect(process.stderr_text.find("[!! ") != std::string::npos,
               "#2591: qps-ploc runtime-host quit prompt should pseudo-localize the prompt prose");
        expect(process.stderr_text.find("[y/N]: ") != std::string::npos,
               "#2591: qps-ploc runtime-host quit prompt should preserve confirmation tokens");
    }

    fs::remove_all(temp_root, ignored);
}

void run_runtime_host_test(
    const char* name,
    const std::string& runtime_host_path,
    void (*test)(const std::string&)) {
    std::cerr << "BEGIN: " << name << '\n';
    test(runtime_host_path);
    std::cerr << "END: " << name << '\n';
}

}  // namespace
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "FAIL: runtime host executable path argument is required\n";
        return 1;
    }

    const std::string runtime_host_path = argv[1];
    run_runtime_host_test("breakpoint management", runtime_host_path, test_runtime_host_supports_breakpoint_management_commands);
    run_runtime_host_test("single breakpoint removal", runtime_host_path, test_runtime_host_supports_single_breakpoint_removal);
    run_runtime_host_test("implicit debug manifest", runtime_host_path, test_runtime_host_prefers_debug_manifest_for_implicit_debug_launches);
    run_runtime_host_test("compatibility launcher note", runtime_host_path, test_runtime_host_compatibility_launcher_note_reflects_xasset_fallback);
    run_runtime_host_test("xAsset pause identity", runtime_host_path, test_runtime_host_reports_xasset_pause_identity);
    run_runtime_host_test("xAsset action breakpoints", runtime_host_path, test_runtime_host_supports_xasset_action_breakpoint_commands);
    run_runtime_host_test("xAsset breakpoint metadata", runtime_host_path, test_runtime_host_surfaces_xasset_breakpoint_metadata_in_pause_output);
    run_runtime_host_test("xAsset bootstrap cleanup", runtime_host_path, test_runtime_host_removes_xasset_bootstrap_after_execution);
    run_runtime_host_test("bridge response", runtime_host_path, test_runtime_host_writes_bridge_response_artifact);
    run_runtime_host_test("verified bridge source", runtime_host_path, test_security_enabled_bridge_source_stays_inside_verified_package);
    run_runtime_host_test("zero-argument bridge export", runtime_host_path, test_runtime_host_invokes_zero_argument_bridge_export);
    run_runtime_host_test("bridge bootstrap cleanup", runtime_host_path, test_runtime_host_removes_bridge_routine_bootstrap_after_execution);
    run_runtime_host_test("escaped bridge descriptor", runtime_host_path, test_runtime_host_unescapes_bridge_descriptor_string_fields);
    run_runtime_host_test("bridge parameters", runtime_host_path, test_runtime_host_passes_bridge_request_parameters_to_export);
    run_runtime_host_test("bridge parameter count mismatch", runtime_host_path, test_runtime_host_rejects_bridge_parameter_count_mismatch);
    run_runtime_host_test("nested bridge parameter array", runtime_host_path, test_runtime_host_rejects_nested_bridge_parameter_array_for_nonzero_arity);
    run_runtime_host_test("nested bridge parameter values", runtime_host_path, test_runtime_host_rejects_nested_bridge_parameter_values_for_nonzero_arity);
    run_runtime_host_test("bridge parameter name mismatch", runtime_host_path, test_runtime_host_rejects_bridge_parameter_name_mismatch);
    run_runtime_host_test("bridge request contract", runtime_host_path, test_runtime_host_rejects_bridge_request_contract_mismatch);
    run_runtime_host_test("nested bridge descriptor", runtime_host_path, test_runtime_host_rejects_nested_bridge_descriptor_fields);
    run_runtime_host_test("bridge descriptor identity", runtime_host_path, test_runtime_host_rejects_bridge_descriptor_identity_mismatch);
    run_runtime_host_test("bridge descriptor metadata", runtime_host_path, test_runtime_host_rejects_bridge_descriptor_metadata_mismatch);
    run_runtime_host_test("usage localization", runtime_host_path, test_runtime_host_usage_text_localizes_without_changing_cli_tokens);
    run_runtime_host_test("debug error localization", runtime_host_path, test_runtime_host_debug_errors_localize_without_changing_command_tokens);
    run_runtime_host_test("pause localization", runtime_host_path, test_runtime_host_pause_messages_localize_without_changing_pause_reasons);
    run_runtime_host_test("watch localization", runtime_host_path, test_runtime_host_watch_errors_localize_without_changing_watch_fields);
    run_runtime_host_test("quit prompt localization", runtime_host_path, test_runtime_host_quit_prompt_localizes_without_changing_confirmation_tokens);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }

    std::cout << "All runtime host debug-output formatting tests passed\n";
    return 0;
}
