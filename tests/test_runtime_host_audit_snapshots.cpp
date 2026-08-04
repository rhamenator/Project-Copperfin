// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_runtime_host_debug_output_support.h"

// Safety-relevant coverage: these tests exercise immutable audit-chain and integrity contracts.

void test_security_enabled_query_file_uses_verified_snapshot(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;
    const int failures_before_test = failures;
    const fs::path temp_root =
        runtime_host_audit_temp_root("copperfin_runtime_host_verified_query_snapshot");
    const fs::path deployed_root = temp_root / "deployed";
    const fs::path content_root = deployed_root / "content";
    const fs::path startup_path = content_root / "main.prg";
    const fs::path table_path = content_root / "customers.dbf";
    const fs::path query_path = content_root / "names.qpr";
    const fs::path manifest_path = deployed_root / "app.cfmanifest";
    const fs::path locale_root = temp_root / "locales";
    const fs::path deployed_runtime_host = deployed_runtime_host_path(deployed_root, runtime_host_path);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(content_root);
    write_runtime_host_usage_catalogs(locale_root);
    write_text(
        startup_path,
        "USE 'customers.dbf' ALIAS customers\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.RowSourceType = 4\n"
        "oList.RowSource = 'names.qpr'\n"
        "oList.Requery()\n"
        "cBefore = oList.List(1)\n"
        "oList.Requery()\n"
        "cAfter = oList.List(1)\n"
        "RETURN\n");
    const auto table_create = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        {{.name = "NAME", .type = 'C', .length = 16U}},
        {{"Ada"}, {"Grace"}});
    expect(table_create.ok, "verified query snapshot fixture should create its DBF");
    const std::string query_bytes =
        "SELECT name FROM customers WHERE name = 'Ada' INTO CURSOR temp2\n";
    write_text(query_path, query_bytes);
    fs::copy_file(runtime_host_path, deployed_runtime_host, fs::copy_options::overwrite_existing);
#if defined(__unix__) || defined(__APPLE__)
    fs::permissions(
        deployed_runtime_host,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add,
        ignored);
#endif

    const auto runtime_host_hash = copperfin::security::sha256_hex_for_file(deployed_runtime_host.string());
    const auto startup_hash = copperfin::security::sha256_hex_for_file(startup_path.string());
    const auto table_hash = copperfin::security::sha256_hex_for_file(table_path.string());
    const auto query_hash = copperfin::security::sha256_hex_for_file(query_path.string());
    expect(runtime_host_hash.ok && startup_hash.ok && table_hash.ok && query_hash.ok,
           "verified query snapshot fixture should hash all packaged inputs");
    if (!runtime_host_hash.ok || !startup_hash.ok || !table_hash.ok || !query_hash.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const fs::path recorded_package_root = temp_root / "builder" / "QuerySnapshotApp";
    const fs::path recorded_content_root = recorded_package_root / "content";
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=QuerySnapshotApp\n"
        "package_root=" + quote_manifest_value(recorded_package_root.string()) + "\n"
        "content_root=" + quote_manifest_value(recorded_content_root.string()) + "\n"
        "working_directory=" + quote_manifest_value(recorded_content_root.string()) + "\n"
        "startup_item=main.prg\n"
        "startup_source=" + quote_manifest_value((recorded_content_root / "main.prg").string()) + "\n"
        "security_enabled=true\n"
        "security_role=runtime-operator\n"
        "security_mode=native\n"
        "runtime_host_sha256=" + runtime_host_hash.hex_digest + "\n"
        "asset=1|main.prg|" + quote_manifest_value((recorded_content_root / "main.prg").string()) +
            "|Program|false|true|" + startup_hash.hex_digest + "|true\n"
        "asset=2|customers.dbf|" + quote_manifest_value((recorded_content_root / "customers.dbf").string()) +
            "|Table|false|true|" + table_hash.hex_digest + "|true\n"
        "asset=3|names.qpr|" + quote_manifest_value((recorded_content_root / "names.qpr").string()) +
            "|Query|false|true|" + query_hash.hex_digest + "|true\n"
        "dotnet_story=none\n");

    ScopedEnvironmentValue locale_dir(locale_root.string());
    const auto process = run_process_capture(
        deployed_runtime_host.string(),
        {
            "--manifest", manifest_path.string(),
            "--debug",
            "--breakpoint", "6",
            "--breakpoint", "9",
            "--debug-command", "continue",
            "--debug-command", "watch:STRTOFILE(\"SELECT name FROM customers WHERE name = 'Grace' INTO CURSOR temp2\", 'names.qpr')",
            "--debug-command", "continue",
            "--debug-command", "watch:cBefore",
            "--debug-command", "watch:cAfter",
            "--debug-command", "continue"
        },
        deployed_root);
    if (process.exit_code != 0) {
        std::cerr << "verified query snapshot stdout:\n" << process.stdout_text << "\n";
        std::cerr << "verified query snapshot stderr:\n" << process.stderr_text << "\n";
    }
    expect(process.exit_code == 0,
           "security-enabled query-file startup should continue after physical query mutation");
    const std::string watch_value = "debug.watch.value: Ada";
    expect(process.stdout_text.find(watch_value) != std::string::npos,
           "verified query snapshot should preserve the initial admitted query result");
    const std::size_t first_watch = process.stdout_text.find(watch_value);
    expect(first_watch != std::string::npos &&
               process.stdout_text.find(watch_value, first_watch + watch_value.size()) != std::string::npos,
           "verified query snapshot should preserve the result after Requery");
    expect(read_text(query_path).find("'Grace'") != std::string::npos,
           "verified query snapshot should prove the physical query was replaced during debugging");

    if (failures == failures_before_test) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_security_enabled_report_and_label_execute_verified_snapshots(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    struct LayoutSnapshotCase {
        const char* file_name;
        copperfin::studio::StudioAssetKind kind;
        const char* manifest_kind;
        const char* fixture_suffix;
    };
    const std::vector<LayoutSnapshotCase> cases{
        {"verified.frx", copperfin::studio::StudioAssetKind::report, "Report", "report"},
        {"verified.lbx", copperfin::studio::StudioAssetKind::label, "Label", "label"}
    };

    for (const auto& layout_case : cases) {

    const fs::path temp_root =
        runtime_host_audit_temp_root(
            (std::string("copperfin_runtime_host_verified_") + layout_case.fixture_suffix + "_snapshot").c_str());
    const fs::path content_root = temp_root / "content";
    const fs::path asset_path = content_root / layout_case.file_name;
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path locale_root = temp_root / "locales";
    const fs::path deployed_runtime_host = deployed_runtime_host_path(temp_root, runtime_host_path);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(content_root);
    write_runtime_host_usage_catalogs(locale_root);
    write_synthetic_report_asset(asset_path);
    const fs::path sidecar_path = copperfin::studio::infer_sidecar_path(
        asset_path.string(),
        layout_case.kind);
    expect(fs::exists(sidecar_path), "synthetic layout snapshot fixture should include its memo sidecar");
    const std::string original_asset_bytes = read_text(asset_path);
    const std::string original_sidecar_bytes = read_text(sidecar_path);

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
    const auto asset_hash = copperfin::security::sha256_hex_for_file(asset_path.string());
    const auto sidecar_hash = copperfin::security::sha256_hex_for_file(sidecar_path.string());
    expect(runtime_host_hash.ok && asset_hash.ok && sidecar_hash.ok,
           "verified layout snapshot fixture should hash host, primary, and sidecar payloads");
    if (!runtime_host_hash.ok || !asset_hash.ok || !sidecar_hash.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=VerifiedLayoutSnapshot\n") +
        "package_root=" + temp_root.string() + "\n"
        "content_root=" + content_root.string() + "\n"
        "working_directory=" + content_root.string() + "\n"
        "startup_item=" + layout_case.file_name + "\n"
        "startup_source=" + asset_path.string() + "\n"
        "security_enabled=true\n"
        "security_role=runtime-operator\n"
        "security_mode=native\n"
        "runtime_host_sha256=" + runtime_host_hash.hex_digest + "\n"
        "asset=1|" + layout_case.file_name + "|" + asset_path.string() +
            "|" + layout_case.manifest_kind + "|false|true|" + asset_hash.hex_digest + "|true\n"
        "extension_payload=" + sidecar_path.string() + "|" + sidecar_hash.hex_digest + "\n"
        "dotnet_story=none\n");

    const std::string mutate_command =
        "watch:STRTOFILE('corrupt','" + asset_path.generic_string() + "')";
    const auto process = run_process_capture(
        deployed_runtime_host.string(),
        {
            "--manifest", manifest_path.string(),
            "--debug",
            "--breakpoint", "2",
            "--debug-command", "continue",
            "--debug-command", mutate_command,
            "--debug-command", "continue"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "verified layout snapshot stdout:\n" << process.stdout_text << "\n";
        std::cerr << "verified layout snapshot stderr:\n" << process.stderr_text << "\n";
    }
    expect(process.exit_code == 0,
           "security-enabled report/label startup should continue from its verified snapshot after live-file mutation");
    expect(read_text(asset_path) == "corrupt",
           "verified layout snapshot smoke should prove the paused watch command mutated the live package file");
    expect(process.stdout_text.find("debug.reason: event_loop") != std::string::npos,
           "verified layout snapshot should still reach preview after live-file mutation");
    expect(process.stdout_text.find(asset_path.string()) != std::string::npos,
           "verified layout snapshot should preserve the logical package path in debug output");
    expect(process.stdout_text.find("copperfin_xasset_snapshot_") == std::string::npos,
           "private xAsset snapshot paths should not leak into runtime debug output");

    write_text(asset_path, original_asset_bytes);
    write_text(sidecar_path, original_sidecar_bytes + "tampered-sidecar");
    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto tampered_sidecar_process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", manifest_path.string()},
            temp_root);
        expect(tampered_sidecar_process.exit_code == 8,
               "security-enabled report/label startup should reject a sidecar changed after packaging");
        expect(tampered_sidecar_process.stdout_text.find(
                   "error: Extension payload hash mismatch: " + sidecar_path.filename().string()) != std::string::npos,
               "tampered layout sidecars should fail through the localized extension-payload digest contract");
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
    }
}

void test_security_enabled_form_class_and_menu_companion_integrity(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    struct ExecutableXAssetCase {
        const char* file_name;
        copperfin::studio::StudioAssetKind kind;
        const char* manifest_kind;
        const char* fixture_suffix;
    };
    const std::vector<ExecutableXAssetCase> cases{
        {"verified.scx", copperfin::studio::StudioAssetKind::form, "Form", "form"},
        {"verified.vcx", copperfin::studio::StudioAssetKind::class_library, "Class Library", "class"},
        {"verified.mnx", copperfin::studio::StudioAssetKind::menu, "Menu", "menu"}
    };

    for (const auto& xasset_case : cases) {
        const int failures_before_case = failures;
        const fs::path temp_root =
            runtime_host_audit_temp_root(
                (std::string("copperfin_runtime_host_verified_") + xasset_case.fixture_suffix + "_companion").c_str());
        const fs::path recorded_package_root = temp_root / "builder" / "DemoApp";
        const fs::path deployed_root = temp_root / "deployed";
        const fs::path content_root = deployed_root / "content";
        const fs::path asset_path = content_root / xasset_case.file_name;
        const fs::path manifest_path = deployed_root / "app.cfmanifest";
        const fs::path locale_root = temp_root / "locales";
        const fs::path deployed_runtime_host = deployed_runtime_host_path(deployed_root, runtime_host_path);
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(content_root);
        write_runtime_host_usage_catalogs(locale_root);
        if (xasset_case.kind == copperfin::studio::StudioAssetKind::menu) {
            write_synthetic_menu_asset(asset_path);
        } else {
            write_synthetic_form_asset(asset_path);
        }
        const fs::path sidecar_path = copperfin::studio::infer_sidecar_path(
            asset_path.string(),
            xasset_case.kind);
        expect(fs::exists(sidecar_path), "executable xAsset security fixture should include its memo sidecar");
        const std::string original_sidecar_bytes = read_text(sidecar_path);

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
        const auto asset_hash = copperfin::security::sha256_hex_for_file(asset_path.string());
        const auto sidecar_hash = copperfin::security::sha256_hex_for_file(sidecar_path.string());
        expect(runtime_host_hash.ok && asset_hash.ok && sidecar_hash.ok,
               "executable xAsset security fixture should hash host, primary, and sidecar payloads");
        if (!runtime_host_hash.ok || !asset_hash.ok || !sidecar_hash.ok) {
            continue;
        }

        const fs::path recorded_content_root = recorded_package_root / "content";
        const auto write_manifest = [&](const bool include_sidecar_digest) {
            std::string text =
                std::string("manifest_version=1\n") +
                "project_title=VerifiedExecutableXAsset\n" +
                "package_root=" + recorded_package_root.string() + "\n" +
                "content_root=" + recorded_content_root.string() + "\n" +
                "working_directory=" + recorded_content_root.string() + "\n" +
                "startup_item=" + xasset_case.file_name + "\n" +
                "startup_source=" + (recorded_content_root / xasset_case.file_name).string() + "\n" +
                "security_enabled=true\n" +
                "security_role=runtime-operator\n" +
                "security_mode=native\n" +
                "runtime_host_sha256=" + runtime_host_hash.hex_digest + "\n" +
                "asset=1|" + xasset_case.file_name + "|" +
                    (recorded_content_root / xasset_case.file_name).string() + "|" +
                    xasset_case.manifest_kind + "|false|true|" + asset_hash.hex_digest + "|true\n";
            if (include_sidecar_digest) {
                text += "extension_payload=" +
                    (recorded_content_root / sidecar_path.filename()).string() + "|" +
                    sidecar_hash.hex_digest + "\n";
            }
            text += "dotnet_story=none\n";
            write_text(manifest_path, text);
        };
        const auto run_manifest = [&](const fs::path& host, const fs::path& manifest) {
            return run_process_capture(
                host.string(),
                {
                    "--manifest", manifest.string(),
                    "--debug",
                    "--breakpoint", "2",
                    "--debug-command", "continue",
                    "--debug-command", "continue"
                },
                deployed_root);
        };

        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        write_manifest(true);
        const auto valid_process = run_manifest(deployed_runtime_host, manifest_path);
        if (valid_process.exit_code != 0) {
            std::cerr << xasset_case.fixture_suffix << " companion valid stdout:\n"
                      << valid_process.stdout_text << "\n";
            std::cerr << xasset_case.fixture_suffix << " companion valid stderr:\n"
                      << valid_process.stderr_text << "\n";
        }
        expect(valid_process.exit_code == 0,
               "security-enabled form/class/menu startup should execute after primary and companion verification");
        expect(valid_process.stdout_text.find("runtime.mode: xasset-bootstrap") != std::string::npos,
               "verified form/class/menu startup should use xasset-bootstrap mode");
        expect(valid_process.stdout_text.find("debug.reason: event_loop") != std::string::npos,
               "verified form/class/menu startup should reach its event loop");
        expect(valid_process.stdout_text.find("startup.source: " + asset_path.string()) != std::string::npos,
               "relocated xAsset packages should preserve their rebound logical startup identity");
        expect(valid_process.stdout_text.find("copperfin_xasset_snapshot_") == std::string::npos,
               "private form/class/menu snapshot paths should not leak into runtime output");

        const fs::path package_alias = temp_root / "deployed-alias";
        const bool alias_created = create_directory_indirection(deployed_root, package_alias);
        expect(alias_created,
               "form/class/menu companion smoke should create its deployment-root symlink or Windows junction");
        if (alias_created) {
            const auto alias_process = run_manifest(
                package_alias / deployed_runtime_host.filename(),
                package_alias / manifest_path.filename());
            expect(alias_process.exit_code == 0,
                   "verified xAsset packages should run through a deployment-root symlink or Windows junction");
            expect(alias_process.stdout_text.find("runtime.mode: xasset-bootstrap") != std::string::npos,
                   "deployment-root xAsset indirection should preserve bootstrap execution mode");
            expect(alias_process.stdout_text.find(
                       "startup.source: " + asset_path.string()) !=
                       std::string::npos,
                   "deployment-root xAsset indirection should preserve its admitted startup identity");
            remove_directory_indirection(package_alias);
        }

        write_text(sidecar_path, original_sidecar_bytes + "tampered-sidecar");
        const auto tampered_process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", manifest_path.string()},
            deployed_root);
        expect(tampered_process.exit_code == 8,
               "security-enabled form/class/menu startup should reject a modified companion");
        expect(tampered_process.stdout_text.find(
                   "status: error\nerror: Extension payload hash mismatch: " +
                   sidecar_path.filename().string()) != std::string::npos,
               "modified executable xAsset companions should use the invariant verification status and localized error");
        write_text(sidecar_path, original_sidecar_bytes);

        write_manifest(false);
        const auto missing_digest_process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", manifest_path.string()},
            deployed_root);
        expect(missing_digest_process.exit_code == 8,
               "security-enabled form/class/menu startup should reject an undigested companion");
        expect(missing_digest_process.stdout_text.find(
                   "status: error\nerror: Packaged asset is missing a verified digest: " +
                   sidecar_path.filename().string()) != std::string::npos,
               "undigested executable xAsset companions should preserve machine status and localize the error");

        fs::remove(sidecar_path, ignored);
        ProcessResult missing_companion_process;
        if (xasset_case.kind == copperfin::studio::StudioAssetKind::menu) {
            ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
            missing_companion_process = run_process_capture(
                deployed_runtime_host.string(),
                {"--manifest", manifest_path.string()},
                deployed_root);
        } else {
            missing_companion_process = run_process_capture(
                deployed_runtime_host.string(),
                {"--manifest", manifest_path.string()},
                deployed_root);
        }
        expect(missing_companion_process.exit_code == 8,
               "security-enabled form/class/menu startup should reject a missing required companion");
        const std::string expected_missing_companion_error =
            xasset_case.kind == copperfin::studio::StudioAssetKind::menu
                ? "status: error\nerro: O asset empacotado esta ausente do pacote: "
                : "status: error\nerror: Packaged asset is missing from the package: ";
        expect(missing_companion_process.stdout_text.find(
                   expected_missing_companion_error + sidecar_path.filename().string()) != std::string::npos,
               "missing executable xAsset companions should localize text without changing machine status");

        write_text(sidecar_path, original_sidecar_bytes);
        write_manifest(true);
        fs::remove(sidecar_path, ignored);
        const auto missing_staged_companion_process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", manifest_path.string()},
            deployed_root);
        expect(missing_staged_companion_process.exit_code == 8,
               "security-enabled form/class/menu packages should reject a deleted staged companion");
        expect(missing_staged_companion_process.stdout_text.find(
                   "status: error\nerror: Extension payload is missing from the package: " +
                   sidecar_path.filename().string()) != std::string::npos,
               "deleted staged companions should fail through the extension-payload verification contract");

        const fs::path outside_sidecar = temp_root / sidecar_path.filename();
        write_text(outside_sidecar, original_sidecar_bytes);
        std::error_code symlink_error;
        fs::create_symlink(outside_sidecar, sidecar_path, symlink_error);
        if (!symlink_error) {
            write_manifest(true);
            const auto redirected_process = run_process_capture(
                deployed_runtime_host.string(),
                {"--manifest", manifest_path.string()},
                deployed_root);
            expect(redirected_process.exit_code == 8,
                   "security-enabled form/class/menu startup should reject redirected companions");
            expect(redirected_process.stdout_text.find(
                       "status: error\nerror: Package path failed physical containment validation: " +
                       sidecar_path.filename().string()) != std::string::npos,
                   "redirected executable xAsset companions should fail through physical containment verification");
            fs::remove(sidecar_path, ignored);
        }
        write_text(sidecar_path, original_sidecar_bytes);

        if (failures == failures_before_case) {
            fs::remove_all(temp_root, ignored);
        }
    }
}

void test_runtime_host_preserves_logical_identity_across_nested_directory_aliases(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const int failures_before_test = failures;
    const fs::path test_root =
        runtime_host_audit_temp_root("copperfin_runtime_host_nested_alias_identity");
    const fs::path physical_namespace = test_root / "physical";
    const fs::path logical_namespace = test_root / "logical";
    const fs::path physical_package = physical_namespace / "deployed";
    const fs::path logical_package = logical_namespace / "deployed";
    const fs::path logical_package_alias = logical_namespace / "deployed-alias";
    const fs::path physical_content = physical_package / "content";
    const fs::path logical_prg = logical_package / "content" / "main.prg";
    const fs::path logical_form = logical_package / "content" / "alias.scx";
    const fs::path manifest_path = physical_package / "app.cfmanifest";
    const fs::path deployed_runtime_host =
        deployed_runtime_host_path(physical_package, runtime_host_path);
    const fs::path locale_root = physical_namespace / "locales";
    std::error_code ignored;
    fs::remove_all(test_root, ignored);
    fs::create_directories(physical_content);
    write_runtime_host_usage_catalogs(locale_root);
    write_text(
        physical_content / "main.prg",
        "#INCLUDE 'verified.h'\nLOCAL nValue\nnValue = VERIFIED_VALUE\nRETURN\n");
    write_text(physical_content / "verified.h", "#DEFINE VERIFIED_VALUE 1\n");
    write_synthetic_form_asset(physical_content / "alias.scx");
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
    const auto prg_hash =
        copperfin::security::sha256_hex_for_file((physical_content / "main.prg").string());
    const auto include_hash =
        copperfin::security::sha256_hex_for_file((physical_content / "verified.h").string());
    expect(runtime_host_hash.ok && prg_hash.ok && include_hash.ok,
           "nested-alias PRG fixture should hash its executable package inputs");
    if (!runtime_host_hash.ok || !prg_hash.ok || !include_hash.ok) {
        fs::remove_all(test_root, ignored);
        return;
    }

    const bool namespace_alias_created =
        create_directory_indirection(physical_namespace, logical_namespace);
    expect(namespace_alias_created,
           "nested-alias identity smoke should create its outer namespace alias");
    if (!namespace_alias_created) {
        fs::remove_all(test_root, ignored);
        return;
    }

    const fs::path recorded_package = logical_namespace / "builder" / "DemoApp";
    const fs::path recorded_content = recorded_package / "content";
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=NestedAliasPrg\n"
        "package_root=" + recorded_package.string() + "\n"
        "content_root=" + recorded_content.string() + "\n"
        "working_directory=" + recorded_content.string() + "\n"
        "startup_item=main.prg\n"
        "security_enabled=true\n"
        "security_role=runtime-operator\n"
        "security_mode=native\n"
        "runtime_host_sha256=" + runtime_host_hash.hex_digest + "\n"
        "asset=1|main.prg|" + (recorded_content / "main.prg").string() +
            "|Program|false|true|" + prg_hash.hex_digest + "|true\n"
        "extension_payload=" + (recorded_content / "verified.h").string() +
            "|" + include_hash.hex_digest + "\n"
        "dotnet_story=none\n");

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            (logical_package / deployed_runtime_host.filename()).string(),
            {
                "--manifest", (logical_package / manifest_path.filename()).string(),
                "--debug",
                "--debug-command", "break:remove:3"
            },
            logical_package);
        if (process.exit_code != 5) {
            std::cerr << "nested-alias unknown-breakpoint stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "nested-alias unknown-breakpoint stderr:\n"
                      << process.stderr_text << "\n";
        }
        expect(process.exit_code == 5,
               "nested-alias unknown breakpoint should retain the debug error exit code");
        expect(process.stdout_text.find(
                   "Breakpoint desconocido: " + logical_prg.string() + ":3") !=
                   std::string::npos,
               "nested-alias unknown breakpoint should preserve the logical PRG identity");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        const auto process = run_process_capture(
            (logical_package / deployed_runtime_host.filename()).string(),
            {
                "--manifest", (logical_package / manifest_path.filename()).string(),
                "--debug",
                "--breakpoint", logical_prg.string() + ":3",
                "--debug-command", "continue",
                "--debug-command", "watch:"
            },
            logical_package);
        if (process.exit_code != 0) {
            std::cerr << "nested-alias PRG debug stdout:\n" << process.stdout_text << "\n";
            std::cerr << "nested-alias PRG debug stderr:\n" << process.stderr_text << "\n";
        }
        expect(process.exit_code == 0,
               "nested-alias PRG debug flow should retain the runtime-host success exit code");
        expect(process.stdout_text.find("startup.source: " + logical_prg.string()) !=
                   std::string::npos,
               "nested-alias PRG summary should preserve the logical startup identity");
        expect(process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
               "nested-alias logical breakpoint should match the physically verified PRG");
        expect(process.stdout_text.find("debug.location: " + logical_prg.string() + ":3") !=
                   std::string::npos,
               "nested-alias PRG pause location should preserve the logical source identity");
        expect(process.stdout_text.find("debug.breakpoint[0]: " + logical_prg.string() + ":3") !=
                   std::string::npos,
               "nested-alias PRG breakpoint inventory should preserve the logical source identity");
        expect(process.stdout_text.find("debug.frame[0]: main@" + logical_prg.string() + ":3") !=
                   std::string::npos,
               "nested-alias PRG stack frames should preserve the logical source identity");
        expect(process.stdout_text.find("debug.watch.ok: false") != std::string::npos,
               "nested-alias watch errors should preserve invariant watch fields");
        expect(process.stdout_text.find("debug.watch.error: [!! ") != std::string::npos,
               "nested-alias watch errors should preserve pseudo-localized prose");
    }

    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=NestedAliasRootlessPrg\n"
        "startup_item=main.prg\n"
        "startup_source=" + logical_prg.string() + "\n"
        "security_enabled=true\n"
        "security_role=runtime-operator\n"
        "security_mode=native\n"
        "runtime_host_sha256=" + runtime_host_hash.hex_digest + "\n"
        "asset=1|main.prg|" + (physical_content / "main.prg").string() +
            "|Program|false|true|" + prg_hash.hex_digest + "|true\n"
        "extension_payload=" + (physical_content / "verified.h").string() +
            "|" + include_hash.hex_digest + "\n"
        "dotnet_story=none\n");
    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", manifest_path.string()},
            physical_package);
        if (process.exit_code != 0) {
            std::cerr << "rootless nested-alias PRG stdout:\n" << process.stdout_text << "\n";
            std::cerr << "rootless nested-alias PRG stderr:\n" << process.stderr_text << "\n";
        }
        expect(process.exit_code == 0,
               "rootless logical startup should execute after physical alias admission");
        expect(process.stdout_text.find("startup.source: " + logical_prg.string()) !=
                   std::string::npos,
               "rootless startup summary should preserve the admitted manifest path spelling");
    }

    const fs::path physical_form = physical_content / "alias.scx";
    const fs::path physical_sidecar = copperfin::studio::infer_sidecar_path(
        physical_form.string(),
        copperfin::studio::StudioAssetKind::form);
    const auto form_hash = copperfin::security::sha256_hex_for_file(physical_form.string());
    const auto sidecar_hash =
        copperfin::security::sha256_hex_for_file(physical_sidecar.string());
    expect(runtime_host_hash.ok && form_hash.ok && sidecar_hash.ok,
           "nested-alias xAsset fixture should hash its executable package inputs");
    if (runtime_host_hash.ok && form_hash.ok && sidecar_hash.ok) {
        const std::string page_activate_action = "frmdemo.pgfmain.page2.activate";
        write_text(
            manifest_path,
            "manifest_version=1\n"
            "project_title=NestedAliasXAsset\n"
            "package_root=" + recorded_package.string() + "\n"
            "content_root=" + recorded_content.string() + "\n"
            "working_directory=" + recorded_content.string() + "\n"
            "startup_item=alias.scx\n"
            "startup_source=" + (recorded_content / "alias.scx").string() + "\n"
            "security_enabled=true\n"
            "security_role=runtime-operator\n"
            "security_mode=native\n"
            "runtime_host_sha256=" + runtime_host_hash.hex_digest + "\n"
            "asset=1|alias.scx|" + (recorded_content / "alias.scx").string() +
                "|Form|false|true|" + form_hash.hex_digest + "|true\n"
            "extension_payload=" + (recorded_content / physical_sidecar.filename()).string() +
                "|" + sidecar_hash.hex_digest + "\n"
            "dotnet_story=none\n");

        const bool package_alias_created = create_directory_indirection(
            physical_package,
            physical_namespace / "deployed-alias");
        expect(package_alias_created,
               "nested-alias xAsset smoke should create its package-root alias");
        if (package_alias_created) {
            ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
            const auto process = run_process_capture(
                (logical_package_alias / deployed_runtime_host.filename()).string(),
                {
                    "--manifest", (logical_package_alias / manifest_path.filename()).string(),
                    "--debug",
                    "--debug-command", "continue",
                    "--debug-command", "break:add-action:" + page_activate_action,
                    "--debug-command", "select:" + page_activate_action,
                    "--debug-command", "break:remove-action:" + page_activate_action
                },
                logical_package_alias);
            if (process.exit_code != 0) {
                std::cerr << "nested-alias xAsset stdout:\n" << process.stdout_text << "\n";
                std::cerr << "nested-alias xAsset stderr:\n" << process.stderr_text << "\n";
            }
            expect(process.exit_code == 0,
                   "nested-alias verified xAsset should execute through both aliases");
            expect(process.stdout_text.find("runtime.mode: xasset-bootstrap") !=
                       std::string::npos,
                   "nested-alias verified xAsset should retain bootstrap execution mode");
            expect(process.stdout_text.find("debug.reason: breakpoint") !=
                       std::string::npos,
                   "nested-alias xAsset action breakpoint should hit through the verified bootstrap");
            expect(process.stdout_text.find(
                       "debug.breakpoint[0].xasset.action_id: " + page_activate_action) !=
                       std::string::npos,
                   "nested-alias xAsset breakpoint inventory should preserve the logical action identity");
            expect(process.stdout_text.find("debug.xasset.action_id: " + page_activate_action) !=
                       std::string::npos,
                   "nested-alias xAsset pause output should preserve the logical action identity");
            expect(process.stdout_text.find("debug.breakpoint.count: 0") !=
                       std::string::npos,
                   "nested-alias xAsset action breakpoint should remove cleanly after the hit");
            expect(process.stdout_text.find("startup.source: " + logical_form.string()) !=
                       std::string::npos,
                   "nested-alias xAsset summary should resolve the package alias while preserving the logical namespace");
            expect(process.stdout_text.find("copperfin_xasset_snapshot_") ==
                       std::string::npos,
                   "nested-alias xAsset output should not leak private snapshot identities");
            const auto audit_chain = copperfin::security::verify_immutable_audit_chain(
                (physical_package / "security_audit.log").string());
            expect(audit_chain.ok && audit_chain.entries >= 2U,
                   "nested-alias security audit output should remain contained in the physical package");
            remove_directory_indirection(physical_namespace / "deployed-alias");
        }
    }

    remove_directory_indirection(logical_namespace);
    if (failures == failures_before_test) {
        fs::remove_all(test_root, ignored);
    }
}

void test_app_cfdebug_preserves_external_xasset_source_compatibility(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;
    const int failures_before_test = failures;
    const fs::path temp_root =
        runtime_host_audit_temp_root("copperfin_runtime_host_external_xasset_debug");
    const fs::path deployed_root = temp_root / "deployed";
    const fs::path source_root = temp_root / "source";
    const fs::path asset_path = source_root / "external.scx";
    const fs::path debug_manifest_path = deployed_root / "app.cfdebug";
    const fs::path deployed_runtime_host = deployed_runtime_host_path(deployed_root, runtime_host_path);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(deployed_root);
    fs::create_directories(source_root);
    write_synthetic_form_asset(asset_path);
    expect(fs::exists(copperfin::studio::infer_sidecar_path(
               asset_path.string(),
               copperfin::studio::StudioAssetKind::form)),
           "external app.cfdebug xAsset fixture should include its source-side memo companion");
    fs::copy_file(runtime_host_path, deployed_runtime_host, fs::copy_options::overwrite_existing);
#if defined(__unix__) || defined(__APPLE__)
    fs::permissions(
        deployed_runtime_host,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add,
        ignored);
#endif
    write_text(
        debug_manifest_path,
        "debug_manifest_version=2\n"
        "project_title=ExternalXAssetDebug\n"
        "package_root=" + (temp_root / "builder" / "DemoApp").string() + "\n"
        "content_root=" + (temp_root / "builder" / "DemoApp" / "content").string() + "\n"
        "working_directory=" + source_root.string() + "\n"
        "startup_item=external.scx\n"
        "startup_source=" + asset_path.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    const auto process = run_process_capture(
        deployed_runtime_host.string(),
        {
            "--manifest", debug_manifest_path.string(),
            "--debug",
            "--breakpoint", "2",
            "--debug-command", "continue",
            "--debug-command", "continue"
        },
        deployed_root);
    if (process.exit_code != 0) {
        std::cerr << "external xAsset app.cfdebug stdout:\n" << process.stdout_text << "\n";
        std::cerr << "external xAsset app.cfdebug stderr:\n" << process.stderr_text << "\n";
    }
    expect(process.exit_code == 0,
           "app.cfdebug should continue to run an external xAsset source and companion");
    expect(process.stdout_text.find("startup.source: " + asset_path.string()) != std::string::npos,
           "app.cfdebug should preserve the external xAsset source identity");
    expect(process.stdout_text.find("runtime.mode: xasset-bootstrap") != std::string::npos,
           "external app.cfdebug xAssets should continue to use bootstrap execution");
    expect(process.stdout_text.find("debug.reason: event_loop") != std::string::npos,
           "external app.cfdebug should execute memo-backed xAsset lifecycle methods");

    if (failures == failures_before_test) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_app_cfdebug_rejects_inaccessible_external_startup_source(
    const std::string& runtime_host_path) {
#if defined(_WIN32)
    (void)runtime_host_path;
    return;
#else
    namespace fs = std::filesystem;
    const int failures_before_test = failures;
    const fs::path temp_root =
        runtime_host_audit_temp_root("copperfin_runtime_host_inaccessible_external_debug");
    const fs::path deployed_root = temp_root / "deployed";
    const fs::path inaccessible_parent = temp_root / "inaccessible";
    const fs::path external_source = inaccessible_parent / "startup.prg";
    const fs::path debug_manifest_path = deployed_root / "app.cfdebug";
    const fs::path deployed_runtime_host = deployed_runtime_host_path(deployed_root, runtime_host_path);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(deployed_root);
    fs::create_directories(inaccessible_parent);
    write_text(external_source, "RETURN\n");
    fs::copy_file(runtime_host_path, deployed_runtime_host, fs::copy_options::overwrite_existing);
    fs::permissions(
        deployed_runtime_host,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add,
        ignored);
    fs::permissions(inaccessible_parent, fs::perms::none, fs::perm_options::replace, ignored);

    std::error_code inaccessible_probe_error;
    (void)fs::status(external_source, inaccessible_probe_error);
    if (!inaccessible_probe_error) {
        fs::permissions(inaccessible_parent, fs::perms::owner_all, fs::perm_options::replace, ignored);
        fs::remove_all(temp_root, ignored);
        return;
    }

    write_text(
        debug_manifest_path,
        "debug_manifest_version=2\n"
        "project_title=InaccessibleExternalStartupSource\n"
        "package_root=" + deployed_root.string() + "\n"
        "content_root=" + (deployed_root / "content").string() + "\n"
        "working_directory=" + deployed_root.string() + "\n"
        "startup_item=startup.prg\n"
        "startup_source=" + external_source.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");
    const auto process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", debug_manifest_path.string(), "--debug"},
        deployed_root);

    expect(process.exit_code == 4,
           "#4331: inaccessible external startup source should use the documented startup failure exit code");
    expect(process.stdout_text.find("status: error") != std::string::npos,
           "#4331: inaccessible external startup source should return the normal error status");
    expect(process.stderr_text.find("filesystem_error") == std::string::npos &&
               process.stderr_text.find("filesystem error") == std::string::npos,
           "#4331: inaccessible external startup source should not escape a filesystem exception");

    fs::permissions(inaccessible_parent, fs::perms::owner_all, fs::perm_options::replace, ignored);
    if (failures == failures_before_test) {
        fs::remove_all(temp_root, ignored);
    }
#endif
}

void test_app_cfdebug_rejects_file_valued_working_directory(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        runtime_host_audit_temp_root("copperfin_runtime_host_file_working_directory");
    const fs::path deployed_root = temp_root / "deployed";
    const fs::path content_root = deployed_root / "content";
    const fs::path startup_path = content_root / "main.prg";
    const fs::path working_directory_file = temp_root / "not-a-directory.txt";
    const fs::path debug_manifest_path = deployed_root / "app.cfdebug";
    const fs::path deployed_runtime_host = deployed_runtime_host_path(deployed_root, runtime_host_path);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(content_root);
    write_text(startup_path, "RETURN\n");
    write_text(working_directory_file, "file-valued debug working directory");
    fs::copy_file(runtime_host_path, deployed_runtime_host, fs::copy_options::overwrite_existing);
#if defined(__unix__) || defined(__APPLE__)
    fs::permissions(
        deployed_runtime_host,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add,
        ignored);
#endif

    write_text(
        debug_manifest_path,
        "debug_manifest_version=2\n"
        "project_title=FileWorkingDirectory\n"
        "package_root=" + deployed_root.string() + "\n"
        "content_root=" + content_root.string() + "\n"
        "working_directory=" + working_directory_file.string() + "\n"
        "startup_item=main.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    const auto process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", debug_manifest_path.string(), "--debug"},
        deployed_root);
    expect(process.exit_code == 0,
           "#3989: app.cfdebug should launch after rejecting a file-valued working directory");
    expect(process.stdout_text.find("working.directory: " + content_root.string()) !=
               std::string::npos,
           "#3989: runtime host should fall back from a file-valued debug working directory to package content");
    expect(process.stdout_text.find("working.directory: " + working_directory_file.string()) ==
               std::string::npos,
           "#3989: runtime host must not publish a file as the effective working directory");

    const fs::path missing_working_directory =
        temp_root / "missing" / "basename-directory-decoy";
    const fs::path basename_directory_decoy =
        deployed_root / missing_working_directory.filename();
    fs::create_directories(basename_directory_decoy);
    write_text(
        debug_manifest_path,
        "debug_manifest_version=2\n"
        "project_title=MissingWorkingDirectory\n"
        "package_root=" + deployed_root.string() + "\n"
        "content_root=" + content_root.string() + "\n"
        "working_directory=" + missing_working_directory.string() + "\n"
        "startup_item=main.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");
    const auto missing_process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", debug_manifest_path.string(), "--debug"},
        deployed_root);
    expect(missing_process.exit_code == 0,
           "#3989: app.cfdebug should launch after rejecting a missing working directory");
    expect(missing_process.stdout_text.find("working.directory: " + content_root.string()) !=
               std::string::npos,
           "#3989: a missing debug working directory should retain the package-content fallback");
    expect(missing_process.stdout_text.find(
               "working.directory: " + basename_directory_decoy.string()) == std::string::npos,
           "#3989: debug working-directory resolution must not bind an unrelated basename directory");

#if !defined(_WIN32)
    const fs::path inaccessible_parent = temp_root / "inaccessible";
    const fs::path inaccessible_working_directory = inaccessible_parent / "working";
    fs::create_directories(inaccessible_working_directory);
    fs::permissions(inaccessible_parent, fs::perms::none, fs::perm_options::replace, ignored);
    std::error_code inaccessible_probe_error;
    (void)fs::status(inaccessible_working_directory, inaccessible_probe_error);
    if (inaccessible_probe_error) {
        write_text(
            debug_manifest_path,
            "debug_manifest_version=2\n"
            "project_title=InaccessibleWorkingDirectory\n"
            "package_root=" + deployed_root.string() + "\n"
            "content_root=" + content_root.string() + "\n"
            "working_directory=" + inaccessible_working_directory.string() + "\n"
            "startup_item=main.prg\n"
            "startup_source=" + startup_path.string() + "\n"
            "security_enabled=false\n"
            "security_role=\n"
            "security_mode=native\n"
            "dotnet_story=none\n");
        const auto inaccessible_process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", debug_manifest_path.string(), "--debug"},
            deployed_root);
        expect(inaccessible_process.exit_code == 0,
               "#3989: an unstatable debug working directory should fall back without throwing");
        expect(inaccessible_process.stdout_text.find(
                   "working.directory: " + content_root.string()) != std::string::npos,
               "#3989: an unstatable debug working directory should retain the package-content fallback");
    }
    fs::permissions(
        inaccessible_parent,
        fs::perms::owner_all,
        fs::perm_options::replace,
        ignored);
#endif

    write_text(
        debug_manifest_path,
        "debug_manifest_version=2\n"
        "project_title=StartupFallbackWorkingDirectory\n"
        "package_root=" + deployed_root.string() + "\n"
        "content_root=" + (temp_root / "missing-content-root").string() + "\n"
        "working_directory=" + working_directory_file.string() + "\n"
        "startup_item=main.prg\n"
        "startup_source=\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");
    const auto startup_fallback_process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", debug_manifest_path.string(), "--debug"},
        deployed_root);
    expect(startup_fallback_process.exit_code == 0,
           "#3989: legacy startup_item lookup should survive a file-valued working-directory root");
    expect(startup_fallback_process.stdout_text.find(
               "startup.source: " + startup_path.string()) != std::string::npos,
           "#3989: legacy startup_item lookup should fall back to package content");
    expect(startup_fallback_process.stdout_text.find(
               "working.directory: " + content_root.string()) != std::string::npos,
           "#3989: startup lookup and session setup should share the package-content directory fallback");

    fs::remove_all(temp_root, ignored);
}
