// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_runtime_host_debug_output_support.h"

// Safety-relevant coverage: these tests exercise immutable audit-chain and integrity contracts.

void test_runtime_host_validates_manifest_versions_without_changing_error_contracts(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = runtime_host_audit_temp_root("copperfin_runtime_host_manifest_version_contracts");
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
        runtime_host_audit_temp_root("copperfin_runtime_host_debug_document_contract");
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

    const fs::path temp_root = runtime_host_audit_temp_root("copperfin_runtime_host_federation_ai_permission_tests");
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
