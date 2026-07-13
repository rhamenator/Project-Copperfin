// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/security/audit_stream.h"
#include "test_environment_support.h"

#include <algorithm>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#if !defined(_WIN32)
#include <sys/wait.h>
#endif

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void write_text(const std::filesystem::path& path, const std::string& text) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << text;
}

std::string read_text(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

std::string quote_command_argument(const std::string& value) {
    std::string quoted = "\"";
    quoted.reserve(value.size() + 2U);
    for (const char ch : value) {
        if (ch == '"') {
            quoted += "\\\"";
        } else {
            quoted.push_back(ch);
        }
    }
    quoted.push_back('"');
    return quoted;
}

struct ProcessResult {
    int exit_code = -1;
    std::string stdout_text;
};

ProcessResult run_process_capture(
    const std::string& executable_path,
    const std::vector<std::string>& arguments,
    const std::filesystem::path& working_directory) {
    namespace fs = std::filesystem;

    const fs::path stdout_path = working_directory / "runtime_host_stdout.log";
    const fs::path stderr_path = working_directory / "runtime_host_stderr.log";
    std::string command = quote_command_argument(executable_path);
    for (const auto& argument : arguments) {
        command += " ";
        command += quote_command_argument(argument);
    }
    command += " > ";
    command += quote_command_argument(stdout_path.string());
    command += " 2> ";
    command += quote_command_argument(stderr_path.string());

    const fs::path original_directory = fs::current_path();
    fs::current_path(working_directory);
    const int raw_exit_code = copperfin::test_support::run_shell_command(command);
    fs::current_path(original_directory);

    ProcessResult result;
    if (fs::exists(stdout_path)) {
        result.stdout_text = read_text(stdout_path);
    }
#if defined(_WIN32)
    result.exit_code = raw_exit_code;
#else
    if (raw_exit_code != -1 && WIFEXITED(raw_exit_code)) {
        result.exit_code = WEXITSTATUS(raw_exit_code);
    } else {
        result.exit_code = raw_exit_code;
    }
#endif
    return result;
}

bool create_directory_indirection(
    const std::filesystem::path& target,
    const std::filesystem::path& link) {
#if defined(_WIN32)
    const std::string command =
        "cmd.exe /d /c mklink /J " + quote_command_argument(link.string()) + " " +
        quote_command_argument(target.string()) + " > NUL 2>&1";
    return copperfin::test_support::run_shell_command(command) == 0;
#else
    std::error_code error;
    std::filesystem::create_directory_symlink(target, link, error);
    return !error;
#endif
}

void remove_directory_indirection(const std::filesystem::path& link) {
#if defined(_WIN32)
    const std::string command =
        "cmd.exe /d /c rmdir " + quote_command_argument(link.string()) + " > NUL 2>&1";
    (void)copperfin::test_support::run_shell_command(command);
#else
    std::error_code ignored;
    std::filesystem::remove(link, ignored);
#endif
}

void write_denial_manifest(
    const std::filesystem::path& case_root,
    const std::string& package_root,
    const std::string& audit_log_path) {
    const std::filesystem::path content_root = case_root / "content";
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
}

void test_runtime_host_audit_containment(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_runtime_host_audit_path_containment_focused";
    const fs::path packages_root = temp_root / "packages";
    const fs::path external_root = temp_root / "external";
    const fs::path external_audit_path = external_root / "security_audit.log";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(packages_root);
    fs::create_directories(external_root);

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
               "#4015: a package-local audit path should create directories and a valid chain");
        expect(!fs::exists(case_root / "security_audit.log"),
               "#4015: a custom package-local path should not use the default audit leaf");
    }

#if defined(_WIN32)
    {
        const fs::path case_root = packages_root / "windows_case_fidelity";
        const fs::path local_audit_path = case_root / "logs" / "case-spelling.log";
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
               "#4015: Windows package components should compare case-insensitively");
        const auto audit_chain = copperfin::security::verify_immutable_audit_chain(
            local_audit_path.string());
        expect(audit_chain.ok && audit_chain.entries == 1U,
               "#4015: Windows case rebinding should preserve the packaged audit leaf");
    }
#endif

    const auto expect_rejected_path = [&] (
                                           const std::string& case_name,
                                           const std::string& package_root,
                                           const std::string& audit_log_path,
                                           const fs::path& sentinel_path,
                                           const std::string& expected_file_name) {
        const fs::path case_root = packages_root / case_name;
        write_text(sentinel_path, "external sentinel\n");
        write_denial_manifest(case_root, package_root, audit_log_path);

        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", (case_root / "app.cfmanifest").string()},
            temp_root);

        expect(process.exit_code == 8,
               "#4015: rejected audit paths should retain exit code 8 for " + case_name);
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#4015: rejected audit paths should retain invariant status for " + case_name);
        expect(process.stdout_text.find(
                   "error: Package path failed physical containment validation: " +
                   expected_file_name) != std::string::npos,
               "#4015: rejected paths should use localized containment prose for " + case_name);
        expect(read_text(sentinel_path) == "external sentinel\n",
               "#4015: rejected paths must not modify the sentinel for " + case_name);
        expect(!fs::exists(case_root / "security_audit.log"),
               "#4015: rejected explicit paths must not append to a fallback for " + case_name);
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
        write_text(external_audit_path, "external sentinel\n");
        fs::create_directories(case_root);
        std::error_code hard_link_error;
        fs::create_hard_link(external_audit_path, hard_link_path, hard_link_error);
        if (!hard_link_error) {
            write_denial_manifest(case_root, case_root.string(), "hard-linked.log");
            const auto process = run_process_capture(
                runtime_host_path,
                {"--manifest", (case_root / "app.cfmanifest").string()},
                temp_root);

            expect(process.exit_code == 8,
                   "#4015: a package-local hard link should fail containment");
            expect(process.stdout_text.find(
                       "error: Package path failed physical containment validation: hard-linked.log") !=
                       std::string::npos,
                   "#4015: hard-link rejection should retain localized containment prose");
            expect(read_text(external_audit_path) == "external sentinel\n",
                   "#4015: hard-link rejection must preserve the external identity");
        }
    }

    {
        const fs::path case_root = packages_root / "ambiguous_rebind";
        const fs::path recorded_root = temp_root / "builder" / "package";
        const fs::path basename_sentinel = case_root / "ambiguous.log";
        const fs::path exact_rebound_path = case_root / "logs" / "ambiguous.log";
        write_text(basename_sentinel, "external sentinel\n");
        write_denial_manifest(
            case_root,
            recorded_root.string(),
            (recorded_root / "logs" / "ambiguous.log").string());

        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", (case_root / "app.cfmanifest").string()},
            temp_root);

        expect(process.exit_code == 7,
               "#4015: exact nested rebinding should retain the policy-denial exit code");
        expect(read_text(basename_sentinel) == "external sentinel\n",
               "#4015: exact rebinding must not use a same-named root fallback");
        const auto audit_chain = copperfin::security::verify_immutable_audit_chain(
            exact_rebound_path.string());
        expect(audit_chain.ok && audit_chain.entries == 1U,
               "#4015: exact rebinding should create the recorded nested path");
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

}  // namespace

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "FAIL: runtime host executable and locale root arguments are required\n";
        return 1;
    }

    copperfin::test_support::ScopedEnvironmentPath locale_dir(
        "COPPERFIN_LOCALE_DIR",
        std::filesystem::path(argv[2]));
    copperfin::test_support::ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "en-US");
    test_runtime_host_audit_containment(argv[1]);
    return failures == 0 ? 0 : 1;
}
