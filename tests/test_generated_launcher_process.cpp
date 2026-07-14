// Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/localization/localization.h"
#include "copperfin/platform/extensibility_model.h"
#include "copperfin/runtime/runtime_pipeline.h"
#include "copperfin/security/security_model.h"
#include "copperfin/studio/document_model.h"
#include "copperfin/studio/project_workspace.h"
#include "test_environment_support.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace {

constexpr int skip_return_code = 77;
constexpr std::string_view fixture_environment =
    "COPPERFIN_TEST_GENERATED_LAUNCHER_HOST";

std::string manifest_value(const std::filesystem::path& path, const std::string& key) {
    std::ifstream input(path, std::ios::binary);
    const std::string prefix = key + "=";
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.rfind(prefix, 0U) == 0U) {
            return line.substr(prefix.size());
        }
    }
    return {};
}

int run_recording_runtime_host(int argc, char** argv) {
    if (argc < 3 || std::string_view(argv[1]) != "--manifest") {
        std::cerr << "fixture runtime host requires --manifest and a manifest path\n";
        return 90;
    }

    const std::filesystem::path manifest_path = argv[2];
    std::cout << "fixture.status=ok\n";
    std::cout << "fixture.cwd=" << std::filesystem::current_path().string() << "\n";
    std::cout << "fixture.argument_count=" << (argc - 1) << "\n";
    for (int index = 1; index < argc; ++index) {
        std::cout << "fixture.argument[" << (index - 1) << "]=" << argv[index] << "\n";
    }
    std::cout << "fixture.manifest.project_title="
              << manifest_value(manifest_path, "project_title") << "\n";
    std::cout << "fixture.manifest.startup_item="
              << manifest_value(manifest_path, "startup_item") << "\n";
    std::cout << "fixture.manifest.startup_source="
              << manifest_value(manifest_path, "startup_source") << "\n";
    return 0;
}

#if defined(_WIN32)

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void write_text(const std::filesystem::path& path, const std::string& text) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << text;
}

std::string read_text(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return std::string(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>());
}

std::string line_value(const std::string& text, const std::string& prefix) {
    const std::size_t start = text.find(prefix);
    if (start == std::string::npos) {
        return {};
    }
    const std::size_t value_start = start + prefix.size();
    const std::size_t end = text.find('\n', value_start);
    std::string value = text.substr(
        value_start,
        end == std::string::npos ? std::string::npos : end - value_start);
    if (!value.empty() && value.back() == '\r') {
        value.pop_back();
    }
    return value;
}

struct ProcessResult {
    int exit_code = -1;
    unsigned long start_error = 0;
    bool timed_out = false;
    std::string stdout_text;
    std::string stderr_text;
};

std::wstring quote_windows_argument(const std::wstring& value) {
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

std::wstring widen_utf8_argument(const std::string& value) {
    if (value.empty()) {
        return {};
    }
    const int required = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.data(),
        static_cast<int>(value.size()),
        nullptr,
        0);
    if (required <= 0) {
        return std::wstring(value.begin(), value.end());
    }
    std::wstring result(static_cast<std::size_t>(required), L'\0');
    MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.data(),
        static_cast<int>(value.size()),
        result.data(),
        required);
    return result;
}

std::string current_windows_runtime_identifier() {
#if defined(_M_ARM64) || defined(__aarch64__)
    return "win-arm64";
#elif defined(_M_IX86) || defined(__i386__)
    return "win-x86";
#elif defined(_M_X64) || defined(__x86_64__)
    return "win-x64";
#else
    return {};
#endif
}

ProcessResult run_process_capture(
    const std::filesystem::path& executable,
    const std::vector<std::string>& arguments,
    const std::filesystem::path& working_directory,
    const std::filesystem::path& log_root,
    const std::string& log_name,
    unsigned long timeout_ms) {
    namespace fs = std::filesystem;

    const fs::path stdout_path = log_root / (log_name + ".stdout.log");
    const fs::path stderr_path = log_root / (log_name + ".stderr.log");
    SECURITY_ATTRIBUTES security_attributes{};
    security_attributes.nLength = sizeof(security_attributes);
    security_attributes.bInheritHandle = TRUE;

    const HANDLE stdout_handle = CreateFileW(
        stdout_path.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        &security_attributes,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    const HANDLE stderr_handle = CreateFileW(
        stderr_path.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        &security_attributes,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    const HANDLE stdin_handle = CreateFileW(
        L"NUL",
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        &security_attributes,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    ProcessResult result;
    if (stdout_handle == INVALID_HANDLE_VALUE ||
        stderr_handle == INVALID_HANDLE_VALUE ||
        stdin_handle == INVALID_HANDLE_VALUE) {
        result.start_error = GetLastError();
        if (stdout_handle != INVALID_HANDLE_VALUE) {
            CloseHandle(stdout_handle);
        }
        if (stderr_handle != INVALID_HANDLE_VALUE) {
            CloseHandle(stderr_handle);
        }
        if (stdin_handle != INVALID_HANDLE_VALUE) {
            CloseHandle(stdin_handle);
        }
        return result;
    }

    std::wstring command_line = quote_windows_argument(executable.wstring());
    for (const std::string& argument : arguments) {
        command_line.push_back(L' ');
        command_line += quote_windows_argument(widen_utf8_argument(argument));
    }
    std::vector<wchar_t> mutable_command(command_line.begin(), command_line.end());
    mutable_command.push_back(L'\0');

    STARTUPINFOW startup_info{};
    startup_info.cb = sizeof(startup_info);
    startup_info.dwFlags = STARTF_USESTDHANDLES;
    startup_info.hStdInput = stdin_handle;
    startup_info.hStdOutput = stdout_handle;
    startup_info.hStdError = stderr_handle;
    PROCESS_INFORMATION process_info{};

    const BOOL started = CreateProcessW(
        executable.c_str(),
        mutable_command.data(),
        nullptr,
        nullptr,
        TRUE,
        CREATE_NO_WINDOW,
        nullptr,
        working_directory.c_str(),
        &startup_info,
        &process_info);
    if (!started) {
        result.start_error = GetLastError();
    } else {
        const DWORD wait_result = WaitForSingleObject(process_info.hProcess, timeout_ms);
        if (wait_result == WAIT_TIMEOUT) {
            result.timed_out = true;
            TerminateProcess(process_info.hProcess, 124U);
            WaitForSingleObject(process_info.hProcess, 5000U);
        }
        DWORD exit_code = 0U;
        if (GetExitCodeProcess(process_info.hProcess, &exit_code)) {
            result.exit_code = static_cast<int>(exit_code);
        }
        CloseHandle(process_info.hThread);
        CloseHandle(process_info.hProcess);
    }

    CloseHandle(stdin_handle);
    CloseHandle(stdout_handle);
    CloseHandle(stderr_handle);
    result.stdout_text = read_text(stdout_path);
    result.stderr_text = read_text(stderr_path);
    return result;
}

bool paths_are_equivalent(
    const std::filesystem::path& left,
    const std::filesystem::path& right) {
    std::error_code error;
    return std::filesystem::equivalent(left, right, error) && !error;
}

void expect_manifest_selection(
    const ProcessResult& process,
    const std::filesystem::path& expected_manifest,
    const std::string& expected_title,
    const std::string& expected_startup_item,
    const std::filesystem::path& expected_working_directory,
    const std::string& context) {
    expect(process.start_error == 0U, context + " should start the generated launcher");
    expect(!process.timed_out, context + " should not time out");
    expect(process.exit_code == 0, context + " should preserve the fixture host exit code");
    expect(
        line_value(process.stdout_text, "fixture.status=") == "ok",
        context + " should reach the recording runtime host");
    expect(
        line_value(process.stdout_text, "fixture.argument[0]=") == "--manifest",
        context + " should preserve the invariant manifest switch");
    expect(
        paths_are_equivalent(
            line_value(process.stdout_text, "fixture.argument[1]="),
            expected_manifest),
        context + " should forward the selected manifest as one native argument");
    expect(
        line_value(process.stdout_text, "fixture.manifest.project_title=") == expected_title,
        context + " should reach the selected manifest title");
    expect(
        line_value(process.stdout_text, "fixture.manifest.startup_item=") == expected_startup_item,
        context + " should reach the selected startup identity");
    expect(
        paths_are_equivalent(
            line_value(process.stdout_text, "fixture.cwd="),
            expected_working_directory),
        context + " should run the host from the package directory");
}

int run_generated_launcher_test(const std::filesystem::path& dotnet_path, char** argv) {
    namespace fs = std::filesystem;

    if (!fs::exists(dotnet_path)) {
        std::cout << "SKIP: generated .NET launcher process test requires the Windows .NET SDK\n";
        return skip_return_code;
    }
    const std::string runtime_identifier = current_windows_runtime_identifier();
    if (runtime_identifier.empty()) {
        std::cout << "SKIP: generated .NET launcher process test does not recognize this Windows architecture\n";
        return skip_return_code;
    }

    const fs::path temp_root = fs::temp_directory_path() /
        ("copperfin generated launcher process " +
         std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const fs::path project_dir = temp_root / "source project with spaces";
    const fs::path output_dir = temp_root / "package output with spaces";
    const fs::path caller_dir = temp_root / "unrelated caller with spaces";
    const fs::path runtime_host_source = temp_root / "copperfin_runtime_host.exe";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    fs::create_directories(caller_dir);

    const fs::path test_executable = fs::weakly_canonical(fs::absolute(argv[0]), ignored);
    expect(!ignored && fs::exists(test_executable),
           "generated launcher process test should resolve its own executable");
    if (failures != 0) {
        return 1;
    }
    fs::copy_file(
        test_executable,
        runtime_host_source,
        fs::copy_options::overwrite_existing,
        ignored);
    expect(!ignored, "generated launcher process test should create its recording host fixture");

    write_text(project_dir / "main program.prg", "RETURN\n");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "launcher process.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "GeneratedLauncherProcess";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = workspace.project_title;
    workspace.build_plan.output_path =
        (output_dir / "GeneratedLauncherProcess.exe").string();
    workspace.build_plan.startup_item = "main program.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {
            .record_index = 1U,
            .name = "main program.prg",
            .relative_path = "main program.prg",
            .type_title = "Program"
        }
    };

    const auto security_profile = copperfin::security::default_native_security_profile();
    const auto extensibility_profile = copperfin::platform::default_extensibility_profile();
    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        security_profile,
        extensibility_profile,
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        true);
    expect(plan.ok, "generated launcher process package plan should be created");
    expect(plan.emit_dotnet_launcher, "generated launcher process plan should emit a .NET launcher");

    const auto materialized = copperfin::runtime::materialize_runtime_package(
        plan,
        security_profile,
        extensibility_profile,
        runtime_host_source.string());
    expect(materialized.ok,
           "generated launcher process package should materialize: " + materialized.error);
    if (!materialized.ok || failures != 0) {
        std::cerr << "fixture root: " << temp_root << "\n";
        return 1;
    }

    const fs::path package_root = materialized.plan.package_root;
    const fs::path release_manifest = materialized.plan.manifest_path;
    const fs::path debug_manifest = materialized.plan.debug_manifest_path;
    write_text(
        release_manifest,
        "manifest_version=1\n"
        "project_title=Generated Launcher Release\n"
        "startup_item=release startup.prg\n"
        "startup_source=release source path with spaces.prg\n");
    write_text(
        debug_manifest,
        "debug_manifest_version=2\n"
        "project_title=Generated Launcher Debug\n"
        "startup_item=debug startup.prg\n"
        "startup_source=debug source path with spaces.prg\n");

    const ProcessResult publish = run_process_capture(
        dotnet_path,
        {
            "publish",
            materialized.plan.launcher_project_path,
            "-c",
            "Release",
            "-r",
            runtime_identifier,
            "--self-contained",
            "false",
            "-o",
            package_root.string()
        },
        fs::path(materialized.plan.launcher_project_path).parent_path(),
        temp_root,
        "dotnet-publish",
        300000U);
    expect(publish.start_error == 0U, "dotnet publish should start for the generated launcher");
    expect(!publish.timed_out, "dotnet publish should not time out for the generated launcher");
    expect(
        publish.exit_code == 0,
        "dotnet publish should build the generated launcher\nstdout:\n" +
            publish.stdout_text + "\nstderr:\n" + publish.stderr_text);
    expect(fs::exists(materialized.plan.launcher_output_path),
           "dotnet publish should materialize the planned launcher executable");
    if (failures != 0) {
        std::cerr << "fixture root: " << temp_root << "\n";
        return 1;
    }

    copperfin::test_support::ScopedEnvironmentValue fixture_mode{
        std::string(fixture_environment)};
    fixture_mode.set("1");

    const fs::path launcher = materialized.plan.launcher_output_path;
    const ProcessResult ordinary = run_process_capture(
        launcher,
        {},
        caller_dir,
        temp_root,
        "ordinary",
        30000U);
    expect_manifest_selection(
        ordinary,
        release_manifest,
        "Generated Launcher Release",
        "release startup.prg",
        package_root,
        "ordinary launcher invocation");
    expect(
        line_value(ordinary.stdout_text, "fixture.argument_count=") == "2",
        "ordinary launcher invocation should forward only the manifest switch and release path");

    const std::string quoted_debug_command = "watch:\"alpha beta\"";
    const std::string trailing_separator_argument =
        (temp_root / "native argument path with spaces").string() + "\\";
    const ProcessResult long_debug = run_process_capture(
        launcher,
        {
            "--debug",
            "--debug-command",
            quoted_debug_command,
            "--source-path",
            trailing_separator_argument
        },
        caller_dir,
        temp_root,
        "long-debug",
        30000U);
    expect_manifest_selection(
        long_debug,
        debug_manifest,
        "Generated Launcher Debug",
        "debug startup.prg",
        package_root,
        "--debug launcher invocation");
    expect(
        line_value(long_debug.stdout_text, "fixture.argument[2]=") == "--debug",
        "--debug launcher invocation should preserve the normalized debug switch");
    expect(
        line_value(long_debug.stdout_text, "fixture.argument[4]=") == quoted_debug_command,
        "generated launcher should preserve embedded quotes in forwarded arguments");
    expect(
        line_value(long_debug.stdout_text, "fixture.argument[6]=") == trailing_separator_argument,
        "generated launcher should preserve spaced native paths ending in a separator");

    const ProcessResult slash_debug = run_process_capture(
        launcher,
        {"/debug", "/locale", "qps-ploc"},
        caller_dir,
        temp_root,
        "slash-debug",
        30000U);
    expect_manifest_selection(
        slash_debug,
        debug_manifest,
        "Generated Launcher Debug",
        "debug startup.prg",
        package_root,
        "/debug launcher invocation");
    expect(
        line_value(slash_debug.stdout_text, "fixture.argument[2]=") == "--debug",
        "/debug launcher invocation should normalize the debug switch before forwarding");
    expect(
        line_value(slash_debug.stdout_text, "fixture.argument[3]=") == "--locale",
        "/locale launcher invocation should normalize the locale switch before forwarding");
    expect(
        line_value(slash_debug.stdout_text, "fixture.argument[4]=") == "qps-ploc",
        "normalized locale forwarding should preserve the locale value");

    fs::remove(debug_manifest, ignored);
    expect(!ignored && !fs::exists(debug_manifest),
           "generated launcher process fixture should remove app.cfdebug for fallback coverage");
    const ProcessResult missing_debug = run_process_capture(
        launcher,
        {"--debug"},
        caller_dir,
        temp_root,
        "missing-debug",
        30000U);
    expect_manifest_selection(
        missing_debug,
        release_manifest,
        "Generated Launcher Release",
        "release startup.prg",
        package_root,
        "missing app.cfdebug launcher invocation");
    expect(
        line_value(missing_debug.stdout_text, "fixture.argument[2]=") == "--debug",
        "missing app.cfdebug fallback should still forward the normalized debug switch");

    const ProcessResult ordinary_after_fallback = run_process_capture(
        launcher,
        {},
        caller_dir,
        temp_root,
        "ordinary-after-fallback",
        30000U);
    expect_manifest_selection(
        ordinary_after_fallback,
        release_manifest,
        "Generated Launcher Release",
        "release startup.prg",
        package_root,
        "ordinary invocation after debug fallback");

    write_text(
        materialized.plan.runtime_host_destination_path,
        "not a valid Windows executable image\n");
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const std::string english_start_failure = copperfin::localization::load_catalogs(
        catalog_root,
        "en-US").translate("Runtime.Package.Launcher.Error.RuntimeHostStartFailed");
    const std::string portuguese_start_failure = copperfin::localization::load_catalogs(
        catalog_root,
        "pt-BR").translate("Runtime.Package.Launcher.Error.RuntimeHostStartFailed");
    const std::string pseudo_start_failure = copperfin::localization::load_catalogs(
        catalog_root,
        "qps-ploc").translate("Runtime.Package.Launcher.Error.RuntimeHostStartFailed");
    expect(!portuguese_start_failure.empty() &&
               portuguese_start_failure != english_start_failure,
           "invalid-host fixture should resolve a distinct pt-BR start failure");
    expect(!pseudo_start_failure.empty() && pseudo_start_failure != english_start_failure,
           "invalid-host fixture should resolve a pseudo-localized start failure");

    const ProcessResult portuguese_failure = run_process_capture(
        launcher,
        {"--locale", "pt-BR"},
        caller_dir,
        temp_root,
        "invalid-host-portuguese",
        30000U);
    expect(portuguese_failure.start_error == 0U,
           "generated launcher should start when its runtime-host image is invalid");
    expect(!portuguese_failure.timed_out,
           "invalid runtime-host launch should not time out");
    expect(portuguese_failure.exit_code == 5,
           "invalid runtime-host launch should preserve exit code 5");
    expect(portuguese_failure.stderr_text.find(portuguese_start_failure) != std::string::npos,
           "invalid runtime-host launch should emit the pt-BR localized start failure");
    expect(portuguese_failure.stderr_text.find("Exception") == std::string::npos,
           "invalid runtime-host launch should not leak an unhandled .NET exception");

    const ProcessResult pseudo_failure = run_process_capture(
        launcher,
        {"/locale", "qps-ploc"},
        caller_dir,
        temp_root,
        "invalid-host-pseudo",
        30000U);
    expect(pseudo_failure.exit_code == 5,
           "pseudo-localized invalid runtime-host launch should preserve exit code 5");
    expect(pseudo_failure.stderr_text.find(pseudo_start_failure) != std::string::npos,
           "invalid runtime-host launch should emit the pseudo-localized start failure");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
        std::cout << "All generated .NET launcher process tests passed\n";
        return 0;
    }

    std::cerr << failures << " generated launcher process test(s) failed\n";
    std::cerr << "fixture root: " << temp_root << "\n";
    return 1;
}

#endif

}  // namespace

int main(int argc, char** argv) {
    if (copperfin::test_support::getenv_value(std::string(fixture_environment)) == "1") {
        return run_recording_runtime_host(argc, argv);
    }

#if defined(_WIN32)
    if (argc < 2 || std::string(argv[1]).empty()) {
        std::cout << "SKIP: generated .NET launcher process test requires the Windows .NET SDK\n";
        return skip_return_code;
    }
    return run_generated_launcher_test(argv[1], argv);
#else
    (void)argc;
    (void)argv;
    std::cout << "SKIP: generated .NET launchers are currently available only on Windows\n";
    return skip_return_code;
#endif
}
