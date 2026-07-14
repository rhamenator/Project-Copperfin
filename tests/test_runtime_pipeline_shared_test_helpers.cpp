// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_runtime_pipeline_support.h"
#include "test_environment_support.h"

namespace cf_test_runtime_pipeline {
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

std::filesystem::path runtime_pipeline_locale_root() {
    return std::filesystem::path(COPPERFIN_TEST_SOURCE_DIR) / "resources" / "locales";
}

const copperfin::localization::LocalizedCatalog& runtime_pipeline_english_catalog() {
    static const auto catalog = copperfin::localization::load_catalogs(
        runtime_pipeline_locale_root(),
        "en-US");
    return catalog;
}

std::size_t count_missing_locale_keys(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view locale,
    const std::vector<std::string_view>& keys) {
    const auto locale_entries = catalog.catalogs.find(std::string(locale));
    if (locale_entries == catalog.catalogs.end()) {
        return keys.size();
    }

    std::size_t missing = 0U;
    for (const auto key : keys) {
        if (locale_entries->second.find(std::string(key)) == locale_entries->second.end()) {
            ++missing;
        }
    }
    return missing;
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

std::string manifest_value_for_key(const std::string& text, const std::string& key) {
    std::istringstream input(text);
    std::string line;
    const std::string prefix = key + "=";
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

bool dotnet_is_available() {
#if defined(_WIN32)
    const char* argv[] = {"dotnet", "--version", nullptr};
    return _spawnvp(_P_WAIT, "dotnet", const_cast<char* const*>(argv)) == 0;
#else
    return std::system("command -v dotnet >/dev/null 2>&1") == 0;
#endif
}

std::string native_cxx_command() {
    const std::string value = getenv_value("CXX");
    if (!value.empty()) {
        return value;
    }
    return "c++";
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

bool ninja_multi_config_is_available() {
#if defined(_WIN32)
    return false;
#else
    return cmake_is_available() && std::system("command -v ninja >/dev/null 2>&1") == 0;
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
        "-fvisibility=hidden",
        "-fvisibility-inlines-hidden",
#endif
        source_path.string(),
        "-o",
        output_path.string()
    };
#if !defined(_WIN32) && !defined(__APPLE__)
    build_args.push_back("-ldl");
#endif

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
    if (copperfin::test_support::run_shell_command(configure_command) != 0) {
        error = "native wrapper CMake configure failed";
        if (fs::exists(configure_log_path)) {
            error += ":\n" + read_text(configure_log_path);
        }
        return false;
    }

    const std::string build_command =
        "cmake --build \"" + build_root.string() + "\" > \"" + build_log_path.string() + "\" 2>&1";
    if (copperfin::test_support::run_shell_command(build_command) != 0) {
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

bool build_native_wrapper_with_ninja_multi_config(
    const std::filesystem::path& cmake_lists_path,
    const std::filesystem::path& expected_output_path,
    std::string& error) {
    namespace fs = std::filesystem;
    const fs::path source_root = cmake_lists_path.parent_path();
    const fs::path build_root = source_root / "cmake_ninja_multi_config_check";
    const fs::path configure_log_path = build_root / "cmake-configure.log";
    const fs::path build_log_path = build_root / "cmake-build.log";
    std::error_code ignored;
    fs::remove_all(build_root, ignored);
    fs::remove(expected_output_path, ignored);
    fs::create_directories(build_root);

    const std::string configure_command =
        "cmake -G \"Ninja Multi-Config\" -S \"" + source_root.string() + "\" -B \"" +
        build_root.string() + "\" > \"" + configure_log_path.string() + "\" 2>&1";
    if (copperfin::test_support::run_shell_command(configure_command) != 0) {
        error = "native wrapper Ninja Multi-Config configure failed";
        if (fs::exists(configure_log_path)) {
            error += ":\n" + read_text(configure_log_path);
        }
        return false;
    }

    for (const std::string configuration : {"Debug", "Release"}) {
        fs::remove(expected_output_path, ignored);
        const std::string build_command =
            "cmake --build \"" + build_root.string() + "\" --config " + configuration +
            " > \"" + build_log_path.string() + "\" 2>&1";
        if (copperfin::test_support::run_shell_command(build_command) != 0) {
            error = "native wrapper Ninja Multi-Config " + configuration + " build failed";
            if (fs::exists(build_log_path)) {
                error += ":\n" + read_text(build_log_path);
            }
            return false;
        }
        if (!fs::exists(expected_output_path)) {
            error = "native wrapper Ninja Multi-Config " + configuration +
                " build did not produce the requested package-root artifact";
            return false;
        }
    }
    return true;
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
    if (copperfin::test_support::run_shell_command(command) != 0) {
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
#if defined(__APPLE__)
    const std::string command =
        "nm -gU \"" + binary_path.string() + "\" > \"" + log_path.string() + "\" 2>&1";
#else
    const std::string command =
        "nm -D --defined-only \"" + binary_path.string() + "\" > \"" + log_path.string() + "\" 2>&1";
#endif
    if (copperfin::test_support::run_shell_command(command) != 0) {
        error = "native wrapper symbol inspection failed";
        if (fs::exists(log_path)) {
            error += ":\n" + read_text(log_path);
        }
        return symbols;
    }

    std::istringstream input(read_text(log_path));
    std::string line;
    while (std::getline(input, line)) {
        std::istringstream line_input(line);
        std::vector<std::string> tokens;
        std::string token;
        while (line_input >> token) {
            tokens.push_back(token);
        }
        if (tokens.empty()) {
            continue;
        }
#if defined(__APPLE__)
        std::string symbol = tokens.back();
        if (!symbol.empty() && symbol.front() == '_') {
            symbol.erase(symbol.begin());
        }
        if (symbol.empty() || (symbol.front() == '_' && symbol != "_FoxTable")) {
            continue;
        }
#else
        if (tokens.size() < 3U || tokens[tokens.size() - 2U].size() != 1U) {
            continue;
        }
        const char symbol_type = tokens[tokens.size() - 2U].front();
        const unsigned char normalized_type = static_cast<unsigned char>(symbol_type);
        if (!std::isupper(normalized_type) || symbol_type == 'V' || symbol_type == 'W') {
            continue;
        }
        const std::string& symbol = tokens.back();
#endif
        symbols.insert(symbol);
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
    const std::string export_warning =
        runtime_pipeline_english_catalog().translate("Runtime.Package.Warning.LibraryExportsUnresolved");
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

}  // namespace cf_test_runtime_pipeline
