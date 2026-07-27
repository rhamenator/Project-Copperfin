// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/licensing/license_status.h"
#include "copperfin/licensing/license_status_display.h"
#include "copperfin/localization/localization.h"
#include "copperfin/platform/environment.h"
#include "copperfin/platform/executable_path.h"
#include "copperfin/platform/extensibility_model.h"
#include "copperfin/platform/path.h"
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
#include <cctype>
#include <filesystem>
#include <iostream>
#if defined(_WIN32)
#include <process.h>
#else
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
extern char** environ;
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
    const std::filesystem::path& executable_path,
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
            {"externalIncludeRootOption", "--external-include-root"},
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

std::string normalized_build_configuration(std::string value) {
    const auto not_space = [](unsigned char ch) { return std::isspace(ch) == 0; };
    const auto first = std::find_if(value.begin(), value.end(), not_space);
    const auto last = std::find_if(value.rbegin(), value.rend(), not_space).base();
    if (first >= last) {
        return {};
    }
    value = std::string(first, last);
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

void print_license_status(
    const copperfin::licensing::LicenseStatus& status,
    const copperfin::localization::LocalizedCatalog& catalog) {
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
        std::cout << "diagnostic: " << copperfin::licensing::localized_license_diagnostic(status, catalog) << "\n";
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

bool path_exists_without_error(const std::filesystem::path& path) {
    std::error_code error;
    return std::filesystem::exists(path, error) && !error;
}

bool regular_file_exists_without_error(const std::filesystem::path& path) {
    std::error_code error;
    return std::filesystem::is_regular_file(path, error) && !error;
}

std::string resolve_runtime_host_path(
    const std::string& override_path,
    const std::filesystem::path& running_executable_path) {
    if (!override_path.empty()) {
        return override_path;
    }

    const std::string resolved =
        copperfin::platform::read_environment_variable_or_empty("COPPERFIN_RUNTIME_HOST_PATH");
    if (!resolved.empty()) {
        return resolved;
    }

    const std::filesystem::path host_root = running_executable_path.parent_path();
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
        if (path_exists_without_error(candidate)) {
            return copperfin::platform::path_to_utf8_string(candidate);
        }
    }

    return copperfin::platform::path_to_utf8_string(host_root / host_name);
}

std::vector<std::string> dotnet_allowed_path_roots() {
#if defined(_WIN32)
    return {
        R"(C:\Program Files\dotnet)",
        R"(C:\Program Files (x86)\dotnet)"
    };
#else
    std::vector<std::string> roots{
        "/usr/bin",
        "/usr/local/bin",
        "/usr/share/dotnet",
        "/usr/local/share/dotnet"
    };
    const auto configured_path = copperfin::platform::read_environment_variable("PATH");
    const std::string path_value = configured_path.has_value()
        ? *configured_path
        : copperfin::platform::default_posix_search_path().value_or(std::string{});
    std::size_t start = 0U;
    for (;;) {
        const std::size_t separator = path_value.find(':', start);
        const std::string directory = path_value.substr(
            start,
            separator == std::string::npos ? std::string::npos : separator - start);
        const std::filesystem::path candidate =
            copperfin::platform::path_from_utf8_string(directory.empty() ? "." : directory) /
            "dotnet";
        std::error_code candidate_error;
        if (std::filesystem::is_regular_file(candidate, candidate_error) && !candidate_error) {
            const std::filesystem::path canonical =
                std::filesystem::weakly_canonical(candidate, candidate_error);
            if (!candidate_error) {
                roots.push_back(copperfin::platform::path_to_utf8_string(canonical.parent_path()));
            }
        }
        if (separator == std::string::npos) {
            break;
        }
        start = separator + 1U;
    }
    for (const char* name : {"DOTNET_ROOT", "DOTNET_ROOT_X64"}) {
        const auto root = copperfin::platform::read_environment_path(name);
        if (root.has_value()) {
            roots.push_back(copperfin::platform::path_to_utf8_string(*root));
        }
    }
    const auto home = copperfin::platform::read_environment_path("HOME");
    if (home.has_value()) {
        roots.push_back(copperfin::platform::path_to_utf8_string(*home / ".dotnet"));
    }
    return roots;
#endif
}

copperfin::security::ExternalProcessPolicy dotnet_process_policy() {
    return {
#if defined(_WIN32)
        .executable_name = "dotnet.exe",
#else
        .executable_name = "dotnet",
#endif
        .allowed_path_roots = dotnet_allowed_path_roots(),
#if defined(_WIN32)
        .allowed_publishers = {"Microsoft Corporation"},
        .require_trusted_signature = true
#else
        .allowed_publishers = {},
        .require_trusted_signature = false
#endif
    };
}

std::string current_dotnet_runtime_identifier() {
#if defined(_WIN32)
#if defined(_M_ARM64) || defined(__aarch64__)
    return "win-arm64";
#elif defined(_M_IX86) || defined(__i386__)
    return "win-x86";
#elif defined(_M_X64) || defined(__x86_64__)
    return "win-x64";
#else
    return {};
#endif
#elif defined(__APPLE__)
#if defined(__aarch64__) || defined(__arm64__)
    return "osx-arm64";
#elif defined(__x86_64__)
    return "osx-x64";
#else
    return {};
#endif
#elif defined(__linux__)
#if defined(__aarch64__)
    return "linux-arm64";
#elif defined(__x86_64__)
    return "linux-x64";
#elif defined(__arm__)
    return "linux-arm";
#elif defined(__i386__)
    return "linux-x86";
#else
    return {};
#endif
#else
    return {};
#endif
}

#if defined(_WIN32)
std::wstring quote_windows_spawn_argument(const std::wstring& value) {
    if (!value.empty() && value.find_first_of(L" \t\n\v\"") == std::wstring::npos) {
        return value;
    }

    std::wstring quoted = L"\"";
    std::size_t backslashes = 0U;
    for (const wchar_t ch : value) {
        if (ch == L'\\') {
            ++backslashes;
            continue;
        }
        if (ch == L'\"') {
            quoted.append(backslashes * 2U + 1U, L'\\');
            quoted.push_back(ch);
            backslashes = 0U;
            continue;
        }
        quoted.append(backslashes, L'\\');
        backslashes = 0U;
        quoted.push_back(ch);
    }
    quoted.append(backslashes * 2U, L'\\');
    quoted.push_back(L'\"');
    return quoted;
}
#endif

bool run_dotnet_publish(
    const copperfin::runtime::RuntimePackagePlan& plan,
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::filesystem::path& running_executable_path,
    std::string& error) {
    const std::filesystem::path project_path =
        copperfin::platform::path_from_utf8_string(plan.launcher_project_path);
    const std::filesystem::path output_dir =
        copperfin::platform::path_from_utf8_string(plan.package_root);
    const std::string configuration = plan.configuration == copperfin::runtime::BuildConfiguration::release ? "Release" : "Debug";

    auto auth = copperfin::security::authorize_external_process(dotnet_process_policy());
    if (!auth.allowed) {
        error = message(catalog, "BuildHost.Error.DotnetPublishDenied", {{"error", auth.error}});
        return false;
    }

    std::vector<std::string> publish_args = {
        auth.resolved_path,
        "publish",
        copperfin::platform::path_to_utf8_string(project_path),
        "-noAutoResponse",
        "-p:ImportDirectoryBuildProps=false",
        "-p:ImportDirectoryBuildTargets=false",
        "-c",
        configuration,
        "-r",
        current_dotnet_runtime_identifier(),
        "--self-contained",
        "false",
        "-p:UseAppHost=true",
        "-o",
        copperfin::platform::path_to_utf8_string(output_dir)
    };

    intptr_t exit_code = -1;
#if defined(_WIN32)
    if (!copperfin::security::revalidate_external_process_authorization(auth)) {
        error = message(catalog, "BuildHost.Error.DotnetPublishDenied", {{"error", auth.error}});
        return false;
    }
    std::vector<std::wstring> wide_args;
    wide_args.reserve(publish_args.size());
    wide_args.push_back(
        copperfin::platform::path_from_utf8_string(auth.resolved_path).filename().wstring());
    for (std::size_t index = 1U; index < publish_args.size(); ++index) {
        wide_args.push_back(quote_windows_spawn_argument(
            copperfin::platform::path_from_utf8_string(publish_args[index]).wstring()));
    }

    std::vector<const wchar_t*> argv;
    argv.reserve(wide_args.size() + 1U);
    for (const auto& arg : wide_args) {
        argv.push_back(arg.c_str());
    }
    argv.push_back(nullptr);
    exit_code = _wspawnv(
        _P_WAIT,
        copperfin::platform::path_from_utf8_string(auth.resolved_path).c_str(),
        argv.data());
#else
#if defined(__linux__)
    // Keep the descriptor open across interpreter handoff for authorized
    // shebang tools such as the POSIX dotnet shim.
    const int verified_executable = ::open(auth.resolved_path.c_str(), O_RDONLY);
    if (verified_executable < 0) {
        error = message(
            catalog,
            "BuildHost.Error.DotnetPublishFailedToStart",
            {{"error", std::error_code(errno, std::generic_category()).message()}});
        return false;
    }
#endif
    if (!copperfin::security::revalidate_external_process_authorization(auth)) {
#if defined(__linux__)
        ::close(verified_executable);
#endif
        error = message(catalog, "BuildHost.Error.DotnetPublishDenied", {{"error", auth.error}});
        return false;
    }
    std::vector<const char*> argv;
    argv.reserve(publish_args.size() + 1U);
    for (const auto& arg : publish_args) {
        argv.push_back(arg.c_str());
    }
    argv.push_back(nullptr);
    const pid_t child = fork();
    if (child == 0) {
#if defined(__linux__)
        fexecve(verified_executable, const_cast<char* const*>(argv.data()), environ);
#else
        execvp(auth.resolved_path.c_str(), const_cast<char* const*>(argv.data()));
#endif
        _exit(127);
    }
#if defined(__linux__)
    ::close(verified_executable);
#endif
    if (child > 0) {
        int status = 0;
        pid_t waited = -1;
        do {
            waited = waitpid(child, &status, 0);
        } while (waited < 0 && errno == EINTR);
        if (waited == child && WIFEXITED(status)) {
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

    const std::string project_stem =
        copperfin::platform::path_to_utf8_string(project_path.stem());
#if defined(_WIN32)
    const std::filesystem::path published_launcher =
        output_dir / copperfin::platform::path_from_utf8_string(project_stem + ".exe");
    const std::filesystem::path internal_apphost =
        output_dir / copperfin::platform::path_from_utf8_string(project_stem + ".apphost.exe");
#else
    const std::filesystem::path published_launcher =
        output_dir / copperfin::platform::path_from_utf8_string(project_stem);
#endif
    const std::filesystem::path configured_launcher =
        copperfin::platform::path_from_utf8_string(plan.launcher_output_path);
#if defined(_WIN32)
    const std::filesystem::path guard_source =
        running_executable_path.parent_path() /
        "copperfin_launcher_guard.exe";
#else
    (void)running_executable_path;
#endif
    if (!path_exists_without_error(published_launcher)) {
        error = message(catalog, "BuildHost.Error.GeneratedLauncherMissing");
        return false;
    }
#if defined(_WIN32)
    if (published_launcher != internal_apphost) {
        std::error_code rename_error;
        std::filesystem::rename(published_launcher, internal_apphost, rename_error);
        if (rename_error) {
            error = message(catalog, "BuildHost.Error.GeneratedLauncherMissing");
            return false;
        }
    }
    if (!regular_file_exists_without_error(internal_apphost) ||
        !regular_file_exists_without_error(guard_source)) {
        error = message(catalog, "BuildHost.Error.GeneratedLauncherMissing");
        return false;
    }

    std::error_code copy_error;
    std::filesystem::copy_file(
        guard_source,
        configured_launcher,
        std::filesystem::copy_options::overwrite_existing,
        copy_error);
    if (copy_error || !regular_file_exists_without_error(configured_launcher)) {
        error = message(catalog, "BuildHost.Error.GeneratedLauncherMissing");
        return false;
    }
#else
    if (published_launcher != configured_launcher) {
        std::error_code rename_error;
        std::filesystem::rename(published_launcher, configured_launcher, rename_error);
        if (rename_error) {
            error = message(catalog, "BuildHost.Error.GeneratedLauncherMissing");
            return false;
        }
    }
    if (!regular_file_exists_without_error(configured_launcher)) {
        error = message(catalog, "BuildHost.Error.GeneratedLauncherMissing");
        return false;
    }
#endif

    return true;
}

bool is_library_output_kind(const copperfin::runtime::BuildOutputKind output_kind) {
    return output_kind == copperfin::runtime::BuildOutputKind::dll ||
        output_kind == copperfin::runtime::BuildOutputKind::fll ||
        output_kind == copperfin::runtime::BuildOutputKind::ocx;
}

bool supports_dotnet_launcher_publish() {
    return !current_dotnet_runtime_identifier().empty() &&
        copperfin::security::authorize_external_process(dotnet_process_policy()).allowed;
}

}  // namespace

int run_build_host_main(int argc, char** argv) {
    const std::filesystem::path invocation_path =
        argc > 0 && argv[0] != nullptr
            ? copperfin::platform::path_from_utf8_string(argv[0])
            : std::filesystem::path();
    const std::filesystem::path running_executable_path =
        copperfin::platform::resolve_running_executable_path(invocation_path);
    const copperfin::localization::LocalizedCatalog catalog =
        load_localization(running_executable_path, explicit_locale_from_arguments(argc, argv));

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
        print_license_status(copperfin::licensing::load_license_status(running_executable_path), catalog);
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
    std::vector<std::string> external_include_roots;

    for (std::size_t index = 1; index < args.size(); ++index) {
        const auto& arg = args[index];
        if (arg == "--project" && (index + 1U) < args.size()) {
            project_path = args[++index];
        } else if (arg == "--output-dir" && (index + 1U) < args.size()) {
            output_dir = args[++index];
        } else if (arg == "--configuration" && (index + 1U) < args.size()) {
            const std::string configuration_value = normalized_build_configuration(args[++index]);
            if (configuration_value != "debug" && configuration_value != "release") {
                std::cout << "status: error\n";
                print_error_line(
                    catalog,
                    message(
                        catalog,
                        "BuildHost.Error.InvalidConfiguration",
                        {{"configuration", configuration_value}}));
                return 2;
            }
            configuration = copperfin::runtime::parse_build_configuration(configuration_value);
        } else if (arg == "--enable-security") {
            enable_security = true;
        } else if (arg == "--emit-dotnet-launcher") {
            emit_dotnet_launcher = true;
        } else if (arg == "--external-include-root" && (index + 1U) < args.size()) {
            external_include_roots.push_back(args[++index]);
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

    const auto open_result = copperfin::studio::open_document({.path = project_path}, catalog);
    if (!open_result.ok) {
        std::cout << "status: error\n";
        print_error_line(catalog, open_result.error);
        return 3;
    }

    const auto workspace = copperfin::studio::build_project_workspace(open_result.document, catalog);
    const auto security_profile = copperfin::security::default_native_security_profile(catalog);

    if (enable_security) {
        const std::string role_env =
            copperfin::platform::read_environment_variable_or_empty("COPPERFIN_SECURITY_ROLE");
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
            const std::string signing_ref =
                copperfin::platform::read_environment_variable_or_empty("COPPERFIN_RELEASE_SIGNING_KEY_REF");

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

    auto extensibility_profile = copperfin::platform::default_extensibility_profile(catalog);
    if (emit_dotnet_launcher && !supports_dotnet_launcher_publish()) {
        extensibility_profile.dotnet_output.available = false;
    }
    auto plan = copperfin::runtime::create_runtime_package_plan(
        open_result.document,
        workspace,
        security_profile,
        extensibility_profile,
        output_dir,
        configuration,
        enable_security,
        emit_dotnet_launcher,
        external_include_roots);

    if (!plan.ok) {
        std::cout << "status: error\n";
        print_error_line(catalog, message(catalog, "BuildHost.Error.BuildPlanCreationFailed"));
        return 4;
    }

    // Stamped into app.cfmanifest/app.cfdebug as inert, informational
    // license metadata -- never gates whether the build succeeds or
    // what it produces. Local source-path provenance stays out of the
    // durable package artifacts. See LicenseState::perpetual_out_of_version's
    // doc comment.
    const auto license_status = copperfin::licensing::load_license_status(running_executable_path);
    plan.license_state = std::string(copperfin::licensing::license_state_name(license_status.state));
    plan.license_type = license_status.license_type;
    plan.license_id = license_status.license_id;
    plan.license_licensee = license_status.licensee_name;
    plan.license_seats = license_status.seats;
    plan.license_subscription_expires = license_status.subscription_expires;
    plan.license_perpetual_max_major_version = license_status.perpetual_max_major_version;

    const std::string runtime_host_path = resolve_runtime_host_path(
        runtime_host_override,
        running_executable_path);
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
        auto build_result = copperfin::runtime::build_runtime_package_primary_output(
            materialized.plan,
            security_profile,
            extensibility_profile);
        if (!build_result.ok) {
            const auto rollback_result = copperfin::runtime::abort_runtime_package_transaction(
                materialized.plan);
            if (!rollback_result.ok) {
                build_result.error += "\n" + rollback_result.error;
            }
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
        if (!run_dotnet_publish(final_plan, catalog, running_executable_path, publish_error)) {
            const auto rollback_result = copperfin::runtime::abort_runtime_package_transaction(final_plan);
            if (!rollback_result.ok) {
                publish_error += "\n" + rollback_result.error;
            }
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

        const auto finalize_result = copperfin::runtime::finalize_runtime_package_primary_output(
            final_plan,
            security_profile,
            extensibility_profile);
        if (!finalize_result.ok) {
            if (enable_security && !final_plan.audit_log_path.empty()) {
                (void)copperfin::security::append_immutable_audit_event(
                    final_plan.audit_log_path,
                    "policy.denied",
                    finalize_result.error);
            }
            std::cout << "status: error\n";
            print_error_line(catalog, finalize_result.error);
            return 6;
        }
        final_plan = finalize_result.plan;
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

#if defined(_WIN32)
int wmain(int argc, wchar_t* argv[]) {
    std::vector<std::string> utf8_arguments;
    std::vector<char*> narrow_arguments;
    utf8_arguments.reserve(static_cast<std::size_t>(argc));
    narrow_arguments.reserve(static_cast<std::size_t>(argc));
    for (int index = 0; index < argc; ++index) {
        utf8_arguments.push_back(copperfin::platform::path_to_utf8_string(
            std::filesystem::path(argv[index])));
        narrow_arguments.push_back(utf8_arguments.back().data());
    }
    return run_build_host_main(argc, narrow_arguments.data());
}
#else
int main(int argc, char** argv) {
    return run_build_host_main(argc, argv);
}
#endif
