#include "copperfin/platform/extensibility_model.h"
#include "copperfin/runtime/runtime_pipeline.h"
#include "copperfin/security/security_model.h"
#include "copperfin/studio/project_workspace.h"
#include "copperfin/vfp/dbf_table.h"

#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <system_error>
#include <unordered_map>
#include <vector>

#if defined(_WIN32)
#include <process.h>
#else
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void write_text(const std::filesystem::path& path, const std::string& contents) {
    std::ofstream output(path, std::ios::binary);
    output << contents;
}

std::filesystem::path runtime_host_fixture_path(const std::filesystem::path& root) {
#if defined(_WIN32)
    return root / "copperfin_runtime_host.exe";
#else
    return root / "copperfin_runtime_host";
#endif
}

std::string read_text(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    std::ostringstream stream;
    stream << input.rdbuf();
    return stream.str();
}

std::string hex_decode_bytes(const std::string& encoded) {
    std::string bytes;
    bytes.reserve(encoded.size() / 2U);
    for (std::size_t index = 0; index + 1U < encoded.size(); index += 2U) {
        const std::string chunk = encoded.substr(index, 2U);
        bytes.push_back(static_cast<char>(std::strtoul(chunk.c_str(), nullptr, 16)));
    }
    return bytes;
}

std::unordered_map<std::string, std::string> parse_app_archive_payloads(const std::string& archive_text) {
    std::unordered_map<std::string, std::string> payloads;
    std::istringstream input(archive_text);
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.rfind("payload=", 0U) != 0U) {
            continue;
        }

        const std::string payload = line.substr(8U);
        const std::size_t separator = payload.find('|');
        if (separator == std::string::npos) {
            continue;
        }
        payloads.emplace(payload.substr(0U, separator), hex_decode_bytes(payload.substr(separator + 1U)));
    }
    return payloads;
}

std::string trim_copy(std::string value) {
    const auto is_space = [](unsigned char ch) {
        return std::isspace(ch) != 0;
    };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [&](unsigned char ch) {
        return !is_space(ch);
    }));
    while (!value.empty() && is_space(static_cast<unsigned char>(value.back()))) {
        value.pop_back();
    }
    return value;
}

std::string quote_manifest_value(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (const char ch : value) {
        if (ch == '\\') {
            escaped += "\\\\";
        } else if (ch == '\n') {
            escaped += "\\n";
        } else if (ch == '\r') {
            escaped += "\\r";
        } else {
            escaped.push_back(ch);
        }
    }
    return escaped;
}

std::vector<std::string> lines_with_prefix(const std::string& text, const std::string& prefix) {
    std::vector<std::string> matches;
    std::istringstream input(text);
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.rfind(prefix, 0U) == 0U) {
            matches.push_back(line);
        }
    }
    return matches;
}

std::string getenv_value(const std::string& name) {
#if defined(_WIN32)
    char* raw = nullptr;
    std::size_t length = 0;
    if (_dupenv_s(&raw, &length, name.c_str()) != 0 || raw == nullptr) {
        return {};
    }
    const std::string value = raw;
    std::free(raw);
    return value;
#else
    const char* raw = std::getenv(name.c_str());
    if (raw == nullptr) {
        return {};
    }
    return raw;
#endif
}

void set_env_variable(const std::string& name, const std::string& value, bool has_value) {
#if defined(_WIN32)
    if (has_value) {
        _putenv_s(name.c_str(), value.c_str());
    } else {
        _putenv_s((name + "=").c_str(), "");
    }
#else
    if (has_value) {
        setenv(name.c_str(), value.c_str(), 1);
    } else {
        unsetenv(name.c_str());
    }
#endif
}

struct ScopedEnvironmentVariable {
    std::string name;
    std::string original;
    bool had_original = false;

    explicit ScopedEnvironmentVariable(const std::string& var_name, const std::string& value)
        : name(var_name),
          original(getenv_value(name)) {
        had_original = !original.empty();
        set_env_variable(name, value, true);
    }

    explicit ScopedEnvironmentVariable(const std::string& var_name)
        : name(var_name),
          original(getenv_value(name)) {
        had_original = !original.empty();
        set_env_variable(name, "", false);
    }

    ~ScopedEnvironmentVariable() {
        set_env_variable(name, original, had_original);
    }
};

bool dotnet_is_available() {
#if defined(_WIN32)
    const char* argv[] = {"dotnet", "--version", nullptr};
    return _spawnvp(_P_WAIT, "dotnet", const_cast<char* const*>(argv)) == 0;
#else
    return std::system("command -v dotnet >/dev/null 2>&1") == 0;
#endif
}

std::string native_cxx_command() {
    const char* configured = std::getenv("CXX");
    if (configured != nullptr) {
        const std::string value(configured);
        if (!value.empty()) {
            return value;
        }
    }
#if defined(_WIN32)
    return "c++";
#else
    return "c++";
#endif
}

bool native_cxx_is_available() {
#if defined(_WIN32)
    std::vector<std::string> args = {native_cxx_command(), "--version"};
    std::vector<const char*> argv;
    argv.reserve(args.size() + 1U);
    for (const auto& arg : args) {
        argv.push_back(arg.c_str());
    }
    argv.push_back(nullptr);
    return _spawnvp(_P_WAIT, native_cxx_command().c_str(), const_cast<char* const*>(argv.data())) == 0;
#else
    return std::system(("command -v " + native_cxx_command() + " >/dev/null 2>&1").c_str()) == 0;
#endif
}

bool native_symbol_dump_is_available() {
#if defined(_WIN32)
    return false;
#else
    return std::system("command -v nm >/dev/null 2>&1") == 0;
#endif
}

bool cmake_is_available() {
#if defined(_WIN32)
    std::vector<std::string> args = {"cmake", "--version"};
    std::vector<const char*> argv;
    argv.reserve(args.size() + 1U);
    for (const auto& arg : args) {
        argv.push_back(arg.c_str());
    }
    argv.push_back(nullptr);
    return _spawnvp(_P_WAIT, "cmake", const_cast<char* const*>(argv.data())) == 0;
#else
    return std::system("command -v cmake >/dev/null 2>&1") == 0;
#endif
}

bool shell_is_available() {
#if defined(_WIN32)
    return false;
#else
    return std::system("command -v sh >/dev/null 2>&1") == 0;
#endif
}

bool compile_native_wrapper_scaffold(
    const std::filesystem::path& source_path,
    std::filesystem::path& output_path,
    std::string& error) {
    namespace fs = std::filesystem;
    const fs::path compile_root = source_path.parent_path() / "native_wrapper_compile_check";
    output_path =
#if defined(_WIN32)
        compile_root / "wrapper_smoke.dll";
#else
        compile_root / "libwrapper_smoke.so";
#endif
    const fs::path build_log_path = compile_root / "native-wrapper-build.log";
    std::error_code ignored;
    fs::remove_all(compile_root, ignored);
    fs::create_directories(compile_root);

    std::vector<std::string> build_args = {
        native_cxx_command(),
        "-std=c++20",
        "-shared",
#if !defined(_WIN32)
        "-fPIC",
#endif
        source_path.string(),
        "-o",
        output_path.string()
    };

    intptr_t exit_code = -1;
#if defined(_WIN32)
    std::vector<const char*> argv;
    argv.reserve(build_args.size() + 1U);
    for (const auto& arg : build_args) {
        argv.push_back(arg.c_str());
    }
    argv.push_back(nullptr);
    exit_code = _spawnvp(_P_WAIT, build_args.front().c_str(), const_cast<char* const*>(argv.data()));
#else
    const pid_t child = fork();
    if (child == 0) {
        ::close(STDOUT_FILENO);
        ::close(STDERR_FILENO);
        const int log_fd = ::creat(build_log_path.c_str(), 0644);
        if (log_fd >= 0) {
            ::dup2(log_fd, STDOUT_FILENO);
            ::dup2(log_fd, STDERR_FILENO);
            ::close(log_fd);
        }

        std::vector<const char*> argv;
        argv.reserve(build_args.size() + 1U);
        for (const auto& arg : build_args) {
            argv.push_back(arg.c_str());
        }
        argv.push_back(nullptr);
        ::execvp(build_args.front().c_str(), const_cast<char* const*>(argv.data()));
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
        error = "native wrapper compile failed to launch: " + std::error_code(errno, std::generic_category()).message();
        return false;
    }
    if (exit_code != 0) {
        error = "native wrapper compile failed";
        if (fs::exists(build_log_path)) {
            error += ":\n" + read_text(build_log_path);
        }
        return false;
    }
    if (!fs::exists(output_path)) {
        error = "native wrapper compile did not produce an output library";
        return false;
    }

    return true;
}

bool build_native_wrapper_with_cmake(
    const std::filesystem::path& cmake_lists_path,
    const std::filesystem::path& expected_output_path,
    std::filesystem::path& output_path,
    std::string& error) {
    namespace fs = std::filesystem;
    const fs::path source_root = cmake_lists_path.parent_path();
    const fs::path build_root = source_root / "cmake_build_check";
    const fs::path configure_log_path = build_root / "cmake-configure.log";
    const fs::path build_log_path = build_root / "cmake-build.log";
    std::error_code ignored;
    fs::remove_all(build_root, ignored);
    fs::remove(expected_output_path, ignored);
    fs::create_directories(build_root);

    const std::string configure_command =
        "cmake -S \"" + source_root.string() + "\" -B \"" + build_root.string() + "\" > \"" +
        configure_log_path.string() + "\" 2>&1";
    if (std::system(configure_command.c_str()) != 0) {
        error = "native wrapper CMake configure failed";
        if (fs::exists(configure_log_path)) {
            error += ":\n" + read_text(configure_log_path);
        }
        return false;
    }

    const std::string build_command =
        "cmake --build \"" + build_root.string() + "\" > \"" + build_log_path.string() + "\" 2>&1";
    if (std::system(build_command.c_str()) != 0) {
        error = "native wrapper CMake build failed";
        if (fs::exists(build_log_path)) {
            error += ":\n" + read_text(build_log_path);
        }
        return false;
    }

    if (fs::exists(expected_output_path)) {
        output_path = expected_output_path;
        return true;
    }

    error = "native wrapper CMake build did not produce the expected shared-library artifact";
    return false;
}

bool build_native_wrapper_with_script(
    const std::filesystem::path& script_path,
    const std::filesystem::path& expected_output_path,
    std::string& error) {
#if defined(_WIN32)
    (void)script_path;
    (void)expected_output_path;
    error = "native wrapper script execution is not implemented on Windows hosts";
    return false;
#else
    namespace fs = std::filesystem;
    const fs::path log_path = script_path.parent_path() / "native-wrapper-script-build.log";
    std::error_code ignored;
    fs::remove(expected_output_path, ignored);
    const std::string command =
        "sh \"" + script_path.string() + "\" > \"" + log_path.string() + "\" 2>&1";
    if (std::system(command.c_str()) != 0) {
        error = "native wrapper build script failed";
        if (fs::exists(log_path)) {
            error += ":\n" + read_text(log_path);
        }
        return false;
    }
    if (!fs::exists(expected_output_path)) {
        error = "native wrapper build script did not produce the expected primary output";
        if (fs::exists(log_path)) {
            error += ":\n" + read_text(log_path);
        }
        return false;
    }
    return true;
#endif
}

bool runtime_pipeline_primary_output_build_supported() {
    return cmake_is_available();
}

std::set<std::string> read_native_exported_symbols(const std::filesystem::path& binary_path, std::string& error) {
    std::set<std::string> symbols;
#if defined(_WIN32)
    (void)binary_path;
    error = "native symbol inspection is not implemented on Windows hosts";
    return symbols;
#else
    namespace fs = std::filesystem;
    const fs::path log_path = binary_path.parent_path() / "native-wrapper-symbols.log";
    const std::string command =
        "nm -D --defined-only \"" + binary_path.string() + "\" > \"" + log_path.string() + "\" 2>&1";
    if (std::system(command.c_str()) != 0) {
        error = "native wrapper symbol inspection failed";
        if (fs::exists(log_path)) {
            error += ":\n" + read_text(log_path);
        }
        return symbols;
    }

    std::istringstream input(read_text(log_path));
    std::string line;
    while (std::getline(input, line)) {
        const std::size_t name_pos = line.find_last_of(" \t");
        if (name_pos == std::string::npos || name_pos + 1U >= line.size()) {
            continue;
        }
        const std::string symbol = line.substr(name_pos + 1U);
        if (!symbol.empty()) {
            symbols.insert(symbol);
        }
    }
    return symbols;
#endif
}

std::set<std::string> read_module_definition_exports(const std::filesystem::path& path) {
    std::set<std::string> exports;
    std::istringstream input(read_text(path));
    std::string line;
    bool in_exports = false;
    while (std::getline(input, line)) {
        line = trim_copy(std::move(line));
        if (line.empty()) {
            continue;
        }
        if (line == "EXPORTS") {
            in_exports = true;
            continue;
        }
        if (!in_exports) {
            continue;
        }
        const std::size_t split = line.find_first_of(" \t");
        exports.insert(split == std::string::npos ? line : line.substr(0U, split));
    }
    return exports;
}

std::set<std::string> read_fll_api_declared_symbols(const std::filesystem::path& path) {
    std::set<std::string> symbols;
    std::istringstream input(read_text(path));
    std::string line;
    while (std::getline(input, line)) {
        line = trim_copy(std::move(line));
        if (line.rfind("function=", 0U) == 0U) {
            symbols.insert(line.substr(9U));
            continue;
        }
        if (line.rfind("loader_entrypoint=", 0U) == 0U) {
            symbols.insert(line.substr(18U));
            continue;
        }
        if (line.rfind("registration_symbol=", 0U) == 0U) {
            symbols.insert(line.substr(20U));
            continue;
        }
    }
    return symbols;
}

std::set<std::string> read_library_api_declared_symbols(const std::filesystem::path& path) {
    std::set<std::string> symbols;
    std::istringstream input(read_text(path));
    std::string line;
    while (std::getline(input, line)) {
        line = trim_copy(std::move(line));
        if (line.rfind("function=", 0U) == 0U) {
            symbols.insert(line.substr(9U));
        }
    }
    return symbols;
}

bool compile_csharp_artifact(const std::filesystem::path& source_path, std::string& error) {
    namespace fs = std::filesystem;
    const fs::path compile_root = source_path.parent_path() / "transpiled_compile_check";
    std::error_code ignored;
    fs::remove_all(compile_root, ignored);
    fs::create_directories(compile_root);

    const fs::path compile_source_path = compile_root / "TranspiledProgram.cs";
    const fs::path compile_project_path = compile_root / "TranspiledProgram.csproj";
    const fs::path build_log_path = compile_root / "dotnet-build.log";
    write_text(compile_source_path, read_text(source_path));
    write_text(
        compile_project_path,
        "<Project Sdk=\"Microsoft.NET.Sdk\">\n"
        "  <PropertyGroup>\n"
        "    <TargetFramework>net8.0</TargetFramework>\n"
        "    <OutputType>Library</OutputType>\n"
        "    <ImplicitUsings>enable</ImplicitUsings>\n"
        "    <Nullable>disable</Nullable>\n"
        "  </PropertyGroup>\n"
        "</Project>\n");

    std::vector<std::string> build_args = {
        "dotnet",
        "build",
        compile_project_path.string(),
        "--nologo",
        "-v",
        "minimal"
    };

    intptr_t exit_code = -1;
#if defined(_WIN32)
    std::vector<const char*> argv;
    argv.reserve(build_args.size() + 1U);
    for (const auto& arg : build_args) {
        argv.push_back(arg.c_str());
    }
    argv.push_back(nullptr);
    exit_code = _spawnvp(_P_WAIT, "dotnet", const_cast<char* const*>(argv.data()));
#else
    const pid_t child = fork();
    if (child == 0) {
        ::close(STDOUT_FILENO);
        ::close(STDERR_FILENO);
        const int log_fd = ::creat(build_log_path.c_str(), 0644);
        if (log_fd >= 0) {
            ::dup2(log_fd, STDOUT_FILENO);
            ::dup2(log_fd, STDERR_FILENO);
            ::close(log_fd);
        }

        std::vector<const char*> argv;
        argv.reserve(build_args.size() + 1U);
        for (const auto& arg : build_args) {
            argv.push_back(arg.c_str());
        }
        argv.push_back(nullptr);
        ::execvp("dotnet", const_cast<char* const*>(argv.data()));
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
        error = "dotnet build failed to launch: " + std::error_code(errno, std::generic_category()).message();
        return false;
    }
    if (exit_code != 0) {
        error = "dotnet build failed for emitted transpilation";
        if (fs::exists(build_log_path)) {
            error += ":\n" + read_text(build_log_path);
        }
        return false;
    }

    return true;
}

void write_synthetic_class_library_asset(const std::filesystem::path& table_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "custWidget",
            "",
            "custom",
            "PROCEDURE Load\r\nx = 1\r\nENDPROC\r\n"
            "PROCEDURE Init\r\nx = 2\r\nENDPROC\r\n"
            "PROCEDURE Destroy\r\nx = 3\r\nENDPROC\r\n"
        },
        {
            "txtName",
            "custWidget",
            "textbox",
            "PROCEDURE Valid\r\nTHISFORM.Refresh\r\nENDPROC\r\n"
        }
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "synthetic VCX/VCT fixture should be created");
}

void test_materialize_runtime_package() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_tests";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg", "DO FORM customer\n");
    write_text(project_dir / "customer.scx", "synthetic form");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "demo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "DemoApp";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "DemoApp";
    workspace.build_plan.output_path = (output_dir / "DemoApp.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"},
        {.record_index = 2U, .name = "customer.scx", .relative_path = "customer.scx", .type_title = "Form"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        true,
        true);

    expect(plan.ok, "runtime package plan should be created");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "runtime package should materialize");
    if (result.ok) {
        expect(fs::exists(result.plan.manifest_path), "runtime package should emit a manifest");
        expect(fs::exists(result.plan.debug_manifest_path), "runtime package should emit a debug manifest");
        expect(fs::exists(result.plan.runtime_host_destination_path), "runtime package should bundle the runtime host");
        expect(fs::exists(fs::path(result.plan.content_root) / "main.prg"), "runtime package should stage the startup source");
        expect(fs::exists(fs::path(result.plan.content_root) / "customer.scx"), "runtime package should stage project assets");
        expect(fs::exists(result.plan.launcher_project_path), "runtime package should emit a generated launcher project");
        expect(fs::exists(result.plan.launcher_source_path), "runtime package should emit a generated launcher source file");

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        expect(
            result.plan.startup_source_path == (fs::path(result.plan.content_root) / "main.prg").string(),
            "runtime plan should point startup to staged package content");
        expect(
            result.plan.debug_plan.startup_source_path == (project_dir / "main.prg").string(),
            "debug plan should point startup to source content");
        expect(result.plan.debug_plan.supports_breakpoints, "debug plan should enable breakpoints for PRG startup");
        expect(result.plan.debug_plan.supports_step_debugging, "debug plan should enable step debugging for PRG startup");
        expect(runtime_manifest.find("startup_source=") != std::string::npos, "runtime manifest should include a startup source field");
        expect(debug_manifest.find("startup_source=") != std::string::npos, "debug manifest should include a startup source field");
        expect(runtime_manifest.find("runtime_host_sha256=") != std::string::npos, "runtime manifest should include a runtime host SHA-256 digest");
        expect(runtime_manifest.find("security_role=") != std::string::npos, "runtime manifest should include the effective security role");
        expect(runtime_manifest.find("audit_log_path=") != std::string::npos, "runtime manifest should include the audit log path");
        expect(runtime_manifest.find("launcher_mode=dotnet_launcher") != std::string::npos, "runtime manifest should record the effective .NET launcher mode");
        expect(runtime_manifest.find("launcher_fallback=none") != std::string::npos, "runtime manifest should record the absence of launcher fallback");
        expect(runtime_manifest.find("dotnet_policy_allowlist=") != std::string::npos, "runtime manifest should include .NET policy allowlist metadata");
        expect(runtime_manifest.find("dotnet_policy_denylist=") != std::string::npos, "runtime manifest should include .NET policy denylist metadata");
        expect(runtime_manifest.find("dotnet_parity_matrix_entries=") != std::string::npos, "runtime manifest should include .NET parity matrix metadata");
        expect(runtime_manifest.find("dotnet_policy_allowlist_items=") != std::string::npos, "runtime manifest should include .NET policy allowlist item count");
        expect(runtime_manifest.find("dotnet_policy_denylist_items=") != std::string::npos, "runtime manifest should include .NET policy denylist item count");
        expect(runtime_manifest.find("dotnet_policy_allowlist_item=task-primitives") != std::string::npos,
               "runtime manifest should emit task-primitives allowlist entry");
        expect(runtime_manifest.find("dotnet_policy_denylist_item=unsafe-reflection-load") != std::string::npos,
               "runtime manifest should emit unsafe-reflection-load denylist entry");
        expect(runtime_manifest.find("dotnet_parity_matrix_count=") != std::string::npos, "runtime manifest should include .NET parity matrix count");
        expect(runtime_manifest.find("dotnet_parity_matrix_item=task-primitives") != std::string::npos,
               "runtime manifest should emit task-primitives parity matrix entry");
        expect(runtime_manifest.find("dotnet_parity_matrix_item=unsafe-reflection-load") != std::string::npos,
               "runtime manifest should emit unsafe-reflection-load parity matrix entry");
        expect(runtime_manifest.find("language_integration_count=") != std::string::npos, "runtime manifest should include language integration count");
        expect(runtime_manifest.find("language_integration=python|") != std::string::npos,
               "runtime manifest should emit python sidecar language integration");
        expect(runtime_manifest.find("language_integration=r|") != std::string::npos,
               "runtime manifest should emit R sidecar language integration");
        expect(runtime_manifest.find("ai_feature_count=") != std::string::npos, "runtime manifest should include AI feature count");
        expect(runtime_manifest.find("ai_feature=mcp-host|") != std::string::npos,
               "runtime manifest should emit MCP host AI feature metadata");
        expect(runtime_manifest.find("ai_feature=ai-assist|") != std::string::npos,
               "runtime manifest should emit AI-assisted developer workflow metadata");
        expect(runtime_manifest.find("extensibility_guardrail_count=") != std::string::npos,
               "runtime manifest should include extensibility guardrail count");
        expect(runtime_manifest.find("The trusted execution core stays native-first and security-first.") != std::string::npos,
               "runtime manifest should include explicit extensibility guardrails");
        expect(runtime_manifest.find("dotnet_gateway_task_primitives=") != std::string::npos, "runtime manifest should include .NET gateway allow decision diagnostics");
        expect(runtime_manifest.find("dotnet_gateway_unsafe_reflection=") != std::string::npos, "runtime manifest should include .NET gateway deny decision diagnostics");
        expect(runtime_manifest.find("feature_flag=launcher.dotnet.requested|true|rollout") != std::string::npos,
               "runtime manifest should expose the requested .NET launcher feature flag");
        expect(runtime_manifest.find("feature_flag=launcher.dotnet.active|true|host_compatibility") != std::string::npos,
               "runtime manifest should expose the active .NET launcher feature flag");
        expect(debug_manifest.find("launcher_mode=dotnet_launcher") != std::string::npos,
               "debug manifest should record the effective launcher mode");
        expect(debug_manifest.find("launcher_fallback=none") != std::string::npos,
               "debug manifest should record the launcher fallback state");
    }

    fs::remove_all(temp_root, ignored);
}

void test_generated_launcher_forwards_manifest_and_debug_flag() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_launcher_contract";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg", "RETURN\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "launcher_contract.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "LauncherContract";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "LauncherContract";
    workspace.build_plan.output_path = (output_dir / "LauncherContract.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        true);

    expect(plan.ok, "launcher contract plan should be created");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "launcher contract package should materialize");
    if (result.ok) {
        const std::string launcher_source = read_text(result.plan.launcher_source_path);
        const std::string launcher_project = read_text(result.plan.launcher_project_path);
        expect(
            launcher_source.find("var forwarded = new List<string> { \"--manifest\", Quote(manifest) };") != std::string::npos,
            "generated launcher should forward the manifest path to the runtime host");
        expect(
            launcher_source.find("string.Equals(arg, \"--debug\", StringComparison.OrdinalIgnoreCase)") != std::string::npos &&
            launcher_source.find("string.Equals(arg, \"/debug\", StringComparison.OrdinalIgnoreCase)") != std::string::npos,
            "generated launcher should preserve debug command-line forwarding");
        expect(
            launcher_source.find("forwarded.Add(Quote(arg));") != std::string::npos,
            "generated launcher should preserve ordinary application arguments instead of dropping them");
        expect(
            launcher_source.find("WorkingDirectory = baseDir") != std::string::npos,
            "generated launcher should run the runtime host from the package directory");
        expect(
            launcher_project.find("<AssemblyName>LauncherContract</AssemblyName>") != std::string::npos,
            "generated launcher project should preserve the sanitized assembly name contract");
    }

    fs::remove_all(temp_root, ignored);
}

void test_materialize_excluded_xasset_startup_package() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_xasset_tests";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "startup.scx", "synthetic form table");
    write_text(project_dir / "startup.sct", "synthetic form memo");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "demo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "DemoXAsset";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "DemoXAsset";
    workspace.build_plan.output_path = (output_dir / "DemoXAsset.exe").string();
    workspace.build_plan.startup_item = "startup.scx";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "startup.scx", .relative_path = "startup.scx", .type_title = "Form", .excluded = true}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        false);

    expect(plan.ok, "xasset runtime package plan should be created");
    expect(plan.debug_plan.supports_breakpoints, "xasset startup should advertise breakpoint support");
    expect(plan.debug_plan.supports_step_debugging, "xasset startup should advertise step debugging");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "xasset runtime package should materialize");
    if (result.ok) {
        expect(fs::exists(fs::path(result.plan.content_root) / "startup.scx"), "packaged xasset startup should be staged even if excluded");
        expect(fs::exists(fs::path(result.plan.content_root) / "startup.sct"), "packaged xasset memo sidecar should be staged");
    }

    fs::remove_all(temp_root, ignored);
}

void test_dotnet_launcher_request_falls_back_to_native_host_when_unavailable() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_dotnet_fallback";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg", "RETURN\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "dotnet_fallback.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "DotNetFallback";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "DotNetFallback";
    workspace.build_plan.output_path = (output_dir / "DotNetFallback.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"}
    };

    auto extensibility_profile = copperfin::platform::default_extensibility_profile();
    extensibility_profile.dotnet_output.available = false;

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        extensibility_profile,
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        true);

    expect(plan.ok, "dotnet-fallback plan should be created");
    expect(plan.requested_dotnet_launcher, "dotnet-fallback plan should record the requested .NET launcher");
    expect(!plan.emit_dotnet_launcher, "dotnet-fallback plan should disable .NET launcher emission when unavailable");
    expect(plan.launcher_mode == "native_runtime_host", "dotnet-fallback plan should resolve to native runtime host mode");
    expect(plan.launcher_fallback == "dotnet_output_unavailable", "dotnet-fallback plan should record the fallback reason");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        extensibility_profile,
        runtime_host.string());

    expect(result.ok, "dotnet-fallback package should materialize");
    if (result.ok) {
        expect(!fs::exists(result.plan.launcher_project_path),
               "dotnet-fallback package should not emit a launcher project when .NET output is unavailable");
        expect(!fs::exists(result.plan.launcher_source_path),
               "dotnet-fallback package should not emit launcher source when .NET output is unavailable");
        expect(fs::exists(result.plan.launcher_output_path),
               "dotnet-fallback package should materialize a project-named native entrypoint");
        expect(read_text(result.plan.launcher_output_path) == "runtime-host",
               "dotnet-fallback native entrypoint should package the runtime host payload bytes");
        expect(
            std::any_of(
                result.plan.extension_payload_digests.begin(),
                result.plan.extension_payload_digests.end(),
                [&](const copperfin::runtime::RuntimeArtifactDigest& digest) {
                    return digest.path == result.plan.launcher_output_path;
                }),
            "dotnet-fallback package should record the native entrypoint in extension payload digests");

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        expect(runtime_manifest.find("launcher_mode=native_runtime_host") != std::string::npos,
               "dotnet-fallback manifest should record the native runtime host mode");
        expect(runtime_manifest.find("launcher_fallback=dotnet_output_unavailable") != std::string::npos,
               "dotnet-fallback manifest should record the .NET-unavailable fallback reason");
        expect(runtime_manifest.find("feature_flag=launcher.dotnet.requested|true|rollout") != std::string::npos,
               "dotnet-fallback manifest should preserve the requested .NET launcher feature flag");
        expect(runtime_manifest.find("feature_flag=launcher.dotnet.active|false|host_compatibility") != std::string::npos,
               "dotnet-fallback manifest should record the inactive .NET launcher feature flag");
        expect(debug_manifest.find("launcher_mode=native_runtime_host") != std::string::npos,
               "dotnet-fallback debug manifest should record the native runtime host mode");
        expect(debug_manifest.find("launcher_fallback=dotnet_output_unavailable") != std::string::npos,
               "dotnet-fallback debug manifest should record the fallback reason");
        expect(runtime_manifest.find("extension_payload=" + quote_manifest_value(result.plan.launcher_output_path) + "|") != std::string::npos,
               "dotnet-fallback manifest should include the native entrypoint payload digest");
    }

    fs::remove_all(temp_root, ignored);
}

void test_library_output_package_emits_module_definition_from_prg_routines() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_library_contract";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "librarymain.prg",
               "PROCEDURE InitLibrary\nLPARAMETERS tcMode\nRETURN\nENDPROC\n");
    write_text(project_dir / "helper.prg",
               "FUNCTION AddNumbers\nPARAMETERS tnLeft, tnRight\nRETURN 1\nENDFUNC\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "librarydemo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "LibraryDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "LibraryDemo";
    workspace.build_plan.output_path = (output_dir / "LibraryDemo.dll").string();
    workspace.build_plan.output_kind = "dll";
    workspace.build_plan.build_target = "x64 Windows dynamic-link library";
    workspace.build_plan.startup_item = "librarymain.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "librarymain.prg", .relative_path = "librarymain.prg", .type_title = "Program"},
        {.record_index = 2U, .name = "helper.prg", .relative_path = "helper.prg", .type_title = "Program"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        true);

    expect(plan.ok, "library-output plan should be created");
    expect(plan.output_kind == copperfin::runtime::BuildOutputKind::dll,
           "library-output plan should preserve DLL output kind");
    expect(!plan.emit_dotnet_launcher,
           "library-output plan should not route through .NET launcher emission");
    expect(plan.launcher_mode == "foxpro_library_definition",
           "library-output plan should switch to the library-definition packaging mode");
    expect(plan.launcher_fallback == "library_binary_generation_pending",
           "library-output plan should record the honest non-binary fallback state");
    expect(fs::path(plan.launcher_output_path).filename() == "LibraryDemo.dll",
           "library-output plan should preserve the requested output filename");
    expect(fs::path(plan.module_definition_path).filename() == "LibraryDemo.def",
           "library-output plan should derive a matching module-definition filename");
    expect(fs::path(plan.native_wrapper_source_path).filename() == "LibraryDemo_wrapper.cpp",
           "library-output plan should derive a matching native-wrapper source filename");
    expect(fs::path(plan.native_wrapper_cmake_path).filename() == "CMakeLists.txt",
           "library-output plan should derive a native-wrapper CMake filename");
    expect(fs::path(plan.native_wrapper_build_script_path).filename() == "build_wrapper.sh",
           "library-output plan should derive a native-wrapper shell build script filename");
    expect(fs::path(plan.native_wrapper_build_powershell_path).filename() == "build_wrapper.ps1",
           "library-output plan should derive a native-wrapper PowerShell build script filename");
    expect(fs::path(plan.library_api_manifest_path).filename() == "LibraryDemo.dll.api",
           "library-output plan should derive a matching DLL API-manifest filename");
    expect(plan.exported_symbols.size() == 2U,
           "library-output plan should discover routine exports from PRG assets");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "library-output package should materialize");
    if (result.ok) {
        expect(fs::exists(result.plan.module_definition_path),
               "library-output package should emit a module-definition file");
        expect(fs::exists(result.plan.native_wrapper_source_path),
               "library-output package should emit a native-wrapper source scaffold");
        expect(fs::exists(result.plan.native_wrapper_cmake_path),
               "library-output package should emit native-wrapper build metadata");
        expect(fs::exists(result.plan.native_wrapper_build_script_path),
               "library-output package should emit a native-wrapper shell build script");
        expect(fs::exists(result.plan.native_wrapper_build_powershell_path),
               "library-output package should emit a native-wrapper PowerShell build script");
        expect(fs::exists(result.plan.library_api_manifest_path),
               "library-output package should emit a dedicated DLL API manifest");
        expect(!fs::exists(result.plan.launcher_output_path),
               "library-output package should not fake a DLL binary");
        expect(!fs::exists(result.plan.runtime_host_destination_path),
               "library-output package should not bundle an executable runtime host into the DLL output slot");
        expect(!result.plan.primary_output_materialized,
               "library-output package should report that the primary DLL binary is not yet materialized");

        const std::string module_definition = read_text(result.plan.module_definition_path);
        expect(module_definition.find("LIBRARY LibraryDemo") != std::string::npos,
               "module-definition file should declare the library name");
        expect(module_definition.find("EXPORTS") != std::string::npos,
               "module-definition file should include an EXPORTS section");
        expect(module_definition.find("InitLibrary") != std::string::npos,
               "module-definition file should export discovered procedure names");
        expect(module_definition.find("AddNumbers") != std::string::npos,
               "module-definition file should export discovered function names");
        const std::string wrapper_source = read_text(result.plan.native_wrapper_source_path);
        expect(wrapper_source.find("Generated Copperfin native wrapper scaffold") != std::string::npos,
               "library-output wrapper source should identify the generated scaffold");
        expect(wrapper_source.find("extern \"C\"") != std::string::npos,
               "library-output wrapper source should use C exports");
        expect(wrapper_source.find("#define COPPERFIN_VFP_DLL_CALL __stdcall") != std::string::npos,
               "library-output wrapper source should declare the VFP DLL calling-convention macro");
        expect(wrapper_source.find("int COPPERFIN_VFP_DLL_CALL InitLibrary(int tcMode)") != std::string::npos,
               "library-output wrapper source should scaffold procedure entrypoints with the VFP calling convention");
        expect(wrapper_source.find("(void)tcMode;") != std::string::npos,
               "library-output wrapper source should consume placeholder DLL arguments");
        expect(wrapper_source.find("int COPPERFIN_VFP_DLL_CALL AddNumbers(int tnLeft, int tnRight)") != std::string::npos,
               "library-output wrapper source should scaffold function entrypoints with the VFP calling convention");
        expect(wrapper_source.find("(void)tnRight;") != std::string::npos,
               "library-output wrapper source should consume multiple placeholder DLL arguments");
        const std::string wrapper_cmake = read_text(result.plan.native_wrapper_cmake_path);
        expect(wrapper_cmake.find("add_library(LibraryDemo SHARED LibraryDemo_wrapper.cpp)") != std::string::npos,
               "library-output wrapper CMake should declare a shared library target");
        expect(wrapper_cmake.find("PREFIX \"\" SUFFIX \".dll\"") != std::string::npos,
               "library-output wrapper CMake should preserve the requested DLL filename shape");
        expect(wrapper_cmake.find("LIBRARY_OUTPUT_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/..\"") != std::string::npos,
               "library-output wrapper CMake should route built libraries to the package root");
        expect(wrapper_cmake.find("RUNTIME_OUTPUT_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/..\"") != std::string::npos,
               "library-output wrapper CMake should route built runtime artifacts to the package root");
        expect(wrapper_cmake.find("/DEF:${CMAKE_CURRENT_SOURCE_DIR}/../LibraryDemo.def") != std::string::npos,
               "library-output wrapper CMake should forward the module-definition file on MSVC");
        const std::string wrapper_shell_script = read_text(result.plan.native_wrapper_build_script_path);
        expect(wrapper_shell_script.find("cmake -S \"$SCRIPT_DIR\" -B \"$SCRIPT_DIR/build\"") != std::string::npos,
               "library-output wrapper shell script should configure the emitted CMake project");
        expect(wrapper_shell_script.find("cmake --build \"$SCRIPT_DIR/build\"") != std::string::npos,
               "library-output wrapper shell script should build the emitted CMake project");
        const std::string wrapper_powershell_script = read_text(result.plan.native_wrapper_build_powershell_path);
        expect(wrapper_powershell_script.find("cmake -S $scriptDir -B $buildDir") != std::string::npos,
               "library-output wrapper PowerShell script should configure the emitted CMake project");
        expect(wrapper_powershell_script.find("cmake --build $buildDir") != std::string::npos,
               "library-output wrapper PowerShell script should build the emitted CMake project");
        if (native_cxx_is_available()) {
            fs::path compiled_wrapper_path;
            std::string compile_error;
            const bool compiled = compile_native_wrapper_scaffold(
                result.plan.native_wrapper_source_path,
                compiled_wrapper_path,
                compile_error);
            if (!compiled && !compile_error.empty()) {
                std::cerr << "FAIL: " << compile_error << "\n";
            }
            expect(compiled,
                   "library-output wrapper scaffold should compile under the host C++ toolchain");
            if (compiled && native_symbol_dump_is_available()) {
                std::string symbol_error;
                const std::set<std::string> exported_symbols = read_native_exported_symbols(compiled_wrapper_path, symbol_error);
                const std::set<std::string> declared_symbols = read_module_definition_exports(result.plan.module_definition_path);
                const std::set<std::string> declared_api_symbols = read_library_api_declared_symbols(result.plan.library_api_manifest_path);
                if (exported_symbols.empty() && !symbol_error.empty()) {
                    std::cerr << "FAIL: " << symbol_error << "\n";
                }
                expect(exported_symbols.contains("InitLibrary"),
                       "library-output compiled wrapper should export InitLibrary");
                expect(exported_symbols.contains("AddNumbers"),
                       "library-output compiled wrapper should export AddNumbers");
                expect(exported_symbols == declared_symbols,
                       "library-output compiled wrapper exports should stay synchronized with the module-definition contract");
                expect(exported_symbols == declared_api_symbols,
                       "library-output compiled wrapper exports should stay synchronized with the DLL API-manifest contract");
            }
        }
        if (cmake_is_available() && shell_is_available()) {
            std::string script_error;
            const bool script_built = build_native_wrapper_with_script(
                result.plan.native_wrapper_build_script_path,
                result.plan.launcher_output_path,
                script_error);
            if (!script_built && !script_error.empty()) {
                std::cerr << "FAIL: " << script_error << "\n";
            }
            expect(script_built,
                   "library-output wrapper shell script should build the requested primary output");
        }
        if (cmake_is_available()) {
            fs::path cmake_output_path;
            std::string cmake_error;
            const bool cmake_built = build_native_wrapper_with_cmake(
                result.plan.native_wrapper_cmake_path,
                result.plan.launcher_output_path,
                cmake_output_path,
                cmake_error);
            if (!cmake_built && !cmake_error.empty()) {
                std::cerr << "FAIL: " << cmake_error << "\n";
            }
            expect(cmake_built,
                   "library-output wrapper CMake metadata should configure and build under CMake");
            if (cmake_built) {
                expect(cmake_output_path == result.plan.launcher_output_path,
                       "library-output generated-CMake artifact should materialize the requested primary output path");
            }
            if (cmake_built && native_symbol_dump_is_available()) {
                std::string symbol_error;
                const std::set<std::string> exported_symbols = read_native_exported_symbols(cmake_output_path, symbol_error);
                const std::set<std::string> declared_symbols = read_module_definition_exports(result.plan.module_definition_path);
                const std::set<std::string> declared_api_symbols = read_library_api_declared_symbols(result.plan.library_api_manifest_path);
                if (exported_symbols.empty() && !symbol_error.empty()) {
                    std::cerr << "FAIL: " << symbol_error << "\n";
                }
                expect(exported_symbols == declared_symbols,
                       "library-output generated-CMake artifact exports should stay synchronized with the module-definition contract");
                expect(exported_symbols == declared_api_symbols,
                       "library-output generated-CMake artifact exports should stay synchronized with the DLL API-manifest contract");
            }
        }

        const std::string library_api_manifest = read_text(result.plan.library_api_manifest_path);
        expect(library_api_manifest.find("output_kind=dll") != std::string::npos,
               "library-output DLL API manifest should declare the DLL output kind");
        expect(library_api_manifest.find("callable_convention=vfp_declare_default") != std::string::npos,
               "library-output DLL API manifest should declare the VFP DLL calling convention");
        expect(library_api_manifest.find("function=InitLibrary") != std::string::npos,
               "library-output DLL API manifest should list discovered procedure names");
        expect(library_api_manifest.find("function=AddNumbers") != std::string::npos,
               "library-output DLL API manifest should list discovered function names");
        expect(library_api_manifest.find("function_kind=InitLibrary|procedure") != std::string::npos,
               "library-output DLL API manifest should record InitLibrary routine kind");
        expect(library_api_manifest.find("function_kind=AddNumbers|function") != std::string::npos,
               "library-output DLL API manifest should record AddNumbers routine kind");
        expect(library_api_manifest.find("function_source=InitLibrary|" + (project_dir / "librarymain.prg").string() + "|1") != std::string::npos,
               "library-output DLL API manifest should record InitLibrary source provenance");
        expect(library_api_manifest.find("function_source=AddNumbers|" + (project_dir / "helper.prg").string() + "|1") != std::string::npos,
               "library-output DLL API manifest should record AddNumbers source provenance");
        expect(library_api_manifest.find("function_parameters=InitLibrary|tcMode") != std::string::npos,
               "library-output DLL API manifest should record InitLibrary parameter names");
        expect(library_api_manifest.find("function_parameters=AddNumbers|tnLeft|tnRight") != std::string::npos,
               "library-output DLL API manifest should record AddNumbers parameter names");
        expect(library_api_manifest.find("function_parameter_declaration=InitLibrary|lparameters") != std::string::npos,
               "library-output DLL API manifest should record InitLibrary parameter declaration style");
        expect(library_api_manifest.find("function_parameter_declaration=AddNumbers|parameters") != std::string::npos,
               "library-output DLL API manifest should record AddNumbers parameter declaration style");

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        expect(runtime_manifest.find("output_kind=dll") != std::string::npos,
               "library-output manifest should record DLL output kind");
        expect(runtime_manifest.find("project_title=LibraryDemo") != std::string::npos,
               "library-output manifest should record the project title");
        expect(runtime_manifest.find("project_path=" + quote_manifest_value((project_dir / "librarydemo.pjx").string())) != std::string::npos,
               "library-output manifest should record the project path");
        expect(runtime_manifest.find("package_root=" + quote_manifest_value(result.plan.package_root)) != std::string::npos,
               "library-output manifest should record the package root");
        expect(runtime_manifest.find("content_root=" + quote_manifest_value(result.plan.content_root)) != std::string::npos,
               "library-output manifest should record the content root");
        expect(runtime_manifest.find("ast_manifest_path=" + quote_manifest_value(result.plan.ast_manifest_path)) != std::string::npos,
               "library-output manifest should record the AST manifest path");
        expect(runtime_manifest.find("ir_manifest_path=" + quote_manifest_value(result.plan.ir_manifest_path)) != std::string::npos,
               "library-output manifest should record the IR manifest path");
        expect(runtime_manifest.find("transpiled_csharp_path=" + quote_manifest_value(result.plan.transpiled_csharp_path)) != std::string::npos,
               "library-output manifest should record the transpiled C# path");
        expect(runtime_manifest.find("configuration=debug") != std::string::npos,
               "library-output manifest should record the debug build configuration");
        expect(runtime_manifest.find("module_definition_path=" + quote_manifest_value(result.plan.module_definition_path)) != std::string::npos,
               "library-output manifest should record the emitted module-definition path");
        expect(runtime_manifest.find("library_api_manifest_path=" + quote_manifest_value(result.plan.library_api_manifest_path)) != std::string::npos,
               "library-output manifest should record the dedicated DLL API-manifest path");
        expect(runtime_manifest.find("native_wrapper_source_path=" + quote_manifest_value(result.plan.native_wrapper_source_path)) != std::string::npos,
               "library-output manifest should record the wrapper source path");
        expect(runtime_manifest.find("native_wrapper_cmake_path=" + quote_manifest_value(result.plan.native_wrapper_cmake_path)) != std::string::npos,
               "library-output manifest should record the wrapper CMake path");
        expect(runtime_manifest.find("native_wrapper_build_script_path=" + quote_manifest_value(result.plan.native_wrapper_build_script_path)) != std::string::npos,
               "library-output manifest should record the wrapper shell build script path");
        expect(runtime_manifest.find("native_wrapper_build_powershell_path=" + quote_manifest_value(result.plan.native_wrapper_build_powershell_path)) != std::string::npos,
               "library-output manifest should record the wrapper PowerShell build script path");
        expect(runtime_manifest.find("library_callable_convention=vfp_declare_default") != std::string::npos,
               "library-output manifest should record the VFP DLL calling convention contract");
        expect(runtime_manifest.find("library_function_arity=InitLibrary|1") != std::string::npos,
               "library-output manifest should record InitLibrary arity");
        expect(runtime_manifest.find("library_function_arity=AddNumbers|2") != std::string::npos,
               "library-output manifest should record AddNumbers arity");
        expect(runtime_manifest.find("library_function_kind=InitLibrary|procedure") != std::string::npos,
               "library-output manifest should record InitLibrary routine kind");
        expect(runtime_manifest.find("library_function_kind=AddNumbers|function") != std::string::npos,
               "library-output manifest should record AddNumbers routine kind");
        expect(runtime_manifest.find("library_function_source=InitLibrary|" + (project_dir / "librarymain.prg").string() + "|1") != std::string::npos,
               "library-output manifest should record InitLibrary source provenance");
        expect(runtime_manifest.find("library_function_source=AddNumbers|" + (project_dir / "helper.prg").string() + "|1") != std::string::npos,
               "library-output manifest should record AddNumbers source provenance");
        expect(runtime_manifest.find("library_function_parameters=InitLibrary|tcMode") != std::string::npos,
               "library-output manifest should record InitLibrary parameter names");
        expect(runtime_manifest.find("library_function_parameters=AddNumbers|tnLeft|tnRight") != std::string::npos,
               "library-output manifest should record AddNumbers parameter names");
        expect(runtime_manifest.find("library_function_parameter_declaration=InitLibrary|lparameters") != std::string::npos,
               "library-output manifest should record InitLibrary parameter declaration style");
        expect(runtime_manifest.find("library_function_parameter_declaration=AddNumbers|parameters") != std::string::npos,
               "library-output manifest should record AddNumbers parameter declaration style");
        expect(runtime_manifest.find("library_function_call_surface=InitLibrary|vfp_declare_default|int tcMode") != std::string::npos,
               "library-output manifest should record InitLibrary call-surface contract");
        expect(runtime_manifest.find("library_function_call_surface=AddNumbers|vfp_declare_default|int tnLeft, int tnRight") != std::string::npos,
               "library-output manifest should record AddNumbers call-surface contract");
        expect(runtime_manifest.find("export_symbol=InitLibrary") != std::string::npos,
               "library-output manifest should record discovered export symbols");
        expect(runtime_manifest.find("export_symbol=AddNumbers") != std::string::npos,
               "library-output manifest should record all discovered export symbols");
        expect(runtime_manifest.find("primary_output_materialized=false") != std::string::npos,
               "library-output manifest should record the honest non-materialized DLL state");
        expect(runtime_manifest.find("feature_flag=build.output.library_contract|true|build_output") != std::string::npos,
               "library-output manifest should expose the library-contract feature flag");
        expect(runtime_manifest.find("feature_flag=build.output.native_library_wrapper|true|build_output") != std::string::npos,
               "library-output manifest should expose the native-wrapper feature flag");
        const std::vector<std::string> runtime_asset_lines = lines_with_prefix(runtime_manifest, "asset=");
        expect(!runtime_asset_lines.empty(),
               "library-output manifest should record staged asset inventory");
        expect(debug_manifest.find("output_kind=dll") != std::string::npos,
               "library-output debug manifest should record DLL output kind");
        expect(debug_manifest.find("project_title=LibraryDemo") != std::string::npos,
               "library-output debug manifest should record the project title");
        expect(debug_manifest.find("project_path=" + quote_manifest_value((project_dir / "librarydemo.pjx").string())) != std::string::npos,
               "library-output debug manifest should record the project path");
        expect(debug_manifest.find("package_root=" + quote_manifest_value(result.plan.package_root)) != std::string::npos,
               "library-output debug manifest should record the package root");
        expect(debug_manifest.find("content_root=" + quote_manifest_value(result.plan.content_root)) != std::string::npos,
               "library-output debug manifest should record the content root");
        expect(debug_manifest.find("ast_manifest_path=" + quote_manifest_value(result.plan.ast_manifest_path)) != std::string::npos,
               "library-output debug manifest should record the AST manifest path");
        expect(debug_manifest.find("ir_manifest_path=" + quote_manifest_value(result.plan.ir_manifest_path)) != std::string::npos,
               "library-output debug manifest should record the IR manifest path");
        expect(debug_manifest.find("transpiled_csharp_path=" + quote_manifest_value(result.plan.transpiled_csharp_path)) != std::string::npos,
               "library-output debug manifest should record the transpiled C# path");
        expect(debug_manifest.find("configuration=debug") != std::string::npos,
               "library-output debug manifest should record the debug build configuration");
        expect(debug_manifest.find("primary_output_path=" + quote_manifest_value(result.plan.launcher_output_path)) != std::string::npos,
               "library-output debug manifest should record the requested DLL output path");
        expect(debug_manifest.find("primary_output_materialized=false") != std::string::npos,
               "library-output debug manifest should record the honest non-materialized DLL state");
        expect(debug_manifest.find("module_definition_path=" + quote_manifest_value(result.plan.module_definition_path)) != std::string::npos,
               "library-output debug manifest should record the module-definition path");
        expect(debug_manifest.find("library_api_manifest_path=" + quote_manifest_value(result.plan.library_api_manifest_path)) != std::string::npos,
               "library-output debug manifest should record the dedicated DLL API-manifest path");
        expect(debug_manifest.find("library_callable_convention=vfp_declare_default") != std::string::npos,
               "library-output debug manifest should record the VFP DLL calling convention contract");
        expect(debug_manifest.find("compiler_contract=" + quote_manifest_value(result.plan.module_definition_path) + "|") != std::string::npos,
               "library-output debug manifest should record the module-definition compiler-contract digest");
        expect(debug_manifest.find("compiler_contract=" + quote_manifest_value(result.plan.library_api_manifest_path) + "|") != std::string::npos,
               "library-output debug manifest should record the DLL API-manifest compiler-contract digest");
        expect(debug_manifest.find("feature_flag=build.output.library_contract|true|build_output") != std::string::npos,
               "library-output debug manifest should expose the library-contract feature flag");
        expect(debug_manifest.find("feature_flag=build.output.native_library_wrapper|true|build_output") != std::string::npos,
               "library-output debug manifest should expose the native-wrapper feature flag");
        expect(debug_manifest.find("export_symbol=InitLibrary") != std::string::npos,
               "library-output debug manifest should record discovered DLL export symbols");
        expect(debug_manifest.find("export_symbol=AddNumbers") != std::string::npos,
               "library-output debug manifest should record all DLL export symbols");
        expect(debug_manifest.find("native_wrapper_source_path=" + quote_manifest_value(result.plan.native_wrapper_source_path)) != std::string::npos,
               "library-output debug manifest should record the wrapper source path");
        expect(debug_manifest.find("native_wrapper_cmake_path=" + quote_manifest_value(result.plan.native_wrapper_cmake_path)) != std::string::npos,
               "library-output debug manifest should record the wrapper CMake path");
        expect(debug_manifest.find("native_wrapper_build_script_path=" + quote_manifest_value(result.plan.native_wrapper_build_script_path)) != std::string::npos,
               "library-output debug manifest should record the wrapper shell build script path");
        expect(debug_manifest.find("native_wrapper_build_powershell_path=" + quote_manifest_value(result.plan.native_wrapper_build_powershell_path)) != std::string::npos,
               "library-output debug manifest should record the wrapper PowerShell build script path");
        for (const auto& asset_line : runtime_asset_lines) {
            expect(debug_manifest.find(asset_line) != std::string::npos,
                   "library-output debug manifest should mirror each staged asset line from the runtime manifest");
        }

        if (runtime_pipeline_primary_output_build_supported()) {
            const auto build_result = copperfin::runtime::build_runtime_package_primary_output(
                result.plan,
                copperfin::security::default_native_security_profile(),
                copperfin::platform::default_extensibility_profile());
            if (!build_result.ok && !build_result.error.empty()) {
                std::cerr << "FAIL: " << build_result.error << "\n";
            }
            expect(build_result.ok,
                   "library-output runtime pipeline should build the requested primary output");
            if (build_result.ok) {
                expect(build_result.plan.primary_output_materialized,
                       "library-output runtime pipeline should mark the primary output as materialized");
                expect(fs::exists(build_result.plan.launcher_output_path),
                       "library-output runtime pipeline should materialize the requested DLL output");
                const std::string built_runtime_manifest = read_text(build_result.plan.manifest_path);
                const std::string built_debug_manifest = read_text(build_result.plan.debug_manifest_path);
                expect(built_runtime_manifest.find("primary_output_materialized=true") != std::string::npos,
                       "library-output runtime pipeline should rewrite the manifest with a materialized primary output state");
                expect(built_runtime_manifest.find("extension_payload=" + quote_manifest_value(build_result.plan.launcher_output_path) + "|") != std::string::npos,
                       "library-output runtime pipeline should record the built DLL as an extension payload");
                expect(built_debug_manifest.find("primary_output_path=" + quote_manifest_value(build_result.plan.launcher_output_path)) != std::string::npos,
                       "library-output runtime pipeline should rewrite the debug manifest with the materialized DLL output path");
                expect(built_debug_manifest.find("project_title=LibraryDemo") != std::string::npos,
                       "library-output runtime pipeline should preserve the project title in the rewritten debug manifest");
                expect(built_debug_manifest.find("project_path=" + quote_manifest_value((project_dir / "librarydemo.pjx").string())) != std::string::npos,
                       "library-output runtime pipeline should preserve the project path in the rewritten debug manifest");
                expect(built_debug_manifest.find("package_root=" + quote_manifest_value(build_result.plan.package_root)) != std::string::npos,
                       "library-output runtime pipeline should preserve the package root in the rewritten debug manifest");
                expect(built_debug_manifest.find("content_root=" + quote_manifest_value(build_result.plan.content_root)) != std::string::npos,
                       "library-output runtime pipeline should preserve the content root in the rewritten debug manifest");
                expect(built_debug_manifest.find("ast_manifest_path=" + quote_manifest_value(build_result.plan.ast_manifest_path)) != std::string::npos,
                       "library-output runtime pipeline should preserve the AST manifest path in the rewritten debug manifest");
                expect(built_debug_manifest.find("ir_manifest_path=" + quote_manifest_value(build_result.plan.ir_manifest_path)) != std::string::npos,
                       "library-output runtime pipeline should preserve the IR manifest path in the rewritten debug manifest");
                expect(built_debug_manifest.find("transpiled_csharp_path=" + quote_manifest_value(build_result.plan.transpiled_csharp_path)) != std::string::npos,
                       "library-output runtime pipeline should preserve the transpiled C# path in the rewritten debug manifest");
                expect(built_debug_manifest.find("configuration=debug") != std::string::npos,
                       "library-output runtime pipeline should preserve the debug build configuration in the rewritten debug manifest");
                expect(built_debug_manifest.find("primary_output_materialized=true") != std::string::npos,
                       "library-output runtime pipeline should rewrite the debug manifest with a materialized primary output state");
                expect(built_debug_manifest.find("extension_payload=" + quote_manifest_value(build_result.plan.launcher_output_path) + "|") != std::string::npos,
                       "library-output runtime pipeline should rewrite the debug manifest with the built DLL extension-payload digest");
                expect(built_debug_manifest.find("compiler_contract=" + quote_manifest_value(build_result.plan.module_definition_path) + "|") != std::string::npos,
                       "library-output runtime pipeline should preserve the module-definition compiler-contract digest in the rewritten debug manifest");
                expect(built_debug_manifest.find("compiler_contract=" + quote_manifest_value(build_result.plan.library_api_manifest_path) + "|") != std::string::npos,
                       "library-output runtime pipeline should preserve the DLL API-manifest compiler-contract digest in the rewritten debug manifest");
                expect(built_debug_manifest.find("feature_flag=build.output.library_contract|true|build_output") != std::string::npos,
                       "library-output runtime pipeline should preserve the library-contract feature flag in the rewritten debug manifest");
                expect(built_debug_manifest.find("feature_flag=build.output.native_library_wrapper|true|build_output") != std::string::npos,
                       "library-output runtime pipeline should preserve the native-wrapper feature flag in the rewritten debug manifest");
                expect(built_debug_manifest.find("export_symbol=InitLibrary") != std::string::npos,
                       "library-output runtime pipeline should preserve DLL export symbols in the rewritten debug manifest");
                expect(built_debug_manifest.find("export_symbol=AddNumbers") != std::string::npos,
                       "library-output runtime pipeline should preserve all DLL export symbols in the rewritten debug manifest");
                const std::vector<std::string> built_runtime_asset_lines = lines_with_prefix(built_runtime_manifest, "asset=");
                expect(!built_runtime_asset_lines.empty(),
                       "library-output runtime pipeline should preserve staged asset inventory in the rewritten runtime manifest");
                for (const auto& asset_line : built_runtime_asset_lines) {
                    expect(built_debug_manifest.find(asset_line) != std::string::npos,
                           "library-output runtime pipeline should preserve each staged asset line in the rewritten debug manifest");
                }
                if (native_symbol_dump_is_available()) {
                    std::string symbol_error;
                    const std::set<std::string> exported_symbols = read_native_exported_symbols(build_result.plan.launcher_output_path, symbol_error);
                    const std::set<std::string> declared_symbols = read_module_definition_exports(build_result.plan.module_definition_path);
                    const std::set<std::string> declared_api_symbols = read_library_api_declared_symbols(build_result.plan.library_api_manifest_path);
                    if (exported_symbols.empty() && !symbol_error.empty()) {
                        std::cerr << "FAIL: " << symbol_error << "\n";
                    }
                    expect(exported_symbols == declared_symbols,
                           "library-output runtime pipeline build should preserve the module-definition export contract");
                    expect(exported_symbols == declared_api_symbols,
                           "library-output runtime pipeline build should preserve the DLL API-manifest export contract");
                }
            }
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_fll_output_package_emits_api_manifest_from_prg_routines() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_fll_contract";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "librarymain.prg",
               "PROCEDURE InitLibrary\nLPARAMETERS tcMode\nRETURN\nENDPROC\n");
    write_text(project_dir / "helper.prg",
               "FUNCTION AddNumbers\nPARAMETERS tnLeft, tnRight\nRETURN 1\nENDFUNC\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "librarydemo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "LibraryDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "LibraryDemo";
    workspace.build_plan.output_path = (output_dir / "LibraryDemo.fll").string();
    workspace.build_plan.output_kind = "fll";
    workspace.build_plan.build_target = "x64 Visual FoxPro library";
    workspace.build_plan.startup_item = "librarymain.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "librarymain.prg", .relative_path = "librarymain.prg", .type_title = "Program"},
        {.record_index = 2U, .name = "helper.prg", .relative_path = "helper.prg", .type_title = "Program"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        true);

    expect(plan.ok, "fll-output plan should be created");
    expect(plan.output_kind == copperfin::runtime::BuildOutputKind::fll,
           "fll-output plan should preserve FLL output kind");
    expect(!plan.emit_dotnet_launcher,
           "fll-output plan should not route through .NET launcher emission");
    expect(plan.launcher_mode == "foxpro_library_definition",
           "fll-output plan should switch to the library-definition packaging mode");
    expect(plan.launcher_fallback == "library_binary_generation_pending",
           "fll-output plan should record the honest non-binary fallback state");
    expect(fs::path(plan.launcher_output_path).filename() == "LibraryDemo.fll",
           "fll-output plan should preserve the requested output filename");
    expect(fs::path(plan.module_definition_path).filename() == "LibraryDemo.def",
           "fll-output plan should derive a matching module-definition filename");
    expect(fs::path(plan.native_wrapper_source_path).filename() == "LibraryDemo_wrapper.cpp",
           "fll-output plan should derive a matching native-wrapper source filename");
    expect(fs::path(plan.native_wrapper_cmake_path).filename() == "CMakeLists.txt",
           "fll-output plan should derive a native-wrapper CMake filename");
    expect(fs::path(plan.native_wrapper_build_script_path).filename() == "build_wrapper.sh",
           "fll-output plan should derive a native-wrapper shell build script filename");
    expect(fs::path(plan.native_wrapper_build_powershell_path).filename() == "build_wrapper.ps1",
           "fll-output plan should derive a native-wrapper PowerShell build script filename");
    expect(fs::path(plan.fll_api_manifest_path).filename() == "LibraryDemo.fll.api",
           "fll-output plan should derive a matching API-manifest filename");
    expect(plan.exported_symbols.size() == 2U,
           "fll-output plan should discover routine exports from PRG assets");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "fll-output package should materialize");
    if (result.ok) {
        expect(fs::exists(result.plan.module_definition_path),
               "fll-output package should emit a module-definition file");
        expect(fs::exists(result.plan.native_wrapper_source_path),
               "fll-output package should emit a native-wrapper source scaffold");
        expect(fs::exists(result.plan.native_wrapper_cmake_path),
               "fll-output package should emit native-wrapper build metadata");
        expect(fs::exists(result.plan.native_wrapper_build_script_path),
               "fll-output package should emit a native-wrapper shell build script");
        expect(fs::exists(result.plan.native_wrapper_build_powershell_path),
               "fll-output package should emit a native-wrapper PowerShell build script");
        expect(fs::exists(result.plan.fll_api_manifest_path),
               "fll-output package should emit an API manifest");
        expect(!fs::exists(result.plan.launcher_output_path),
               "fll-output package should not fake an FLL binary");
        expect(!fs::exists(result.plan.runtime_host_destination_path),
               "fll-output package should not bundle an executable runtime host into the FLL output slot");
        expect(!result.plan.primary_output_materialized,
               "fll-output package should report that the primary FLL binary is not yet materialized");

        const std::string module_definition = read_text(result.plan.module_definition_path);
        expect(module_definition.find("LIBRARY LibraryDemo") != std::string::npos,
               "fll-output module-definition file should declare the library name");
        expect(module_definition.find("InitLibrary") != std::string::npos,
               "fll-output module-definition file should export discovered procedure names");
        expect(module_definition.find("AddNumbers") != std::string::npos,
               "fll-output module-definition file should export discovered function names");
        const std::string wrapper_source = read_text(result.plan.native_wrapper_source_path);
        expect(wrapper_source.find("Generated Copperfin native wrapper scaffold") != std::string::npos,
               "fll-output wrapper source should identify the generated scaffold");
        expect(wrapper_source.find("struct ParamBlk") != std::string::npos,
               "fll-output wrapper source should declare a ParamBlk-shaped callable surface");
        expect(wrapper_source.find("static int _RetInt(int value)") != std::string::npos,
               "fll-output wrapper source should declare the default return helper");
        expect(wrapper_source.find("int InitLibrary(ParamBlk* parm)") != std::string::npos,
               "fll-output wrapper source should scaffold ParamBlk procedure entrypoints");
        expect(wrapper_source.find("int AddNumbers(ParamBlk* parm)") != std::string::npos,
               "fll-output wrapper source should scaffold ParamBlk function entrypoints");
        expect(wrapper_source.find("return _RetInt(-1);") != std::string::npos,
               "fll-output wrapper source should route stub returns through the default return helper");
        expect(wrapper_source.find("struct CopperfinFoxInfoRecord") != std::string::npos,
               "fll-output wrapper source should emit FoxInfo registration metadata");
        expect(wrapper_source.find("struct CopperfinFoxTableRecord") != std::string::npos,
               "fll-output wrapper source should emit FoxTable registration metadata");
        expect(wrapper_source.find("const char* routine_kind;") != std::string::npos,
               "fll-output wrapper source should record routine kind fields in the FoxInfo table");
        expect(wrapper_source.find("const char* source_path;") != std::string::npos,
               "fll-output wrapper source should record source-path fields in the FoxInfo table");
        expect(wrapper_source.find("unsigned int source_line;") != std::string::npos,
               "fll-output wrapper source should record source-line fields in the FoxInfo table");
        expect(wrapper_source.find("const char* parameter_declaration_kind;") != std::string::npos,
               "fll-output wrapper source should record parameter-declaration fields in the FoxInfo table");
        expect(wrapper_source.find("const char* parameter_names;") != std::string::npos,
               "fll-output wrapper source should record parameter-name fields in the FoxInfo table");
        expect(wrapper_source.find("const CopperfinFoxTableRecord _FoxTable") != std::string::npos,
               "fll-output wrapper source should export the FoxTable registration symbol");
        expect(wrapper_source.find("{\"InitLibrary\", &InitLibrary, \"procedure\", \"" + (project_dir / "librarymain.prg").string() + "\", 1U, \"lparameters\", \"tcMode\", 1U}") != std::string::npos,
               "fll-output wrapper source should record InitLibrary metadata in the FoxInfo table");
        expect(wrapper_source.find("{\"AddNumbers\", &AddNumbers, \"function\", \"" + (project_dir / "helper.prg").string() + "\", 1U, \"parameters\", \"tnLeft|tnRight\", 2U}") != std::string::npos,
               "fll-output wrapper source should record AddNumbers metadata in the FoxInfo table");
        expect(wrapper_source.find("const CopperfinFoxTableRecord* FoxInfo()") != std::string::npos,
               "fll-output wrapper source should scaffold the FoxInfo entrypoint");
        const std::string wrapper_cmake = read_text(result.plan.native_wrapper_cmake_path);
        expect(wrapper_cmake.find("add_library(LibraryDemo SHARED LibraryDemo_wrapper.cpp)") != std::string::npos,
               "fll-output wrapper CMake should declare a shared library target");
        expect(wrapper_cmake.find("PREFIX \"\" SUFFIX \".fll\"") != std::string::npos,
               "fll-output wrapper CMake should preserve the requested FLL filename shape");
        expect(wrapper_cmake.find("LIBRARY_OUTPUT_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/..\"") != std::string::npos,
               "fll-output wrapper CMake should route built libraries to the package root");
        expect(wrapper_cmake.find("RUNTIME_OUTPUT_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/..\"") != std::string::npos,
               "fll-output wrapper CMake should route built runtime artifacts to the package root");
        expect(wrapper_cmake.find("/DEF:${CMAKE_CURRENT_SOURCE_DIR}/../LibraryDemo.def") != std::string::npos,
               "fll-output wrapper CMake should forward the module-definition file on MSVC");
        const std::string wrapper_shell_script = read_text(result.plan.native_wrapper_build_script_path);
        expect(wrapper_shell_script.find("cmake -S \"$SCRIPT_DIR\" -B \"$SCRIPT_DIR/build\"") != std::string::npos,
               "fll-output wrapper shell script should configure the emitted CMake project");
        expect(wrapper_shell_script.find("cmake --build \"$SCRIPT_DIR/build\"") != std::string::npos,
               "fll-output wrapper shell script should build the emitted CMake project");
        const std::string wrapper_powershell_script = read_text(result.plan.native_wrapper_build_powershell_path);
        expect(wrapper_powershell_script.find("cmake -S $scriptDir -B $buildDir") != std::string::npos,
               "fll-output wrapper PowerShell script should configure the emitted CMake project");
        expect(wrapper_powershell_script.find("cmake --build $buildDir") != std::string::npos,
               "fll-output wrapper PowerShell script should build the emitted CMake project");
        if (native_cxx_is_available()) {
            fs::path compiled_wrapper_path;
            std::string compile_error;
            const bool compiled = compile_native_wrapper_scaffold(
                result.plan.native_wrapper_source_path,
                compiled_wrapper_path,
                compile_error);
            if (!compiled && !compile_error.empty()) {
                std::cerr << "FAIL: " << compile_error << "\n";
            }
            expect(compiled,
                   "fll-output wrapper scaffold should compile under the host C++ toolchain");
            if (compiled && native_symbol_dump_is_available()) {
                std::string symbol_error;
                const std::set<std::string> exported_symbols = read_native_exported_symbols(compiled_wrapper_path, symbol_error);
                const std::set<std::string> declared_module_symbols = read_module_definition_exports(result.plan.module_definition_path);
                const std::set<std::string> declared_api_symbols = read_fll_api_declared_symbols(result.plan.fll_api_manifest_path);
                if (exported_symbols.empty() && !symbol_error.empty()) {
                    std::cerr << "FAIL: " << symbol_error << "\n";
                }
                expect(exported_symbols.contains("InitLibrary"),
                       "fll-output compiled wrapper should export InitLibrary");
                expect(exported_symbols.contains("AddNumbers"),
                       "fll-output compiled wrapper should export AddNumbers");
                expect(exported_symbols.contains("FoxInfo"),
                       "fll-output compiled wrapper should export FoxInfo");
                expect(exported_symbols.contains("_FoxTable"),
                       "fll-output compiled wrapper should export the FoxTable registration symbol");
                expect(exported_symbols == declared_module_symbols,
                       "fll-output compiled wrapper exports should stay synchronized with the module-definition contract");
                expect(exported_symbols == declared_api_symbols,
                       "fll-output compiled wrapper exports should stay synchronized with the API manifest contract");
            }
        }
        if (cmake_is_available() && shell_is_available()) {
            std::string script_error;
            const bool script_built = build_native_wrapper_with_script(
                result.plan.native_wrapper_build_script_path,
                result.plan.launcher_output_path,
                script_error);
            if (!script_built && !script_error.empty()) {
                std::cerr << "FAIL: " << script_error << "\n";
            }
            expect(script_built,
                   "fll-output wrapper shell script should build the requested primary output");
        }
        if (cmake_is_available()) {
            fs::path cmake_output_path;
            std::string cmake_error;
            const bool cmake_built = build_native_wrapper_with_cmake(
                result.plan.native_wrapper_cmake_path,
                result.plan.launcher_output_path,
                cmake_output_path,
                cmake_error);
            if (!cmake_built && !cmake_error.empty()) {
                std::cerr << "FAIL: " << cmake_error << "\n";
            }
            expect(cmake_built,
                   "fll-output wrapper CMake metadata should configure and build under CMake");
            if (cmake_built) {
                expect(cmake_output_path == result.plan.launcher_output_path,
                       "fll-output generated-CMake artifact should materialize the requested primary output path");
            }
            if (cmake_built && native_symbol_dump_is_available()) {
                std::string symbol_error;
                const std::set<std::string> exported_symbols = read_native_exported_symbols(cmake_output_path, symbol_error);
                const std::set<std::string> declared_module_symbols = read_module_definition_exports(result.plan.module_definition_path);
                const std::set<std::string> declared_api_symbols = read_fll_api_declared_symbols(result.plan.fll_api_manifest_path);
                if (exported_symbols.empty() && !symbol_error.empty()) {
                    std::cerr << "FAIL: " << symbol_error << "\n";
                }
                expect(exported_symbols == declared_module_symbols,
                       "fll-output generated-CMake artifact exports should stay synchronized with the module-definition contract");
                expect(exported_symbols == declared_api_symbols,
                       "fll-output generated-CMake artifact exports should stay synchronized with the API manifest contract");
            }
        }

        const std::string api_manifest = read_text(result.plan.fll_api_manifest_path);
        expect(api_manifest.find("output_kind=fll") != std::string::npos,
               "fll-output API manifest should declare the FLL output kind");
        expect(api_manifest.find("library_file=LibraryDemo.fll") != std::string::npos,
               "fll-output API manifest should name the requested FLL file");
        expect(api_manifest.find("registration_model=FoxInfo/FoxTable") != std::string::npos,
               "fll-output API manifest should declare the FoxInfo/FoxTable registration model");
        expect(api_manifest.find("registration_command=SET LIBRARY TO") != std::string::npos,
               "fll-output API manifest should declare the registration command");
        expect(api_manifest.find("release_command=RELEASE LIBRARY") != std::string::npos,
               "fll-output API manifest should declare the release command");
        expect(api_manifest.find("additive_supported=true") != std::string::npos,
               "fll-output API manifest should declare additive loading support");
        expect(api_manifest.find("loader_entrypoint=FoxInfo") != std::string::npos,
               "fll-output API manifest should declare the loader entrypoint");
        expect(api_manifest.find("registration_symbol=_FoxTable") != std::string::npos,
               "fll-output API manifest should declare the FoxTable registration symbol");
        expect(api_manifest.find("callable_signature=ParamBlk*") != std::string::npos,
               "fll-output API manifest should declare the ParamBlk callable signature");
        expect(api_manifest.find("default_return_helper=_RetInt") != std::string::npos,
               "fll-output API manifest should declare the default return helper");
        expect(api_manifest.find("function=InitLibrary") != std::string::npos,
               "fll-output API manifest should list discovered procedure names");
        expect(api_manifest.find("function=AddNumbers") != std::string::npos,
               "fll-output API manifest should list discovered function names");
        expect(api_manifest.find("function_arity=InitLibrary|1") != std::string::npos,
               "fll-output API manifest should declare InitLibrary arity");
        expect(api_manifest.find("function_arity=AddNumbers|2") != std::string::npos,
               "fll-output API manifest should declare AddNumbers arity");
        expect(api_manifest.find("function_kind=InitLibrary|procedure") != std::string::npos,
               "fll-output API manifest should record InitLibrary routine kind");
        expect(api_manifest.find("function_kind=AddNumbers|function") != std::string::npos,
               "fll-output API manifest should record AddNumbers routine kind");
        expect(api_manifest.find("function_source=InitLibrary|" + (project_dir / "librarymain.prg").string() + "|1") != std::string::npos,
               "fll-output API manifest should record InitLibrary source provenance");
        expect(api_manifest.find("function_source=AddNumbers|" + (project_dir / "helper.prg").string() + "|1") != std::string::npos,
               "fll-output API manifest should record AddNumbers source provenance");
        expect(api_manifest.find("function_parameters=InitLibrary|tcMode") != std::string::npos,
               "fll-output API manifest should record InitLibrary parameter names");
        expect(api_manifest.find("function_parameters=AddNumbers|tnLeft|tnRight") != std::string::npos,
               "fll-output API manifest should record AddNumbers parameter names");
        expect(api_manifest.find("function_parameter_declaration=InitLibrary|lparameters") != std::string::npos,
               "fll-output API manifest should record InitLibrary parameter declaration style");
        expect(api_manifest.find("function_parameter_declaration=AddNumbers|parameters") != std::string::npos,
               "fll-output API manifest should record AddNumbers parameter declaration style");
        expect(api_manifest.find("function_call_surface=InitLibrary|ParamBlk*|_RetInt") != std::string::npos,
               "fll-output API manifest should declare InitLibrary callable surface");
        expect(api_manifest.find("function_call_surface=AddNumbers|ParamBlk*|_RetInt") != std::string::npos,
               "fll-output API manifest should declare AddNumbers callable surface");

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        expect(runtime_manifest.find("output_kind=fll") != std::string::npos,
               "fll-output manifest should record FLL output kind");
        expect(runtime_manifest.find("fll_api_manifest_path=" + quote_manifest_value(result.plan.fll_api_manifest_path)) != std::string::npos,
               "fll-output manifest should record the emitted API-manifest path");
        expect(runtime_manifest.find("fll_loader_entrypoint=FoxInfo") != std::string::npos,
               "fll-output manifest should record the FLL loader entrypoint");
        expect(runtime_manifest.find("fll_registration_symbol=_FoxTable") != std::string::npos,
               "fll-output manifest should record the FoxTable registration symbol");
        expect(runtime_manifest.find("fll_callable_signature=ParamBlk*") != std::string::npos,
               "fll-output manifest should record the ParamBlk callable signature");
        expect(runtime_manifest.find("fll_default_return_helper=_RetInt") != std::string::npos,
               "fll-output manifest should record the default return helper");
        expect(runtime_manifest.find("library_function_kind=InitLibrary|procedure") != std::string::npos,
               "fll-output manifest should mirror InitLibrary routine kind");
        expect(runtime_manifest.find("library_function_kind=AddNumbers|function") != std::string::npos,
               "fll-output manifest should mirror AddNumbers routine kind");
        expect(runtime_manifest.find("library_function_source=InitLibrary|" + (project_dir / "librarymain.prg").string() + "|1") != std::string::npos,
               "fll-output manifest should mirror InitLibrary source provenance");
        expect(runtime_manifest.find("library_function_source=AddNumbers|" + (project_dir / "helper.prg").string() + "|1") != std::string::npos,
               "fll-output manifest should mirror AddNumbers source provenance");
        expect(runtime_manifest.find("library_function_parameters=InitLibrary|tcMode") != std::string::npos,
               "fll-output manifest should mirror InitLibrary parameter names");
        expect(runtime_manifest.find("library_function_parameters=AddNumbers|tnLeft|tnRight") != std::string::npos,
               "fll-output manifest should mirror AddNumbers parameter names");
        expect(runtime_manifest.find("library_function_parameter_declaration=InitLibrary|lparameters") != std::string::npos,
               "fll-output manifest should mirror InitLibrary parameter declaration style");
        expect(runtime_manifest.find("library_function_parameter_declaration=AddNumbers|parameters") != std::string::npos,
               "fll-output manifest should mirror AddNumbers parameter declaration style");
        expect(runtime_manifest.find("library_function_call_surface=InitLibrary|ParamBlk*|_RetInt") != std::string::npos,
               "fll-output manifest should mirror InitLibrary callable surface");
        expect(runtime_manifest.find("library_function_call_surface=AddNumbers|ParamBlk*|_RetInt") != std::string::npos,
               "fll-output manifest should mirror AddNumbers callable surface");
        expect(runtime_manifest.find("native_wrapper_source_path=" + quote_manifest_value(result.plan.native_wrapper_source_path)) != std::string::npos,
               "fll-output manifest should record the wrapper source path");
        expect(runtime_manifest.find("native_wrapper_cmake_path=" + quote_manifest_value(result.plan.native_wrapper_cmake_path)) != std::string::npos,
               "fll-output manifest should record the wrapper CMake path");
        expect(runtime_manifest.find("native_wrapper_build_script_path=" + quote_manifest_value(result.plan.native_wrapper_build_script_path)) != std::string::npos,
               "fll-output manifest should record the wrapper shell build script path");
        expect(runtime_manifest.find("native_wrapper_build_powershell_path=" + quote_manifest_value(result.plan.native_wrapper_build_powershell_path)) != std::string::npos,
               "fll-output manifest should record the wrapper PowerShell build script path");
        expect(runtime_manifest.find("feature_flag=build.output.library_contract|true|build_output") != std::string::npos,
               "fll-output manifest should expose the library-contract feature flag");
        expect(runtime_manifest.find("feature_flag=build.output.native_library_wrapper|true|build_output") != std::string::npos,
               "fll-output manifest should expose the native-wrapper feature flag");
        expect(runtime_manifest.find("feature_flag=build.output.fll_api_contract|true|build_output") != std::string::npos,
               "fll-output manifest should expose the FLL API-contract feature flag");
        expect(runtime_manifest.find("project_title=LibraryDemo") != std::string::npos,
               "fll-output manifest should record the project title");
        expect(runtime_manifest.find("project_path=" + quote_manifest_value((project_dir / "librarydemo.pjx").string())) != std::string::npos,
               "fll-output manifest should record the project path");
        expect(runtime_manifest.find("package_root=" + quote_manifest_value(result.plan.package_root)) != std::string::npos,
               "fll-output manifest should record the package root");
        expect(runtime_manifest.find("content_root=" + quote_manifest_value(result.plan.content_root)) != std::string::npos,
               "fll-output manifest should record the content root");
        expect(runtime_manifest.find("ast_manifest_path=" + quote_manifest_value(result.plan.ast_manifest_path)) != std::string::npos,
               "fll-output manifest should record the AST manifest path");
        expect(runtime_manifest.find("ir_manifest_path=" + quote_manifest_value(result.plan.ir_manifest_path)) != std::string::npos,
               "fll-output manifest should record the IR manifest path");
        expect(runtime_manifest.find("transpiled_csharp_path=" + quote_manifest_value(result.plan.transpiled_csharp_path)) != std::string::npos,
               "fll-output manifest should record the transpiled C# path");
        expect(runtime_manifest.find("configuration=debug") != std::string::npos,
               "fll-output manifest should record the debug build configuration");
        const std::vector<std::string> runtime_asset_lines = lines_with_prefix(runtime_manifest, "asset=");
        expect(!runtime_asset_lines.empty(),
               "fll-output manifest should record staged asset inventory");
        expect(debug_manifest.find("output_kind=fll") != std::string::npos,
               "fll-output debug manifest should record FLL output kind");
        expect(debug_manifest.find("project_title=LibraryDemo") != std::string::npos,
               "fll-output debug manifest should record the project title");
        expect(debug_manifest.find("project_path=" + quote_manifest_value((project_dir / "librarydemo.pjx").string())) != std::string::npos,
               "fll-output debug manifest should record the project path");
        expect(debug_manifest.find("package_root=" + quote_manifest_value(result.plan.package_root)) != std::string::npos,
               "fll-output debug manifest should record the package root");
        expect(debug_manifest.find("content_root=" + quote_manifest_value(result.plan.content_root)) != std::string::npos,
               "fll-output debug manifest should record the content root");
        expect(debug_manifest.find("ast_manifest_path=" + quote_manifest_value(result.plan.ast_manifest_path)) != std::string::npos,
               "fll-output debug manifest should record the AST manifest path");
        expect(debug_manifest.find("ir_manifest_path=" + quote_manifest_value(result.plan.ir_manifest_path)) != std::string::npos,
               "fll-output debug manifest should record the IR manifest path");
        expect(debug_manifest.find("transpiled_csharp_path=" + quote_manifest_value(result.plan.transpiled_csharp_path)) != std::string::npos,
               "fll-output debug manifest should record the transpiled C# path");
        expect(debug_manifest.find("configuration=debug") != std::string::npos,
               "fll-output debug manifest should record the debug build configuration");
        expect(debug_manifest.find("primary_output_path=" + quote_manifest_value(result.plan.launcher_output_path)) != std::string::npos,
               "fll-output debug manifest should record the requested FLL output path");
        expect(debug_manifest.find("primary_output_materialized=false") != std::string::npos,
               "fll-output debug manifest should record the honest non-materialized FLL state");
        expect(debug_manifest.find("module_definition_path=" + quote_manifest_value(result.plan.module_definition_path)) != std::string::npos,
               "fll-output debug manifest should record the module-definition path");
        expect(debug_manifest.find("fll_api_manifest_path=" + quote_manifest_value(result.plan.fll_api_manifest_path)) != std::string::npos,
               "fll-output debug manifest should record the emitted API-manifest path");
        expect(debug_manifest.find("fll_loader_entrypoint=FoxInfo") != std::string::npos,
               "fll-output debug manifest should record the FLL loader entrypoint");
        expect(debug_manifest.find("fll_registration_symbol=_FoxTable") != std::string::npos,
               "fll-output debug manifest should record the FoxTable registration symbol");
        expect(debug_manifest.find("fll_callable_signature=ParamBlk*") != std::string::npos,
               "fll-output debug manifest should record the ParamBlk callable signature");
        expect(debug_manifest.find("fll_default_return_helper=_RetInt") != std::string::npos,
               "fll-output debug manifest should record the default return helper");
        expect(debug_manifest.find("compiler_contract=" + quote_manifest_value(result.plan.module_definition_path) + "|") != std::string::npos,
               "fll-output debug manifest should record the module-definition compiler-contract digest");
        expect(debug_manifest.find("compiler_contract=" + quote_manifest_value(result.plan.fll_api_manifest_path) + "|") != std::string::npos,
               "fll-output debug manifest should record the API-manifest compiler-contract digest");
        expect(debug_manifest.find("feature_flag=build.output.library_contract|true|build_output") != std::string::npos,
               "fll-output debug manifest should expose the library-contract feature flag");
        expect(debug_manifest.find("feature_flag=build.output.native_library_wrapper|true|build_output") != std::string::npos,
               "fll-output debug manifest should expose the native-wrapper feature flag");
        expect(debug_manifest.find("feature_flag=build.output.fll_api_contract|true|build_output") != std::string::npos,
               "fll-output debug manifest should expose the FLL API-contract feature flag");
        expect(debug_manifest.find("export_symbol=InitLibrary") != std::string::npos,
               "fll-output debug manifest should record discovered FLL routine export symbols");
        expect(debug_manifest.find("export_symbol=AddNumbers") != std::string::npos,
               "fll-output debug manifest should record all FLL export symbols");
        expect(debug_manifest.find("library_function_kind=InitLibrary|procedure") != std::string::npos,
               "fll-output debug manifest should mirror InitLibrary routine kind");
        expect(debug_manifest.find("library_function_kind=AddNumbers|function") != std::string::npos,
               "fll-output debug manifest should mirror AddNumbers routine kind");
        expect(debug_manifest.find("library_function_source=InitLibrary|" + (project_dir / "librarymain.prg").string() + "|1") != std::string::npos,
               "fll-output debug manifest should mirror InitLibrary source provenance");
        expect(debug_manifest.find("library_function_source=AddNumbers|" + (project_dir / "helper.prg").string() + "|1") != std::string::npos,
               "fll-output debug manifest should mirror AddNumbers source provenance");
        expect(debug_manifest.find("library_function_parameters=InitLibrary|tcMode") != std::string::npos,
               "fll-output debug manifest should mirror InitLibrary parameter names");
        expect(debug_manifest.find("library_function_parameters=AddNumbers|tnLeft|tnRight") != std::string::npos,
               "fll-output debug manifest should mirror AddNumbers parameter names");
        expect(debug_manifest.find("library_function_parameter_declaration=InitLibrary|lparameters") != std::string::npos,
               "fll-output debug manifest should mirror InitLibrary parameter declaration style");
        expect(debug_manifest.find("library_function_parameter_declaration=AddNumbers|parameters") != std::string::npos,
               "fll-output debug manifest should mirror AddNumbers parameter declaration style");
        expect(debug_manifest.find("library_function_call_surface=InitLibrary|ParamBlk*|_RetInt") != std::string::npos,
               "fll-output debug manifest should mirror InitLibrary callable surface");
        expect(debug_manifest.find("library_function_call_surface=AddNumbers|ParamBlk*|_RetInt") != std::string::npos,
               "fll-output debug manifest should mirror AddNumbers callable surface");
        expect(debug_manifest.find("native_wrapper_source_path=" + quote_manifest_value(result.plan.native_wrapper_source_path)) != std::string::npos,
               "fll-output debug manifest should record the wrapper source path");
        expect(debug_manifest.find("native_wrapper_cmake_path=" + quote_manifest_value(result.plan.native_wrapper_cmake_path)) != std::string::npos,
               "fll-output debug manifest should record the wrapper CMake path");
        expect(debug_manifest.find("native_wrapper_build_script_path=" + quote_manifest_value(result.plan.native_wrapper_build_script_path)) != std::string::npos,
               "fll-output debug manifest should record the wrapper shell build script path");
        expect(debug_manifest.find("native_wrapper_build_powershell_path=" + quote_manifest_value(result.plan.native_wrapper_build_powershell_path)) != std::string::npos,
               "fll-output debug manifest should record the wrapper PowerShell build script path");
        for (const auto& asset_line : runtime_asset_lines) {
            expect(debug_manifest.find(asset_line) != std::string::npos,
                   "fll-output debug manifest should mirror each staged asset line from the runtime manifest");
        }

        if (runtime_pipeline_primary_output_build_supported()) {
            const auto build_result = copperfin::runtime::build_runtime_package_primary_output(
                result.plan,
                copperfin::security::default_native_security_profile(),
                copperfin::platform::default_extensibility_profile());
            if (!build_result.ok && !build_result.error.empty()) {
                std::cerr << "FAIL: " << build_result.error << "\n";
            }
            expect(build_result.ok,
                   "fll-output runtime pipeline should build the requested primary output");
            if (build_result.ok) {
                expect(build_result.plan.primary_output_materialized,
                       "fll-output runtime pipeline should mark the primary output as materialized");
                expect(fs::exists(build_result.plan.launcher_output_path),
                       "fll-output runtime pipeline should materialize the requested FLL output");
                const std::string built_runtime_manifest = read_text(build_result.plan.manifest_path);
                const std::string built_debug_manifest = read_text(build_result.plan.debug_manifest_path);
                expect(built_runtime_manifest.find("primary_output_materialized=true") != std::string::npos,
                       "fll-output runtime pipeline should rewrite the manifest with a materialized primary output state");
                expect(built_runtime_manifest.find("extension_payload=" + quote_manifest_value(build_result.plan.launcher_output_path) + "|") != std::string::npos,
                       "fll-output runtime pipeline should record the built FLL as an extension payload");
                expect(built_debug_manifest.find("primary_output_path=" + quote_manifest_value(build_result.plan.launcher_output_path)) != std::string::npos,
                       "fll-output runtime pipeline should rewrite the debug manifest with the materialized FLL output path");
                expect(built_debug_manifest.find("project_title=LibraryDemo") != std::string::npos,
                       "fll-output runtime pipeline should preserve the project title in the rewritten debug manifest");
                expect(built_debug_manifest.find("project_path=" + quote_manifest_value((project_dir / "librarydemo.pjx").string())) != std::string::npos,
                       "fll-output runtime pipeline should preserve the project path in the rewritten debug manifest");
                expect(built_debug_manifest.find("package_root=" + quote_manifest_value(build_result.plan.package_root)) != std::string::npos,
                       "fll-output runtime pipeline should preserve the package root in the rewritten debug manifest");
                expect(built_debug_manifest.find("content_root=" + quote_manifest_value(build_result.plan.content_root)) != std::string::npos,
                       "fll-output runtime pipeline should preserve the content root in the rewritten debug manifest");
                expect(built_debug_manifest.find("ast_manifest_path=" + quote_manifest_value(build_result.plan.ast_manifest_path)) != std::string::npos,
                       "fll-output runtime pipeline should preserve the AST manifest path in the rewritten debug manifest");
                expect(built_debug_manifest.find("ir_manifest_path=" + quote_manifest_value(build_result.plan.ir_manifest_path)) != std::string::npos,
                       "fll-output runtime pipeline should preserve the IR manifest path in the rewritten debug manifest");
                expect(built_debug_manifest.find("transpiled_csharp_path=" + quote_manifest_value(build_result.plan.transpiled_csharp_path)) != std::string::npos,
                       "fll-output runtime pipeline should preserve the transpiled C# path in the rewritten debug manifest");
                expect(built_debug_manifest.find("configuration=debug") != std::string::npos,
                       "fll-output runtime pipeline should preserve the debug build configuration in the rewritten debug manifest");
                expect(built_debug_manifest.find("primary_output_materialized=true") != std::string::npos,
                       "fll-output runtime pipeline should rewrite the debug manifest with a materialized primary output state");
                expect(built_debug_manifest.find("extension_payload=" + quote_manifest_value(build_result.plan.launcher_output_path) + "|") != std::string::npos,
                       "fll-output runtime pipeline should rewrite the debug manifest with the built FLL extension-payload digest");
                expect(built_debug_manifest.find("compiler_contract=" + quote_manifest_value(build_result.plan.module_definition_path) + "|") != std::string::npos,
                       "fll-output runtime pipeline should preserve the module-definition compiler-contract digest in the rewritten debug manifest");
                expect(built_debug_manifest.find("compiler_contract=" + quote_manifest_value(build_result.plan.fll_api_manifest_path) + "|") != std::string::npos,
                       "fll-output runtime pipeline should preserve the API-manifest compiler-contract digest in the rewritten debug manifest");
                expect(built_debug_manifest.find("feature_flag=build.output.library_contract|true|build_output") != std::string::npos,
                       "fll-output runtime pipeline should preserve the library-contract feature flag in the rewritten debug manifest");
                expect(built_debug_manifest.find("feature_flag=build.output.native_library_wrapper|true|build_output") != std::string::npos,
                       "fll-output runtime pipeline should preserve the native-wrapper feature flag in the rewritten debug manifest");
                expect(built_debug_manifest.find("feature_flag=build.output.fll_api_contract|true|build_output") != std::string::npos,
                       "fll-output runtime pipeline should preserve the FLL API-contract feature flag in the rewritten debug manifest");
                expect(built_debug_manifest.find("export_symbol=InitLibrary") != std::string::npos,
                       "fll-output runtime pipeline should preserve discovered FLL routine export symbols in the rewritten debug manifest");
                expect(built_debug_manifest.find("export_symbol=AddNumbers") != std::string::npos,
                       "fll-output runtime pipeline should preserve all FLL export symbols in the rewritten debug manifest");
                const std::vector<std::string> built_runtime_asset_lines = lines_with_prefix(built_runtime_manifest, "asset=");
                expect(!built_runtime_asset_lines.empty(),
                       "fll-output runtime pipeline should preserve staged asset inventory in the rewritten runtime manifest");
                for (const auto& asset_line : built_runtime_asset_lines) {
                    expect(built_debug_manifest.find(asset_line) != std::string::npos,
                           "fll-output runtime pipeline should preserve each staged asset line in the rewritten debug manifest");
                }
                if (native_symbol_dump_is_available()) {
                    std::string symbol_error;
                    const std::set<std::string> exported_symbols = read_native_exported_symbols(build_result.plan.launcher_output_path, symbol_error);
                    const std::set<std::string> declared_module_symbols = read_module_definition_exports(build_result.plan.module_definition_path);
                    const std::set<std::string> declared_api_symbols = read_fll_api_declared_symbols(build_result.plan.fll_api_manifest_path);
                    if (exported_symbols.empty() && !symbol_error.empty()) {
                        std::cerr << "FAIL: " << symbol_error << "\n";
                    }
                    expect(exported_symbols == declared_module_symbols,
                           "fll-output runtime pipeline build should preserve the module-definition export contract");
                    expect(exported_symbols == declared_api_symbols,
                           "fll-output runtime pipeline build should preserve the API-manifest export contract");
                }
            }
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_fxp_output_package_emits_token_manifest_from_prg_statements() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_fxp_contract";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg",
               "LOCAL nValue\n"
               "nValue = 1\n"
               "DO worker\n"
               "RETURN\n"
               "PROCEDURE worker\n"
               "WAIT WINDOW 'hello'\n"
               "RETURN\n"
               "ENDPROC\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "compiledemo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "CompileDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "CompileDemo";
    workspace.build_plan.output_path = (output_dir / "CompileDemo.fxp").string();
    workspace.build_plan.output_kind = "fxp";
    workspace.build_plan.build_target = "x64 Visual FoxPro tokenized program";
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        true);

    expect(plan.ok, "fxp-output plan should be created");
    expect(plan.output_kind == copperfin::runtime::BuildOutputKind::fxp,
           "fxp-output plan should preserve FXP output kind");
    expect(!plan.emit_dotnet_launcher,
           "fxp-output plan should not route through .NET launcher emission");
    expect(plan.launcher_mode == "foxpro_tokenized_contract",
           "fxp-output plan should switch to the tokenized-contract packaging mode");
    expect(plan.launcher_fallback == "foxpro_fxp_binary_generation_pending",
           "fxp-output plan should record the honest non-binary fallback state");
    expect(fs::path(plan.launcher_output_path).filename() == "CompileDemo.fxp",
           "fxp-output plan should preserve the requested output filename");
    expect(fs::path(plan.fxp_token_manifest_path).filename() == "CompileDemo.fxp.tokens",
           "fxp-output plan should derive a matching token-manifest filename");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "fxp-output package should materialize");
    if (result.ok) {
        expect(fs::exists(result.plan.fxp_token_manifest_path),
               "fxp-output package should emit a token manifest");
        expect(fs::exists(result.plan.launcher_output_path),
               "fxp-output package should materialize an honest FXP contract file");
        expect(!fs::exists(result.plan.runtime_host_destination_path),
               "fxp-output package should not bundle an executable runtime host into the FXP output slot");
        expect(result.plan.primary_output_materialized,
               "fxp-output package should report that the FXP contract file is materialized");

        const std::string token_manifest = read_text(result.plan.fxp_token_manifest_path);
        expect(token_manifest.find("output_kind=fxp") != std::string::npos,
               "fxp-output token manifest should declare the FXP output kind");
        expect(token_manifest.find("token_contract=logical_statements") != std::string::npos,
               "fxp-output token manifest should declare the token-contract mode");
        expect(token_manifest.find("primary_output=CompileDemo.fxp") != std::string::npos,
               "fxp-output token manifest should name the requested FXP file");
        expect(token_manifest.find("program=main.prg") != std::string::npos,
               "fxp-output token manifest should list the source program");
        expect(token_manifest.find("statement=MAIN|") != std::string::npos,
               "fxp-output token manifest should include main-scope statements");
        expect(token_manifest.find("DO worker") != std::string::npos,
               "fxp-output token manifest should preserve logical statement text");
        expect(token_manifest.find("statement=worker|") != std::string::npos,
               "fxp-output token manifest should include routine-scope statements");
        expect(token_manifest.find("WAIT WINDOW 'hello'") != std::string::npos,
               "fxp-output token manifest should preserve routine statement text");

        const std::string fxp_contract = read_text(result.plan.launcher_output_path);
        expect(fxp_contract.find("copperfin_fxp_contract_version=1") != std::string::npos,
               "fxp-output primary output should identify the Copperfin FXP contract format");
        expect(fxp_contract.find("token_contract=copperfin_logical_statement_contract_v1") != std::string::npos,
               "fxp-output primary output should declare the Copperfin FXP contract");
        expect(fxp_contract.find("token_manifest=" + quote_manifest_value(result.plan.fxp_token_manifest_path)) != std::string::npos,
               "fxp-output primary output should point back to the token manifest");
        expect(fxp_contract.find("output_kind=fxp") != std::string::npos,
               "fxp-output primary output should embed the FXP token-manifest content");
        expect(fxp_contract.find("statement=MAIN|") != std::string::npos,
               "fxp-output primary output should preserve main-scope logical statements");
        expect(fxp_contract.find("statement=worker|") != std::string::npos,
               "fxp-output primary output should preserve routine-scope logical statements");

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        expect(runtime_manifest.find("output_kind=fxp") != std::string::npos,
               "fxp-output manifest should record FXP output kind");
        expect(runtime_manifest.find("fxp_token_manifest_path=" + quote_manifest_value(result.plan.fxp_token_manifest_path)) != std::string::npos,
               "fxp-output manifest should record the emitted token-manifest path");
        expect(runtime_manifest.find("primary_output_materialized=true") != std::string::npos,
               "fxp-output manifest should record the materialized FXP contract file");
        expect(runtime_manifest.find("extension_payload=" + quote_manifest_value(result.plan.launcher_output_path) + "|") != std::string::npos,
               "fxp-output manifest should record the emitted FXP contract as an extension payload");
        expect(runtime_manifest.find("feature_flag=build.output.fxp_token_contract|true|build_output") != std::string::npos,
               "fxp-output manifest should expose the FXP token-contract feature flag");
        expect(debug_manifest.find("output_kind=fxp") != std::string::npos,
               "fxp-output debug manifest should record FXP output kind");
        expect(debug_manifest.find("launcher_mode=foxpro_tokenized_contract") != std::string::npos,
               "fxp-output debug manifest should record the tokenized-contract mode");
    }

    fs::remove_all(temp_root, ignored);
}

void run_library_output_warning_debug_manifest_smoke(const std::string& output_kind, const std::string& extension) {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / ("copperfin_runtime_pipeline_library_warning_" + extension);
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg", "RETURN\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / ("library_warning_" + extension + ".pjx")).string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "LibraryWarning";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "LibraryWarning";
    workspace.build_plan.output_path = (output_dir / ("LibraryWarning." + extension)).string();
    workspace.build_plan.output_kind = output_kind;
    workspace.build_plan.build_target = "warning-path regression";
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        true);

    expect(plan.ok, "library warning-path plan should be created for " + extension + " outputs");
    const std::string export_warning = "No PRG routine exports were discovered for the library output contract.";
    expect(std::find(plan.warnings.begin(), plan.warnings.end(), export_warning) != plan.warnings.end(),
           "library warning-path plan should surface the no-export warning for " + extension + " outputs");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "library warning-path package should materialize for " + extension + " outputs");
    if (result.ok) {
        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        const std::string warning_line = "warning=" + quote_manifest_value(export_warning);
        expect(runtime_manifest.find(warning_line) != std::string::npos,
               "library warning-path runtime manifest should record the no-export warning for " + extension + " outputs");
        expect(debug_manifest.find(warning_line) != std::string::npos,
               "library warning-path debug manifest should mirror the no-export warning for " + extension + " outputs");
    }

    fs::remove_all(temp_root, ignored);
}

void test_library_output_warning_lines_are_mirrored_into_debug_manifest() {
    run_library_output_warning_debug_manifest_smoke("dll", "dll");
    run_library_output_warning_debug_manifest_smoke("fll", "fll");
}

void test_app_output_package_emits_archive_manifest_for_staged_assets() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_app_contract";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg", "DO helper\nRETURN\n");
    write_text(project_dir / "helper.prg", "WAIT WINDOW 'archived'\nRETURN\n");
    write_text(project_dir / "config.txt", "mode=demo");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "archivedemo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "ArchiveDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "ArchiveDemo";
    workspace.build_plan.output_path = (output_dir / "ArchiveDemo.app").string();
    workspace.build_plan.output_kind = "app";
    workspace.build_plan.build_target = "x64 Visual FoxPro application archive";
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"},
        {.record_index = 2U, .name = "helper.prg", .relative_path = "helper.prg", .type_title = "Program"},
        {.record_index = 3U, .name = "config.txt", .relative_path = "config.txt", .type_title = "Text"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        true);

    expect(plan.ok, "app-output plan should be created");
    expect(plan.output_kind == copperfin::runtime::BuildOutputKind::app,
           "app-output plan should preserve APP output kind");
    expect(!plan.emit_dotnet_launcher,
           "app-output plan should not route through .NET launcher emission");
    expect(plan.launcher_mode == "foxpro_application_archive_contract",
           "app-output plan should switch to the archive-contract packaging mode");
    expect(plan.launcher_fallback == "foxpro_app_binary_generation_pending",
           "app-output plan should record the honest non-binary fallback state");
    expect(fs::path(plan.launcher_output_path).filename() == "ArchiveDemo.app",
           "app-output plan should preserve the requested output filename");
    expect(fs::path(plan.app_archive_manifest_path).filename() == "ArchiveDemo.app.contents",
           "app-output plan should derive a matching archive-manifest filename");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "app-output package should materialize");
    if (result.ok) {
        expect(fs::exists(result.plan.app_archive_manifest_path),
               "app-output package should emit an archive manifest");
        expect(fs::exists(result.plan.launcher_output_path),
               "app-output package should materialize an honest APP archive contract");
        expect(!fs::exists(result.plan.runtime_host_destination_path),
               "app-output package should not bundle an executable runtime host into the APP output slot");
        expect(result.plan.primary_output_materialized,
               "app-output package should report that the APP archive contract is materialized");
        expect(fs::exists(fs::path(result.plan.content_root) / "main.prg"),
               "app-output package should still stage the startup program");
        expect(fs::exists(fs::path(result.plan.content_root) / "helper.prg"),
               "app-output package should still stage supporting program assets");
        expect(fs::exists(fs::path(result.plan.content_root) / "config.txt"),
               "app-output package should still stage non-program assets");

        const std::string archive_manifest = read_text(result.plan.app_archive_manifest_path);
        expect(archive_manifest.find("output_kind=app") != std::string::npos,
               "app-output archive manifest should declare the APP output kind");
        expect(archive_manifest.find("archive_contract=staged_content_manifest") != std::string::npos,
               "app-output archive manifest should declare the archive-contract mode");
        expect(archive_manifest.find("primary_output=ArchiveDemo.app") != std::string::npos,
               "app-output archive manifest should name the requested APP file");
        expect(archive_manifest.find("startup_item=main.prg") != std::string::npos,
               "app-output archive manifest should record the startup item");
        expect(archive_manifest.find("asset=main.prg|Program|true|true") != std::string::npos,
               "app-output archive manifest should record the staged startup program asset");
        expect(archive_manifest.find("asset=helper.prg|Program|false|true") != std::string::npos,
               "app-output archive manifest should record staged supporting program assets");
        expect(archive_manifest.find("asset=config.txt|Text|false|true") != std::string::npos,
               "app-output archive manifest should record staged non-program assets");

        const std::string app_archive = read_text(result.plan.launcher_output_path);
        expect(app_archive.find("copperfin_app_archive_version=1") != std::string::npos,
               "app-output primary output should identify the Copperfin APP archive format");
        expect(app_archive.find("archive_contract=copperfin_content_archive_v1") != std::string::npos,
               "app-output primary output should declare the APP archive contract");
        expect(app_archive.find("content_manifest=" + quote_manifest_value(result.plan.app_archive_manifest_path)) != std::string::npos,
               "app-output primary output should point back to the staged-content manifest");
        const auto archive_payloads = parse_app_archive_payloads(app_archive);
        expect(archive_payloads.contains("main.prg"),
               "app-output primary archive should carry the startup program payload");
        expect(archive_payloads.contains("helper.prg"),
               "app-output primary archive should carry supporting program payloads");
        expect(archive_payloads.contains("config.txt"),
               "app-output primary archive should carry non-program payloads");
        if (archive_payloads.contains("main.prg")) {
            expect(archive_payloads.at("main.prg") == "DO helper\nRETURN\n",
                   "app-output primary archive should preserve startup program bytes");
        }
        if (archive_payloads.contains("helper.prg")) {
            expect(archive_payloads.at("helper.prg") == "WAIT WINDOW 'archived'\nRETURN\n",
                   "app-output primary archive should preserve supporting program bytes");
        }
        if (archive_payloads.contains("config.txt")) {
            expect(archive_payloads.at("config.txt") == "mode=demo",
                   "app-output primary archive should preserve non-program asset bytes");
        }

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        expect(runtime_manifest.find("output_kind=app") != std::string::npos,
               "app-output manifest should record APP output kind");
        expect(runtime_manifest.find("app_archive_manifest_path=" + quote_manifest_value(result.plan.app_archive_manifest_path)) != std::string::npos,
               "app-output manifest should record the emitted archive-manifest path");
        expect(runtime_manifest.find("primary_output_materialized=true") != std::string::npos,
               "app-output manifest should record the materialized primary archive");
        expect(runtime_manifest.find("extension_payload=" + quote_manifest_value(result.plan.launcher_output_path) + "|") != std::string::npos,
               "app-output manifest should record the emitted APP archive as an extension payload");
        expect(runtime_manifest.find("feature_flag=build.output.app_archive_contract|true|build_output") != std::string::npos,
               "app-output manifest should expose the APP archive-contract feature flag");
        expect(debug_manifest.find("output_kind=app") != std::string::npos,
               "app-output debug manifest should record APP output kind");
        expect(debug_manifest.find("launcher_mode=foxpro_application_archive_contract") != std::string::npos,
               "app-output debug manifest should record the archive-contract mode");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_package_emits_ast_manifest_for_prg_sources() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_ast_contract";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg",
               "LOCAL nValue\n"
               "nValue = 1\n"
               "DO worker\n"
               "RETURN\n"
               "PROCEDURE worker\n"
               "WAIT WINDOW 'ast'\n"
               "RETURN\n"
               "ENDPROC\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "astdemo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "AstDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "AstDemo";
    workspace.build_plan.output_path = (output_dir / "AstDemo.exe").string();
    workspace.build_plan.output_kind = "executable";
    workspace.build_plan.build_target = "x64 Windows executable";
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        false);

    expect(plan.ok, "ast-output plan should be created");
    expect(fs::path(plan.ast_manifest_path).filename() == "AstDemo.exe.ast.json",
           "ast-output plan should derive a target-specific AST manifest filename");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "ast-output package should materialize");
    if (result.ok) {
        expect(fs::exists(result.plan.ast_manifest_path),
               "ast-output package should emit an AST manifest");

        const std::string ast_manifest = read_text(result.plan.ast_manifest_path);
        expect(ast_manifest.find("\"schema_version\": 1") != std::string::npos,
               "ast manifest should declare the schema version");
        expect(ast_manifest.find("\"project_title\": \"AstDemo\"") != std::string::npos,
               "ast manifest should record the project title");
        expect(ast_manifest.find("\"output_kind\": \"executable\"") != std::string::npos,
               "ast manifest should record the selected output kind");
        expect(ast_manifest.find("\"relative_path\": \"main.prg\"") != std::string::npos,
               "ast manifest should record the source-relative program path");
        expect(ast_manifest.find("\"name\": \"MAIN\"") != std::string::npos,
               "ast manifest should emit the MAIN routine");
        expect(ast_manifest.find("\"text\": \"DO worker\"") != std::string::npos,
               "ast manifest should preserve main-scope statement text");
        expect(ast_manifest.find("\"name\": \"worker\"") != std::string::npos,
               "ast manifest should emit named routines");
        expect(ast_manifest.find("\"text\": \"WAIT WINDOW 'ast'\"") != std::string::npos,
               "ast manifest should preserve routine statement text");

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        expect(runtime_manifest.find("ast_manifest_path=" + quote_manifest_value(result.plan.ast_manifest_path)) != std::string::npos,
               "runtime manifest should record the AST-manifest path");
        expect(runtime_manifest.find("feature_flag=build.output.ast_contract|true|build_output") != std::string::npos,
               "runtime manifest should expose the AST-contract feature flag");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_package_emits_ir_manifest_with_instruction_mapping() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_ir_contract";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg",
               "LOCAL nValue\n"
               "nValue = 1\n"
               "DO worker\n"
               "RETURN\n"
               "PROCEDURE worker\n"
               "WAIT WINDOW 'ir'\n"
               "RETURN\n"
               "ENDPROC\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "irdemo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "IrDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "IrDemo";
    workspace.build_plan.output_path = (output_dir / "IrDemo.exe").string();
    workspace.build_plan.output_kind = "executable";
    workspace.build_plan.build_target = "x64 Windows executable";
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        false);

    expect(plan.ok, "ir-output plan should be created");
    expect(fs::path(plan.ir_manifest_path).filename() == "IrDemo.exe.ir.json",
           "ir-output plan should derive a target-specific IR manifest filename");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "ir-output package should materialize");
    if (result.ok) {
        expect(fs::exists(result.plan.ir_manifest_path),
               "ir-output package should emit an IR manifest");

        const std::string ir_manifest = read_text(result.plan.ir_manifest_path);
        expect(ir_manifest.find("\"schema_version\": 1") != std::string::npos,
               "ir manifest should declare the schema version");
        expect(ir_manifest.find("\"project_title\": \"IrDemo\"") != std::string::npos,
               "ir manifest should record the project title");
        expect(ir_manifest.find("\"output_kind\": \"executable\"") != std::string::npos,
               "ir manifest should record the selected output kind");
        expect(ir_manifest.find("\"relative_path\": \"main.prg\"") != std::string::npos,
               "ir manifest should record the source-relative program path");
        expect(ir_manifest.find("\"name\": \"MAIN\"") != std::string::npos,
               "ir manifest should emit the MAIN routine");
        expect(ir_manifest.find("\"opcode\": \"local_declaration\"") != std::string::npos,
               "ir manifest should map LOCAL statements to a stable opcode");
        expect(ir_manifest.find("\"opcode\": \"assignment\"") != std::string::npos,
               "ir manifest should map assignments to a stable opcode");
        expect(ir_manifest.find("\"opcode\": \"do_command\"") != std::string::npos,
               "ir manifest should map DO statements to a stable opcode");
        expect(ir_manifest.find("\"opcode\": \"wait_command\"") != std::string::npos,
               "ir manifest should map WAIT WINDOW statements to a stable opcode");
        expect(ir_manifest.find("\"name\": \"worker\"") != std::string::npos,
               "ir manifest should emit named routines");

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        expect(runtime_manifest.find("ir_manifest_path=" + quote_manifest_value(result.plan.ir_manifest_path)) != std::string::npos,
               "runtime manifest should record the IR-manifest path");
        expect(runtime_manifest.find("feature_flag=build.output.ir_contract|true|build_output") != std::string::npos,
               "runtime manifest should expose the IR-contract feature flag");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_package_emits_csharp_transpilation_for_procedural_prg_code() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_csharp_contract";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg",
               "LOCAL nValue\n"
               "nValue = 1\n"
               "DO worker\n"
               "RETURN\n"
               "PROCEDURE worker\n"
               "WAIT WINDOW 'csharp'\n"
               "RETURN\n"
               "ENDPROC\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "csharpdemo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "CSharpDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "CSharpDemo";
    workspace.build_plan.output_path = (output_dir / "CSharpDemo.exe").string();
    workspace.build_plan.output_kind = "executable";
    workspace.build_plan.build_target = "x64 Windows executable";
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        true);

    expect(plan.ok, "csharp-output plan should be created");
    expect(fs::path(plan.transpiled_csharp_path).filename() == "CSharpDemo.exe.transpiled.cs",
           "csharp-output plan should derive a target-specific transpilation filename");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "csharp-output package should materialize");
    if (result.ok) {
        expect(fs::exists(result.plan.transpiled_csharp_path),
               "csharp-output package should emit a C# transpilation artifact");

        const std::string transpiled = read_text(result.plan.transpiled_csharp_path);
        expect(transpiled.find("public static class TranspiledProgram") != std::string::npos,
               "csharp transpilation should emit the generated container type");
        expect(transpiled.find("public static void MainRoutine()") != std::string::npos,
               "csharp transpilation should emit a main routine");
        expect(transpiled.find("dynamic nValue = null;") != std::string::npos,
               "csharp transpilation should map LOCAL declarations to dynamic locals");
        expect(transpiled.find("nValue = 1;") != std::string::npos,
               "csharp transpilation should preserve simple assignments");
        expect(transpiled.find("Worker();") != std::string::npos,
               "csharp transpilation should map DO worker to a routine call");
        expect(transpiled.find("public static void worker()") != std::string::npos ||
               transpiled.find("public static void Worker()") != std::string::npos,
               "csharp transpilation should emit the called FoxPro routine");
        expect(transpiled.find("Console.WriteLine(\"csharp\");") != std::string::npos,
               "csharp transpilation should map WAIT WINDOW literal output to Console.WriteLine");
        if (dotnet_is_available()) {
            std::string compile_error;
            const bool compiled = compile_csharp_artifact(result.plan.transpiled_csharp_path, compile_error);
            if (!compiled && !compile_error.empty()) {
                std::cerr << "FAIL: " << compile_error << "\n";
            }
            expect(compiled,
                   "csharp transpilation should compile under dotnet");
        }

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        expect(runtime_manifest.find("transpiled_csharp_path=" + quote_manifest_value(result.plan.transpiled_csharp_path)) != std::string::npos,
               "runtime manifest should record the transpiled C# artifact path");
        expect(runtime_manifest.find("feature_flag=build.output.csharp_transpilation|true|build_output") != std::string::npos,
               "runtime manifest should expose the C# transpilation feature flag");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_package_emits_csharp_transpilation_for_class_library_objects() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_csharp_xasset_contract";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    const fs::path class_library_path = project_dir / "widget.vcx";
    write_synthetic_class_library_asset(class_library_path);
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "widgetdemo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "WidgetDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "WidgetDemo";
    workspace.build_plan.output_path = (output_dir / "WidgetDemo.exe").string();
    workspace.build_plan.output_kind = "executable";
    workspace.build_plan.build_target = "x64 Windows executable";
    workspace.build_plan.startup_item = "widget.vcx";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "widget.vcx", .relative_path = "widget.vcx", .type_title = "Class Library"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        true);

    expect(plan.ok, "class-library csharp-output plan should be created");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "class-library csharp-output package should materialize");
    if (result.ok) {
        expect(fs::exists(result.plan.transpiled_csharp_path),
               "class-library csharp-output package should emit a C# transpilation artifact");

        const std::string transpiled = read_text(result.plan.transpiled_csharp_path);
        expect(transpiled.find("public sealed class CustWidget") != std::string::npos,
               "class-library transpilation should emit a concrete C# type for the root object");
        expect(transpiled.find("public void Load()") != std::string::npos,
               "class-library transpilation should surface the root Load lifecycle method");
        expect(transpiled.find("public void Init()") != std::string::npos,
               "class-library transpilation should surface the root Init lifecycle method");
        expect(transpiled.find("public void Destroy()") != std::string::npos,
               "class-library transpilation should surface the root Destroy lifecycle method");
        expect(transpiled.find("public void TxtName_Valid()") != std::string::npos,
               "class-library transpilation should surface nested object methods");
        expect(transpiled.find("public void RunStartup()") != std::string::npos,
               "class-library transpilation should emit an ordered startup wrapper");
        expect(transpiled.find("Load();") != std::string::npos &&
               transpiled.find("Init();") != std::string::npos,
               "class-library transpilation should preserve root startup ordering");
        expect(transpiled.find("public void RunShutdown()") != std::string::npos,
               "class-library transpilation should emit an ordered shutdown wrapper");
        expect(transpiled.find("Destroy();") != std::string::npos,
               "class-library transpilation should preserve root shutdown ordering");
        expect(transpiled.find("Manual port required for FoxPro xAsset method: custWidget.txtName.Valid") != std::string::npos,
               "class-library transpilation should stay honest about untranslated xAsset method bodies");

        if (dotnet_is_available()) {
            std::string compile_error;
            const bool compiled = compile_csharp_artifact(result.plan.transpiled_csharp_path, compile_error);
            if (!compiled && !compile_error.empty()) {
                std::cerr << "FAIL: " << compile_error << "\n";
            }
            expect(compiled,
                   "class-library csharp transpilation should compile under dotnet");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_manifest_records_generated_compiler_contract_digests() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_compiler_contract_digests";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg",
               "LOCAL nValue\n"
               "nValue = 1\n"
               "RETURN\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "contractdigests.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "ContractDigests";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "ContractDigests";
    workspace.build_plan.output_path = (output_dir / "ContractDigests.exe").string();
    workspace.build_plan.output_kind = "executable";
    workspace.build_plan.build_target = "x64 Windows executable";
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        true);

    expect(plan.ok, "compiler-contract-digest plan should be created");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "compiler-contract-digest package should materialize");
    if (result.ok) {
        const auto has_digest = [&](const std::string& path) {
            return std::find_if(
                       result.plan.compiler_contract_digests.begin(),
                       result.plan.compiler_contract_digests.end(),
                       [&](const copperfin::runtime::RuntimeArtifactDigest& digest) {
                           return digest.path == path && !digest.sha256.empty();
                       }) != result.plan.compiler_contract_digests.end();
        };

        expect(has_digest(result.plan.ast_manifest_path),
               "compiler-contract digests should include the AST artifact");
        expect(has_digest(result.plan.ir_manifest_path),
               "compiler-contract digests should include the IR artifact");
        expect(has_digest(result.plan.transpiled_csharp_path),
               "compiler-contract digests should include the transpiled C# artifact");

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        for (const auto& digest : result.plan.compiler_contract_digests) {
            expect(runtime_manifest.find("compiler_contract=" + quote_manifest_value(digest.path) + "|" + quote_manifest_value(digest.sha256)) != std::string::npos,
                   "runtime manifest should record each generated compiler-contract digest");
            expect(debug_manifest.find("compiler_contract=" + quote_manifest_value(digest.path) + "|" + quote_manifest_value(digest.sha256)) != std::string::npos,
                   "debug manifest should record each generated compiler-contract digest");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_startup_dbf_companion_assets_are_staged() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_dbf_companions";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "startup.dbf", "synthetic dbf");
    write_text(project_dir / "startup.fpt", "synthetic memo");
    write_text(project_dir / "startup.cdx", "synthetic index");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "companion_demo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "DbfCompanionDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "DbfCompanionDemo";
    workspace.build_plan.output_path = (output_dir / "DbfCompanionDemo.exe").string();
    workspace.build_plan.startup_item = "startup.dbf";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "startup.dbf", .relative_path = "startup.dbf", .type_title = "Table", .excluded = true}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        false);

    expect(plan.ok, "dbf companion runtime package plan should be created");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "dbf companion runtime package should materialize");
    if (result.ok) {
        const std::filesystem::path content_root(result.plan.content_root);
        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        expect(fs::exists(content_root / "startup.dbf"), "startup DBF should be staged even when marked excluded");
        expect(fs::exists(content_root / "startup.fpt"), "startup DBF memo companion should be staged");
        expect(fs::exists(content_root / "startup.cdx"), "startup DBF index companion should be staged");
        expect(
            runtime_manifest.find("asset=1|startup.dbf|") != std::string::npos &&
            runtime_manifest.find("asset=1|startup.dbf|") < runtime_manifest.find("|true|true|") &&
            runtime_manifest.find("|true|true|", runtime_manifest.find("asset=1|startup.dbf|")) != std::string::npos,
            "runtime manifest should report the startup DBF asset as copied");
    }

    fs::remove_all(temp_root, ignored);
}

void test_security_enabled_runtime_host_name_validation() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_security_tests";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path canonical_runtime_host = runtime_host_fixture_path(temp_root);
    const fs::path non_canonical_runtime_host = temp_root / "runtime_host_custom.exe";

    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg", "RETURN\n");
    write_text(canonical_runtime_host, "runtime-host");
    write_text(non_canonical_runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "secure_demo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "SecureDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "SecureDemo";
    workspace.build_plan.output_path = (output_dir / "SecureDemo.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"}
    };

    const auto secure_plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        true,
        false);

    expect(secure_plan.ok, "security-enabled plan should be created");

    const auto rejected_result = copperfin::runtime::materialize_runtime_package(
        secure_plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        non_canonical_runtime_host.string());

    expect(!rejected_result.ok, "security-enabled packaging should reject non-standard runtime host names");

    const auto accepted_result = copperfin::runtime::materialize_runtime_package(
        secure_plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        canonical_runtime_host.string());

    expect(accepted_result.ok, "security-enabled packaging should accept canonical runtime host name");

    fs::remove_all(temp_root, ignored);
}

void test_runtime_security_role_environment_fidelity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_security_role_fidelity";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);

    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    write_text(project_dir / "main.prg", "RETURN\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "security_role_fidelity.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "SecurityRoleFidelity";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "SecurityRoleFidelity";
    workspace.build_plan.output_path = (output_dir / "SecurityRoleFidelity.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"}
    };

    {
        ScopedEnvironmentVariable valid_role("COPPERFIN_SECURITY_ROLE", "security-admin");
        const auto plan = copperfin::runtime::create_runtime_package_plan(
            document,
            workspace,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile(),
            output_dir.string(),
            copperfin::runtime::BuildConfiguration::debug,
            true,
            true);

        expect(plan.security_role == "security-admin", "security role should accept explicit valid role from environment");
        expect(std::find(plan.warnings.begin(), plan.warnings.end(), plan.security_role) == plan.warnings.end() &&
               std::none_of(plan.warnings.begin(), plan.warnings.end(), [](const std::string& warning) {
                   return warning.find("Unknown security role requested") != std::string::npos;
               }),
               "security plan should not emit unknown-role warning for valid role");
    }

    {
        ScopedEnvironmentVariable invalid_role("COPPERFIN_SECURITY_ROLE", "not-a-real-role");
        const auto invalid_plan = copperfin::runtime::create_runtime_package_plan(
            document,
            workspace,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile(),
            output_dir.string(),
            copperfin::runtime::BuildConfiguration::debug,
            true,
            true);
        expect(invalid_plan.security_role == "developer", "invalid security role should fallback to default developer role");
        expect(std::any_of(invalid_plan.warnings.begin(), invalid_plan.warnings.end(), [](const std::string& warning) {
                   return warning.find("Unknown security role requested") != std::string::npos;
               }),
               "invalid security role should emit explicit unknown-role warning");

        const auto materialize_invalid = copperfin::runtime::materialize_runtime_package(
            invalid_plan,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile(),
            runtime_host.string());
        expect(materialize_invalid.ok, "package materialization should proceed with defaulted security role");
        if (materialize_invalid.ok) {
            const std::string runtime_manifest = read_text(materialize_invalid.plan.manifest_path);
            expect(runtime_manifest.find("security_role=developer") != std::string::npos,
                   "runtime manifest should record fallback security role after invalid role request");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_materialize_fails_before_asset_staging_when_runtime_host_source_is_invalid() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_failfast_invalid_host";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path invalid_runtime_host = temp_root / "missing_runtime_host.exe";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg", "RETURN\n");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "failfast_host.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "FailFastHost";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "FailFastHost";
    workspace.build_plan.output_path = (output_dir / "FailFastHost.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        false);

    expect(plan.ok, "fail-fast invalid-host plan should be created");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        invalid_runtime_host.string());

    expect(!result.ok, "invalid runtime host source should fail materialization");
    expect(!fs::exists(fs::path(plan.content_root) / "main.prg"),
           "invalid runtime host source should fail before staging startup assets");

    fs::remove_all(temp_root, ignored);
}

void test_startup_prg_extension_matching_is_case_insensitive() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_case_insensitive_startup";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "MAIN.PRG", "RETURN\n");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "case_demo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "CaseDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "CaseDemo";
    workspace.build_plan.output_path = (output_dir / "CaseDemo.exe").string();
    workspace.build_plan.startup_item = "MAIN.PRG";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "MAIN.PRG", .relative_path = "MAIN.PRG", .type_title = "Program"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        false);

    expect(plan.ok, "runtime package plan should be created for uppercase PRG startup");
    expect(plan.debug_plan.supports_breakpoints,
           "uppercase .PRG startup should enable breakpoint support");
    expect(plan.debug_plan.supports_step_debugging,
           "uppercase .PRG startup should enable step-debug support");

    fs::remove_all(temp_root, ignored);
}

void test_startup_asset_is_staged_even_when_marked_excluded() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_startup_excluded_stage";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "MAIN.PRG", "RETURN\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "startup_excluded.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "StartupExcluded";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "StartupExcluded";
    workspace.build_plan.output_path = (output_dir / "StartupExcluded.exe").string();
    workspace.build_plan.startup_item = "MAIN.PRG";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "MAIN.PRG", .relative_path = "MAIN.PRG", .type_title = "Program", .excluded = true}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        false);

    expect(plan.ok, "runtime package plan should be created when startup asset is excluded");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "runtime package should materialize when startup asset is excluded");
    if (result.ok) {
        expect(fs::exists(fs::path(result.plan.content_root) / "MAIN.PRG"),
               "startup program should still be staged even when entry is marked excluded");
    }

    fs::remove_all(temp_root, ignored);
}

void test_missing_startup_record_surfaces_plan_warnings_and_disables_debug_startup_support() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_missing_startup_record";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg", "RETURN\n");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "missing_startup.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "MissingStartup";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "MissingStartup";
    workspace.build_plan.output_path = (output_dir / "MissingStartup.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 42U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        false);

    expect(plan.ok, "runtime package plan should still be creatable when startup record is unresolved");
    expect(!plan.debug_plan.supports_breakpoints,
           "missing startup record should disable debug startup breakpoint support");
    expect(!plan.debug_plan.supports_step_debugging,
           "missing startup record should disable debug startup step-debug support");
    const bool has_runtime_startup_warning = std::any_of(
        plan.warnings.begin(),
        plan.warnings.end(),
        [](const std::string& warning) {
            return warning.find("No startup source asset could be resolved.") != std::string::npos;
        });
    const bool has_debug_startup_warning = std::any_of(
        plan.warnings.begin(),
        plan.warnings.end(),
        [](const std::string& warning) {
            return warning.find("No source-side startup asset could be resolved for debugging.") != std::string::npos;
        });
    expect(has_runtime_startup_warning, "missing startup record should emit runtime startup resolution warning");
    expect(has_debug_startup_warning, "missing startup record should emit debug startup resolution warning");

    fs::remove_all(temp_root, ignored);
}

void test_manifest_asset_lines_include_copy_state_contract() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_manifest_asset_copy_state";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg", "RETURN\n");
    write_text(project_dir / "excluded.txt", "do not stage");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "manifest_contract.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "ManifestAssetContract";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "ManifestAssetContract";
    workspace.build_plan.output_path = (output_dir / "ManifestAssetContract.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"},
        {.record_index = 2U, .name = "excluded.txt", .relative_path = "excluded.txt", .type_title = "Text", .excluded = true}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        false);

    expect(plan.ok, "manifest-asset-copy-state plan should be created");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect(result.ok, "manifest-asset-copy-state package should materialize");
    if (result.ok) {
        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string startup_line_marker = "asset=1|main.prg|";
        const std::string excluded_line_marker = "asset=2|excluded.txt|";
        const std::size_t startup_line_pos = runtime_manifest.find(startup_line_marker);
        const std::size_t excluded_line_pos = runtime_manifest.find(excluded_line_marker);
        expect(startup_line_pos != std::string::npos,
               "runtime manifest should include startup asset line");
        expect(excluded_line_pos != std::string::npos,
               "runtime manifest should include excluded asset line");

        const bool startup_copied = startup_line_pos != std::string::npos &&
            runtime_manifest.find("|true\n", startup_line_pos) != std::string::npos;
        const bool excluded_not_copied = excluded_line_pos != std::string::npos &&
            runtime_manifest.find("|false\n", excluded_line_pos) != std::string::npos;
        expect(startup_copied, "startup asset line should report copied=true in manifest contract");
        expect(excluded_not_copied, "excluded non-runtime asset line should report copied=false in manifest contract");
    }

    fs::remove_all(temp_root, ignored);
}

void test_debug_source_roots_are_unique_when_source_and_content_paths_match() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_debug_roots_unique";
    const fs::path output_dir = temp_root / "output";
    const std::string project_title = "SourceRootParity";
    const fs::path project_dir = output_dir / project_title / "content";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "main.prg", "RETURN\n");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "source_root_parity.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = project_title;
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = project_title;
    workspace.build_plan.output_path = (output_dir / "SourceRootParity.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        false);

    expect(plan.ok, "debug source-root uniqueness plan should be created");
    expect(plan.debug_plan.source_roots.size() == 1U,
           "debug source roots should collapse to one unique path when source and content roots match");

    const std::string debug_manifest = copperfin::runtime::build_debug_manifest_text(plan);
    const std::string expected_roots_line = "source_roots=" + project_dir.lexically_normal().string();
    expect(debug_manifest.find(expected_roots_line) != std::string::npos,
           "debug manifest should emit a single normalized source_roots entry");

    fs::remove_all(temp_root, ignored);
}

void test_debug_source_roots_preserve_source_first_and_content_second_order() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_debug_roots_order";
    const fs::path source_root = temp_root / "source";
    const fs::path output_dir = temp_root / "output";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_root);

    write_text(source_root / "main.prg", "RETURN\n");

    copperfin::studio::StudioDocumentModel document;
    document.path = (source_root / "debug_roots_order.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "DebugRootsOrder";
    workspace.home_directory = source_root.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "DebugRootsOrder";
    workspace.build_plan.output_path = (output_dir / "DebugRootsOrder.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "main.prg", .relative_path = "main.prg", .type_title = "Program"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        false);

    expect(plan.ok, "ordered debug source-root plan should be created");
    expect(plan.debug_plan.source_roots.size() == 2U,
           "ordered debug source-root plan should preserve both source and content roots");
    if (plan.debug_plan.source_roots.size() == 2U) {
        expect(plan.debug_plan.source_roots.front() == source_root.lexically_normal().string(),
               "debug source roots should keep the source-side working directory first");
        expect(plan.debug_plan.source_roots.back() == (output_dir / "DebugRootsOrder" / "content").lexically_normal().string(),
               "debug source roots should keep the packaged content root second");
    }

    const std::string debug_manifest = copperfin::runtime::build_debug_manifest_text(plan);
    const std::string expected_roots_line =
        "source_roots=" + source_root.lexically_normal().string() + ";" +
        (output_dir / "DebugRootsOrder" / "content").lexically_normal().string();
    expect(debug_manifest.find(expected_roots_line) != std::string::npos,
           "debug manifest should preserve source-first source_roots ordering");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_materialize_runtime_package();
    test_generated_launcher_forwards_manifest_and_debug_flag();
    test_materialize_excluded_xasset_startup_package();
    test_dotnet_launcher_request_falls_back_to_native_host_when_unavailable();
    test_library_output_package_emits_module_definition_from_prg_routines();
    test_fll_output_package_emits_api_manifest_from_prg_routines();
    test_library_output_warning_lines_are_mirrored_into_debug_manifest();
    test_fxp_output_package_emits_token_manifest_from_prg_statements();
    test_app_output_package_emits_archive_manifest_for_staged_assets();
    test_runtime_package_emits_ast_manifest_for_prg_sources();
    test_runtime_package_emits_ir_manifest_with_instruction_mapping();
    test_runtime_package_emits_csharp_transpilation_for_procedural_prg_code();
    test_runtime_package_emits_csharp_transpilation_for_class_library_objects();
    test_runtime_manifest_records_generated_compiler_contract_digests();
    test_startup_dbf_companion_assets_are_staged();
    test_security_enabled_runtime_host_name_validation();
    test_runtime_security_role_environment_fidelity();
    test_materialize_fails_before_asset_staging_when_runtime_host_source_is_invalid();
    test_startup_prg_extension_matching_is_case_insensitive();
    test_startup_asset_is_staged_even_when_marked_excluded();
    test_missing_startup_record_surfaces_plan_warnings_and_disables_debug_startup_support();
    test_manifest_asset_lines_include_copy_state_contract();
    test_debug_source_roots_are_unique_when_source_and_content_paths_match();
    test_debug_source_roots_preserve_source_first_and_content_second_order();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
