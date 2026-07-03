// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/licensing/license_status.h"
#include "copperfin/platform/extensibility_model.h"
#include "copperfin/localization/localization.h"
#include "copperfin/runtime/runtime_pipeline.h"
#include "copperfin/security/audit_stream.h"
#include "copperfin/security/authorization.h"
#include "copperfin/security/external_process_policy.h"
#include "copperfin/security/process_hardening.h"
#include "copperfin/security/secret_provider.h"
#include "copperfin/security/security_model.h"
#include "copperfin/studio/document_model.h"
#include "copperfin/studio/project_workspace.h"

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#if defined(_WIN32)
#include <process.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif
#include <string>
#include <system_error>
#include <vector>

namespace {

std::string explicit_locale_from_arguments(int argc, char** argv) {
    for (int index = 1; index + 1 < argc; ++index) {
        if (std::string(argv[index]) == "--locale") {
            return argv[index + 1];
        }
    }
    return {};
}

copperfin::localization::LocalizedCatalog load_localization(
    const char* executable_path,
    const std::string& explicit_locale) {
    const std::filesystem::path locale_root = copperfin::localization::resolve_catalog_root(executable_path);
    return copperfin::localization::load_catalogs(
        locale_root,
        copperfin::localization::select_locale(explicit_locale));
}

void print_usage(const copperfin::localization::LocalizedCatalog& catalog) {
    std::cout << catalog.translate(
        "BuildHost.Usage",
        {
            {"buildCommand", "build"},
            {"commandName", "copperfin_build_host"},
            {"configurationOption", "--configuration"},
            {"configurationValue", "debug|release"},
            {"emitDotnetLauncherOption", "--emit-dotnet-launcher"},
            {"enableSecurityOption", "--enable-security"},
            {"outputDirOption", "--output-dir"},
            {"outputDirValue", "<directory>"},
            {"projectOption", "--project"},
            {"projectValue", "<path-to-pjx>"},
            {"runtimeHostOption", "--runtime-host"},
            {"runtimeHostValue", "<path>"}
        }) << "\n";
    std::cout << catalog.translate(
        "BuildHost.Usage.LicenseStatus",
        {
            {"commandName", "copperfin_build_host"},
            {"licenseStatusOption", "--license-status"}
        }) << "\n";
}

std::string message(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& key,
    const copperfin::localization::PlaceholderMap& placeholders = {}) {
    return catalog.translate(key, placeholders);
}

void print_license_status(const copperfin::licensing::LicenseStatus& status) {
    using copperfin::licensing::LicenseState;

    std::cout << "status: ok\n";
    std::cout << "state: " << copperfin::licensing::license_state_name(status.state) << "\n";
    if (status.state == LicenseState::free) {
        return;
    }

    if (!status.license_id.empty()) {
        std::cout << "license_id: " << status.license_id << "\n";
    }
    if (!status.license_type.empty()) {
        std::cout << "license_type: " << status.license_type << "\n";
    }
    if (!status.pricing_model.empty()) {
        std::cout << "pricing_model: " << status.pricing_model << "\n";
    }
    if (!status.licensee_name.empty()) {
        std::cout << "licensee_name: " << status.licensee_name << "\n";
    }
    if (!status.licensee_email.empty()) {
        std::cout << "licensee_email: " << status.licensee_email << "\n";
    }
    if (status.seats > 0) {
        std::cout << "seats: " << status.seats << "\n";
    }
    if (!status.issued_date.empty()) {
        std::cout << "issued_date: " << status.issued_date << "\n";
    }
    if (!status.subscription_expires.empty()) {
        std::cout << "subscription_expires: " << status.subscription_expires << "\n";
    }
    if (status.perpetual_max_major_version > 0) {
        std::cout << "perpetual_max_major_version: " << status.perpetual_max_major_version << "\n";
    }
    if (!status.source_path.empty()) {
        std::cout << "source_path: " << status.source_path << "\n";
    }
    if (!status.diagnostic.empty()) {
        std::cout << "diagnostic: " << status.diagnostic << "\n";
    }
}

void print_error_line(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& error) {
    std::cout << message(catalog, "BuildHost.Prefix.Error") << error << "\n";
}

void print_warning_line(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& warning) {
    std::cout << message(catalog, "BuildHost.Prefix.Warning") << warning << "\n";
}

std::string environment_value(const char* name) {
#ifdef _WIN32
    char* raw = nullptr;
    std::size_t length = 0;
    if (_dupenv_s(&raw, &length, name) != 0 || raw == nullptr) {
        return {};
    }
    std::string value(raw);
    std::free(raw);
    return value;
#else
    if (const char* raw = std::getenv(name); raw != nullptr) {
        return raw;
    }
    return {};
#endif
}

std::string resolve_runtime_host_path(const std::string& override_path, const std::string& executable_path) {
    if (!override_path.empty()) {
        return override_path;
    }

    const std::string resolved = environment_value("COPPERFIN_RUNTIME_HOST_PATH");
    if (!resolved.empty()) {
        return resolved;
    }

    const std::filesystem::path host_root = std::filesystem::absolute(executable_path).parent_path();
    const std::filesystem::path host_name =
#ifdef _WIN32
        "copperfin_runtime_host.exe";
#else
        "copperfin_runtime_host";
#endif
    const std::vector<std::filesystem::path> candidate_paths{
        host_root / host_name,
#ifdef _WIN32
        host_root / "copperfin_runtime_host"
#else
        host_root / "copperfin_runtime_host.exe"
#endif
    };
    for (const auto& candidate : candidate_paths) {
        if (std::filesystem::exists(candidate)) {
            return candidate.string();
        }
    }

    return (host_root / host_name).string();
}

bool run_dotnet_publish(
    const copperfin::runtime::RuntimePackagePlan& plan,
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string& error) {
    const std::filesystem::path project_path(plan.launcher_project_path);
    const std::filesystem::path output_dir(plan.package_root);
    const std::string configuration = plan.configuration == copperfin::runtime::BuildConfiguration::release ? "Release" : "Debug";

    const auto auth = copperfin::security::authorize_external_process({
        .executable_name = "dotnet.exe",
        .allowed_path_roots = {
            R"(C:\Program Files\dotnet)",
            R"(C:\Program Files (x86)\dotnet)"
        },
        .allowed_publishers = {"Microsoft Corporation"},
        .require_trusted_signature = true
    });
    if (!auth.allowed) {
        error = message(catalog, "BuildHost.Error.DotnetPublishDenied", {{"error", auth.error}});
        return false;
    }

    std::vector<std::string> publish_args = {
        auth.resolved_path,
        "publish",
        project_path.string(),
        "-c",
        configuration,
        "-r",
        "win-x64",
        "--self-contained",
        "false",
        "-o",
        output_dir.string()
    };

    std::vector<const char*> argv;
    argv.reserve(publish_args.size() + 1U);
    for (const auto& arg : publish_args) {
        argv.push_back(arg.c_str());
    }
    argv.push_back(nullptr);

    intptr_t exit_code = -1;
#if defined(_WIN32)
    exit_code = _spawnvp(_P_WAIT, auth.resolved_path.c_str(), const_cast<char* const*>(argv.data()));
#else
    const pid_t child = fork();
    if (child == 0) {
        execvp(auth.resolved_path.c_str(), const_cast<char* const*>(argv.data()));
        _exit(127);
    }
    if (child > 0) {
        int status = 0;
        if (waitpid(child, &status, 0) == child && WIFEXITED(status)) {
            exit_code = WEXITSTATUS(status);
        }
    }
#endif
    if (exit_code == -1) {
        error = message(
            catalog,
            "BuildHost.Error.DotnetPublishFailedToStart",
            {{"error", std::error_code(errno, std::generic_category()).message()}});
        return false;
    }

    if (exit_code != 0) {
        error = message(catalog, "BuildHost.Error.DotnetPublishFailed");
        return false;
    }

    const std::filesystem::path expected = output_dir / (project_path.stem().string() == "Copperfin.GeneratedLauncher"
        ? (std::filesystem::path(plan.launcher_output_path).filename().string())
        : (project_path.stem().string() + ".exe"));
    if (!std::filesystem::exists(expected)) {
        const std::filesystem::path generated = output_dir / "Copperfin.GeneratedLauncher.exe";
        if (std::filesystem::exists(generated)) {
            std::error_code rename_error;
            std::filesystem::rename(generated, plan.launcher_output_path, rename_error);
            if (!rename_error) {
                return true;
            }
        }

        if (!std::filesystem::exists(plan.launcher_output_path)) {
            error = message(catalog, "BuildHost.Error.GeneratedLauncherMissing");
            return false;
        }
    }

    return true;
}

bool is_library_output_kind(const copperfin::runtime::BuildOutputKind output_kind) {
    return output_kind == copperfin::runtime::BuildOutputKind::dll ||
        output_kind == copperfin::runtime::BuildOutputKind::fll ||
        output_kind == copperfin::runtime::BuildOutputKind::ocx;
}

}  // namespace

int main(int argc, char** argv) {
    const copperfin::localization::LocalizedCatalog catalog =
        load_localization(argv[0], explicit_locale_from_arguments(argc, argv));

    const auto hardening = copperfin::security::apply_default_process_hardening();
    if (!hardening.applied) {
        std::cerr << message(
            catalog,
            "BuildHost.Warning.ProcessHardening",
            {{"message", hardening.message}}) << "\n";
    }

    std::vector<std::string> args;
    for (int index = 1; index < argc; ++index) {
        if (std::string(argv[index]) == "--locale" && index + 1 < argc) {
            ++index;
            continue;
        }
        args.emplace_back(argv[index]);
    }

    const bool legacy_license_status = args.size() == 1U && args[0] == "license-status";
    const bool license_status_requested =
        std::find(args.begin(), args.end(), "--license-status") != args.end();
    if (legacy_license_status || license_status_requested) {
        print_license_status(copperfin::licensing::load_license_status(argv[0]));
        return 0;
    }

    if (args.empty() || args[0] != "build") {
        print_usage(catalog);
        return 2;
    }

    std::string project_path;
    std::string output_dir;
    std::string runtime_host_override;
    auto configuration = copperfin::runtime::BuildConfiguration::debug;
    bool enable_security = false;
    bool emit_dotnet_launcher = false;
    std::string security_role = "developer";

    for (std::size_t index = 1; index < args.size(); ++index) {
        const auto& arg = args[index];
        if (arg == "--project" && (index + 1U) < args.size()) {
            project_path = args[++index];
        } else if (arg == "--output-dir" && (index + 1U) < args.size()) {
            output_dir = args[++index];
        } else if (arg == "--configuration" && (index + 1U) < args.size()) {
            configuration = copperfin::runtime::parse_build_configuration(args[++index]);
        } else if (arg == "--enable-security") {
            enable_security = true;
        } else if (arg == "--emit-dotnet-launcher") {
            emit_dotnet_launcher = true;
        } else if (arg == "--runtime-host" && (index + 1U) < args.size()) {
            runtime_host_override = args[++index];
        } else {
            std::cout << "status: error\n";
            print_error_line(
                catalog,
                message(
                    catalog,
                    "BuildHost.Error.UnknownOrIncompleteArgument",
                    {{"argument", arg}}));
            print_usage(catalog);
            return 2;
        }
    }

    if (project_path.empty() || output_dir.empty()) {
        std::cout << "status: error\n";
        print_error_line(
            catalog,
            message(
                catalog,
                "BuildHost.Error.RequiredProjectAndOutput",
                {
                    {"outputDirOption", "--output-dir"},
                    {"projectOption", "--project"}
                }));
        print_usage(catalog);
        return 2;
    }

    const auto open_result = copperfin::studio::open_document({.path = project_path});
    if (!open_result.ok) {
        std::cout << "status: error\n";
        print_error_line(catalog, open_result.error);
        return 3;
    }

    const auto workspace = copperfin::studio::build_project_workspace(open_result.document);
    const auto security_profile = copperfin::security::default_native_security_profile();

    if (enable_security) {
        std::string role_env;
#ifdef _WIN32
        char* role_env_raw = nullptr;
        std::size_t role_env_length = 0;
        if (_dupenv_s(&role_env_raw, &role_env_length, "COPPERFIN_SECURITY_ROLE") == 0 && role_env_raw != nullptr) {
            role_env = role_env_raw;
            std::free(role_env_raw);
        }
#else
        if (const char* role_env_raw = std::getenv("COPPERFIN_SECURITY_ROLE"); role_env_raw != nullptr) {
            role_env = role_env_raw;
        }
#endif
        if (!role_env.empty()) {
            security_role = role_env;
        }

        if (!copperfin::security::role_has_permission(security_profile, security_role, "build.execute")) {
            std::cout << "status: error\n";
            print_error_line(
                catalog,
                message(
                    catalog,
                    "BuildHost.Error.SecurityPolicyDenied",
                    {
                        {"permission", "build.execute"},
                        {"role", security_role}
                    }));
            return 7;
        }

        if (configuration == copperfin::runtime::BuildConfiguration::release &&
            !copperfin::security::role_has_permission(security_profile, security_role, "build.release")) {
            std::cout << "status: error\n";
            print_error_line(
                catalog,
                message(
                    catalog,
                    "BuildHost.Error.SecurityPolicyDenied",
                    {
                        {"permission", "build.release"},
                        {"role", security_role}
                    }));
            return 7;
        }

        if (configuration == copperfin::runtime::BuildConfiguration::release) {
            std::string signing_ref;
#ifdef _WIN32
            char* signing_ref_raw = nullptr;
            std::size_t signing_ref_length = 0;
            if (_dupenv_s(&signing_ref_raw, &signing_ref_length, "COPPERFIN_RELEASE_SIGNING_KEY_REF") == 0 && signing_ref_raw != nullptr) {
                signing_ref = signing_ref_raw;
                std::free(signing_ref_raw);
            }
#else
            if (const char* signing_ref_raw = std::getenv("COPPERFIN_RELEASE_SIGNING_KEY_REF"); signing_ref_raw != nullptr) {
                signing_ref = signing_ref_raw;
            }
#endif

            if (signing_ref.empty()) {
                std::cout << "status: error\n";
                print_error_line(
                    catalog,
                    message(
                        catalog,
                        "BuildHost.Error.ReleaseSigningKeyRequired",
                        {
                            {"environmentVariable", "COPPERFIN_RELEASE_SIGNING_KEY_REF"},
                            {"environmentValue", "env:<NAME>"}
                        }));
                return 7;
            }

            const auto secret = copperfin::security::resolve_secret_reference(signing_ref);
            if (!secret.ok) {
                std::cout << "status: error\n";
                print_error_line(
                    catalog,
                    message(
                        catalog,
                        "BuildHost.Error.SigningKeyValidationFailed",
                        {{"error", secret.error}}));
                return 7;
            }
        }
    }

    const auto extensibility_profile = copperfin::platform::default_extensibility_profile();
    auto plan = copperfin::runtime::create_runtime_package_plan(
        open_result.document,
        workspace,
        security_profile,
        extensibility_profile,
        output_dir,
        configuration,
        enable_security,
        emit_dotnet_launcher);

    if (!plan.ok) {
        std::cout << "status: error\n";
        print_error_line(catalog, message(catalog, "BuildHost.Error.BuildPlanCreationFailed"));
        return 4;
    }

    // Stamped into app.cfmanifest/app.cfdebug as inert, informational
    // provenance -- never gates whether the build succeeds or what it
    // produces. See LicenseState::perpetual_out_of_version's doc comment.
    const auto license_status = copperfin::licensing::load_license_status(argv[0]);
    plan.license_state = std::string(copperfin::licensing::license_state_name(license_status.state));
    plan.license_type = license_status.license_type;
    plan.license_id = license_status.license_id;
    plan.license_licensee = license_status.licensee_name;
    plan.license_seats = license_status.seats;
    plan.license_subscription_expires = license_status.subscription_expires;
    plan.license_perpetual_max_major_version = license_status.perpetual_max_major_version;
    plan.license_source_path = license_status.source_path;

    const std::string runtime_host_path = resolve_runtime_host_path(runtime_host_override, argv[0]);
    const auto materialized = copperfin::runtime::materialize_runtime_package(
        plan,
        security_profile,
        extensibility_profile,
        runtime_host_path);
    if (!materialized.ok) {
        std::cout << "status: error\n";
        print_error_line(catalog, materialized.error);
        return 5;
    }

    auto final_plan = materialized.plan;

    if (is_library_output_kind(materialized.plan.output_kind)) {
        const auto build_result = copperfin::runtime::build_runtime_package_primary_output(
            materialized.plan,
            security_profile,
            extensibility_profile);
        if (!build_result.ok) {
            std::cout << "status: error\n";
            print_error_line(catalog, build_result.error);
            return 8;
        }
        final_plan = build_result.plan;
    }

    if (enable_security && !final_plan.audit_log_path.empty()) {
        (void)copperfin::security::append_immutable_audit_event(
            final_plan.audit_log_path,
            "build.package_materialized",
            "role=" + security_role + ",project=" + final_plan.project_title);
    }

    if (final_plan.emit_dotnet_launcher) {
        std::string publish_error;
        if (!run_dotnet_publish(final_plan, catalog, publish_error)) {
            if (enable_security && !final_plan.audit_log_path.empty()) {
                (void)copperfin::security::append_immutable_audit_event(
                    final_plan.audit_log_path,
                    "policy.denied",
                    publish_error);
            }
            std::cout << "status: error\n";
            print_error_line(catalog, publish_error);
            return 6;
        }
    }

    std::cout << "status: ok\n";
    std::cout << "project.title: " << final_plan.project_title << "\n";
    std::cout << "package.root: " << final_plan.package_root << "\n";
    std::cout << "manifest.path: " << final_plan.manifest_path << "\n";
    std::cout << "debug.manifest.path: " << final_plan.debug_manifest_path << "\n";
    std::cout << "ast.manifest.path: " << final_plan.ast_manifest_path << "\n";
    std::cout << "ir.manifest.path: " << final_plan.ir_manifest_path << "\n";
    std::cout << "transpiled.csharp.path: " << final_plan.transpiled_csharp_path << "\n";
    std::cout << "startup.item: " << final_plan.startup_item << "\n";
    std::cout << "startup.source: " << final_plan.startup_source_path << "\n";
    std::cout << "output.kind: " << copperfin::runtime::build_output_kind_name(final_plan.output_kind) << "\n";
    std::cout << "launcher.output: " << final_plan.launcher_output_path << "\n";
    std::cout << "module.definition: " << final_plan.module_definition_path << "\n";
    std::cout << "library.api.manifest: " << final_plan.library_api_manifest_path << "\n";
    std::cout << "fll.api.manifest: " << final_plan.fll_api_manifest_path << "\n";
    std::cout << "fxp.token.manifest: " << final_plan.fxp_token_manifest_path << "\n";
    std::cout << "app.archive.manifest: " << final_plan.app_archive_manifest_path << "\n";
    std::cout << "primary.output.materialized: " << (final_plan.primary_output_materialized ? "true" : "false") << "\n";
    std::cout << "security.enabled: " << (final_plan.security_enabled ? "true" : "false") << "\n";
    std::cout << "warnings: " << final_plan.warnings.size() << "\n";
    for (const auto& warning : final_plan.warnings) {
        print_warning_line(catalog, warning);
    }

    return 0;
}
