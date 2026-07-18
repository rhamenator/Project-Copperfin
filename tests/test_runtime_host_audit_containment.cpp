// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_runtime_host_debug_output_support.h"

// Safety-relevant coverage: these tests exercise immutable audit-chain and integrity contracts.

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
