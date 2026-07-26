// Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/platform/extensibility_model.h"
#include "copperfin/runtime/runtime_pipeline.h"
#include "copperfin/studio/document_model.h"
#include "copperfin/studio/project_workspace.h"
#include "test_process_capture_support.h"
#include "copperfin/vfp/dbf_table.h"
#include "test_environment_support.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

namespace {

int failures = 0;

void expect(const bool condition, const std::string& message) {
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
    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

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
            lines.push_back(std::move(line));
        }
    }
    return lines;
}

std::filesystem::path find_dotnet_root(const std::filesystem::path& dotnet_path) {
    const auto configured = copperfin::platform::read_environment_path("DOTNET_ROOT");
    if (configured.has_value() &&
        std::filesystem::is_regular_file(*configured / "dotnet")) {
        return *configured;
    }
    const auto home = copperfin::platform::read_environment_path("HOME");
    if (!home.has_value()) {
        return {};
    }
    std::error_code iterator_error;
    for (std::filesystem::directory_iterator it(*home, iterator_error), end;
         it != end;
         it.increment(iterator_error)) {
        if (iterator_error || !it->is_directory(iterator_error)) {
            continue;
        }
        const std::string name = it->path().filename().string();
        if (name.rfind(".dotnet", 0U) != 0U) {
            continue;
        }
        const std::filesystem::path candidate = it->path() / "dotnet";
        if (std::filesystem::is_regular_file(candidate, iterator_error)) {
            return it->path();
        }
    }
    if (std::filesystem::is_regular_file(dotnet_path) &&
        read_text(dotnet_path).rfind("#!", 0U) != 0U) {
        return dotnet_path.parent_path();
    }
    return {};
}

void write_synthetic_project(
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
    const auto result = copperfin::vfp::create_dbf_table_file(
        project_path.string(),
        fields,
        {
            {"H", project_title, project_dir.string(),
             output_path.empty() ? "<Source>" : output_path.string(), "", "false"},
            {"K", "", "", "", "main program.prg", "true"}
        });
    expect(result.ok, "POSIX launcher test should create its synthetic PJX");
}

void expect_launcher_inventory(
    const std::filesystem::path& package_root,
    const std::filesystem::path& manifest,
    const std::filesystem::path& debug_manifest,
    const std::string& public_launcher_name) {
    const std::vector<std::string> required{
        public_launcher_name,
        "Copperfin.GeneratedLauncher.dll",
        "Copperfin.GeneratedLauncher.deps.json",
        "Copperfin.GeneratedLauncher.runtimeconfig.json"
    };
    for (const auto& name : required) {
        expect(std::filesystem::is_regular_file(package_root / name),
               "POSIX launcher package should contain " + name);
    }

    const auto runtime_inventory = manifest_lines_with_prefix(manifest, "launcher_artifact=");
    const auto debug_inventory = manifest_lines_with_prefix(debug_manifest, "launcher_artifact=");
    expect(runtime_inventory == debug_inventory,
           "POSIX runtime/debug manifests should preserve launcher inventory parity");
    expect(runtime_inventory.size() >= 4U && runtime_inventory.size() <= 5U,
           "POSIX launcher inventory should contain required artifacts and at most one PDB");
    for (const auto& name : required) {
        expect(std::any_of(runtime_inventory.begin(), runtime_inventory.end(), [&](const std::string& line) {
                   return line.find(name + "|") != std::string::npos;
               }),
               "POSIX launcher inventory should include " + name);
    }
}

int run_recording_runtime_host(int argc, char** argv) {
    if (argc < 3 || std::string_view(argv[1]) != "--manifest") {
        std::cerr << "recording runtime host requires --manifest and a manifest path\n";
        return 90;
    }
    const std::filesystem::path manifest = argv[2];
    std::cout << "fixture.status=ok\n";
    std::cout << "fixture.manifest.project_title="
              << manifest_value(manifest, "project_title") << "\n";
    std::cout << "fixture.manifest.startup_item="
              << manifest_value(manifest, "startup_item") << "\n";
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
#if defined(_WIN32)
    (void)argc;
    (void)argv;
    std::cout << "SKIP: POSIX generated launcher process test is not for Windows\n";
    return 77;
#else
    if (copperfin::test_support::getenv_value(
            "COPPERFIN_TEST_GENERATED_LAUNCHER_HOST") == "1") {
        return run_recording_runtime_host(argc, argv);
    }
    if (argc < 3 || argv[1] == nullptr || argv[2] == nullptr) {
        std::cout << "SKIP: POSIX generated launcher process test requires dotnet and build host paths\n";
        return 77;
    }

    const std::filesystem::path dotnet_path = argv[1];
    const std::filesystem::path build_host_path = argv[2];
    if (!std::filesystem::is_regular_file(dotnet_path) ||
        !std::filesystem::is_regular_file(build_host_path)) {
        std::cout << "SKIP: POSIX generated launcher process prerequisites are unavailable\n";
        return 77;
    }

    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() /
        ("copperfin generated launcher posix " +
         std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const fs::path project_dir = temp_root / "source project with spaces";
    const fs::path output_root = temp_root / "package output with spaces";
    const fs::path caller_dir = temp_root / "unrelated caller with spaces";
    const fs::path project_path = project_dir / "launcher project.pjx";
    const fs::path runtime_host_source = temp_root / "copperfin_runtime_host";
    const std::string project_title = "Generated Launcher POSIX Project";
    const std::string public_launcher_name = "Generated Launcher POSIX Project";
    std::error_code error;
    fs::remove_all(temp_root, error);
    fs::create_directories(project_dir);
    fs::create_directories(output_root);
    fs::create_directories(caller_dir);

    const fs::path test_executable = fs::weakly_canonical(fs::absolute(argv[0]), error);
    expect(!error && fs::is_regular_file(test_executable),
           "POSIX launcher test should resolve its own executable");
    error.clear();
    fs::copy_file(test_executable, runtime_host_source, fs::copy_options::overwrite_existing, error);
    expect(!error, "POSIX launcher test should stage its recording runtime host source");
    error.clear();
    fs::permissions(
        runtime_host_source,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add,
        error);
    expect(!error, "POSIX launcher test should preserve executable runtime-host permissions");

    write_text(project_dir / "main program.prg", "RETURN\n");
    write_synthetic_project(
        project_path,
        project_dir,
        {},
        project_title);

    copperfin::test_support::ScopedEnvironmentValue fixture_mode{
        "COPPERFIN_TEST_GENERATED_LAUNCHER_HOST",
        "1"};
    const fs::path dotnet_root = find_dotnet_root(dotnet_path);
    if (dotnet_root.empty()) {
        std::cout << "SKIP: POSIX generated launcher runtime root is unavailable\n";
        fs::remove_all(temp_root, error);
        return 77;
    }
    copperfin::test_support::ScopedEnvironmentValue dotnet_root_environment{
        "DOTNET_ROOT",
        dotnet_root.string()};
    copperfin::test_support::ScopedEnvironmentValue dotnet_root_x64_environment{
        "DOTNET_ROOT_X64",
        dotnet_root.string()};
    copperfin::test_support::ScopedEnvironmentValue dotnet_roll_forward{
        "DOTNET_ROLL_FORWARD",
        "Major"};
    const auto build = copperfin::test_support::run_process_capture(
        build_host_path,
        {
            "build",
            "--project", project_path.string(),
            "--output-dir", output_root.string(),
            "--runtime-host", runtime_host_source.string(),
            "--emit-dotnet-launcher"
        },
        caller_dir);
    expect(build.started, "POSIX build host launcher invocation should start");
    expect(build.exit_code == 0,
           "POSIX build host launcher invocation should succeed\nstdout:\n" +
               build.stdout_text + "\nstderr:\n" + build.stderr_text);
    if (build.exit_code != 0 || !build.started) {
        std::cerr << "build stdout:\n" << build.stdout_text
                  << "build stderr:\n" << build.stderr_text << "\n";
    }
    if (failures != 0) {
        fs::remove_all(temp_root, error);
        return 1;
    }

    const fs::path package_root = output_root / "Generated_Launcher_POSIX_Project";
    const fs::path launcher = package_root / public_launcher_name;
    const fs::path manifest = line_value(build.stdout_text, "manifest.path: ");
    const fs::path debug_manifest = line_value(build.stdout_text, "debug.manifest.path: ");
    expect(fs::is_regular_file(launcher), "POSIX build host should publish the configured apphost name");
    expect(manifest_value(manifest, "manifest_version") == "3",
           "POSIX generated launcher should retain the versioned runtime manifest");
    expect(manifest_value(debug_manifest, "debug_manifest_version") == "3",
           "POSIX generated launcher should retain the versioned debug manifest");
    expect_launcher_inventory(package_root, manifest, debug_manifest, public_launcher_name);
    error.clear();
    const auto staged_runtime_host_status = fs::status(
        package_root / "copperfin_runtime_host", error);
    expect(!error &&
               (staged_runtime_host_status.permissions() &
                (fs::perms::owner_exec |
                 fs::perms::group_exec |
                 fs::perms::others_exec)) != fs::perms::none,
           "POSIX launcher package should preserve executable runtime-host permissions");

    const auto launched = copperfin::test_support::run_process_capture(
        launcher,
        {},
        caller_dir);
    if (!launched.started || launched.exit_code != 0) {
        const fs::path staged_runtime_host = package_root / "copperfin_runtime_host";
        const auto staged_status = fs::status(staged_runtime_host, error);
        std::cerr << "launcher exit=" << launched.exit_code
                  << " started=" << launched.started
                  << " launch_error=" << launched.launch_error
                  << " staged_host=" << staged_runtime_host
                  << " staged_exists=" << fs::exists(staged_runtime_host)
                  << " staged_permissions=" << static_cast<unsigned int>(staged_status.permissions())
                  << "\nlauncher stdout:\n" << launched.stdout_text
                  << "launcher stderr:\n" << launched.stderr_text << "\n";
    }
    expect(launched.started && launched.exit_code == 0,
           "POSIX generated launcher should start its staged runtime host");
    expect(launched.stdout_text.find("fixture.status=ok\n") != std::string::npos,
           "POSIX generated launcher should reach the recording runtime host");
    expect(launched.stdout_text.find("fixture.manifest.project_title=" + project_title) != std::string::npos,
           "POSIX generated launcher should select the package manifest");

    const fs::path runtime_config = package_root / "Copperfin.GeneratedLauncher.runtimeconfig.json";
    const std::string original_runtime_config = read_text(runtime_config);
    error.clear();
    fs::remove(runtime_config, error);
    expect(!error, "POSIX launcher test should remove a required sidecar for tamper coverage");
    if (!error) {
        const auto tampered = copperfin::test_support::run_process_capture(
            launcher,
            {},
            caller_dir);
        expect(tampered.started && tampered.exit_code != 0,
               "POSIX generated launcher should reject a missing runtimeconfig sidecar");
        expect(tampered.stdout_text.find("fixture.status=ok\n") == std::string::npos,
               "POSIX sidecar rejection should occur before the recording runtime host starts");
        write_text(runtime_config, original_runtime_config);
    }

    fs::remove_all(temp_root, error);
    if (failures == 0) {
        std::cout << "All POSIX generated .NET launcher process tests passed\n";
        return 0;
    }
    std::cerr << failures << " POSIX generated launcher test(s) failed\n";
    return 1;
#endif
}
