// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/localization/localization.h"
#include "copperfin/platform/extensibility_model.h"
#include "copperfin/runtime/runtime_pipeline.h"
#include "copperfin/security/security_model.h"
#include "copperfin/studio/document_model.h"
#include "copperfin/studio/project_workspace.h"
#include "copperfin/vfp/dbf_table.h"
#include "test_environment_support.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <thread>
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

void write_synthetic_executable_project(
    const std::filesystem::path& project_path,
    const std::filesystem::path& project_dir,
    const std::filesystem::path& output_path,
    const std::string& project_title) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "TYPE", .type = 'C', .length = 1U},
        {.name = "KEY", .type = 'C', .length = 32U},
        {.name = "HOMEDIR", .type = 'C', .length = 200U},
        {.name = "OUTFILE", .type = 'C', .length = 200U},
        {.name = "NAME", .type = 'C', .length = 200U},
        {.name = "MAINPROG", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"H", project_title, project_dir.string(), output_path.string(), "", "false"},
        {"K", "", "", "", "main.prg", "true"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        project_path.string(),
        fields,
        records);
    expect(create_result.ok,
           "generated launcher process test should create its synthetic PJX fixture");
}

std::string read_text(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return std::string(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>());
}

std::vector<std::string> manifest_lines_with_prefix(
    const std::filesystem::path& path,
    const std::string& prefix) {
    std::vector<std::string> lines;
    std::ifstream input(path, std::ios::binary);
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.rfind(prefix, 0U) == 0U) {
            lines.push_back(line);
        }
    }
    return lines;
}

// finalize_runtime_package_primary_output() (via
// inventory_generated_launcher_artifacts()) has already opened, read, and
// verified each of these sidecar files by the time it returns -- but on
// Windows CI, a `PackageRootTransaction` commit that promotes files from a
// staging location into package_root has been observed to leave a brief
// window where those same files are not yet visible to a fresh
// std::filesystem::is_regular_file() check immediately afterward, even
// though the finalize call itself reported success. Poll for up to ~5
// seconds rather than checking once, so this test tolerates that window
// instead of failing on it; a check that never becomes true after the full
// polling period is still a real failure, not silently swallowed.
bool wait_until_regular_files_exist(
    const std::filesystem::path& package_root,
    const std::vector<std::string>& required_sidecars,
    const int max_attempts = 20,
    const std::chrono::milliseconds delay = std::chrono::milliseconds(250)) {
    for (int attempt = 0; attempt < max_attempts; ++attempt) {
        const bool all_present = std::all_of(
            required_sidecars.begin(),
            required_sidecars.end(),
            [&](const std::string& name) {
                return std::filesystem::is_regular_file(package_root / name);
            });
        if (all_present) {
            return true;
        }
        if (attempt + 1 < max_attempts) {
            std::this_thread::sleep_for(delay);
        }
    }
    return false;
}

void expect_launcher_artifact_inventory(
    const std::filesystem::path& package_root,
    const std::filesystem::path& runtime_manifest,
    const std::filesystem::path& debug_manifest,
    const std::string& public_apphost_name,
    const std::string& context) {
    const std::vector<std::string> required_sidecars{
#if defined(_WIN32)
        "Copperfin.GeneratedLauncher.apphost.exe",
#endif
        "Copperfin.GeneratedLauncher.dll",
        "Copperfin.GeneratedLauncher.deps.json",
        "Copperfin.GeneratedLauncher.runtimeconfig.json"
    };
    wait_until_regular_files_exist(package_root, required_sidecars);
    for (const auto& required_sidecar : required_sidecars) {
        expect(std::filesystem::is_regular_file(package_root / required_sidecar),
               context + " should publish required sidecar " + required_sidecar);
    }

    const auto runtime_inventory =
        manifest_lines_with_prefix(runtime_manifest, "launcher_artifact=");
    const auto debug_inventory =
        manifest_lines_with_prefix(debug_manifest, "launcher_artifact=");
    expect(runtime_inventory == debug_inventory,
           context + " should preserve identical runtime/debug launcher provenance");
    expect(
#if defined(_WIN32)
               5U <= runtime_inventory.size() && runtime_inventory.size() <= 6U,
#else
               4U <= runtime_inventory.size() && runtime_inventory.size() <= 5U,
#endif
        context + " should inventory the public launcher, required runtime artifacts, and at most one optional PDB");

    const auto contains = [&](const std::string& marker) {
        return std::any_of(runtime_inventory.begin(), runtime_inventory.end(), [&](const std::string& line) {
            return line.find(marker) != std::string::npos;
        });
    };
    expect(std::any_of(runtime_inventory.begin(), runtime_inventory.end(), [&](const std::string& line) {
               return line.find(public_apphost_name) != std::string::npos &&
                   line.find("|public_apphost|") != std::string::npos;
           }),
           context + " should inventory the configured public apphost");
    for (const auto& required_sidecar : required_sidecars) {
        expect(contains(required_sidecar + "|runtime_required|"),
               context + " should classify required sidecar " + required_sidecar);
    }

    const bool pdb_exists = std::filesystem::is_regular_file(
        package_root / "Copperfin.GeneratedLauncher.pdb");
    expect(contains("Copperfin.GeneratedLauncher.pdb|debug_optional|") == pdb_exists,
           context + " should classify the optional PDB exactly when the SDK publishes it");
}

void expect_ambient_msbuild_customizations_ignored(
    const std::filesystem::path& package_root,
    const std::string& context) {
    expect(!std::filesystem::exists(package_root / "Copperfin.GeneratedLauncher.xml"),
           context + " should ignore ambient documentation-file settings");
    expect(!std::filesystem::exists(package_root / "Copperfin.GeneratedLauncher.ambient-target"),
           context + " should ignore ambient Directory.Build.targets output");
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

int run_generated_launcher_test(
    const std::filesystem::path& dotnet_path,
    const std::filesystem::path& build_host_path,
    char** argv) {
    namespace fs = std::filesystem;

    if (!fs::exists(dotnet_path)) {
        std::cout << "SKIP: generated .NET launcher process test requires the Windows .NET SDK\n";
        return skip_return_code;
    }
    if (!fs::exists(build_host_path)) {
        std::cerr << "FAIL: generated launcher process test requires the Copperfin build host\n";
        return 1;
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
    write_text(
        temp_root / "Directory.Build.props",
        "<Project>\n"
        "  <PropertyGroup>\n"
        "    <GenerateDocumentationFile>true</GenerateDocumentationFile>\n"
        "  </PropertyGroup>\n"
        "</Project>\n");
    write_text(
        temp_root / "Directory.Build.targets",
        "<Project>\n"
        "  <Target Name=\"EmitAmbientLauncherSidecar\" AfterTargets=\"Publish\">\n"
        "    <WriteLinesToFile File=\"$(PublishDir)Copperfin.GeneratedLauncher.ambient-target\" "
        "Lines=\"ambient\" Overwrite=\"true\" />\n"
        "  </Target>\n"
        "</Project>\n");
    write_text(
        temp_root / "Directory.Build.rsp",
        "-p:GenerateDocumentationFile=true\n");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "launcher process.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "Generated Launcher Project";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = workspace.project_title;
    const std::string direct_launcher_name = "Copperfin.GeneratedLauncher.exe";
    workspace.build_plan.output_path =
        (output_dir / direct_launcher_name).string();
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
    const ProcessResult publish = run_process_capture(
        dotnet_path,
        {
            "publish",
            materialized.plan.launcher_project_path,
            "-noAutoResponse",
            "-p:ImportDirectoryBuildProps=false",
            "-p:ImportDirectoryBuildTargets=false",
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
#if defined(_WIN32)
    const fs::path internal_apphost = package_root / "Copperfin.GeneratedLauncher.apphost.exe";
    const fs::path launcher_guard = build_host_path.parent_path() / "copperfin_launcher_guard.exe";
    const fs::path published_apphost = package_root / direct_launcher_name;
    expect(fs::is_regular_file(launcher_guard),
           "generated launcher process test should have the native launcher guard beside the build host");
    ignored.clear();
    fs::rename(published_apphost, internal_apphost, ignored);
    expect(!ignored && fs::is_regular_file(internal_apphost),
           "direct SDK publication should move the managed apphost to its stable internal identity");
    ignored.clear();
    fs::copy_file(
        launcher_guard,
        materialized.plan.launcher_output_path,
        fs::copy_options::overwrite_existing,
        ignored);
    expect(!ignored && fs::is_regular_file(materialized.plan.launcher_output_path),
           "direct SDK publication should materialize the configured native launcher guard");
#endif
    expect(fs::exists(materialized.plan.launcher_output_path),
           "dotnet publish should materialize the planned launcher executable");
    expect(
        fs::path(materialized.plan.launcher_output_path).filename() == direct_launcher_name,
        "direct dotnet publish should preserve the configured public launcher filename");
    expect(!fs::exists(package_root / "Generated_Launcher_Project.exe"),
           "dotnet publish should not leave a title-derived alternate launcher");
    if (failures != 0) {
        std::cerr << "fixture root: " << temp_root << "\n";
        return 1;
    }

    const auto finalized = copperfin::runtime::finalize_runtime_package_primary_output(
        materialized.plan,
        security_profile,
        extensibility_profile);
    expect(finalized.ok,
           "configured generated launcher output should finalize: " + finalized.error);
    expect(finalized.plan.primary_output_materialized,
           "configured generated launcher finalization should record materialized output");
    expect(manifest_value(release_manifest, "manifest_version") == "3" &&
               manifest_value(release_manifest, "primary_output_path").empty(),
           "configured launcher finalization should preserve the runtime manifest contract");
    expect(manifest_value(debug_manifest, "debug_manifest_version") == "3" &&
               manifest_value(debug_manifest, "primary_output_materialized") == "true" &&
               manifest_value(debug_manifest, "primary_output_path").find(
                   direct_launcher_name) != std::string::npos,
           "configured launcher finalization should retain output provenance in app.cfdebug");
    expect_launcher_artifact_inventory(
        package_root,
        release_manifest,
        debug_manifest,
        direct_launcher_name,
        "direct SDK launcher publication");
    expect_ambient_msbuild_customizations_ignored(
        package_root,
        "direct SDK launcher publication");
    if (!finalized.ok || failures != 0) {
        std::cerr << "fixture root: " << temp_root << "\n";
        return 1;
    }

    const fs::path launcher = materialized.plan.launcher_output_path;
    const fs::path launcher_dll = package_root / "Copperfin.GeneratedLauncher.dll";
    const fs::path launcher_deps = package_root / "Copperfin.GeneratedLauncher.deps.json";
    const fs::path launcher_runtime_config = package_root / "Copperfin.GeneratedLauncher.runtimeconfig.json";
    const std::string original_launcher_dll = read_text(launcher_dll);
    const std::string original_launcher_deps = read_text(launcher_deps);
    const std::string original_runtime_config = read_text(launcher_runtime_config);
    const fs::path missing_launcher_deps = launcher_deps.string() + ".missing";
    write_text(launcher_dll, "tampered launcher sidecar\n");
    const ProcessResult tampered_sidecar = run_process_capture(
        launcher,
        {},
        caller_dir,
        temp_root,
        "tampered-launcher-sidecar",
        30000U);
    expect(tampered_sidecar.start_error == 0U && !tampered_sidecar.timed_out,
           "tampered launcher sidecar verification should start and finish");
    expect(tampered_sidecar.exit_code == 4 && tampered_sidecar.stdout_text.empty(),
           "tampered launcher sidecar should be rejected before the managed apphost starts");
    write_text(launcher_dll, original_launcher_dll);

    write_text(launcher_deps, "tampered launcher dependency manifest\n");
    const ProcessResult tampered_deps = run_process_capture(
        launcher,
        {},
        caller_dir,
        temp_root,
        "tampered-launcher-deps",
        30000U);
    expect(tampered_deps.start_error == 0U && !tampered_deps.timed_out,
           "tampered launcher dependency manifest verification should start and finish");
    expect(tampered_deps.exit_code == 4 && tampered_deps.stdout_text.empty(),
           "tampered launcher dependency manifest should be rejected before the managed apphost starts");
    write_text(launcher_deps, original_launcher_deps);

    write_text(launcher_runtime_config, "tampered launcher runtime configuration\n");
    const ProcessResult tampered_runtime_config = run_process_capture(
        launcher,
        {},
        caller_dir,
        temp_root,
        "tampered-launcher-runtimeconfig",
        30000U);
    expect(tampered_runtime_config.start_error == 0U && !tampered_runtime_config.timed_out,
           "tampered launcher runtime configuration verification should start and finish");
    expect(tampered_runtime_config.exit_code == 4 && tampered_runtime_config.stdout_text.empty(),
           "tampered launcher runtime configuration should be rejected before the managed apphost starts");
    write_text(launcher_runtime_config, original_runtime_config);

    ignored.clear();
    fs::rename(launcher_deps, missing_launcher_deps, ignored);
    expect(!ignored, "generated launcher process fixture should temporarily remove a required sidecar");
    if (!ignored) {
        const ProcessResult missing_sidecar = run_process_capture(
            launcher,
            {},
            caller_dir,
            temp_root,
            "missing-launcher-sidecar",
            30000U);
        expect(missing_sidecar.start_error == 0U && !missing_sidecar.timed_out,
               "missing launcher sidecar verification should start and finish");
        expect(missing_sidecar.exit_code == 4 && missing_sidecar.stdout_text.empty(),
               "missing launcher sidecar should be rejected before the managed apphost starts");
        ignored.clear();
        fs::rename(missing_launcher_deps, launcher_deps, ignored);
        expect(!ignored, "generated launcher process fixture should restore its required sidecar");
    }

    const fs::path redirected_sidecar = launcher_runtime_config;
    const fs::path redirected_target = temp_root / "redirected-runtimeconfig.json";
    write_text(redirected_target, original_runtime_config);
    ignored.clear();
    fs::remove(redirected_sidecar, ignored);
    ignored.clear();
    fs::create_symlink(redirected_target, redirected_sidecar, ignored);
    if (!ignored) {
        const ProcessResult redirected = run_process_capture(
            launcher,
            {},
            caller_dir,
            temp_root,
            "redirected-launcher-sidecar",
            30000U);
        expect(redirected.start_error == 0U && !redirected.timed_out,
               "redirected launcher sidecar verification should start and finish");
        expect(redirected.exit_code == 4 && redirected.stdout_text.empty(),
               "redirected launcher sidecar should be rejected before the managed apphost starts");
        fs::remove(redirected_sidecar, ignored);
        write_text(redirected_sidecar, original_runtime_config);
    } else {
        write_text(redirected_sidecar, original_runtime_config);
    }

    const fs::path trust_envelope = package_root / "app.cftrust";
    const fs::path trust_signature = package_root / "app.cftrust.sig";
    write_text(trust_envelope, "launcher_inventory_version=1\nmalformed=true\n");
    write_text(trust_signature, "launcher_signature_version=1\n");
    const ProcessResult malformed_trust = run_process_capture(
        launcher,
        {},
        caller_dir,
        temp_root,
        "malformed-launcher-trust",
        30000U);
    expect(malformed_trust.start_error == 0U && !malformed_trust.timed_out,
           "malformed launcher trust metadata should be rejected cleanly");
    expect(malformed_trust.exit_code == 4 && malformed_trust.stdout_text.empty(),
           "malformed launcher trust metadata should be rejected before the managed apphost starts");
    fs::remove(trust_envelope, ignored);
    fs::remove(trust_signature, ignored);

    const auto finalized_launcher_inventory =
        manifest_lines_with_prefix(release_manifest, "launcher_artifact=");
    std::string launcher_inventory_suffix;
    for (const auto& line : finalized_launcher_inventory) {
        launcher_inventory_suffix += line;
        launcher_inventory_suffix.push_back('\n');
    }
    write_text(
        release_manifest,
        "manifest_version=1\n"
        "project_title=Generated Launcher Release\n"
        "startup_item=release startup.prg\n"
        "startup_source=release source path with spaces.prg\n" +
            launcher_inventory_suffix);
    write_text(
        debug_manifest,
        "debug_manifest_version=2\n"
        "project_title=Generated Launcher Debug\n"
        "startup_item=debug startup.prg\n"
        "startup_source=debug source path with spaces.prg\n" +
            launcher_inventory_suffix);

    copperfin::test_support::ScopedEnvironmentValue fixture_mode{
        std::string(fixture_environment)};
    fixture_mode.set("1");

    const fs::path build_host_project_dir = temp_root / "build host source project with spaces";
    const fs::path build_host_output_root = temp_root / "build host package output with spaces";
    const fs::path build_host_project_path = build_host_project_dir / "custom launcher project.pjx";
    const std::string build_host_project_title = "Build Host Launcher Project";
    const fs::path build_host_package_root =
        build_host_output_root / "Build_Host_Launcher_Project";
    const std::string configured_launcher_name =
        "Configured $(Configuration) @(Items); 100% & Launcher.exe";
    const fs::path build_host_launcher = build_host_package_root / configured_launcher_name;
    fs::create_directories(build_host_project_dir);
    fs::create_directories(build_host_output_root);
    write_text(build_host_project_dir / "main.prg", "RETURN\n");
    write_synthetic_executable_project(
        build_host_project_path,
        build_host_project_dir,
        fs::path(configured_launcher_name),
        build_host_project_title);

    const ProcessResult build_host = run_process_capture(
        build_host_path,
        {
            "build",
            "--project",
            build_host_project_path.string(),
            "--output-dir",
            build_host_output_root.string(),
            "--runtime-host",
            runtime_host_source.string(),
            "--emit-dotnet-launcher"
        },
        caller_dir,
        temp_root,
        "build-host-custom-launcher",
        300000U);
    expect(build_host.start_error == 0U,
           "custom-output build host invocation should start");
    expect(!build_host.timed_out,
           "custom-output build host invocation should not time out");
    expect(
        build_host.exit_code == 0,
        "custom-output build host invocation should succeed\nstdout:\n" +
            build_host.stdout_text + "\nstderr:\n" + build_host.stderr_text);
    expect(line_value(build_host.stdout_text, "status: ") == "ok" &&
               line_value(build_host.stdout_text, "primary.output.materialized: ") == "true",
           "custom-output build host invocation should preserve invariant success fields");
    expect(fs::exists(build_host_launcher) &&
               paths_are_equivalent(
                   line_value(build_host.stdout_text, "launcher.output: "),
                   build_host_launcher),
           "build host should report and materialize the configured launcher path");
    expect(!fs::exists(build_host_package_root / "Build_Host_Launcher_Project.exe") &&
               !fs::exists(build_host_package_root / "Copperfin.GeneratedLauncher.exe"),
           "build host should not retain title-derived or legacy alternate launchers");

    const fs::path build_host_manifest =
        line_value(build_host.stdout_text, "manifest.path: ");
    const fs::path build_host_debug_manifest =
        line_value(build_host.stdout_text, "debug.manifest.path: ");
    expect(manifest_value(build_host_manifest, "manifest_version") == "3" &&
               manifest_value(build_host_manifest, "primary_output_path").empty(),
           "custom-output build should preserve the runtime manifest schema");
    expect(manifest_value(build_host_debug_manifest, "debug_manifest_version") == "3" &&
               manifest_value(build_host_debug_manifest, "primary_output_materialized") == "true" &&
               manifest_value(build_host_debug_manifest, "primary_output_path").find(
                   configured_launcher_name) != std::string::npos,
           "custom-output build should preserve output provenance in app.cfdebug");
    expect_launcher_artifact_inventory(
        build_host_package_root,
        build_host_manifest,
        build_host_debug_manifest,
        configured_launcher_name,
        "build-host SDK launcher publication");
    expect_ambient_msbuild_customizations_ignored(
        build_host_package_root,
        "build-host SDK launcher publication");
    if (failures != 0) {
        std::cerr << "fixture root: " << temp_root << "\n";
        return 1;
    }

    const ProcessResult built_launcher = run_process_capture(
        build_host_launcher,
        {},
        caller_dir,
        temp_root,
        "build-host-generated-launcher",
        30000U);
    expect_manifest_selection(
        built_launcher,
        build_host_manifest,
        build_host_project_title,
        "main.prg",
        build_host_package_root,
        "build-host generated custom launcher invocation");

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

    const HANDLE runtime_host_lock = CreateFileW(
        fs::path(materialized.plan.runtime_host_destination_path).c_str(),
        GENERIC_READ,
        0U,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    expect(runtime_host_lock != INVALID_HANDLE_VALUE,
           "generated launcher process fixture should lock the runtime host exclusively");
    if (runtime_host_lock == INVALID_HANDLE_VALUE) {
        std::cerr << "fixture root: " << temp_root << "\n";
        return 1;
    }
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
           "unstartable-host fixture should resolve a distinct pt-BR start failure");
    expect(!pseudo_start_failure.empty() && pseudo_start_failure != english_start_failure,
           "unstartable-host fixture should resolve a pseudo-localized start failure");

    const ProcessResult portuguese_failure = run_process_capture(
        launcher,
        {"--locale", "pt-BR"},
        caller_dir,
        temp_root,
        "unstartable-host-portuguese",
        30000U);
    expect(portuguese_failure.start_error == 0U,
           "generated launcher should start when its runtime host is unstartable");
    expect(!portuguese_failure.timed_out,
           "unstartable runtime-host launch should not time out");
    expect(portuguese_failure.exit_code == 5,
           "unstartable runtime-host launch should preserve exit code 5");
    expect(portuguese_failure.stderr_text.find(portuguese_start_failure) != std::string::npos,
           "unstartable runtime-host launch should emit the pt-BR localized start failure");
    expect(portuguese_failure.stderr_text.find("Exception") == std::string::npos,
           "unstartable runtime-host launch should not leak an unhandled .NET exception");

    const ProcessResult portuguese_posix_argument_failure = run_process_capture(
        launcher,
        {"--locale", "pt_BR.UTF-8"},
        caller_dir,
        temp_root,
        "unstartable-host-portuguese-posix-argument",
        30000U);
    expect(
        portuguese_posix_argument_failure.exit_code == 5 &&
            portuguese_posix_argument_failure.stderr_text.find(portuguese_start_failure) != std::string::npos,
        "generated launcher should normalize pt_BR.UTF-8 from an explicit locale argument");

    {
        copperfin::test_support::ScopedEnvironmentValue locale(
            "COPPERFIN_LOCALE",
            "pt_BR@modifier");
        const ProcessResult portuguese_posix_environment_failure = run_process_capture(
            launcher,
            {},
            caller_dir,
            temp_root,
            "unstartable-host-portuguese-posix-environment",
            30000U);
        expect(
            portuguese_posix_environment_failure.exit_code == 5 &&
                portuguese_posix_environment_failure.stderr_text.find(portuguese_start_failure) != std::string::npos,
            "generated launcher should normalize pt_BR@modifier from COPPERFIN_LOCALE");
    }

    const ProcessResult pseudo_failure = run_process_capture(
        launcher,
        {"/locale", "qps-ploc"},
        caller_dir,
        temp_root,
        "unstartable-host-pseudo",
        30000U);
    expect(pseudo_failure.exit_code == 5,
           "pseudo-localized unstartable runtime-host launch should preserve exit code 5");
    expect(pseudo_failure.stderr_text.find(pseudo_start_failure) != std::string::npos,
           "unstartable runtime-host launch should emit the pseudo-localized start failure");
    CloseHandle(runtime_host_lock);

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
    if (argc < 3 || std::string(argv[1]).empty()) {
        std::cout << "SKIP: generated .NET launcher process test requires the Windows .NET SDK\n";
        return skip_return_code;
    }
    return run_generated_launcher_test(argv[1], argv[2], argv);
#else
    (void)argc;
    (void)argv;
    std::cout << "SKIP: generated .NET launchers are currently available only on Windows\n";
    return skip_return_code;
#endif
}
