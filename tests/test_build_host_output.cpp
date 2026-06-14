#include "copperfin/vfp/dbf_table.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#if !defined(_WIN32)
#include <sys/wait.h>
#else
#include <process.h>
#endif

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

std::string getenv_value(const std::string& name) {
#ifdef _WIN32
    const char* value = std::getenv(name.c_str());
    if (value == nullptr) {
        return {};
    }
    return value;
#else
    const char* value = std::getenv(name.c_str());
    if (value == nullptr) {
        return {};
    }
    return value;
#endif
}

void set_env_value(const std::string& name, const std::string& value, bool has_value) {
#ifdef _WIN32
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

struct ScopedEnvironmentValue {
    std::string name;
    std::string original;
    bool had_original = false;

    explicit ScopedEnvironmentValue(const std::string& environment_name)
        : name(environment_name),
          original(getenv_value(name)) {
        had_original = !original.empty();
        set_env_value(name, "", false);
    }

    ~ScopedEnvironmentValue() {
        set_env_value(name, original, had_original);
    }
};

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
    std::string stderr_text;
};

ProcessResult run_process_capture(
    const std::string& executable_path,
    const std::vector<std::string>& arguments,
    const std::filesystem::path& working_directory) {
    namespace fs = std::filesystem;

    const fs::path resolved_executable_path = fs::absolute(executable_path);
    const fs::path stdout_path = working_directory / "build_host_stdout.log";
    const fs::path stderr_path = working_directory / "build_host_stderr.log";

    std::string command = quote_command_argument(resolved_executable_path.string());
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
    const int raw_exit_code = std::system(command.c_str());
    fs::current_path(original_directory);

    ProcessResult result;
    if (fs::exists(stdout_path)) {
        result.stdout_text = read_text(stdout_path);
    }
    if (fs::exists(stderr_path)) {
        result.stderr_text = read_text(stderr_path);
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

bool native_symbol_dump_is_available() {
#if defined(_WIN32)
    return false;
#else
    return std::system("command -v nm >/dev/null 2>&1") == 0;
#endif
}

std::set<std::string> read_native_exported_symbols(const std::filesystem::path& binary_path, std::string& error) {
    std::set<std::string> symbols;
#if defined(_WIN32)
    (void)binary_path;
    error = "native symbol inspection is not implemented on Windows hosts";
    return symbols;
#else
    namespace fs = std::filesystem;
    const fs::path log_path = binary_path.parent_path() / "build-host-symbols.log";
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
        std::istringstream line_input(line);
        std::string address;
        char symbol_type = '\0';
        std::string symbol;
        if (!(line_input >> address >> symbol_type >> symbol)) {
            continue;
        }
        const unsigned char normalized_type = static_cast<unsigned char>(symbol_type);
        if (!std::isupper(normalized_type) || symbol_type == 'V' || symbol_type == 'W') {
            continue;
        }
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
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line == "EXPORTS") {
            in_exports = true;
            continue;
        }
        if (!in_exports || line.empty()) {
            continue;
        }
        const std::size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) {
            continue;
        }
        const std::size_t end = line.find_first_of(" \t", start);
        exports.insert(line.substr(start, end == std::string::npos ? std::string::npos : end - start));
    }
    return exports;
}

std::set<std::string> read_fll_api_declared_symbols(const std::filesystem::path& path) {
    std::set<std::string> symbols;
    std::istringstream input(read_text(path));
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.rfind("function=", 0U) == 0U) {
            symbols.insert(line.substr(9U));
        } else if (line.rfind("loader_entrypoint=", 0U) == 0U) {
            symbols.insert(line.substr(18U));
        } else if (line.rfind("registration_symbol=", 0U) == 0U) {
            symbols.insert(line.substr(20U));
        }
    }
    return symbols;
}

std::set<std::string> read_library_api_declared_symbols(const std::filesystem::path& path) {
    std::set<std::string> symbols;
    std::istringstream input(read_text(path));
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.rfind("function=", 0U) == 0U) {
            symbols.insert(line.substr(9U));
        }
    }
    return symbols;
}

std::string value_for_key(const std::string& text, const std::string& key) {
    std::istringstream input(text);
    std::string line;
    const std::string prefix = key + ": ";
    while (std::getline(input, line)) {
        if (line.rfind(prefix, 0U) == 0U) {
            return line.substr(prefix.size());
        }
    }
    return {};
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

void write_synthetic_project(
    const std::filesystem::path& project_path,
    const std::filesystem::path& project_dir,
    const std::filesystem::path& output_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "TYPE", .type = 'C', .length = 1U},
        {.name = "KEY", .type = 'C', .length = 32U},
        {.name = "HOMEDIR", .type = 'C', .length = 200U},
        {.name = "OUTFILE", .type = 'C', .length = 200U},
        {.name = "NAME", .type = 'C', .length = 200U},
        {.name = "MAINPROG", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"H", "LibraryDemo", project_dir.string(), output_path.string(), "", "false"},
        {"K", "", "", "", "librarymain.prg", "true"},
        {"K", "", "", "", "helper.prg", "false"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(project_path.string(), fields, records);
    expect(create_result.ok, "synthetic PJX fixture should be created");
}

void write_synthetic_executable_project(
    const std::filesystem::path& project_path,
    const std::filesystem::path& project_dir,
    const std::filesystem::path& output_path,
    const std::string& project_title = "HostResolutionDemo") {
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
    const auto create_result = copperfin::vfp::create_dbf_table_file(project_path.string(), fields, records);
    expect(create_result.ok, "synthetic executable PJX fixture should be created");
}

void write_synthetic_app_project(
    const std::filesystem::path& project_path,
    const std::filesystem::path& project_dir,
    const std::filesystem::path& output_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "TYPE", .type = 'C', .length = 1U},
        {.name = "KEY", .type = 'C', .length = 32U},
        {.name = "HOMEDIR", .type = 'C', .length = 200U},
        {.name = "OUTFILE", .type = 'C', .length = 200U},
        {.name = "NAME", .type = 'C', .length = 200U},
        {.name = "MAINPROG", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"H", "ArchiveDemo", project_dir.string(), output_path.string(), "", "false"},
        {"K", "", "", "", "main.prg", "true"},
        {"K", "", "", "", "helper.prg", "false"},
        {"K", "", "", "", "config.txt", "false"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(project_path.string(), fields, records);
    expect(create_result.ok, "synthetic APP PJX fixture should be created");
}

void write_synthetic_fxp_project(
    const std::filesystem::path& project_path,
    const std::filesystem::path& project_dir,
    const std::filesystem::path& output_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "TYPE", .type = 'C', .length = 1U},
        {.name = "KEY", .type = 'C', .length = 32U},
        {.name = "HOMEDIR", .type = 'C', .length = 200U},
        {.name = "OUTFILE", .type = 'C', .length = 200U},
        {.name = "NAME", .type = 'C', .length = 200U},
        {.name = "MAINPROG", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"H", "CompileDemo", project_dir.string(), output_path.string(), "", "false"},
        {"K", "", "", "", "main.prg", "true"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(project_path.string(), fields, records);
    expect(create_result.ok, "synthetic FXP PJX fixture should be created");
}

void run_library_build_host_smoke(
    const std::string& build_host_path,
    const std::string& extension) {
    namespace fs = std::filesystem;

    expect(fs::exists(build_host_path), "build host executable should exist before running the smoke test");
    const fs::path temp_root = fs::temp_directory_path() / ("copperfin_build_host_" + extension + "_smoke");
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path project_path = project_dir / "librarydemo.pjx";
    const fs::path expected_output = output_dir / "LibraryDemo" / ("LibraryDemo." + extension);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    fs::create_directories(output_dir);

    write_text(project_dir / "librarymain.prg", "PROCEDURE InitLibrary\nLPARAMETERS tcMode\nRETURN\nENDPROC\n");
    write_text(project_dir / "helper.prg", "FUNCTION AddNumbers\nPARAMETERS tnLeft, tnRight\nRETURN 1\nENDFUNC\n");
    write_synthetic_project(project_path, project_dir, output_dir / ("LibraryDemo." + extension));

    const auto process = run_process_capture(
        build_host_path,
        {"build", "--project", project_path.string(), "--output-dir", output_dir.string()},
        temp_root);

    expect(process.exit_code == 0, "build host should succeed for " + extension + " library outputs");
    expect(process.stdout_text.find("status: ok") != std::string::npos,
           "build host should report success for " + extension + " outputs");
    expect(process.stdout_text.find("output.kind: " + extension) != std::string::npos,
           "build host should report the correct output kind for " + extension + " outputs");
    expect(process.stdout_text.find("primary.output.materialized: true") != std::string::npos,
           "build host should report a materialized primary output for " + extension + " outputs");
    expect(fs::exists(expected_output),
           "build host should materialize the requested primary output for " + extension + " outputs");

    const fs::path manifest_path = value_for_key(process.stdout_text, "manifest.path");
    const fs::path debug_manifest_path = value_for_key(process.stdout_text, "debug.manifest.path");
    const fs::path expected_ast_manifest = output_dir / "LibraryDemo" / ("LibraryDemo." + extension + ".ast.json");
    const fs::path expected_ir_manifest = output_dir / "LibraryDemo" / ("LibraryDemo." + extension + ".ir.json");
    const fs::path expected_transpiled_csharp = output_dir / "LibraryDemo" / ("LibraryDemo." + extension + ".transpiled.cs");
    const fs::path expected_audit_log = output_dir / "LibraryDemo" / "security_audit.log";
    expect(!manifest_path.empty(), "build host should report a manifest path for " + extension + " outputs");
    expect(!debug_manifest_path.empty(), "build host should report a debug-manifest path for " + extension + " outputs");
    const std::string init_library_source = (project_dir / "librarymain.prg").string();
    const std::string add_numbers_source = (project_dir / "helper.prg").string();
    const std::string manifest_text = manifest_path.empty() ? std::string{} : read_text(manifest_path);
    const std::string debug_manifest_text = debug_manifest_path.empty() ? std::string{} : read_text(debug_manifest_path);
    const std::vector<std::string> manifest_asset_lines = lines_with_prefix(manifest_text, "asset=");
    if (!manifest_path.empty()) {
        expect(manifest_text.find("primary_output_materialized=true") != std::string::npos,
               "build host manifest should record a materialized primary output for " + extension + " outputs");
        expect(manifest_text.find("project_title=LibraryDemo") != std::string::npos,
               "build host manifest should record the project title for " + extension + " outputs");
        expect(manifest_text.find("project_path=" + project_path.string()) != std::string::npos,
               "build host manifest should record the project path for " + extension + " outputs");
        expect(manifest_text.find("package_root=" + (output_dir / "LibraryDemo").string()) != std::string::npos,
               "build host manifest should record the package root for " + extension + " outputs");
        expect(manifest_text.find("content_root=" + (output_dir / "LibraryDemo" / "content").string()) != std::string::npos,
               "build host manifest should record the content root for " + extension + " outputs");
        expect(manifest_text.find("ast_manifest_path=" + expected_ast_manifest.string()) != std::string::npos,
               "build host manifest should record the AST manifest path for " + extension + " outputs");
        expect(manifest_text.find("ir_manifest_path=" + expected_ir_manifest.string()) != std::string::npos,
               "build host manifest should record the IR manifest path for " + extension + " outputs");
        expect(manifest_text.find("transpiled_csharp_path=" + expected_transpiled_csharp.string()) != std::string::npos,
               "build host manifest should record the transpiled C# path for " + extension + " outputs");
        expect(manifest_text.find("configuration=debug") != std::string::npos,
               "build host manifest should record the debug build configuration for " + extension + " outputs");
        expect(manifest_text.find("security_enabled=false") != std::string::npos,
               "build host manifest should record the disabled security state for " + extension + " outputs");
        expect(manifest_text.find("audit_log_path=" + expected_audit_log.string()) != std::string::npos,
               "build host manifest should record the audit log path for " + extension + " outputs");
        expect(manifest_text.find("runtime_host_sha256=") != std::string::npos,
               "build host manifest should record the runtime host SHA-256 digest for " + extension + " outputs");
        expect(manifest_text.find("security_roles=") != std::string::npos,
               "build host manifest should record the security-role count for " + extension + " outputs");
        expect(manifest_text.find("extension_payload=" + expected_output.string() + "|") != std::string::npos,
               "build host manifest should record the built primary output as an extension payload for " + extension + " outputs");
        if (extension == "dll") {
            expect(manifest_text.find("library_callable_convention=vfp_declare_default") != std::string::npos,
                   "build host manifest should record the VFP DLL calling convention contract");
            expect(manifest_text.find("library_api_manifest_path=") != std::string::npos,
                   "build host manifest should record the dedicated DLL API-manifest path");
            expect(manifest_text.find("library_function_arity=InitLibrary|1") != std::string::npos,
                   "build host manifest should record InitLibrary DLL arity");
            expect(manifest_text.find("library_function_arity=AddNumbers|2") != std::string::npos,
                   "build host manifest should record AddNumbers DLL arity");
            expect(manifest_text.find("library_function_kind=InitLibrary|procedure") != std::string::npos,
                   "build host manifest should record InitLibrary DLL routine kind");
            expect(manifest_text.find("library_function_kind=AddNumbers|function") != std::string::npos,
                   "build host manifest should record AddNumbers DLL routine kind");
            expect(manifest_text.find("library_function_source=InitLibrary|" + init_library_source + "|1") != std::string::npos,
                   "build host manifest should record InitLibrary DLL source provenance");
            expect(manifest_text.find("library_function_source=AddNumbers|" + add_numbers_source + "|1") != std::string::npos,
                   "build host manifest should record AddNumbers DLL source provenance");
            expect(manifest_text.find("library_function_parameters=InitLibrary|tcMode") != std::string::npos,
                   "build host manifest should record InitLibrary DLL parameter names");
            expect(manifest_text.find("library_function_parameters=AddNumbers|tnLeft|tnRight") != std::string::npos,
                   "build host manifest should record AddNumbers DLL parameter names");
            expect(manifest_text.find("library_function_parameter_declaration=InitLibrary|lparameters") != std::string::npos,
                   "build host manifest should record InitLibrary DLL parameter declaration style");
            expect(manifest_text.find("library_function_parameter_declaration=AddNumbers|parameters") != std::string::npos,
                   "build host manifest should record AddNumbers DLL parameter declaration style");
            expect(manifest_text.find("library_function_call_surface=InitLibrary|vfp_declare_default|int tcMode") != std::string::npos,
                   "build host manifest should record InitLibrary DLL call surface");
            expect(manifest_text.find("library_function_call_surface=AddNumbers|vfp_declare_default|int tnLeft, int tnRight") != std::string::npos,
                   "build host manifest should record AddNumbers DLL call surface");
        }
    }
    if (!debug_manifest_path.empty()) {
        expect(debug_manifest_text.find("primary_output_path=" + expected_output.string()) != std::string::npos,
               "build host debug manifest should record the materialized primary output path for " + extension + " outputs");
        expect(debug_manifest_text.find("project_title=LibraryDemo") != std::string::npos,
               "build host debug manifest should record the project title for " + extension + " outputs");
        expect(debug_manifest_text.find("project_path=" + project_path.string()) != std::string::npos,
               "build host debug manifest should record the project path for " + extension + " outputs");
        expect(debug_manifest_text.find("package_root=" + (output_dir / "LibraryDemo").string()) != std::string::npos,
               "build host debug manifest should record the package root for " + extension + " outputs");
        expect(debug_manifest_text.find("content_root=" + (output_dir / "LibraryDemo" / "content").string()) != std::string::npos,
               "build host debug manifest should record the content root for " + extension + " outputs");
        expect(debug_manifest_text.find("ast_manifest_path=" + expected_ast_manifest.string()) != std::string::npos,
               "build host debug manifest should record the AST manifest path for " + extension + " outputs");
        expect(debug_manifest_text.find("ir_manifest_path=" + expected_ir_manifest.string()) != std::string::npos,
               "build host debug manifest should record the IR manifest path for " + extension + " outputs");
        expect(debug_manifest_text.find("transpiled_csharp_path=" + expected_transpiled_csharp.string()) != std::string::npos,
               "build host debug manifest should record the transpiled C# path for " + extension + " outputs");
        expect(debug_manifest_text.find("configuration=debug") != std::string::npos,
               "build host debug manifest should record the debug build configuration for " + extension + " outputs");
        expect(debug_manifest_text.find("security_enabled=false") != std::string::npos,
               "build host debug manifest should record the disabled security state for " + extension + " outputs");
        expect(debug_manifest_text.find("audit_log_path=" + expected_audit_log.string()) != std::string::npos,
               "build host debug manifest should record the audit log path for " + extension + " outputs");
        const std::string security_role = manifest_value_for_key(manifest_text, "security_role");
        const std::string security_mode = manifest_value_for_key(manifest_text, "security_mode");
        const std::string runtime_host_sha256 = manifest_value_for_key(manifest_text, "runtime_host_sha256");
        const std::string security_roles = manifest_value_for_key(manifest_text, "security_roles");
        const std::string dotnet_enabled = manifest_value_for_key(manifest_text, "dotnet_enabled");
        const std::string dotnet_story = manifest_value_for_key(manifest_text, "dotnet_story");
        const std::string dotnet_policy_allowlist = manifest_value_for_key(manifest_text, "dotnet_policy_allowlist");
        const std::string dotnet_policy_denylist = manifest_value_for_key(manifest_text, "dotnet_policy_denylist");
        const std::string dotnet_parity_matrix_entries = manifest_value_for_key(manifest_text, "dotnet_parity_matrix_entries");
        const std::string dotnet_policy_allowlist_items = manifest_value_for_key(manifest_text, "dotnet_policy_allowlist_items");
        const std::string dotnet_policy_denylist_items = manifest_value_for_key(manifest_text, "dotnet_policy_denylist_items");
        const std::string dotnet_parity_matrix_count = manifest_value_for_key(manifest_text, "dotnet_parity_matrix_count");
        const std::string dotnet_gateway_task_primitives = manifest_value_for_key(manifest_text, "dotnet_gateway_task_primitives");
        const std::string dotnet_gateway_unsafe_reflection = manifest_value_for_key(manifest_text, "dotnet_gateway_unsafe_reflection");
        expect(debug_manifest_text.find("security_role=" + security_role) != std::string::npos,
               "build host debug manifest should mirror the effective security role for " + extension + " outputs");
        expect(debug_manifest_text.find("security_mode=" + security_mode) != std::string::npos,
               "build host debug manifest should mirror the security mode for " + extension + " outputs");
        expect(debug_manifest_text.find("runtime_host_sha256=" + runtime_host_sha256) != std::string::npos,
               "build host debug manifest should mirror the runtime host SHA-256 digest for " + extension + " outputs");
        expect(debug_manifest_text.find("security_roles=" + security_roles) != std::string::npos,
               "build host debug manifest should mirror the security-role count for " + extension + " outputs");
        expect(debug_manifest_text.find("dotnet_enabled=" + dotnet_enabled) != std::string::npos,
               "build host debug manifest should mirror the .NET availability flag for " + extension + " outputs");
        expect(debug_manifest_text.find("dotnet_story=" + dotnet_story) != std::string::npos,
               "build host debug manifest should mirror the .NET story for " + extension + " outputs");
        expect(debug_manifest_text.find("dotnet_policy_allowlist=" + dotnet_policy_allowlist) != std::string::npos,
               "build host debug manifest should mirror the .NET allowlist summary for " + extension + " outputs");
        expect(debug_manifest_text.find("dotnet_policy_denylist=" + dotnet_policy_denylist) != std::string::npos,
               "build host debug manifest should mirror the .NET denylist summary for " + extension + " outputs");
        expect(debug_manifest_text.find("dotnet_parity_matrix_entries=" + dotnet_parity_matrix_entries) != std::string::npos,
               "build host debug manifest should mirror the .NET parity summary for " + extension + " outputs");
        expect(debug_manifest_text.find("dotnet_policy_allowlist_items=" + dotnet_policy_allowlist_items) != std::string::npos,
               "build host debug manifest should mirror the .NET allowlist item count for " + extension + " outputs");
        expect(debug_manifest_text.find("dotnet_policy_denylist_items=" + dotnet_policy_denylist_items) != std::string::npos,
               "build host debug manifest should mirror the .NET denylist item count for " + extension + " outputs");
        expect(debug_manifest_text.find("dotnet_parity_matrix_count=" + dotnet_parity_matrix_count) != std::string::npos,
               "build host debug manifest should mirror the .NET parity item count for " + extension + " outputs");
        expect(debug_manifest_text.find("dotnet_gateway_task_primitives=" + dotnet_gateway_task_primitives) != std::string::npos,
               "build host debug manifest should mirror the .NET gateway allow decision for " + extension + " outputs");
        expect(debug_manifest_text.find("dotnet_gateway_unsafe_reflection=" + dotnet_gateway_unsafe_reflection) != std::string::npos,
               "build host debug manifest should mirror the .NET gateway deny decision for " + extension + " outputs");
        const std::vector<std::string> runtime_allowlist_items = lines_with_prefix(manifest_text, "dotnet_policy_allowlist_item=");
        const std::vector<std::string> debug_allowlist_items = lines_with_prefix(debug_manifest_text, "dotnet_policy_allowlist_item=");
        expect(debug_allowlist_items == runtime_allowlist_items,
               "build host debug manifest should mirror the .NET allowlist items for " + extension + " outputs");
        const std::vector<std::string> runtime_denylist_items = lines_with_prefix(manifest_text, "dotnet_policy_denylist_item=");
        const std::vector<std::string> debug_denylist_items = lines_with_prefix(debug_manifest_text, "dotnet_policy_denylist_item=");
        expect(debug_denylist_items == runtime_denylist_items,
               "build host debug manifest should mirror the .NET denylist items for " + extension + " outputs");
        const std::vector<std::string> runtime_parity_items = lines_with_prefix(manifest_text, "dotnet_parity_matrix_item=");
        const std::vector<std::string> debug_parity_items = lines_with_prefix(debug_manifest_text, "dotnet_parity_matrix_item=");
        expect(debug_parity_items == runtime_parity_items,
               "build host debug manifest should mirror the .NET parity entries for " + extension + " outputs");
        const std::vector<std::string> extensibility_summary_keys{
            "language_integration_count",
            "ai_feature_count",
            "extensibility_guardrail_count",
            "language_integrations",
            "ai_features"};
        for (const auto& key : extensibility_summary_keys) {
            const std::string value = manifest_value_for_key(manifest_text, key);
            expect(!value.empty(),
                   "build host runtime manifest should provide " + key + " for debug-manifest mirroring on " + extension + " outputs");
            expect(debug_manifest_text.find(key + "=" + value) != std::string::npos,
                   "build host debug manifest should mirror " + key + " for " + extension + " outputs");
        }
        const std::vector<std::string> runtime_language_integrations = lines_with_prefix(manifest_text, "language_integration=");
        const std::vector<std::string> debug_language_integrations = lines_with_prefix(debug_manifest_text, "language_integration=");
        expect(debug_language_integrations == runtime_language_integrations,
               "build host debug manifest should mirror language integration entries for " + extension + " outputs");
        const std::vector<std::string> runtime_ai_features = lines_with_prefix(manifest_text, "ai_feature=");
        const std::vector<std::string> debug_ai_features = lines_with_prefix(debug_manifest_text, "ai_feature=");
        expect(debug_ai_features == runtime_ai_features,
               "build host debug manifest should mirror AI feature entries for " + extension + " outputs");
        const std::vector<std::string> runtime_guardrails = lines_with_prefix(manifest_text, "extensibility_guardrail=");
        const std::vector<std::string> debug_guardrails = lines_with_prefix(debug_manifest_text, "extensibility_guardrail=");
        expect(debug_guardrails == runtime_guardrails,
               "build host debug manifest should mirror extensibility guardrails for " + extension + " outputs");
        const std::vector<std::string> runtime_feature_flags = lines_with_prefix(manifest_text, "feature_flag=");
        const std::vector<std::string> debug_feature_flags = lines_with_prefix(debug_manifest_text, "feature_flag=");
        expect(debug_feature_flags == runtime_feature_flags,
               "build host debug manifest should mirror runtime feature-flag lines for " + extension + " outputs");
        expect(debug_manifest_text.find("primary_output_materialized=true") != std::string::npos,
               "build host debug manifest should record a materialized primary output for " + extension + " outputs");
        expect(debug_manifest_text.find("extension_payload=" + expected_output.string() + "|") != std::string::npos,
               "build host debug manifest should record the built primary output as an extension payload for " + extension + " outputs");
        expect(!manifest_asset_lines.empty(),
               "build host manifest should record staged asset inventory for " + extension + " outputs");
        for (const auto& asset_line : manifest_asset_lines) {
            expect(debug_manifest_text.find(asset_line) != std::string::npos,
                   "build host debug manifest should mirror each staged asset line for " + extension + " outputs");
        }
        if (extension == "dll") {
            expect(debug_manifest_text.find("module_definition_path=") != std::string::npos,
                   "build host DLL debug manifest should record the module-definition path");
            expect(debug_manifest_text.find("library_api_manifest_path=") != std::string::npos,
                   "build host DLL debug manifest should record the dedicated API-manifest path");
            const fs::path module_definition_path = value_for_key(process.stdout_text, "module.definition");
            const fs::path library_api_manifest_path = value_for_key(process.stdout_text, "library.api.manifest");
            expect(debug_manifest_text.find("compiler_contract=" + module_definition_path.string() + "|") != std::string::npos,
                   "build host DLL debug manifest should record the module-definition compiler-contract digest");
            expect(debug_manifest_text.find("compiler_contract=" + library_api_manifest_path.string() + "|") != std::string::npos,
                   "build host DLL debug manifest should record the API-manifest compiler-contract digest");
            expect(debug_manifest_text.find("feature_flag=build.output.library_contract|true|build_output") != std::string::npos,
                   "build host DLL debug manifest should expose the library-contract feature flag");
            expect(debug_manifest_text.find("feature_flag=build.output.native_library_wrapper|true|build_output") != std::string::npos,
                   "build host DLL debug manifest should expose the native-wrapper feature flag");
            expect(debug_manifest_text.find("export_symbol=InitLibrary") != std::string::npos,
                   "build host DLL debug manifest should record discovered export symbols");
            expect(debug_manifest_text.find("export_symbol=AddNumbers") != std::string::npos,
                   "build host DLL debug manifest should record all export symbols");
        }
        if (extension == "fll") {
            expect(debug_manifest_text.find("module_definition_path=") != std::string::npos,
                   "build host FLL debug manifest should record the module-definition path");
            expect(debug_manifest_text.find("fll_api_manifest_path=") != std::string::npos,
                   "build host FLL debug manifest should record the dedicated API-manifest path");
            const fs::path module_definition_path = value_for_key(process.stdout_text, "module.definition");
            const fs::path fll_api_manifest_path = value_for_key(process.stdout_text, "fll.api.manifest");
            expect(debug_manifest_text.find("compiler_contract=" + module_definition_path.string() + "|") != std::string::npos,
                   "build host FLL debug manifest should record the module-definition compiler-contract digest");
            expect(debug_manifest_text.find("compiler_contract=" + fll_api_manifest_path.string() + "|") != std::string::npos,
                   "build host FLL debug manifest should record the API-manifest compiler-contract digest");
            expect(debug_manifest_text.find("feature_flag=build.output.library_contract|true|build_output") != std::string::npos,
                   "build host FLL debug manifest should expose the library-contract feature flag");
            expect(debug_manifest_text.find("feature_flag=build.output.native_library_wrapper|true|build_output") != std::string::npos,
                   "build host FLL debug manifest should expose the native-wrapper feature flag");
            expect(debug_manifest_text.find("feature_flag=build.output.fll_api_contract|true|build_output") != std::string::npos,
                   "build host FLL debug manifest should expose the FLL API-contract feature flag");
            expect(debug_manifest_text.find("export_symbol=InitLibrary") != std::string::npos,
                   "build host FLL debug manifest should record discovered routine export symbols");
            expect(debug_manifest_text.find("export_symbol=AddNumbers") != std::string::npos,
                   "build host FLL debug manifest should record all routine export symbols");
        }
    }

    if (fs::exists(expected_output) && native_symbol_dump_is_available()) {
        std::string symbol_error;
        const std::set<std::string> exported_symbols = read_native_exported_symbols(expected_output, symbol_error);
        if (exported_symbols.empty() && !symbol_error.empty()) {
            std::cerr << "FAIL: " << symbol_error << "\n";
        }

        const fs::path module_definition_path = value_for_key(process.stdout_text, "module.definition");
        const std::set<std::string> declared_module_symbols = read_module_definition_exports(module_definition_path);
        expect(exported_symbols == declared_module_symbols,
               "build host should preserve the module-definition export contract for " + extension + " outputs");

        if (extension == "dll") {
            const fs::path library_api_manifest_path = value_for_key(process.stdout_text, "library.api.manifest");
            const std::set<std::string> declared_api_symbols = read_library_api_declared_symbols(library_api_manifest_path);
            expect(exported_symbols == declared_api_symbols,
                   "build host should preserve the dedicated DLL API-manifest export contract");
            const std::string api_manifest = read_text(library_api_manifest_path);
            const fs::path wrapper_source_path = manifest_value_for_key(manifest_text, "native_wrapper_source_path");
            const std::string wrapper_source = wrapper_source_path.empty() ? std::string{} : read_text(wrapper_source_path);
            const fs::path wrapper_cmake_path = manifest_value_for_key(manifest_text, "native_wrapper_cmake_path");
            const std::string wrapper_cmake = wrapper_cmake_path.empty() ? std::string{} : read_text(wrapper_cmake_path);
            expect(api_manifest.find("output_kind=dll") != std::string::npos,
                   "build host DLL API manifest should declare the DLL output kind");
            expect(api_manifest.find("callable_convention=vfp_declare_default") != std::string::npos,
                   "build host DLL API manifest should declare the VFP DLL calling convention");
            expect(wrapper_source.find("static std::filesystem::path copperfin_wrapper_module_path(void* symbol_address)") != std::string::npos,
                   "build host DLL wrapper should derive its loaded module path");
            expect(wrapper_source.find("static std::filesystem::path copperfin_runtime_manifest_path(void* symbol_address)") != std::string::npos,
                   "build host DLL wrapper should derive a sibling manifest path");
            expect(wrapper_source.find("static std::filesystem::path copperfin_runtime_host_path(void* symbol_address)") != std::string::npos,
                   "build host DLL wrapper should derive a sibling runtime-host path");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeDescriptor") != std::string::npos,
                   "build host DLL wrapper should declare a shared bridge-descriptor surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeDescriptor copperfin_build_runtime_bridge_descriptor(") != std::string::npos,
                   "build host DLL wrapper should declare a bridge-descriptor helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeInvocation") != std::string::npos,
                   "build host DLL wrapper should declare a shared bridge-invocation surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeInvocation copperfin_build_runtime_bridge_invocation(") != std::string::npos,
                   "build host DLL wrapper should declare a bridge-invocation helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeParameter") != std::string::npos,
                   "build host DLL wrapper should declare a bridge-parameter surface");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeCall") != std::string::npos,
                   "build host DLL wrapper should declare a bridge-call surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeCall copperfin_build_runtime_bridge_call(") != std::string::npos,
                   "build host DLL wrapper should declare a bridge-call helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeReturn") != std::string::npos,
                   "build host DLL wrapper should declare a return-binding surface");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeResult") != std::string::npos,
                   "build host DLL wrapper should declare a bridge-result surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResult copperfin_build_runtime_bridge_result(") != std::string::npos,
                   "build host DLL wrapper should declare a bridge-result helper");
            expect(wrapper_source.find("const auto descriptor = copperfin_build_runtime_bridge_descriptor(\"InitLibrary\"") != std::string::npos,
                   "build host DLL wrapper should build a bridge descriptor for InitLibrary");
            expect(wrapper_source.find("const auto descriptor = copperfin_build_runtime_bridge_descriptor(\"AddNumbers\"") != std::string::npos,
                   "build host DLL wrapper should build a bridge descriptor for AddNumbers");
            expect(wrapper_source.find("const auto invocation = copperfin_build_runtime_bridge_invocation(descriptor);") != std::string::npos,
                   "build host DLL wrapper should build a bridge invocation from the descriptor");
            expect(wrapper_source.find("const auto call = copperfin_build_runtime_bridge_call(") != std::string::npos,
                   "build host DLL wrapper should build a bridge call from the invocation");
            expect(wrapper_source.find("const auto result = copperfin_build_runtime_bridge_result(") != std::string::npos,
                   "build host DLL wrapper should build a bridge result from the call");
            expect(wrapper_source.find("\"--library-export\"") != std::string::npos,
                   "build host DLL wrapper should encode the export name into the bridge invocation plan");
            expect(wrapper_source.find("{\"tcMode\", std::to_string(tcMode), \"int\"}") != std::string::npos,
                   "build host DLL wrapper should preserve the DLL placeholder argument binding");
            expect(wrapper_source.find("{std::to_string(-1), \"int\"}") != std::string::npos,
                   "build host DLL wrapper should preserve the DLL placeholder return binding");
            expect(wrapper_cmake.find("target_link_libraries(LibraryDemo PRIVATE dl)") != std::string::npos,
                   "build host DLL wrapper CMake should link dl on supported Unix hosts");
        }

        if (extension == "fll") {
            expect(manifest_text.find("fll_loader_entrypoint=FoxInfo") != std::string::npos,
                   "build host manifest should record the FLL loader entrypoint");
            expect(manifest_text.find("fll_registration_symbol=_FoxTable") != std::string::npos,
                   "build host manifest should record the FLL registration symbol");
            expect(manifest_text.find("fll_callable_signature=ParamBlk*") != std::string::npos,
                   "build host manifest should record the FLL callable signature");
            expect(manifest_text.find("fll_default_return_helper=_RetInt") != std::string::npos,
                   "build host manifest should record the FLL default return helper");
            expect(manifest_text.find("library_function_kind=InitLibrary|procedure") != std::string::npos,
                   "build host manifest should mirror InitLibrary FLL routine kind");
            expect(manifest_text.find("library_function_kind=AddNumbers|function") != std::string::npos,
                   "build host manifest should mirror AddNumbers FLL routine kind");
            expect(manifest_text.find("library_function_source=InitLibrary|" + init_library_source + "|1") != std::string::npos,
                   "build host manifest should mirror InitLibrary FLL source provenance");
            expect(manifest_text.find("library_function_source=AddNumbers|" + add_numbers_source + "|1") != std::string::npos,
                   "build host manifest should mirror AddNumbers FLL source provenance");
            expect(manifest_text.find("library_function_parameters=InitLibrary|tcMode") != std::string::npos,
                   "build host manifest should mirror InitLibrary FLL parameter names");
            expect(manifest_text.find("library_function_parameters=AddNumbers|tnLeft|tnRight") != std::string::npos,
                   "build host manifest should mirror AddNumbers FLL parameter names");
            expect(manifest_text.find("library_function_parameter_declaration=InitLibrary|lparameters") != std::string::npos,
                   "build host manifest should mirror InitLibrary FLL parameter declaration style");
            expect(manifest_text.find("library_function_parameter_declaration=AddNumbers|parameters") != std::string::npos,
                   "build host manifest should mirror AddNumbers FLL parameter declaration style");
            expect(manifest_text.find("library_function_call_surface=InitLibrary|ParamBlk*|_RetInt") != std::string::npos,
                   "build host manifest should mirror InitLibrary FLL callable surface");
            expect(manifest_text.find("library_function_call_surface=AddNumbers|ParamBlk*|_RetInt") != std::string::npos,
                   "build host manifest should mirror AddNumbers FLL callable surface");
        }
    }

    if (fs::exists(expected_output) && native_symbol_dump_is_available()) {
        std::string symbol_error;
        const std::set<std::string> exported_symbols = read_native_exported_symbols(expected_output, symbol_error);
        if (exported_symbols.empty() && !symbol_error.empty()) {
            std::cerr << "FAIL: " << symbol_error << "\n";
        }

        const fs::path module_definition_path = value_for_key(process.stdout_text, "module.definition");
        const std::set<std::string> declared_module_symbols = read_module_definition_exports(module_definition_path);
        expect(exported_symbols == declared_module_symbols,
               "build host should preserve the module-definition export contract for " + extension + " outputs");

        if (extension == "dll") {
            const fs::path library_api_manifest_path = value_for_key(process.stdout_text, "library.api.manifest");
            const std::set<std::string> declared_api_symbols = read_library_api_declared_symbols(library_api_manifest_path);
            expect(exported_symbols == declared_api_symbols,
                   "build host should preserve the dedicated DLL API-manifest export contract");
            const std::string api_manifest = read_text(library_api_manifest_path);
            expect(api_manifest.find("output_kind=dll") != std::string::npos,
                   "build host DLL API manifest should declare the DLL output kind");
            expect(api_manifest.find("callable_convention=vfp_declare_default") != std::string::npos,
                   "build host DLL API manifest should declare the VFP DLL calling convention");
        }

        if (extension == "fll") {
            const fs::path fll_api_manifest_path = value_for_key(process.stdout_text, "fll.api.manifest");
            const std::set<std::string> declared_api_symbols = read_fll_api_declared_symbols(fll_api_manifest_path);
            expect(exported_symbols == declared_api_symbols,
                   "build host should preserve the API-manifest export contract for fll outputs");
            const std::string api_manifest = read_text(fll_api_manifest_path);
            const fs::path wrapper_source_path = manifest_value_for_key(manifest_text, "native_wrapper_source_path");
            const std::string wrapper_source = wrapper_source_path.empty() ? std::string{} : read_text(wrapper_source_path);
            const fs::path wrapper_cmake_path = manifest_value_for_key(manifest_text, "native_wrapper_cmake_path");
            const std::string wrapper_cmake = wrapper_cmake_path.empty() ? std::string{} : read_text(wrapper_cmake_path);
            expect(api_manifest.find("registration_symbol=_FoxTable") != std::string::npos,
                   "build host FLL manifest should declare the FoxTable registration symbol");
            expect(api_manifest.find("callable_signature=ParamBlk*") != std::string::npos,
                   "build host FLL manifest should declare the ParamBlk callable signature");
            expect(api_manifest.find("default_return_helper=_RetInt") != std::string::npos,
                   "build host FLL manifest should declare the default return helper");
            expect(wrapper_source.find("static std::filesystem::path copperfin_wrapper_module_path(void* symbol_address)") != std::string::npos,
                   "build host FLL wrapper should derive its loaded module path");
            expect(wrapper_source.find("static std::filesystem::path copperfin_runtime_manifest_path(void* symbol_address)") != std::string::npos,
                   "build host FLL wrapper should derive a sibling manifest path");
            expect(wrapper_source.find("static std::filesystem::path copperfin_runtime_host_path(void* symbol_address)") != std::string::npos,
                   "build host FLL wrapper should derive a sibling runtime-host path");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeDescriptor") != std::string::npos,
                   "build host FLL wrapper should declare a shared bridge-descriptor surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeDescriptor copperfin_build_runtime_bridge_descriptor(") != std::string::npos,
                   "build host FLL wrapper should declare a bridge-descriptor helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeInvocation") != std::string::npos,
                   "build host FLL wrapper should declare a shared bridge-invocation surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeInvocation copperfin_build_runtime_bridge_invocation(") != std::string::npos,
                   "build host FLL wrapper should declare a bridge-invocation helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeParameter") != std::string::npos,
                   "build host FLL wrapper should declare a bridge-parameter surface");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeCall") != std::string::npos,
                   "build host FLL wrapper should declare a bridge-call surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeCall copperfin_build_runtime_bridge_call(") != std::string::npos,
                   "build host FLL wrapper should declare a bridge-call helper");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeReturn") != std::string::npos,
                   "build host FLL wrapper should declare a return-binding surface");
            expect(wrapper_source.find("struct CopperfinRuntimeBridgeResult") != std::string::npos,
                   "build host FLL wrapper should declare a bridge-result surface");
            expect(wrapper_source.find("static CopperfinRuntimeBridgeResult copperfin_build_runtime_bridge_result(") != std::string::npos,
                   "build host FLL wrapper should declare a bridge-result helper");
            expect(wrapper_source.find("const char* routine_kind;") != std::string::npos,
                   "build host FLL wrapper should record routine kind fields in the FoxInfo table");
            expect(wrapper_source.find("const char* source_path;") != std::string::npos,
                   "build host FLL wrapper should record source-path fields in the FoxInfo table");
            expect(wrapper_source.find("unsigned int source_line;") != std::string::npos,
                   "build host FLL wrapper should record source-line fields in the FoxInfo table");
            expect(wrapper_source.find("const char* parameter_declaration_kind;") != std::string::npos,
                   "build host FLL wrapper should record parameter-declaration fields in the FoxInfo table");
            expect(wrapper_source.find("const char* parameter_names;") != std::string::npos,
                   "build host FLL wrapper should record parameter-name fields in the FoxInfo table");
            expect(wrapper_source.find("{\"InitLibrary\", &InitLibrary, \"procedure\", \"" + init_library_source + "\", 1U, \"lparameters\", \"tcMode\", 1U}") != std::string::npos,
                   "build host FLL wrapper should record InitLibrary metadata in the FoxInfo table");
            expect(wrapper_source.find("{\"AddNumbers\", &AddNumbers, \"function\", \"" + add_numbers_source + "\", 1U, \"parameters\", \"tnLeft|tnRight\", 2U}") != std::string::npos,
                   "build host FLL wrapper should record AddNumbers metadata in the FoxInfo table");
            expect(wrapper_source.find("const auto descriptor = copperfin_build_runtime_bridge_descriptor(\"InitLibrary\"") != std::string::npos,
                   "build host FLL wrapper should build a bridge descriptor for InitLibrary");
            expect(wrapper_source.find("const auto descriptor = copperfin_build_runtime_bridge_descriptor(\"AddNumbers\"") != std::string::npos,
                   "build host FLL wrapper should build a bridge descriptor for AddNumbers");
            expect(wrapper_source.find("const auto invocation = copperfin_build_runtime_bridge_invocation(descriptor);") != std::string::npos,
                   "build host FLL wrapper should build a bridge invocation from the descriptor");
            expect(wrapper_source.find("const auto call = copperfin_build_runtime_bridge_call(") != std::string::npos,
                   "build host FLL wrapper should build a bridge call from the invocation");
            expect(wrapper_source.find("const auto result = copperfin_build_runtime_bridge_result(") != std::string::npos,
                   "build host FLL wrapper should build a bridge result from the call");
            expect(wrapper_source.find("\"--library-export\"") != std::string::npos,
                   "build host FLL wrapper should encode the export name into the bridge invocation plan");
            expect(wrapper_source.find("{{\"parm\", std::to_string(static_cast<unsigned long long>(reinterpret_cast<std::uintptr_t>(parm))), \"ParamBlk*\"}}") != std::string::npos,
                   "build host FLL wrapper should preserve the ParamBlk call-surface binding");
            expect(wrapper_source.find("{std::to_string(-1), \"_RetInt(int)\"}") != std::string::npos,
                   "build host FLL wrapper should preserve the FLL placeholder return binding");
            expect(wrapper_cmake.find("target_link_libraries(LibraryDemo PRIVATE dl)") != std::string::npos,
                   "build host FLL wrapper CMake should link dl on supported Unix hosts");
            expect(api_manifest.find("function_arity=InitLibrary|1") != std::string::npos,
                   "build host FLL manifest should declare InitLibrary arity");
            expect(api_manifest.find("function_arity=AddNumbers|2") != std::string::npos,
                   "build host FLL manifest should declare AddNumbers arity");
            expect(api_manifest.find("function_kind=InitLibrary|procedure") != std::string::npos,
                   "build host FLL manifest should declare InitLibrary routine kind");
            expect(api_manifest.find("function_kind=AddNumbers|function") != std::string::npos,
                   "build host FLL manifest should declare AddNumbers routine kind");
            expect(api_manifest.find("function_source=InitLibrary|" + init_library_source + "|1") != std::string::npos,
                   "build host FLL manifest should declare InitLibrary source provenance");
            expect(api_manifest.find("function_source=AddNumbers|" + add_numbers_source + "|1") != std::string::npos,
                   "build host FLL manifest should declare AddNumbers source provenance");
            expect(api_manifest.find("function_parameters=InitLibrary|tcMode") != std::string::npos,
                   "build host FLL manifest should declare InitLibrary parameter names");
            expect(api_manifest.find("function_parameters=AddNumbers|tnLeft|tnRight") != std::string::npos,
                   "build host FLL manifest should declare AddNumbers parameter names");
            expect(api_manifest.find("function_parameter_declaration=InitLibrary|lparameters") != std::string::npos,
                   "build host FLL manifest should declare InitLibrary parameter declaration style");
            expect(api_manifest.find("function_parameter_declaration=AddNumbers|parameters") != std::string::npos,
                   "build host FLL manifest should declare AddNumbers parameter declaration style");
            expect(api_manifest.find("function_call_surface=InitLibrary|ParamBlk*|_RetInt") != std::string::npos,
                   "build host FLL manifest should declare InitLibrary callable surface");
            expect(api_manifest.find("function_call_surface=AddNumbers|ParamBlk*|_RetInt") != std::string::npos,
                   "build host FLL manifest should declare AddNumbers callable surface");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void run_app_build_host_smoke(const std::string& build_host_path) {
    namespace fs = std::filesystem;

    expect(fs::exists(build_host_path), "build host executable should exist before running the APP smoke test");
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_build_host_app_smoke";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path project_path = project_dir / "archivedemo.pjx";
    const fs::path expected_output = output_dir / "ArchiveDemo" / "ArchiveDemo.app";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    fs::create_directories(output_dir);

    write_text(project_dir / "main.prg", "DO helper\nRETURN\n");
    write_text(project_dir / "helper.prg", "WAIT WINDOW 'archived'\nRETURN\n");
    write_text(project_dir / "config.txt", "mode=demo");
    write_synthetic_app_project(project_path, project_dir, expected_output);

    const auto process = run_process_capture(
        build_host_path,
        {"build", "--project", project_path.string(), "--output-dir", output_dir.string()},
        temp_root);

    expect(process.exit_code == 0, "build host should succeed for APP outputs");
    expect(process.stdout_text.find("status: ok") != std::string::npos,
           "build host should report success for APP outputs");
    expect(process.stdout_text.find("output.kind: app") != std::string::npos,
           "build host should report the correct output kind for APP outputs");
    expect(process.stdout_text.find("primary.output.materialized: true") != std::string::npos,
           "build host should report a materialized primary output for APP outputs");
    expect(fs::exists(expected_output),
           "build host should materialize the requested APP primary output");

    const fs::path manifest_path = value_for_key(process.stdout_text, "manifest.path");
    expect(!manifest_path.empty(), "build host should report a manifest path for APP outputs");
    if (!manifest_path.empty()) {
        const std::string manifest_text = read_text(manifest_path);
        expect(manifest_text.find("primary_output_materialized=true") != std::string::npos,
               "build host manifest should record a materialized APP primary output");
        expect(manifest_text.find("extension_payload=" + expected_output.string() + "|") != std::string::npos,
               "build host manifest should record the APP archive as an extension payload");
    }

    const std::string app_archive = read_text(expected_output);
    expect(app_archive.find("copperfin_app_archive_version=1") != std::string::npos,
           "build host APP output should identify the Copperfin APP archive format");
    expect(app_archive.find("archive_contract=copperfin_content_archive_v1") != std::string::npos,
           "build host APP output should declare the Copperfin APP archive contract");
    const auto archive_payloads = parse_app_archive_payloads(app_archive);
    expect(archive_payloads.contains("main.prg"),
           "build host APP archive should carry the startup program payload");
    expect(archive_payloads.contains("helper.prg"),
           "build host APP archive should carry supporting program payloads");
    expect(archive_payloads.contains("config.txt"),
           "build host APP archive should carry non-program payloads");
    if (archive_payloads.contains("main.prg")) {
        expect(archive_payloads.at("main.prg") == "DO helper\nRETURN\n",
               "build host APP archive should preserve startup program bytes");
    }
    if (archive_payloads.contains("helper.prg")) {
        expect(archive_payloads.at("helper.prg") == "WAIT WINDOW 'archived'\nRETURN\n",
               "build host APP archive should preserve supporting program bytes");
    }
    if (archive_payloads.contains("config.txt")) {
        expect(archive_payloads.at("config.txt") == "mode=demo",
               "build host APP archive should preserve non-program asset bytes");
    }

    fs::remove_all(temp_root, ignored);
}

void run_fxp_build_host_smoke(const std::string& build_host_path) {
    namespace fs = std::filesystem;

    expect(fs::exists(build_host_path), "build host executable should exist before running the FXP smoke test");
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_build_host_fxp_smoke";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path project_path = project_dir / "compiledemo.pjx";
    const fs::path expected_output = output_dir / "CompileDemo" / "CompileDemo.fxp";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);
    fs::create_directories(output_dir);

    write_text(project_dir / "main.prg",
               "LOCAL nValue\n"
               "nValue = 1\n"
               "DO worker\n"
               "RETURN\n"
               "PROCEDURE worker\n"
               "WAIT WINDOW 'hello'\n"
               "RETURN\n"
               "ENDPROC\n");
    write_synthetic_fxp_project(project_path, project_dir, expected_output);

    const auto process = run_process_capture(
        build_host_path,
        {"build", "--project", project_path.string(), "--output-dir", output_dir.string()},
        temp_root);

    expect(process.exit_code == 0, "build host should succeed for FXP outputs");
    expect(process.stdout_text.find("status: ok") != std::string::npos,
           "build host should report success for FXP outputs");
    expect(process.stdout_text.find("output.kind: fxp") != std::string::npos,
           "build host should report the correct output kind for FXP outputs");
    expect(process.stdout_text.find("primary.output.materialized: true") != std::string::npos,
           "build host should report a materialized primary output for FXP outputs");
    expect(fs::exists(expected_output),
           "build host should materialize the requested FXP primary output");

    const fs::path manifest_path = value_for_key(process.stdout_text, "manifest.path");
    expect(!manifest_path.empty(), "build host should report a manifest path for FXP outputs");
    if (!manifest_path.empty()) {
        const std::string manifest_text = read_text(manifest_path);
        expect(manifest_text.find("primary_output_materialized=true") != std::string::npos,
               "build host manifest should record a materialized FXP primary output");
        expect(manifest_text.find("extension_payload=" + expected_output.string() + "|") != std::string::npos,
               "build host manifest should record the FXP contract as an extension payload");
    }

    const std::string fxp_contract = read_text(expected_output);
    expect(fxp_contract.find("copperfin_fxp_contract_version=1") != std::string::npos,
           "build host FXP output should identify the Copperfin FXP contract format");
    expect(fxp_contract.find("token_contract=copperfin_logical_statement_contract_v1") != std::string::npos,
           "build host FXP output should declare the Copperfin FXP contract");
    expect(fxp_contract.find("output_kind=fxp") != std::string::npos,
           "build host FXP output should embed the FXP token-manifest content");
    expect(fxp_contract.find("statement=MAIN|") != std::string::npos,
           "build host FXP output should preserve main-scope logical statements");
    expect(fxp_contract.find("statement=worker|") != std::string::npos,
           "build host FXP output should preserve routine-scope logical statements");
    expect(fxp_contract.find("WAIT WINDOW 'hello'") != std::string::npos,
           "build host FXP output should preserve statement text");

    fs::remove_all(temp_root, ignored);
}

void run_default_runtime_host_resolution_smoke(const std::string& build_host_path) {
    namespace fs = std::filesystem;

    expect(fs::exists(build_host_path), "build host executable should exist before running runtime-host resolution smoke test");
    const fs::path original_build_host = fs::path(build_host_path);
    const fs::path original_runtime_host_dir = original_build_host.parent_path();
    const std::vector<fs::path> runtime_host_candidates{
        original_runtime_host_dir / "copperfin_runtime_host.exe",
        original_runtime_host_dir / "copperfin_runtime_host"
    };
    fs::path source_runtime_host;
    for (const auto& candidate : runtime_host_candidates) {
        if (fs::exists(candidate)) {
            source_runtime_host = candidate;
            break;
        }
    }
    expect(!source_runtime_host.empty(), "runtime host executable should be discoverable for runtime-host resolution smoke test");
    if (source_runtime_host.empty()) {
        return;
    }

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_build_host_runtime_host_fallback";
    const fs::path temp_bundle = temp_root / "bundle";
    const fs::path temp_project_dir = temp_bundle / "project";
    const fs::path output_dir = temp_bundle / "output";
    const fs::path temp_build_host = temp_bundle / original_build_host.filename();
    const fs::path temp_runtime_host = temp_bundle / source_runtime_host.filename();
    const fs::path project_path = temp_project_dir / "hostresolution.pjx";
    const fs::path expected_output = output_dir / "HostResolutionDemo" / "HostResolutionDemo.exe";

    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_project_dir);
    fs::create_directories(output_dir);

    std::error_code copy_error;
    fs::copy_file(original_build_host, temp_build_host, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "copied build-host fixture should be readable");
    if (copy_error) {
        fs::remove_all(temp_root, ignored);
        return;
    }
    fs::copy_file(source_runtime_host, temp_runtime_host, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "copied runtime-host fixture should be readable");
    if (copy_error) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    write_text(temp_bundle / "main.prg", "WAIT WINDOW 'host-resolution'\nRETURN\n");
    write_synthetic_executable_project(project_path, temp_project_dir, expected_output);

    {
        ScopedEnvironmentValue clear_runtime_host_env("COPPERFIN_RUNTIME_HOST_PATH");

        const auto process = run_process_capture(
            temp_build_host.string(),
            {"build", "--project", project_path.string(), "--output-dir", output_dir.string()},
            temp_root);

        expect(process.exit_code == 0, "build host should resolve runtime host from executable directory");
        expect(process.stdout_text.find("status: ok") != std::string::npos,
               "runtime-host resolution smoke test should report status: ok");
        expect(process.stdout_text.find("output.kind: executable") != std::string::npos,
               "runtime-host resolution smoke test should build an executable output");
        expect(process.stdout_text.find("primary.output.materialized: true") != std::string::npos,
               "runtime-host resolution smoke test should materialize executable output");
        const fs::path manifest_path = value_for_key(process.stdout_text, "manifest.path");
        expect(!manifest_path.empty(), "runtime-host resolution smoke test should report manifest path");
        if (!manifest_path.empty()) {
            const std::string manifest_text = read_text(manifest_path);
            expect(manifest_text.find("runtime_host_sha256=") != std::string::npos,
                   "runtime-host resolution smoke test should persist runtime host digest");
            expect(manifest_text.find("primary_output_materialized=true") != std::string::npos,
                   "runtime-host resolution smoke test manifest should report primary output materialized");
        }
        expect(fs::exists(expected_output), "runtime-host fallback test should materialize requested executable");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: test_build_host_output <path-to-build-host>\n";
        return EXIT_FAILURE;
    }

    run_library_build_host_smoke(argv[1], "dll");
    run_library_build_host_smoke(argv[1], "fll");
    run_app_build_host_smoke(argv[1]);
    run_fxp_build_host_smoke(argv[1]);
    run_default_runtime_host_resolution_smoke(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
