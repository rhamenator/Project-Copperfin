// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_runtime_host_debug_output_support.h"

// Safety-relevant coverage: these tests exercise immutable audit-chain and integrity contracts.
namespace {
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
        fs::temp_directory_path() /
        (std::string("copperfin_runtime_host_verified_") + layout_case.fixture_suffix + "_snapshot");
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
            fs::temp_directory_path() /
            (std::string("copperfin_runtime_host_verified_") + xasset_case.fixture_suffix + "_companion");
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
        fs::temp_directory_path() / "copperfin_runtime_host_nested_alias_identity";
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
        fs::temp_directory_path() / "copperfin_runtime_host_external_xasset_debug";
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

void test_app_cfdebug_rejects_file_valued_working_directory(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_runtime_host_file_working_directory";
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

void test_security_enabled_writable_package_data_contract(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;
    const int failures_before_test = failures;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_writable_package_data";
    const fs::path recorded_package_root = temp_root / "builder" / "WritableDataApp";
    const fs::path recorded_content_root = recorded_package_root / "content";
    const fs::path deployed_root = temp_root / "deployed";
    const fs::path content_root = deployed_root / "content";
    const fs::path startup_path = content_root / "main.prg";
    const fs::path table_path = content_root / "customers.dbf";
    const fs::path memo_path = content_root / "customers.fpt";
    const fs::path database_path = content_root / "catalog.dbc";
    const fs::path database_memo_path = content_root / "catalog.dct";
    const fs::path database_index_path = content_root / "catalog.dcx";
    const fs::path manifest_path = deployed_root / "app.cfmanifest";
    const fs::path locale_root = temp_root / "locales";
    const fs::path deployed_runtime_host = deployed_runtime_host_path(deployed_root, runtime_host_path);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(content_root);
    write_runtime_host_usage_catalogs(locale_root);
    write_text(
        startup_path,
        "OPEN DATABASE 'catalog.dbc' SHARED NOUPDATE\n"
        "USE 'customers.dbf' ALIAS MutableData IN 0\n"
        "REPLACE NOTE WITH 'runtime-change'\n"
        "USE IN MutableData\n"
        "CLOSE DATABASE\n"
        "RETURN\n");
    write_synthetic_writable_data_asset(table_path);
    const std::vector<copperfin::vfp::DbfFieldDescriptor> database_fields{
        {.name = "OBJECTTYPE", .type = 'C', .length = 12U},
        {.name = "OBJECTNAME", .type = 'C', .length = 40U},
        {.name = "PARENTNAME", .type = 'C', .length = 40U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U},
        {.name = "CODE", .type = 'M', .length = 4U},
    };
    const auto database_create = copperfin::vfp::create_dbf_table_file(
        database_path.string(),
        database_fields,
        {{"Database", "catalog", "", "Packaged database fixture", "PUBLIC gDbcStoredCode\ngDbcStoredCode = .T."}});
    expect(database_create.ok, "writable package-data fixture should create a real DBC/DCT pair");
    write_synthetic_database_index(database_index_path);
    const std::string original_database_contents = read_text(database_path);
    const std::string original_database_memo_contents = read_text(database_memo_path);
    const std::string original_database_index_contents = read_text(database_index_path);
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
    const auto table_seed_hash = copperfin::security::sha256_hex_for_file(table_path.string());
    const auto memo_seed_hash = copperfin::security::sha256_hex_for_file(memo_path.string());
    const auto database_seed_hash = copperfin::security::sha256_hex_for_file(database_path.string());
    const auto database_memo_seed_hash = copperfin::security::sha256_hex_for_file(database_memo_path.string());
    const auto database_index_seed_hash = copperfin::security::sha256_hex_for_file(database_index_path.string());
    expect(
        runtime_host_hash.ok && startup_hash.ok && table_seed_hash.ok && memo_seed_hash.ok &&
            database_seed_hash.ok && database_memo_seed_hash.ok && database_index_seed_hash.ok,
        "writable package-data fixture should hash immutable and writable seed files");
    if (!runtime_host_hash.ok || !startup_hash.ok || !table_seed_hash.ok || !memo_seed_hash.ok ||
        !database_seed_hash.ok || !database_memo_seed_hash.ok || !database_index_seed_hash.ok) {
        return;
    }

    const auto write_manifest = [&](
        const bool include_memo_payload,
        const std::string& data_policy,
        const std::string& table_seed_override = {},
        const std::string& memo_seed_override = {}) {
        const std::string& table_seed = table_seed_override.empty()
            ? table_seed_hash.hex_digest
            : table_seed_override;
        const std::string& memo_seed = memo_seed_override.empty()
            ? memo_seed_hash.hex_digest
            : memo_seed_override;
        std::string text =
            "manifest_version=3\n"
            "project_title=WritableDataApp\n"
            "package_root=" + recorded_package_root.string() + "\n"
            "content_root=" + recorded_content_root.string() + "\n"
            "working_directory=" + recorded_content_root.string() + "\n"
            "startup_item=main.prg\n"
            "startup_source=" + (recorded_content_root / "main.prg").string() + "\n"
            "security_enabled=true\n"
            "security_role=runtime-operator\n"
            "security_mode=native\n"
            "runtime_host_sha256=" + runtime_host_hash.hex_digest + "\n"
            "data_policy=" + data_policy + "\n"
            "asset=1|main.prg|" + (recorded_content_root / "main.prg").string() +
                "|Program|false|true|" + startup_hash.hex_digest + "|true\n"
            "asset=2|customers.dbf|" + (recorded_content_root / "customers.dbf").string() +
                "|Table|false|true|" + table_seed + "|true\n"
            "asset=3|catalog.dbc|" + (recorded_content_root / "catalog.dbc").string() +
                "|Database|false|true|" + database_seed_hash.hex_digest + "|true\n"
            "data_asset=" + (recorded_content_root / "customers.dbf").string() + "|package_writable\n";
        if (include_memo_payload) {
            text += "data_payload=" + (recorded_content_root / "customers.fpt").string() +
                "|package_writable|" + memo_seed + "\n";
        }
        text +=
            "extension_payload=" + (recorded_content_root / "catalog.dct").string() +
                "|" + database_memo_seed_hash.hex_digest + "\n"
            "extension_payload=" + (recorded_content_root / "catalog.dcx").string() +
                "|" + database_index_seed_hash.hex_digest + "\n"
            "dotnet_story=none\n";
        write_text(manifest_path, text);
    };

    const auto prelaunch_write = copperfin::vfp::replace_record_field_value(
        table_path.string(),
        0U,
        "NOTE",
        "prelaunch-change");
    expect(prelaunch_write.ok, "writable package-data fixture should mutate DBF/FPT bytes before launch");

    ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
    write_manifest(true, "package_writable");
    const auto first_process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", manifest_path.string()},
        deployed_root);
    if (first_process.exit_code != 0) {
        std::cerr << "writable package-data first stdout:\n" << first_process.stdout_text << "\n";
        std::cerr << "writable package-data first stderr:\n" << first_process.stderr_text << "\n";
    }
    expect(first_process.exit_code == 0,
           "security startup should open verified DBC/DCT/DCX bytes and accept writable DBF/FPT data");
    expect(first_process.stdout_text.find("data.policy: package_writable") != std::string::npos &&
               first_process.stdout_text.find("data.asset_count: 1") != std::string::npos &&
               first_process.stdout_text.find("data.payload_count: 1") != std::string::npos,
           "runtime summary should expose invariant writable package-data fields");

    const auto changed_table = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
    expect(changed_table.ok && !changed_table.table.records.empty(),
           "runtime USE/REPLACE should leave writable packaged DBF data parseable");
    if (changed_table.ok && !changed_table.table.records.empty()) {
        const auto note = std::find_if(
            changed_table.table.records.front().values.begin(),
            changed_table.table.records.front().values.end(),
            [](const auto& value) { return value.field_name == "NOTE"; });
        expect(note != changed_table.table.records.front().values.end() &&
                   note->display_value == "runtime-change",
               "runtime USE/REPLACE should persist memo updates in package-writable DBF/FPT data");
    }

    const auto second_process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", manifest_path.string()},
        deployed_root);
    expect(second_process.exit_code == 0,
           "later security launches should accept ordinary writable DBF/FPT data changes");
    const auto relaunched_table = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
    bool relaunch_persisted = false;
    if (relaunched_table.ok && !relaunched_table.table.records.empty()) {
        const auto relaunched_note = std::find_if(
            relaunched_table.table.records.front().values.begin(),
            relaunched_table.table.records.front().values.end(),
            [](const auto& value) { return value.field_name == "NOTE"; });
        relaunch_persisted =
            relaunched_note != relaunched_table.table.records.front().values.end() &&
            relaunched_note->display_value == "runtime-change";
    }
    expect(
        relaunch_persisted,
        "package-writable DBF/FPT data should remain parseable and persistent after relaunch");

    const fs::path debug_manifest_path = deployed_root / "app.cfdebug";
    std::string debug_manifest_text = read_text(manifest_path);
    debug_manifest_text.replace(
        debug_manifest_text.find("manifest_version=3"),
        std::string("manifest_version=3").size(),
        "debug_manifest_version=3");
    write_text(debug_manifest_path, debug_manifest_text);
    const auto debug_manifest_process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", debug_manifest_path.string()},
        deployed_root);
    expect(debug_manifest_process.exit_code == 0,
           "security-enabled app.cfdebug should share the version-3 writable DBF contract");

    write_text(database_path, "modified executable database metadata");
    const auto database_tamper_process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", manifest_path.string()},
        deployed_root);
    expect(database_tamper_process.exit_code == 8,
           "OPEN DATABASE package execution should retain immutable DBC digest enforcement");
    expect(database_tamper_process.stdout_text.find(
               "error: Packaged asset hash mismatch: catalog.dbc") != std::string::npos,
           "DBC tamper should retain the localized immutable packaged-asset digest error");
    write_text(database_path, original_database_contents);

    const std::string startup_source = read_text(startup_path);
    write_text(startup_path, startup_source + "* immutable tamper\n");
    const auto immutable_tamper_process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", manifest_path.string()},
        deployed_root);
    expect(immutable_tamper_process.exit_code == 8,
           "writable data classification should not weaken immutable PRG digest enforcement");
    expect(immutable_tamper_process.stdout_text.find(
               "error: Packaged asset hash mismatch: main.prg") != std::string::npos,
           "immutable PRG tamper should retain the localized packaged-asset digest error");
    write_text(startup_path, startup_source);

    write_manifest(false, "package_writable");
    const auto omitted_payload_process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", manifest_path.string()},
        deployed_root);
    expect(omitted_payload_process.exit_code == 8,
           "security startup should reject an existing writable companion omitted from data_payload");
    expect(omitted_payload_process.stdout_text.find(
               "error: Packaged asset is missing a verified digest: customers.fpt") != std::string::npos,
           "omitted writable companion records should preserve localized missing-digest diagnostics");

    write_manifest(true, "package_writable");
    const std::string memo_contents = read_text(memo_path);
    fs::remove(memo_path, ignored);
    const auto missing_writable_payload_process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", manifest_path.string()},
        deployed_root);
    expect(missing_writable_payload_process.exit_code == 8,
           "security startup should reject a missing declared writable DBF companion");
    expect(missing_writable_payload_process.stdout_text.find(
               "error: Extension payload is missing from the package: customers.fpt") != std::string::npos,
           "missing writable DBF companions should preserve localized payload-missing diagnostics");
    write_text(memo_path, memo_contents);

    write_manifest(true, "package_writable");
    fs::remove(database_memo_path, ignored);
    const auto missing_payload_process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", manifest_path.string()},
        deployed_root);
    expect(missing_payload_process.exit_code == 8,
           "security startup should reject a missing declared immutable DBC companion");
    expect(missing_payload_process.stdout_text.find(
               "error: Extension payload is missing from the package: catalog.dct") != std::string::npos,
           "missing immutable DBC companions should retain localized payload-missing diagnostics");
    write_text(database_memo_path, original_database_memo_contents);

    fs::remove(database_index_path, ignored);
    const auto missing_index_process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", manifest_path.string()},
        deployed_root);
    expect(missing_index_process.exit_code == 8,
           "security startup should reject a missing declared immutable DCX companion");
    expect(missing_index_process.stdout_text.find(
               "error: Extension payload is missing from the package: catalog.dcx") != std::string::npos,
           "missing immutable DCX companions should retain localized payload-missing diagnostics");
    write_text(database_index_path, original_database_index_contents);

    fs::remove(database_path, ignored);
    const auto missing_database_process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", manifest_path.string()},
        deployed_root);
    expect(missing_database_process.exit_code == 8,
           "security startup should reject a missing declared immutable DBC primary");
    expect(missing_database_process.stdout_text.find(
               "error: Packaged asset is missing from the package: catalog.dbc") != std::string::npos,
           "missing immutable DBC primaries should retain localized asset-missing diagnostics");
    write_text(database_path, original_database_contents);

    write_manifest(true, "unsupported");
    {
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
        const auto policy_process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", manifest_path.string()},
            deployed_root);
        expect(policy_process.exit_code == 4,
               "unsupported writable data policies should fail through manifest-contract validation");
        expect(policy_process.stdout_text.find(
                   "status: error\nerro: data_policy esta ausente ou nao e compativel no manifesto.") !=
                   std::string::npos,
               "writable data policy errors should localize without changing machine status");
    }

    write_manifest(true, "package_writable", "not-a-sha256");
    const auto malformed_primary_seed_process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", manifest_path.string()},
        deployed_root);
    expect(malformed_primary_seed_process.exit_code == 8 &&
               malformed_primary_seed_process.stdout_text.find(
                   "error: data_asset entry is malformed in manifest.") != std::string::npos,
           "writable DBF seed provenance should require SHA-256 syntax");

    write_manifest(true, "package_writable", {}, "not-a-sha256");
    const auto malformed_payload_seed_process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", manifest_path.string()},
        deployed_root);
    expect(malformed_payload_seed_process.exit_code == 8 &&
               malformed_payload_seed_process.stdout_text.find(
                   "error: data_payload entry is malformed in manifest.") != std::string::npos,
           "writable DBF companion seed provenance should require SHA-256 syntax");

    write_manifest(true, "package_writable");
    write_text(
        manifest_path,
        read_text(manifest_path) + "data_policy=package_writable\n");
    const auto duplicate_policy_process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", manifest_path.string()},
        deployed_root);
    expect(duplicate_policy_process.exit_code == 4 &&
               duplicate_policy_process.stdout_text.find(
                   "error: data_policy is missing or unsupported in manifest.") != std::string::npos,
           "version-3 manifests should reject duplicate data_policy fields before security verification");

    write_manifest(true, "package_writable");
    write_text(
        manifest_path,
        read_text(manifest_path) +
            "data_asset=" + (recorded_content_root / "customers.dbf").string() +
            "|package_writable\n");
    const auto duplicate_data_asset_process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", manifest_path.string()},
        deployed_root);
    expect(duplicate_data_asset_process.exit_code == 8 &&
               duplicate_data_asset_process.stdout_text.find(
                   "error: data_asset entry is malformed in manifest.") != std::string::npos,
           "security verification should reject duplicate writable DBF declarations");

    write_manifest(true, "package_writable");
    write_text(
        manifest_path,
        read_text(manifest_path) +
            "data_payload=" + (recorded_content_root / "customers.fpt").string() +
            "|package_writable|" + memo_seed_hash.hex_digest + "\n");
    const auto duplicate_data_payload_process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", manifest_path.string()},
        deployed_root);
    expect(duplicate_data_payload_process.exit_code == 8 &&
               duplicate_data_payload_process.stdout_text.find(
                   "error: data_payload entry is malformed in manifest.") != std::string::npos,
           "security verification should reject duplicate writable DBF companion declarations");

    if (failures == failures_before_test) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_extension_payload_basename_fallback(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_payload_path_fidelity";
    const fs::path builder_root = temp_root / "builder" / "DemoApp";
    const fs::path deployed_root = temp_root / "deployed";
    const fs::path content_root = deployed_root / "content";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(builder_root / "content" / "plugins");
    fs::create_directories(content_root);

    const fs::path deployed_runtime_host = deployed_runtime_host_path(deployed_root, runtime_host_path);
    const fs::path startup_path = content_root / "main.prg";
    const fs::path root_helper_path = deployed_root / "helper.dll";
    const fs::path manifest_path = deployed_root / "app.cfmanifest";
    const fs::path locale_root = temp_root / "locales";

    fs::copy_file(runtime_host_path, deployed_runtime_host, fs::copy_options::overwrite_existing);
#if defined(__unix__) || defined(__APPLE__)
    fs::permissions(
        deployed_runtime_host,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add,
        ignored);
#endif

    write_runtime_host_usage_catalogs(locale_root);
    write_text(startup_path, "RETURN\n");
    write_text(root_helper_path, "plugin-payload");

    const auto runtime_host_hash = copperfin::security::sha256_hex_for_file(deployed_runtime_host.string());
    const auto helper_hash = copperfin::security::sha256_hex_for_file(root_helper_path.string());
    expect(runtime_host_hash.ok, "payload-path fidelity fixture should hash the deployed runtime host");
    expect(helper_hash.ok, "payload-path fidelity fixture should hash the decoy root helper payload");
    if (!runtime_host_hash.ok || !helper_hash.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=PayloadPathFidelity\n"
        "project_path=" + (builder_root / "demo.pjx").string() + "\n"
        "package_root=" + builder_root.string() + "\n"
        "content_root=" + (builder_root / "content").string() + "\n"
        "working_directory=" + (builder_root / "content").string() + "\n"
        "startup_item=main.prg\n"
        "startup_source=" + (builder_root / "content" / "main.prg").string() + "\n"
        "configuration=debug\n"
        "security_enabled=true\n"
        "security_role=developer\n"
        "security_mode=native\n"
        "audit_log_path=" + (builder_root / "security_audit.log").string() + "\n"
        "runtime_host_sha256=" + runtime_host_hash.hex_digest + "\n"
        "extension_payload=" + (builder_root / deployed_runtime_host.filename()).string() + "|" + runtime_host_hash.hex_digest + "\n"
        "extension_payload=" + (builder_root / "content" / "plugins" / "helper.dll").string() + "|" + helper_hash.hex_digest + "\n"
        "dotnet_story=none\n");

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", manifest_path.string()},
            temp_root);

        if (process.exit_code == 0) {
            std::cerr << "payload-path fidelity stdout:\n" << process.stdout_text << "\n";
            std::cerr << "payload-path fidelity stderr:\n" << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 8,
               "runtime host should reject extension payloads that only match by basename outside their recorded package path");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "runtime host payload-path fidelity failures should preserve machine-readable status");
        expect(process.stdout_text.find("error: Extension payload is missing from the package: helper.dll") !=
                   std::string::npos,
               "runtime host should report the missing recorded payload path instead of accepting a same-named root payload");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
        const auto process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", manifest_path.string()},
            temp_root);

        expect(process.exit_code == 8,
               "#2588: pt-BR payload-path fidelity failures should keep the manifest verification exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2588: pt-BR payload-path fidelity failures should preserve machine-readable status");
        expect(process.stdout_text.find("erro: O payload de extensao esta ausente do pacote: helper.dll") !=
                   std::string::npos,
               "#2588: pt-BR payload-path fidelity failures should localize the missing payload error while preserving the file name");
        expect(process.stdout_text.find("Extension payload is missing from the package: helper.dll") ==
                   std::string::npos,
               "#2588: pt-BR payload-path fidelity failures should not fall back to the raw English error");
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_manifest_verification_errors_localize_without_changing_contracts(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_manifest_error_localization";
    const fs::path deployed_root = temp_root / "deployed";
    const fs::path content_root = deployed_root / "content";
    const fs::path locale_root = temp_root / "locales";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(content_root);

    const fs::path deployed_runtime_host = deployed_runtime_host_path(deployed_root, runtime_host_path);
    const fs::path startup_path = content_root / "main.prg";
    const fs::path manifest_path = deployed_root / "app.cfmanifest";

    fs::copy_file(runtime_host_path, deployed_runtime_host, fs::copy_options::overwrite_existing);
#if defined(__unix__) || defined(__APPLE__)
    fs::permissions(
        deployed_runtime_host,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add,
        ignored);
#endif

    write_runtime_host_usage_catalogs(locale_root);
    write_text(startup_path, "RETURN\n");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=ManifestErrorLocalization\n"
        "project_path=" + (temp_root / "demo.pjx").string() + "\n"
        "package_root=" + deployed_root.string() + "\n"
        "content_root=" + content_root.string() + "\n"
        "working_directory=" + content_root.string() + "\n"
        "startup_item=main.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "configuration=debug\n"
        "security_enabled=true\n"
        "security_role=developer\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", manifest_path.string()},
            temp_root);

        expect(process.exit_code == 8,
               "#2588: es-419 manifest verification failures should keep the manifest verification exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2588: es-419 manifest verification failures should preserve machine-readable status");
        expect(process.stdout_text.find(
                   "error: Al manifiesto con seguridad habilitada le falta runtime_host_sha256.") !=
                   std::string::npos,
               "#2588: es-419 manifest verification failures should localize the missing runtime_host_sha256 error");
        expect(process.stdout_text.find("Security-enabled manifest is missing runtime_host_sha256.") ==
                   std::string::npos,
               "#2588: es-419 manifest verification failures should not fall back to the raw English error");
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_audit_paths_outside_the_direct_package(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_runtime_host_audit_path_containment";
    const fs::path packages_root = temp_root / "packages";
    const fs::path external_root = temp_root / "external";
    const fs::path external_audit_path = external_root / "security_audit.log";
    const fs::path locale_root = temp_root / "locales";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(packages_root);
    fs::create_directories(external_root);
    write_runtime_host_usage_catalogs(locale_root);

    const auto write_denial_manifest = [](
                                           const fs::path& case_root,
                                           const std::string& package_root,
                                           const std::string& audit_log_path) {
        const fs::path content_root = case_root / "content";
        fs::create_directories(content_root);
        write_text(content_root / "main.prg", "RETURN\n");
        write_text(
            case_root / "app.cfmanifest",
            "manifest_version=1\n"
            "project_title=AuditPathContainment\n"
            "package_root=" + package_root + "\n"
            "content_root=" + content_root.string() + "\n"
            "working_directory=" + content_root.string() + "\n"
            "startup_item=main.prg\n"
            "startup_source=" + (content_root / "main.prg").string() + "\n"
            "security_enabled=true\n"
            "security_role=guest\n"
            "security_mode=native\n"
            "audit_log_path=" + audit_log_path + "\n"
            "dotnet_story=none\n");
    };

    ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
    ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "en-US");

    {
        const fs::path case_root = packages_root / "valid_local";
        const fs::path local_audit_path = case_root / "logs" / "custom.log";
        write_denial_manifest(case_root, case_root.string(), "logs/custom.log");

        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", (case_root / "app.cfmanifest").string()},
            temp_root);

        expect(process.exit_code == 7,
               "#4015: a direct package-local audit path should preserve the policy-denial exit code");
        const auto audit_chain = copperfin::security::verify_immutable_audit_chain(
            local_audit_path.string());
        expect(audit_chain.ok && audit_chain.entries == 1U,
               "#4015: a new package-local audit path should create direct directories and retain valid chain behavior");
        expect(!fs::exists(case_root / "security_audit.log"),
               "#4015: a valid custom package-local audit path should not be replaced by the default leaf");
    }

#if defined(_WIN32)
    {
        const fs::path case_root = packages_root / "windows_case_fidelity";
        const fs::path local_audit_path = case_root / "logs" / "case-sensitive-spelling.log";
        std::wstring differently_cased_root = case_root.native();
        std::transform(
            differently_cased_root.begin(),
            differently_cased_root.end(),
            differently_cased_root.begin(),
            [](const wchar_t ch) { return static_cast<wchar_t>(std::towupper(ch)); });
        write_denial_manifest(
            case_root,
            fs::path(differently_cased_root).string(),
            local_audit_path.string());

        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", (case_root / "app.cfmanifest").string()},
            temp_root);

        expect(process.exit_code == 7,
               "#4015: Windows audit rebinding should compare package components case-insensitively");
        const auto audit_chain = copperfin::security::verify_immutable_audit_chain(
            local_audit_path.string());
        expect(audit_chain.ok && audit_chain.entries == 1U,
               "#4015: Windows case-insensitive rebinding should preserve the exact packaged audit leaf");
    }
#endif

    const auto expect_rejected_path = [&](const std::string& case_name,
                                           const std::string& package_root,
                                           const std::string& audit_log_path,
                                           const fs::path& sentinel_path,
                                           const std::string& expected_file_name) {
        const fs::path case_root = packages_root / case_name;
        fs::create_directories(case_root);
        write_text(sentinel_path, "");
        write_denial_manifest(case_root, package_root, audit_log_path);

        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", (case_root / "app.cfmanifest").string()},
            temp_root);

        expect(process.exit_code == 8,
               "#4015: rejected audit paths should preserve the manifest-verification exit code for " + case_name);
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#4015: rejected audit paths should preserve machine-readable error status for " + case_name);
        expect(process.stdout_text.find(
                   "error: Package path failed physical containment validation: " + expected_file_name) !=
                   std::string::npos,
               "#4015: rejected audit paths should use the localized physical-containment diagnostic for " + case_name);
        expect(read_text(sentinel_path).empty(),
               "#4015: rejected audit paths must not modify the sentinel for " + case_name);
        expect(!fs::exists(case_root / "security_audit.log"),
               "#4015: rejected explicit audit paths should fail instead of silently appending to a fallback for " + case_name);
    };

    expect_rejected_path(
        "absolute_external",
        (packages_root / "absolute_external").string(),
        external_audit_path.string(),
        external_audit_path,
        external_audit_path.filename().string());
    expect_rejected_path(
        "relative_escape",
        (packages_root / "relative_escape").string(),
        "../../external/security_audit.log",
        external_audit_path,
        external_audit_path.filename().string());

    {
        const fs::path case_root = packages_root / "hard_link_leaf";
        const fs::path hard_link_path = case_root / "hard-linked.log";
        fs::create_directories(case_root);
        write_text(external_audit_path, "");
        std::error_code hard_link_error;
        fs::create_hard_link(external_audit_path, hard_link_path, hard_link_error);
        if (!hard_link_error) {
            write_denial_manifest(case_root, case_root.string(), "hard-linked.log");
            const auto process = run_process_capture(
                runtime_host_path,
                {"--manifest", (case_root / "app.cfmanifest").string()},
                temp_root);

            expect(process.exit_code == 8,
                   "#4015: a package-local hard link should fail the audit containment contract");
            expect(process.stdout_text.find(
                       "error: Package path failed physical containment validation: hard-linked.log") !=
                       std::string::npos,
                   "#4015: hard-linked audit rejection should retain the localized containment diagnostic");
            expect(read_text(external_audit_path).empty(),
                   "#4015: rejecting a hard-linked audit leaf must leave its external identity unchanged");
        }
    }

    {
        const fs::path case_root = packages_root / "ambiguous_rebind";
        const fs::path recorded_root = temp_root / "builder" / "package";
        const fs::path ambiguous_sentinel = case_root / "ambiguous.log";
        const fs::path exact_rebound_audit_path = case_root / "logs" / "ambiguous.log";
        fs::create_directories(case_root);
        write_text(ambiguous_sentinel, "");
        write_denial_manifest(
            case_root,
            recorded_root.string(),
            (recorded_root / "logs" / "ambiguous.log").string());

        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", (case_root / "app.cfmanifest").string()},
            temp_root);

        expect(process.exit_code == 7,
               "#4015: exact audit-path rebinding should preserve the policy-denial exit code");
        expect(read_text(ambiguous_sentinel).empty(),
               "#4015: exact audit-path rebinding must not append to a same-named package-root fallback");
        const auto audit_chain = copperfin::security::verify_immutable_audit_chain(
            exact_rebound_audit_path.string());
        expect(audit_chain.ok && audit_chain.entries == 1U,
               "#4015: exact audit-path rebinding should create the recorded nested package path");
        expect(!fs::exists(case_root / "security_audit.log"),
               "#4015: exact audit-path rebinding should not use the default audit leaf");
    }

    {
        const fs::path case_root = packages_root / "redirected_component";
        const fs::path redirected_directory = case_root / "audit-link";
        fs::create_directories(case_root);
        if (create_directory_indirection(external_root, redirected_directory)) {
            expect_rejected_path(
                "redirected_component",
                case_root.string(),
                "audit-link/security_audit.log",
                external_audit_path,
                external_audit_path.filename().string());
            remove_directory_indirection(redirected_directory);
        }
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_security_denial_audit_details_localize_without_changing_audit_contracts(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_security_denial_audit_localization";
    const fs::path locale_root = temp_root / "locales";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    write_runtime_host_usage_catalogs(locale_root);

    {
        const fs::path case_root = temp_root / "project_open_denied";
        const fs::path content_root = case_root / "content";
        const fs::path manifest_path = case_root / "project_open_denied.cfmanifest";
        const fs::path startup_path = content_root / "project_open_denied.prg";
        const fs::path audit_log_path = case_root / "security_audit.log";
        fs::create_directories(content_root);
        write_text(startup_path, "RETURN\n");
        const std::string manifest_text =
            std::string("manifest_version=1\n"
            "project_title=ProjectOpenDeniedLocalization\n"
            "package_root=") + case_root.string() + "\n"
            "content_root=" + content_root.string() + "\n"
            "working_directory=" + content_root.string() + "\n"
            "startup_item=project_open_denied.prg\n"
            "startup_source=" + startup_path.string() + "\n"
            "security_enabled=true\n"
            "security_role=guest\n"
            "security_mode=native\n"
            "audit_log_path=security_audit.log\n"
            "dotnet_story=none\n";
        write_text(manifest_path, manifest_text);

        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string()},
            temp_root);

        expect(process.exit_code == 7,
               "#2592: es-419 project.open denials should keep the security denial exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2592: es-419 project.open denials should preserve machine-readable status");
        expect(process.stdout_text.find(
                   "error: La politica de seguridad denego project.open para el rol 'guest'.") !=
                   std::string::npos,
               "#2592: es-419 project.open denials should localize console prose while preserving invariant ids");

        const auto audit_chain = copperfin::security::verify_immutable_audit_chain(audit_log_path.string());
        expect(audit_chain.ok && audit_chain.entries == 1U,
               "#2592: es-419 project.open denials should preserve the immutable audit chain format");
        const std::string audit_text = read_text(audit_log_path);
        expect(audit_text.find("|policy.denied|") != std::string::npos,
               "#2592: es-419 project.open denials should preserve the policy.denied audit event name");
        expect(audit_text.find("La politica de seguridad denego project.open para el rol 'guest'.") !=
                   std::string::npos,
               "#2592: es-419 project.open denials should localize audit detail prose");
        expect(audit_text.find("role missing permission") == std::string::npos,
               "#2592: es-419 project.open denials should not leave raw English audit detail wrappers");
    }

    {
        const fs::path deployed_root = temp_root / "runtime_admin_denied";
        const fs::path content_root = deployed_root / "content";
        const fs::path deployed_runtime_host = deployed_runtime_host_path(deployed_root, runtime_host_path);
        const fs::path manifest_path = deployed_root / "app.cfmanifest";
        const fs::path startup_path = content_root / "main.prg";
        const fs::path audit_log_path = deployed_root / "security_audit.log";
        fs::create_directories(content_root);
        fs::copy_file(runtime_host_path, deployed_runtime_host, fs::copy_options::overwrite_existing);
#if defined(__unix__) || defined(__APPLE__)
        fs::permissions(
            deployed_runtime_host,
            fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
            fs::perm_options::add,
            ignored);
#endif

        const auto runtime_host_hash = copperfin::security::sha256_hex_for_file(deployed_runtime_host.string());
        expect(runtime_host_hash.ok, "#2592: runtime-admin denial fixture should hash the deployed runtime host");
        if (!runtime_host_hash.ok) {
            fs::remove_all(temp_root, ignored);
            return;
        }

        write_text(startup_path, "RETURN\n");
        const auto startup_hash = copperfin::security::sha256_hex_for_file(startup_path.string());
        expect(startup_hash.ok, "#2592: runtime-admin denial fixture should hash the startup asset");
        if (!startup_hash.ok) {
            fs::remove_all(temp_root, ignored);
            return;
        }
        const std::string manifest_text =
            std::string("manifest_version=1\n"
            "project_title=RuntimeAdminDeniedLocalization\n"
            "project_path=") + (deployed_root / "demo.pjx").string() + "\n"
            "package_root=" + deployed_root.string() + "\n"
            "content_root=" + content_root.string() + "\n"
            "working_directory=" + content_root.string() + "\n"
            "startup_item=main.prg\n"
            "startup_source=" + startup_path.string() + "\n"
            "configuration=debug\n"
            "security_enabled=true\n"
            "security_role=developer\n"
            "security_mode=native\n"
            "audit_log_path=" + audit_log_path.string() + "\n"
            "runtime_host_sha256=" + runtime_host_hash.hex_digest + "\n"
            "asset=1|main.prg|" + startup_path.string() + "|Program|false|true|" +
                startup_hash.hex_digest + "|true\n"
            "dotnet_story=none\n";
        write_text(manifest_path, manifest_text);

        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
        const auto process = run_process_capture(
            deployed_runtime_host.string(),
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "continue"
            },
            temp_root);

        expect(process.exit_code == 9,
               "#2592: pt-BR runtime.admin denials should keep the debug-command security denial exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2592: pt-BR runtime.admin denials should preserve machine-readable status");
        expect(process.stdout_text.find(
                   "erro: A politica de seguranca negou runtime.admin para a funcao 'developer'.") !=
                   std::string::npos,
               "#2592: pt-BR runtime.admin denials should localize console prose while preserving invariant ids");

        const auto audit_chain = copperfin::security::verify_immutable_audit_chain(audit_log_path.string());
        expect(audit_chain.ok && audit_chain.entries >= 1U,
               "#2592: pt-BR runtime.admin denials should preserve the immutable audit chain format");
        const std::string audit_text = read_text(audit_log_path);
        expect(audit_text.find("|policy.denied|") != std::string::npos,
               "#2592: pt-BR runtime.admin denials should preserve the policy.denied audit event name");
        expect(audit_text.find("A politica de seguranca negou runtime.admin para a funcao 'developer'.") !=
                   std::string::npos,
               "#2592: pt-BR runtime.admin denials should localize audit detail prose");
        expect(audit_text.find("role missing permission") == std::string::npos,
               "#2592: pt-BR runtime.admin denials should not leave raw English audit detail wrappers");
    }

    {
        const fs::path case_root = temp_root / "manifest_hash_denied";
        const fs::path content_root = case_root / "content";
        const fs::path manifest_path = case_root / "manifest_hash_denied.cfmanifest";
        const fs::path startup_path = content_root / "manifest_hash_denied.prg";
        const fs::path audit_log_path = case_root / "security_audit.log";
        fs::create_directories(content_root);
        write_text(startup_path, "RETURN\n");
        const std::string manifest_text =
            std::string("manifest_version=1\n"
            "project_title=ManifestHashDeniedLocalization\n"
            "package_root=") + case_root.string() + "\n"
            "content_root=" + content_root.string() + "\n"
            "working_directory=" + content_root.string() + "\n"
            "startup_item=manifest_hash_denied.prg\n"
            "startup_source=" + startup_path.string() + "\n"
            "security_enabled=true\n"
            "security_role=developer\n"
            "security_mode=native\n"
            "audit_log_path=security_audit.log\n"
            "dotnet_story=none\n";
        write_text(manifest_path, manifest_text);

        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string()},
            temp_root);

        expect(process.exit_code == 8,
               "#2592: es-419 manifest-hash denials should keep the manifest verification exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2592: es-419 manifest-hash denials should preserve machine-readable status");
        expect(process.stdout_text.find(
                   "error: Al manifiesto con seguridad habilitada le falta runtime_host_sha256.") !=
                   std::string::npos,
               "#2592: es-419 manifest-hash denials should localize console verification prose");

        const auto audit_chain = copperfin::security::verify_immutable_audit_chain(audit_log_path.string());
        expect(audit_chain.ok && audit_chain.entries == 1U,
               "#2592: es-419 manifest-hash denials should preserve the immutable audit chain format");
        const std::string audit_text = read_text(audit_log_path);
        expect(audit_text.find("|policy.denied|") != std::string::npos,
               "#2592: es-419 manifest-hash denials should preserve the policy.denied audit event name");
        expect(audit_text.find("Al manifiesto con seguridad habilitada le falta runtime_host_sha256.") !=
                   std::string::npos,
               "#2592: es-419 manifest-hash denials should localize audit detail prose");
        expect(audit_text.find("hash verification failed") == std::string::npos,
               "#2592: es-419 manifest-hash denials should not leave the raw English hash-verification wrapper");
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_validates_manifest_versions_without_changing_error_contracts(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_manifest_version_contracts";
    const fs::path locale_root = temp_root / "locales";
    const fs::path startup_path = temp_root / "main.prg";
    const fs::path supported_v3_manifest_path = temp_root / "supported_v3.cfmanifest";
    const fs::path supported_v2_manifest_path = temp_root / "supported_v2.cfmanifest";
    const fs::path missing_manifest_path = temp_root / "missing_version.cfmanifest";
    const fs::path unsupported_manifest_path = temp_root / "unsupported_version.cfmanifest";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_runtime_host_usage_catalogs(locale_root);
    write_text(startup_path, "RETURN\n");

    const std::string base_manifest =
        "project_title=ManifestVersionContract\n"
        "project_path=" + (temp_root / "demo.pjx").string() + "\n"
        "package_root=" + temp_root.string() + "\n"
        "content_root=" + temp_root.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "startup_item=main.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "configuration=debug\n"
        "security_enabled=false\n"
        "security_role=developer\n"
        "security_mode=off\n"
        "dotnet_story=none\n";
    write_text(supported_v3_manifest_path, "manifest_version=3\ndata_policy=package_writable\n" + base_manifest);
    write_text(supported_v2_manifest_path, "manifest_version=2\n" + base_manifest);
    write_text(missing_manifest_path, base_manifest);
    write_text(unsupported_manifest_path, "manifest_version=99\n" + base_manifest);

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", supported_v3_manifest_path.string()},
            temp_root);
        expect(process.exit_code == 0,
               "runtime host should accept supported manifest_version=3 package contracts");
        expect(process.stdout_text.find("status: ok") != std::string::npos,
               "runtime host should preserve machine-readable success status for supported manifest versions");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", supported_v2_manifest_path.string()},
            temp_root);
        expect(process.exit_code == 0,
               "runtime host should retain legacy manifest_version=2 package compatibility");
        expect(process.stdout_text.find("status: ok") != std::string::npos,
               "legacy manifest compatibility should preserve machine-readable success status");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", missing_manifest_path.string()},
            temp_root);
        expect(process.exit_code == 4,
               "runtime host should reject manifests that omit manifest_version");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "runtime host missing-manifest-version failures should preserve machine-readable status");
        expect(process.stdout_text.find("error: Manifest is missing manifest_version.") != std::string::npos,
               "runtime host should report a localized missing-manifest-version error");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", unsupported_manifest_path.string()},
            temp_root);
        expect(process.exit_code == 4,
               "runtime host should reject unsupported manifest versions");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "runtime host unsupported-manifest-version failures should preserve machine-readable status");
        expect(process.stdout_text.find("error: manifest_version no es compatible: 99. Las versiones compatibles son: 1, 2, 3.") != std::string::npos,
               "runtime host should localize unsupported-manifest-version errors while preserving the rejected value");
        expect(process.stdout_text.find("Unsupported manifest_version: 99. Supported versions: 1, 2, 3.") == std::string::npos,
               "runtime host unsupported-manifest-version localization should not fall back to raw English prose");
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_debug_privileges_require_debug_document_contract(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_runtime_host_debug_document_contract";
    const fs::path deployed_root = temp_root / "deployed";
    const fs::path content_root = deployed_root / "content";
    const fs::path external_root = temp_root / "external";
    const fs::path external_prg = external_root / "external.prg";
    const fs::path external_form = external_root / "external.scx";
    const fs::path execution_marker = external_root / "executed.txt";
    const fs::path packaged_decoy = content_root / "decoy.prg";
    const fs::path locale_root = temp_root / "locales";
    const fs::path deployed_runtime_host =
        deployed_runtime_host_path(deployed_root, runtime_host_path);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(content_root);
    fs::create_directories(external_root);
    write_runtime_host_usage_catalogs(locale_root);
    write_text(
        external_prg,
        "LOCAL nWritten\n"
        "nWritten = STRTOFILE('executed', '" + execution_marker.generic_string() + "')\n"
        "RETURN\n");
    write_synthetic_form_asset(external_form);
    write_text(packaged_decoy, "RETURN\n");

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
    const auto decoy_hash =
        copperfin::security::sha256_hex_for_file(packaged_decoy.string());
    expect(runtime_host_hash.ok && decoy_hash.ok,
           "#3987: debug-contract fixture should hash its deployed host and packaged asset");
    if (!runtime_host_hash.ok || !decoy_hash.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const auto base_manifest = [&](const std::string& version_header,
                                   const std::filesystem::path& startup_path,
                                   const bool security_enabled) {
        std::string text =
            version_header +
            "project_title=DebugDocumentContract\n"
            "package_root=" + quote_manifest_value(deployed_root.string()) + "\n"
            "content_root=" + quote_manifest_value(content_root.string()) + "\n"
            "working_directory=" + quote_manifest_value(external_root.string()) + "\n"
            "startup_item=" + startup_path.filename().string() + "\n"
            "startup_source=" + quote_manifest_value(startup_path.string()) + "\n"
            "security_enabled=" + std::string(security_enabled ? "true" : "false") + "\n"
            "security_role=" + std::string(security_enabled ? "runtime-operator" : "") + "\n"
            "security_mode=native\n";
        if (security_enabled) {
            text +=
                "runtime_host_sha256=" + runtime_host_hash.hex_digest + "\n"
                "asset=1|decoy.prg|" + quote_manifest_value(packaged_decoy.string()) +
                    "|Program|false|true|" + decoy_hash.hex_digest + "|true\n";
        }
        text += "dotnet_story=none\n";
        return text;
    };

    const fs::path renamed_prg_manifest = deployed_root / "renamed_prg.cfdebug";
    write_text(
        renamed_prg_manifest,
        base_manifest("manifest_version=1\n", external_prg, false));
    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", renamed_prg_manifest.string(), "--debug"},
            deployed_root);
        expect(process.exit_code == 4,
               "#3987: a normal manifest renamed to .cfdebug must reject an external PRG");
        expect(process.stdout_text.find("status: error") != std::string::npos &&
                   process.stdout_text.find("status: ok") == std::string::npos,
               "#3987: renamed normal PRG rejection should preserve machine status fields");
        expect(!fs::exists(execution_marker),
               "#3987: a renamed normal manifest must not execute its external PRG");
    }

    const fs::path renamed_xasset_manifest = deployed_root / "renamed_xasset.cfdebug";
    write_text(
        renamed_xasset_manifest,
        base_manifest("manifest_version=1\n", external_form, false));
    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", renamed_xasset_manifest.string(), "--debug"},
            deployed_root);
        expect(process.exit_code == 4,
               "#3987: a normal manifest renamed to .cfdebug must reject an external xAsset");
        expect(process.stdout_text.find("runtime.mode: xasset-bootstrap") == std::string::npos,
               "#3987: renamed normal xAsset rejection must happen before bootstrap execution");
    }

    const fs::path secure_renamed_manifest = deployed_root / "secure_renamed.cfdebug";
    write_text(
        secure_renamed_manifest,
        base_manifest("manifest_version=1\n", external_prg, true));
    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", secure_renamed_manifest.string(), "--debug"},
            deployed_root);
        expect(process.exit_code == 4,
               "#3987: a security-enabled normal manifest must retain external-source containment after rename");
        expect(!fs::exists(execution_marker),
               "#3987: renamed security manifests must not bypass verified-source enforcement");
    }

    const fs::path header_swapped_manifest = deployed_root / "app.cfmanifest";
    write_text(
        header_swapped_manifest,
        base_manifest("debug_manifest_version=1\n", external_prg, true));
    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", header_swapped_manifest.string(), "--debug"},
            deployed_root);
        expect(process.exit_code == 4,
               "#3987: replacing a normal header with debug_manifest_version must not grant debug trust");
        expect(!fs::exists(execution_marker),
               "#3987: a debug header in app.cfmanifest must retain verified-source enforcement");
    }

    const fs::path ambiguous_manifest = deployed_root / "ambiguous.cfdebug";
    write_text(
        ambiguous_manifest,
        base_manifest(
            "manifest_version=1\ndebug_manifest_version=1\n",
            external_prg,
            false));
    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", ambiguous_manifest.string(), "--debug"},
            deployed_root);
        expect(process.exit_code == 4,
               "#3987: mixed normal/debug version headers should fail before execution");
        expect(process.stdout_text.find(
                   "error: El manifiesto debe contener exactamente uno de manifest_version o "
                   "debug_manifest_version.") != std::string::npos,
               "#3987: mixed version-header rejection should use localized prose");
        expect(process.stdout_text.find(
                   "Manifest must contain exactly one of manifest_version or debug_manifest_version.") ==
                   std::string::npos,
               "#3987: localized mixed-header rejection should not fall back to English");
        expect(!fs::exists(execution_marker),
               "#3987: ambiguous manifest contracts must not execute external sources");
    }

    const fs::path duplicate_version_manifest = deployed_root / "duplicate_version.cfdebug";
    write_text(
        duplicate_version_manifest,
        base_manifest(
            "debug_manifest_version=1\ndebug_manifest_version=1\n",
            external_prg,
            false));
    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", duplicate_version_manifest.string(), "--debug"},
            deployed_root);
        expect(process.exit_code == 4,
               "#3987: duplicate version headers should fail before execution");
        expect(process.stdout_text.find(
                   "error: Manifest must contain exactly one of manifest_version or "
                   "debug_manifest_version.") != std::string::npos,
               "#3987: duplicate version-header rejection should use the catalog diagnostic");
        expect(!fs::exists(execution_marker),
               "#3987: duplicate version headers must not execute external sources");
    }

    const fs::path genuine_debug_manifest = deployed_root / "app.cfdebug";
    write_text(
        genuine_debug_manifest,
        base_manifest("debug_manifest_version=1\n", external_prg, false));
    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", genuine_debug_manifest.string()},
            deployed_root);
        expect(process.exit_code == 4,
               "#3987: a debug document should not grant external-source trust without --debug");
        expect(!fs::exists(execution_marker),
               "#3987: explicit debug launch authorization should be required before external execution");
    }
    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", genuine_debug_manifest.string(), "--debug"},
            deployed_root);
        expect(process.exit_code == 0,
               "#3987: an explicitly launched genuine debug document should retain external PRG compatibility");
        expect(process.stdout_text.find("status: ok") != std::string::npos,
               "#3987: genuine debug execution should preserve machine success status");
        expect(fs::exists(execution_marker) && read_text(execution_marker) == "executed",
               "#3987: debug_manifest_version should remain the external-source trust contract");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_host_rejects_ai_federation_planning_without_ai_permission(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_federation_ai_permission_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--federation-backend", "oracle",
            "--federation-query", "DELETE FROM customer",
            "--federation-planning-enable", "true"
        },
        temp_root);

    if (process.exit_code == 0) {
        std::cerr << "federation-ai-permission stdout:\n" << process.stdout_text << "\n";
        std::cerr << "federation-ai-permission stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 7,
           "runtime host should deny AI-assisted federation planning when the effective role lacks ai.mcp");
    expect(process.stdout_text.find("runtime.mode: federation-query-plan") != std::string::npos,
           "runtime host should keep the federation runtime mode visible on AI permission denials");
    expect(process.stdout_text.find("error: Security policy denied ai.mcp for role 'developer'.") != std::string::npos,
           "runtime host should report the missing ai.mcp permission for the default developer role");

    {
        ScopedEnvironmentValue allow_ai_role("COPPERFIN_SECURITY_ROLE", "runtime-operator");
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto allowed_process = run_process_capture(
            runtime_host_path,
            {
                "--federation-backend", "oracle",
                "--federation-query", "DELETE FROM customer",
                "--federation-planning-enable", "true"
            },
            temp_root);

        expect(allowed_process.exit_code == 6,
               "#2593: es-419 runtime host should advance past AI permission gating for runtime-operator and reach planner fallback");
        expect(allowed_process.stdout_text.find(
                   "error: El planner aun no esta implementado para la politica de IA optional. La traduccion deterministica fallo: "
                   "Solo se admite la traduccion SQL deterministica de primera pasada de SELECT...FROM.") !=
                   std::string::npos,
               "#2594: es-419 runtime host should localize both the planner-fallback wrapper and translator payload once AI permission is granted");
        expect(allowed_process.stdout_text.find("runtime.mode: federation-query-plan") != std::string::npos,
               "#2593: es-419 runtime host should preserve the federation runtime mode during planner fallback");
        expect(allowed_process.stdout_text.find("Platform.QueryTranslator.Error.SelectFromOnly") == std::string::npos,
               "#2594: es-419 runtime host should not leak the unresolved translator diagnostic key");
        expect(allowed_process.stdout_text.find("Planner is not yet implemented for optional AI policy.") == std::string::npos,
               "#2593: es-419 runtime host should not fall back to raw English planner-fallback prose");
    }

    {
        ScopedEnvironmentValue allow_ai_role("COPPERFIN_SECURITY_ROLE", "runtime-operator");
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        const std::string pseudo_translation_error = copperfin::localization::pseudo_localize(
            "Only first-pass SELECT...FROM SQL translation is supported.");
        const auto allowed_process = run_process_capture(
            runtime_host_path,
            {
                "--federation-backend", "oracle",
                "--federation-query", "DELETE FROM customer",
                "--federation-planning-enable", "true"
            },
            temp_root);

        expect(allowed_process.exit_code == 6,
               "#2593: qps-ploc runtime host should keep the planner-fallback exit code after AI permission is granted");
        expect(allowed_process.stdout_text.find("runtime.mode: federation-query-plan") != std::string::npos,
               "#2593: qps-ploc runtime host should preserve the federation runtime mode during planner fallback");
        expect(allowed_process.stdout_text.find("[!! ërrør:  !!][!! ") != std::string::npos,
               "#2593: qps-ploc runtime host should pseudo-localize the planner-fallback prose");
        expect(allowed_process.stdout_text.find("Platform.QueryTranslator.Error.SelectFromOnly") ==
                   std::string::npos,
               "#2594: qps-ploc runtime host should not leak the unresolved translator diagnostic key");
        expect(allowed_process.stdout_text.find("Only first-pass SELECT...FROM SQL translation is supported.") ==
                   std::string::npos,
               "#2594: qps-ploc runtime host should pseudo-localize the deterministic translator payload prose");
        expect(allowed_process.stdout_text.find(pseudo_translation_error) != std::string::npos,
               "#2594: qps-ploc runtime host should surface the pseudo-localized deterministic translator payload");
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
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
    run_runtime_host_test("verified report and label snapshots", runtime_host_path, test_security_enabled_report_and_label_execute_verified_snapshots);
    run_runtime_host_test("verified xAsset companions", runtime_host_path, test_security_enabled_form_class_and_menu_companion_integrity);
    run_runtime_host_test("nested directory alias identity", runtime_host_path, test_runtime_host_preserves_logical_identity_across_nested_directory_aliases);
    run_runtime_host_test("external xAsset debug source", runtime_host_path, test_app_cfdebug_preserves_external_xasset_source_compatibility);
    run_runtime_host_test("file-valued working directory", runtime_host_path, test_app_cfdebug_rejects_file_valued_working_directory);
    run_runtime_host_test("writable package data", runtime_host_path, test_security_enabled_writable_package_data_contract);
    run_runtime_host_test("extension payload basename fallback", runtime_host_path, test_runtime_host_rejects_extension_payload_basename_fallback);
    run_runtime_host_test("manifest versions", runtime_host_path, test_runtime_host_validates_manifest_versions_without_changing_error_contracts);
    run_runtime_host_test("debug privilege document", runtime_host_path, test_runtime_host_debug_privileges_require_debug_document_contract);
    run_runtime_host_test("manifest verification localization", runtime_host_path, test_runtime_host_manifest_verification_errors_localize_without_changing_contracts);
    run_runtime_host_test("audit package boundary", runtime_host_path, test_runtime_host_rejects_audit_paths_outside_the_direct_package);
    run_runtime_host_test("security audit localization", runtime_host_path, test_runtime_host_security_denial_audit_details_localize_without_changing_audit_contracts);
    run_runtime_host_test("AI federation permission", runtime_host_path, test_runtime_host_rejects_ai_federation_planning_without_ai_permission);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }

    std::cout << "All runtime host audit-stream tests passed\n";
    return 0;
}
