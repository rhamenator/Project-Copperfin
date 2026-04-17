#include "copperfin/platform/extensibility_model.h"
#include "copperfin/runtime/runtime_pipeline.h"
#include "copperfin/security/audit_stream.h"
#include "copperfin/security/authorization.h"
#include "copperfin/security/external_process_policy.h"
#include "copperfin/security/process_hardening.h"
#include "copperfin/security/secret_provider.h"
#include "copperfin/security/security_model.h"
#include "copperfin/studio/document_model.h"
#include "copperfin/studio/project_workspace.h"

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

void print_usage() {
    std::cout << "Usage: copperfin_build_host build --project <path-to-pjx> --output-dir <directory> [--configuration debug|release] [--enable-security] [--emit-dotnet-launcher] [--runtime-host <path>]\n";
}

std::string resolve_runtime_host_path(const std::string& override_path) {
    if (!override_path.empty()) {
        return override_path;
    }

    std::string resolved;
#ifdef _WIN32
    char* configured = nullptr;
    std::size_t configured_length = 0;
    if (_dupenv_s(&configured, &configured_length, "COPPERFIN_RUNTIME_HOST_PATH") == 0 && configured != nullptr) {
        resolved = configured;
        std::free(configured);
    }
#else
    if (const char* configured = std::getenv("COPPERFIN_RUNTIME_HOST_PATH"); configured != nullptr) {
        resolved = configured;
    }
#endif

    if (!resolved.empty()) {
        return resolved;
    }

    return R"(E:\Project-Copperfin\build\Release\copperfin_runtime_host.exe)";
}

bool run_dotnet_publish(const copperfin::runtime::RuntimePackagePlan& plan, std::string& error) {
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
        error = "dotnet publish denied by external process policy: " + auth.error;
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
        error = "dotnet publish failed to start: " + std::error_code(errno, std::generic_category()).message();
        return false;
    }

    if (exit_code != 0) {
        error = "dotnet publish failed for generated launcher.";
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
            error = "Generated launcher executable was not found after publish.";
            return false;
        }
    }

    return true;
}

}  // namespace

int main(int argc, char** argv) {
    const auto hardening = copperfin::security::apply_default_process_hardening();
    if (!hardening.applied) {
        std::cerr << "warning: " << hardening.message << "\n";
    }

    std::vector<std::string> args;
    for (int index = 1; index < argc; ++index) {
        args.emplace_back(argv[index]);
    }

    if (args.empty() || args[0] != "build") {
        print_usage();
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
            std::cout << "error: Unknown or incomplete argument: " << arg << "\n";
            print_usage();
            return 2;
        }
    }

    if (project_path.empty() || output_dir.empty()) {
        std::cout << "status: error\n";
        std::cout << "error: --project and --output-dir are required.\n";
        print_usage();
        return 2;
    }

    const auto open_result = copperfin::studio::open_document({.path = project_path});
    if (!open_result.ok) {
        std::cout << "status: error\n";
        std::cout << "error: " << open_result.error << "\n";
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
            std::cout << "error: Security policy denied build.execute for role '" << security_role << "'.\n";
            return 7;
        }

        if (configuration == copperfin::runtime::BuildConfiguration::release &&
            !copperfin::security::role_has_permission(security_profile, security_role, "build.release")) {
            std::cout << "status: error\n";
            std::cout << "error: Security policy denied build.release for role '" << security_role << "'.\n";
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
                std::cout << "error: Security-enabled release builds require COPPERFIN_RELEASE_SIGNING_KEY_REF (env:<NAME>).\n";
                return 7;
            }

            const auto secret = copperfin::security::resolve_secret_reference(signing_ref);
            if (!secret.ok) {
                std::cout << "status: error\n";
                std::cout << "error: Signing key reference validation failed: " << secret.error << "\n";
                return 7;
            }
        }
    }

    const auto extensibility_profile = copperfin::platform::default_extensibility_profile();
    const auto plan = copperfin::runtime::create_runtime_package_plan(
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
        std::cout << "error: Build plan creation failed.\n";
        return 4;
    }

    const std::string runtime_host_path = resolve_runtime_host_path(runtime_host_override);
    const auto materialized = copperfin::runtime::materialize_runtime_package(
        plan,
        security_profile,
        extensibility_profile,
        runtime_host_path);
    if (!materialized.ok) {
        std::cout << "status: error\n";
        std::cout << "error: " << materialized.error << "\n";
        return 5;
    }

    if (enable_security && !materialized.plan.audit_log_path.empty()) {
        (void)copperfin::security::append_immutable_audit_event(
            materialized.plan.audit_log_path,
            "build.package_materialized",
            "role=" + security_role + ",project=" + materialized.plan.project_title);
    }

    if (emit_dotnet_launcher) {
        std::string publish_error;
        if (!run_dotnet_publish(materialized.plan, publish_error)) {
            if (enable_security && !materialized.plan.audit_log_path.empty()) {
                (void)copperfin::security::append_immutable_audit_event(
                    materialized.plan.audit_log_path,
                    "policy.denied",
                    publish_error);
            }
            std::cout << "status: error\n";
            std::cout << "error: " << publish_error << "\n";
            return 6;
        }
    }

    std::cout << "status: ok\n";
    std::cout << "project.title: " << materialized.plan.project_title << "\n";
    std::cout << "package.root: " << materialized.plan.package_root << "\n";
    std::cout << "manifest.path: " << materialized.plan.manifest_path << "\n";
    std::cout << "debug.manifest.path: " << materialized.plan.debug_manifest_path << "\n";
    std::cout << "startup.item: " << materialized.plan.startup_item << "\n";
    std::cout << "startup.source: " << materialized.plan.startup_source_path << "\n";
    std::cout << "launcher.output: " << materialized.plan.launcher_output_path << "\n";
    std::cout << "security.enabled: " << (materialized.plan.security_enabled ? "true" : "false") << "\n";
    std::cout << "warnings: " << materialized.plan.warnings.size() << "\n";
    for (const auto& warning : materialized.plan.warnings) {
        std::cout << "warning: " << warning << "\n";
    }

    return 0;
}
