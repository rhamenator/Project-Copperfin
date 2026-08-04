// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_runtime_host_debug_output_support.h"

// Safety-relevant coverage: these tests exercise immutable audit-chain and integrity contracts.

void test_security_enabled_writable_package_data_contract(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;
    const int failures_before_test = failures;
    const fs::path temp_root = runtime_host_audit_temp_root("copperfin_runtime_host_writable_package_data");
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
    const fs::path hard_link_target = content_root / "immutable-alias.dbf";
    fs::copy_file(table_path, hard_link_target, fs::copy_options::overwrite_existing, ignored);
    fs::remove(table_path, ignored);
    std::error_code hard_link_error;
    fs::create_hard_link(hard_link_target, table_path, hard_link_error);
    if (!hard_link_error) {
        const auto hard_link_process = run_process_capture(
            deployed_runtime_host.string(),
            {"--manifest", manifest_path.string()},
            deployed_root);
        expect(hard_link_process.exit_code == 8,
               "security verification should reject hard-linked package-writable DBF data");
        expect(hard_link_process.stdout_text.find(
                   "error: Package path failed physical containment validation: customers.dbf") !=
                   std::string::npos,
               "hard-linked package-writable DBF data should retain the physical-containment diagnostic");
    }
    fs::remove(table_path, ignored);
    fs::copy_file(hard_link_target, table_path, fs::copy_options::overwrite_existing, ignored);
    fs::remove(hard_link_target, ignored);

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

    const fs::path temp_root = runtime_host_audit_temp_root("copperfin_runtime_host_payload_path_fidelity");
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

void test_runtime_host_accepts_escaped_manifest_pipe_fields(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = runtime_host_audit_temp_root("copperfin_runtime_host_escaped_manifest_pipe");
    const fs::path builder_root = temp_root / "builder" / "PipeFieldApp";
    const fs::path builder_content_root = builder_root / "content";
    const fs::path deployed_root = temp_root / "deployed";
    const fs::path deployed_content_root = deployed_root / "content";
    const fs::path startup_path = deployed_content_root / "main.prg";
    const fs::path manifest_path = deployed_root / "app.cfmanifest";
    const fs::path locale_root = temp_root / "locales";
    const fs::path deployed_runtime_host = deployed_runtime_host_path(deployed_root, runtime_host_path);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(builder_content_root);
    fs::create_directories(deployed_content_root);
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
    const auto runtime_host_hash = copperfin::security::sha256_hex_for_file(deployed_runtime_host.string());
    const auto startup_hash = copperfin::security::sha256_hex_for_file(startup_path.string());
    expect(runtime_host_hash.ok && startup_hash.ok,
           "escaped manifest pipe fixture should hash the runtime host and startup asset");
    if (!runtime_host_hash.ok || !startup_hash.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=EscapedManifestPipe\n"
        "project_path=" + (builder_root / "pipefield.pjx").string() + "\n"
        "package_root=" + builder_root.string() + "\n"
        "content_root=" + builder_content_root.string() + "\n"
        "working_directory=" + builder_content_root.string() + "\n"
        "startup_item=main.prg\n"
        "startup_source=" + (builder_content_root / "main.prg").string() + "\n"
        "configuration=debug\n"
        "security_enabled=true\n"
        "security_role=developer\n"
        "security_mode=native\n"
        "runtime_host_sha256=" + runtime_host_hash.hex_digest + "\n"
        "asset=1|main.prg|" + (builder_content_root / "main.prg").string() +
            "|Program\\|Preview|false|true|" + startup_hash.hex_digest + "|true\n"
        "dotnet_story=none\n");

    ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
    const auto process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", manifest_path.string()},
        deployed_root);
    if (process.exit_code != 0) {
        std::cerr << "escaped manifest pipe stdout:\n" << process.stdout_text << "\n";
        std::cerr << "escaped manifest pipe stderr:\n" << process.stderr_text << "\n";
    }
    expect(process.exit_code == 0,
           "runtime host should accept an asset metadata field containing an escaped pipe");
    expect(process.stdout_text.find("status: ok") != std::string::npos,
           "escaped manifest pipe success should preserve the machine-readable status contract");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_preserves_escaped_pipe_in_direct_manifest_paths(const std::string& runtime_host_path) {
#if defined(_WIN32)
    (void)runtime_host_path;
    return;
#else
    namespace fs = std::filesystem;

    const fs::path temp_root = runtime_host_audit_temp_root("copperfin_runtime_host_direct_manifest_pipe");
    const fs::path deployed_root = temp_root / "deployed|package\\literal";
    const fs::path content_root = deployed_root / "content|root\\literal";
    const fs::path startup_path = content_root / "main.prg";
    const fs::path manifest_path = deployed_root / "app.cfmanifest";
    const fs::path locale_root = temp_root / "locales";
    const fs::path deployed_runtime_host = deployed_runtime_host_path(deployed_root, runtime_host_path);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(content_root);
    fs::copy_file(runtime_host_path, deployed_runtime_host, fs::copy_options::overwrite_existing);
    fs::permissions(
        deployed_runtime_host,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add,
        ignored);

    write_runtime_host_usage_catalogs(locale_root);
    write_text(startup_path, "RETURN\n");
    const auto runtime_host_hash = copperfin::security::sha256_hex_for_file(deployed_runtime_host.string());
    const auto startup_hash = copperfin::security::sha256_hex_for_file(startup_path.string());
    expect(runtime_host_hash.ok && startup_hash.ok,
           "direct escaped-pipe fixture should hash the runtime host and startup asset");
    if (!runtime_host_hash.ok || !startup_hash.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const auto quote_manifest_path = [](std::string value) {
        std::string escaped;
        escaped.reserve(value.size());
        for (const char ch : value) {
            if (ch == '\\') {
                escaped += "\\\\";
            } else if (ch == '|') {
                escaped += "\\|";
            } else {
                escaped.push_back(ch);
            }
        }
        return escaped;
    };
    const std::string quoted_package_root = quote_manifest_path(deployed_root.string());
    const std::string quoted_content_root = quote_manifest_path(content_root.string());
    const std::string quoted_startup_path = quote_manifest_path(startup_path.string());
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "manifest_value_encoding=backslash-v1\n"
        "project_title=DirectEscapedManifestPipe\n"
        "project_path=" + quote_manifest_path((temp_root / "demo.pjx").string()) + "\n"
        "package_root=" + quoted_package_root + "\n"
        "content_root=" + quoted_content_root + "\n"
        "working_directory=" + quoted_content_root + "\n"
        "startup_item=main.prg\n"
        "startup_source=" + quoted_startup_path + "\n"
        "configuration=debug\n"
        "security_enabled=true\n"
        "security_role=developer\n"
        "security_mode=native\n"
        "runtime_host_sha256=" + runtime_host_hash.hex_digest + "\n"
        "asset=1|main.prg|" + quoted_startup_path +
            "|Program\\|Preview|false|true|" + startup_hash.hex_digest + "|true\n"
        "dotnet_story=none\n");

    const std::string manifest_text = read_text(manifest_path);
    expect(manifest_text.find("deployed\\|package\\\\literal") != std::string::npos,
           "direct escaped-pipe fixture should write a backslash-escaped package path");
    expect(manifest_text.find("content\\|root\\\\literal") != std::string::npos,
           "direct escaped-pipe fixture should write a backslash-escaped content path");

    ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
    const auto process = run_process_capture(
        deployed_runtime_host.string(),
        {"--manifest", manifest_path.string()},
        deployed_root);
    if (process.exit_code != 0) {
        std::cerr << "direct escaped manifest pipe stdout:\n" << process.stdout_text << "\n";
        std::cerr << "direct escaped manifest pipe stderr:\n" << process.stderr_text << "\n";
    }
    expect(process.exit_code == 0,
           "runtime host should resolve direct manifest paths containing an escaped pipe");
    expect(process.stdout_text.find("status: ok") != std::string::npos,
           "direct escaped manifest pipe success should preserve the machine-readable status contract");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
#endif
}

void test_runtime_host_manifest_verification_errors_localize_without_changing_contracts(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = runtime_host_audit_temp_root("copperfin_runtime_host_manifest_error_localization");
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
